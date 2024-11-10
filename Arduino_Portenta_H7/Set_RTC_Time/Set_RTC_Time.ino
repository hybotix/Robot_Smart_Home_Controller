#include <NTPClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include  <RTClib.h>
RTC_DS3231 rtc;

#include  "Secrets.h"

const char *ssid     = WIFI_SSID;
const char *password = WIFI_PASSWD;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

void setup() {
  uint32_t epoch_time;

  Serial.begin(115200);

  WiFi.begin(ssid, password);

  if (! rtc.begin()) {
    Serial.println("Could not find the RTC!");
    Serial.flush();

    while (1) {
      delay(10);
    }
  }

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  timeClient.begin();
  timeClient.setTimeOffset(-7 * 3600);
  timeClient.getEpochTime();

  rtc.adjust(DateTime(epoch_time));

  Serial.print("RTC time is now set to NTP time");
}

void loop() {
  DateTime now = rtc.now();


  delay(5000);
}
