#include <Arduino.h>
#include <Adafruit_SPIFlash.h>
#include <SdFat.h>
#include <cmath>

// =========================== Config ===========================
// UART from control unit: Feather M4 default Serial1 pins
// RX = D0, TX = D1
static const uint32_t USB_BAUD = 115200;
static const uint32_t UART_BAUD = 115200;

// NTC parameters (same model used in your ESP32 sketch)
#define B 3435.0f
#define To 298.15f
#define Ro 10.0f
#define VREF 3.3f
#define ADC_RES 4095.0f

// Feather analog pins (adapt if wiring differs)
static const uint8_t PIN_NTC1 = A0;
static const uint8_t PIN_NTC2 = A1;
static const uint8_t PIN_NTC3 = A2;

// Flash FS paths
static const char* LOG_DIR = "/logs";

// Flush policies to reduce data loss on power-off but keep write overhead low
// Flush after every record and periodically every 2 seconds
static const uint16_t FLUSH_EVERY_N_RECORDS = 1;
static const uint32_t FLUSH_EVERY_MS = 2000;

// ======================= Flash objects ========================
Adafruit_FlashTransport_QSPI flashTransport;
Adafruit_SPIFlash flash(&flashTransport);
FatFileSystem fatfs;

bool flashReady = false;
bool loggingEnabled = false;
File activeLogFile;
String activeLogPath;

// ====================== Runtime buffers =======================
String rxBuffer;
String cmdBuffer;
uint16_t pendingRecordsSinceFlush = 0;
uint32_t lastFlushMs = 0;

float readNtcTempC(uint8_t pin) {
  const int adc = analogRead(pin);
  if (adc <= 0) {
    return -999.0f;
  }

  const float voltage = adc * (VREF / ADC_RES);
  const float rTherm = 10.0f * (VREF / voltage - 1.0f);
  const float tempK = 1.0f / ((log(rTherm / Ro) / B) + (1.0f / To));
  return tempK - 273.15f;
}

String trimCopy(String s) {
  s.trim();
  return s;
}

String makeNextLogPath() {
  for (int i = 0; i < 1000; i++) {
    char name[32];
    snprintf(name, sizeof(name), "%s/log_%03d.csv", LOG_DIR, i);
    if (!fatfs.exists(name)) {
      return String(name);
    }
  }
  return String("/logs/log_overflow.csv");
}

bool openNewLogFile(const String& requestedPath = "") {
  if (!flashReady) {
    return false;
  }

  if (activeLogFile) {
    activeLogFile.flush();
    activeLogFile.close();
  }

  activeLogPath = requestedPath.length() > 0 ? requestedPath : makeNextLogPath();
  activeLogFile = fatfs.open(activeLogPath.c_str(), FILE_WRITE);

  if (!activeLogFile) {
    Serial.println("ERROR: cannot open log file");
    return false;
  }

  if (activeLogFile.size() == 0) {
    //CSV HEADER
    //writer.writerow(['TempMotor','TempInverter','TempIGBT','Voltage','Speed','Id','Iq','TorqueMotor','PedalPerc','NTC1', 'NTC2', 'NTC3',"Time_s"])

    activeLogFile.println("Time_s,TempMotor,TempInverter,TempIGBT,Voltage,Speed,Iq,Id,TorqueMotor,PedalPerc,NTC1,NTC2,NTC3");
    activeLogFile.flush();
  }

  pendingRecordsSinceFlush = 0;
  lastFlushMs = millis(); 

  Serial.print("LOG_OPEN:");
  Serial.println(activeLogPath);
  return true;
}

void flushLogIfNeeded(bool force = false) {
  if (!activeLogFile) {
    return;
  }

  const uint32_t now = millis();
  if (force || pendingRecordsSinceFlush >= FLUSH_EVERY_N_RECORDS || (now - lastFlushMs) >= FLUSH_EVERY_MS) {
    activeLogFile.flush();
    pendingRecordsSinceFlush = 0;
    lastFlushMs = now;
  }
}

void stopLogging() {
  loggingEnabled = false;
  if (activeLogFile) {
    activeLogFile.flush();
    activeLogFile.close();
  }
}

void appendRecordToFlash(const String& payload) {
  if (!flashReady || !activeLogFile) {
    return;
  }

  const float t1 = readNtcTempC(PIN_NTC1);
  const float t2 = readNtcTempC(PIN_NTC2);
  const float t3 = readNtcTempC(PIN_NTC3);

  String line = payload + "," + String(t1, 2) + "," + String(t2, 2) + "," + String(t3, 2);
  activeLogFile.println(line);
  pendingRecordsSinceFlush++;
  flushLogIfNeeded(false);

  Serial.println(line);
}

void listLogs() {
  if (!flashReady) {
    Serial.println("ERROR: flash not ready");
    return;
  }

  if (!fatfs.exists(LOG_DIR)) {
    Serial.println("ERROR: /logs missing");
    return;
  }

  File dir = fatfs.open(LOG_DIR);
  if (!dir || !dir.isDirectory()) {
    Serial.println("ERROR: cannot open /logs");
    return;
  }

  Serial.println("FILES_START");
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) {
      break;
    }

    if (!entry.isDirectory()) {
      char name[64];
      entry.getName(name, sizeof(name));
      Serial.print(name);
      Serial.print(",");
      Serial.println(entry.size());
    }
    entry.close();
  }
  dir.close();
  Serial.println("FILES_END");
}

void readLogFile(const String& name) {
  if (!flashReady) {
    Serial.println("ERROR: flash not ready");
    return;
  }

  String path = String(LOG_DIR) + "/" + name;
  if (!fatfs.exists(path.c_str())) {
    Serial.println("ERROR: file not found");
    return;
  }

  File f = fatfs.open(path.c_str(), FILE_READ);
  if (!f) {
    Serial.println("ERROR: cannot open file");
    return;
  }

  Serial.println("DATA_START");
  while (f.available()) {
    Serial.write(f.read());
  }
  f.close();
  Serial.println();
  Serial.println("DATA_END");
}

void deleteLogFile(const String& name) {
  if (!flashReady) {
    Serial.println("ERROR: flash not ready");
    return;
  }

  String path = String(LOG_DIR) + "/" + name;
  if (!fatfs.exists(path.c_str())) {
    Serial.println("ERROR: file not found");
    return;
  }

  if (path == activeLogPath) {
    Serial.println("ERROR: cannot delete active log");
    return;
  }

  if (fatfs.remove(path.c_str())) {
    Serial.println("OK: file deleted");
  } else {
    Serial.println("ERROR: delete failed");
  }
}

void formatLogsFolder() {
  if (!flashReady) {
    Serial.println("ERROR: flash not ready");
    return;
  }

  if (!fatfs.exists(LOG_DIR)) {
    fatfs.mkdir(LOG_DIR);
    Serial.println("OK: /logs created");
    return;
  }

  File dir = fatfs.open(LOG_DIR);
  if (!dir || !dir.isDirectory()) {
    Serial.println("ERROR: cannot open /logs");
    return;
  }

  String toDelete[64];
  int count = 0;

  while (true) {
    File entry = dir.openNextFile();
    if (!entry) {
      break;
    }

    if (!entry.isDirectory() && count < 64) {
      char name[64];
      entry.getName(name, sizeof(name));
      toDelete[count++] = String(name);
    }
    entry.close();
  }
  dir.close();

  int deleted = 0;
  for (int i = 0; i < count; i++) {
    if (toDelete[i] == activeLogPath) {
      continue;
    }
    if (fatfs.remove(toDelete[i].c_str())) {
      deleted++;
    }
  }

  Serial.print("OK: deleted ");
  Serial.print(deleted);
  Serial.println(" files");
}

void printStatus() {
  Serial.print("FLASH:");
  Serial.println(flashReady ? "READY" : "NOT_READY");
  if (flashReady) {
    Serial.print("FLASH_SIZE:");
    Serial.println(flash.size());
  }
  Serial.print("ACTIVE_LOG:");
  Serial.println(activeLogPath);
}

void printHelp() {
  Serial.println("Commands:");
  Serial.println("HELP");
  Serial.println("STATUS");
  Serial.println("LIST");
  Serial.println("READ:<filename>");
  Serial.println("DELETE:<filename>");
  Serial.println("NEWLOG");
  Serial.println("NEWLOG:<filename>");
  Serial.println("FORMAT");
}

void handleCommand(const String& rawCommand) {
  String command = trimCopy(rawCommand);
  if (command.length() == 0) {
    return;
  }

  if (command == "HELP") {
    printHelp();
  } else if (command == "STATUS") {
    printStatus();
  } else if (command == "LIST") {
    listLogs();
  } else if (command.startsWith("READ:")) {
    readLogFile(trimCopy(command.substring(5)));
  } else if (command.startsWith("DELETE:")) {
    deleteLogFile(trimCopy(command.substring(7)));
  } else if (command == "NEWLOG") {
    if (openNewLogFile("")) {
      loggingEnabled = true;
    }
  } else if (command.startsWith("NEWLOG:")) {
    String fileName = trimCopy(command.substring(7));
    if (fileName.length() == 0) {
      Serial.println("ERROR: empty filename");
      return;
    }
    if (fileName.charAt(0) == '/') {
      if (openNewLogFile(fileName)) {
        loggingEnabled = true;
      }
    } else {
      if (openNewLogFile(String(LOG_DIR) + "/" + fileName)) {
        loggingEnabled = true;
      }
    }
  } else if (command == "FORMAT") {
    formatLogsFolder();
  } else {
    Serial.print("UNKNOWN:");
    Serial.println(command);
  }
}

void handleUsbSerialCommands() {
  while (Serial.available() > 0) {
    const char c = static_cast<char>(Serial.read());
    if (c == '\r') {
      continue;
    }

    if (c == '\n') {
      handleCommand(cmdBuffer);
      cmdBuffer = "";
    } else {
      cmdBuffer += c;
    }
  }
}

void handleIncomingUartData() {
  while (Serial1.available() > 0) {
    const char c = static_cast<char>(Serial1.read());

    if (c == '\r') {
      continue;
    }

    if (c == '\n') {
      String payload = trimCopy(rxBuffer);
      rxBuffer = "";

      if (payload.length() > 0) {
        if (payload == "[START]") {
          if (openNewLogFile("")) {
            loggingEnabled = true;
            Serial.println("OK: logging started");
          } else {
            loggingEnabled = false;
            Serial.println("ERROR: cannot start logging");
          }
        } else if (payload == "[STOP]") {
          stopLogging();
          Serial.println("OK: logging stopped");
        } else if (loggingEnabled) {
          // Accetta solo righe CSV (iniziano con un numero, es. "0.0,25.0,...")
          // Scarta righe diagnostiche come [STATE_TRANS], [ERROR], ecc.
          if (payload.length() > 0 && isDigit(static_cast<unsigned char>(payload.charAt(0)))) {
            appendRecordToFlash(payload);
          }
        }
      }
    } else {
      rxBuffer += c;
      if (rxBuffer.length() > 512) {
        rxBuffer.remove(0, rxBuffer.length() - 512);
      }
    }
  }
}

void setup() {
  Serial.begin(USB_BAUD);


  Serial.println("Feather M4 CAN - UART to Flash Logger");

  analogReadResolution(12);
  pinMode(PIN_NTC1, INPUT);
  pinMode(PIN_NTC2, INPUT);
  pinMode(PIN_NTC3, INPUT);

  Serial1.begin(UART_BAUD);
  Serial.println("Serial1 ready (D0/D1)");

  if (flash.begin()) {
    if (fatfs.begin(&flash)) {
      flashReady = true;
      if (!fatfs.exists(LOG_DIR)) {
        fatfs.mkdir(LOG_DIR);
      }
      Serial.println("Waiting for [START] on Serial1 to create and start log file");
    } else {
      Serial.println("ERROR: cannot mount flash filesystem");
    }
  } else {
    Serial.println("ERROR: flash init failed");
  }

  printHelp();
}

void loop() {
  handleIncomingUartData();
  handleUsbSerialCommands();
  flushLogIfNeeded(false);
}