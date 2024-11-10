/*
 */

#if defined(ARDUINO_PORTENTA_C33)
#include <WiFiC3.h>
#elif defined(ARDUINO_UNOWIFIR4)
#include <WiFiS3.h>
#endif

/*
  Web server controls
*/
#define WEB_SERVER_PORT               80
#define WEB_SERVER_DELAY_MS           1
#define WEB_SERVER_PAGE               true

#define GET_PAGE_BASE                 0

#define PAGE_HOME                     (GET_PAGE_BASE)
#define PAGE_HOME_TITLE               "Robot Smart Home Controller"
#define PAGE_HOME_NAME                PAGE_HOME_TITLE

#define PAGE_ENVIRONMENT              (GET_PAGE_BASE + 1)
#define PAGE_ENVIRONMENT_TITLE        "Robot Smart Home Controller: Environment"
#define PAGE_ENVIRONMENT_NAME         PAGE_ENVIRONMENT_TITLE

#define PAGE_SWITCHES                 (GET_PAGE_BASE + 2)
#define PAGE_SWITCHES_TITLE           "Robot Smart Home Controller: Switches"
#define PAGE_SWITCHES_NAME            PAGE_SWITCHES_TITLE

#define PAGE_POTENTIOMETER            (GET_PAGE_BASE + 3)
#define PAGE_POTENTIOMETER_TITLE      "Robot Smart Home Controller: Potentiometer"
#define PAGE_POTENTIOMETER_NAME       PAGE_POTENTIOMETER_TITLE

#define PAGE_LIGHT                    (GET_PAGE_BASE + 4)
#define PAGE_LIGHT_TITLE              "Robot Smart Home Controller: Light/Lux"
#define PAGE_LIGHT_NAME               PAGE_LIGHT_TITLE

#define PAGE_IMU                      (GET_PAGE_BASE + 5)
#define PAGE_IMU_TITLE                "Robot Smart Home Controller: IMU"
#define PAGE_IMU_NAME                 PAGE_IMU_TITLE

#define PAGE_ERROR_404                -1
#define PAGE_ERROR_405                -2

#define PUT_BASE                      128

#define PUT_STATUS                    (PUT_BASE)
#define PUT_CHANGE                    (PUT-BASE + 1)

WiFiServer server(WEB_SERVER_PORT);
