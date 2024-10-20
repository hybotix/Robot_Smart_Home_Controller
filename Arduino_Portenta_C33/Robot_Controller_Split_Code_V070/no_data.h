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

#ifndef NO_DATA_H
#define NO_DATA_H

const char *HTML_CONTENT_NO_DATA = R""""(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="icon" href="data:,">
    <title>PAGE__NO_DATA_TITLE_MARKER</title>
</head>
<body>
    <H2><CENTER><SPAN style="color: blue">PAGE_NO_DATA_NAME_MARKER</SPAN></CENTER></H2>
    <H3><CENTER>DATESTAMP_MARKER</CENTER></H2><BR>
    <H3><CENTER>Sequence #SEQUENCE_COUNT_MARKER (SKETCH_CODE_MARKER)</CENTER><BR><BR>

    <H3><CENTER>No data is available for page PAGE_NAME_MARKER</CENTER></H3>
</body>
</html>
)"""";

#endif
