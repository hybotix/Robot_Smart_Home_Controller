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

#include  "Robot_Controller.h"
#include  "Web_Server_Control.h"
#include  "Arduino_HAT_Carrier_C33.h"
#include  "Secrets.h"

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
#include <SparkFun_I2C_Mux_Arduino_Library.h>
QWIICMUX mux;

//  DS3231 Real Time Clock - Adafruit Fork
#include "DS3232.h"
DS3232 rtc;

//Include the NTP library
#include <NTPClient.h>
WiFiUDP Udp; // A UDP instance to let us send and receive packets over UDP
NTPClient time_client(Udp);

#include <Adafruit_LIS3MDL.h>
Adafruit_LIS3MDL lis3;

#include "Adafruit_SHT4x.h"
Adafruit_SHT4x sht45 = Adafruit_SHT4x();

#include "Adafruit_VEML7700.h"
Adafruit_VEML7700 veml = Adafruit_VEML7700();

#include "ScioSense_ENS160.h"  // ENS160 library
//  ScioSense_ENS160  ens160(ENS160_I2CADDR_0);
ScioSense_ENS160  ens160(ENS160_I2CADDR_1);

#include <SensirionI2CScd4x.h>
SensirionI2CScd4x scd40;

//  Check I2C device address and correct line below (by default address is 0x29 or 0x28)
//    id, address
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
Adafruit_BNO055 bno055 = Adafruit_BNO055(55, 0x28);

/**********************
  Utility routines
**********************/

void print_uint_16_hex(uint16_t value) {
  Serial.print(value < 4096 ? "0" : "");
  Serial.print(value < 256 ? "0" : "");
  Serial.print(value < 16 ? "0" : "");
  Serial.print(value, HEX);
}

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

void bno_print_event(sensors_event_t* event) {
  //  Dumb values, easy to spot any problem
  double x = -1000000, y = -1000000 , z = -1000000;

  if (event->type == SENSOR_TYPE_ACCELEROMETER) {
    Serial.print("Accelerometer: ");
    x = event->acceleration.x;
    y = event->acceleration.y;
    z = event->acceleration.z;
  } else if (event->type == SENSOR_TYPE_ORIENTATION) {
    Serial.print("Orientation: ");
    x = event->orientation.x;
    y = event->orientation.y;
    z = event->orientation.z;
  } else if (event->type == SENSOR_TYPE_MAGNETIC_FIELD) {
    Serial.print("Magnetometer: ");
    x = event->magnetic.x;
    y = event->magnetic.y;
    z = event->magnetic.z;
  } else if (event->type == SENSOR_TYPE_GYROSCOPE) {
    Serial.print("Gyroscope: ");
    x = event->gyro.x;
    y = event->gyro.y;
    z = event->gyro.z;
  } else if (event->type == SENSOR_TYPE_ROTATION_VECTOR) {
    Serial.print("Rotation: ");
    x = event->gyro.x;
    y = event->gyro.y;
    z = event->gyro.z;
  } else if (event->type == SENSOR_TYPE_LINEAR_ACCELERATION) {
    Serial.print("Linear: ");
    x = event->acceleration.x;
    y = event->acceleration.y;
    z = event->acceleration.z;
  } else if (event->type == SENSOR_TYPE_GRAVITY) {
    Serial.print("Gravity: ");
    x = event->acceleration.x;
    y = event->acceleration.y;
    z = event->acceleration.z;
  } else {
    Serial.print("Unknown: ");
  }

  Serial.print("x = ");
  Serial.print(x);
  Serial.print(", y = ");
  Serial.print(y);
  Serial.print(", z = ");
  Serial.println(z);
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
String timestamp (QWIICMUX *mx, DS3232 *clock, uint8_t port, bool show_full_date=SHOW_FULL_DATE, bool hours_mode=SHOW_24_HOURS, bool long_date=SHOW_LONG_DATE, bool show_seconds=SHOW_SECONDS) {
  String date_time = "", date_str = "D*", time_str = "T*";
  String year_str = "Y*", month_str = "M*", day_str = "D*", week_day_str;
  //String hours_str;
  String am_pm = " AM", day_suffix = "**";
  uint16_t year;
  uint8_t week_day, day, suffix;
  uint8_t month, hours, minutes, seconds;

  set_mux_port(mx, port, MUX_DS3231_NAME);

  clock->read();

  year = clock->year();
  month = clock->month();
  day = clock->day();
  hours = clock->hours();
  minutes = clock->minutes();
  seconds = clock->seconds();
  week_day = clock->weekDay();

  //  Create a standard date and time stamp
  date_time = left_pad(String(year), 4) + "-" + left_pad(String(month), 2) + "-" +
    left_pad(String(day), 2) + "T" + left_pad(String(hours), 2) + ":" +
    left_pad(String(minutes), 2) + ":" + left_pad(String(seconds), 2);

  //  Default time and date strings
  time_str = left_pad(String(hours), 2) + ":" + left_pad(String(minutes), 2);

  if (show_seconds) {
    time_str = time_str + ":" + left_pad(String(seconds), 2);
  }
  
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
      date_str = week_day_str + ", " + long_months[month - 1] + " " + day_str + ", 20" + left_pad(String(year), 2);
    } else {
      date_str = left_pad(String(day), 2) + "/" + left_pad(String(month), 2) + "/20" + left_pad(String(year), 2);
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

float to_fahrenheit (float celsius) {
  return celsius * 1.8 + 32;
}

void set_mux_port (QWIICMUX *mx, uint8_t port, String device_name) {
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

bool init_ds3231 (QWIICMUX *mx, DS3232 *clock, System_Sensor_Status *sen_stat) {
  //  Set the mux port
  set_mux_port(mx, MUX_DS3231_PORT, MUX_DS3231_NAME);

  if (clock->begin() == DS3232_OK) {
    sen_stat->ds3231 = true;
    Serial.println("Found a DS3231 Real Time Clock");
  } else {
    sen_stat->ds3231 = false;
    halt("Could not find a DS3231 Real Time Clock!");
  }

  return sen_stat->ds3231;
}

bool init_veml7700 (QWIICMUX *mx, System_Sensor_Status *sen_stat) {
  set_mux_port(mx, MUX_VEML7700_PORT, MUX_VEML7700_NAME);

  if (veml.begin()) {
    sen_stat->veml7700 = true;

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
    sen_stat->veml7700 = false;
  }

  return sen_stat->veml7700;
}

bool init_bno055 (QWIICMUX *mx, Adafruit_BNO055 *bno, System_Sensor_Status *sen_stat) {
  //  Set the mux port
  set_mux_port(mx, MUX_BNO055_PORT, MUX_BNO055_NAME);

  if (bno->begin()) {
    sen_stat->bno055 = true;
    Serial.println("Found a BNO055 IMU");
  } else {
    Serial.println("There is no BNO055 IMU!");
    sen_stat->bno055 = false;
  }

  return sen_stat->bno055;
}

bool init_ens160 (QWIICMUX *mx, ScioSense_ENS160 *ens, System_Sensor_Status *sen_stat) {
  //  Set the mux port
  set_mux_port(mx, MUX_ENS160_PORT, MUX_ENS160_NAME);

  sen_stat->ens160 = ens->begin();

  if (sen_stat->ens160) {
    Serial.print("Found an ENS160 MOX Sensor, ");

    if (sen_stat->ens160) {
      sen_stat->ens160 = ens->available();

      if (sen_stat->ens160) {
        Serial.print("Revison ");
        Serial.print(ens->getMajorRev());
        Serial.print(".");
        Serial.print(ens->getMinorRev());
        Serial.print(".");
        Serial.print(ens->getBuild());  

        sen_stat->ens160 = ens->setMode(ENS160_OPMODE_STD);

        if (sen_stat->ens160) {
          Serial.println(" in Standard mode ");
        }
      }
    }
  }

  return sen_stat->ens160;
}

/*
  Initialize the SCD-40 CO2, Temperature, and Humidity sensor
*/
uint16_t init_scd40 (QWIICMUX *mx, SensirionI2CScd4x *scd, System_Sensor_Status *sen_stat) {
  String error_message;
  uint16_t error = 0, serial0, serial1, serial2;

  /*
    Error message codes:
      268 - can not get serial number
  */

  //  Set the mux port
  set_mux_port(mx, MUX_SCD40_PORT, MUX_SCD40_NAME);

  scd->begin(Wire);

  error = scd->getSerialNumber(serial0, serial1, serial2);

  if (error) {
    Serial.println("SCD-40: Error trying to get the serial number: Code " + String(error));
  } else {
    Serial.print("Found an SCD-40 with serial number: 0x");
    print_uint_16_hex(serial0);
    print_uint_16_hex(serial1);
    print_uint_16_hex(serial2);
    Serial.println();

    //  Stop potentially previously started measurement
    error = scd->stopPeriodicMeasurement();
      
    if (error) {
      Serial.println("SCD-40: Error trying to stop periodic measurement: Code " + String(error));
    } else {
      //  Start Measurement
      error = scd->startPeriodicMeasurement();

      if (error) {
        Serial.println("SCD-40: Error trying to execute start periodic measurement: Code " + String(error));
      }
    }
  }

  if (error) {
    sen_stat->scd40 = false;
  }

  return error;
}

/*
  Initialize the SHT4x temperature and humidity `
*/
bool init_sht4x (QWIICMUX *mx, Adafruit_SHT4x *sht, System_Sensor_Status *sen_stat) {
  //  Set the mux port
  set_mux_port(mx, MUX_SHT45_PORT, MUX_SHT45_NAME);

  sen_stat->sht45 = sht->begin();

  if (sen_stat->sht45) {
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

  return sen_stat->sht45;
}

/*
  Initialize the SHT45 Temeprature and Humidity sensor
*/
bool init_lis3mdl (QWIICMUX *mx, Adafruit_LIS3MDL *lis3, System_Sensor_Status *sen_stat) {
  //  Set the mux port
  set_mux_port(mx, MUX_LIS3MDL_PORT, MUX_LIS3MDL_NAME);

  // Try to initialize!
  sen_stat->lis3mdl = lis3->begin_I2C();

  if (sen_stat->lis3mdl) {          // hardware I2C mode, can pass in address & alt Wire
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
    Serial.print("Data rate set to: ");

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
      default:
        Serial.println("Invalid value!");
        break;
    }
    
    lis3->setRange(LIS3MDL_RANGE_4_GAUSS);

    Serial.print("Range set to: ");

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
  }

  return sen_stat->lis3mdl;
}

/*
  Initialize the LSM6DSOX IMU
*/
bool init_lsm6dsox (QWIICMUX *mx, System_Sensor_Status *sen_stat) {
  //  Set the mux port
  set_mux_port(mx, MUX_LSM6DSOX_9DOF_PORT, MUX_LSM6DSOX_9DOF_NAME);

  //  Initialize the IMU
  if (IMU.begin()) {
    Serial.println("Found an LSM6DSOX IMU!");
    sen_stat->lsm6dsox = true;

    Serial.print("Accelerometer sample rate = ");
    Serial.print(IMU.accelerationSampleRate());
    Serial.println(" Hz");

    Serial.print("Gyroscope sample rate = ");
    Serial.print(IMU.gyroscopeSampleRate());
    Serial.println(" Hz");
  } else {
    sen_stat->lsm6dsox = false;
  }

  return sen_stat->lsm6dsox;
}

/*    
  Print out the connection status:
*/
void print_wifi_status(void) {
  //  Print your board's IP address:
  Serial.print("Connected! IP Address is ");
  Serial.println(WiFi.localIP());

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
  while (wifi_status != WL_CONNECTED and (connect_count < max_nr_connects)) {
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
Environment_Data check_data (QWIICMUX *mx, Environment_Data curr_data) {
  Environment_Data result;
  Three_Axis filler;

  //  If we have current data, copy it
  if (curr_data.valid) {
    //  Save existing data
    result = curr_data;
  } else {
    //  Initialize the Environment data structure
    curr_data.valid = false;

    filler.x = 0.0;
    filler.y = 0.0;
    filler.z = 0.0;

    result.sht45.fahrenheit = 0.0;
    result.sht45.celsius = 0.0;
    result.sht45.humidity = 0.0;

    //  Initialize data structure
    result.ens160.valid = false;
    result.ens160.aqi = 0;
    result.ens160.tvoc = 0;
    result.ens160.eCO2 = 0;
    result.ens160.hp0 = 0;
    result.ens160.hp1 = 0;
    result.ens160.hp2 = 0;
    result.ens160.hp3 = 0;

    result.scd40.valid = false;
    result.scd40.celsius = 0.0;
    result.scd40.fahrenheit = 0.0;
    result.scd40.humidity = 0.0;
    result.scd40.CO2 = 0;

    result.lsm6dsox.accelerometer = filler;
    result.lsm6dsox.gyroscope = filler;
    result.lsm6dsox.magnetometer = filler;
    result.lsm6dsox.fahrenheit = 0.0;
    result.lsm6dsox.celsius = 0.0;

    if (USING_BNO055) {
      //  Set the mux port
      set_mux_port(mx, MUX_BNO055_PORT, MUX_BNO055_NAME);

      //  Can not call get_bno055() because it also calls this procedure
      //  Initialize data for the BNO055 IMU    
      bno055.getCalibration(&sensors.bno055.system_cal, &sensors.bno055.gyroscope_cal, &sensors.bno055.accelerometer_cal, &sensors.bno055.magnetometer_cal);
      bno055.getEvent(&sensors.bno055.orientation_data, Adafruit_BNO055::VECTOR_EULER);
      bno055.getEvent(&sensors.bno055.angular_velocity_data, Adafruit_BNO055::VECTOR_GYROSCOPE);
      bno055.getEvent(&sensors.bno055.linear_acceleration_data, Adafruit_BNO055::VECTOR_LINEARACCEL);
      bno055.getEvent(&sensors.bno055.magnetometer_data, Adafruit_BNO055::VECTOR_MAGNETOMETER);
      bno055.getEvent(&sensors.bno055.accelerometer_data, Adafruit_BNO055::VECTOR_ACCELEROMETER);
      bno055.getEvent(&sensors.bno055.gravity_data, Adafruit_BNO055::VECTOR_GRAVITY);

      sensors.bno055.x_pos = sensors.bno055.x_pos + ACCEL_POS_TRANSITION * sensors.bno055.linear_acceleration_data.acceleration.x;
      sensors.bno055.y_pos = sensors.bno055.y_pos + ACCEL_POS_TRANSITION * sensors.bno055.linear_acceleration_data.acceleration.y;

      //  Velocity of sensor in the direction it's facing
      sensors.bno055.heading_velocity = ACCEL_VEL_TRANSITION * sensors.bno055.linear_acceleration_data.acceleration.x / cos(DEG_2_RAD * sensors.bno055.orientation_data.orientation.x);
      sensors.bno055.heading = sensors.bno055.orientation_data.orientation.x;
    }
  }

  return result;
}

/*
  Get readings from the LIS3MDL Magnetometer and put them in the environment
    data structure.
*/
Environment_Data get_lis3mdl (QWIICMUX *mx, Environment_Data curr_data, Adafruit_LIS3MDL *lis3) {
  Environment_Data sensors;
  sensors_event_t event;
  sensors = check_data(mx,curr_data);

  //  Set the mux port
  set_mux_port(mx, MUX_LIS3MDL_PORT, MUX_LIS3MDL_NAME);

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

Environment_Data get_ens160 (QWIICMUX *mx, Environment_Data curr_data, ScioSense_ENS160 *ens) {
  uint16_t count = 0;
  Environment_Data sensors;

  //  Set the mux port
  set_mux_port(mx, MUX_ENS160_PORT, MUX_ENS160_NAME);

  while (!ens->available() and count < ENS160_MAX_TRIES) {
    //  Wait for data to be available
    delay(100);
    count++;
  }

  if (count < ENS160_MAX_TRIES) {
    sensors = check_data(mx, curr_data);

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
  Get readings from the LSM6DSOX IMU and put them in the environment
    data structure.
*/
Environment_Data get_lsm6dsox (QWIICMUX *mx, Environment_Data curr_data, Adafruit_LIS3MDL *lis3, bool is_9d0f=true) {
  Environment_Data sensors;
  float x, y, z;
  float temperature = 0.0;

  //  Set the mux port
  set_mux_port(mx, MUX_LSM6DSOX_9DOF_PORT, MUX_LSM6DSOX_9DOF_NAME);
  
  sensors = check_data(mx, curr_data);

  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);

    sensors.lsm6dsox.accelerometer.x = x;
    sensors.lsm6dsox.accelerometer.y = y;
    sensors.lsm6dsox.accelerometer.z = z;
  }

  if (IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(x, y, z);

    sensors.lsm6dsox.gyroscope.x = x;
    sensors.lsm6dsox.gyroscope.y = y;
    sensors.lsm6dsox.gyroscope.z = z;
  }

  if (IMU.temperatureAvailable()) {
    IMU.readTemperatureFloat(temperature);

    sensors.lsm6dsox.celsius = temperature;
    sensors.lsm6dsox.fahrenheit = to_fahrenheit(temperature);
  }

  if (is_9d0f) {
    //  Read the LIS3MDL Magnetometer that is available
    get_lis3mdl (mx, curr_data, lis3);
  }

  return sensors;
}

/*
  Read ALL the data from the BNO055 9DOF IMU
*/
Environment_Data get_bno055 (QWIICMUX *mx, Environment_Data curr_data) {
  //  Set the mux port
  set_mux_port(mx, MUX_BNO055_PORT, MUX_BNO055_NAME);

  sensors = check_data(mx, curr_data);

  bno055.getCalibration(&sensors.bno055.system_cal, &sensors.bno055.gyroscope_cal, &sensors.bno055.accelerometer_cal, &sensors.bno055.magnetometer_cal);
  bno055.getEvent(&sensors.bno055.orientation_data, Adafruit_BNO055::VECTOR_EULER);
  bno055.getEvent(&sensors.bno055.angular_velocity_data, Adafruit_BNO055::VECTOR_GYROSCOPE);
  bno055.getEvent(&sensors.bno055.linear_acceleration_data, Adafruit_BNO055::VECTOR_LINEARACCEL);
  bno055.getEvent(&sensors.bno055.magnetometer_data, Adafruit_BNO055::VECTOR_MAGNETOMETER);
  bno055.getEvent(&sensors.bno055.accelerometer_data, Adafruit_BNO055::VECTOR_ACCELEROMETER);
  bno055.getEvent(&sensors.bno055.gravity_data, Adafruit_BNO055::VECTOR_GRAVITY);

  sensors.bno055.x_pos = sensors.bno055.x_pos + ACCEL_POS_TRANSITION * sensors.bno055.linear_acceleration_data.acceleration.x;
  sensors.bno055.y_pos = sensors.bno055.y_pos + ACCEL_POS_TRANSITION * sensors.bno055.linear_acceleration_data.acceleration.y;

  //  Velocity of sensor in the direction it's facing
  sensors.bno055.heading_velocity = ACCEL_VEL_TRANSITION * sensors.bno055.linear_acceleration_data.acceleration.x / cos(DEG_2_RAD * sensors.bno055.orientation_data.orientation.x);
  sensors.bno055.heading = sensors.bno055.orientation_data.orientation.x;

  return sensors;
}

/*
  Get temperature and humidity readings from the SHT45 and put them i sensor
    and put them into the environment data structure.
*/
Environment_Data get_sht45 (QWIICMUX *mx, Environment_Data curr_data, Adafruit_SHT4x *sht) {
  Environment_Data sensors;
  sensors_event_t rel_humidity, temperature;
  uint32_t timestamp = millis();
  float celsius, humidity;

  //  Set the mux port
  set_mux_port(mx, MUX_SHT45_PORT, MUX_SHT45_NAME);

  sensors = check_data(mx, curr_data);

  sht->getEvent(&rel_humidity, &temperature);// populate temp and humidity objects with fresh data
  celsius = temperature.temperature;

  sensors.sht45.celsius = celsius;
  sensors.sht45.fahrenheit = to_fahrenheit(celsius);
  sensors.sht45.humidity = rel_humidity.relative_humidity;

  return sensors;
}

Environment_Data get_scd40 (QWIICMUX *mx, Environment_Data curr_data, SensirionI2CScd4x *scd) {
  Environment_Data sensors;
  String error_message;
  uint16_t CO2, error;
  float celsius, fahrenheit, humidity;

  //  Set the mux port
  set_mux_port(mx, MUX_SCD40_PORT, MUX_SCD40_NAME);

  sensors = check_data(mx, curr_data);

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
  uint16_t scd_error = 0;
  String firmware_version;
  bool mux_present;
  bool connected, sensor_found;

  //  Initialize serial and wait for the port to open:
  Serial.begin(SERIAL_BAUDRATE);

  while(!Serial) {
    delay(SERIAL_DELAY_MS);
  }

  //  Start the I2C bus
  Wire.begin();

  //  Make sure the buzzer is off
  analogWrite(PIEZO_BUZZER_PIN, 0);

  Serial.print(ROBOT_DEVICE_NAME);
  Serial.print(", Version ");
  Serial.print(ROBOT_DEVICE_VERSION);
  Serial.print(", ");
  Serial.println(ROBOT_DEVICE_DATE);
  Serial.println();

  //  Initialize the Sparkfun I2C Multiplexor - Halt if not present
  init_mux(&mux);

  //  Start the Real Time Clock and set the current time
  sensor_found = init_ds3231(&mux, &rtc, &sensor_status);

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
    sensors = check_data(&mux, sensors);

    if (USING_ENS160) {
      sensor_found = init_ens160(&mux, &ens160, &sensor_status);
    }

    //  initialize the SCD-40 CO2, Temperature, and Humidity sensor
    if (USING_SCD40) {
      scd_error = init_scd40(&mux, &scd40, &sensor_status);

      if (scd_error == 268) {
        halt("Could not get the SCD-40 serial number!");
      } else if (scd_error > 0) {
        halt("Halt on SCD-40 error, code " + String(scd_error));
      }
    }

    //  Initialize the SHT4x Temperature and Humidity sensors
    if (USING_SHT45) {
      sensor_found = init_sht4x(&mux, &sht45, &sensor_status);
    }

    //  Initialize the LSM6DSOX IMU
    if (USING_LSM6DSOX_9DOF) { 
      sensor_found = init_lsm6dsox(&mux, &sensor_status); 
    }

    if (USING_BNO055) {
      sensor_found = init_bno055(&mux, &bno055, &sensor_status);
    }

    //  Initialize the LIS3MDL Magnetometer
    if (USING_LIS3MDL) {
      sensor_found = init_lis3mdl(&mux, &lis3, &sensor_status);
    }

    //  Initialize the VEML7700 Light (Lux) sensor
    if (USING_VEML7700) {
      sensor_found = init_veml7700(&mux, &sensor_status);
    }

    Serial.println();
    Serial.print("Today is ");
    Serial.println(timestamp(&mux, &rtc, MUX_DS3231_PORT, SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, SHOW_SECONDS));
  
    //  Start the web servertimestamp 
    Serial.println();
    Serial.println("Starting the web server");
    server.begin(WEB_SERVER_PORT);
  } else {
    halt("Unable to connect to network", true, ssid);
  }

  //  For testing setup()
  //halt("END OF SETUP HALT");

  Serial.println("Initization complete!");
  //delay(1000);
}

void loop (void) {
  WiFiClient client = server.available();
  Three_Axis triple;

  //  Counter to avoid printing BNO055 data every 10MS sample
  uint16_t bno_print_count = 0;

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
    blink_rgb(Blue);

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

      blink_rgb(Magenta);

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
            date_time = timestamp(&mux, &rtc, MUX_DS3231_PORT, SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, SHOW_SECONDS);
            Serial.println(PAGE_HOME_NAME);
            html.replace("SEQUENCE_COUNT_MARKER", String(sequence_nr));
            html.replace("DATESTAMP_MARKER", date_time);
            break;
          case PAGE_ENVIRONMENT:
            html =  PAGE_HTML[PAGE_ENVIRONMENT]; //String(HTML_CONTENT_ENVIRONMENT);
            date_time = timestamp(&mux, &rtc, MUX_DS3231_PORT, SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, SHOW_SECONDS);
            Serial.println(PAGE_ENVIRONMENT_NAME);
            html.replace("DATESTAMP_MARKER", date_time);
            html.replace("SEQUENCE_COUNT_MARKER", String(sequence_nr));

            Serial.println("Sending sensor readings");

            if (USING_SHT45) {
              sensors = get_sht45(&mux, sensors, &sht45);

              html.replace("SHT45_FAHRENHEIT_MARKER", String(sensors.sht45.fahrenheit));
              html.replace("SHT45_CELSIUS_MARKER", String(sensors.sht45.celsius));
              html.replace("SHT45_HUMIDITY_MARKER", String(sensors.sht45.humidity));
            } else {
              html.replace("SHT45_FAHRENHEIT_MARKER", "Not used");
              html.replace("SHT45_CELSIUS_MARKER", "Not used");
              html.replace("SHT45_HUMIDITY_MARKER", "Not used");
            }

            if (USING_SCD40) {
              Serial.println("Getting temperature and humidity readings");

              sensors = get_scd40 (&mux,sensors, &scd40);
            
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

            if (USING_ENS160) {
              sensors = get_ens160(&mux, sensors, &ens160);

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
            date_time = timestamp(&mux, &rtc, MUX_DS3231_PORT, SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, SHOW_SECONDS);
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
            date_time = timestamp(&mux, &rtc, MUX_DS3231_PORT, SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, SHOW_SECONDS);

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
            date_time = timestamp(&mux, &rtc, MUX_DS3231_PORT, SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, SHOW_SECONDS);
            lux = read_veml7700(&veml);

            //  Replace the markers by real values
            html.replace("SEQUENCE_COUNT_MARKER", String(sequence_nr));
            html.replace("LIGHT_VALUE_MARKER", String(lux));
            html.replace("DATESTAMP_MARKER", date_time);

            break;
          case PAGE_IMU:
            html = PAGE_HTML[PAGE_IMU]; //String(HTML_CONTENT_IMU);
            date_time = timestamp(&mux, &rtc, MUX_DS3231_PORT, SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, SHOW_SECONDS);

            if (USING_LSM6DSOX_9DOF or USING_LSM6DSOX_6DOF) {
              sensors = get_lsm6dsox(&mux, sensors, &lis3);

              //  Replace the markers by real values
              temp_html = imu_format_xyz_html(sensors.lsm6dsox.accelerometer);
              html.replace("IMU_ACCELEROMETER_MARKER", temp_html);
              temp_html = imu_format_xyz_html(sensors.lsm6dsox.gyroscope);
              html.replace("IMU_GYROSCOPE_MARKER", temp_html);
              temp_html = imu_format_xyz_html(sensors.lsm6dsox.magnetometer);
              html.replace("IMU_MAGNETOMETER_MARKER", temp_html);
            }

            if (USING_BNO055) {
              //  Read all the data from the IMU
              sensors = get_bno055(&mux, sensors);

              bno_print_event(&sensors.bno055.orientation_data);
              bno_print_event(&sensors.bno055.angular_velocity_data);
              bno_print_event(&sensors.bno055.linear_acceleration_data);
              bno_print_event(&sensors.bno055.magnetometer_data);
              bno_print_event(&sensors.bno055.accelerometer_data);
              bno_print_event(&sensors.bno055.gravity_data);

              triple.x = sensors.bno055.accelerometer_data.acceleration.x;
              triple.y = sensors.bno055.accelerometer_data.acceleration.y;
              triple.z = sensors.bno055.accelerometer_data.acceleration.z;

              temp_html = imu_format_xyz_html(triple);
              html.replace("IMU_ACCELEROMETER_MARKER", temp_html);

              triple.x = sensors.bno055.gravity_data.gyro.x;
              triple.y = sensors.bno055.gravity_data.gyro.y;
              triple.z = sensors.bno055.gravity_data.gyro.z;

              temp_html = imu_format_xyz_html(triple);
              html.replace("IMU_GYROSCOPE_MARKER", temp_html);

              triple.x = sensors.bno055.magnetometer_data.magnetic.x;
              triple.y = sensors.bno055.magnetometer_data.magnetic.y;
              triple.z = sensors.bno055.magnetometer_data.magnetic.z;

              temp_html = imu_format_xyz_html(triple);
              html.replace("IMU_MAGNETOMETER_MARKER", temp_html);

               if (bno_print_count * BNO055_SAMPLERATE_DELAY_MS >= PRINT_DELAY_MS) {
                //  Enough iterations have passed that we can print the latest data
                Serial.print("Heading: ");
                Serial.println(sensors.bno055.heading);
                Serial.print(", Position: ");
                Serial.print(sensors.bno055.x_pos);
                Serial.print(", ");
                Serial.println(sensors.bno055.y_pos);
                Serial.print("Speed: ");
                Serial.print(sensors.bno055.heading_velocity);
                Serial.println("---------------------------");

                bno_print_count = 0;
              } else {
                bno_print_count += 1;
              }
            }

            html.replace("DATESTAMP_MARKER", date_time);
            html.replace("SEQUENCE_COUNT_MARKER", String(sequence_nr));
            break;
          case PAGE_ERROR_404:
            html = String(HTML_CONTENT_404);
            date_time = timestamp(&mux, &rtc, MUX_DS3231_PORT, SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, SHOW_SECONDS);

            //  Replace the markers by real values
            html.replace("SEQUENCE_COUNTER_MARKER", String(sequence_nr));
            html.replace("DATESTAMP_MARKER", date_time);
            break;
          case PAGE_ERROR_405:
            html = String(HTML_CONTENT_405);
            date_time = timestamp(&mux, &rtc, MUX_DS3231_PORT, SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, SHOW_SECONDS);

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

    //  Allow time for the web server to receive data
    delay(WEB_SERVER_DELAY_MS);
  }
}