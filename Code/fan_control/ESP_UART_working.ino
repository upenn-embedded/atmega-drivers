#include <Arduino.h>

// FeatherS2 UART pins
#define RX_PIN 44
#define TX_PIN 43

// Baud rate for communication with ATmega328PB
#define ATMEGA_BAUD_RATE 9600

// Hardware Serial for ATmega328PB communication
HardwareSerial AtmegaSerial(1);

void setup() {
  // Initialize the native USB Serial for the Serial Monitor
  Serial.begin(115200);

  // Initialize the Hardware Serial to communicate with ATmega328PB
  // Parameters: Baud Rate, Config, RX Pin, TX Pin
  AtmegaSerial.begin(ATMEGA_BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN);
  
  Serial.println("Ready to forward data to ATmega328PB...");
}

void loop() {
  // Check if data is available from USB Serial (computer)
  if (Serial.available()) {
    // Read the incoming byte from USB
    char incomingByte = Serial.read();

    // Forward the byte to ATmega328PB via UART
    AtmegaSerial.write(incomingByte);

    // Print confirmation to Serial Monitor
    Serial.print("Forwarded to ATmega: ");
    Serial.println(incomingByte);
  }
}