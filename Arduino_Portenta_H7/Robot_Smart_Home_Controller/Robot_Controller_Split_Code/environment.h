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

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

const char *HTML_CONTENT_ENVIRONMENT = R""""(
<!DOCTYPE HTML>
<HTML>
  <HEAD>
    <TITLE>PAGE_ENVIRONMENT_TITLE_MARKER</TITLE>
    <META name="viewport" content="width=device-width, initial-scale=0.7, maximum-scale=0.7">
    <META charset="utf-8">
  </HEAD>
  <BODY>
    <H2><CENTER><span style="color: blue">PAGE_ENVIRONMENT_NAME_MARKER</span></H2></CENTER>
    <H3><CENTER>DATESTAMP_MARKER</CENTER></H3>
    <H3>Request #REQUEST_COUNTER_MARKER</H3>
    <H3>Temperature: <span style="color: green">FAHRENHEIT_MARKER &deg;F</span> (<span style="color: yellow">CELSIUS_MARKER &deg;C</span>)<BR /></H3>
    <H3>Humidity: <span style="color: magenta">HUMIDITY_MARKER% rH</span><BR /></H3>
  </BODY>
</HTML>
)"""";

#endif
