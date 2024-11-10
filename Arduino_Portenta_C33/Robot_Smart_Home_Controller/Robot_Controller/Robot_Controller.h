
#if defined(ARDUINO_PORTENTA_C33)
#include <WiFiC3.h>
#elif defined(ARDUINO_UNOWIFIR4)
#include <WiFiS3.h>
#endif

#define ROBOT_DEVICE_NAME "Robot Smart Home Controller"
#define ROBOT_DEVICE_VERSION "0.3.1"
#define ROBOT_DEVICE_DATE "07-Feb-2024"

//  Sketch control - turn on (true) or off (false) as needed.
#define USING_SHT45_TEMP true
#define USING_LSM6DSOX_LIS3MDL_IMU false
#define USING_LIS3MDL_MAG false
#define USING_VEML_LUX false

#if (USING_LSM6DSOX_LIS3MDL_IMU)
#define USING_LIS3MDL_MAG true
#endif

#define SKETCH_ID_CODE "Robot Smart Home Controller 0.3.1, 07-Feb-2024"

/*
  Sketch control - turn on (true) or off (false) as needed.
*/
#define USING_SHT45_TEMP true
#define USING_LSM6DSOX_LIS3MDL_IMU false
#define USING_LIS3MDL_MAG false
#define USING_VEML_LUX false

#if (USING_LSM6DSOX_LIS3MDL_IMU)
#define USING_LIS3MDL_MAG true
#endif

#define SCRIPT_NAME_TEXT "Smart Home Controller Robot"
/*
  Controls for the timestamp() function
*/
#define SHOW_24_HOURS false
#define SHOW_LONG_DATE true
#define SHOW_SECONDS true

/*
  Web server controls
*/
#define WEB_SERVER_PORT 80
#define WEB_SERVER_DELAY_MS 7000
#define WEB_SERVER_PAGE true

#define UTC_OFFSET_HRS  -8

/*
  Defaults for the blink_rgb() routine
*/
#define DEFAULT_BLINK_RATE_MS 250
#define DEFAULT_NR_CYCLES 1

/*
  Controls for the connect_wifi() routine
*/
#define MAX_NR_CONNECTS 5
#define CONNECTION_TIMEOUT_MS 2000
#define CONNECTION_DELAY_MS 20

/*
  Routing controls for pages
*/
#define PAGE_HOME 0
#define PAGE_HOME_NAME "Robot Smart Home Controller: Home"
#define PAGE_ENVIRONMENT 1
#define PAGE_ENVIRONMENT_NAME "Robot Smart Home Controller: Environment"
#define PAGE_DOOR 2
#define PAGE_DOOR_NAME "Robot Smart Home Controller: Door Status"
#define PAGE_LED 3
#define PAGE_LED_NAME "Robot Smart Home Controller: LED Change"

#define PAGE_ERROR_404 -1
#define PAGE_ERROR_405 -2

/////////////////////////////////
#define WIFI_DELAY_MS 3000
#define SEVENZYYEARS 2208988800UL
/////////////////////////////////

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
