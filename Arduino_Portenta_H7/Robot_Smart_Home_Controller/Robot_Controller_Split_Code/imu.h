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

#ifndef IMU_H
#define IMU_H

const char *HTML_CONTENT_IMU = R""""(
<!DOCTYPE html>
<HTML lang="en">
<HEAD>
  <META charset="UTF-8">
  <META name="viewport" content="width=device-width, initial-scale=1.0">
  <LINK rel="icon" href="data:,">
  <TITLE>PAGE_IMU_TITLE_MARKER</TITLE>
</HEAD>
<BODY>
    <CENTER><H2><SPAN style="color: blue">PAGE_IMU_NAME_MARKER</SPAN></H2></CENTER>
    <CENTER><H3>DATESTAMP_MARKER</H3></CENTER>
    <H3>Request #REQUEST_COUNTER_MARKER<H3>
    <P><H3>Accelerometer: IMU_ACCELEROMETER_MARKER </P>
    <P>Gyroscope: IMU_GYROSCOPE_MARKER </P>
    <P>Magnetometer: IMU_MAGNETOMETER_MARKER</H3></P>
</BODY>
</HTML>
)"""";

#endif
