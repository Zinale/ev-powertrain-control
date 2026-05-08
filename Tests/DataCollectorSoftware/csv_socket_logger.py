import socket
import csv
import datetime
from datetime import time
import sys

ESP_IP = '192.168.1.239'

# ---------------------------------------------------------------------------
# Status-word bit definitions (must match Inverter.h)
# ---------------------------------------------------------------------------
_SW_BITS = {
    8:  "SysRdy",
    9:  "Err",
    10: "Warn",
    11: "QDCon",
    12: "DCon",
    13: "QInvOn",
    14: "InvOn",
    15: "Der",
}

def decode_status_word(sw: int) -> str:
    """Return a human-readable string of active status-word bits."""
    active = [name for bit, name in _SW_BITS.items() if sw & (1 << bit)]
    return f"0x{sw:04X} [{', '.join(active) if active else 'none'}]"

# CSV column index of StatusWord (0-based, must match the header row below)
_SW_COL_IDX = 12
PORT = 8080
CSV_FILE = f'dati_sensori_{datetime.datetime.now().strftime("%Y%m%d_%H%M%S")}.csv'
# ----------------------

def main():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        print(f"Tentativo di connessione a {ESP_IP}:{PORT}...", flush=True)
        try:
            s.connect((ESP_IP, PORT))
            s.settimeout(5.0)  # Timeout di 5 secondi
            print("Connesso con successo! In attesa di dati...\n", flush=True)
        except Exception as e:
            print(f"Errore di connessione: {e}", flush=True)
            sys.exit(1)
        
        with open(CSV_FILE, mode='a', newline='') as file:
            writer = csv.writer(file)
            
            #HEADERS
            writer.writerow(["Time_ms","TempMotor","TempInverter","TempIGBT","Voltage","Speed","Iq","Id","TorqueMotor","PedalPerc","InvState","ErrCode","StatusWord","ErrInfo1","PhaseU_mA","PhaseV_mA","PhaseW_mA","Power_W","TorqueSetpoint","TorqueLimitDyn","NTC1","NTC2","NTC3"])

            buffer = ""
            start_time = datetime.datetime.now()
            timeout_counter = 0
            while True:
                try:
                    data = s.recv(1024)
                    timeout_counter = 0  # Reset on successful recv
                    if not data:
                        print("Connessione chiusa dall'ESP32.", flush=True)
                        break
                    
                    print(f"[DEBUG] Ricevuti {len(data)} byte grezzi: {data[:50]}", flush=True)
                    buffer += data.decode('utf-8')
                    
                    # Pulisce il buffer da caratteri indesiderati
                    buffer = buffer.replace('\x00', '')  # Rimuove byte null
                    buffer = buffer.replace('\r\n', '\n')  
                    buffer = buffer.replace('\r', '\n')  
                    
                    while '\n' in buffer:
                        line, buffer = buffer.split('\n', 1)
                        line = line.strip()
                        
                        if line:
                            # Ignora le righe di debug, inclusa la connessione del bridge
                            if "ESP32 Bridge" in line or "ERROR" in line or "Conditions" in line or "(" in line or ")" in line or "|" in line or ":" in line:
                                print(f"[INFO] Riga di debug ignorata: {line}")
                                continue
                            
                            row = [val.strip() for val in line.split(',')]

                            # 1° CONTROLLO: Numero di colonne corretto
                            EXPECTED_COLS = 23
                            if len(row) != EXPECTED_COLS:
                                print(f"[WARNING] Riga ignorata (colonne errate {len(row)}/{EXPECTED_COLS}): {line}")
                                continue

                            # 2° CONTROLLO: I dati devono essere numerici (evita "0*0" o simili)
                            is_valid = True
                            parsed_row = []
                            for val in row:
                                try:
                                    # Proviamo a convertire in float per verifica
                                    parsed_val = float(val)
                                    parsed_row.append(parsed_val)
                                except ValueError:
                                    is_valid = False
                                    break
                            
                            if not is_valid:
                                print(f"[WARNING] Riga ignorata (caratteri non numerici): {line}")
                                continue

                            
                            # Se passa i controlli, possiamo scrivere nel CSV (usiamo i dati originali testuali o quelli convertiti)
                            writer.writerow(row)
                            file.flush()
                            
                            # Decode and print status-word bits
                            try:
                                sw_val = int(float(row[_SW_COL_IDX]))
                                print(f"  [SW] {decode_status_word(sw_val)}", flush=True)
                            except (IndexError, ValueError):
                                pass
                            
                except socket.timeout:
                    timeout_counter += 1
                    print(f"[TIMEOUT] Socket timeout #{timeout_counter} - nessun dato ricevuto", flush=True)
                    if timeout_counter >= 3:  # Dopo 3 timeout, esci
                        print("[ERROR] Troppi timeout - ESP32 non sta inviando dati!", flush=True)
                        break
                    continue
                except KeyboardInterrupt:
                    print("\nRegistrazione interrotta dall'utente.", flush=True)
                    break
                except Exception as e:
                    print(f"\nErrore di rete: {e}", flush=True)
                    break

if __name__ == '__main__':
    main()