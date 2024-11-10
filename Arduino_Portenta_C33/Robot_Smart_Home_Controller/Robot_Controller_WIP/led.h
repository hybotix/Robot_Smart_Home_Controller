/*
 * Created by ArduinoGetStarted.com
 *
 * This example code is in the public domain
 *
 * Tutorial page: https://arduinogetstarted.com/tutorials/arduino-web-server-multiple-pages
 */

const char *HTML_CONTENT_LED = R""""(
<!DOCTYPE html>
<HTML lang="en">
<HEAD>
  <META charset="UTF-8">
  <META name="viewport" content="width=device-width, initial-scale=1.0">
  <LINK rel="icon" href="data:,">
  <TITLE>PAGE_LED_TITLE_MARKER</TITLE>
</HEAD>
<BODY>
    <CENTER><H1><SPAN style="color: blue">PAGE_LED_NAME_MARKER</SPAN></H1></CENTER>
    <CENTER><H3>DATESTAMP_MARKER</H3></CENTER>
    <H3>Request #REQUEST_COUNTER_MARKER<H3>
    <P>LED State: <SPAN style="color: red;">LED_STATE_MARKER</SPAN></P>
</BODY>
</HTML>
)"""";
