/*
 *
 *  Copyright (c) by Dale Weber <hybotics@hybotics.dev> 2024
*/
#ifndef ROBOT_CONTROLLER_H
#define ROBOT_CONTROLLER_H

#include  "Arduino_HAT_Carrier_C33.h"
#include  "Secrets.h"

#if defined(ARDUINO_PORTENTA_C33)
#include <WiFiC3.h>
#elif defined(ARDUINO_UNOWIFIR4)
#include <WiFiS3.h>
#endif

#define ROBOT_DEVICE_NAME           "Robot Smart Home Controller"
#define ROBOT_DEVICE_VERSION        "0.5.0"
#define ROBOT_DEVICE_DATE           "20-May-2024"


#define SKETCH_ID_CODE              "Robot Smart Home Controller Split Code"

/*
  Sketch control - turn on (true) or off (false) as needed.
*/
#define USING_SHT45_TEMP            true
#define USING_LSM6DSOX_LIS3MDL_IMU  true
#define USING_LIS3MDL_MAG           false
#define USING_VEML7700_LUX              true

/*
#if (USING_LSM6DSOX_LIS3MDL_IMU)
#define USING_LIS3MDL_MAG           true
#endif
*/

/*
  Controls for the timestamp() function
*/
#define SHOW_TIME_ONLY            false
#define SHOW_FULL_DATE            true

#define SHOW_12_HOURS             false
#define SHOW_24_HOURS             true

#define SHOW_LONG_DATE            true
#define SHOW_SHORT_DATE           false

#define SHOW_NORMAL_TIME          false
#define SHOW_SECONDS              true

#define UTC_OFFSET_HRS            -7

/*
  Maximum voltage to be read from analog pins in volts
*/
#define MAXIMUM_ANALOG_VOLTAGE    5.0
#define ANALOG_RESOLUTION         4096.0

/*
  Defaults for the blink_rgb() routine
*/
#define DEFAULT_BLINK_RATE_MS     250
#define DEFAULT_NR_CYCLES         1

/*
  Defaults for left_pad()
*/
#define DEFAULT_PAD_LENGTH        2
#define DEFAULT_PAD_STRING        "0"

/*
  Controls for the connect_wifi() routine
*/
#define MAX_NR_CONNECTS           5
#define CONNECTION_TIMEOUT_MS     5000
#define CONNECTION_DELAY_MS       20
#define	SERIAL_BAUDRATE           115200
#define SERIAL_DELAY_MS           5000

///////////////////////////////////////////////
#define WIFI_DELAY_MS             3000
#define SEVENZYYEARS              2208988800UL
//////////////////////////////////////////////

#define PIEZO_BUZZER_PIN          D5          //  PWM

#define LED_RASPI_CONNECT_PIN     GPIO_21     //  Green - Connected to WiFi
#define LED_RASPI_WIFI_PIN        GPIO_20     //  Yellow - Lost WiFi connection
#define LED_RASPI_HALT_PIN        GPIO_26     //  Red - Unable to connect to WiFi

#define ANALOG_POT_PIN            A5
#define ANALOG_POT_NUMBER          0

/*
  Circuit Digital Pins: LEDs
*/
#define LED_WHITE_PIN             A5          //  Works
#define LED_BLUE_PIN              D1          //  Works
#define LED_RED_PIN               D2          //  Works
#define LED_YELLOW_PIN            D3          //  Works
#define LED_GREEN_PIN             D4          //  Works

/*
  Switches
*/
#define SWITCH_WHITE_PIN          A0          //  A0 Works Raspberry Pi 07 (White)
#define SWITCH_BLUE_PIN           A1          //  A1 Works Raspberry Pi 29 (Blue)
#define SWITCH_RED_PIN            A2          //  6 Works Raspberry Pi 31 (Red)
#define SWITCH_YELLOW_PIN         A3          //  Works   Raspberry Pi 32 (Yellow)
#define SWITCH_GREEN_PIN          A4          //  Works   Raspberry Pi 22 (Green)

/*
  Circuit Analog Pins: Resistors
*/
#define ANALOG_220_PIN            A0          //  Works   Raspberry Pi 07
#define ANALOG_330_PIN            A1          //  Works   Raspberry Pi 29
#define ANALOG_1K_PIN             A2          //  Works   Raspberry Pi 31
#define ANALOG_2K_PIN             A3          //  Works   Raspberry Pi 32
#define ANALOG_5K_PIN             A4          //  Works   Raspberry Pi 22
#define ANALOG_10K_PIN            A5          //  Works
#define ANALOG_100K_PIN           A6          //  Works
#define ANALOG_1M_PIN             A7          //  Works

#define NOTE_DURATION_MS          500
#define NOTE_DELAY_MS             1000

/*
  System Status for hotswapping sensors
*/
struct System_Sensor_Status {
  bool sht45_status = USING_SHT45_TEMP;
  bool lsm6dsox_status = USING_LSM6DSOX_LIS3MDL_IMU;
  bool lis3mdl_status = USING_LIS3MDL_MAG;
  bool veml7700_status = USING_VEML7700_LUX;
};

/*
  This holds data read from any three-axis device such as IMUs
*/  
struct Three_Axis {
    float x, y, z;
};

/*
  Used to hold all sensor data
*/
struct Environment_Data {
  bool valid = false;
  float celsius;
  float fahrenheit;
  float humidity;

  Three_Axis accelerometer;
  Three_Axis gyroscope;
  float temperature;
  Three_Axis magnetometer;
};

/*
  The color to show for the RGB LED in the blink_rgb() routine
*/
struct ColorRGB {
  uint8_t red, green, blue;
  bool redB, greenB, blueB;
};

char ssid[] = WIFI_SSID;
char passwd[] = WIFI_PASSWD;
bool connected = true;        //  True = Connected to WiFi

int MUSIC_NOTES[12] = { 261, 277, 293, 311, 329, 349, 369, 392, 415, 440, 466, 493 };
uint8_t MAX_NR_NOTES = 12;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

String long_months[12] = { "January", "February", "March", "April", "May", "June", "July",
  "August", "September", "October", "November", "December" };

String week_days[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", 
  "Thursday", "Friday", "Saturday"};

int wifi_status = WL_IDLE_STATUS;
uint16_t request_count = 0;
uint16_t looper = 0;
Environment_Data sensors;
System_Sensor_Status sensor_status;

uint8_t LED_PINS[5] = { LED_WHITE_PIN, LED_BLUE_PIN, LED_RED_PIN, LED_YELLOW_PIN, LED_GREEN_PIN };
#define NUMBER_OF_LEDS (sizeof(LED_PINS) / sizeof(uint8_t))
String LED_NAMES[NUMBER_OF_LEDS] = { "White", "Blue", "Red", "Yellow", "Green" };

uint8_t SWITCH_PINS[5] = { SWITCH_WHITE_PIN, SWITCH_BLUE_PIN, SWITCH_RED_PIN, SWITCH_YELLOW_PIN, SWITCH_GREEN_PIN };
#define NUMBER_OF_SWITCHES (sizeof(SWITCH_PINS) / sizeof(uint8_t))
String SWITCH_NAMES[NUMBER_OF_SWITCHES] = { "White", "Blue", "Red", "Yellow", "Green" };
bool SWITCH_READINGS[NUMBER_OF_SWITCHES] = { false, false, false, false, false };

uint8_t RESISTOR_PINS[8] = { ANALOG_POT_PIN, ANALOG_330_PIN, ANALOG_1K_PIN, ANALOG_2K_PIN, 
  ANALOG_5K_PIN, ANALOG_10K_PIN, ANALOG_100K_PIN, ANALOG_1M_PIN };

#define NUMBER_OF_RESISTORS ((sizeof(RESISTOR_PINS) / sizeof(uint8_t)))
int RESISTOR_READINGS[NUMBER_OF_RESISTORS];
String RESISTOR_NAMES[NUMBER_OF_RESISTORS] = { "220", "330", "1K", "2K", "5K", "10K", "100K", "1Meg" };
float RESISTOR_VOLTAGES[NUMBER_OF_RESISTORS];

//  The default color for the RGB LED is Blue
ColorRGB  red = {LOW, HIGH, HIGH, false, true, true};
ColorRGB  green = {HIGH, LOW, HIGH, true, false, true};
ColorRGB  blue = {HIGH, HIGH, LOW, true, true, false};
ColorRGB  magenta = {LOW, HIGH, LOW, false, true, false};
ColorRGB  orange = {LOW, LOW, HIGH, false, false, true};
ColorRGB  yellow = {HIGH, LOW, LOW, false, false, true};

#endif