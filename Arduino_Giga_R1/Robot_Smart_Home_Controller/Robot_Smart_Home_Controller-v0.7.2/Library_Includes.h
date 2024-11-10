/******************************************************************************************
 * Code from the original sketch was created by ArduinoGetStarted.com
 *
 * This example code is in the public domain
 *
 * Tutorial page: https://arduinogetstarted.com/tutorials/arduino-web-server-multiple-pages
 *******************************************************************************************
 *  Heavily modifed and extended by Dale Weber <dalew@hybridrobotix.io>
 *
 *  14-Oct-2024: Code has been completely reorganized into sections and function names
 *    have been changed to better reflect their purpose.
 *
 *  This is the control sketch for the Robot Smart Home Controller
 *  Copyright (c) by Dale Weber <hybotics@hybotics.dev> 2024
 ******************************************************************************************/

/*****************************************************************************************
    Library includes
******************************************************************************************/

#ifndef LIBRARY_INCLUDES_H
#define LIBRARY_INCLUDES_H

/*************************
  Internal Arduino Libraries
*************************/
#include  <Wire.h>

/*************************
  Arduino Libraries
*************************/

#include "Arduino_LSM6DSOX.h"

/*************************
  Third-Party Libraries
*************************/
#include <SparkFun_I2C_Mux_Arduino_Library.h>
QWIICMUX mux;

//  DS3231 Real Time Clock - Adafruit Fork
#include "DS3232.h"
DS3232 rtc;

#include <Adafruit_LIS3MDL.h>
Adafruit_LIS3MDL lis3;

#include  <Adafruit_Sensor.h>

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
#endif