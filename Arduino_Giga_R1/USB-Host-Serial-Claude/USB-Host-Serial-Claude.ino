#include <Arduino_USBHostMbed5.h>

USBHostSerial serial;  // Create USB Host Serial object

// Buffer for receiving data
const int BUFFER_SIZE = 256;
uint8_t buffer[BUFFER_SIZE];
int bufferPosition = 0;

void setup() {
  // Initialize built-in serial for debugging
  Serial.begin(115200);

  while (!Serial) {
    delay(10); // Wait for serial port to connect
  }

  Serial.println("Arduino Giga R1 WiFi USB Host Serial Example");
}

void loop() {
  // Check if USB device is connected and available
  if (serial.connected() && serial.available()) {
    // Read from USB device
    int bytesRead = serial.read(buffer + bufferPosition, 
                               BUFFER_SIZE - bufferPosition);
    
    if (bytesRead > 0) {
      bufferPosition += bytesRead;
      
      // Process complete messages
      processBuffer();
      
      // Echo received data to debug serial
      Serial.write(buffer, bytesRead);
      Serial.flush();
    }
  }
  
  // Check if there's data to send to USB device
  if (serial.connected() && Serial.available()) {
    // Read from debug serial and send to USB device
    int bytesRead = Serial.readBytes(buffer, BUFFER_SIZE);
    if (bytesRead > 0) {
      serial.write(buffer, bytesRead);
    }
  }
  
  // If not connected, check periodically
  if (!serial.connected()) {
    // Reset buffer position when device disconnected
    bufferPosition = 0;
    
    // Print connection status periodically
    static unsigned long lastCheck = 0;
    unsigned long now = millis();
    if (now - lastCheck > 1000) {
      Serial.println("Waiting for USB device...");
      lastCheck = now;
    }
  }
}

void processBuffer() {
  // Process complete messages in buffer
  // This example looks for newline-terminated messages
  int processedUntil = 0;
  
  for (int i = 0; i < bufferPosition; i++) {
    if (buffer[i] == '\n') {
      // Found complete message, process it
      handleMessage(buffer + processedUntil, i - processedUntil + 1);
      processedUntil = i + 1;
    }
  }
  
  // Move any remaining incomplete message to start of buffer
  if (processedUntil < bufferPosition) {
    memmove(buffer, buffer + processedUntil, 
            bufferPosition - processedUntil);
    bufferPosition -= processedUntil;
  } else {
    bufferPosition = 0;
  }
}

void handleMessage(uint8_t* msg, int length) {
  // Example message handler - modify as needed
  // This example just prints the message length
  Serial.print("Received message of length: ");
  Serial.println(length);
  
  // Add your message processing logic here
  // For example, parsing commands or data protocols
}