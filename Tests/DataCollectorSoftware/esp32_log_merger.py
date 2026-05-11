#!/usr/bin/env python3
"""
esp32_log_merger.py
===================
Unifica e pulisce i file di log ESP32 registrati in modalità WiFi (CSV)
e Bluetooth (TXT seriale da terminale).

Differenze gestite automaticamente:
  - WiFi CSV  : header + riga "ESP32 Bridge connesso" + dati puri
  - BT  TXT   : prefisso timestamp "HH:MM:SS.mmm " + righe di log non-dati
                ([HB][BT]..., [INV_R]:...)
  - Corruzione caratteri  : '!' → '1', '#' → '3', '-.' → '-0.', '- .' → '-0.'
  - Valori sentinella NTC : -999.0 e -273.15 → NaN
  - Jitter BT di Time_ms  : arrotonda ai multipli di 500 ms (opzionale, default: on)

Filtro rumore (Savitzky-Golay, default: on):
  - Aggiunge colonne `<nome>_filt` per i canali analogici rumorosi
  - I NaN (sentinella, lacune) vengono preservati

Uso
---
    python esp32_log_merger.py <cartella_log>              # un CSV pulito per file (default)
    python esp32_log_merger.py <cartella_log> --merge      # unisce tutto in un solo CSV
    python esp32_log_merger.py <cartella_log> -o out.csv --merge
    python esp32_log_merger.py <cartella_log> --no-filter --no-round-time
    python esp32_log_merger.py <singolo_file.csv>

Requisiti
---------
    pip install pandas numpy scipy
"""

from __future__ import annotations

import argparse
import re
import sys
from datetime import datetime
from pathlib import Path
from typing import Optional


# ===========================================================================
# Selezione interattiva file/cartella via tkinter
# ===========================================================================

def _pick_path_ui() -> Path:
    """
    Apre una finestra Tk che chiede all'utente di scegliere tra:
      - un singolo file CSV/TXT
      - una cartella contenente i log
    Restituisce il Path selezionato, oppure esce se l'utente annulla.
    """
    try:
        import tkinter as tk
        from tkinter import filedialog, messagebox, ttk
    except ImportError:
        sys.exit("tkinter non disponibile. Passa il percorso come argomento da riga di comando.")

    root = tk.Tk()
    root.title("ESP32 Log Merger")
    root.resizable(False, False)

    # Centra la finestra
    root.update_idletasks()
    w, h = 420, 160
    x = (root.winfo_screenwidth()  - w) // 2
    y = (root.winfo_screenheight() - h) // 2
    root.geometry(f"{w}x{h}+{x}+{y}")

    result: dict[str, Path | None] = {"path": None}

    tk.Label(root, text="Seleziona il percorso da elaborare:",
             font=("Segoe UI", 10)).pack(pady=(18, 6))

    path_var = tk.StringVar()
    entry = tk.Entry(root, textvariable=path_var, width=48, state="readonly")
    entry.pack(padx=20)

    btn_frame = tk.Frame(root)
    btn_frame.pack(pady=10)

    def _pick_file():
        p = filedialog.askopenfilename(
            title="Scegli un file di log",
            filetypes=[
                ("Log ESP32", "dati_sensori_*.csv serial_*.txt"),
                ("CSV WiFi",  "*.csv"),
                ("TXT BT",    "*.txt"),
                ("Tutti",     "*.*"),
            ],
        )
        if p:
            path_var.set(p)

    def _pick_folder():
        p = filedialog.askdirectory(title="Scegli una cartella di log")
        if p:
            path_var.set(p)

    def _confirm():
        val = path_var.get().strip()
        if not val:
            messagebox.showwarning("Nessuna selezione", "Seleziona un file o una cartella.")
            return
        chosen = Path(val)
        if not chosen.exists():
            messagebox.showerror("Errore", f"Percorso non trovato:\n{chosen}")
            return
        result["path"] = chosen
        root.destroy()

    def _cancel():
        root.destroy()

    ttk.Button(btn_frame, text="📄 File",    command=_pick_file,   width=12).pack(side="left", padx=4)
    ttk.Button(btn_frame, text="📁 Cartella", command=_pick_folder, width=12).pack(side="left", padx=4)
    ttk.Button(btn_frame, text="✓ Avvia",   command=_confirm,     width=12).pack(side="left", padx=4)
    ttk.Button(btn_frame, text="✗ Annulla",  command=_cancel,      width=12).pack(side="left", padx=4)

    root.mainloop()

    if result["path"] is None:
        sys.exit("Operazione annullata.")
    return result["path"]

# ---------------------------------------------------------------------------
# Import con messaggio di errore leggibile
# ---------------------------------------------------------------------------
try:
    import numpy as np
    import pandas as pd
    from scipy.signal import savgol_filter
except ImportError as _e:
    sys.exit(
        f"Dipendenza mancante: {_e}\n"
        "Installa con:  pip install pandas numpy scipy"
    )

# ---------------------------------------------------------------------------
# Definizione colonne (deve corrispondere al protocollo firmware ESP32)
# ---------------------------------------------------------------------------
COLUMNS = [
    "Time_ms",
    "TempMotor", "TempInverter", "TempIGBT",
    "Voltage", "Speed",
    "Iq", "Id", "TorqueMotor",
    "PedalPerc",
    "InvState", "ErrCode", "StatusWord", "ErrInfo1",
    "PhaseU_mA", "PhaseV_mA", "PhaseW_mA",
    "Power_W", "TorqueSetpoint", "TorqueLimitDyn",
    "NTC1", "NTC2", "NTC3",
]

# ---------------------------------------------------------------------------
# Canali da filtrare: (window_length dispari, polyorder)
# window_length controlla la "larghezza" dello smoothing;
# polyorder controlla quanto il filtro segue le variazioni rapide.
# ---------------------------------------------------------------------------
FILTER_CFG: dict[str, tuple[int, int]] = {
    # (window_length, polyorder) — window in num. campioni (1 campione = 1000 ms)
    "PhaseU_mA":    (7, 2),   # correnti di fase: più rumorose, window moderata
    "PhaseV_mA":    (7, 2),
    "PhaseW_mA":    (7, 2),
    "NTC1":         (5, 2),   # temperature NTC
    "NTC2":         (5, 2),
    "NTC3":         (5, 2),
    "TempMotor":    (5, 2),
    "TempInverter": (5, 2),
    "TempIGBT":     (5, 2),
    "Voltage":      (5, 2),
    "Speed":        (5, 2),
    "Power_W":      (5, 2),
    "Iq":           (5, 2),
    "Id":           (5, 2),
    "TorqueMotor":  (5, 2),
}

# Valori sentinella da considerare NaN
SENTINEL_VALUES = [-999.0, -273.15]

# Pattern per il prefisso timestamp del terminale BT: "18:07:35.101 "
_BT_TIMESTAMP_RE = re.compile(r"^\d{2}:\d{2}:\d{2}\.\d{3} ")

# Colonne intere (non vanno filtrate con Savitzky-Golay)
INT_COLUMNS = {"InvState", "ErrCode", "StatusWord", "ErrInfo1",
               "PedalPerc", "TorqueSetpoint", "TorqueLimitDyn"}

# ---------------------------------------------------------------------------
# Limiti fisici per canale: (min, max)  →  valori fuori range → clampati
#
# Temperature
#   TempMotor      : AMK motore, range operativo 0–150 °C (max assoluto 200 °C)
#   TempInverter   : modulo potenza inverter, limite AMK ~100 °C
#   TempIGBT       : giunzione IGBT, limite di derating AMK ~100 °C
#   NTC1/2/3       : sonde esterne NTC, range generico -40..200 °C
#
# Elettriche / meccaniche
#   Voltage        : bus DC pacco batterie, range operativo -50..600 V
#   Speed          : AMK DD5 n_max ≈ 20000 RPM
#   Iq / Id        : correnti d-q in A; picco AMK DD5 ≈ ±200 A
#   PhaseU/V/W_mA  : correnti di fase in mA; ±200 A = ±200 000 mA
#   TorqueMotor    : coppia misurata (Nm); AMK DD5 picco ≈ ±25 Nm
#   TorqueSetpoint : setpoint coppia richiesto (Nm), stesso range
#   TorqueLimitDyn : limite dinamico coppia (Nm), solo positivo 0..25 Nm
#   Power_W        : potenza meccanica/elettrica in W; 4×25 Nm×2000 rad/s ≈ ±35 kW
#   PedalPerc      : posizione pedale acceleratore 0–100 %
#
# Digitali / stato
#   InvState       : -1 (OFF) … 5 (ERROR) per protocollo firmware MCU
#   ErrCode        : codice errore AMK a 16 bit, 0–65535
#   StatusWord     : parola di stato AMK a 16 bit, 0–65535
#   ErrInfo1       : informazione errore AMK a 16 bit, 0–65535
# ---------------------------------------------------------------------------
PHYSICAL_LIMITS: dict[str, tuple[float, float]] = {
    # Temperature (°C)
    "TempMotor":      (  -40.0,   200.0),
    "TempInverter":   (  -40.0,   100.0),
    "TempIGBT":       (  -40.0,   100.0),
    "NTC1":           (  -40.0,   200.0),
    "NTC2":           (  -40.0,   200.0),
    "NTC3":           (  -40.0,   200.0),
    # Elettriche (tensione / corrente / potenza)
    "Voltage":        (  -50.0,   600.0),
    "Iq":             ( -150,   150.0),
    "Id":             ( -150,   150.0),
    "PhaseU_mA":      (-150_000.0, 150_000.0),
    "PhaseV_mA":      (-150_000.0, 150_000.0),
    "PhaseW_mA":      (-150_000.0, 150_000.0),
    "Power_W":        (-35_000.0,  35_000.0),
    # Meccaniche
    "Speed":          (     -5000, 20_000.0),
    "TorqueMotor":    (   -25.0,    25.0),
    "TorqueSetpoint": (   -2100,    2100),
    "TorqueLimitDyn": (     -25,    25.0),
    # Pedale
    "PedalPerc":      (     0.0,   100.0),
    # Stato / codici (protezione da overflow/corruzione UART)
    "InvState":       (    -1.0,     5.0),
    "ErrCode":        (     0.0, 65535.0),
    "StatusWord":     (     0.0, 65535.0),
    "ErrInfo1":       (     0.0, 65535.0),
}


# ===========================================================================
# Correzione caratteri corrotti
# ===========================================================================

def _fix_field(value: str) -> str:
    """
    Corregge i caratteri tipicamente corrotti nella trasmissione BT SPP:
      '!'  → '1'         (es. 4!.1  → 41.1)
      '#'  → '3'         (es. 2#59  → 2359)
      '*'  → '.'         (es. 21*9  → 21.9, 49*2 → 49.2)
      '-.' → '-0.'       (es. -.2   → -0.2)
      '- .' → '-0.'      (es. - .0  → -0.0)
      spazi interni      (es. 290 0 → 2900)
      '/'  tra cifre → '.'  (es. 0/1  → 0.1)
    """
    v = value.strip()
    v = v.replace("!", "1")
    v = v.replace("#", "3")
    # asterisco come separatore decimale: "21*9" → "21.9"
    v = re.sub(r"(\d)\*(\d)", r"\1.\2", v)
    # slash come separatore decimale: "0/1" → "0.1"
    v = re.sub(r"(\d)/(\d)", r"\1.\2", v)
    # spazio dopo il segno meno: "- .x" → "-0.x"
    v = re.sub(r"^-\s+\.", "-0.", v)
    # meno direttamente prima del punto decimale: "-.x" → "-0.x"
    v = re.sub(r"^-(\.\d)", r"-0\1", v)
    # spazi interni nei campi numerici (es. "290 0" → "2900")
    v = v.replace(" ", "")
    return v


def _parse_row(raw_fields: list[str]) -> Optional[list[str]]:
    """
    Applica _fix_field a ogni campo, verifica che Time_ms sia un intero
    e porta la riga a esattamente len(COLUMNS) campi (riempie con "" le
    colonne mancanti, es. NTC in file vecchi).
    """
    fields = [_fix_field(f) for f in raw_fields]

    # Una virgola persa shifta tutti i campi successivi: rifiuta righe con
    # troppo poche o troppe colonne. Min = 23-3 (senza NTC), Max = 23+3.
    _min_f = len(COLUMNS) - 3   # 20
    _max_f = len(COLUMNS) + 3   # 26
    if not (_min_f <= len(fields) <= _max_f):
        return None

    try:
        int(fields[0])            # Time_ms deve essere intero
    except ValueError:
        return None

    # Pad con stringa vuota se mancano le colonne NTC (file firmware vecchio)
    while len(fields) < len(COLUMNS):
        fields.append("")

    return fields[: len(COLUMNS)]


# ===========================================================================
# Parser per file WiFi CSV
# ===========================================================================

def parse_wifi_csv(path: Path) -> tuple[list[list[str]], int]:
    """
    Legge un file CSV WiFi.
    Salta: riga header, riga diagnostica "ESP32 Bridge connesso",
           qualsiasi riga non-dati.
    Ritorna (righe_valide, n_scartate).
    """
    rows: list[list[str]] = []
    n_discarded = 0
    with open(path, encoding="utf-8", errors="replace") as fh:
        for line in fh:
            stripped = line.strip()
            if not stripped:
                continue
            # Salta header e righe di diagnostica
            if stripped.startswith("Time_ms") or not stripped[0].isdigit():
                continue
            row = _parse_row(stripped.split(","))
            if row is not None:
                rows.append(row)
            else:
                n_discarded += 1
    return rows, n_discarded


# ===========================================================================
# Parser per file BT TXT (seriale)
# ===========================================================================

def parse_bt_txt(path: Path) -> tuple[list[list[str]], int]:
    """
    Legge un file TXT dal terminale BT.
    Ogni riga dati ha il prefisso "HH:MM:SS.mmm " che viene rimosso.
    Le righe di log non-dati ([HB][BT]..., [INV_R]:...) vengono scartate.
    Ritorna (righe_valide, n_scartate).
    """
    rows: list[list[str]] = []
    n_discarded = 0
    with open(path, encoding="utf-8", errors="replace") as fh:
        for line in fh:
            stripped = line.strip()
            if not stripped:
                continue

            # Rimuovi prefisso timestamp seriale
            data_part = _BT_TIMESTAMP_RE.sub("", stripped)

            # Scarta header e righe non-dati (log, heartbeat, ecc.)
            if not data_part or not data_part[0].isdigit():
                continue

            row = _parse_row(data_part.split(","))
            if row is not None:
                rows.append(row)
            else:
                n_discarded += 1
    return rows, n_discarded


# ===========================================================================
# Caricamento cartella
# ===========================================================================

def load_folder(folder: Path) -> pd.DataFrame:
    """
    Scansiona la cartella cercando:
      - dati_sensori_*.csv  → WiFi
      - serial_*.txt        → BT
    Restituisce un DataFrame grezzo con colonna "Source" aggiuntiva.
    """
    all_rows: list[list] = []
    sources: list[str]   = []
    source_files: list[str] = []

    n_wifi = n_bt = 0
    n_discarded_total = 0
    for path in sorted(folder.iterdir()):
        name_lower = path.name.lower()
        if path.suffix.lower() == ".csv" and name_lower.startswith("dati_sensori"):
            print(f"  [WiFi] {path.name}")
            rows, n_disc = parse_wifi_csv(path)
            all_rows.extend(rows)
            sources.extend(["WiFi"] * len(rows))
            source_files.extend([path.name] * len(rows))
            n_discarded_total += n_disc
            n_wifi += 1
        elif path.suffix.lower() == ".txt" and name_lower.startswith("serial"):
            print(f"  [BT]   {path.name}")
            rows, n_disc = parse_bt_txt(path)
            all_rows.extend(rows)
            sources.extend(["BT"] * len(rows))
            source_files.extend([path.name] * len(rows))
            n_discarded_total += n_disc
            n_bt += 1

    if not all_rows:
        sys.exit("Nessun file di log trovato nella cartella.")

    print(f"  → {n_wifi} file WiFi, {n_bt} file BT, {len(all_rows)} righe valide")
    if n_discarded_total:
        print(f"  [WARN] Scartate {n_discarded_total} righe corrotte (errori UART / merge di campi)")

    df = pd.DataFrame(all_rows, columns=COLUMNS)
    df.insert(0, "SourceFile", source_files)
    df.insert(0, "Source", sources)

    # Conversione numerica
    for col in COLUMNS:
        df[col] = pd.to_numeric(df[col], errors="coerce")

    # Valori sentinella → NaN
    for sv in SENTINEL_VALUES:
        mask = df[COLUMNS].isin([sv])
        df[COLUMNS] = df[COLUMNS].where(~mask, other=np.nan)

    return df


def load_single_file(path: Path) -> pd.DataFrame:
    """Carica e pulisce un singolo file (WiFi o BT)."""
    name_lower = path.name.lower()
    if path.suffix.lower() == ".csv" and name_lower.startswith("dati_sensori"):
        rows, n_disc = parse_wifi_csv(path)
        source = "WiFi"
    elif path.suffix.lower() == ".txt" and name_lower.startswith("serial"):
        rows, n_disc = parse_bt_txt(path)
        source = "BT"
    else:
        return pd.DataFrame()

    if not rows:
        return pd.DataFrame()

    if n_disc:
        print(f"  [WARN] Scartate {n_disc} righe corrotte (errori UART / merge di campi)")

    df = pd.DataFrame(rows, columns=COLUMNS)
    df.insert(0, "SourceFile", path.name)
    df.insert(0, "Source", source)

    for col in COLUMNS:
        df[col] = pd.to_numeric(df[col], errors="coerce")

    for sv in SENTINEL_VALUES:
        mask = df[COLUMNS].isin([sv])
        df[COLUMNS] = df[COLUMNS].where(~mask, other=np.nan)

    return df


# ===========================================================================
# Post-processing comune
# ===========================================================================

def round_time_ms(df: pd.DataFrame, step: int = 1000) -> pd.DataFrame:
    """
    Arrotonda Time_ms al multiplo di `step` più vicino.
    Utile per compensare il jitter BT (es. 1010 → 1000, 2005 → 2000).
    Lascia invariati i Time_ms = 0 (pre-sync inverter).
    """
    df = df.copy()
    nonzero = df["Time_ms"] != 0
    df.loc[nonzero, "Time_ms"] = (
        (df.loc[nonzero, "Time_ms"] / step).round() * step
    ).astype("Int64")
    return df


def drop_duplicates_time(df: pd.DataFrame) -> pd.DataFrame:
    """
    Rimuove righe duplicate su Time_ms all'interno dello stesso SourceFile.
    In caso di duplicato, preferisce la riga WiFi se disponibile, altrimenti la prima.
    """
    before = len(df)

    # Ordina per dare priorità a WiFi sui duplicati
    source_order = {"WiFi": 0, "BT": 1}
    df = df.copy()
    df["_src_order"] = df["Source"].map(source_order).fillna(99)
    df.sort_values(["SourceFile", "Time_ms", "_src_order"], inplace=True)
    df.drop_duplicates(subset=["SourceFile", "Time_ms"], keep="first", inplace=True)
    df.drop(columns=["_src_order"], inplace=True)
    df.reset_index(drop=True, inplace=True)

    dropped = before - len(df)
    if dropped:
        print(f"  → {dropped} righe duplicate su (SourceFile, Time_ms) rimosse")
    return df


# ---------------------------------------------------------------------------
# Correzione monotonia temporale
# ---------------------------------------------------------------------------

# Se Time_ms scende di più di questa soglia assume una riconnessione ESP32
# (il firmware MCU fa ripartire il contatore da zero); aggiunge un offset
# per mantenere la continuità del grafico.
# Se la caduta è più piccola (bit UART flippato sul timestamp), la riga
# viene semplicemente scartata.
_TIME_RESET_THRESHOLD_MS = 5_000


def _restore_digit_drop(raw_t: float, t_max: float, tolerance_ms: float = 5000.0) -> float | None:
    """
    Verifica se raw_t è un timestamp a cui manca una cifra decimale iniziale
    (digit-drop UART: es. 178938 → 78938, 215938 → 15938).
    Se il valore "ripristinato" cade entro tolerance_ms dopo t_max,
    lo restituisce; altrimenti restituisce None.
    """
    if raw_t <= 0:
        return None
    n_digits = len(str(int(raw_t)))
    power = 10 ** n_digits
    # Prova il blocco di cifra iniziale che rende il candidato ≥ t_max
    for leading in range(int(t_max / power), int(t_max / power) + 3):
        if leading < 0:
            continue
        candidate = raw_t + leading * power
        if 0 <= candidate - t_max <= tolerance_ms:
            return candidate
    return None


def enforce_time_monotone(
    df: pd.DataFrame,
    reset_threshold: int = _TIME_RESET_THRESHOLD_MS,
) -> pd.DataFrame:
    """
    Assicura che Time_ms sia strettamente crescente all'interno di ogni
    SourceFile, nell'ordine in cui le righe compaiono nel file originale.

    Tre casi di non-monotonicità:
      - Digit-drop UART (es. 178938 → 78938):
          Un singolo byte perso fa sembrare il timestamp arretrato di ~10^n ms.
          Viene corretto il solo timestamp anomalo; l'offset rimane invariato.
      - Caduta grande (> reset_threshold ms), non digit-drop:
          riconnessione ESP32 → il contatore MCU è ripartito da zero.
          Aggiunge un offset pari a (max_precedente + 1000 ms) in modo che
          i dati del nuovo segmento continuino senza salti indietro.
      - Caduta piccola (≤ reset_threshold ms):
          timestamp UART corrotto → la riga viene scartata.
    """
    if df.empty or "Time_ms" not in df.columns:
        return df

    df = df.copy().reset_index(drop=True)
    keep = np.ones(len(df), dtype=bool)

    sf_col = "SourceFile" if "SourceFile" in df.columns else None
    group_labels = df[sf_col] if sf_col else pd.Series(["_all_"] * len(df), index=df.index)

    n_reset = 0
    n_corrupt = 0
    n_digit_fix = 0

    for sf in group_labels.unique():
        positions = group_labels[group_labels == sf].index.tolist()
        t_raw = df.loc[positions, "Time_ms"].to_numpy(dtype=float, na_value=np.nan)

        offset: float = 0.0
        t_max: float = -np.inf

        for i, pos in enumerate(positions):
            raw_t = t_raw[i]
            if np.isnan(raw_t):
                continue

            t_adj = raw_t + offset

            if t_adj < t_max:
                drop = t_max - t_adj
                if drop > reset_threshold:
                    # Prima controlla se è un digit-drop (singolo byte UART perso)
                    corrected = _restore_digit_drop(raw_t + offset, t_max)
                    if corrected is not None:
                        df.at[pos, "Time_ms"] = corrected
                        t_max = corrected
                        n_digit_fix += 1
                        continue
                    # Vero reset MCU / riconnessione: allunga la linea temporale
                    offset = t_max + 1000.0 - raw_t
                    t_adj = raw_t + offset
                    n_reset += 1
                else:
                    # Timestamp UART corrotto (piccolo jitter): scarta la riga
                    keep[pos] = False
                    n_corrupt += 1
                    continue

            df.at[pos, "Time_ms"] = t_adj
            t_max = max(t_max, t_adj)

    if n_digit_fix:
        print(f"  [TIME] {n_digit_fix} timestamp corretti (digit-drop UART: cifra iniziale ripristinata)")
    if n_reset:
        print(f"  [TIME] {n_reset} riconnessione/i rilevata/e: Time_ms offsettato per continuità")
    if n_corrupt:
        print(f"  [TIME] {n_corrupt} righe scartate per timestamp UART corrotto (non monotono)")

    return df[keep].reset_index(drop=True)


# ===========================================================================
# Clipping fisico
# ===========================================================================

def apply_physical_limits(df: pd.DataFrame) -> pd.DataFrame:
    """
    Clampla i valori fuori dal range fisico definito in PHYSICAL_LIMITS
    al rispettivo limite minimo o massimo consentito.
    I valori NaN (sentinella, lacune) non vengono toccati.
    Opera su una copia del DataFrame.
    """
    df = df.copy()
    n_clipped = 0
    for col, (lo, hi) in PHYSICAL_LIMITS.items():
        if col not in df.columns:
            continue
        mask_lo = df[col].notna() & (df[col] < lo)
        mask_hi = df[col].notna() & (df[col] > hi)
        n_out = mask_lo.sum() + mask_hi.sum()
        if n_out:
            df.loc[mask_lo, col] = lo
            df.loc[mask_hi, col] = hi
            n_clipped += n_out
    if n_clipped:
        print(f"  [CLIP] {n_clipped} valori fuori range fisico → clampati al limite")
    return df


# ===========================================================================
# Filtro Savitzky-Golay
# ===========================================================================

def apply_filter(df: pd.DataFrame) -> pd.DataFrame:
    """
    Applica Savitzky-Golay ai canali analogici rumorosi.
    Sovrascrive le colonne originali con i valori filtrati (nessuna ridondanza).
    Il filtro viene applicato PER SEGMENTO: i gap temporali > GAP_THRESHOLD_MS
    vengono trattati come discontinuità; le finestre del filtro non "attraversano"
    mai un buco dati, evitando transizioni spurie.
    I NaN vengono interpolati solo all'interno del segmento, poi ripristinati.
    """
    GAP_THRESHOLD_MS = 5000  # gap > 5 s = segmento separato

    df = df.copy()
    filtered_cols = []

    # --- Calcola i confini dei segmenti continui --------------------------
    time_arr = pd.to_numeric(df["Time_ms"], errors="coerce").to_numpy(dtype=float)
    diffs = np.diff(time_arr)
    # indici di inizio di ogni nuovo segmento (il primo è sempre 0)
    breaks = np.where((diffs > GAP_THRESHOLD_MS) | np.isnan(diffs))[0] + 1
    seg_starts = np.concatenate([[0], breaks])
    seg_ends   = np.concatenate([breaks, [len(df)]])
    segments   = list(zip(seg_starts.tolist(), seg_ends.tolist()))

    if len(segments) > 1:
        print(f"  [FILTER] {len(segments)} segmenti continui rilevati (gap > {GAP_THRESHOLD_MS} ms)")

    # --- Applica il filtro su ogni segmento per ogni colonna ---------------
    for col, (win, poly) in FILTER_CFG.items():
        if col not in df.columns:
            continue

        series = df[col].astype(float).copy()

        for seg_start, seg_end in segments:
            seg = series.iloc[seg_start:seg_end]
            nan_mask = seg.isna()
            valid_count = (~nan_mask).sum()

            if valid_count < win:
                # Segmento troppo corto: lascia invariato
                continue

            # Interpolazione lineare per gestire i NaN interni al segmento
            seg_interp = seg.interpolate(method="linear", limit_direction="both")
            filtered_values = savgol_filter(
                seg_interp.to_numpy().astype(float),
                window_length=win,
                polyorder=poly,
            )
            result = pd.Series(filtered_values, index=seg.index)
            result[nan_mask] = np.nan          # ripristina i NaN originali
            series.iloc[seg_start:seg_end] = result.values

        df[col] = series
        filtered_cols.append(col)

    if filtered_cols:
        print(f"  → Filtro in-place su: {', '.join(filtered_cols)}")

    return df


# ===========================================================================
# Main
# ===========================================================================

def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(
        description=(
            "Unifica e pulisce i log ESP32 WiFi (.csv) e BT (.txt).\n"
            "Output default: <cartella>/merged_<nome_cartella>.csv"
        ),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    p.add_argument("path", type=Path, nargs="?", default=None,
                   help="Cartella o singolo file (.csv/.txt). "
                        "Se omesso, si apre una finestra di selezione.")
    p.add_argument("-o", "--output", type=Path, default=None,
                   help="Percorso del CSV di output.")
    p.add_argument("--no-filter", action="store_true",
                   help="Disabilita il filtro Savitzky-Golay.")
    p.add_argument("--no-round-time", action="store_true",
                   help="Non arrotondare Time_ms al multiplo di 1000 ms.")
    p.add_argument("--no-dedup", action="store_true",
                   help="Non rimuovere righe duplicate su Time_ms.")
    p.add_argument("--merge", action="store_true",
                   help=(
                       "Unisce tutti i file della cartella in un unico CSV. "
                       "Per default ogni file viene elaborato separatamente."
                   ))
    p.add_argument("--sort", action="store_true", default=True,
                   help="Ordina per Time_ms (default: True).")
    return p


def process_df(df: pd.DataFrame, args: argparse.Namespace) -> pd.DataFrame:
    """Applica round, monotonia temporale, dedup, sort, clipping fisico e filtro."""
    if not args.no_round_time:
        df = round_time_ms(df)

    # Corregge riconnessioni e timestamp UART corrotti prima del dedup/sort
    df = enforce_time_monotone(df)

    if not args.no_dedup:
        df = drop_duplicates_time(df)

    if args.sort:
        df.sort_values("Time_ms", inplace=True, ignore_index=True)

    # Clamping fisico: porta i valori fuori range al limite (es. pedale > 100 → 100, Voltage > 650 → 650)
    df = apply_physical_limits(df)

    if not args.no_filter:
        df = apply_filter(df)

    return df


def main() -> None:
    args = build_parser().parse_args()

    # Se non è stato passato nessun argomento, apri la UI di selezione
    if args.path is None:
        args.path = _pick_path_ui()

    input_path = args.path.resolve()
    ts_str = datetime.now().strftime("%Y%m%d_%H%M%S")

    # ── Modalità: singolo file ───────────────────────────────────────────────
    if input_path.is_file():
        name_lower = input_path.name.lower()
        is_wifi = input_path.suffix.lower() == ".csv" and name_lower.startswith("dati_sensori")
        is_bt   = input_path.suffix.lower() == ".txt" and name_lower.startswith("serial")
        if not (is_wifi or is_bt):
            sys.exit(f"Errore: '{input_path.name}' non è un file di log riconosciuto "
                     f"(atteso dati_sensori_*.csv oppure serial_*.txt).")

        label = "WiFi" if is_wifi else "BT"
        print(f"\n  [{label}] {input_path.name}")
        df = load_single_file(input_path)
        if df.empty:
            sys.exit("Nessun dato valido nel file.")

        df = process_df(df, args)
        out_path = args.output or (input_path.parent / (input_path.stem + "_clean.csv"))
        df_out = df.drop(columns=[c for c in ("Source", "SourceFile") if c in df.columns])
        df_out.to_csv(out_path, index=False, float_format="%.4f")
        print(f"\n✓ Salvato: {out_path}")
        print(f"  {len(df_out)} righe × {len(df_out.columns)} colonne")
        return

    if not input_path.is_dir():
        sys.exit(f"Errore: '{input_path}' non è né un file né una cartella valida.")

    folder = input_path

    # ── Modalità: merge unico (--merge) ─────────────────────────────────────
    if args.merge:
        print(f"\nModalità MERGE – carico tutti i log da: {folder}\n")
        df = load_folder(folder)

        time_range = (
            f"{df['Time_ms'].min():.0f} – {df['Time_ms'].max():.0f} ms"
            if df["Time_ms"].notna().any() else "N/A"
        )
        print(f"  Intervallo Time_ms: {time_range}")

        df = process_df(df, args)

        out_path = args.output or (folder / f"merged_{folder.name}_{ts_str}.csv")
        df_out = df.drop(columns=[c for c in ("Source", "SourceFile") if c in df.columns])
        df_out.to_csv(out_path, index=False, float_format="%.4f")

        print(f"\n✓ Salvato: {out_path}")
        print(f"  {len(df_out)} righe × {len(df_out.columns)} colonne")
        return

    # ── Modalità default: un CSV pulito per file ─────────────────────────────
    print(f"\nElaborazione singoli file in: {folder}\n")
    out_dir = folder
    processed = 0

    for path in sorted(folder.iterdir()):
        name_lower = path.name.lower()
        is_wifi = path.suffix.lower() == ".csv" and name_lower.startswith("dati_sensori")
        is_bt   = path.suffix.lower() == ".txt" and name_lower.startswith("serial")
        if not (is_wifi or is_bt):
            continue

        label = "WiFi" if is_wifi else "BT"
        print(f"  [{label}] {path.name}")
        df = load_single_file(path)
        if df.empty:
            print(f"    → Nessun dato valido, file saltato.")
            continue

        df = process_df(df, args)
        out_path = out_dir / (path.stem + "_clean.csv")
        df_out = df.drop(columns=[c for c in ("Source", "SourceFile") if c in df.columns])
        df_out.to_csv(out_path, index=False, float_format="%.4f")
        print(f"    → {len(df_out)} righe → {out_path.name}\n")
        processed += 1

    if processed == 0:
        sys.exit("Nessun file di log trovato.")
    print(f"✓ Elaborati {processed} file.")


if __name__ == "__main__":
    main()
