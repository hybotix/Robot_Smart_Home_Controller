//
//    FILE: DS3232_demo_read.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: test basic read function
//     URL: https://github.com/RobTillaart/DS3232
#include "DS3232.h"
DS3232 rtc;

#include <SparkFun_I2C_Mux_Arduino_Library.h>
QWIICMUX mux;

#include "Robot_Controller.h"

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
void halt (String message, bool wifi_halt=false, char *wifi_ssid=ssid) {
  Serial.println();
  Serial.print(message);

  if (wifi_halt) {
    Serial.print(" ");
    Serial.print(wifi_ssid);
  }

  //  Infinite loop
  while (true) {
    blink_rgb(Red);
    delay(100);
  }
}

void set_mux_port(QWIICMUX *mx, uint8_t port, String device_name) {
  if (port == DEVICE_NOT_PRESENT) {
    halt(device_name + " mux port is not valid!");
  } else {
    mx->setPort(port);
  }
}

void init_mux (QWIICMUX *mx) {
  bool mux_present;

  //  Initialize the Sparkfun I2C Multiplexor
  mux_present = mx->begin();

  if (mux_present) {
    Serial.println("Found a SparkFun I2C Multiplexor");
  } else {
    halt("No SparkFun I2C Multiplexor was detected!");
  }
}

void init_ds3231 (QWIICMUX *mx, DS3232 *clock, uint8_t port, String device_name) {
  //  Set the mux port
  set_mux_port(mx, port, device_name);

  if (clock->begin() == DS3232_OK) {
    Serial.println("Found a " + device_name);
  } else {
    halt("Could not find a "+ device_name + "!");
  }
}

void setup(void) {
  Serial.begin(115200);
  Serial.println();
  Serial.println(__FILE__);
  Serial.print("DS3232_LIB_VERSION: ");
  Serial.println(DS3232_LIB_VERSION);
  Serial.println();

  Wire.begin();

  init_mux(&mux);

  init_ds3231(&mux, &rtc, MUX_DS3231_RTC, "DS3231 Real Time Clock");
}

void loop(void) {
  //  Make sure we have the DS3231 port selected
  set_mux_port(&mux, MUX_DS3231_RTC, "DS3231 Real Time Clock");

  rtc.read();

  Serial.print("Last read: ");
  Serial.print(rtc.lastRead());
  Serial.print(", Date is ");
  Serial.print(rtc.month());
  Serial.print("/");
  Serial.print(rtc.day());
  Serial.print("/");
  Serial.print(rtc.year());
  Serial.print(", Time is ");
  Serial.print(rtc.hours());
  Serial.print(":");
  Serial.print(rtc.minutes());
  Serial.print(":");
  Serial.print(rtc.seconds());
  Serial.print(", ");
  Serial.print(rtc.weekDay());
  Serial.println();

  delay(2000);
}
