/*
 */

//#include  "Hat_Carrier-Raspberry_Pi_Pins.h"
#include  "RaspberryPi_to_Portenta_C33.h"

#include  "Secrets.h"
#include  "Robot_Controller.h"
#include  "Web_Server_Control.h"

#include  "index.h"
#include  "Environment.h"
#include  "switches.h"
#include  "potentiometer.h"
#include  "light.h"
#include  "imu.h"
#include  "error_404.h"
#include  "error_405.h"

//Include the NTP library
#include <NTPClient.h>
WiFiUDP Udp; // A UDP instance to let us send and receive packets over UDP
NTPClient time_client(Udp);

// Include the RTC library
#include "RTC.h"

#include <Adafruit_LSM6DSOX.h>
Adafruit_LSM6DSOX sox;

#include <Adafruit_LIS3MDL.h>
#include <Adafruit_Sensor.h>
Adafruit_LIS3MDL lis3;

#include "Adafruit_SHT4x.h"
Adafruit_SHT4x sht4 = Adafruit_SHT4x();

#include "Adafruit_VEML7700.h"
Adafruit_VEML7700 veml = Adafruit_VEML7700();

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
  Blink an LED on the selected pin.
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
  Blink an LED on the selected Raspberry Pi pin.
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
*/
void halt (void) {
  Serial.println();
  Serial.println("Halting...");

  //  Infinite loop
  while (true) {
    blink_rgb(red, 100);
    delay(1);
  }
}

/*
  Add "0" to the left of a numeric string to the required length
*/
String left_pad (String str, uint8_t pad_length=DEFAULT_PAD_LENGTH, String pad_char=DEFAULT_PAD_STRING) {
  uint8_t str_len, str_index = 0, position = 0;
  String result_str = "", digits = "0123456789", temp_str = "";
  bool is_number = true;

  str_len = str.length();

  if (str_len == pad_length) {
    result_str = str;
  } else {
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
      //  Add the appropriate number of "0" to the left of the string
      for (str_index=1; str_index < pad_length; str_index++) {
        result_str = pad_char + result_str;
      }
    } else {
      //  Invalid - Non-digit character is present
      result_str = "**";
    }
  }

  return result_str;
}

/*
  Get the current date and time
*/
String timestamp (NTPClient *time_cl, bool show_full=SHOW_FULL_DATE, bool hours_1224=SHOW_12_HOURS, bool long_date=SHOW_LONG_DATE, bool show_seconds=SHOW_SECONDS) {
  RTCTime current_time;
  String date_time, date_str = "D*", time_str = "T*";
  String year_str = "Y*", month_str = "M*", day_str = "D*";
  String hours_str, min_sec_str, week_day_str = "*";
  String am_pm = " AM", day_suffix = "**";
  uint8_t position, str_len, week_day_nr, day, suffix, month, hours;

  //  Retrieve the date and time from the RTC
  //  String is in the format yyy-mm-ddThh:mm:ss
  RTC.getTime(current_time); 
  date_time = String(current_time);
  str_len = date_time.length();
  position = date_time.indexOf("T");

  if (show_full) {
    time_str = date_time.substring(position + 1, str_len);
    position = date_time.indexOf("T");
    date_str = date_time.substring(0, position);

    str_len = date_str.length();
    year_str = date_str.substring(0, 4);
    month_str = date_str.substring(5, 7);
    month = month_str.toInt();
    day_str = date_str.substring(8, str_len);
    day = day_str.toInt();

    week_day_nr = time_cl->getDay();
    week_day_str = week_days[week_day_nr];

    day_str = String(day);
    str_len = day_str.length();

    if (day < 10) {
      suffix = day;
    } else if (str_len == 2) {
      suffix = day_str.substring(str_len - 1, str_len - 1).toInt();
    }

    if (long_date) {
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
        dafault:
          day_suffix = "**";
          break;
      }
      
      day_str = day_str + day_suffix;
    } else {
      day_str = left_pad(day_str, 2, "0");
    }

    str_len = time_str.length();
    hours_str = time_str.substring(0, 2);
    hours = hours_str.toInt();

    if (show_seconds) {
      min_sec_str = time_str.substring(3, str_len);
    } else {
      min_sec_str = time_str.substring(3, str_len - 3);
    }

    if (hours == 0) {
      hours = 12;
      am_pm = " AM";
    }

    if (hours_1224) {
      time_str = String(hours) + ":" + min_sec_str + am_pm;
    } else {
      if (hours > 12) {
        hours = hours - 12;
        am_pm = " PM";
      } else {
        am_pm = " AM";
      }

      time_str = left_pad(String(hours), 2, "0") + ":" + min_sec_str;
    }

    if (long_date) {
      date_str = week_day_str + ", " + long_months[month - 1] + " " + day_str + ", " + year_str;
    } else {
      date_str = left_pad(month_str, 2, "0") + "/" + day_str + "/" + year_str;
    }

    date_time = date_str + " at " + time_str;
  } else {
    date_time = date_time.substring(position + 1, str_len);
  }

  return date_time;
}

/*
  Set the onboard RTC from an NTP (internet) time source

  Returns: (RTCTime) current time
*/
RTCTime set_rtc (int8_t utc_offset_hrs, NTPClient *time_cl) {
  RTCTime current_time, time_to_set;
  uint32_t unix_time;

  //  Get the current date and time from an NTP server and convert
  //    it to UTC +2 by passing the time zone offset in hours.
  // You may change the time zone offset to your local one.
  unix_time = time_cl->getEpochTime() + (utc_offset_hrs * 3600);
  time_to_set = RTCTime(unix_time);
  RTC.setTime(time_to_set);

  //  Set the UTC time offset for the current timezone
  time_cl->setTimeOffset(utc_offset_hrs);

  RTC.getTime(current_time);

  return current_time;
}

/*
  Convert the Celsius temperature to Fahrenheit

  Returns: (float) temperature in fahrenheit
*/
float to_fahrenheit (float celsius) {
  return celsius * 1.8 + 32;
}

/*
  Initialize the SHT4x temperature and humidity sensor
*/
Adafruit_SHT4x init_sht4x (Adafruit_SHT4x *sht) {
  if (sht->begin()) {
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
    Serial.println();
    Serial.println("Could not find any SHT4x sensors!");

    halt();
  }

  return *sht;
}

/*
  Initialize the SHT45 Temeprature and Humidity sensor
*/
Adafruit_LIS3MDL init_lis3mdl (Adafruit_LIS3MDL *lis3) {
  // Try to initialize!
  if (lis3->begin_I2C()) {          // hardware I2C mode, can pass in address & alt Wire
    Serial.println("LIS3MDL Found!");

    lis3->setPerformanceMode(LIS3MDL_MEDIUMMODE);
    Serial.print("Performance mode set to: ");
    switch (lis3->getPerformanceMode()) {
      case LIS3MDL_LOWPOWERMODE:
        Serial.println("Low");
        break;
      case LIS3MDL_MEDIUMMODE:
        Serial.println("Medium");
        break;
      case LIS3MDL_HIGHMODE:
        Serial.println("High");
        break;
      case LIS3MDL_ULTRAHIGHMODE:
        Serial.println("Ultra-High");
        break;
    }

    lis3->setOperationMode(LIS3MDL_CONTINUOUSMODE);

    Serial.print("Operation mode set to: ");
    // Single shot mode will complete conversion and go into power down
    switch (lis3->getOperationMode()) {
      case LIS3MDL_CONTINUOUSMODE:
        Serial.println("Continuous");
          break;
      case LIS3MDL_SINGLEMODE:
        Serial.println("Single mode");
        break;
      case LIS3MDL_POWERDOWNMODE:
        Serial.println("Power-down");
        break;
    }

    lis3->setDataRate(LIS3MDL_DATARATE_155_HZ);
    // You can check the datarate by looking at the frequency of the DRDY pin
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
  } else {
    Serial.println("Unable to find the magnetometer!");

    halt();
  }

  return *lis3;
}

/*
  Initialize the LSM6DSOX IMU
*/
Adafruit_LSM6DSOX init_lsm6dsox (Adafruit_LSM6DSOX *sx) {
  //  Initialie the IMU
  if (sx->begin_I2C()) {
    Serial.println("Found the LSM6DSOX IMU!");

/*
  Possible values:
      case LSM6DS_ACCEL_RANGE_2_G:
      case LSM6DS_ACCEL_RANGE_4_G:
      case LSM6DS_ACCEL_RANGE_8_G:
      case LSM6DS_ACCEL_RANGE_16_G:
*/
    sx->setAccelRange(LSM6DS_ACCEL_RANGE_2_G);
/*
  Possible Values:
    case LSM6DS_RATE_SHUTDOWN:
    case LSM6DS_RATE_12_5_HZ:
    case LSM6DS_RATE_26_HZ:
    case LSM6DS_RATE_52_HZ:
    case LSM6DS_RATE_104_HZ:
    case LSM6DS_RATE_208_HZ:
    case LSM6DS_RATE_416_HZ:
    case LSM6DS_RATE_833_HZ:
    case LSM6DS_RATE_1_66K_HZ:
    case LSM6DS_RATE_3_33K_HZ:
    case LSM6DS_RATE_6_66K_HZ:
*/
    sx->setAccelDataRate(LSM6DS_RATE_12_5_HZ);

    Serial.print("Accelerometer data rate set to: ");
    sx->setGyroRange(LSM6DS_GYRO_RANGE_250_DPS );

    Serial.print("Gyroscope range set to: ");

    switch (sx->getGyroRange()) {
      case LSM6DS_GYRO_RANGE_125_DPS:
        Serial.println("125 degrees/s");
        break;
      case LSM6DS_GYRO_RANGE_250_DPS:
        Serial.println("250 degrees/s");
        break;
      case LSM6DS_GYRO_RANGE_500_DPS:
        Serial.println("500 degrees/s");
        break;
      case LSM6DS_GYRO_RANGE_1000_DPS:
        Serial.println("1000 degrees/s");
        break;
      case LSM6DS_GYRO_RANGE_2000_DPS:
        Serial.println("2000 degrees/s");
        break;
      case ISM330DHCX_GYRO_RANGE_4000_DPS:
        break; // unsupported range for the DSOX
    }

    sx->setGyroDataRate(LSM6DS_RATE_12_5_HZ);

    Serial.print("Gyro data rate set to: ");

    switch (sx->getGyroDataRate()) {
      case LSM6DS_RATE_SHUTDOWN:
        Serial.println("0 Hz");
        break;
      case LSM6DS_RATE_12_5_HZ:
        Serial.println("12.5 Hz");
        break;
      case LSM6DS_RATE_26_HZ:
        Serial.println("26 Hz");
        break;
      case LSM6DS_RATE_52_HZ:
        Serial.println("52 Hz");
        break;
      case LSM6DS_RATE_104_HZ:
        Serial.println("104 Hz");
        break;
      case LSM6DS_RATE_208_HZ:
        Serial.println("208 Hz");
        break;
      case LSM6DS_RATE_416_HZ:
        Serial.println("416 Hz");
        break;
      case LSM6DS_RATE_833_HZ:
        Serial.println("833 Hz");
        break;
      case LSM6DS_RATE_1_66K_HZ:
        Serial.println("1.66 KHz");
        break;
      case LSM6DS_RATE_3_33K_HZ:
        Serial.println("3.33 KHz");
        break;
      case LSM6DS_RATE_6_66K_HZ:
        Serial.println("6.66 KHz");
        break;
    }
  } else {
    Serial.println();
    Serial.println("Failed to find the LSM6DSOX IMU!");
    Serial.println();

    halt();
  }

  return *sx;
}

/*
Adafruit_VEML7700 init_veml7700 (Adafruit_VEML7700 *vm) {
  if (vm->begin()) {
    Serial.println("Found a VEML7700 Lux sensor");
    Serial.println("Settings used for reading:");
    Serial.print(F("Gain: "));

    switch (vm->getGain()) {
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
  Serial.println(WiFi.localIP());

  //  Print the received signal strength:
  Serial.print("Signal strength (RSSI):");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
}

bool connect_to_wifi (char *ssid, char *passwd, uint8_t connection_timeout_ms=CONNECTION_TIMEOUT_MS, uint8_t  max_nr_connects=MAX_NR_CONNECTS) {
  bool connected = true;
  uint8_t connect_count = 0;

  //  Attempt to connect to WiFi network:
  while (wifi_status != WL_CONNECTED and (connect_count < max_nr_connects)) {
    connect_count += 1;

    Serial.print("Attempt #");
    Serial.print(connect_count);
    Serial.print(" to connect to the '");
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

  if (wifi_status != WL_CONNECTED) {
    connected = false;
  } else {
    //  Print connection status
    print_wifi_status();
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
        digitalWrite(LED_PINS[index], LOW);
      } else {
        digitalWrite(LED_PINS[index], HIGH);
      }
    }
  } else {
    result = false;
  }
}

/*
  Initialize the digital LED outputs

  For the Portenta C33, LOW = Active (HIGH or ON state)
*/
void init_leds (uint8_t nr_of_leds=NUMBER_OF_LEDS, uint8_t blink_delay_ms=DEFAULT_BLINK_RATE_MS) {
  uint8_t index;

  //  Set the LED pins to OUTPUT
  for (index=0; index < nr_of_leds; index++) {
    //  Set the LED pins to OUTPUT
    pinMode(LED_PINS[index], OUTPUT);
    digitalWrite(LED_PINS[index], HIGH);

    //  Blink each of the LEDs
    blink_led_c33(LED_PINS[index]);
    delay(100);
  }

  Serial.println();
  Serial.print("There are ");
  Serial.print(nr_of_leds);
  Serial.println(" LEDs (Digital Output)");
}

/*
  Initialize the digital pins for the switches
*/
void init_switches(uint8_t nr_of_switches=NUMBER_OF_SWITCHES) {
  uint8_t index;

  for (index=0; index < nr_of_switches; index++) {
    //  Set the analog pin to INPUT
    pinMode(SWITCH_PINS[index], INPUT_PULLDOWN);
  }

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
  Check the environment data and intialize it if necessary
*/
Environment_Data check_data (Environment_Data curr_data) {
  Environment_Data result;
  Three_Axis xyz;

  //  If we have current data, copy it
  if (curr_data.valid) {
    //  Save existing data
    result = curr_data;
  } else {
    xyz.x = 0.0;
    xyz.y = 0.0;
    xyz.z = 0.0;

    //  Initialize data structure
    result.valid = false;

    result.celsius = 0.0;
    result.fahrenheit = 0.0;
    result.humidity = 0.0;

    result.accelerometer = xyz;
    result.gyroscope = xyz;
    result.temperature = 0.0;
  
    result.magnetometer = xyz;
  }

  return result;
}

/*
  Get readings from the LIS3MDL Magnetometer and put them in the environment
    data structure.
*/
Environment_Data get_lis3mdl (Environment_Data curr_data, Adafruit_LIS3MDL *lis3) {
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
Environment_Data get_lsm6dsox (Environment_Data curr_data, Adafruit_LSM6DSOX *sx) {
  Environment_Data sensors;
  sensors_event_t accel, gyro, temperature, mag;

  sensors = check_data(curr_data);

  sx->getEvent(&accel, &gyro, &temperature);

  sensors.accelerometer.x = accel.acceleration.x;
  sensors.accelerometer.y = accel.acceleration.y;
  sensors.accelerometer.z = accel.acceleration.z;
  sensors.gyroscope.x = gyro.gyro.x;
  sensors.gyroscope.y = gyro.gyro.y;
  sensors.gyroscope.z = gyro.gyro.z;
  sensors.temperature = temperature.temperature;

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
      state = "<SPAN style=\"color: green\">ON</SPAN>";
    } else {
      state ="<SPAN style=\"color: magenta\">OFF</SPAN>";
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
      Serial.print("ON");
    } else {
      Serial.print("OFF");
    }

    if (index < nr_of_switches - 1) {
      Serial.print(", ");
    }
  }

  Serial.println();
}

float read_veml7700(Adafruit_VEML7700 *vm) {
  return 0.0;
}

void read_switches (uint8_t nr_of_switches=NUMBER_OF_SWITCHES) {
  uint8_t index;
  String html;

  for (index=0; index < nr_of_switches; index++) {
    SWITCH_READINGS[index] = digitalRead(SWITCH_PINS[index]);
  }
}

void setup (void) {
  RTCTime current_time;
  String firmware_version;
  bool connected;

  //  Initialize serial and wait for the port to open:
  Serial.begin(115200);
  delay(5000);

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

  //  Check firmware version
  firmware_version = WiFi.firmwareVersion();

  if (firmware_version < WIFI_FIRMWARE_LATEST_VERSION)
    Serial.println("Please upgrade the firmware!");

  connected = connect_to_wifi(ssid, passwd);

  if (connected) {
    // Blink it Green because we are connected
    blink_rgb(green, 100, 2);

    init_leds();

    init_switches();

    //  Set analog resolutionto 12 bits
    analogReadResolution(12);

    init_resistors();

    //  Initialize the sensors data structure
    sensors.valid = false;
    sensors = check_data(sensors);

    //  Initialize the Raspberry Pi LED
    pinMode(LED_RASPI_PIN, OUTPUT);
    blink_led_raspi(LED_RASPI_PIN, 100, 2);

    //  Initialize the RGB LED - Set pins to be outputs
    pinMode(LEDR, OUTPUT);
    pinMode(LEDG, OUTPUT);
    pinMode(LEDB, OUTPUT);

    //  Initialize the SHT4x Temperature and Humidity sensors
    if (USING_SHT45_TEMP) {
       sht4 = init_sht4x(&sht4);
    }

    //  Initialize the LSM6DSOX IMU
    if (USING_LSM6DSOX_LIS3MDL_IMU) {  
      sox = init_lsm6dsox(&sox);
    }

    //  Initialize the LIS3MDL Magnetometer
    if (USING_LIS3MDL_MAG) {
      lis3 = init_lis3mdl(&lis3);
    }

    //  Initialize the VEML7700 Light (Lux) sensor
    if (USING_VEML_LUX) {
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
        Serial.println("Unable to find the VEML7700 sensor!");

        halt();
      }
    }

    //  Start the time client
    Serial.println("Starting the time client");
    time_client.begin();
    time_client.update();

    //  Start the Real Time Clock and set the current time
    Serial.println("Starting the RTC");
    RTC.begin();
    set_rtc(UTC_OFFSET_HRS, &time_client);
    RTC.getTime(current_time);

    Serial.println();
    Serial.print("Today is ");
    Serial.println(timestamp(&time_client, SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, SHOW_SECONDS));
  
    //  Start the web server
    Serial.println();
    Serial.println("Starting the web server");
    server.begin(WEB_SERVER_PORT);
    Serial.println();
  } else {
    Serial.println();
    Serial.print("Unable to connect to the '");
    Serial.print(ssid);
    Serial.println("' network!");

    halt();
  }
}

void loop (void) {
  WiFiClient client;
  bool send_page = true;
  float lux, potentiometer_voltage;
  uint16_t potentiometer_reading;
  uint8_t index = 0;
  int page_id = 0, index_pos = 0, start_pos = 0;
  unsigned long start_millis, end_millis;
  String HTTP_req, html = "", date_time = "", temp_html = "", led_html = "";
  String potentiometer_units;

  blink_rgb(blue);
  blink_led_raspi(LED_RASPI_PIN, 100, 2);

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
          date_time = timestamp(&time_client, SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, SHOW_SECONDS);
          Serial.println(PAGE_HOME_NAME);
          html = String(HTML_CONTENT_HOME);
          html.replace("PAGE_HOME_TITLE_MARKER", PAGE_HOME_TITLE);
          html.replace("PAGE_HOME_NAME_MARKER", PAGE_HOME_NAME);
          html.replace("REQUEST_COUNT_MARKER", String(request_count));
          html.replace("SKETCH_CODE_MARKER", SKETCH_ID_CODE);
          html.replace("DATESTAMP_MARKER", date_time);
          html.replace("ROBOT_PAGE_NAME_MARKER", ROBOT_DEVICE_NAME);
          html.replace("ROBOT_NAME_MARKER", ROBOT_DEVICE_NAME);
          break;
        case PAGE_ENVIRONMENT:
          date_time = timestamp(&time_client, SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, SHOW_SECONDS);
          Serial.println(PAGE_ENVIRONMENT_NAME);
          html = String(HTML_CONTENT_ENVIRONMENT);

          if (USING_SHT45_TEMP) {
            Serial.println("Getting temperature and humidity readings");

            sensors = get_temperature(sensors, &sht4);

            Serial.println("Sending sensor readings");
          
            //  Replace the markers by real values
            html.replace("PAGE_ENVIRONMENT_TITLE_MARKER", PAGE_ENVIRONMENT_NAME);
            html.replace("PAGE_ENVIRONMENT_NAME_MARKER", PAGE_ENVIRONMENT_NAME);
            html.replace("DATESTAMP_MARKER", date_time);
            html.replace("REQUEST_COUNTER_MARKER", String(request_count));
            html.replace("FAHRENHEIT_MARKER", String(sensors.fahrenheit));
            html.replace("CELSIUS_MARKER", String(sensors.celsius));
            html.replace("HUMIDITY_MARKER", String(sensors.humidity));
          }
          break;
        case PAGE_SWITCHES:
          date_time = timestamp(&time_client, SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, SHOW_SECONDS);
          html = String(HTML_CONTENT_SWITCHES);
          read_switches();
          show_switches();
          set_leds();
          temp_html = switch_format_html();

          //  Replace the markers by real values
          html.replace("PAGE_SWITCHES_TITLE_MARKER", PAGE_SWITCHES_TITLE);
          html.replace("PAGE_SWITCHES_NAME_MARKER", PAGE_SWITCHES_NAME);
          html.replace("SWITCHES_TEXT_MARKER", temp_html);
          html.replace("DATESTAMP_MARKER", date_time);
          html.replace("REQUEST_COUNTER_MARKER", String(request_count));
          break;
        case PAGE_POTENTIOMETER:
          date_time = timestamp(&time_client, SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, SHOW_SECONDS);
          html = String(HTML_CONTENT_POTENTIOMETER);
          //  Read the potentiomenter
          potentiometer_reading = analogRead(ANALOG_POT_PIN);
          potentiometer_voltage = (MAXIMUM_ANALOG_VOLTAGE / ANALOG_RESOLUTION) * potentiometer_reading; // * 1000.0;

          //  Print the potentiomenter reading and voltage
          Serial.print("Potentiometer: Reading = ");
          Serial.print(potentiometer_reading);
          Serial.print(", Voltage = ");
          Serial.println(potentiometer_voltage);

          if (potentiometer_voltage < 1.0) {
            potentiometer_voltage = potentiometer_voltage * 1000;
            potentiometer_units = " mV";
          } else {
            potentiometer_units = " V";
          }

          //  Replace the markers by real values
          html.replace("PAGE_POTENTIOMETER_TITLE_MARKER", PAGE_POTENTIOMETER_TITLE);
          html.replace("PAGE_POTENTIOMETER_NAME_MARKER", PAGE_POTENTIOMETER_NAME);
          html.replace("POTENTIOMETER_READING_MARKER", String(potentiometer_reading));
          html.replace("POTENTIOMETER_VOLTAGE_MARKER", String(potentiometer_voltage));
          html.replace("POTENTIOMETER_UNITS_MARKER", potentiometer_units);
          html.replace("DATESTAMP_MARKER", date_time);
          html.replace("REQUEST_COUNTER_MARKER", String(request_count));
          break;
        case PAGE_LIGHT:
          date_time = timestamp(&time_client, SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, SHOW_SECONDS);
          html = String(HTML_CONTENT_LIGHT);
          lux = read_veml7700(&veml);

          //  Replace the markers by real values
          html.replace("PAGE_LIGHT_TITLE_MARKER", PAGE_LIGHT_TITLE);
          html.replace("PAGE_LIGHT_NAME_MARKER", PAGE_LIGHT_NAME);
          html.replace("LIGHT_VALUE_MARKER", String(lux));
          html.replace("DATESTAMP_MARKER", date_time);
          html.replace("REQUEST_COUNTER_MARKER", String(request_count));
          break;
        case PAGE_IMU:
          date_time = timestamp(&time_client, SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, SHOW_SECONDS);
          html = String(HTML_CONTENT_IMU);
          sensors = get_lsm6dsox(sensors, &sox);

          //  Replace the markers by real values
          html.replace("PAGE_IMU_TITLE_MARKER", PAGE_IMU_TITLE);
          html.replace("PAGE_IMU_NAME_MARKER", PAGE_IMU_NAME);
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
          date_time = timestamp(&time_client, SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, SHOW_SECONDS);
          html = String(HTML_CONTENT_404);

          //  Replace the markers by real values
          html.replace("REQUEST_COUNTER_MARKER", String(request_count));
          html.replace("DATESTAMP_MARKER", date_time);
          break;
        case PAGE_ERROR_405:
          date_time = timestamp(&time_client, SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, SHOW_SECONDS);
          html = String(HTML_CONTENT_405);

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

  //  Give the web browser time to receive the data
  delay(WEB_SERVER_DELAY_MS);
}
