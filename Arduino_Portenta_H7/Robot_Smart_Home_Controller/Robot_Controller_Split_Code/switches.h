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

#ifndef SWITCHES_H
#define SWITCHES_H

const char *HTML_CONTENT_SWITCHES = R""""(
<!DOCTYPE html>
<HTML lang="en">
<HEAD>
  <META charset="UTF-8">
  <META name="viewport" content="width=device-width, initial-scale=1.0">
  <TITLE>PAGE_SWITCHES_TITLE_MARKER</TITLE>
</HEAD>
<BODY>
    <CENTER><H2><SPAN style="color: blue">PAGE_SWITCHES_NAME_MARKER</SPAN></H2></CENTER>
    <CENTER><H3>DATESTAMP_MARKER</H3></CENTER>
    <H3>Request #REQUEST_COUNTER_MARKER<H3>
    <P>Switch States: SWITCHES_TEXT_MARKER</H3></P>
</BODY>
</HTML>
)"""";

#endif
