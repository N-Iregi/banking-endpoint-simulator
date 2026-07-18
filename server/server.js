const https = require('https');
const fs = require('fs');
const express = require('express');

const app = express();
app.use(express.json());

// Load TLS certificate and key
const options = {
  key: fs.readFileSync('key.pem'),
  cert: fs.readFileSync('cert.pem')
};

// Transaction log file
const LOG_FILE = 'transactions.log';

// POST /log — receives transaction from ESP32-S3
app.post('/log', (req, res) => {
  const transaction = req.body;
  const timestamp = new Date().toISOString();

  // Validate basic structure
  if (!transaction || !transaction.transaction_id) {
    return res.status(400).json({
      status: 'ERROR',
      message: 'Invalid transaction payload'
    });
  }

  // Build log entry
  const logEntry = `[${timestamp}] ${JSON.stringify(transaction)}\n`;

  // Write to log file
  fs.appendFileSync(LOG_FILE, logEntry);

  //q Print to console
  console.log(`Transaction received: ${logEntry.trim()}`);

  // Respond to ESP32-S3
  res.status(200).json({
    status: 'OK',
    message: 'Transaction logged',
    timestamp: timestamp
  });
});

// GET /status — health check endpoint
app.get('/status', (req, res) => {
  res.json({
    status: 'OK',
    message: 'Banking Endpoint Simulator running',
    uptime: process.uptime()
  });
});

// Start HTTPS server
const PORT = 8443;
https.createServer(options, app).listen(PORT, () => {
  console.log(`Secure Banking Endpoint running on https://localhost:${PORT}`);
  console.log(`Waiting for transactions from ESP32-S3...`);
});
