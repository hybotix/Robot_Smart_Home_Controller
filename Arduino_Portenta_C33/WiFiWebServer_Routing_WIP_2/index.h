/*
 * Created by ArduinoGetStarted.com
 *
 * This example code is in the public domain
 *
 * Tutorial page: https://arduinogetstarted.com/tutorials/arduino-web-server-multiple-pages
 */

const char *HTML_CONTENT_HOME = R""""(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <link rel="icon" href="data:,">
    <title>ROBOT_PAGE_NAME_MARKER</title>
</head>
<body>
    <H1><CENTER><span style="color: blue">ROBOT_NAME_MARKER</span></CENTER></H1>
    <H2><CENTER>DATESTAMP_MARKER</CENTER></H2><BR>
    <H3>Request #REQUEST_COUNT_MARKER (SKETCH_CODE_MARKER)</CENTER><BR>
    <ul>
        <li><a href="/led"><span style="color: yellow">LED Status</span</a></li>
        <li><a href="/environment"><span style="color: yellow">Environment Information</span></a></li>
        <li><a href="/door"><span style="color: yellow">Door Status</span></a></li>
    </ul></H3>
</body>
</html>
)"""";
