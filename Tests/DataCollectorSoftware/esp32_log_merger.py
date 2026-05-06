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
    python esp32_log_merger.py <cartella_log>
    python esp32_log_merger.py <cartella_log> -o merged.csv
    python esp32_log_merger.py <cartella_log> --no-filter --no-round-time
    python esp32_log_merger.py <cartella_log> --separate   # un CSV per file

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
    # (window_length, polyorder) — window in num. campioni (1 campione = 500 ms)
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
# Limiti fisici per canale: (min, max)  →  valori fuori range → NaN
# ---------------------------------------------------------------------------
PHYSICAL_LIMITS: dict[str, tuple[float, float]] = {
    "PedalPerc":    (  0.0,  100.0),   # percentuale pedale 0–100 %
    "Voltage":      (-50.0,  650.0),   # bus DC
    "TempMotor":    (0,  200.0),
    "TempInverter": (0,  100.0),
    "TempIGBT":     (0,  100.0),
    "NTC1":         (0,  200.0),
    "NTC2":         (0,  200.0),
    "NTC3":         (0,  200.0),
    "Speed":        (0, 20000.0),
    "Power_W":      (-5000.0, 35000.0),
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

def round_time_ms(df: pd.DataFrame, step: int = 500) -> pd.DataFrame:
    """
    Arrotonda Time_ms al multiplo di `step` più vicino.
    Utile per compensare il jitter BT (es. 501 → 500, 1501 → 1500).
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


def enforce_time_monotone(
    df: pd.DataFrame,
    reset_threshold: int = _TIME_RESET_THRESHOLD_MS,
) -> pd.DataFrame:
    """
    Assicura che Time_ms sia strettamente crescente all'interno di ogni
    SourceFile, nell'ordine in cui le righe compaiono nel file originale.

    Due casi di non-monotonicità:
      - Caduta grande (> reset_threshold ms):
          riconnessione ESP32 → il contatore MCU è ripartito da zero.
          Aggiunge un offset pari a (max_precedente + 500 ms) in modo che
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
                    # Riconnessione: allunga la linea temporale
                    offset = t_max + 500.0 - raw_t
                    t_adj = raw_t + offset
                    n_reset += 1
                else:
                    # Timestamp UART corrotto: scarta la riga
                    keep[pos] = False
                    n_corrupt += 1
                    continue

            df.at[pos, "Time_ms"] = t_adj
            t_max = max(t_max, t_adj)

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
    Imposta NaN sui valori fuori dal range fisico definito in PHYSICAL_LIMITS.
    Opera in-place su una copia del DataFrame.
    """
    df = df.copy()
    n_clipped = 0
    for col, (lo, hi) in PHYSICAL_LIMITS.items():
        if col not in df.columns:
            continue
        mask = df[col].notna() & ((df[col] < lo) | (df[col] > hi))
        n_out = mask.sum()
        if n_out:
            df.loc[mask, col] = np.nan
            n_clipped += n_out
    if n_clipped:
        print(f"  [CLIP] {n_clipped} valori fuori range fisico → NaN")
    return df


# ===========================================================================
# Filtro Savitzky-Golay
# ===========================================================================

def apply_filter(df: pd.DataFrame) -> pd.DataFrame:
    """
    Applica Savitzky-Golay ai canali analogici rumorosi.
    Sovrascrive le colonne originali con i valori filtrati (nessuna ridondanza).
    I NaN vengono interpolati per il calcolo del filtro,
    poi ripristinati nel risultato.
    """
    df = df.copy()
    filtered_cols = []

    for col, (win, poly) in FILTER_CFG.items():
        if col not in df.columns:
            continue

        series = df[col].astype(float)
        nan_mask = series.isna()
        valid_count = (~nan_mask).sum()

        if valid_count < win:
            # Troppo pochi dati: lascia la colonna invariata
            filtered_cols.append(col)
            continue

        # Interpolazione lineare per gestire i NaN durante il filtro
        series_interp = series.interpolate(method="linear", limit_direction="both")
        filtered_values = savgol_filter(
            series_interp.to_numpy().astype(float),
            window_length=win,
            polyorder=poly,
        )
        result = pd.Series(filtered_values, index=df.index)
        result[nan_mask] = np.nan          # ripristina i NaN originali
        df[col] = result                   # sovrascrive l'originale
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
                   help="Non arrotondare Time_ms al multiplo di 500 ms.")
    p.add_argument("--no-dedup", action="store_true",
                   help="Non rimuovere righe duplicate su Time_ms.")
    p.add_argument("--separate", action="store_true",
                   help=(
                       "Produce un CSV pulito per ogni file di input "
                       "invece di un unico file unificato."
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

    # Clipping fisico: rimuove valori fuori range (es. pedale > 100, Voltage > 650)
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
        df.to_csv(out_path, index=False, float_format="%.4f")
        print(f"\n✓ Salvato: {out_path}")
        print(f"  {len(df)} righe × {len(df.columns)} colonne")
        return

    if not input_path.is_dir():
        sys.exit(f"Errore: '{input_path}' non è né un file né una cartella valida.")

    folder = input_path

    # ── Modalità: un CSV pulito per file ────────────────────────────────────
    if args.separate:
        print(f"\nModalità SEPARATA – elaboro ogni file in {folder}\n")
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
            df.to_csv(out_path, index=False, float_format="%.4f")
            print(f"    → {len(df)} righe → {out_path.name}\n")
            processed += 1

        if processed == 0:
            sys.exit("Nessun file di log trovato.")
        print(f"✓ Elaborati {processed} file.")
        return

    # ── Modalità: merge unico ───────────────────────────────────────────────
    print(f"\nCaricamento log da: {folder}\n")    # folder è già risolto sopra
    df = load_folder(folder)

    time_range = (
        f"{df['Time_ms'].min():.0f} – {df['Time_ms'].max():.0f} ms"
        if df["Time_ms"].notna().any() else "N/A"
    )
    print(f"  Intervallo Time_ms: {time_range}")

    df = process_df(df, args)

    out_path = args.output or (folder / f"merged_{folder.name}_{ts_str}.csv")
    df.to_csv(out_path, index=False, float_format="%.4f")

    print(f"\n✓ Salvato: {out_path}")
    print(f"  {len(df)} righe × {len(df.columns)} colonne")


if __name__ == "__main__":
    main()
