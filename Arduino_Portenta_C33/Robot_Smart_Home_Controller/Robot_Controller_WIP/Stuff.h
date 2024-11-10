
#define SKETCH_ID_CODE "WiFi Web Server Separate Files"

//  Sketch control - turn on (true) or off (false) as needed.
#define USING_SHT45_TEMP true
#define USING_LSM6DSOX_LIS3MDL_IMU false
#define USING_LIS3MDL_MAG false
#define USING_VEML_LUX false

#if (USING_LSM6DSOX_LIS3MDL_IMU)
#define USING_LIS3MDL_MAG true
#endif

//  Sketch control - turn on (true) or off (false) as needed.
#define USING_SHT45_TEMP true
#define USING_LSM6DSOX_LIS3MDL_IMU false
#define USING_LIS3MDL_MAG false
#define USING_VEML_LUX false

#if (USING_LSM6DSOX_LIS3MDL_IMU)
#define USING_LIS3MDL_MAG true
#endif

#define WIFI_DELAY_MS 3000
#define CONNECTION_TIMEOUT_MS 2000
#define WEB_SERVER_PORT 80
#define UTC_OFFSET_HRS  -8
#define DEFAULT_BLINK_RATE_MS 250
#define DEFAULT_NR_CYCLES 1
#define SEVENZYYEARS 2208988800UL
#define CONNECTION_DELAY_MS 20

struct Three_Axis {
    float x, y, z;
};

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

struct ColorRGB {
  uint8_t red, green, blue;
  bool redB, greenB, blueB;
};
