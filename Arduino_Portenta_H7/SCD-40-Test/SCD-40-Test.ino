/*
 * Copyright (c) 2021, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <Arduino.h>
#include <Wire.h>

#include <SensirionI2CScd4x.h>
SensirionI2CScd4x scd4x;

#include  "Robot_Controller.h"
#include  "Secrets.h"

/**********************
  Utility routines
**********************/

/*
  Blink the onboard RGB LED with the selected color

  Parameters:
    color:          a ColorRGB structure that defines the desired color
    blink_rate_ms:  The blink rate in ms
    nr_cycles:      The number of times to blink the LED

  Returns:          void
*/
void blink_rgb (ColorRGB color, uint8_t blink_rate_ms=DEFAULT_BLINK_RATE_MS, uint8_t nr_cycles=DEFAULT_NR_CYCLES) {
  uint8_t count;

  for (count=0; count<nr_cycles; count++) {
    digitalWrite(LEDR, color.redB);
    digitalWrite(LEDG, color.greenB);
    digitalWrite(LEDB, color.blueB);

    delay(blink_rate_ms);

    digitalWrite(LEDR, HIGH);
    digitalWrite(LEDG, HIGH);
    digitalWrite(LEDB, HIGH);

    delay(blink_rate_ms);
  }
}

/*
  Blink a standard LED on the selected Raspberry Pi pin.
  Pins are active HIGH

  Parameters:
    pin:            The pin the LED is connected to
    blink_rate_ms:  The blink rate in ms
    nr_cycles:      The number of times to blink the LED

  Returns:          void
*/
void blink_led_raspi (uint8_t pin, uint8_t blink_rate_ms=DEFAULT_BLINK_RATE_MS, uint8_t nr_cycles=DEFAULT_NR_CYCLES) {
  uint8_t index;

  for (index=0; index < nr_cycles; index++) {
    //  Turn the LED ON
    digitalWrite(pin, HIGH);
    delay(blink_rate_ms);
      
    //  Turn the LED OFF
    digitalWrite(pin, LOW);
    delay(blink_rate_ms);
  }
}

/*
  Halt everything - used for unrecoverable errors

  Parameters:
    message:    The message to disaplay on the serial console
    wifi_halt:  If this is a WiFi connection halt, true
    wifi_ssid:  If wifi_halt is true, this should be the SSID of the WiFi network

  Returns:      void
*/
void halt (String message, bool wifi_halt=false, char* wifi_ssid=ssid) {
  Serial.println();
  Serial.print(message);

  if (wifi_halt) {
    Serial.print(" ");
    Serial.print(wifi_ssid);
  }

  //  Turn off all other status LEDs
  digitalWrite(LED_RASPI_WIFI_PIN, LOW);
  digitalWrite(LED_RASPI_CONNECT_PIN, LOW);

  //  Infinite loop
  while (true) {
    blink_led_raspi(LED_RASPI_HALT_PIN);
    delay(100);
  }
}

/*
  Convert the Celsius temperature to Fahrenheit

  Returns: (float) temperature in fahrenheit
*/
float to_fahrenheit (float celsius) {
  return celsius * 1.8 + 32;
}

void printUint16Hex(uint16_t value) {
    Serial.print(value < 4096 ? "0" : "");
    Serial.print(value < 256 ? "0" : "");
    Serial.print(value < 16 ? "0" : "");
    Serial.print(value, HEX);
}

void printSerialNumber(uint16_t serial0, uint16_t serial1, uint16_t serial2) {
    Serial.print("Found an SCD-4x with serial number: 0x");
    printUint16Hex(serial0);
    printUint16Hex(serial1);
    printUint16Hex(serial2);
    Serial.println();
}

void setup() {
  uint16_t error;
  String errorMessage;
  uint16_t serial0;
  uint16_t serial1;
  uint16_t serial2;

  Serial.begin(115200);

  while (!Serial) {
    delay(100);
  }

  looper = 0;

  Wire.begin();

  scd4x.begin(Wire);
//  Serial.println("Waiting for I2C...");
//  delay(5000);

  error = scd4x.getSerialNumber(serial0, serial1, serial2);

  if (error) {
    //errorToString(error, errorMessage, 256);
    errorMessage = "Error trying to execute getSerialNumber(): Code " + String(error);
    halt(errorMessage);
  } else {
    printSerialNumber(serial0, serial1, serial2);

    //  Stop potentially previously started measurement
    error = scd4x.stopPeriodicMeasurement();
    
    if (error) {
      //errorToString(error, errorMessage, 256);
      errorMessage = "Error trying to execute stopPeriodicMeasurement(): Code " + String(error);
      halt(errorMessage);
    } else {
      //  Start Measurement
      error = scd4x.startPeriodicMeasurement();

      if (error) {
        //errorToString(error, errorMessage, 256);
        errorMessage = "Error trying to execute startPeriodicMeasurement(): Code " + String(error);
        halt(errorMessage);
      } else {
        Serial.println();
        Serial.println("Waiting for first measurement... (5 seconds)");
        Serial.println();

        delay(5000);
      }
    }
  }
}

void loop() {
  uint16_t error;
  char errorMessage[256];
  uint16_t co2;
  float celsius, fahrenheit;
  float humidity;

  looper++;
  
  // Read Measurement
  error = scd4x.readMeasurement(co2, celsius, humidity);
  
  if (error) {
    Serial.print("Error trying to execute readMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  } else if (co2 == 0) {
      Serial.println("Invalid sample detected, skipping.");
  } else {
    fahrenheit = to_fahrenheit(celsius);

    Serial.print("Loop #");
    Serial.print(looper);
    Serial.print(", CO2: ");
    Serial.print(co2);
    Serial.print(" ppm, Temperature: ");
    Serial.print(fahrenheit);
    Serial.print("°F (");
    Serial.print(celsius);
    Serial.print("°C), Humidity: ");
    Serial.print(humidity);
    Serial.println("%");

    Serial.println();
    
    delay(29000);
  }
 
  delay(5000);
}
