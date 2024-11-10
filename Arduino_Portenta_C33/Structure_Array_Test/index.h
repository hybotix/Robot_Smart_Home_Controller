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

#ifndef INDEX_H
#define INDEX_H

const char *HTML_CONTENT_HOME = R""""(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="icon" href="data:,">
    <title>PAGE_HOME_TITLE_MARKER</title>
</head>
<body>
    <H2><CENTER><SPAN style="color: blue">PAGE_HOME_NAME_MARKER</SPAN></CENTER></H2>
    <H3><CENTER>DATESTAMP_MARKER</CENTER></H2><BR>
    <H3>Sequence #SEQUENCE_COUNT_MARKER (SKETCH_CODE_MARKER)</CENTER><BR>
    <ul>
        <li><a href="/environment"><span style="color: yellow">Environment Information</span></a></li>
        <li><a href="/switches"><span style="color: yellow">Switch Status</span></a></li>
        <li><a href="/potentiometer"><span style="color: yellow">Potentiometer Data</span></a></li>
        <li><a href="/light"><span style="color: yellow">Light level</span></a></li>
        <li><a href="/imu_bno055"><span style="color: yellow">BNO055 IMU Data</span></a></li>
        <li><a href="/imu_lsm6dsox"><span style="color: yellow">LSM6DSOX IMU Data</span></a></li>
    </ul></H3>
</body>
</html>
)"""";

#endif
