#include <WiFi.h>
#include <Arduino.h>

// Baud rate for the serial connection with the ATmega
#define ATMEGA_BAUD_RATE 19200

// ESP32-S2 pins for Hardware Serial (Serial1)
#define RX_PIN 44
#define TX_PIN 43

// WiFi credentials
const char* ssid = "Apple Vision Pro";
const char* password = "xiangchen";

// Python server details
const char* serverIP = "172.20.10.7"; // Replace with your computer's IP address
const int serverPort = 7040;

// Objects
HardwareSerial MySerial(1); // Use Serial1
WiFiClient client;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  // enable USB Serial for Monitoring
  Serial.begin(115200);
  Serial.println("--- FeatherS2 Receiver & WiFi Sender Started ---");
  
  // Initialize Hardware Serial for ATmega communication
  MySerial.begin(ATMEGA_BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN);
  Serial.print("Hardware Serial initialized on pins RX:");
  Serial.print(RX_PIN);
  Serial.print(" / TX:");
  Serial.println(TX_PIN);

  // 3. Connect to WiFi
  digitalWrite(LED_BUILTIN, HIGH);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(LED_BUILTIN, LOW);
  
  Serial.println("Waiting for data...");
}

void loop() {
  // Check if data is available
  if (MySerial.available()) {
    Serial.println("serial available");
    char lastChar = '\0'; // Null character
    
    // Buffer Draining
    // Read EVERYTHING currently in the buffer until it's empty.
    // We only care about the very last byte received (the most current state).
    while (MySerial.available()) {
      lastChar = MySerial.read();
    }

    // Now 'lastChar' holds the most recent command from the MCU
    if (lastChar != '\0') {
      
      // Connect/Send only if we have fresh data
      if (client.connected() || client.connect(serverIP, serverPort)) {
          client.print("hand: ");
          client.println(lastChar);
      }
    }
  }
  
  // No delay here, or a tiny one (1ms) just to keep the watchdog happy
  delay(1); 
}