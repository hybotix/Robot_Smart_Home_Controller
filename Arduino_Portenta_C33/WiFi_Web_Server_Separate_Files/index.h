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
    <title>Hybrid Robotics' Livingroom</title>
</head>
<body>
    <H1>><CENTER>Welcome to <span style=\"color: blue;\">Hybrid Robotics'</span> Livingroom</CENTER></H1>
    <ul>
        <li><a href="/led">LED Page</a></li>
        <li><a href="/temperature">Temperature Page</a></li>
        <li><a href="/door">Door Page</a></li>
    </ul>
</body>
</html>
)"""";
