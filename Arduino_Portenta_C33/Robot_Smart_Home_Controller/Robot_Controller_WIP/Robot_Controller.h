//#include  "Hat_Carrier-Raspberry_Pi_Pins.h"

#if defined(ARDUINO_PORTENTA_C33)
#include <WiFiC3.h>
#elif defined(ARDUINO_UNOWIFIR4)
#include <WiFiS3.h>
#endif

#define ROBOT_DEVICE_NAME           "Robot Smart Home Controller WIP"
#define ROBOT_DEVICE_VERSION        "0.4.1"
#define ROBOT_DEVICE_DATE           "17-Feb-2024"

#define SCRIPT_NAME_TEXT            "Smart Home Controller Robot"

//  Sketch control - turn on (true) or off (false) as needed.
#define USING_SHT45_TEMP            true
#define USING_LSM6DSOX_LIS3MDL_IMU  false
#define USING_LIS3MDL_MAG           false
#define USING_VEML_LUX              false

#if (USING_LSM6DSOX_LIS3MDL_IMU)
#define USING_LIS3MDL_MAG           true
#endif

/*
  Sketch control - turn on (true) or off (false) as needed.
*/
#define USING_SHT45_TEMP            true
#define USING_LSM6DSOX_LIS3MDL_IMU  false
#define USING_LIS3MDL_MAG           false
#define USING_VEML_LUX              false

#if (USING_LSM6DSOX_LIS3MDL_IMU)
#define USING_LIS3MDL_MAG           true
#endif

/*
  Controls for the timestamp() function
*/
#define SHOW_TIME_ONLY            false
#define SHOW_FULL_DATE            true

#define SHOW_12_HOURS             true
#define SHOW_24_HOURS             false

#define SHOW_LONG_DATE            true
#define SHOW_SHORT_DATE           false

#define SHOW_NORMAL_TIME          false
#define SHOW_SECONDS              true

#define UTC_OFFSET_HRS            -8

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

///////////////////////////////////////////////
#define WIFI_DELAY_MS             3000
#define SEVENZYYEARS              2208988800UL
//////////////////////////////////////////////

#define SKETCH_ID_CODE            ROBOT_DEVICE_NAME

/*
  Circuit Digital Pins: LEDs
*/
#define LED_WHITE_PIN             D1 
#define LED_BLUE_PIN              D2
#define LED_RED_PIN               D3
#define LED_YELLOW_PIN            D4
#define LED_GREEN_PIN             D5

/*
  Circuit Analog Pins: Resistors
*/
#define ANALOG_220_PIN            A0
#define ANALOG_330_PIN            A1
#define ANALOG_1K_PIN             A2
#define ANALOG_2K_PIN             A3
#define ANALOG_5K_PIN             A4
#define ANALOG_10K_PIN            A5
#define ANALOG_100K_PIN           A6
#define ANALOG_1M_PIN             A7

#define SWITCH_0_PIN              D6
//#define SWITCH_1_PIN              53                    //  RP08 | RP08_GPIO14 | D53
//#define SWITCH_2_PIN              54                    //  RP10 | RP10_GPIO15 | D54

enum {
  White,
  Blue,
  Red,
  Yellow,
  Green
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

char ssid[] = WIFI_SSID;  // change your network SSID (name)
char passwd[] = WIFI_PASSWD;   // change your network password (use for WPA, or use as key for WEP)

String long_months[12] = { "January", "February", "March", "April", "May", "June", "July",
  "August", "September", "October", "November", "December" };

String week_days[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", 
  "Thursday", "Friday", "Saturday"};

int wifi_status = WL_IDLE_STATUS;
uint16_t request_count = 0;
uint16_t looper = 0;

uint8_t SWITCH_PINS[1] = { SWITCH_0_PIN };  //, SWITCH_1_PIN, SWITCH_2_PIN };
#define NUMBER_OF_SWITCHES (sizeof(SWITCH_PINS) / sizeof(uint8_t))
String SWITCH_NAMES[NUMBER_OF_SWITCHES] = { "One" }; //, "Two", "Three" };
bool switch_readings[NUMBER_OF_SWITCHES] = { false };  //, false, false, };

uint8_t LED_PINS[5] = { LED_WHITE_PIN, LED_BLUE_PIN, LED_RED_PIN, LED_YELLOW_PIN, LED_GREEN_PIN };
#define NUMBER_OF_LEDS (sizeof(LED_PINS) / sizeof(uint8_t))
String LED_NAMES[NUMBER_OF_LEDS] = { "1", "2", "3", "4", "5" };

uint8_t RESISTOR_PINS[8] = { ANALOG_220_PIN, ANALOG_330_PIN, ANALOG_1K_PIN, ANALOG_2K_PIN, 
  ANALOG_5K_PIN, ANALOG_10K_PIN, ANALOG_100K_PIN, ANALOG_1M_PIN };

#define NUMBER_OF_RESISTORS ((sizeof(RESISTOR_PINS) / sizeof(uint8_t)))
int resistor_readings[NUMBER_OF_RESISTORS];
String RESISTOR_NAMES[NUMBER_OF_RESISTORS] = { "220", "330", "1K", "2K", "5K", "10K", "100K", "1Meg" };
float resistor_voltages[NUMBER_OF_RESISTORS];

//  The default color for the RGB LED is Blue
ColorRGB  red = {LOW, HIGH, HIGH, false, true, true};
ColorRGB  green = {HIGH, LOW, HIGH, true, false, true};
ColorRGB  blue = {HIGH, HIGH, LOW, true, true, false};
ColorRGB  magenta = {LOW, HIGH, LOW, false, true, false};
