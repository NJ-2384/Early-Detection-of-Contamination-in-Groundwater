#include <Arduino.h>

// Updated pins based on your working snippet
#define MODEM_RX 17
#define MODEM_TX 16

void waitForResponse() {
  // Wait for response from Modem
  unsigned long timeout = millis() + 2000; 
  while (!Serial2.available() && millis() < timeout) {
    // Yield to avoid watchdog trigger
    delay(1);
  }
  // Print the response to Serial Monitor
  while (Serial2.available()) {
    Serial.write(Serial2.read());
  }
}

void setup() {
  Serial.begin(115200);
  // Initialize hardware Serial with the pins that worked for you
  Serial2.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  
  delay(1000);
  Serial.println("\n--- A7672S MODEM BRIDGE (CORRECTED PINS) ---");
  Serial.println("Type your AT command and press Enter.");
  Serial.println("--------------------------------------------");
}

void loop() {
  // Check if any data is available on Serial Monitor
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    
    if (cmd.length() > 0) {
      Serial.print("[SENDING] ");
      Serial.println(cmd);
      
      Serial2.println(cmd);
      
      // Give the modem time to process and respond
      delay(500); 
      waitForResponse();
    }
  }

  // Also catch any unsolicited messages (like SMS or Network status)
  if (Serial2.available()) {
    while (Serial2.available()) {
      Serial.write(Serial2.read());
    }
  }
}