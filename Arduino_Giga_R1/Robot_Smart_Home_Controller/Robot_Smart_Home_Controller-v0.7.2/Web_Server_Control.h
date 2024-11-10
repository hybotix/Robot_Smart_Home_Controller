/*
 *
 *  Copyright (c) by Dale Weber <hybotics@hybotics.dev> 2024
 */

#ifndef WEB_SERVER_CONTROL_H
#define WEB_SERVER_CONTROL_H

#include  "Robot_Definitions.h"
#include  "environment.h"
#include  "error_404.h"
#include  "error_405.h"
#include  "index.h"
#include  "imu.h"
#include  "imu_bno055.h"
#include  "light.h"
#include  "no_data.h"
#include  "potentiometer.h"
#include  "switches.h"

WiFiServer server(WEB_SERVER_PORT);

Web_Page_Info web_pages[MAX_NUM_WEB_PAGES] = {
  { String(HTML_CONTENT_HOME), PAGE_HOME_NAME, PAGE_HOME_TITLE },
  { String(HTML_CONTENT_ENVIRONMENT), PAGE_ENVIRONMENT_NAME, PAGE_ENVIRONMENT_TITLE },
  { String(HTML_CONTENT_SWITCHES), PAGE_SWITCHES_NAME, PAGE_SWITCHES_TITLE },
  { String(HTML_CONTENT_POTENTIOMETER), PAGE_POTENTIOMETER_NAME, PAGE_POTENTIOMETER_TITLE },
  { String(HTML_CONTENT_LIGHT), PAGE_LIGHT_NAME, PAGE_LIGHT_TITLE },
  { String(HTML_CONTENT_IMU_BNO055), PAGE_IMU_BNO055_NAME, PAGE_IMU_BNO055_TITLE },
  { String(HTML_CONTENT_IMU), PAGE_IMU_NAME, PAGE_IMU_TITLE },
  { String(HTML_CONTENT_NO_DATA), PAGE_NO_DATA_NAME, PAGE_NO_DATA_TITLE },
  { String(HTML_CONTENT_404), PAGE_ERROR_404_NAME, PAGE_ERROR_404_TITLE },
  { String(HTML_CONTENT_405), PAGE_ERROR_405_NAME, PAGE_ERROR_405_TITLE }
};

#endif