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

#ifndef POTENTIOMETER_H
#define POTENTIOMETER_H

const char *HTML_CONTENT_POTENTIOMETER = R""""(
<!DOCTYPE html>
<HTML lang="en">
<HEAD>
  <META charset="UTF-8">
  <META name="viewport" content="width=device-width, initial-scale=1.0">
  <LINK rel="icon" href="data:,">
  <TITLE>PAGE_POTENTIOMETER_TITLE_MARKER</TITLE>
</HEAD>
<BODY>
    <CENTER><H2><SPAN style="color: blue">PAGE_POTENTIOMETER_NAME_MARKER</SPAN></H2></CENTER>
    <CENTER><H3>DATESTAMP_MARKER</H3></CENTER>
    <H3>Sequence #SEQUENCE_COUNT_MARKER<H3>
    <P>Potentiometer: <SPAN STYLE="color: yellow">Reading</SPAN> = <SPAN STYLE="color: green">POTENTIOMETER_READING_MARKER</SPAN>, <SPAN STYLE="color: yellow">Voltage</SPAN> = <SPAN STYLE="color: green">POTENTIOMETER_VOLTAGE_MARKER</SPAN> POTENTIOMETER_UNITS_MARKER</H3></P>
</BODY>
</HTML>
)"""";

#endif
