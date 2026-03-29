/*
#include <Arduino.h>
#include <WiFi.h>

const char* ap_ssid = "ESP32_Bridge_WIFI";     
const char* ap_password = "12345678";         // Password (minimo 8 caratteri)

// Server TCP sulla porta 8080
WiFiServer server(8080);
WiFiClient client;

#define RXD2 16  
#define TXD2 17  

void setup() {
  Serial.begin(115200);
  Serial.println("Avvio ESP32 WiFi Bridge in modalità Access Point...");
  
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Serial2 inizializzata su RX=16, TX=17 a 115200 baud");
  
  // Crea Access Point WiFi
  Serial.println("Creazione Access Point WiFi...");
  WiFi.softAP(ap_ssid, ap_password);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.println("Access Point avviato!");
  Serial.print("SSID: ");
  Serial.println(ap_ssid);
  Serial.print("Password: ");
  Serial.println(ap_password);
  Serial.print("Indirizzo IP: ");
  Serial.println(IP);
  
  // Avvia il server TCP
  server.begin();
  Serial.println("Server TCP avviato sulla porta 8080");
  Serial.println("Connettiti alla rete WiFi e poi a: " + IP.toString() + ":8080");
}

void loop() {
  // Accetta nuove connessioni
  if (!client.connected()) {
    client = server.available();
    if (client) {
      Serial.println("Nuovo client connesso!");
      client.println("ESP32 WiFi Bridge AP - Connesso");
    }
  }
  
  // Leggi da Serial2 e invia al client WiFi
  if (Serial2.available() && client.connected()) {
    while (Serial2.available()) {
      char c = Serial2.read();
      client.write(c);     // Invia via WiFi al client
      Serial.write(c);     // Echo su seriale USB per debug
    }
  }
  
  // Leggi dal client WiFi e invia a Serial2 (comunicazione bidirezionale)
  if (client.available()) {
    while (client.available()) {
      char c = client.read();
      Serial2.write(c);    // Invia a Serial2
      Serial.write(c);     // Echo su seriale USB per debug
    }
  }
  
  delay(10);
}
*/

/* ==================== VERSIONE WiFi CLIENT (SI CONNETTE A RETE ESISTENTE) ====================
*/

#include <Arduino.h>
#include <WiFi.h>
#include "secrets.h"

#define B 3435
#define To 298.15f
#define Ro 10 //kilo
#define Vref 3.3f 
#define res 4095  //12 bit
#define pinNTC1 36  
#define pinNTC2 39
#define pinNTC3 34


float calcolaTempDaTensione(uint8_t pin){
  int valore_analogico = analogRead(pin); //0<x<res
  if (valore_analogico == 0) return -999.0f;  // ← protezione
  float tensione = valore_analogico*(Vref/res); //0<x<Vref
  float Res_Termoresistore = 10.0*(Vref/tensione - 1);
  float Temperatura = (1 / ( (log(Res_Termoresistore/Ro))/B + 1/To ) ) - 273.15;
  return Temperatura;
}

// Configurazione WiFi

const char* ssid     = "labpesanti";
const char* password = "s3VJWdwMew";

// Server TCP sulla porta 8080
WiFiServer server(8080);
WiFiClient client;

#define RXD2 16  
#define TXD2 17  

void setup() {
  Serial.begin(115200);
  Serial.println("Avvio ESP32 WiFi Bridge...");
  
  Serial2.setRxBufferSize(2048); 
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Serial2 inizializzata su RX=16, TX=17 a 115200 baud");
  
  // Connessione WiFi
  Serial.print("Connessione a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connesso!");
  Serial.print("Indirizzo IP: ");
  Serial.println(WiFi.localIP());
  
  // Avvia il server TCP
  server.begin();
  Serial.println("Server TCP avviato sulla porta 8080");
  Serial.println("Pronto per connessioni client...");
}


/*
void loop() {
  // Accetta nuove connessioni
  if (!client.connected()) {
    client = server.available();
    if (client) {
      
      Serial.println("Nuovo client connesso!");
      client.println("ESP32 WiFi Bridge - Connesso");
    }
  }
  
  if (Serial2.available() && client.connected()) {
    uint8_t buffer[2048];
    size_t len = Serial2.readBytes((char*)buffer, sizeof(buffer));
    
    if (len > 0) {
      client.write(buffer, len); // Invia via WiFi al client l'intero chunk
      Serial.write(buffer, len); // Echo opzionale su USB 
    }
  }

  if (client.available()) {
    while (client.available()) {
      char c = client.read();
      Serial2.write(c);    // Invia a Serial2
      Serial.write(c);     // Echo su seriale USB per debug
    }
  }
  
  delay(1);
}
*/

void loop() {
  if (!client.connected()) {
    client = server.available();
    if (client) {
      Serial.println("Nuovo client connesso!");
    }
    Serial2.println("Client connesso: " + String(client.connected()));
  }

  
  static String rx_buffer = ""; 
  
  static int i = 0;
  static int j = 0;
  
   //if (client.connected()) {
   //   String outData = ("0,"+String(j+=2) + "," + String(i++) + "\n");
   //       client.print(outData);
   // }
    
  
  while (Serial2.available() > 0) {
    char c = Serial2.read();
    
    if (c == '\n') { 
      rx_buffer.trim();
      
      if (rx_buffer.length() > 0) {
        float t1 = calcolaTempDaTensione(pinNTC1);
        float t2 = calcolaTempDaTensione(pinNTC2);
        float t3 = calcolaTempDaTensione(pinNTC3);
        
        String outData = rx_buffer + ", " + String(t1, 2) + ", " + String(t2, 2) + ", " + String(t3, 2) + "\n\r";
        String outData2 = rx_buffer;
        if (client.connected()) {
          Serial.println("Invio dati al client: " + outData);  //debug
          client.println(outData);
          //client.println(outData2);
        }
        
        Serial.print(outData);  //debug
        
        rx_buffer = "";
      }
    } else {
      rx_buffer += c;
    }
  }

  
  if (client.connected() && client.available()) {
    while (client.available()) {
      char c = client.read();
      Serial2.write(c);   
      Serial.write(c);    
    }
  }
  //Serial.println("Temp1: " + String(calcolaTempDaTensione(pinNTC1), 2) + " °C, Temp2: " + String(calcolaTempDaTensione(pinNTC2), 2) + " °C, Temp3: " + String(calcolaTempDaTensione(pinNTC3), 2) + " °C");
  
  delay(10);
}








/*
/* ==================== VERSIONE WiFi CLIENT (SI CONNETTE A RETE ESISTENTE) - FIX (nonFix) CLAUDE====================
*/
/*
#include <Arduino.h>
#include <WiFi.h>

// Configurazione WiFi
const char* ssid     = "labpesanti";
const char* password = "s3VJWdwMew";

// Server TCP sulla porta 8080
WiFiServer server(8080);
WiFiClient client;

#define RXD2 16  
#define TXD2 17

unsigned long lastHeartbeat = 0;
unsigned long lastTestTx = 0;

// Buffer circolare per dati in arrivo
#define CIRCULAR_BUFFER_SIZE 2048
char circularBuffer[CIRCULAR_BUFFER_SIZE];
int bufferHead = 0;
int bufferTail = 0;
int bufferCount = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Avvio ESP32 WiFi Bridge...");
  
  // Aumenta il buffer di Serial2 (default è 256 bytes)
  Serial2.setRxBufferSize(1024);  // 1KB buffer per evitare overflow
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Serial2 inizializzata su RX=16, TX=17 a 115200 baud");
  Serial.println("Buffer RX Serial2 impostato a 1024 bytes");
  
  // Connessione WiFi
  Serial.print("Connessione a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connesso!");
  Serial.print("Indirizzo IP: ");
  Serial.println(WiFi.localIP());
  
  // Avvia il server TCP
  server.begin();
  Serial.println("Server TCP avviato sulla porta 8080");
  Serial.println("Pronto per connessioni client...");
  Serial.println("\n=== MODALITA' DEBUG ATTIVA ===");
  Serial.println("- Heartbeat ogni 5 secondi");
  Serial.println("- Test TX su Serial2 ogni 3 secondi");
  Serial.println("==============================\n");
  
  // Test hardware Serial2
  Serial.println("[HW TEST] Test lettura GPIO16 (RX2)...");
  pinMode(RXD2, INPUT_PULLUP);
  delay(100);
  int rxState = digitalRead(RXD2);
  Serial.print("[HW TEST] Stato GPIO16 (RX2): ");
  Serial.println(rxState == HIGH ? "HIGH (idle - OK)" : "LOW (possibile corto o non connesso)");
  
  // Reinizializza Serial2 dopo il test
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  Serial.println("[HW TEST] Serial2 re-inizializzata");
  Serial.println("\n*** PER TEST LOOPBACK: Collega GPIO17 con GPIO16 ***");
  Serial.println("*** Dovresti vedere i PING in [RX SERIAL2] ***\n");
  Serial.println("==============================\n");
}

void loop() {
  unsigned long now = millis();
  
  // Heartbeat ogni 5 secondi
  if (now - lastHeartbeat > 5000) {
    lastHeartbeat = now;
    Serial.print("[HEARTBEAT] Loop attivo - Client: ");
    Serial.print(client.connected() ? "CONNESSO" : "NON CONNESSO");
    Serial.print(" - Buffer: ");
    Serial.print(bufferCount);
    Serial.println(" bytes");
  }
  
  // Test TX su Serial2 ogni 3 secondi (per verificare se MCU riceve)
  if (now - lastTestTx > 3000) {
    lastTestTx = now;
    Serial.println("[TEST] Invio 'PING' su Serial2...");
    Serial2.println("PING da ESP32");
  }
  
  // Accetta nuove connessioni
  if (!client.connected()) {
    client = server.available();
    if (client) {
      
      Serial.println("=============================");
      Serial.println(">>> NUOVO CLIENT CONNESSO <<<");
      Serial.println("=============================");
      client.println("ESP32 WiFi Bridge - Connesso");
      client.println("Indirizzo IP ESP32: " + WiFi.localIP().toString());
      
      // Invia il buffer accumulato al nuovo client
      if (bufferCount > 0) {
        Serial.print("[BUFFER] Invio ");
        Serial.print(bufferCount);
        Serial.println(" bytes accumulati al client...");
        
        int sent = 0;
        while (bufferCount > 0) {
          client.write(circularBuffer[bufferTail]);
          bufferTail = (bufferTail + 1) % CIRCULAR_BUFFER_SIZE;
          bufferCount--;
          sent++;
        }
        client.flush();
        Serial.print("[BUFFER] Inviati ");
        Serial.print(sent);
        Serial.println(" bytes");
      }
    }
  }
  
  // Leggi da Serial2
  int available = Serial2.available();
  if (available > 0) {
    int bytesRead = 0;
    bool clientWasConnected = client.connected();
    
    // Log se il buffer è quasi pieno (possibile overflow)
    if (available > 800) {
      Serial.print("[WARN] Buffer Serial2 quasi pieno: ");
      Serial.print(available);
      Serial.println(" bytes!");
    }
    
    while (Serial2.available()) {
      char c = Serial2.read();
      Serial.write(c);  // Echo su seriale USB per debug
      bytesRead++;
      
      // Se client connesso, invia direttamente
      if (client.connected()) {
        client.write(c);
      } else {
        // Altrimenti memorizza nel buffer circolare
        if (bufferCount < CIRCULAR_BUFFER_SIZE) {
          circularBuffer[bufferHead] = c;
          bufferHead = (bufferHead + 1) % CIRCULAR_BUFFER_SIZE;
          bufferCount++;
        } else {
          // Buffer pieno, sovrascrivi i dati più vecchi
          circularBuffer[bufferHead] = c;
          bufferHead = (bufferHead + 1) % CIRCULAR_BUFFER_SIZE;
          bufferTail = (bufferTail + 1) % CIRCULAR_BUFFER_SIZE;
        }
      }
    }
    
    // Log dettagliato
    Serial.print("\n[DEBUG] Letti ");
    Serial.print(bytesRead);
    Serial.print(" bytes - Client connesso: ");
    Serial.print(clientWasConnected ? "SI" : "NO");
    Serial.print(" - Inviati al client: ");
    Serial.println(clientWasConnected ? "SI" : "NO (salvati in buffer)");
    
    // Flush solo DOPO aver letto tutti i dati - NON bloccante
    if (client.connected() && bytesRead > 0) {
      // Non usare flush() che è bloccante, i dati vengono inviati automaticamente
      Serial.println("[DEBUG] Dati inviati (senza flush bloccante)");
    }
  }
  
  // Verifica periodica dello stato del client
  static unsigned long lastClientCheck = 0;
  if (client && now - lastClientCheck > 1000) {
    lastClientCheck = now;
    if (!client.connected()) {
      Serial.println("[WARN] Client disconnesso - pulizia...");
      client.stop();
    }
  }
  
  // Leggi dal client WiFi e invia a Serial2 (comunicazione bidirezionale)
  if (client.available()) {
    Serial.print("[RX WiFi] Dati dal client: ");
    while (client.available()) {
      char c = client.read();
      Serial2.write(c);    // Invia a Serial2
      Serial.write(c);     // Echo su seriale USB per debug
    }
    Serial.println(" [INVIATO A SERIAL2]");
  }
  
  delay(10);
}





/* ==================== VERSIONE BLUETOOTH  ====================
*
#include <Arduino.h>
#include <BluetoothSerial.h>

#define B 3435
#define To 298.15f
#define Ro 10 //kilo
#define Vref 3.3f 
#define res 4095  //12 bit
#define pinNTC1 36  
#define pinNTC2 39
#define pinNTC3 34


float calcolaTempDaTensione(uint8_t pin){
  int valore_analogico = analogRead(pin); //0<x<res
  if (valore_analogico == 0) return -999.0f;  // ← protezione
  float tensione = valore_analogico*(Vref/res); //0<x<Vref
  float Res_Termoresistore = 10.0*(Vref/tensione - 1);
  float Temperatura = (1 / ( (log(Res_Termoresistore/Ro))/B + 1/To ) ) - 273.15;
  return Temperatura;
}
// Controllo se Bluetooth è abilitato
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// Oggetto Bluetooth Serial
BluetoothSerial SerialBT;

// Pin per Serial2 (seriale hardware secondaria)
#define RXD2 16  // Pin RX per Serial2
#define TXD2 17  // Pin TX per Serial2

// Nome del dispositivo Bluetooth
const char* bluetoothName = "ESP32_BT";

void setup() {
  // Inizializza seriale principale (USB) per debug
  Serial.begin(115200);
  delay(1000);  // Aspetta che la seriale USB si stabilizzi
  Serial.println("\n\n==== AVVIO ESP32 BLUETOOTH BRIDGE ====");
  Serial.print("Build flags: CONFIG_BT_ENABLED = ");
  Serial.println(CONFIG_BT_ENABLED);
  
  // Inizializza Serial2 (seriale hardware secondaria)
  // Parametri: baud rate, config, RX pin, TX pin
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  Serial.println("[OK] Serial2 inizializzata su RX=16, TX=17 a 115200 baud");
  
  // Inizializza Bluetooth Serial
  Serial.print("[BT] Inizializzazione Bluetooth con nome: '");
  Serial.print(bluetoothName);
  Serial.println("'...");
  
  if(!SerialBT.begin(bluetoothName)) {
    Serial.println("[ERRORE] Bluetooth non si è inizializzato!");
    Serial.println("[DEBUG] Verifica:");
    Serial.println("  - CONFIG_BT_ENABLED e CONFIG_BLUEDROID_ENABLED abilitati in platformio.ini");
    Serial.println("  - Bluetooth hardware presente sul chip");
    while(1); // Blocca se Bluetooth non si avvia
  }
  
  Serial.println("[OK] Bluetooth inizializzato correttamente!");
  Serial.print("[BT] Nome dispositivo: '");
  Serial.print(bluetoothName);
  Serial.println("'");
  Serial.println("[BT] Visibilità: ACCESA (discoverable)");
  Serial.println("[BT] Modalità: Classic Bluetooth SPP (Serial Port Profile)");
  Serial.println("[INFO] Pronto per la connessione Bluetooth...");
  Serial.println("[INFO] Cerca 'ESP32_BT' nella lista dei dispositivi Bluetooth del tuo PC/Telefono");
  Serial.println("======================================\n");
}

void loop() {
  // Leggi da Serial2 e invia a Bluetooth
  if (Serial2.available()) {
    // Leggi tutti i byte disponibili da Serial2
    while (Serial2.available()) {
      char c = Serial2.read();
      SerialBT.write(c);  // Invia via Bluetooth
      Serial.write(c);     // Echo su seriale USB per debug
    }
  }
  
  // Leggi da Bluetooth e invia a Serial2 (comunicazione bidirezionale)
  if (SerialBT.available()) {
    while (SerialBT.available()) {
      char c = SerialBT.read();
      Serial2.write(c);    // Invia a Serial2
      Serial.write(c);     // Echo su seriale USB per debug
    }
  }
  
  delay(10);
}

*/