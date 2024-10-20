/*
 * Created by ArduinoGetStarted.com
 *
 * This example code is in the public domain
 *
 * Tutorial page: https://arduinogetstarted.com/tutorials/arduino-web-server-multiple-pages
  *
 *  Modifed by Dale Weber <hybotics@hybotics.dev>
 *
 *  Copyright (c) by Dale Weber <hybotics@hybotics.dev> 2024
*/

#ifndef IMU_BNO055_H
#define IMU_BNO055_H

const char *HTML_CONTENT_IMU_BNO055 = R""""(
<!DOCTYPE html>
<HTML lang="en">
<HEAD>
  <META charset="UTF-8">
  <META name="viewport" content="width=device-width, initial-scale=1.0">
  <LINK rel="icon" href="data:,">
  <TITLE>PAGE_IMU_BNO055_TITLE_MARKER</TITLE>
</HEAD>
<BODY>
    <CENTER><H2><SPAN style="color: blue">PAGE_IMU_BNO055_NAME_MARKER</SPAN></H2></CENTER>
    <CENTER><H3>DATESTAMP_MARKER</H3></CENTER>
    <H3>Sequence #SEQUENCE_COUNT_MARKER</H3>
    <H3>
    <P>Accelerometer: IMU_ACCELEROMETER_MARKER </P>
    <P>Gyroscope: IMU_GYROSCOPE_MARKER </P>
    <P>Magnetometer: IMU_MAGNETOMETER_MARKER</H3></P>
    <P>Rotation: IMU_ROTATION_MARKER</H3></P>
    <P>Orientation: IMU_ORIENTATION_MARKER</H3></P>
    <P>Linear Acceleration: IMU_LINEAR_ACCEL_MARKER</H3></P>
    <P>Gravity: IMU_GRAVITY_MARKER</H3></P>
</BODY>
</HTML>
)"""";

#endif
