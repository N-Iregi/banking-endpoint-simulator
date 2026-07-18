#include <WiFi.h>
#include <WiFiClientSecure.h> //same as WiFi but adds TLS/HTTPS support, this is the key one for security
#include <Wire.h> //  I2C protocol (needed for OLED)
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <SD.h>
#include "../secrets.h"

// OLED setup
#define SDA_PIN 8
#define SCL_PIN 9
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 // OLED doesn't have a dedicated reset pin wired up, so the library handles it in software.
#define SCREEN_ADDRESS 0x3C // I2C address
#define CS_PIN 10

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// TLS certificate — paste your cert.pem contents here
const char* serverCert = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDIjCCAgqgAwIBAgIUOcfs1/Z0yGd5P6UaEYIfQnUznqEwDQYJKoZIhvcNAQEL\n" \
"BQAwGDEWMBQGA1UEAwwNMTkyLjE2OC4xMDAuNTAeFw0yNjA3MTYxODAxNTdaFw0y\n" \
"NzA3MTYxODAxNTdaMBgxFjAUBgNVBAMMDTE5Mi4xNjguMTAwLjUwggEiMA0GCSqG\n" \
"SIb3DQEBAQUAA4IBDwAwggEKAoIBAQC9zKh0vmitl9CUpBKgq8srmWKldxgToipv\n" \
"5hlxS+bjT79xR0wu79tW7LpFNDNwmF+w55PtiaQvFqwhPfWUsbxm14EYEXB9mQYx\n" \
"SEnE7n0BBoxy2d4rW/Ghke3DPttBUrVmM/oLKRjYGoYjYhCpBaFKM+Ca1bGowuoj\n" \
"sT5hL/PSuPMLkYFNuByyBA8qbepx5X9Kp8HuN9mJkdYbpeDLxL+bNOkWVW9NoWtX\n" \
"PicTeo8OBhUd6WPKN2VORjeG2G8YbDqQUHRdeR53qyhCyCugCWLO6kZDaqWuZWMe\n" \
"RrV8dYLXtIZ8tVIUkO2jO4m54ANgfXUAAgEB7/HinTKXsptRv46zAgMBAAGjZDBi\n" \
"MB0GA1UdDgQWBBQ2AgR6r1/sIUQc4+Lzhk7T60KmXjAfBgNVHSMEGDAWgBQ2AgR6\n" \
"r1/sIUQc4+Lzhk7T60KmXjAPBgNVHRMBAf8EBTADAQH/MA8GA1UdEQQIMAaHBMCo\n" \
"ZAUwDQYJKoZIhvcNAQELBQADggEBABwnTgXaAas6WkFfFFkL+RldA48B0L4RdL3D\n" \
"QUKFyZqiY5SP02QCZKco7wagS6yHOd2Ag9pq8ocPxTXP2kwnSu/XTypCqrOfJ8My\n" \
"rz6mjvolt4l5cfT/faSGdvZGdzLGm9tOPtxg9av7JM/bCxDWQTneJjin1d1p3ukR\n" \
"b8UGcE/sscwjlho8iWhO8oxTJj91EljYYWNlCuiQzftVV5kBWJ/b49Rt6qMkQMg1\n" \
"6fPoH63jCJ2N+38Pk36qdYjDaEsW69piuTH8AvOm8MaAPd7kFyqbS9uXm0y5eYGU\n" \
"cpn0UuICqDNGskByd7h1fPx9xCfZ0OJvNvG994EMJsrQnLZDx/g=\n" \
"-----END CERTIFICATE-----\n";

// Transaction counter
int txnCount = 0;

// UpdateDisplay function
//clears the screen and prints three lines at fixed vertical positions (0, 20, 40 pixels from top)
void updateDisplay(String line1, String line2, String line3) {
  display.clearDisplay();

  display.fillRect(0, 0, 128, 16, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setTextSize(1);
  display.setCursor(2, 4);
  display.println(line1);

  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 20);
  display.println(line2);
  display.setCursor(0, 40);
  display.println(line3);
  
  display.display();
}
// If it connects, updates the OLED.
// If it fails, shows an error.
void connectWiFi() {
  Serial.print("Connecting to WiFi");
  updateDisplay("Connecting...", WIFI_SSID, "Please wait");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
    updateDisplay("WiFi Connected!", WiFi.localIP().toString(), "Endpoint Ready");
  }
  else
  {
    Serial.println("\nWiFi failed");
    updateDisplay("WiFi FAILED", "Check credentials", "Retrying...");
  }
}

// Builds a JSON string manually. Each line adds one field:
  // transaction_id — unique ID using the counter (TXN1, TXN2...)
  // amount — random number between 100 and 50000 (simulating a transaction amount in KES)
  // merchant — hardcoded ATM location identifier
  // currency — KES (Kenyan Shilling)
  // device_id — uses the ESP32-S3's actual MAC address, which is unique per device, just like a real ATM has a unique terminal ID
String buildPayload() {
  txnCount++;
  String payload = "{";
  payload += "\"transaction_id\":\"TXN" + String(txnCount) + "\",";
  payload += "\"amount\":" + String(random(100, 50000)) + ",";
  payload += "\"merchant\":\"ATM-NAIROBI-001\",";
  payload += "\"currency\":\"KES\",";
  payload += "\"device_id\":\"ESP32-" + WiFi.macAddress() + "\"";
  payload += "}";
  return payload;
}

void logToSD(String payload, String status) {
  File logFile = SD.open("/txn_log.txt", FILE_APPEND);
  if (logFile) {
    logFile.println(status + " | " + payload);
    logFile.close();
    Serial.println("Logged to SD card");
  } else {
    Serial.println("SD write failed");
  }
}

String sendTransaction(String payload)
{
  WiFiClientSecure client;
  client.setCACert(serverCert);

  Serial.println("Connecting to server...");

  if (!client.connect(SERVER_HOST, SERVER_PORT))
  {
    Serial.println("Connection failed");
    return "CONN_FAIL";
  }

  // Build HTTP POST request manually
  String request = "POST " + String(SERVER_PATH) + " HTTP/1.1\r\n";
  request += "Host: " + String(SERVER_HOST) + "\r\n";
  request += "Content-Type: application/json\r\n";
  request += "Content-Length: " + String(payload.length()) + "\r\n";
  request += "Connection: close\r\n\r\n";
  request += payload;

  client.print(request);

  // Wait for response
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println("Timeout");
      client.stop();
      return "TIMEOUT";
    }
  }

  // skip the headers and extract only the body.
  String response = "";
  bool bodyStarted = false;
  while (client.available())
  {
    String line = client.readStringUntil('\n');
    // The blank line between headers and body contains only \r (carriage return
    if (line == "\r")
      bodyStarted = true;
    if (bodyStarted)
      response += line;
  }

  client.stop();
  response.trim();
  return response;
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.display();

  connectWiFi();

  // SD card initialization
  SPI.begin(12, 13, 11, CS_PIN);
  if (!SD.begin(CS_PIN))
  {
    Serial.println("SD card failed!");
    updateDisplay("SD card", "FAILED", "Check wiring");
    delay(2000);
  }
  else
  {
    Serial.println("SD Card OK");
  }

  delay(2000);
}

void loop() {
  // Auto-reconnect watchdog
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi dropped — reconnecting");
    updateDisplay("WiFi dropped!", "Reconnecting...", "");
    connectWiFi();
    return;
  }

  // Build and send transaction
  String payload = buildPayload();
  Serial.println("Sending: " + payload);
  updateDisplay("Sending TXN...", "ID: TXN" + String(txnCount), "");

  String response = sendTransaction(payload);
  Serial.println("Response: " + response);

  // Parse response status
  String statusMsg = response.indexOf("OK") >= 0 ? "OK" : "FAILED";

  logToSD(payload, statusMsg);

  // Update OLED with result
  updateDisplay(
    "TXN" + String(txnCount) + " " + statusMsg,
    "Amount: KES " + String(random(100, 50000)),
    "Nairobi-001"
  );

  Serial.println("Waiting 10 seconds...");
  delay(10000); // send a transaction every 10 seconds
}
