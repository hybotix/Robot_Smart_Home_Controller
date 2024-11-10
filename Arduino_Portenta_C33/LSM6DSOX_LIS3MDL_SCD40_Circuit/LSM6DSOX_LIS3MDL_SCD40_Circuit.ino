/*
  Arduino LSM6DSOX - Simple Accelerometer

  This example reads the acceleration values from the LSM6DSOX
  sensor and continuously prints them to the Serial Monitor
  or Serial Plotter.

  The circuit:
  - Arduino Nano RP2040 Connect

  created 10 May 2021
  by Arturo Guadalupi

  This example code is in the public domain.
*/
#include <Arduino.h>
#include <Arduino_LSM6DSOX.h>
#include <Wire.h>

#include <SensirionI2CScd4x.h>
SensirionI2CScd4x scd4x;

#include <Adafruit_LIS3MDL.h>
#include <Adafruit_Sensor.h>

Adafruit_LIS3MDL lis3mdl;

/*
  Convert the Celsius temperature to Fahrenheit

  Returns: (float) temperature in fahrenheit
*/
float to_fahrenheit (float celsius) {
  return celsius * 1.8 + 32;
}

void setup() {
  uint16_t error;
  char errorMessage[256];

  Serial.begin(115200);
  while (!Serial);

  Wire.begin();

  if (IMU.begin()) {
    Serial.println("Found an LSM6DSOX IMU");
  } else {
    Serial.println("Failed to initialize LSM6DSOX IMU!");

    while (true) {
      delay(10);
    };
  }

  if (lis3mdl.begin_I2C()) {
    Serial.println("Found an LIS3MDL Magnetometer");
  } else {
    Serial.println("Failed to find LIS3MDL Magnetometer!");

    while (true) { 
      delay(10);
    }
  }

  Serial.print("Accelerometer sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println(" Hz");
  Serial.println();
  Serial.println("Acceleration readings are in g's");

  Serial.print("Gyroscope sample rate = ");
  Serial.print(IMU.gyroscopeSampleRate());
  Serial.println(" Hz");
  Serial.println("Gyroscope readings are in °/sec");

  delay(2000);

  Serial.println("Initializing the SCD-40 CO2 sensor.");
  scd4x.begin(Wire);

  error = scd4x.startPeriodicMeasurement();

  if (error) {
    Serial.print("Error starting measurement: ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  } else {
    Serial.println("The SCD-40 CO2 sensor is working properly.");
  }
}

void loop() {
  sensors_event_t event; 
  float lsm6dsox_celsius, lsm6dsox_fahrenheit = 0.0;
  float x, y, z;

  uint16_t scd40_co2;
  float scd40_celsius, scd40_fahrenheit, scd40_humidity = 0.0;
  uint16_t scd40_error;
  char scd40_message[256];

  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);

    Serial.print("Accelerometer: x = ");
    Serial.print(x);
    Serial.print(", y = ");
    Serial.print(y);
    Serial.print(", z = ");
    Serial.print(z);
    Serial.println(" g's");
  }

  if (IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(x, y, z);

    Serial.print("Gyroscope: x = ");
    Serial.print(x);
    Serial.print(", y = ");
    Serial.print(y);
    Serial.print(", z = ");
    Serial.print(z);
    Serial.println(" °/sec");
  }

  if (IMU.temperatureAvailable()) {
    IMU.readTemperatureFloat(lsm6dsox_celsius);
    lsm6dsox_fahrenheit = to_fahrenheit(lsm6dsox_celsius);

    Serial.print("Temperature: ");
    Serial.print(lsm6dsox_fahrenheit);
    Serial.print(" °F (");
    Serial.print(lsm6dsox_celsius);
    Serial.println(" °C)");
  }

  lis3mdl.getEvent(&event);

  Serial.print("Magnetometer: x = ");
  Serial.print(event.magnetic.x);
  Serial.print(", y = ");
  Serial.print(event.magnetic.y);
  Serial.print(", z = ");
  Serial.print(event.magnetic.z);
  Serial.println(" uTesla ");

  //  Read the SCD-40 CO2 sensor
  scd40_error = scd4x.readMeasurement(scd40_co2, scd40_celsius, scd40_humidity);

  if (scd40_error) {
    Serial.print("Error reading measurement: ");
    errorToString(scd40_error, scd40_message, 256);
    Serial.println(scd40_message);
  } else if (scd40_co2 == 0) {
    Serial.println("Invalid sample detected, skipping.");
  } else {
    scd40_fahrenheit = to_fahrenheit(scd40_celsius);
    Serial.print("CO2: ");
    Serial.print(scd40_co2);
    Serial.print(" ppm, ");
    Serial.print("Temperature: ");
    Serial.print(scd40_fahrenheit);
    Serial.print(" °F (");
    Serial.print(scd40_celsius);
    Serial.print(" °C)");
    Serial.print(", Humidity: ");
    Serial.print(scd40_humidity);
    Serial.println(" %RH");
  }

  Serial.println();
  delay(5000);
}
