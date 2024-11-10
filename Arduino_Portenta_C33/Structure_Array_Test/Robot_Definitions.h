/*

    Definitions for the Robot Smart Home Controller

*/
#ifndef ROBOT_DEFINITIONS_H
#define ROBOT_DEFINITIONS_H

#if defined(ARDUINO_PORTENTA_C33)
#include <WiFiC3.h>
#elif defined(ARDUINO_UNOWIFIR4)
#include <WiFiS3.h>
#endif

/***************************************************************************************
    From Robot_Controller.h
****************************************************************************************/

#define ROBOT_DEVICE_NAME           "Robot Smart Home Controller - C33)"
#define ROBOT_DEVICE_VERSION        "0.7.0 [C33]"
#define ROBOT_DEVICE_DATE           "10-Oct-2024"

#define SKETCH_ID_CODE              "Robot Smart Home Controller - C33)"
#define DEVICE_NOT_PRESENT          0xFF

/*
  Sketch control - turn on (true) or off (false) as needed.
*/
#define USING_TCA9548A              true
#define MUX_TCA9548A_PORT           0
#define MUX_TCA9548A_NAME           "Sparkfun I2C Mux"

#define USING_DS3231                true
#define MUX_DS3231_PORT             0
#define MUX_DS3231_NAME             "DS3231 RTC"

#define USING_BNO055                true
#define MUX_BNO055_PORT             1
#define MUX_BNO055_NAME             "BNO055 9-DOF IMU"

#define USING_ENS160                true
#define MUX_ENS160_PORT             2
#define MUX_ENS160_NAME             "ENS160 MOX"

#define USING_SCD40                 false
#define MUX_SCD40_PORT              DEVICE_NOT_PRESENT
#define MUX_SCD40_NAME              "SCD-40 CO2"

#define USING_LSM6DSOX_9DOF         false
#define MUX_LSM6DSOX_9DOF_PORT      DEVICE_NOT_PRESENT
#define MUX_LSM6DSOX_9DOF_NAME      "LSM6DSOX 9-DOF IMU"

#define USING_LIS3MDL               false
#define MUX_LIS3MDL_PORT            DEVICE_NOT_PRESENT
#define MUX_LIS3MDL_NAME            "LIS3MDL Magnetometer"

#define USING_LSM6DSOX_6DOF         false
#define MUX_LSM6DSOX_6DOF_PORT      DEVICE_NOT_PRESENT
#define MUX_LSM6DSOX_6DOF_NAME      "LSM6DSOX 6-DOF_IMU"

#define USING_VEML7700              false
#define MUX_VEML7700_PORT           DEVICE_NOT_PRESENT
#define MUX_VEML7700_NAME           "VEML7700 Lux"

#define USING_SHT45                 false
#define MUX_SHT45_PORT              DEVICE_NOT_PRESENT
#define MUX_SHT45_NAME              "SHT45 Temperature/Humidity"

#if (USING_LSM6DSOX_9DOF)
#define USING_LIS3MDL               true
#define MUX_LIS3MDL_PORT            MUX_LSM6DSOX_9DOF_PORT
#endif

#if (USING_BNO055)
#include <math.h>
#endif

/*
  Controls for the timestamp() function
*/
#define SHOW_TIME_ONLY              false
#define SHOW_FULL_DATE              true

#define SHOW_12_HOURS               false
#define SHOW_24_HOURS               true

#define SHOW_LONG_DATE              true
#define SHOW_SHORT_DATE             false

#define NO_SECONDS                  false
#define SHOW_SECONDS                true

#define UTC_OFFSET_HRS              -7

/*
  Maximum voltage to be read from analog pins in volts
*/
#define MAXIMUM_ANALOG_VOLTAGE      5.0
#define ANALOG_RESOLUTION           4096.0

/*
  Defaults for the blink_rgb() routine
*/
#define DEFAULT_BLINK_RATE_MS       250
#define DEFAULT_NR_CYCLES           1

#define ENS160_MAX_TRIES            5

//  BNO055 stuff
#define BNO055_SAMPLERATE_DELAY_MS  10 //how often to read data from the board
#define PRINT_DELAY_MS              50 // how often to print the data

/*
  Defaults for left_pad()
*/
#define DEFAULT_PAD_LENGTH          2
#define DEFAULT_PAD_STRING          "0"

/*
  Controls for the connect_wifi() routine
*/
#define MAX_NR_CONNECTS             5
#define CONNECTION_TIMEOUT_MS       5000
#define CONNECTION_DELAY_MS         20
#define	SERIAL_BAUDRATE             115200
#define SERIAL_DELAY_MS             5000

///////////////////////////////////////////////
#define WIFI_DELAY_MS               3000
#define SEVENZYYEARS                2208988800UL
//////////////////////////////////////////////

#define PIEZO_BUZZER_PIN            D5          //  PWM

#define LED_RASPI_CONNECT_PIN       GPIO_21     //  Green - Connected to WiFi
#define LED_RASPI_WIFI_PIN          GPIO_20     //  Yellow - Lost WiFi connection
#define LED_RASPI_HALT_PIN          GPIO_26     //  Red - Unable to connect to WiFi

#define ANALOG_POT_PIN              A5
#define ANALOG_POT_NUMBER           0

/*
  Circuit Digital Pins: LEDs
*/
#define LED_WHITE_PIN               A5          //  Works
#define LED_BLUE_PIN                D1          //  Works
#define LED_RED_PIN                 D2          //  Works
#define LED_YELLOW_PIN              D3          //  Works
#define LED_GREEN_PIN               D4          //  Works

/*
  Switches
*/
#define SWITCH_WHITE_PIN            A0          //  A0 Works Raspberry Pi 07 (White)
#define SWITCH_BLUE_PIN             A1          //  A1 Works Raspberry Pi 29 (Blue)
#define SWITCH_RED_PIN              A2          //  6 Works Raspberry Pi 31 (Red)
#define SWITCH_YELLOW_PIN           A3          //  Works   Raspberry Pi 32 (Yellow)
#define SWITCH_GREEN_PIN            A4          //  Works   Raspberry Pi 22 (Green)

/*
  Circuit Analog Pins: Resistors
*/
#define ANALOG_220_PIN              A0          //  Works   Raspberry Pi 07
#define ANALOG_330_PIN              A1          //  Works   Raspberry Pi 29
#define ANALOG_1K_PIN               A2          //  Works   Raspberry Pi 31
#define ANALOG_2K_PIN               A3          //  Works   Raspberry Pi 32
#define ANALOG_5K_PIN               A4          //  Works   Raspberry Pi 22
#define ANALOG_10K_PIN              A5          //  Works
#define ANALOG_100K_PIN             A6          //  Works
#define ANALOG_1M_PIN               A7          //  Works

#define NOTE_DURATION_MS            500
#define NOTE_DELAY_MS               1000

/*
  Sensor names, ports, and page ID
*/
struct Sensor_Hardware_Info {
  String name;
  uint8_t mux_port;
  uint8_t web_page_id;
};

/*
  This holds data read from any three-axis device such as IMUs
*/  
struct Three_Axis {
    float x, y, z;
};

/*
  System Status for hotswapping sensors
*/
struct System_Sensor_Status {
  bool tca9548a = USING_TCA9548A;
  bool bno055 = USING_BNO055;
  bool ds3231 = USING_DS3231;
  bool ens160 = USING_ENS160;
  bool scd40 = USING_SCD40;
  bool sht45 = USING_SHT45;
  bool lsm6dsox_9dof = USING_LSM6DSOX_9DOF;
  bool lsm6dsox_6dof = USING_LSM6DSOX_6DOF;
  bool lis3mdl = USING_LIS3MDL;
  bool veml7700 = USING_VEML7700;
};

struct ENS160_Data {
  bool valid;
  uint16_t  aqi;
  uint16_t  tvoc;
  uint16_t  eCO2;
  uint16_t  hp0;
  uint16_t  hp1;
  uint16_t  hp2;
  uint16_t  hp3;
};

struct LSM6DSOX_Data {
  Three_Axis  accelerometer;
  Three_Axis  gyroscope;
  Three_Axis  magnetometer;
  float celsius;
  float fahrenheit;
};

struct SCD40_Data {
  bool valid;
  uint16_t CO2;
  float celsius;
  float fahrenheit;
  float humidity;
};

struct SHT45_Data {
  float celsius;
  float fahrenheit;
  float humidity;
};

struct BNO055_Data {
  uint8_t system_cal = 0;
  uint8_t gyroscope_cal = 0;
  uint8_t accelerometer_cal = 0;
  uint8_t magnetometer_cal = 0;

  uint16_t heading;
  double x_pos = 0;
  double y_pos = 0;
  double heading_velocity = 0.0;

  Three_Axis accelerometer;
  Three_Axis gyroscope;
  Three_Axis magnetometer;
  Three_Axis linear_acceleration;
  Three_Axis orientation;
  Three_Axis rotation;
  Three_Axis gravity;
};

/*
  Used to hold all sensor data
*/
struct Environment_Data {
  bool initialize = true;
  bool valid = false;

  ENS160_Data ens160;
  LSM6DSOX_Data lsm6dsox;
  BNO055_Data bno055;
  SHT45_Data  sht45;
  SCD40_Data  scd40;
};

/*
  The color to show for the RGB LED in the blink_rgb() routine
*/
struct ColorRGB {
  uint8_t red, green, blue;
  bool redB, greenB, blueB;
};

/***************************************************************************************
    From Web_Server_Control.h
****************************************************************************************/

#define WEB_SERVER_PORT               80
#define WEB_SERVER_DELAY_MS           100
#define WEB_SERVER_PAGE               true

#define INFO_PAGE_BASE                0

#define PAGE_HOME_ID                  INFO_PAGE_BASE
#define PAGE_HOME_NAME                "Robot Smart Home Controller"
#define PAGE_HOME_TITLE               PAGE_HOME_NAME

#define PAGE_ENVIRONMENT_ID           INFO_PAGE_BASE + 1
#define PAGE_ENVIRONMENT_NAME         "Robot Smart Home Controller: Environment"
#define PAGE_ENVIRONMENT_TITLE        PAGE_ENVIRONMENT_NAME

#define PAGE_SWITCHES_ID              INFO_PAGE_BASE + 2
#define PAGE_SWITCHES_NAME            "Robot Smart Home Controller: Switches"
#define PAGE_SWITCHES_TITLE           PAGE_SWITCHES_NAME

#define PAGE_POTENTIOMETER_ID         INFO_PAGE_BASE + 3
#define PAGE_POTENTIOMETER_NAME       "Robot Smart Home Controller: Potentiometer"
#define PAGE_POTENTIOMETER_TITLE      PAGE_POTENTIOMETER_NAME

#define PAGE_LIGHT_ID                 INFO_PAGE_BASE + 4
#define PAGE_LIGHT_NAME               "Robot Smart Home Controller: Light/Lux"
#define PAGE_LIGHT_TITLE              PAGE_LIGHT_NAME

#define PAGE_IMU_BNO055_ID            INFO_PAGE_BASE + 5
#define PAGE_IMU_BNO055_NAME          "Robot Smart Home Controller: BNO055 IMU"
#define PAGE_IMU_BNO055_TITLE         PAGE_IMU_BNO055_NAME

#define PAGE_IMU_LSM6DSOX_ID          INFO_PAGE_BASE + 6
#define PAGE_IMU_LSM6DSOX_NAME        "Robot Smart Home Controller: LSM6DSOX IMU"
#define PAGE_IMU_LSM6DSOX_TITLE       PAGE_IMU_LSM6DSOX_NAME

#define PAGE_NO_DATA_ID               INFO_PAGE_BASE + 7
#define PAGE_NO_DATA_NAME             "Robot Smart Home Controller: NO DATA AVAILABLE"
#define PAGE_NO_DATA_TITLE            PAGE_NO_DATA_NAME

#define PAGE_ERROR_404_ID             INFO_PAGE_BASE + 8
#define PAGE_ERROR_404_NAME           "Page Not Found"
#define PAGE_ERROR_404_TITLE          PAGE_ERROR_404_NAME

#define PAGE_ERROR_405_ID             INFO_PAGE_BASE + 9
#define PAGE_ERROR_405_NAME           "Unknown"
#define PAGE_ERROR_405_TITLE          PAGE_ERROR_405_NAME

#define LAST_INFO_PAGE_ID             PAGE_ERROR_405_ID

#define MAX_NUM_WEB_PAGES             LAST_INFO_PAGE_ID + 1

#define PUT_BASE                      128

#define PUT_STATUS                    (PUT_BASE)
#define PUT_CHANGE                    (PUT-BASE + 1)

struct Web_Page_Info {
  String html;
  String name;
  String title;
};
#endif