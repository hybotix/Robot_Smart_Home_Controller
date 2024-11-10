/******************************************************************************************
 * Code from the original sketch was created by ArduinoGetStarted.com
 *
 * This example code is in the public domain
 *
 * Tutorial page: https://arduinogetstarted.com/tutorials/arduino-web-server-multiple-pages
 *******************************************************************************************
 *  Heavily modifed and extended by Dale Weber <hybotics@hybotics.dev>
 *
 *  This is the control sketch for the Robot Smart Home Controller
 *  Copyright (c) by Dale Weber <hybotics@hybotics.dev> 2024
 ******************************************************************************************/
#include  "Secrets.h"
#include  "Robot_Controller.h"
#include  "Web_Server_Control.h"
#include  "Arduino_HAT_Carrier_H7.h"

/*
  Web pages to be served
*/
#include  "index.h"
#include  "environment.h"
#include  "switches.h"
#include  "potentiometer.h"
#include  "light.h"
#include  "imu.h"
#include  "error_404.h"
#include  "error_405.h"

/*************************
  Internal Arduino Libraries
*************************/

#include  <Wire.h>

/*************************
  Arduino Libraries
*************************/

#include <Arduino_LSM6DSOX.h>

/*************************
  Third-Party Libraries
*************************/

// Include the RTC library
#include "RTClib.h"
RTC_DS3231 rtc;

#include "ScioSense_ENS160.h"  // ENS160 library
//  ScioSense_ENS160  ens160(ENS160_I2CADDR_0);
ScioSense_ENS160  ens160(ENS160_I2CADDR_1);

#include <Adafruit_LIS3MDL.h>
Adafruit_LIS3MDL lis3;

#include "Adafruit_SHT4x.h"
Adafruit_SHT4x sht45 = Adafruit_SHT4x();

#include <SensirionI2CScd4x.h>
SensirionI2CScd4x scd40;

#include "Adafruit_VEML7700.h"
Adafruit_VEML7700 veml = Adafruit_VEML7700();

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
  Blink a standard LED on the selected pin.
  Pins are active LOW

  Parameters:
    pin:            The pin the LED is connected to
    blink_rate_ms:  The blink rate in ms
    nr_cycles:      The number of times to blink the LED

  Returns:          void
*/
void blink_led_c33 (uint8_t pin, uint8_t blink_rate_ms=DEFAULT_BLINK_RATE_MS, uint8_t nr_cycles=DEFAULT_NR_CYCLES) {
  uint8_t index;

  for (index=0; index < nr_cycles; index++) {
    //  Turn the LED ON
    digitalWrite(pin, LOW);
    delay(blink_rate_ms);
      
    //  Turn the LED OFF
    digitalWrite(pin, HIGH);
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
void halt (String message, bool wifi_halt=false, char *wifi_ssid=ssid) {
  Serial.println();
  Serial.println(message);

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

void print_uint_16_hex(uint16_t value) {
  Serial.print(value < 4096 ? "0" : "");
  Serial.print(value < 256 ? "0" : "");
  Serial.print(value < 16 ? "0" : "");
  Serial.print(value, HEX);
}

/*
  Do the actual left padding - there is no checking done

  Parameters:
    str:          The string to be padded
    numeric_only: True if the string passed must be numeric
    pad_length:   The length to pad the string to
    pad_char:     The String character for padding

  Returns:      void
*/
String pad_string (String str, uint8_t pad_length=DEFAULT_PAD_LENGTH, String pad_char=DEFAULT_PAD_STRING) {
  uint8_t str_index = 0;
  String result_str = str;

  for (str_index=1; str_index < pad_length; str_index++) {
    result_str = pad_char + result_str;
  }

  return result_str; 
}

/*
  Left pad a string - works with -unsigned- numeric strings only at this time

  Parameters:
    str:          The string to be padded
    numeric_only: True if the string passed must be numeric
    pad_length:   The length to pad the string to
    pad_char:     The String character for padding

  Returns:      void
*/
String left_pad (String str, uint8_t pad_length=DEFAULT_PAD_LENGTH, bool numeric_only=true, String pad_char=DEFAULT_PAD_STRING) {
  uint8_t str_len, str_index = 0, position = 0;
  String result_str = "", digits = "0123456789", temp_str = "";
  bool is_number = true;

  str_len = str.length();

/*
  Serial.print("str = '");
  Serial.print(str);
  Serial.print("', str_len = ");
  Serial.println(str_len);
*/

  if (str_len == pad_length) {
    result_str = str;
  } else {
    if (numeric_only) {
      //  Scan the string to be sure it is all numeric characters
      for (str_index=0; str_index < str_len; str_index++) {
        //  Get the character to check
        temp_str = str.substring(str_index, str_index + 1);

        //  See if the character is a digit
        position = digits.indexOf(temp_str) + 1;
/*
        Serial.print("temp_str = '");
        Serial.print(temp_str);
        Serial.print("', position = ");
        Serial.println(position);
*/
        //  Test the character. If position < 0, character is not a digit
        is_number = (position > 0);

        if (!is_number) {
          //  Break the loop because there is a non-numeric characte in a numeric string
          break;
        }
      }

      if (is_number) {
        result_str = str;
        //  Add the appropriate number of pad_char to the left of the string
        result_str = pad_string(str, pad_length, pad_char);
      } else {
        //  Invalid - Non-digit character is present
        result_str = "**";
      }
    } else {
        //  Add the appropriate number of pad_char to the left of the string
        result_str = pad_string(str, pad_length, pad_char);
    }
  }
/*
  Serial.print("result_str = '");
  Serial.print(result_str);
  Serial.println("'");
*/
  return result_str;
}

/*
  Create a date and time stamp of the current time

  Parameters:
*/
String timestamp (bool show_full_date=SHOW_FULL_DATE, bool hours_mode=SHOW_24_HOURS, bool long_date=SHOW_LONG_DATE, bool show_seconds=SHOW_SECONDS) {
  DateTime current = rtc.now();
  String date_time = "", date_str = "D*", time_str = "T*";
  String year_str = "Y*", month_str = "M*", day_str = "D*", week_day_str;
  //String hours_str;
  String am_pm = " AM", day_suffix = "**";
  uint16_t year;
  uint8_t week_day, day, suffix;
  uint8_t month, hours, minutes, seconds;

  //  Create a standard date and time stamp
  date_time = left_pad(String(current.year()), 4) + "-" + left_pad(String(current.month()), 2) + "-" +
    left_pad(String(current.day()), 2) + "T" + left_pad(String(current.hour()), 2) + ":" +
    left_pad(String(current.minute()), 2) + ":" + left_pad(String(current.second()), 2);

  //  Get all the date and time information
  hours = current.hour();
  minutes = current.minute();
  seconds = current.second();
  year = current.year();
  month = current.month();
  day = current.day();
  week_day = current.dayOfTheWeek();
  year_str = String(year);

  //  Default time and date strings
  time_str = left_pad(String(hours), 2) + ":" + left_pad(String(minutes), 2);
  date_str = left_pad(String(month), 2) + "/" + left_pad(String(day), 2) + "/" + left_pad(String(year), 4);

  if (show_full_date) {
    //  Show the full date with date and time
    if (long_date) {
      //  Get the last digit of the day of the month
      suffix = String(day).substring(1, 1).toInt();
      //  Get the day suffix for the long date
      switch(suffix) {
        case 0:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
          day_suffix = "th";
          break;
        case 1:
          day_suffix = "st";
          break;
        case 2:
          day_suffix = "nd";
          break;
        case 3:
          day_suffix = "rd";
          break;
        default:
          day_suffix = "**";
          break;
      }

      //  Create the date string
      week_day_str = days_of_the_week[week_day];
      day_str = String(day) + day_suffix;
      month_str = long_months[month];
      date_str = week_day_str + ", " + long_months[month - 1] + " " + day_str + ", " + year_str;
    } else {
      day_str = String(day);
      month_str = String(month);
      date_str = day_str + "/" + month_str + "/" + year_str;
    }
  }

  //  Handle the 12 and 24 hour format
  if (hours_mode == SHOW_24_HOURS) {
    //  24 hour format
    am_pm = "";
    time_str = left_pad(String(hours), 2) + ":" + left_pad(String(minutes), 2);
  } else {
    //  12 hour format
    if (hours < 12) {
      am_pm = " am";

      if (hours == 0) {
        hours = 12;
      }
    } else {
      am_pm = " pm";

      if (hours > 12) {
        hours = hours - 12;
      }
    }

    time_str = String(hours) + ":" + left_pad(String(minutes), 2);
  }

  if (show_seconds) {
    time_str = time_str + ":" + left_pad(String(seconds), 2);
  }

  time_str = time_str + am_pm;
  date_time = date_str + " at " + time_str;

  return date_time;
}

/*
  Convert the Celsius temperature to Fahrenheit

  Returns: (float) temperature in fahrenheit
*/
float to_fahrenheit (float celsius) {
  return celsius * 1.8 + 32;
}

void init_ens160 (void) {
  bool success = ens160.begin();

  if (success) {
    Serial.print("Found an ENS160 MOX Sensor");

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
      } else {
        halt("Unable to set mode on the ENS160 MOX sensor!");
      }
    }
  } else {
    halt("Unable to find the ENS160 MOX sensor!");
  }
}

/*
  Initialize the SCD-40 CO2, Temperature, and Humidity sensor
*/
void init_scd40 (void) {
  String error_message;
  uint16_t error, serial0, serial1, serial2;

  scd40.begin(Wire);
//  Serial.println("Waiting for I2C...");
//  delay(5000);

  error = scd40.getSerialNumber(serial0, serial1, serial2);

  if (error) {
    error_message = "SCD-40: Error trying to get the serial number: Code " + String(error);
    halt(error_message);
  } else {
    Serial.print("Found an SCD-40 with serial number: 0x");
    print_uint_16_hex(serial0);
    print_uint_16_hex(serial1);
    print_uint_16_hex(serial2);
    Serial.println();
  }

  //  Stop potentially previously started measurement
  error = scd40.stopPeriodicMeasurement();
    
  if (error) {
    error_message = "SCD-40: Error trying to stop periodic measurement: Code " + String(error);
    halt(error_message);
  } else {
    //  Start Measurement
    error = scd40.startPeriodicMeasurement();

    if (error) {
      error_message = "SCD-40: Error trying to execute start periodic measurement: Code " + String(error);
      halt(error_message);
    }
  }
}

/*
  Initialize the sht45 temperature and humidity `
*/
Adafruit_SHT4x init_sht45 (Adafruit_SHT4x *sht, System_Sensor_Status *sen_stat) {
  sen_stat->sht45_status = sht->begin();

  if (sen_stat->sht45_status) {
    Serial.print("Found an SHT4x sensor with the serial number 0x");
    Serial.println(sht->readSerial(), HEX);

    // You can have 3 different precisions, higher precision takes longer
    sht->setPrecision(SHT4X_HIGH_PRECISION);

    switch (sht->getPrecision()) {
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

    Serial.print(", ");

    //  You can have 6 different heater settings
    //    higher heat and longer times uses more power
    //    and reads will take longer too!
    sht->setHeater(SHT4X_NO_HEATER);

    switch (sht->getHeater()) {
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

    Serial.println();
  } else {
    halt("Could not find any SHT4x sensors!");
  }

  return *sht;
}

/*
  Initialize the SHT45 Temeprature and Humidity sensor
*/
Adafruit_LIS3MDL init_lis3mdl (Environment_Data curr_data, Adafruit_LIS3MDL *lis3, System_Sensor_Status *sen_stat) {
  // Try to initialize!
  if (lis3->begin_I2C()) {
    sen_stat->lis3mdl_status = true;
  } else {
    sen_stat->lis3mdl_status = false; 
  }

  if (sen_stat->lis3mdl_status) {          // hardware I2C mode, can pass in address & alt Wire
    Serial.println("Found the LIS3MDL Magnetometer!");

    /*  
      Magnetometer Performance Mode

      Possible Values:
        LIS3MDL_LOWPOWERMODE:
        LIS3MDL_MEDIUMMODE:
        LIS3MDL_HIGHMODE:
        LIS3MDL_ULTRAHIGHMODE:
    */
    lis3->setPerformanceMode(LIS3MDL_LOWPOWERMODE);

    /*
      Magnetometer Operation Mode

      Single shot mode will complete conversion and go into power down

      Possible Values:
        LIS3MDL_CONTINUOUSMODE:
        LIS3MDL_SINGLEMODE:
        LIS3MDL_POWERDOWNMODE:
    */
    lis3->setOperationMode(LIS3MDL_CONTINUOUSMODE);

    //Serial.print("Operation mode set to: ");
    lis3->setDataRate(LIS3MDL_DATARATE_155_HZ);

    //  You can check the datarate by looking at the frequency of the DRDY pin
    //  Serial.print("Data rate set to: ");
    /*
    switch (lis3->getDataRate()) {
      case LIS3MDL_DATARATE_0_625_HZ:
        Serial.println("0.625 Hz");
        break;
      case LIS3MDL_DATARATE_1_25_HZ:
        Serial.println("1.25 Hz");
        break;
      case LIS3MDL_DATARATE_2_5_HZ:
        Serial.println("2.5 Hz");
        break;
      case LIS3MDL_DATARATE_5_HZ:
        Serial.println("5 Hz");
        break;
      case LIS3MDL_DATARATE_10_HZ:
        Serial.println("10 Hz");
        break;
      case LIS3MDL_DATARATE_20_HZ:
        Serial.println("20 Hz");
        break;
      case LIS3MDL_DATARATE_40_HZ:
        Serial.println("40 Hz");
        break;
      case LIS3MDL_DATARATE_80_HZ:
        Serial.println("80 Hz");
        break;
      case LIS3MDL_DATARATE_155_HZ:
        Serial.println("155 Hz");
        break;
      case LIS3MDL_DATARATE_300_HZ:
        Serial.println("300 Hz");
        break;
      case LIS3MDL_DATARATE_560_HZ:
        Serial.println("560 Hz");
        break;
      case LIS3MDL_DATARATE_1000_HZ:
        Serial.println("1000 Hz");
        break;
      case default:
        Serial.println("Invalid value!");
    }
    */
    
    lis3->setRange(LIS3MDL_RANGE_4_GAUSS);

    //Serial.print("Range set to: ");
    /*
    switch (lis3->getRange()) {
      case LIS3MDL_RANGE_4_GAUSS:
        Serial.println("+-4 gauss");
        break;
      case LIS3MDL_RANGE_8_GAUSS:
        Serial.println("+-8 gauss");
        break;
      case LIS3MDL_RANGE_12_GAUSS:
        Serial.println("+-12 gauss");
        break;
      case LIS3MDL_RANGE_16_GAUSS:
        Serial.println("+-16 gauss");
        break;
    }
    */

    lis3->setIntThreshold(500);

    lis3->configInterrupt(false, false, true, // enable z axis
                            true, // polarity
                            false, // don't latch
                            true); // enabled!  
  } else {
    halt("Unable to find and initialize the LIS3MDL magnetometer!");
  }

  return *lis3;
}

/*
  Initialize the LSM6DSOX IMU
*/
void init_lsm6dsox (System_Sensor_Status *sen_stat) {
  //  Initialie the IMU
  if (IMU.begin()) {
    sen_stat->lsm6dsox_status = true;
  } else {
    sen_stat->lsm6dsox_status = false;
  }

  if (sen_stat->lsm6dsox_status) {
    Serial.println("Found an LSM6DSOX IMU!");

    Serial.print("Accelerometer sample rate = ");
    Serial.print(IMU.accelerationSampleRate());
    Serial.println(" Hz");

    Serial.print("Gyroscope sample rate = ");
    Serial.print(IMU.gyroscopeSampleRate());
    Serial.println(" Hz");

    /*
      Accelerometer Range

      Possible values:
        LSM6DS_ACCEL_RANGE_2_G:
        LSM6DS_ACCEL_RANGE_4_G:
        LSM6DS_ACCEL_RANGE_8_G:
        LSM6DS_ACCEL_RANGE_16_G:
    */
    //IMU.setAccelRange(LSM6DS_ACCEL_RANGE_2_G);

    /*
      Accelerometer Data Rate

      Possible Values:
        LSM6DS_RATE_SHUTDOWN:
        LSM6DS_RATE_12_5_HZ:
        LSM6DS_RATE_26_HZ:
        LSM6DS_RATE_52_HZ:
        LSM6DS_RATE_104_HZ:
        LSM6DS_RATE_208_HZ:
        LSM6DS_RATE_416_HZ:
        LSM6DS_RATE_833_HZ:
        LSM6DS_RATE_1_66K_HZ:
        LSM6DS_RATE_3_33K_HZ:
        LSM6DS_RATE_6_66K_HZ:
    */
    //IMU.setAccelDataRate(LSM6DS_RATE_12_5_HZ);

    /*
      Gyroscope Range

      Possible Values:
        LSM6DS_GYRO_RANGE_125_DPS:
        LSM6DS_GYRO_RANGE_250_DPS:
        LSM6DS_GYRO_RANGE_500_DPS:
        LSM6DS_GYRO_RANGE_1000_DPS:
        LSM6DS_GYRO_RANGE_2000_DPS:
        ISM330DHCX_GYRO_RANGE_4000_DPS:
    */
    //IMU.setGyroRange(LSM6DS_GYRO_RANGE_250_DPS );

    /*
      Gyroscope Data Rate

      Possible Values:
        LSM6DS_RATE_SHUTDOWN:
        LSM6DS_RATE_12_5_HZ:
        LSM6DS_RATE_26_HZ:
        LSM6DS_RATE_52_HZ:
        LSM6DS_RATE_104_HZ:
        LSM6DS_RATE_208_HZ:
        LSM6DS_RATE_416_HZ:
        LSM6DS_RATE_833_HZ:
        LSM6DS_RATE_1_66K_HZ:
        LSM6DS_RATE_3_33K_HZ:
        LSM6DS_RATE_6_66K_HZ:
    */
    //IMU.setGyroDataRate(LSM6DS_RATE_12_5_HZ);
  } else {
    halt("Failed to initialize the LSM6DSOX IMU!");
  }
}

/*    
  Print out the connection status:
*/
void print_wifi_status(void) {
  //  Print your board's IP address:
  Serial.print("Connected! IP Address is ");
  Serial.println(ip_addr);

  //  Print the received signal strength:
  Serial.print("Signal strength (RSSI): ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
}

bool connect_to_wifi (char *ssid, char *passwd, uint8_t connection_timeout_ms=CONNECTION_TIMEOUT_MS, uint8_t  max_nr_connects=MAX_NR_CONNECTS) {
  bool connected;
  uint8_t connect_count = 0;

  //  Global variable in Robot_Controller.h
  wifi_status = WiFi.status();

  // attempt to connect to Wifi network:
  while (wifi_status != WL_CONNECTED and (connect_count < max_nr_connects)) {
    connect_count += 1;

    Serial.print("Attempt #");
    Serial.print(connect_count);
    Serial.print(" connecting to the '");
    Serial.print(ssid);
    Serial.println("' network");

    /*
        Connect to WPA/WPA2 network.
        Change this line if you are using an open or WEP network:
    */
    WiFi.begin(ssid, passwd);

    //  Wait for connection:
    delay(connection_timeout_ms);
    wifi_status = WiFi.status();
  }

  if (wifi_status == WL_CONNECTED and connect_count < max_nr_connects) {
    connected = true;

    //  Global variable in Robot_Controller.h
    ip_addr = WiFi.localIP();

    //  Print connection status
    print_wifi_status();
  } else {
    ip_addr = "0.0.0.0";
    connected = false;
  }

  return connected;
}

/*
  Set the LEDs according to the states of the switches.
*/
bool set_leds (uint8_t nr_of_switches=NUMBER_OF_SWITCHES, uint8_t nr_of_leds=NUMBER_OF_LEDS) {
  uint8_t index;
  bool result = true;

  if (nr_of_switches == nr_of_leds) {
    for (index=0; index < nr_of_switches; index++) {
      if (SWITCH_READINGS[index]) {
        digitalWrite(LED_PINS[index], HIGH);
      } else {
        digitalWrite(LED_PINS[index], LOW);
      }
    }
  } else {
    result = false;
  }
}

/*
  Set the LEDs according to the states of the switches.
*/
bool set_leds_raspi (uint8_t nr_of_switches=NUMBER_OF_SWITCHES, uint8_t nr_of_leds=NUMBER_OF_LEDS) {
  uint8_t index;
  bool result = true;

  if (digitalRead(SWITCH_WHITE_PIN)) {
    digitalWrite(LED_WHITE_PIN, HIGH);
  } else {
    digitalWrite(LED_WHITE_PIN, LOW);
  }
}

/*
  Initialize the digital LED outputs

  For the Portenta C33, LOW = Active (HIGH or ON state)
*/
void init_leds (uint8_t nr_of_leds=NUMBER_OF_LEDS, uint8_t blink_delay_ms=DEFAULT_BLINK_RATE_MS) {
  uint8_t index;

  //  Initialize the RGB LED - Set pins to be outputs
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  //  Turn the RGB LED OFF
  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);

  //  Set the status LED pins to OUTPUT
  for (index=0; index < nr_of_leds; index++) {
    //  Set the LED pins to OUTPUT
    pinMode(LED_PINS[index], OUTPUT);
    digitalWrite(LED_PINS[index], HIGH);

    //  Blink each of the LEDs
    blink_led_c33(LED_PINS[index]);
    delay(100);
  }

  Serial.print("There are ");
  Serial.print(nr_of_leds);
  Serial.println(" LEDs (Digital Output)");
  Serial.println();

  //  Initialize the Raspberry Pi GPIO pins
  pinMode(LED_RASPI_CONNECT_PIN, OUTPUT);             //  ON = Good WiFi connection
  blink_led_raspi(LED_RASPI_CONNECT_PIN, 250, 1);
  digitalWrite(LED_RASPI_CONNECT_PIN, LOW);

  pinMode(LED_RASPI_WIFI_PIN, OUTPUT);                //  When connecting to WiFi, blinks the number
  blink_led_raspi(LED_RASPI_WIFI_PIN, 250, 1);
  digitalWrite(LED_RASPI_WIFI_PIN, LOW);              //    of attempts that have been made if > 1

  pinMode(LED_RASPI_HALT_PIN, OUTPUT);                //  Blinks steady when a WiFi connection
  blink_led_raspi(LED_RASPI_HALT_PIN, 250, 1);
  digitalWrite(LED_RASPI_HALT_PIN, LOW);              //    was not made
}

/*
  Initialize the digital pins for the switches
*/
void init_switches(uint8_t nr_of_switches=NUMBER_OF_SWITCHES) {
  uint8_t index;

  for (index=0; index < nr_of_switches; index++) {
    //  Set the analog pin to INPUT
    pinMode(SWITCH_PINS[index], OUTPUT);
    digitalWrite(SWITCH_PINS[index], HIGH);
  }

  Serial.print("There are ");
  Serial.print(nr_of_switches);
  Serial.println(" switches (Digital Input)");
}

/*
  Initialize the digital pins for the switches
*/
void init_switches_raspi(uint8_t nr_of_switches=NUMBER_OF_SWITCHES) {
  uint8_t index;

  pinMode(SWITCH_WHITE_PIN, INPUT_PULLUP);

  Serial.print("There are ");
  Serial.print(nr_of_switches);
  Serial.println(" switches (Digital Input)");
}

/*
  Initialize the analog pins for the resistors
*/
void init_resistors(uint8_t nr_of_resistors=NUMBER_OF_RESISTORS) {
  uint8_t index;

  for (index=0; index < nr_of_resistors; index++) {
    //  Set the analog pin to INPUT
    pinMode(RESISTOR_PINS[index], INPUT);
  }

  Serial.print("There are ");
  Serial.print(nr_of_resistors);
  Serial.println(" resistors (Analog Input)");
}

/*
  Initialize the HTML for web pages.

  Doing this pre-initialization of the HTML saves some replacements in the
    web server code. These are things that do not change in the web page but
    still have to be easily changeable. This should allow the server to execute a
    bit faster.

  HTML is in String array PAGE_HTML[]
*/
void init_html (uint8_t max_pages=MAX_NUM_PAGES) {
  uint8_t page_nr;
  String html;

  Serial.println();
  Serial.println("Initializing HTML");

  for (page_nr=0; page_nr < max_pages; page_nr++) {
    switch (page_nr) {
      case PAGE_HOME:
        html = String(HTML_CONTENT_HOME);
        html.replace("PAGE_HOME_TITLE_MARKER", PAGE_HOME_TITLE);
        html.replace("PAGE_HOME_NAME_MARKER", PAGE_HOME_NAME);
        html.replace("SKETCH_CODE_MARKER", SKETCH_ID_CODE);

        PAGE_HTML[PAGE_HOME] = html;
        break;
      case PAGE_ENVIRONMENT:        
        html = String(HTML_CONTENT_ENVIRONMENT);
        html.replace("PAGE_ENVIRONMENT_TITLE_MARKER", PAGE_ENVIRONMENT_NAME);
        html.replace("PAGE_ENVIRONMENT_NAME_MARKER", PAGE_ENVIRONMENT_NAME);

        PAGE_HTML[PAGE_ENVIRONMENT] = html;
        break;
      case PAGE_SWITCHES:
        html = String(HTML_CONTENT_SWITCHES);
        html.replace("PAGE_SWITCHES_TITLE_MARKER", PAGE_SWITCHES_TITLE);
        html.replace("PAGE_SWITCHES_NAME_MARKER", PAGE_SWITCHES_NAME);

        PAGE_HTML[PAGE_SWITCHES] = html;
        break;
      case PAGE_POTENTIOMETER:
        html = String(HTML_CONTENT_POTENTIOMETER);
        html.replace("PAGE_POTENTIOMETER_TITLE_MARKER", PAGE_POTENTIOMETER_TITLE);
        html.replace("PAGE_POTENTIOMETER_NAME_MARKER", PAGE_POTENTIOMETER_NAME);

        PAGE_HTML[PAGE_POTENTIOMETER] = html;
        break;
      case PAGE_LIGHT:
        html = String(HTML_CONTENT_LIGHT);
        html.replace("PAGE_LIGHT_TITLE_MARKER", PAGE_LIGHT_TITLE);
        html.replace("PAGE_LIGHT_NAME_MARKER", PAGE_LIGHT_NAME);

        PAGE_HTML[PAGE_LIGHT] = html;
        break;
      case PAGE_IMU:
        html = String(HTML_CONTENT_IMU);
        html.replace("PAGE_IMU_TITLE_MARKER", PAGE_IMU_TITLE);
        html.replace("PAGE_IMU_NAME_MARKER", PAGE_IMU_NAME);

        PAGE_HTML[PAGE_IMU] = html;
        break;
      default:
        halt("Web Page ID is out of bounds!");
        break;
    }
  }
}

/*
  Check the environment data and intialize it if necessary
*/
Environment_Data check_data (Environment_Data curr_data, bool initialize=false) {
  Environment_Data result;
  Three_Axis filler;

  //  If we have current data, copy it
  if (curr_data.valid) {
    //  Save existing data
    result = curr_data;
  } else if (initialize) {
    //  Initialize the sensor data structure
    filler.x = 0.0;
    filler.y = 0.0;
    filler.z = 0.0;

    //  Initialize data structures
    result.valid = false;

    result.ens160.aqi = 0;
    result.ens160.tvoc = 0;
    result.ens160.eCO2 = 0;
    result.ens160.hp0 = 0;
    result.ens160.hp1 = 0;
    result.ens160.hp2 = 0;
    result.ens160.hp3 = 0;

    result.lsm6dsox.accelerometer = filler;
    result.lsm6dsox.gyroscope = filler;
    result.lsm6dsox.magnetometer = filler;
    result.lsm6dsox.celsius = 0.0;
    result.lsm6dsox.fahrenheit = 0.0;

    result.sht45.celsius = 0.0;
    result.sht45.fahrenheit = 0.0;
    result.sht45.humidity = 0.0;

    result.scd40.CO2 = 0;
    result.scd40.celsius = 0.0;
    result.scd40.fahrenheit = 0.0;
  }

  return result;
}

Environment_Data get_ens160 (Environment_Data curr_data, ScioSense_ENS160 *ens) {
  uint16_t count = 0;
  Environment_Data sensors;

  while (!ens->available() and count < ENS160_MAX_TRIES) {
    //  Wait for data to be available
    delay(100);
    count++;
  }

  if (count < ENS160_MAX_TRIES) {
    sensors = check_data(curr_data);

    sensors.ens160.valid = true;

    ens->measure(true);
    ens->measureRaw(true);

    sensors.ens160.aqi = ens->getAQI();
    sensors.ens160.tvoc = ens->getTVOC();
    sensors.ens160.eCO2 = ens->geteCO2();
    sensors.ens160.hp0 = ens->getHP0();
    sensors.ens160.hp1 = ens->getHP1();
    sensors.ens160.hp2 = ens->getHP2();
    sensors.ens160.hp3 = ens->getHP3();
  } else {
    sensors.ens160.valid = false;
  }

  return sensors;
}

/*
  Get readings from the LIS3MDL Magnetometer and put them in the environment
    data structure.
*/
Environment_Data get_lis3mdl (Environment_Data curr_data, Adafruit_LIS3MDL*lis3, bool initialize=false) {
  Environment_Data sensors;
  sensors_event_t event;
  sensors = check_data(curr_data, initialize);

  lis3->getEvent(&event);
  sensors.lsm6dsox.magnetometer.x = event.magnetic.x;
  sensors.lsm6dsox.magnetometer.y = event.magnetic.y;
  sensors.lsm6dsox.magnetometer.z = event.magnetic.z;

  return sensors;
}

/*
  Format the X, Y, and Z readings for output
*/
String imu_format_xyz_html (Three_Axis readings) {
  return "<SPAN STYLE=\"color: yellow\">x</SPAN> = <SPAN STYLE=\"color: green\">" + String(readings.x) + "</SPAN>, <SPAN STYLE=\"color: yellow\">y</SPAN> = <SPAN STYLE=\"color: green\">" + String(readings.y) + "</SPAN>, <SPAN STYLE=\"color: yellow\">z</SPAN> = <SPAN STYLE=\"color: green\">" + String(readings.z) + "</SPAN>";
}

/*
  Get readings from the LSM6DSOX IMU and put them in the environment
    data structure.
*/
Environment_Data get_lsm6dsox (Environment_Data curr_data, Adafruit_LIS3MDL *lis3, bool is_9dof=true) {
  Environment_Data sensors;
  float x, y, z;
  float temperature = 0.0;

  sensors = check_data(curr_data);

  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);

    sensors.lsm6dsox.accelerometer.x = x;
    sensors.lsm6dsox.accelerometer.y = y;
    sensors.lsm6dsox.accelerometer.z = z;

    Serial.print("Accelerometer sample rate = ");
    Serial.print(IMU.accelerationSampleRate());
    Serial.println(" Hz");
  }

  if (IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(x, y, z);

    sensors.lsm6dsox.gyroscope.x = x;
    sensors.lsm6dsox.gyroscope.y = y;
    sensors.lsm6dsox.gyroscope.z = z;

    Serial.print("Gyroscope sample rate = ");
    Serial.print(IMU.gyroscopeSampleRate());
    Serial.println(" Hz");
  }

  if (IMU.temperatureAvailable()) {
    IMU.readTemperatureFloat(temperature);
    sensors.lsm6dsox.celsius = temperature;
    sensors.lsm6dsox.fahrenheit = to_fahrenheit(temperature);
  }

  //  If we have a 9DOF IMU, read the magnetometer
  if (is_9dof) {
    sensors = get_lis3mdl(curr_data, lis3, false);
  }

  return sensors;
}

Environment_Data get_scd40 (Environment_Data curr_data, SensirionI2CScd4x *scd) {
  Environment_Data sensors;
  String error_message;
  uint16_t CO2, error;
  float celsius, fahrenheit, humidity;

  sensors = check_data(curr_data);

  //  Delay to give the sensor a few seconds
  delay(2000);

  // Read Measurement
  error = scd->readMeasurement(CO2, celsius, humidity);

  //  Check if we got an error and retry if so  
  if (error) {
    delay(1000);
    error = scd->readMeasurement(CO2, celsius, humidity);

    if (error){
      error_message = "Error trying to get a measurement, skipping: Code " + String(error);
      Serial.println(error_message);
    }
  } else if (CO2 == 0) {
    //  Wait and retry for invalid samples
    delay(1000);
    error = scd->readMeasurement(CO2, celsius, humidity);

    if (error) {
      error_message = "Invalid SCD-40 CO2 sample detected, code " + String(error);
      Serial.println(error_message);
    }
  }
  
  if (!error) {
    //  Put the new sensor readings into the sensor structure
    fahrenheit = to_fahrenheit(celsius);
    sensors.scd40.celsius = celsius;
    sensors.scd40.fahrenheit = fahrenheit;
    sensors.scd40.humidity = humidity;
    sensors.scd40.CO2 = CO2;
  }

  return sensors;
}

/*
  Get temperature and humidity readings from the SHT45 and put them i sensor
    and put them into the environment data structure.
*/
Environment_Data get_sht45 (Environment_Data curr_data, Adafruit_SHT4x *sht) {
  Environment_Data sensors;
  sensors_event_t rel_humidity, temperature;
  float celsius, fahrenheit, humidity;
  uint32_t timestamp = millis();

  sensors = check_data(curr_data);

  //  Populate temperature and humidity objects with fresh data
  sht->getEvent(&rel_humidity, &temperature);
  celsius = temperature.temperature;
  fahrenheit = to_fahrenheit(celsius);
  humidity = rel_humidity.relative_humidity;

/*
  Serial.print("Temperature: " );
  Serial.print(fahrenheit);
  Serial.print("° F (");
  Serial.print(celsius);
  Serial.print("° C), Relative Humidity: ");
  Serial.print(humidity);
  Serial.println("%");
*/

  sensors.sht45.celsius = celsius;
  sensors.sht45.fahrenheit = fahrenheit;
  sensors.sht45.humidity = humidity;

  return sensors;
}

/*
  Read resistor voltages
*/
void read_resistors (uint8_t nr_of_resistors=NUMBER_OF_RESISTORS) {
  uint8_t index;

  Serial.print("Resistor readings: ");

  for (index=0; index < nr_of_resistors; index++) {
    RESISTOR_READINGS[index] = analogRead(RESISTOR_PINS[index]);
    RESISTOR_VOLTAGES[index] = (MAXIMUM_ANALOG_VOLTAGE / ANALOG_RESOLUTION) * RESISTOR_READINGS[index];

    Serial.print(RESISTOR_NAMES[index]);
    Serial.print(" = ");
    Serial.print(RESISTOR_VOLTAGES[index], 5);

    if (index < nr_of_resistors - 1) {
      Serial.print(", ");
    }

    delay(2);
  }

  Serial.println();

  Serial.print("Resistor voltages: ");

  for (index=0; index < nr_of_resistors; index++) {
    Serial.print(RESISTOR_NAMES[index]);
    Serial.print(" = ");
    Serial.print(RESISTOR_VOLTAGES[index], 5);
    Serial.print("V");

    if (index < nr_of_resistors - 1) {
      Serial.print(", ");
    }
  }

  Serial.println();
}

/*
  Return the HTML for the switches and their states (ON or OFF)
*/
String switch_format_html (uint8_t nr_of_switches=NUMBER_OF_SWITCHES) {
  uint8_t index;
  String html = "", state;

  for (index=0; index < nr_of_switches; index++) {
    if (SWITCH_READINGS[index]) {
      state ="<SPAN style=\"color: magenta\">OFF</SPAN>";
    } else {
      state = "<SPAN style=\"color: green\">ON</SPAN>";
    }

    html = html + "<SPAN style=\"color: yellow\">" + SWITCH_NAMES[index] + "</SPAN> = " + state;

    if (index < nr_of_switches - 1) {
      html = html + ", ";
    }
  }

  return html;
}

void show_switches (uint8_t nr_of_switches=NUMBER_OF_SWITCHES) {
  uint8_t index;

  Serial.print("Switch states: ");

  for (index=0; index < nr_of_switches; index++) {
    Serial.print(SWITCH_NAMES[index]);
    Serial.print(" = ");

    if (SWITCH_READINGS[index]) {
      Serial.print("OFF");
    } else {
      Serial.print("ON");
    }

    if (index < nr_of_switches - 1) {
      Serial.print(", ");
    }
  }

  Serial.println();
}

float read_veml7700(Adafruit_VEML7700 *vm) {
  return vm->readLux();
}

void read_switches (uint8_t nr_of_switches=NUMBER_OF_SWITCHES) {
  uint8_t index;
  String html;

  for (index=0; index < nr_of_switches; index++) {
    SWITCH_READINGS[index] = digitalRead(SWITCH_PINS[index]);
  }
}
          Serial.println("Environment Page");
          page_id = PAGE_ENVIRONMENT;
        } else if (HTTP_req.indexOf("GET /switches ") > -1 || HTTP_req.indexOf("GET /switches.html ") > -1) {
          Serial.println("Door Page");
          page_id = PAGE_SWITCHES;
        } else if (HTTP_req.indexOf("GET /potentiometer ") > -1 || HTTP_req.indexOf("GET /potentiometer.html ") > -1) {
          Serial.println("Potentiometer Page");
          page_id = PAGE_POTENTIOMETER;
        } else if (HTTP_req.indexOf("GET /light ") > -1 || HTTP_req.indexOf("GET /light.html ") > -1) {
          Serial.println("Light Page");
          page_id = PAGE_LIGHT;
        } else if (HTTP_req.indexOf("GET /imu ") > -1 || HTTP_req.indexOf("GET /imu.html ") > -1) {
          Serial.println("IMU Page");
          page_id = PAGE_IMU;
        } else {  // 404 Not Found
          Serial.println("404 Not Found");
          page_id = PAGE_ERROR_404;
        }
        //  Send the HTTP response
        //  Send the HTTP response header
        if (page_id == PAGE_ERROR_404)
          client.println("HTTP/1.1 404 Not Found");
        if (page_id == PAGE_ERROR_405)
          client.println("HTTP/1.1 405 Method Not Allowed");
        else
          client.println("HTTP/1.1 200 OK");

        client.println("Content-Type: text/html");
        client.println("Connection: close");  // the connection will be closed after completion of the response
        client.println();                     // the separator between HTTP header and body

        //  Get the current date and time for time stamping pages

        //  Send the HTTP response body
        switch (page_id) {
          case PAGE_HOME:
            html = PAGE_HTML[PAGE_HOME]; //String(HTML_CONTENT_HOME);
            date_time = timestamp(SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, NO_SECONDS);
            Serial.println(PAGE_HOME_NAME);

            //  Replace the markers by real values
            //html.replace("PAGE_HOME_TITLE_MARKER", PAGE_HOME_TITLE);
            //html.replace("PAGE_HOME_NAME_MARKER", PAGE_HOME_NAME);
            //html.replace("SKETCH_CODE_MARKER", SKETCH_ID_CODE);
            //html.replace("ROBOT_PAGE_NAME_MARKER", ROBOT_DEVICE_NAME);
            //html.replace("ROBOT_NAME_MARKER", ROBOT_DEVICE_NAME);
            html.replace("SEQUENCE_COUNT_MARKER", String(sequence_nr));
            html.replace("DATESTAMP_MARKER", date_time);
            break;
          case PAGE_ENVIRONMENT:
            html =  PAGE_HTML[PAGE_ENVIRONMENT]; //String(HTML_CONTENT_ENVIRONMENT);
            date_time = timestamp(SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, NO_SECONDS);
            Serial.println(PAGE_ENVIRONMENT_NAME);
            html.replace("SEQUENCE_COUNT_MARKER", String(sequence_nr));

            Serial.println("Sending sensor readings");

            if (USING_SHT45_TEMP) {
              sensors = get_sht45(sensors, &sht45);

              html.replace("SHT45_FAHRENHEIT_MARKER", String(sensors.sht45.fahrenheit));
              html.replace("SHT45_CELSIUS_MARKER", String(sensors.sht45.celsius));
              html.replace("SHT45_HUMIDITY_MARKER", String(sensors.sht45.humidity));
            } else {
              html.replace("SHT45_FAHRENHEIT_MARKER", "Not used");
              html.replace("SHT45_CELSIUS_MARKER", "Not used");
              html.replace("SHT45_HUMIDITY_MARKER", "Not used");
    
void setup (void) {
  uint8_t note_nr;
  String firmware_version;
  bool connected;

  analogWrite(PIEZO_BUZZER_PIN, 0);

  //  Initialize serial and wait for the port to open:
  Serial.begin(SERIAL_BAUDRATE);
  delay(SERIAL_DELAY_MS);

  //  Wait for the serial port to stabilize
  //while(!Serial) {
  //  delay(10);
  //}

  Serial.print(ROBOT_DEVICE_NAME);
  Serial.print(", Version ");
  Serial.print(ROBOT_DEVICE_VERSION);
  Serial.print(", ");
  Serial.println(ROBOT_DEVICE_DATE);
  Serial.println();

  init_leds();

/*  
  pinMode(PIEZO_BUZZER_PIN, OUTPUT);

  for (note_nr=0; note_nr < MAX_NR_NOTES; note_nr++) {
    Serial.print("Note #");
    Serial.print(note_nr);
    Serial.print(", note = ");
    Serial.println(MUSIC_NOTES[note_nr]);
    tone(PIEZO_BUZZER_PIN, MUSIC_NOTES[note_nr], NOTE_DURATION_MS);
    delay(NOTE_DELAY_MS);
    noTone(PIEZO_BUZZER_PIN);
    delay(1000);
  }
*/

  //  Check for the WiFi module:
  if (WiFi.status() == WL_NO_SHIELD) {
    halt("There is no WiFi module present!");
  }

  connected = connect_to_wifi(ssid, passwd);

  if (connected) {
    //  Start the I2C bus
    Wire.begin();

    // Turn on the WiFi connection LED
    digitalWrite(LED_RASPI_CONNECT_PIN, HIGH);

    //  Initialize the static HTML
    init_html(MAX_NUM_PAGES);

    //  Initialize all switches
    init_switches();
    //init_switches_raspi();

    //  Set analog resolutionto 12 bits
    analogReadResolution(12);

    //  Initialize the analog pins
    //init_resistors();

    //  Initialize the sensors data structure
    sensors.initialize = true;
    sensors = check_data(sensors);

    if (USING_ENS160_MOX) {
      init_ens160();
    }

    //  initialize the SCD-40 CO2, Temperature, and Humidity sensor
    if (USING_SCD40_CO2) {
      init_scd40();
    }

    //  Initialize the SHT4x Temperature and Humidity sensors
    if (USING_SHT45_TEMP) {
      if (! sht45.begin()) {
        Serial.println("Could not find an SHT4x sensor!");
        while (1) delay(1);
      }
    }

    //  Initialize the LSM6DSOX IMU
    if (USING_LSM6DSOX_LIS3MDL_IMU) {  
      if (IMU.begin()) {
        Serial.println("Found an LSM6DSOX IMU");
      } else {
        halt("Cound not find an LSM6DSOX IMU!");
      }
    }

    //  Initialize the LIS3MDL Magnetometer
    if (USING_LIS3MDL_MAG) {
      lis3 = init_lis3mdl(sensors, &lis3, &sensor_status);
    }

    //  Initialize the VEML7700 Light (Lux) sensor
    if (USING_VEML7700_LUX) {
      if (veml.begin()) {
        Serial.print("Found a VEML7700 Lux sensor with a Gain of ");

        switch (veml.getGain()) {
          case VEML7700_GAIN_1:
            Serial.print("1");
            break;
          case VEML7700_GAIN_2:
            Serial.print("2");
            break;
          case VEML7700_GAIN_1_4:
            Serial.print("1/4");
            break;
          case VEML7700_GAIN_1_8:
            Serial.print("1/8");
            break;
        }

        Serial.print(" and Integration Time of ");

        switch (veml.getIntegrationTime()) {
          case VEML7700_IT_25MS:
            Serial.print("25");
            break;
          case VEML7700_IT_50MS:
            Serial.print("50");
            break;
          case VEML7700_IT_100MS:
            Serial.print("100");
            break;
          case VEML7700_IT_200MS:
            Serial.print("200");
            break;
          case VEML7700_IT_400MS:
            Serial.print("400");
            break;
          case VEML7700_IT_800MS:
            Serial.print("800");
            break;
        }

        Serial.println(" ms");
      } else {
        halt("Unable to find the VEML7700 sensor!");
      }
    }

    //  Start the Real Time Clock and set the current time
    Serial.println("Starting the RTC");

    if (!rtc.begin()) {
      halt("Could not find the DS3231 RTC");
    } else if (rtc.lostPower()) {
      Serial.println("RTC lost power, let's set the time!");
      // When time needs to be set on a new device, or after a power loss, the
      // following line sets the RTC to the date & time this sketch was compiled
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      // This line sets the RTC with an explicit date & time, for example to set
      // January 21, 2014 at 3am you would call:
      // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    }

    Serial.println();
    Serial.print("Today is ");
    Serial.println(timestamp(SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, SHOW_SECONDS));
  
    //  Start the web server
    Serial.println();
    Serial.println("Starting the web server");
    server.begin();
    Serial.println();
  } else {
    halt("Unable to connect to network", true, ssid);
  }

  //  For testing setup()
  //halt("END OF SETUP HALT");

  Serial.println("Initization complete!");
  delay(1000);
}

void loop (void) {
  WiFiClient client = server.available();
  bool send_page = true, connected = (wifi_status == WL_CONNECTED);
  float lux, potentiometer_voltage;
  uint16_t sequence_nr = 0, potentiometer_reading;
  uint8_t index = 0;
  int page_id = 0, index_pos = 0, start_pos = 0;
  unsigned long start_millis, end_millis;
  String HTTP_req, html = "", date_time = "", temp_html = "", led_html = "";
  String potentiometer_units;

  while (connected) {
    //  Heartbeat
    blink_rgb(blue);

    //Serial.println();
    //Serial.print("Loop #");
    looper++;
    //Serial.println(looper);

    read_switches();
    //show_switches();
    set_leds();

    client = server.available();

    //  Listen for incoming clients
    if (client) {
      HTTP_req = "";

      blink_rgb(magenta);

      sequence_nr++;
      start_millis = millis();

      Serial.print("***** Sequence #");
      Serial.print(sequence_nr);
      Serial.print(" (");
      Serial.print(SKETCH_ID_CODE);
      Serial.println(")");

      //  Read the first line of the HTTP request header
      while (client.connected()) {
        if (client.available()) {
          Serial.println("New HTTP Request");

          //  Read the first line of HTTP request
          HTTP_req = client.readStringUntil('\n');
          Serial.print("<< ");

          //  Print the HTTP request to Serial Monitor
          Serial.println(HTTP_req);
          break;
        }
      }

      //  Read the remaining lines of HTTP request header
      while (client.connected()) {
        if (client.available()) {
          String HTTP_header = client.readStringUntil('\n');  // read the header line of HTTP request

          if (HTTP_header.equals("\r"))  // the end of HTTP request
            break;
          /*
          // Print the HTTP request to Serial Monitor
          Serial.print("<< ");
          Serial.println(HTTP_header);
          */
        }
      }

      // ROUTING
      // This sketch supports the following:
      // - GET /
      // - GET /home
      // - GET /index
      // - GET /index.html
      // - GET /environment
      // - GET /environment.html
      // - GET /switches
      // - GET /switches.html
      // - GET /potentiometer
      // - GET /potentiometer.html
      // - GET /light
      // - GET /light.html
      // - GET /imu
      // - GET /imu.html

      page_id = 0;

      if (HTTP_req.indexOf("GET") == 0) {
        //  Check if request method is GET
        if (HTTP_req.indexOf("GET / ") > -1 || HTTP_req.indexOf("GET /index ") > -1 || HTTP_req.indexOf("GET /index.html ") > -1) {
          Serial.println("Home Page");
          page_id = PAGE_HOME;
        } else if (HTTP_req.indexOf("GET /environment ") > -1 || HTTP_req.indexOf("GET /environment.html ") > -1) {
        }

            if (USING_SCD40_CO2) {
              Serial.println("Getting temperature and humidity readings");

              sensors = get_scd40 (sensors, &scd40);
            
              //  Replace the markers by real values
              html.replace("DATESTAMP_MARKER", date_time);
            
              html.replace("SCD40_FAHRENHEIT_MARKER", String(sensors.scd40.fahrenheit));
              html.replace("SCD40_CELSIUS_MARKER", String(sensors.scd40.celsius));
              html.replace("SCD40_HUMIDITY_MARKER", String(sensors.scd40.humidity));
              html.replace("SCD40_CO2_MARKER", String(sensors.scd40.CO2));
            } else {
              html.replace("SCD40_FAHRENHEIT_MARKER", "Not used");
              html.replace("SCD40_CELSIUS_MARKER", "Not used");
              html.replace("SCD40_HUMIDITY_MARKER", "Not used");
              html.replace("SCD40_CO2_MARKER", "Not used");
            }

            if (USING_ENS160_MOX) {
              sensors = get_ens160(sensors, &ens160);

              if (sensors.ens160.valid) {
                html.replace("ENS160_AQI_MARKER", String(sensors.ens160.aqi));
                html.replace("ENS160_TVOC_MARKER", String(sensors.ens160.tvoc));
                html.replace("ENS160_ECO2_MARKER", String(sensors.ens160.eCO2));               
                html.replace("ENS160_HP0_MARKER", String(sensors.ens160.hp0));
                html.replace("ENS160_HP1_MARKER", String(sensors.ens160.hp1));
                html.replace("ENS160_HP2_MARKER", String(sensors.ens160.hp2));               
                html.replace("ENS160_HP3_MARKER", String(sensors.ens160.hp3));               
              } else {
                html.replace("ENS160_AQI_MARKER", "Not used");
                html.replace("ENS160_TVOC_MARKER", "Not used");
                html.replace("ENS160_ECO2_MARKER", "Not used");                              
                html.replace("ENS160_HP0_MARKER", "Not used");
                html.replace("ENS160_HP1_MARKER", "Not used");
                html.replace("ENS160_HP2_MARKER", "Not used");               
                html.replace("ENS160_HP3_MARKER", "Not used");               
              }
            }

            break;
          case PAGE_SWITCHES:
            html = PAGE_HTML[PAGE_SWITCHES];  //String(HTML_CONTENT_SWITCHES);
            date_time = timestamp(SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, NO_SECONDS);
            read_switches();
            show_switches();
            set_leds();
            temp_html = switch_format_html();

            //  Replace the markers by real values
            html.replace("SEQUENCE_COUNT_MARKER", String(sequence_nr));
            html.replace("SWITCHES_TEXT_MARKER", temp_html);
            html.replace("DATESTAMP_MARKER", date_time);
            html.replace("REQUEST_COUNT_MARKER", String(request_count));
            break;
          case PAGE_POTENTIOMETER:
            html = PAGE_HTML[PAGE_POTENTIOMETER]; //String(HTML_CONTENT_POTENTIOMETER);
            date_time = timestamp(SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, NO_SECONDS);

            //  Read the potentiomenter
            potentiometer_reading = analogRead(ANALOG_POT_PIN);
            potentiometer_voltage = (MAXIMUM_ANALOG_VOLTAGE / ANALOG_RESOLUTION) * potentiometer_reading; // * 1000.0;

            //  Print the potentiomenter reading and voltage
            Serial.print("Potentiometer: Reading = ");
            Serial.print(potentiometer_reading);
            Serial.print(", Voltage = ");

            Serial.print(potentiometer_voltage, 2);

            if (potentiometer_voltage < 1.0) {
              potentiometer_units = " mV";
            } else {
              potentiometer_units = " V";
            }

            Serial.println(potentiometer_units);

            //  Replace the markers by real values
            html.replace("SEQUENCE_COUNT_MARKER", String(sequence_nr));
            html.replace("POTENTIOMETER_READING_MARKER", String(potentiometer_reading));
            html.replace("POTENTIOMETER_VOLTAGE_MARKER", String(potentiometer_voltage));
            html.replace("POTENTIOMETER_UNITS_MARKER", potentiometer_units);
            html.replace("DATESTAMP_MARKER", date_time);
            break;
          case PAGE_LIGHT:
            html = PAGE_HTML[PAGE_LIGHT]; //String(HTML_CONTENT_LIGHT);
            date_time = timestamp(SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, NO_SECONDS);
            lux = read_veml7700(&veml);

            //  Replace the markers by real values
            html.replace("SEQUENCE_COUNT_MARKER", String(sequence_nr));
            html.replace("LIGHT_VALUE_MARKER", String(lux));
            html.replace("DATESTAMP_MARKER", date_time);

            break;
          case PAGE_IMU:
            html = PAGE_HTML[PAGE_IMU]; //String(HTML_CONTENT_IMU);
            date_time = timestamp(SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, NO_SECONDS);
            sensors = get_lsm6dsox(sensors, &lis3);

            //  Replace the markers by real values
            temp_html = imu_format_xyz_html(sensors.lsm6dsox.accelerometer);
            html.replace("IMU_ACCELEROMETER_MARKER", temp_html);
            temp_html = imu_format_xyz_html(sensors.lsm6dsox.gyroscope);
            html.replace("IMU_GYROSCOPE_MARKER", temp_html);
            temp_html = imu_format_xyz_html(sensors.lsm6dsox.magnetometer);
            html.replace("IMU_MAGNETOMETER_MARKER", temp_html);
            html.replace("DATESTAMP_MARKER", date_time);
            html.replace("SEQUENCE_COUNT_MARKER", String(sequence_nr));
            break;
          case PAGE_ERROR_404:
            html = String(HTML_CONTENT_404);
            date_time = timestamp(SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, NO_SECONDS);

            //  Replace the markers by real values
            html.replace("SEQUENCE_COUNTER_MARKER", String(sequence_nr));
            html.replace("DATESTAMP_MARKER", date_time);
            break;
          case PAGE_ERROR_405:
            html = String(HTML_CONTENT_405);
            date_time = timestamp(SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, NO_SECONDS);

            //  Replace the markers by real values
            html.replace("DATESTAMP_MARKER", date_time);
            html.replace("SEQUENCE_COUNT_MARKER", String(sequence_nr));
            break;
        }
      } else if (HTTP_req.indexOf("PUT") == 0) {
        //  Check if request method is PUT
        Serial.println("HTTP PUT Request");

        if (HTTP_req.indexOf("PUT /status ") > -1 || HTTP_req.indexOf("PUT /status.html ") > -1) {
          //  Status of the home, a device, etc.
          Serial.println("PUT STATUS");
        } else if (HTTP_req.indexOf("PUT /change ") > -1 || HTTP_req.indexOf("PUT /change.html ") > -1) {
          Serial.println("PUT CHANGE");
          //  Change lights, a device, etc.
        }
      } else {  
        //  Invalid Request
        //  405 Method Not Allowed
        Serial.println("405 Method Not Allowed");
        html = String(HTML_CONTENT_405);
        page_id = PAGE_ERROR_405;
      }

      if (send_page) {
        //  Send the page to the browser
        client.println(html);
        client.flush();
      }

      //  Close the connection
      Serial.println("Closing connection");
      Serial.println();
      client.stop();
    }

    //  Check to be sure the RTC has not lost power
    if (rtc.lostPower()) {
      Serial.println("RTC lost power, let's set the time!");
      // When time needs to be set on a new device, or after a power loss, the
      // following line sets the RTC to the date & time this sketch was compiled
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      // This line sets the RTC with an explicit date & time, for example to set
      // January 21, 2014 at 3am you would call:
      // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    }

    //  Check to be sure we still have a WiFi connection
    wifi_status = WiFi.status();

    if (wifi_status != WL_CONNECTED) {
      Serial.println("I lost my WiFi connection - attempting to reconnect!");
      digitalWrite(LED_RASPI_CONNECT_PIN, LOW);
      digitalWrite(LED_RASPI_WIFI_PIN, HIGH);

      connected = connect_to_wifi(ssid, passwd);

      if (connected) {
        digitalWrite(LED_RASPI_CONNECT_PIN, HIGH);
        digitalWrite(LED_RASPI_WIFI_PIN, LOW);
      } else {
        halt("Unable to reconnect to the network", true, ssid);
      }
    }

    //  Give the web browser time to receive the data
    delay(WEB_SERVER_DELAY_MS);
  }
}
