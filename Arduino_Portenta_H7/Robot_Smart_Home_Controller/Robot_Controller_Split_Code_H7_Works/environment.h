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
    <H3>Sequence #SEQUENCE_COUNT_MARKER</H3>
    <H3>Temperature (SHT45): <span style="color: green">SHT45_FAHRENHEIT_MARKER &deg;F</span> (<span style="color: yellow">SHT45_CELSIUS_MARKER &deg;C</span>), Humidity: <span style="color: magenta">SHT45_HUMIDITY_MARKER% rH</span><BR /></H3>
    <H3>Temperature (SCD-40): <span style="color: green">SCD40_FAHRENHEIT_MARKER &deg;F</span> (<span style="color: yellow">SCD40_CELSIUS_MARKER &deg;C</span>), Humidity: <span style="color: magenta">SCD40_HUMIDITY_MARKER% rH</span><BR /></H3>
    <H3>CO2 Level: <span style="color: green">SCD40_CO2_MARKER</span><span style="color: white"> ppm</span><BR /></H3>
    <H3>AQI: <span style="color: green">ENS160_AQI_MARKER</span><span style="color: white">, </span>TVOC: <span style="color: green">ENS160_TVOC_MARKER</span><span style="color: white"> ppb, eCO2 Level: </span><span style="color: green">ENS160_ECO2_MARKER<span style="color: white"> ppm</span><BR /></H3>   
    <H3>HP0: <span style="color: green">ENS160_HP0_MARKER</span><span style="color: white"> Ohms, </span>HP1: <span style="color: green">ENS160_HP1_MARKER</span><span style="color: white"> Ohms, HP2: </span><span style="color: green">ENS160_HP2_MARKER</span><span style="color: white"> Ohms, HP3: </span><span style="color: green">ENS160_HP3_MARKER<span style="color: white"> Ohms</span><BR /></H3>   
</HTML>
)"""";

#endif
