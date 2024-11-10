#include  "Robot_Controller.h"

struct Web_Page_Info {
  String html;
  String name;
  String title;
};

Web_Page_Info web_pages[MAX_NUM_WEB_PAGES] = {
  { String(HTML_CONTENT_HOME), PAGE_HOME_NAME, PAGE_HOME_TITLE },
  { String(HTML_CONTENT_ENVIRONMENT), PAGE_ENVIRONMENT_NAME, PAGE_ENVIRONMENT_TITLE },
  { String(HTML_CONTENT_SWITCHES), PAGE_SWITCHES_NAME, PAGE_SWITCHES_TITLE },
  { String(HTML_CONTENT_POTENTIOMETER), PAGE_POTENTIOMETER_NAME, PAGE_POTENTIOMETER_TITLE },
  { String(HTML_CONTENT_LIGHT), PAGE_LIGHT_NAME, PAGE_LIGHT_TITLE },
  { String(HTML_CONTENT_IMU_BNO055), PAGE_IMU_BNO055_NAME, PAGE_IMU_BNO055_TITLE },
  { String(HTML_CONTENT_IMU_LSM6DSOX), PAGE_IMU_LSM6DSOX_NAME, PAGE_IMU_LSM6DSOX_TITLE },
  { String(HTML_CONTENT_NO_DATA), PAGE_NO_DATA_NAME, PAGE_NO_DATA_TITLE },
  { String(HTML_CONTENT_404), PAGE_ERROR_404_NAME, PAGE_ERROR_404_TITLE },
  { String(HTML_CONTENT_405), PAGE_ERROR_405_NAME, PAGE_ERROR_405_TITLE }
};

/*
  Initialize the HTML for web pages.

  Doing this pre-initialization of the HTML saves some replacements in the
    web server code. These are things that do not change in the web page but
    still have to be easily changeable. This should allow the server to execute a
    bit faster.

  HTML is in String array Web_Page_Info[]
*/
void init_html (Web_Page_Info *pages, uint8_t max_info_pages=MAX_NUM_WEB_PAGES) {
  uint8_t page_nr;
  String html, message;

  Serial.println();
  Serial.println("Initializing HTML");

  for (page_nr=0; page_nr < max_info_pages; page_nr++) {
    Serial.print("page_nr = ");
    Serial.print(page_nr);
    Serial.print(", '");

    switch (page_nr) {
      case PAGE_HOME_ID:                             //  0
        Serial.print(PAGE_HOME_TITLE);

        pages[page_nr].name = PAGE_HOME_NAME;
        pages[page_nr].title = PAGE_HOME_TITLE;

        html = String(HTML_CONTENT_HOME);
        html.replace("PAGE_HOME_TITLE_MARKER", PAGE_HOME_TITLE);
        html.replace("PAGE_HOME_NAME_MARKER", PAGE_HOME_NAME);
        html.replace("SKETCH_CODE_MARKER", SKETCH_ID_CODE);

        pages[PAGE_HOME_ID].html = html;
        break;
      case PAGE_ENVIRONMENT_ID:                    //  1
        Serial.print(PAGE_ENVIRONMENT_TITLE);

        pages[page_nr].name = PAGE_ENVIRONMENT_NAME;
        pages[page_nr].title = PAGE_ENVIRONMENT_TITLE;

        html = String(HTML_CONTENT_ENVIRONMENT);
        html.replace("PAGE_ENVIRONMENT_TITLE_MARKER", PAGE_ENVIRONMENT_TITLE);
        html.replace("PAGE_ENVIRONMENT_NAME_MARKER", PAGE_ENVIRONMENT_NAME);

        pages[PAGE_ENVIRONMENT_ID].html = html;
        break;
      case PAGE_SWITCHES_ID:                      //  2
        Serial.print(PAGE_SWITCHES_TITLE);

        pages[page_nr].name = PAGE_SWITCHES_NAME;
        pages[page_nr].title = PAGE_SWITCHES_TITLE;

        html = String(HTML_CONTENT_SWITCHES);
        html.replace("PAGE_SWITCHES_TITLE_MARKER", PAGE_SWITCHES_TITLE);
        html.replace("PAGE_SWITCHES_NAME_MARKER", PAGE_SWITCHES_NAME);

        pages[PAGE_SWITCHES_ID].html = html;
        break;
      case PAGE_POTENTIOMETER_ID:                //  3
        Serial.print(PAGE_POTENTIOMETER_TITLE);

        pages[page_nr].name = PAGE_POTENTIOMETER_NAME;
        pages[page_nr].title = PAGE_POTENTIOMETER_TITLE;

        html = String(HTML_CONTENT_POTENTIOMETER);
        html.replace("PAGE_POTENTIOMETER_TITLE_MARKER", PAGE_POTENTIOMETER_TITLE);
        html.replace("PAGE_POTENTIOMETER_NAME_MARKER", PAGE_POTENTIOMETER_NAME);

        pages[PAGE_POTENTIOMETER_ID].html = html;
        break;
      case PAGE_LIGHT_ID:                       //  4
        Serial.print(PAGE_LIGHT_TITLE);

        pages[page_nr].name = PAGE_LIGHT_NAME;
        pages[page_nr].title = PAGE_LIGHT_TITLE;

        html = String(HTML_CONTENT_LIGHT);
        html.replace("PAGE_LIGHT_TITLE_MARKER", PAGE_LIGHT_TITLE);
        html.replace("PAGE_LIGHT_NAME_MARKER", PAGE_LIGHT_NAME);

        pages[PAGE_LIGHT_ID].html = html;
        break;
      case PAGE_IMU_BNO055_ID:                 //  5
        Serial.print(PAGE_IMU_BNO055_TITLE);

        pages[page_nr].name = PAGE_IMU_BNO055_NAME;
        pages[page_nr].title = PAGE_IMU_BNO055_TITLE;

        html = String(HTML_CONTENT_IMU_BNO055);
        html.replace("PAGE_IMU_BNO055_TITLE_MARKER", PAGE_IMU_BNO055_TITLE);
        html.replace("PAGE_IMU_BNO055_NAME_MARKER", PAGE_IMU_BNO055_NAME);

        pages[PAGE_IMU_BNO055_ID].html = html;
       break;
      case PAGE_IMU_LSM6DSOX_ID:               //  6
        Serial.print(PAGE_IMU_LSM6DSOX_TITLE);

        pages[page_nr].name = PAGE_IMU_LSM6DSOX_NAME;
        pages[page_nr].title = PAGE_IMU_LSM6DSOX_TITLE;

        html = String(HTML_CONTENT_IMU_LSM6DSOX);
        html.replace("PAGE_IMU_BNO055_TITLE_MARKER", PAGE_IMU_LSM6DSOX_TITLE);
        html.replace("PAGE_IMU_BNO055_NAME_MARKER", PAGE_IMU_LSM6DSOX_NAME);

        pages[PAGE_IMU_LSM6DSOX_ID].html = html;
       break;
      case PAGE_NO_DATA_ID:                    //  7
        Serial.print(PAGE_NO_DATA_TITLE);

        pages[page_nr].name = PAGE_NO_DATA_NAME;
        pages[page_nr].title = PAGE_NO_DATA_TITLE;

        html = String(HTML_CONTENT_NO_DATA);
        html.replace("PAGE_NO_DATA_TITLE_MARKER", PAGE_NO_DATA_TITLE);
        html.replace("PAGE_NO_DATA_NAME_MARKER", PAGE_NO_DATA_NAME);
        pages[PAGE_NO_DATA_ID].html = html;
      default:
        message = "Web Page ID Error: page_nr = " + String(page_nr) + ", max_pages = " + String(max_info_pages) + "!";
        //halt(message);
        break;
    }

    Serial.println("'");
  }
}

/*
  Set up the empty page for when there is no data - sensor is not
    available.
*/
String set_empty_page (QWIICMUX mx, Web_Page_Info *pages, uint16_t sequence_nr) {
  String date_time, html;

  html =  pages[PAGE_NO_DATA_ID].html;
  date_time = "10/21/2024 at 23:06:00";
  //date_time = timestamp(mx, pages[PAGE_NO_DATA_ID].name, SHOW_FULL_DATE, SHOW_12_HOURS, SHOW_LONG_DATE, SHOW_SECONDS);
  Serial.println(pages[PAGE_NO_DATA_ID].name);
  html.replace("PAGE_NAME_MARKER", pages[PAGE_NO_DATA_ID].name);
  html.replace("DATESTAMP_MARKER", date_time);
  html.replace("SEQUENCE_COUNT_MARKER", String(sequence_nr));

  return html;
}

void setup() {
  String HTML;
  uint8_t page_nr;

  //  Initialize serial and wait for the port to open:
  Serial.begin(SERIAL_BAUDRATE);

  while(!Serial) {
    delay(SERIAL_DELAY_MS);
  }

  //  Initialize the static HTML

  init_html(web_pages, MAX_NUM_WEB_PAGES);

  HTML = set_empty_page(mux, web_pages, 1);

  for(page_nr = 0; page_nr < MAX_NUM_WEB_PAGES - 1; page_nr++) {
    Serial.print("Page = ");
    Serial.print(page_nr);
    Serial.print(", Name = '");
    Serial.print(web_pages[page_nr].name);
    Serial.println("'");
  }

  Serial.println();

  for(page_nr = 0; page_nr < MAX_NUM_WEB_PAGES - 1; page_nr++) {
    Serial.print("Page = ");
    Serial.print(page_nr);
    Serial.print(", Title = '");
    Serial.print(web_pages[page_nr].title);
    Serial.println("'");
  }
}

void loop() {
  //  Move along - there is nothing to do here...
}
