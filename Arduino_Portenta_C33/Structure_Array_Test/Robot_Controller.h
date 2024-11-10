/*
 *
 *  Copyright (c) by Dale Weber <dalew@hybridrobotix.com> 2024
*/
#ifndef ROBOT_CONTROLLER_H
#define ROBOT_CONTROLLER_H

#include  <Adafruit_Sensor.h>

#include  "Library_Includes.h"
#include  "Robot_Definitions.h"
#include  "Arduino_HAT_Carrier.h"
#include  "Secrets.h"

char ssid[] = WIFI_SSID;
char passwd[] = WIFI_PASSWD;
bool connected = true;        //  True = Connected to WiFi

/*
  For the BNO055 IMU
*/
double ACCEL_VEL_TRANSITION =  (double)(BNO055_SAMPLERATE_DELAY_MS) / 1000.0;
double ACCEL_POS_TRANSITION = 0.5 * ACCEL_VEL_TRANSITION * ACCEL_VEL_TRANSITION;
//   Trig functions require radians, BNO055 outputs degrees
double DEG_2_RAD = 0.01745329251;
//  End BNO055 stuff

int MUSIC_NOTES[12] = { 261, 277, 293, 311, 329, 349, 369, 392, 415, 440, 466, 493 };
uint8_t MAX_NR_NOTES = 12;

char days_of_the_week[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

String long_months[12] = { "January", "February", "March", "April", "May", "June", "July",
  "August", "September", "October", "November", "December" };

String week_days[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", 
  "Thursday", "Friday", "Saturday"};

int wifi_status;
uint16_t request_count = 0;
uint16_t looper = 0;
IPAddress ip_addr = "*";
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
ColorRGB  Red = {LOW, HIGH, HIGH, false, true, true};
ColorRGB  Green = {HIGH, LOW, HIGH, true, false, true};
ColorRGB  Blue = {HIGH, HIGH, LOW, true, true, false};
ColorRGB  Magenta = {LOW, HIGH, LOW, false, true, false};
ColorRGB  Orange = {LOW, LOW, HIGH, false, false, true};
ColorRGB  Yellow = {HIGH, LOW, LOW, false, false, true};

#endif