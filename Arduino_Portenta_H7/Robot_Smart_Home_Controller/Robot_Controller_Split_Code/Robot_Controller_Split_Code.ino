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
#include  <WiFi.h>

#include  "Secrets.h"
#include  "Robot_Controller.h"
#include  "Web_Server_Control.h"
#include  "Arduino_HAT_Carrier_C33.h"
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

#include <Adafruit_LIS3MDL.h>
Adafruit_LIS3MDL lis3;

#include "Adafruit_SHT4x.h"
Adafruit_SHT4x sht4 = Adafruit_SHT4x();

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
void halt ( char* message, bool wifi_halt=false, char *wifi_ssid=ssid) {
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
  String result_str = "";

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

        //  Test the character. If position < 0, character is not a digit
        is_number = (position > 0);

        if (!is_number) {
          //  Found a non-numeric character so break the loop
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

  return result_str;
}

/*
  Create a date and time stamp of the current time

  Parameters:
*/
String timestamp (bool show_full=SHOW_FULL_DATE, bool hours_24=SHOW_12_HOURS, bool long_date=SHOW_LONG_DATE, bool show_seconds=SHOW_SECONDS) {
  String date_time = "", date_str = "D*", time_str = "T*";
  String year_str = "Y*", month_str = "M*", day_str = "D*";
  String hours_str, min_sec_str, week_day_str = "*";
  String am_pm = " AM", day_suffix = "**";
  uint8_t position, str_len, week_day_nr, day, suffix, month, hours;

  date_time = left_pad(String(rtc.now().month()), 2) + "/" + left_pad(String(rtc.now().day()), 2) + "/" left_pad(String(rtc.now().year()), 4) + " at " + left_pad(String(rct.now().hour()), 2) + ":" +left_pad(String(rtc.now().minute()), 2) + ":" left_pad(String(rtc.now().second()), 2);
  //  Retrieve the date and time from the RTC
  //  String is in the format yyy-mm-ddThh:mm:ss
  //RTC.getTime(current_time); 
  str_len = date_time.length();
  position = date_time.indexOf("T");

  if (show_full) {
    //Serial.println("Showing full date");

    time_str = date_time.substring(position + 1, str_len);
    min_sec_str = date_time.substring(position + 3, str_len);
    position = date_time.indexOf("T");
    date_str = date_time.substring(0, position);

    str_len = date_str.length();
    year_str = date_str.substring(0, 4);
    month_str = date_str.substring(5, 7);
    month = month_str.toInt();
    day_str = date_str.substring(8, str_len);
    day = day_str.toInt();

    week_day_nr = rtc.now().dayOfTheWeek();
/*
    Serial.print("(1) time_str = '");
    Serial.print(time_str);
    Serial.println("'");
*/
    week_day_str = week_days[week_day_nr];

    day_str = String(rtc.now().day());
    str_len = day_str.length();

    if (day < 10) {
      suffix = day;
    } else if (str_len == 2) {
      suffix = day_str.substring(str_len - 1, str_len - 1).toInt();
    }

    if (long_date) {
      //Serial.println("Showing long date");
/*
      Serial.print("(2) day_str = '");
      Serial.print(day_str);
      Serial.print("', week_day_str = '");
      Serial.print(week_day_str);
      Serial.println("'");
*/
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
      day_str = day_str + day_suffix;
    } else {
      day_str = left_pad(day_str);
    }

    str_len = time_str.length();
    hours_str = time_str.substring(0, 2);
    hours = hours_str.toInt();
    min_sec_str = time_str.substring(3, str_len);
/*
    Serial.print("(3) day_str = '");
    Serial.print(day_str);
    Serial.print("', min_sec_str = '");
    Serial.print(min_sec_str);
    Serial.println("'");
*/    
    if (hours_24) {
      //Serial.println("Showing 24 hour format time");

      time_str = left_pad(String(hours)) + ":" + min_sec_str;
    } else {
      if (hours == 0) {
        hours = 12;
        am_pm = " AM";
      } else {
        if (hours > 12) {
          hours = hours - 12;
          am_pm = " PM";
        } else {
          am_pm = " AM";
        }
      }

      time_str = String(hours) + ":" + min_sec_str + am_pm;
    }
/*
    Serial.print("(4) time_str = '");
    Serial.print(time_str);
    Serial.print("', hours = ");
    Serial.println(hours);
*/
    if (!show_seconds) {;
      str_len = time_str.length();
      time_str = time_str.substring(0, str_len - 3);
/*
      Serial.println("Showing seconds");
      Serial.print("(5) time_str = '");
      Serial.print(time_str);
      Serial.println("'");
*/
    }
/*
    Serial.print("(6) min_sec_str = '");
    Serial.print(min_sec_str);
    Serial.println("'");
*/
    if (long_date) {
      //Serial.println("Showing long date 2");

      date_str = week_day_str + ", " + long_months[month - 1] + " " + day_str + ", " + year_str;
    } else {
      date_str = left_pad(month_str) + "/" + left_pad(day_str) + "/" + year_str;
    }
/*
    Serial.print("(7) date_str = '");
    Serial.print(date_str);
    Serial.print("', min_sec_str = ");
    Serial.print(min_sec_str);
    Serial.println("'");
*/
    date_time = date_str + " at " + time_str;
  } else {    
    date_time = date_str + " at " + time_str;
  }
/*
  Serial.print("(8) date_time = '");
  Serial.print(date_time);
  Serial.print("', time_str = '");
  Serial.print(time_str);
  Serial.println("'");
*/
  return date_time;
}

/*
  Convert the Celsius temperature to Fahrenheit

  Returns: (float) temperature in fahrenheit
*/
float to_fahrenheit (float celsius) {
  return celsius * 1.8 + 32;
}

/*
  Initialize the SHT4x temperature and humidity `
*/
Adafruit_SHT4x init_sht4x (Adafruit_SHT4x *sht, System_Sensor_Status *sen_stat) {
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
  sen_stat->lis3mdl_status = lis3->begin_I2C();

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
    
    //lis3->setRange(LIS3MDL_RANGE_4_GAUSS);

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

    lis3->setIntThreshold(500);

    lis3->configInterrupt(false, false, true, // enable z axis
                            true, // polarity
                            false, // don't latch
                            true); // enabled!
*/
  } else {
    halt("Unable to find and initialize the LIS3MDL magnetometer!");
  }

  return *lis3;
}

/*
  Initialize the LSM6DSOX IMU
*/
/*
void init_lsm6dsox (Sytem_Sensor_Status *sen_stat) {
  //  Initialie the IMU
  lsm6dsox = IMU.begin();
  if (IMU.begin) {}
    sens_stat->lsm6dsox_status = true;

  if (sens_stat->lsm6dsox_status) {
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
    //sx->setAccelRange(LSM6DS_ACCEL_RANGE_2_G);

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
    IMU.setAccelDataRate(LSM6DS_RATE_12_5_HZ);

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
    IMU.setGyroRange(LSM6DS_GYRO_RANGE_250_DPS );

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
    IMU.setGyroDataRate(LSM6DS_RATE_12_5_HZ);
  } else {
    halt("Failed to initialize the LSM6DSOX IMU!");
  }
}

/*
Adafruit_VEML7700 init_veml7700 (Adafruit_VEML7700 *vm) {
  if (vm->begin()) {
    Serial.println("Found a VEML7700 Lux sensor");
    Serial.println("Settings used for reading:");
    Serial.print(F("Gain: "));

    switch (vm->getGain()) {
      case GAIN_1:
        Serial.println("1");
        break;
      case VEML7700_GAIN_2:
        Serial.println("2");
        break;
      case VEML7700_GAIN_1_4:
        Serial.println("1/4");
        break;
      case VEML7700_GAIN_1_8:
        Serial.println("1/8");
        break;
    }

    Serial.print(F("Integration Time (ms): "));

    switch (vm->getIntegrationTime()) {
      case VEML7700_IT_25MS:
        Serial.println("25");
        break;
      case VEML7700_IT_50MS:
        Serial.println("50");
        break;
      case VEML7700_IT_100MS:
        Serial.println("100");
        break;
      case VEML7700_IT_200MS:
        Serial.println("200");
        break;
      case VEML7700_IT_400MS:
        Serial.println("400");
        break;
      case VEML7700_IT_800MS:
        Serial.println("800");
        break;
    }
  } else {
    Serial.println("Unable to find the VEML7700 sensor!");

    halt();
  }

  return *vm;
}
*/

/*    
  Print out the connection status:
*/
void print_wifi_status(void) {
  //  Print your board's IP address:
  Serial.print("Connected! IP Address is ");
  //Serial.println(WiFi.localIP());

  //  Print the received signal strength:
  Serial.print("Signal strength (RSSI):");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
}

bool connect_to_wifi (char *ssid, char *passwd, uint8_t connection_timeout_ms=CONNECTION_TIMEOUT_MS, uint8_t  max_nr_connects=MAX_NR_CONNECTS) {
  uint8_t connect_count = 0;
  IPAddress ip_addr;

  connected = true;       //  Assume we will connect for now

  //  Attempt to connect to WiFi network:
  while (wifi_status != WL_NO_SHIELD and (connect_count < max_nr_connects)) {
    connect_count += 1;

    if (connect_count > 1) {
      blink_led_raspi(LED_RASPI_WIFI_PIN, 500, connect_count);
    }

    Serial.print("Attempt #");
    Serial.print(connect_count);
    Serial.print(" connecting to the '");
    Serial.print(ssid);
    Serial.println("' network");

    /*
        Connect to WPA/WPA2 network.
        Change this line if you are using open or WEP network:
    */
    wifi_status = WiFi.begin(ssid, passwd);

    //  Wait for connection:
    delay(connection_timeout_ms);
  }

  if (wifi_status == WL_CONNECTED) {
    ip_addr = WiFi.localIP();

    if (ip_addr == "0.0.0.0") {
      Serial.println("IP Address Invalid");
      connected = false;
    } else {
      //  Print connection status
      print_wifi_status();
    }
  } else {
    connected = false;
  }

  if (!connected) {
    halt("Unable to connect to the network", true, ssid);
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
      //Serial.print("index = ");
      //Serial.print(index);
      //Serial.print(", switch state = ");
      //Serial.println(SWITCH_READINGS[index]);

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
/*
  if (nr_of_switches == nr_of_leds) {
    for (index=0; index < nr_of_switches; index++) {
      //Serial.print("index = ");
      //Serial.print(index);
      //Serial.print(", switch state = ");
      //Serial.println(SWITCH_READINGS[index]);

      if (SWITCH_READINGS[index]) {
        digitalWrite(LED_PINS[index], LOW);
      } else {
        digitalWrite(LED_PINS[index], HIGH);
      }
    }
  } else {
    result = false;
  }
*/
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
Environment_Data check_data (Environment_Data curr_data) {
  Environment_Data result;
  Three_Axis filler;

  //  If we have current data, copy it
  if (curr_data.valid) {
    //  Save existing data
    result = curr_data;
  } else {
    filler.x = 0.0;
    filler.y = 0.0;
    filler.z = 0.0;

    //  Initialize data structure
    result.valid = false;

    result.celsius = 0.0;
    result.fahrenheit = 0.0;
    result.humidity = 0.0;

    result.accelerometer = filler;
    result.gyroscope = filler;
    result.temperature = 0.0;
  
    result.magnetometer = filler;
  }

  return result;
}

/*
  Get readings from the LIS3MDL Magnetometer and put them in the environment
    data structure.
*/
Environment_Data get_lis3mdl (Environment_Data curr_data, Adafruit_LIS3MDL*lis3) {
  Environment_Data sensors;
  sensors_event_t event;
  sensors = check_data(curr_data);

  lis3->getEvent(&event);
  sensors.magnetometer.x = event.magnetic.x;
  sensors.magnetometer.y = event.magnetic.y;
  sensors.magnetometer.z = event.magnetic.z;

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
Environment_Data get_lsm6dsox (Environment_Data curr_data) {
  Environment_Data sensors;
  float x, y, z;
  float temperature = 0.0;

  sensors = check_data(curr_data);

  if (IMU.accelerationAvailable()) {

    IMU.readAcceleration(x, y, z);

    sensors.accelerometer.x = x;
    sensors.accelerometer.y = y;
    sensors.accelerometer.z = z;

    Serial.print("Accelerometer sample rate = ");
    Serial.print(IMU.accelerationSampleRate());
    Serial.println(" Hz");
  }

  if (IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(x, y, z);

    sensors.gyroscope.x = x;
    sensors.gyroscope.y = y;
    sensors.gyroscope.z = z;

    Serial.print("Gyroscope sample rate = ");
    Serial.print(IMU.gyroscopeSampleRate());
    Serial.println(" Hz");
  }

  if (IMU.temperatureAvailable()) {
    IMU.readTemperatureFloat(temperature);

    sensors.temperature = temperature;
  }

  return sensors;
}

/*
  Get temperature and humidity readings from the SHT45 and put them i sensor
    and put them into the environment data structure.
*/
Environment_Data get_temperature(Environment_Data curr_data, Adafruit_SHT4x *sht) {
  Environment_Data sensors;
  sensors_event_t rel_humidity, temperature;
  uint32_t timestamp = millis();
  float celsius, humidity;

  sensors = check_data(curr_data);

  sht->getEvent(&rel_humidity, &temperature);// populate temp and humidity objects with fresh data
  celsius = temperature.temperature;

  sensors.celsius = celsius;
  sensors.fahrenheit = to_fahrenheit(celsius);
  sensors.humidity = rel_humidity.relative_humidity;

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

void setup (void) {
  uint8_t note_nr;
  DateTime current_time;
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
    init_switches_raspi();

    //  Set analog resolutionto 12 bits
    analogReadResolution(12);

    //  Initialize the analog pins
    //init_resistors();

    //  Initialize the sensors data structure
    sensors.valid = false;
    sensors = check_data(sensors);

    //  Initialize the SHT4x Temperature and Humidity sensors
    if (USING_SHT45_TEMP) {
       sht4 = init_sht4x(&sht4, &sensor_status);
    }

    //  Initialize the LSM6DSOX IMU
    if (USING_LSM6DSOX_LIS3MDL_IMU) {  
      if (IMU.begin()) {
        Serial.println("Found an LSM6DSOX IMU");
      } else {
        halt("Failed to initialize the LSM6DSOX IMU!");
      }
    }

    //  Initialize the LIS3MDL Magnetometer
    if (USING_LIS3MDL_MAG) {
      lis3 = init_lis3mdl(sensors, &lis3, &sensor_status);
    }

    //  Initialize the VEML7700 Light (Lux) sensor
    if (USING_VEML7700_LUX) {
      if (veml.begin()) {
        Serial.println("Found a VEML7700 Lux sensor");
        Serial.println("Settings used for reading:");
        Serial.print(F("Gain: "));

        switch (veml.getGain()) {
          case VEML7700_GAIN_1:
            Serial.println("1");
            break;
          case VEML7700_GAIN_2:
            Serial.println("2");
            break;
          case VEML7700_GAIN_1_4:
            Serial.println("1/4");
            break;
          case VEML7700_GAIN_1_8:
            Serial.println("1/8");
            break;
        }

        Serial.print(F("Integration Time (ms): "));

        switch (veml.getIntegrationTime()) {
          case VEML7700_IT_25MS:
            Serial.println("25");
            break;
          case VEML7700_IT_50MS:
            Serial.println("50");
            break;
          case VEML7700_IT_100MS:
            Serial.println("100");
            break;
          case VEML7700_IT_200MS:
            Serial.println("200");
            break;
          case VEML7700_IT_400MS:
            Serial.println("400");
            break;
          case VEML7700_IT_800MS:
            Serial.println("800");
            break;
        }
      } else {
        halt("Unable to find the VEML7700 sensor!");
      }
    }

    //  Start the time client
    Serial.println("Starting the time client");
    time_client.begin();
    time_client.update();

    //  Start the Real Time Clock and set the current time
    Serial.println("Starting the RTC");

    if (! rtc.begin()) {
      Serial.println("Could not find the DS3231 RTC");
      Serial.flush();
      while (1);
      delay(10);
    else {
      current_time = rtc.now()
    }

    Serial.println();
    Serial.print("Today is ");
    Serial.println(timestamp(SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, SHOW_SECONDS));
  
    //  Start the web server
    Serial.println();
    Serial.println("Starting the web server");
    server.begin(WEB_SERVER_PORT);
    Serial.println();
  } else {
    halt("Unable to connect to network", true, ssid);
  }

  //  For testing setup()
  //halt("END OF SETUP HALT");

  Serial.println("Initization complete!");
  delay(10000);
}

void loop (void) {
  DateTime now = rtc.now();
  WiFiClient client;
  bool send_page = true;
  float lux, potentiometer_voltage;
  uint16_t potentiometer_reading;
  uint8_t index = 0;
  int page_id = 0, index_pos = 0, start_pos = 0;
  unsigned long start_millis, end_millis;
  String HTTP_req, html = "", date_time = "", temp_html = "", led_html = "";
  String potentiometer_units;

  //  Heartbeat
  blink_rgb(blue);

  Serial.println();
  Serial.print("Loop #");
  looper++;
  Serial.println(looper);

  read_switches();
  show_switches();
  set_leds();

/*
  //  Read the potentiomenter
  potentiometer_reading = analogRead(ANALOG_POT_PIN);
  potentiometer_voltage = (MAXIMUM_ANALOG_VOLTAGE / ANALOG_RESOLUTION) * potentiometer_reading; // * 1000.0;

  //  Print the potentiomenter reading and voltage
  Serial.print("Potentiometer: Reading = ");
  Serial.print(potentiometer_reading);
  Serial.print(", Voltage = ");

  if (potentiometer_voltage < 1.0) {
    Serial.print(potentiometer_voltage * 1000.0, 2);
    Serial.println(" mV");
  } else {
    Serial.print(potentiometer_voltage, 2);
    Serial.println(" V");
  }
*/

  client = server.available();

  //  Listen for incoming clients
  if (client) {
    HTTP_req = "";

    blink_rgb(magenta);

    request_count += 1;
    start_millis = millis();

    Serial.print("***** Request #");
    Serial.print(request_count);
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
          date_time = timestamp(SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, SHOW_SECONDS);
          Serial.println(PAGE_HOME_NAME);

          //  Replace the markers by real values
          //html.replace("PAGE_HOME_TITLE_MARKER", PAGE_HOME_TITLE);
          //html.replace("PAGE_HOME_NAME_MARKER", PAGE_HOME_NAME);
          //html.replace("SKETCH_CODE_MARKER", SKETCH_ID_CODE);
          //html.replace("ROBOT_PAGE_NAME_MARKER", ROBOT_DEVICE_NAME);
          //html.replace("ROBOT_NAME_MARKER", ROBOT_DEVICE_NAME);
          html.replace("REQUEST_COUNT_MARKER", String(request_count));
          html.replace("DATESTAMP_MARKER", date_time);
          break;
        case PAGE_ENVIRONMENT:
          html =  PAGE_HTML[PAGE_ENVIRONMENT]; //String(HTML_CONTENT_ENVIRONMENT);
          date_time = timestamp(SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, SHOW_SECONDS);
          Serial.println(PAGE_ENVIRONMENT_NAME);

          if (USING_SHT45_TEMP) {
            Serial.println("Getting temperature and humidity readings");

            sensors = get_temperature(sensors, &sht4);

            Serial.println("Sending sensor readings");
          
            //  Replace the markers by real values
            //html.replace("PAGE_ENVIRONMENT_TITLE_MARKER", PAGE_ENVIRONMENT_NAME);
            //html.replace("PAGE_ENVIRONMENT_NAME_MARKER", PAGE_ENVIRONMENT_NAME);
            html.replace("DATESTAMP_MARKER", date_time);
            html.replace("REQUEST_COUNTER_MARKER", String(request_count));
            html.replace("FAHRENHEIT_MARKER", String(sensors.fahrenheit));
            html.replace("CELSIUS_MARKER", String(sensors.celsius));
            html.replace("HUMIDITY_MARKER", String(sensors.humidity));
          }
          break;
        case PAGE_SWITCHES:
          html = PAGE_HTML[PAGE_SWITCHES];  //String(HTML_CONTENT_SWITCHES);
          date_time = timestamp(SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, SHOW_SECONDS);
          read_switches();
          show_switches();
          set_leds();
          temp_html = switch_format_html();

          //  Replace the markers by real values
          //html.replace("PAGE_SWITCHES_TITLE_MARKER", PAGE_SWITCHES_TITLE);
          //html.replace("PAGE_SWITCHES_NAME_MARKER", PAGE_SWITCHES_NAME);
          html.replace("SWITCHES_TEXT_MARKER", temp_html);
          html.replace("DATESTAMP_MARKER", date_time);
          html.replace("REQUEST_COUNTER_MARKER", String(request_count));
          break;
        case PAGE_POTENTIOMETER:
          html = PAGE_HTML[PAGE_POTENTIOMETER]; //String(HTML_CONTENT_POTENTIOMETER);
          date_time = timestamp(SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, SHOW_SECONDS);
          //  Read the potentiomenter
          potentiometer_reading = analogRead(ANALOG_POT_PIN);
          potentiometer_voltage = (MAXIMUM_ANALOG_VOLTAGE / ANALOG_RESOLUTION) * potentiometer_reading; // * 1000.0;

          if (potentiometer_voltage < 1.0) {
            potentiometer_voltage = potentiometer_voltage * 1000;
            potentiometer_units = " mV";
          } else {
            potentiometer_units = " V";
          }

          //  Replace the markers by real values
          //html.replace("PAGE_POTENTIOMETER_TITLE_MARKER", PAGE_POTENTIOMETER_TITLE);
          //html.replace("PAGE_POTENTIOMETER_NAME_MARKER", PAGE_POTENTIOMETER_NAME);
          html.replace("POTENTIOMETER_READING_MARKER", String(potentiometer_reading));
          html.replace("POTENTIOMETER_VOLTAGE_MARKER", String(potentiometer_voltage));
          html.replace("POTENTIOMETER_UNITS_MARKER", potentiometer_units);
          html.replace("DATESTAMP_MARKER", date_time);
          html.replace("REQUEST_COUNTER_MARKER", String(request_count));
          break;
        case PAGE_LIGHT:
          html = PAGE_HTML[PAGE_LIGHT]; //String(HTML_CONTENT_LIGHT);
          date_time = timestamp(SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, SHOW_SECONDS);
          lux = read_veml7700(&veml);

          //  Replace the markers by real values
          //html.replace("PAGE_LIGHT_TITLE_MARKER", PAGE_LIGHT_TITLE);
          //html.replace("PAGE_LIGHT_NAME_MARKER", PAGE_LIGHT_NAME);
          html.replace("LIGHT_VALUE_MARKER", String(lux));
          html.replace("DATESTAMP_MARKER", date_time);
          html.replace("REQUEST_COUNTER_MARKER", String(request_count));
          break;
        case PAGE_IMU:
          html = PAGE_HTML[PAGE_IMU]; //String(HTML_CONTENT_IMU);
          date_time = timestamp(SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, SHOW_SECONDS);
          sensors = get_lsm6dsox(sensors);

          //  Replace the markers by real values
          //html.replace("PAGE_IMU_TITLE_MARKER", PAGE_IMU_TITLE);
          //html.replace("PAGE_IMU_NAME_MARKER", PAGE_IMU_NAME);
          temp_html = imu_format_xyz_html(sensors.accelerometer);
          html.replace("IMU_ACCELEROMETER_MARKER", temp_html);
          temp_html = imu_format_xyz_html(sensors.gyroscope);
          html.replace("IMU_GYROSCOPE_MARKER", temp_html);
          temp_html = imu_format_xyz_html(sensors.magnetometer);
          html.replace("IMU_MAGNETOMETER_MARKER", temp_html);
          html.replace("DATESTAMP_MARKER", date_time);
          html.replace("REQUEST_COUNTER_MARKER", String(request_count));
          break;
        case PAGE_ERROR_404:
          html = String(HTML_CONTENT_404);
          date_time = timestamp(SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, SHOW_SECONDS);

          //  Replace the markers by real values
          html.replace("REQUEST_COUNTER_MARKER", String(request_count));
          html.replace("DATESTAMP_MARKER", date_time);
          break;
        case PAGE_ERROR_405:
          html = String(HTML_CONTENT_405);
          date_time = timestamp(SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, SHOW_SECONDS);

          //  Replace the markers by real values
          html.replace("DATESTAMP_MARKER", date_time);
          html.replace("REQUEST_COUNTER_MARKER", String(request_count));
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

  //  Check to be sure we still have a WiFi connection
  wifi_status = WiFi.status();

  if (wifi_status != WL_CONNECTED) {
    Serial.println("Lost WiFi connection - attempting to reconnect!");
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
