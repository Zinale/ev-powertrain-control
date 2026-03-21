import socket
import csv
import datetime
from datetime import time
import sys

ESP_IP = '192.168.1.220'
PORT = 8080
CSV_FILE = f'dati_sensori_{datetime.datetime.now().strftime("%Y%m%d_%H%M%S")}.csv'
# ----------------------

def main():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        print(f"Tentativo di connessione a {ESP_IP}:{PORT}...")
        try:
            s.connect((ESP_IP, PORT))
            print("Connesso con successo! In attesa di dati...\n")
        except Exception as e:
            print(f"Errore di connessione: {e}")
            sys.exit(1)
        
        with open(CSV_FILE, mode='a', newline='') as file:
            writer = csv.writer(file)
            
            #HEADERS
            writer.writerow(['TempMotor','TempInverter','TempIGBT','Voltage','Speed','Id','Iq','TorqueMotor','PedalPerc','NTC1', 'NTC2', 'NTC3'])
            
            buffer = ""
            start_time = datetime.datetime.now()
            while True:
                try:
                    data = s.recv(1024)
                    if not data:
                        print("Connessione chiusa dall'ESP32.")
                        break
                        
                    # Decodifica i byte ricevuti in stringa
                    buffer += data.decode('utf-8')
                    
                    # Estrae e processa le righe complete
                    while '\n' in buffer:
                        line, buffer = buffer.split('\n', 1)
                        line = line.strip()
                        
                        if line:
                            print(f"Ricevuto: {line}")
                            if "ERROR" in line or "Conditions" in line or "(" in line or ")" in line or "|" in line or ":" in line:
                                continue;
                            row = [val.strip() for val in line.split(',')]
                            
                            #timestamp = datetime.datetime.now() - start_time
                            #row.append(timestamp)
                            
                            # Scrive sul file e forza il salvataggio su disco (flush)
                            writer.writerow(row)
                            file.flush()
                            
                except KeyboardInterrupt:
                    print("\nRegistrazione interrotta dall'utente.")
                    break
                except Exception as e:
                    print(f"\nErrore di rete: {e}")
                    break

if __name__ == '__main__':
    main()