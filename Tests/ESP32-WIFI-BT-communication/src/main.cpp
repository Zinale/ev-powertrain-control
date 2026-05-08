#include <Arduino.h>
// ---------------------------------------------------------------------------
// Seleziona la modalita di comunicazione cambiando SOLO questo define.
//   COMM_MODE_WIFI      -> si connette alla rete WiFi e apre un server TCP
//   COMM_MODE_BLUETOOTH -> usa Bluetooth SPP
// ---------------------------------------------------------------------------
#define COMM_MODE_WIFI      1
#define COMM_MODE_BLUETOOTH 2
#define COMM_MODE           COMM_MODE_WIFI   // <-- cambia qui

#define DEBUG_HEARTBEAT

#define SERIAL2_BAUD 9600

#if COMM_MODE == COMM_MODE_WIFI

#include <WiFi.h>
#include "secrets.h"
#elif COMM_MODE == COMM_MODE_BLUETOOTH
#include <BluetoothSerial.h>
#else
#error COMM_MODE non valido. Usa COMM_MODE_WIFI o COMM_MODE_BLUETOOTH.
#endif

#define B 3435
#define To 298.15f
#define Ro 10.0f
#define Vref 3.3f
#define res 4095.0f

#define pinNTC1 36
#define pinNTC2 39
#define pinNTC3 34

#define RXD2 16
#define TXD2 17

const uint32_t serial2Baud = SERIAL2_BAUD;
const uint16_t tcpPort = 8080;

const bool filterTelnetNegotiation = true;
const bool forwardClientToSerial2 = false;
const size_t maxRxLineLen = 256;

#if COMM_MODE == COMM_MODE_WIFI
#ifdef WIFI_SSID
//const char* wifiSsid = WIFI_SSID;
#else
//const char* wifiSsid = "Gigante-Fondatore";
const char* wifiSsid = "labpesanti";
#endif

#ifdef WIFI_PASSWORD
//const char* wifiPassword = WIFI_PASSWORD;
#else
//const char* wifiPassword = "grisha-jaeger";
const char* wifiPassword = "s3VJWdwMew";
#endif

WiFiServer server(tcpPort);
WiFiClient client;
#elif COMM_MODE == COMM_MODE_BLUETOOTH
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth non abilitato. Abilita CONFIG_BT_ENABLED e CONFIG_BLUEDROID_ENABLED.
#endif

BluetoothSerial SerialBT;
const char* bluetoothName = "ESP32_BT";
#endif

float calcolaTempDaTensione(uint8_t pin) {
  int valore_analogico = analogRead(pin);
  if (valore_analogico == 0) {
    return -999.0f;
  }

  float tensione = valore_analogico * (Vref / res);
  float resTermoresistore = 10.0f * (Vref / tensione - 1.0f);
  float temperatura = (1.0f / ((log(resTermoresistore / Ro) / B) + (1.0f / To))) - 273.15f;
  return temperatura;
}

void setupTransport() {
#if COMM_MODE == COMM_MODE_WIFI
  Serial.print("Connessione WiFi a ");
  Serial.println(wifiSsid);
  WiFi.begin(wifiSsid, wifiPassword);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connesso");
  Serial.print("Indirizzo IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC WiFi: ");
  Serial.println(WiFi.macAddress());

  server.begin();
  Serial.print("Server TCP avviato sulla porta ");
  Serial.println(tcpPort);
  Serial.println("[INIT OK][WIFI] Bridge pronto");
#elif COMM_MODE == COMM_MODE_BLUETOOTH
  Serial.print("[BT] Inizializzazione Bluetooth con nome: '");
  Serial.print(bluetoothName);
  Serial.println("'");

  if (!SerialBT.begin(bluetoothName)) {
    Serial.println("[ERRORE] Inizializzazione Bluetooth fallita");
    while (1) {
      delay(500);
    }
  }

  Serial.println("[BT] Bluetooth inizializzato");
  Serial.print("[INIT OK][BT] Nome dispositivo: ");
  Serial.println(bluetoothName);
#endif
}

void aggiornaClient() {
#if COMM_MODE == COMM_MODE_WIFI
  if (!client.connected()) {
    WiFiClient nuovoClient = server.available();
    if (nuovoClient) {
      client = nuovoClient;
      client.println("ESP32 Bridge connesso");
    }
  }
#endif
}

bool clientConnesso() {
#if COMM_MODE == COMM_MODE_WIFI
  return client.connected();
#elif COMM_MODE == COMM_MODE_BLUETOOTH
  return SerialBT.hasClient();
#endif
}

void inviaAlClient(const String& payload) {
#if COMM_MODE == COMM_MODE_WIFI
  client.print(payload);
#elif COMM_MODE == COMM_MODE_BLUETOOTH
  SerialBT.print(payload);
#endif
}

bool datiDaClientDisponibili() {
#if COMM_MODE == COMM_MODE_WIFI
  return client.connected() && client.available() > 0;
#elif COMM_MODE == COMM_MODE_BLUETOOTH
  return SerialBT.available() > 0;
#endif
}

char leggiDaClient() {
#if COMM_MODE == COMM_MODE_WIFI
  return static_cast<char>(client.read());
#elif COMM_MODE == COMM_MODE_BLUETOOTH
  return static_cast<char>(SerialBT.read());
#endif
}

bool estraiByteClientValido(char inputByte, char& outputByte) {
#if COMM_MODE == COMM_MODE_WIFI
  enum TelnetState {
    TELNET_NORMAL,
    TELNET_AFTER_IAC,
    TELNET_SKIP_OPTION,
    TELNET_SUBNEG,
    TELNET_SUBNEG_AFTER_IAC
  };

  static TelnetState statoTelnet = TELNET_NORMAL;
  const uint8_t b = static_cast<uint8_t>(inputByte);

  if (!filterTelnetNegotiation) {
    outputByte = inputByte;
    return true;
  }

  switch (statoTelnet) {
    case TELNET_NORMAL:
      if (b == 255) { // IAC
        statoTelnet = TELNET_AFTER_IAC;
        return false;
      }
      outputByte = inputByte;
      return true;

    case TELNET_AFTER_IAC:
      if (b == 255) { // IAC IAC -> byte dati 255
        statoTelnet = TELNET_NORMAL;
        outputByte = inputByte;
        return true;
      }
      if (b == 250) { // SB
        statoTelnet = TELNET_SUBNEG;
        return false;
      }
      if (b == 251 || b == 252 || b == 253 || b == 254) { // WILL/WONT/DO/DONT
        statoTelnet = TELNET_SKIP_OPTION;
        return false;
      }
      statoTelnet = TELNET_NORMAL;
      return false;

    case TELNET_SKIP_OPTION:
      statoTelnet = TELNET_NORMAL;
      return false;

    case TELNET_SUBNEG:
      if (b == 255) {
        statoTelnet = TELNET_SUBNEG_AFTER_IAC;
      }
      return false;

    case TELNET_SUBNEG_AFTER_IAC:
      if (b == 240) { // SE
        statoTelnet = TELNET_NORMAL;
      } else {
        statoTelnet = TELNET_SUBNEG;
      }
      return false;
  }
#endif

#if COMM_MODE == COMM_MODE_BLUETOOTH
  outputByte = inputByte;
  return true;
#endif

  outputByte = inputByte;
  return true;
}

bool isPayloadChar(char c) {
  if (isAlphaNumeric(static_cast<unsigned char>(c))) {
    return true;
  }

  switch (c) {
    case ' ':
    case '[':
    case ']':
    case '(':
    case ')':
    case ':':
    case '|':
    case '%':
    case '=':
    case ',':
    case '.':
    case '_':
    case '-':
    case '+':
    case '/':
    case '>':
    case '<':
    case '!':
    case '*':
    case '#':
      return true;
    default:
      return false;
  }
}

bool isPayloadStartChar(char c) {
  return c == '[' || isAlphaNumeric(static_cast<unsigned char>(c)) || c == '-' || c == '+';
}


String sanitizeLine(const String& line) {
  return (line.length() > 0) ? line : String();
}

void setup() {
  Serial.begin(115200);
  Serial.println("Avvio ESP32 Bridge");

  Serial2.setRxBufferSize(2048);
  Serial2.begin(serial2Baud, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Serial2 inizializzata su RX=16, TX=17");

  setupTransport();
}

void loop() {
  aggiornaClient();

  bool connesso = clientConnesso();
  static bool statoPrecedente = false;
  if (connesso != statoPrecedente) {
    statoPrecedente = connesso;
#if COMM_MODE == COMM_MODE_WIFI
    Serial.println(connesso ? "[WiFi] Client connesso" : "[WiFi] Client disconnesso");
#elif COMM_MODE == COMM_MODE_BLUETOOTH
    Serial.println(connesso ? "[BT] Client connesso" : "[BT] Client disconnesso");
#endif
  }

  #ifdef DEBUG_HEARTBEAT
    static uint32_t lastHeartbeatMs = 0;
    const uint32_t heartbeatIntervalMs = 5000;
    uint32_t nowMs = millis();
    if (connesso && (nowMs - lastHeartbeatMs >= heartbeatIntervalMs)) {
      lastHeartbeatMs = nowMs;
  #if COMM_MODE == COMM_MODE_WIFI
      Serial.printf("[HB][WiFi] Client connesso | t=%lu ms\r\n", nowMs);
      inviaAlClient(String("[HB][WiFi] Client connesso | t=") + nowMs + " ms\r\n");
  #elif COMM_MODE == COMM_MODE_BLUETOOTH
      Serial.printf("[HB][BT] Client connesso | t=%lu ms\r\n", nowMs);
      inviaAlClient(String("[HB][BT] Client connesso | t=") + nowMs + " ms\r\n");
  #endif
    }
  #endif

  static String rxBuffer = "";
  while (Serial2.available() > 0) {
    char c = static_cast<char>(Serial2.read());

    if (c == '\r' || c == '\n') {
      rxBuffer.trim();
      if (rxBuffer.length() > 0) {
        const String validLine = sanitizeLine(rxBuffer);
        if (validLine.length() > 0) {
          float t1 = calcolaTempDaTensione(pinNTC1);
          float t2 = calcolaTempDaTensione(pinNTC2);
          float t3 = calcolaTempDaTensione(pinNTC3);
          String outData;
          if (isDigit(validLine.charAt(0))) {
              outData = validLine + ", " + String(t1,2) + ", " + String(t2,2) + ", " + String(t3,2) + "\r\n";
          } else {
              outData = validLine + "\r\n";  
          }
          if (connesso) inviaAlClient(outData);
          Serial.print(outData);
        }
        rxBuffer = "";
      }
      continue;
    }

    if (!isPayloadChar(c)) continue;
    if (rxBuffer.length() == 0 && !isPayloadStartChar(c)) continue;
    if (rxBuffer.length() >= maxRxLineLen) { rxBuffer = ""; continue; }
    rxBuffer += c;
  }

  if (forwardClientToSerial2 && connesso && datiDaClientDisponibili()) {
    while (datiDaClientDisponibili()) {
      char c = leggiDaClient();
      char byteValido = 0;
      if (!estraiByteClientValido(c, byteValido)) {
        continue;
      }

      Serial2.write(byteValido);

      if (isPrintable(static_cast<unsigned char>(byteValido)) || byteValido == '\r' || byteValido == '\n' || byteValido == '\t') {
        Serial.write(byteValido);
      }
    }
  }

  delay(10);
}
