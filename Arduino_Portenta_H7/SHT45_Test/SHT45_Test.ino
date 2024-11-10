/*************************************************** 
  This is an example for the SHT4x Humidity & Temp Sensor

  Designed specifically to work with the SHT4x sensor from Adafruit
  ----> https://www.adafruit.com/products/4885

  These sensors use I2C to communicate, 2 pins are required to  
  interface
 ****************************************************/

#include "Adafruit_SHT4x.h"

Adafruit_SHT4x sht4 = Adafruit_SHT4x();

/*
  Convert the Celsius temperature to Fahrenheit

  Returns: (float) temperature in fahrenheit
*/
float to_fahrenheit (float celsius) {
  return celsius * 1.8 + 32;
}

void setup() {
  Serial.begin(115200);

  while (!Serial)
    delay(10);     // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("Adafruit SHT4x test");
  if (! sht4.begin()) {
    Serial.println("Couldn't find SHT4x");
    while (1) delay(1);
  }
  Serial.println("Found an SHT4x sensor with serial number 0x");
  Serial.print(sht4.readSerial(), HEX);
  Serial.print(", using ");

  // You can have 3 different precisions, higher precision takes longer
  sht4.setPrecision(SHT4X_HIGH_PRECISION);

  switch (sht4.getPrecision()) {
     case SHT4X_HIGH_PRECISION: 
       Serial.print("High precision");
       break;
     case SHT4X_MED_PRECISION: 
       Serial.print("Med precision");
       break;
     case SHT4X_LOW_PRECISION: 
       Serial.print("Low precision");
       break;
  }

  Serial.print(" and ");

  // You can have 6 different heater settings
  // higher heat and longer times uses more power
  // and reads will take longer too!
  sht4.setHeater(SHT4X_NO_HEATER);

  switch (sht4.getHeater()) {
     case SHT4X_NO_HEATER: 
       Serial.println("No heater");
       break;
     case SHT4X_HIGH_HEATER_1S: 
       Serial.println("High heat for 1 second");
       break;
     case SHT4X_HIGH_HEATER_100MS: 
       Serial.println("High heat for 0.1 second");
       break;
     case SHT4X_MED_HEATER_1S: 
       Serial.println("Medium heat for 1 second");
       break;
     case SHT4X_MED_HEATER_100MS: 
       Serial.println("Medium heat for 0.1 second");
       break;
     case SHT4X_LOW_HEATER_1S: 
       Serial.println("Low heat for 1 second");
       break;
     case SHT4X_LOW_HEATER_100MS: 
       Serial.println("Low heat for 0.1 second");
       break;
  }
}

void loop() {
  sensors_event_t rel_humidity, temp;
  uint32_t timestamp = millis();
  float celsius, fahrenheit, humidity;

  //  Populate temp and humidity objects with fresh data
  sht4.getEvent(&rel_humidity, &temp);
  celsius = temp.temperature;
  fahrenheit = to_fahrenheit(celsius);
  humidity = rel_humidity.relative_humidity;

  timestamp = millis() - timestamp;

  Serial.print("Temperature: ");
  Serial.print(fahrenheit);
  Serial.print("°F (");
  Serial.print(celsius);
  Serial.print("°C), Humidity: ");
  Serial.print(humidity);
  Serial.print("% rH, Read duration: ");
  Serial.print(timestamp);
  Serial.println(" ms");

  Serial.println();

  delay(5000);
}