# Secure Banking Endpoint Simulator

A hardware security project simulating a secure ATM/POS logging node using an ESP32-S3 microcontroller. Demonstrates TLS-encrypted communication, certificate verification, and dual-layer audit logging — concepts directly applicable to banking infrastructure security.

## Security Concepts Demonstrated

- **TLS/HTTPS encryption** — all transaction data transmitted over encrypted channels using `WiFiClientSecure` with certificate pinning
- **Certificate verification** — ESP32-S3 validates the server's certificate before transmitting any data, preventing man-in-the-middle attacks
- **Dual audit trail** — every transaction logged independently to both the server (Node.js) and the device (SD card), ensuring tamper-evident records even during network outages
- **Graceful failure handling** — device maintains local audit log during connectivity loss and auto-reconnects when network is restored
- **Credential separation** — Wi-Fi credentials and server configuration stored in excluded `secrets.h`, never committed to version control

## Hardware

| Component | Role |
|---|---|
| Waveshare ESP32-S3 N16R8 | Main microcontroller (16MB flash, 8MB PSRAM) |
| SSD1306 0.96" OLED (I2C) | Live transaction status display |
| SPI Micro SD Card Module | Local device-side audit log storage |

## Architecture
ESP32-S3                         Laptop (Node.js)
|				    |
|── TLS handshake (cert verify) ───>|
|── POST /log (encrypted JSON) ────>|── server/transactions.log
|<─ {"status":"OK", timestamp} ─────|
|                                    |
SD card: txn_log.txt           OLED: live status
(device audit trail)

## Project Structure
banking-endpoint-simulator/
├── banking_endpoint/
│   ├── banking_endpoint.ino    # ESP32-S3 firmware
├── secrets.h.example       # Credentials template
└── .gitignore
└── server/
	├── server.js               # Node.js HTTPS backend
	├── cert.pem.example        # Certificate placeholder
	└── package.json

## Setup

### Backend (Node.js server)

**Requirements:** Node.js v18+, OpenSSL

```bash
cd server
npm install

# Generate self-signed TLS certificate
openssl req -x509 -newkey rsa:2048 \
  -keyout key.pem \
  -out cert.pem \
  -days 365 \
  -nodes \
  -subj "/CN=YOUR_LAPTOP_IP" \
  -addext "subjectAltName=IP:YOUR_LAPTOP_IP"

node server.js
```

### ESP32-S3 Firmware

**Requirements:** Arduino IDE 2.x, ESP32 board package 3.x

1. Copy `secrets.h.example` to `secrets.h` and fill in your credentials:
```cpp
#define WIFI_SSID "your_wifi_name"
#define WIFI_PASSWORD "your_wifi_password"
#define SERVER_HOST "your_laptop_ip"
#define SERVER_PORT 8443
#define SERVER_PATH "/log"
```

2. Copy the contents of your generated `cert.pem` into the `serverCert` string in `banking_endpoint.ino`

3. Select **Tools → Board → ESP32S3 Dev Module**

4. Upload the sketch

### Wiring

| OLED Pin | ESP32-S3 |
|---|---|
| VCC | 3V3 |
| GND | GND |
| SCL | GPIO9 |
| SDA | GPIO8 |

| SD Module Pin | ESP32-S3 |
|---|---|
| VCC | 3V3 |
| GND | GND |
| MISO | GPIO13 |
| MOSI | GPIO11 |
| SCK | GPIO12 |
| CS | GPIO10 |

## How It Works

The ESP32-S3 acts as a hardware banking endpoint — simulating an ATM logging node. Every 10 seconds it:

1. Generates a mock transaction payload with a unique ID, random amount, merchant code, and the device's MAC address as a hardware fingerprint
2. Establishes a TLS-encrypted HTTPS connection to the Node.js backend, verifying the server certificate before transmitting
3. POSTs the encrypted transaction payload to `/log`
4. Receives a timestamped confirmation response
5. Logs the transaction result to the SD card regardless of server response — maintaining a local audit trail even during network outages
6. Updates the OLED display with live transaction status

## Skills Demonstrated

`ESP32-S3` `C++` `TLS/SSL` `HTTPS` `Node.js` `Express` `Hardware Security` `IoT` `SPI` `I2C` `Embedded Systems` `Digital Forensics`

## Author

Neville Iregi — [Portfolio](https://n-iregi.github.io/labs) | [GitHub](https://github.com/N-Iregi)
