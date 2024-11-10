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

#ifndef LIGHT_H
#define LIGHT_H

const char *HTML_CONTENT_LIGHT = R""""(
<!DOCTYPE html>
<HTML lang="en">
<HEAD>
  <META charset="UTF-8">
  <META name="viewport" content="width=device-width, initial-scale=1.0">
  <TITLE>PAGE_LIGHT_TITLE_MARKER</TITLE>
</HEAD>
<BODY>
    <CENTER><H2><SPAN style="color: blue">PAGE_LIGHT_NAME_MARKER</SPAN></H2></CENTER>
    <CENTER><H3>DATESTAMP_MARKER</H3></CENTER>
    <H3>Sequence #SEQUENCE_COUNT_MARKER
    <P>Light: <SPAN STYLE="color: yellow">level</SPAN> = <SPAN STYLE="color: green">LIGHT_VALUE_MARKER</SPAN> lux</H3></P>
</BODY>
</HTML>
)"""";

#endif
