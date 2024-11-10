#include <Arduino.h>
#include <SensirionI2CScd4x.h>
#include <Wire.h>

SensirionI2CScd4x scd4x;

void setup() {
  Serial.begin(115200);

  while (!Serial) {
    delay(100);
  }

  uint16_t error;
  char errorMessage[256];

  scd4x.begin(Wire);

  error = scd4x.startPeriodicMeasurement();

  if (error) {
    Serial.print("Error starting measurement: ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }
}

void loop() {
  uint16_t error;
  char errorMessage[256];

  delay(5000);

  uint16_t co2;
  float temperature;
  float humidity;

  error = scd4x.readMeasurement(co2, temperature, humidity);

  if (error) {
    Serial.print("Error reading measurement: ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  } else if (co2 == 0) {
    Serial.println("Invalid sample detected, skipping.");
  } else {
    Serial.print("Co2: ");
    Serial.print(co2);
    Serial.print(" ppm\t");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" °C\t");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %RH");
  }
}
