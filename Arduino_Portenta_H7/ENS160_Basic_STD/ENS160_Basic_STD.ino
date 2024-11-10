/***************************************************************************
  ENS160 - Digital Air Quality Sensor
  
  This is an example for ENS160 basic reading in standard mode
    
  Updated by Sciosense / 25-Nov-2021
 ***************************************************************************/

#include <Wire.h>
int ArduinoLED = 13;

#include "ScioSense_ENS160.h"  // ENS160 library
//  ScioSense_ENS160  ens160(ENS160_I2CADDR_0);
ScioSense_ENS160  ens160(ENS160_I2CADDR_1);

#include  "Robot_Controller.h"

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
  Halt everything - used for unrecoverable errors

  Parameters:
    message:    The message to disaplay on the serial console
    wifi_halt:  If this is a WiFi connection halt, true
    wifi_ssid:  If wifi_halt is true, this should be the SSID of the WiFi network

  Returns:      void
*/
void halt ( char* message, bool wifi_halt=false, char *wifi_ssid=ssid) {
  Serial.println();
  Serial.print(message);

  if (wifi_halt) {
    Serial.print(" ");
    Serial.print(wifi_ssid);
  }
}

/*--------------------------------------------------------------------------
  SETUP function
  initiate sensor
 --------------------------------------------------------------------------*/
void setup() {
  bool success = true;

  Serial.begin(115200);

  while (!Serial) {}

  //Switch on LED for init
  pinMode(ArduinoLED, OUTPUT);
  digitalWrite(ArduinoLED, LOW);

  Serial.println("------------------------------------------------------------");
  Serial.println("ENS160 - Digital air quality sensor");
  Serial.println();
  Serial.println("Sensor readout in standard mode");
  Serial.println();
  Serial.println("------------------------------------------------------------");
  delay(1000);

  success = ens160.begin();

  if (success) {
    Serial.print("Found an ENS160 MOX Sensor");
    //Serial.println(ens160.available() ? "done." : "failed!");

    if (ens160.available()) {
      // Print ENS160
      Serial.print("Revison: ");
      Serial.print(ens160.getMajorRev());
      Serial.print(".");
      Serial.print(ens160.getMinorRev());
      Serial.print(".");
      Serial.print(ens160.getBuild());  

      success = ens160.setMode(ENS160_OPMODE_STD);

      if (success) {
        Serial.println(" in Standard mode ");
        Serial.println();
      } else {
        Serial.println();
      }
    }
  } else {
    halt("Unable to find the ENS160 MOX sensor!");
  }
}

/*--------------------------------------------------------------------------
  MAIN LOOP FUNCTION
  Cylce every 1000ms and perform measurement
 --------------------------------------------------------------------------*/

void loop() {
  if (ens160.available()) {
    looper++;
  
    ens160.measure(true);
    ens160.measureRaw(true);

    Serial.print("Loop #");
    Serial.print(looper);
    Serial.print(", AQI: ");
    Serial.print(ens160.getAQI());
    Serial.print(", TVOC: ");
    Serial.print(ens160.getTVOC()); 
    Serial.print(" ppb, eCO2: ");
    Serial.print(ens160.geteCO2());
    Serial.print(" ppm, R HP0: ");
    Serial.print(ens160.getHP0());
    Serial.print(" Ohms, R HP1: ");
    Serial.print(ens160.getHP1());
    Serial.print(" Ohms, R HP2: ");
    Serial.print(ens160.getHP2());
    Serial.print(" Ohms, R HP3: ");
    Serial.print(ens160.getHP3());
    Serial.println(" Ohms");
    Serial.println();
  }
  
  delay(5000);
}
