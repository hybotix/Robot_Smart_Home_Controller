/*
  Web client

 This sketch connects to a website (http://example.com) using the WiFi module.

 This example is written for a network using WPA encryption. For
 WEP or WPA, change the Wifi.begin() call accordingly.

 Circuit:
 * Arduino Portenta H7

 created 13 July 2010
 by dlf (Metodo2 srl)
 modified 31 May 2012
 by Tom Igoe
 */

/*************************
  Internal Arduino Libraries
*************************/

#include <WiFi.h>
#include  <Wire.h>

/*************************
  Arduino Libraries
*************************/

#include <Arduino_LSM6DSOX.h>

/*************************
  Third-Party Libraries
*************************/
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

//Include the NTP library
#include <NTPClient.h>
WiFiUDP Udp; // A UDP instance to let us send and receive packets over UDP
NTPClient time_client(Udp);

// Include the RTC library
#include <DS3231.h>
RTClib rtc(&Wire1);

#include <Adafruit_LIS3MDL.h>
Adafruit_LIS3MDL lis3;

#include "Adafruit_SHT4x.h"
Adafruit_SHT4x sht4 = Adafruit_SHT4x();

#include "Adafruit_VEML7700.h"
Adafruit_VEML7700 veml = Adafruit_VEML7700();

#include "Robot_Controller.h"
#include "Web_Server_Control.h"
#include "Arduino_HAT_Carrier_H7.h"

#include "Secrets.h" 
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
int keyIndex = 0;                 // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
// IPAddress server(93,184,216,34);  // IP address for example.com (no DNS)
char server_url[] = "example.com";       // host name for example.com (using DNS)

WiFiClient client;

/**********************
  Utility routines
**********************/

/*
  Blink the onboard RGB LED with the selected color

  Parameters:
    color:          a ColorRGB structure that defines the desired color
    blink_rate_ms:  The blink rate in ms
    nr_cycles:      The number of times to blink the LED

  Returns:          void
*/
void blink_rgb (ColorRGB color, uint8_t blink_rate_ms=DEFAULT_BLINK_RATE_MS, uint8_t nr_cycles=DEFAULT_NR_CYCLES) {
  uint8_t count;

  for (count=0; count<nr_cycles; count++) {
    digitalWrite(LEDR, color.redB);
    digitalWrite(LEDG, color.greenB);
    digitalWrite(LEDB, color.blueB);

    delay(blink_rate_ms);

    digitalWrite(LEDR, HIGH);
    digitalWrite(LEDG, HIGH);
    digitalWrite(LEDB, HIGH);

    delay(blink_rate_ms);
  }
}

/*
  Blink a standard LED on the selected pin.
  Pins are active LOW

  Parameters:
    pin:            The pin the LED is connected to
    blink_rate_ms:  The blink rate in ms
    nr_cycles:      The number of times to blink the LED

  Returns:          void
*/
void blink_led_c33 (uint8_t pin, uint8_t blink_rate_ms=DEFAULT_BLINK_RATE_MS, uint8_t nr_cycles=DEFAULT_NR_CYCLES) {
  uint8_t index;

  for (index=0; index < nr_cycles; index++) {
    //  Turn the LED ON
    digitalWrite(pin, LOW);
    delay(blink_rate_ms);
      
    //  Turn the LED OFF
    digitalWrite(pin, HIGH);
    delay(blink_rate_ms);
  }
}

/*
  Blink a standard LED on the selected Raspberry Pi pin.
  Pins are active HIGH

  Parameters:
    pin:            The pin the LED is connected to
    blink_rate_ms:  The blink rate in ms
    nr_cycles:      The number of times to blink the LED

  Returns:          void
*/
void blink_led_raspi (uint8_t pin, uint8_t blink_rate_ms=DEFAULT_BLINK_RATE_MS, uint8_t nr_cycles=DEFAULT_NR_CYCLES) {
  uint8_t index;

  for (index=0; index < nr_cycles; index++) {
    //  Turn the LED ON
    digitalWrite(pin, HIGH);
    delay(blink_rate_ms);
      
    //  Turn the LED OFF
    digitalWrite(pin, LOW);
    delay(blink_rate_ms);
  }
}

/*
  Halt everything - used for unrecoverable errors

  Parameters:
    message:    The message to disaplay on the serial console
    wifi_halt:  If this is a WiFi connection halt, true
    wifi_ssid:  If wifi_halt is true, this should be the SSID of the WiFi network

  Returns:      void
*/
void halt ( char* message, bool wifi_halt=false, char *wifi_ssid=ssid) {
  Serial.println();
  Serial.print(message);

  if (wifi_halt) {
    Serial.print(" ");
    Serial.print(wifi_ssid);
  }

  //  Turn off all other status LEDs
  digitalWrite(LED_RASPI_WIFI_PIN, LOW);
  digitalWrite(LED_RASPI_CONNECT_PIN, LOW);

  //  Infinite loop
  while (true) {
    blink_led_raspi(LED_RASPI_HALT_PIN);
    delay(100);
  }
}

/*
  Do the actual left padding - there is no checking done

  Parameters:
    str:          The string to be padded
    numeric_only: True if the string passed must be numeric
    pad_length:   The length to pad the string to
    pad_char:     The String character for padding

  Returns:      void
*/
String pad_string (String str, uint8_t pad_length=DEFAULT_PAD_LENGTH, String pad_char=DEFAULT_PAD_STRING) {
  uint8_t str_index = 0;
  String result_str = "";

  for (str_index=1; str_index < pad_length; str_index++) {
    result_str = pad_char + result_str;
  }

  return result_str; 
}

/*
  Left pad a string - works with -unsigned- numeric strings only at this time

  Parameters:
    str:          The string to be padded
    numeric_only: True if the string passed must be numeric
    pad_length:   The length to pad the string to
    pad_char:     The String character for padding

  Returns:      void
*/
String left_pad (String str, bool numeric_only=true, uint8_t pad_length=DEFAULT_PAD_LENGTH, String pad_char=DEFAULT_PAD_STRING) {
  uint8_t str_len, str_index = 0, position = 0;
  String result_str = "", digits = "0123456789", temp_str = "";
  bool is_number = true;

  str_len = str.length();

  if (str_len == pad_length) {
    result_str = str;
  } else {
    if (numeric_only) {
      //  Scan the string to be sure it is all numeric characters
      for (str_index=0; str_index < str_len; str_index++) {
        //  Get the character to check
        temp_str = str.substring(str_index, str_index + 1);

        //  See if the character is a digit
        position = digits.indexOf(temp_str) + 1;

        //  Test the character. If position < 0, character is not a digit
        is_number = (position > 0);

        if (!is_number) {
          //  Found a non-numeric character so break the loop
          break;
        }
      }

      if (is_number) {
        result_str = str;
        //  Add the appropriate number of pad_char to the left of the string
        result_str = pad_string(str, pad_length, pad_char);
      } else {
        //  Invalid - Non-digit character is present
        result_str = "**";
      }
    } else {
        //  Add the appropriate number of pad_char to the left of the string
        result_str = pad_string(str, pad_length, pad_char);
    }
  }

  return result_str;
}

/*
  Create a date and time stamp of the current time

  Parameters:
*/
String timestamp (RTClib *rtc, bool show_full=SHOW_FULL_DATE, bool hours_24=SHOW_12_HOURS, bool long_date=SHOW_LONG_DATE, bool show_seconds=SHOW_SECONDS) {
  Datetime current_time;
  String date_time, date_str = "D*", time_str = "T*";
  String year_str = "Y*", month_str = "M*", day_str = "D*";
  String hours_str, min_sec_str, week_day_str = "*";
  String am_pm = " AM", day_suffix = "**";
  uint8_t position, str_len, week_day_nr, day, suffix, month, hours;

  //  Retrieve the date and time from the RTC
  //  String is in the format yyy-mm-ddThh:mm:ss
  current_time = rtc->now;
  //RTC.getTime(current_time); 
  date_time = String(current_time);
  str_len = date_time.length();
  position = date_time.indexOf("T");

  if (show_full) {
    //Serial.println("Showing full date");

    time_str = date_time.substring(position + 1, str_len);
    min_sec_str = date_time.substring(position + 3, str_len);
    position = date_time.indexOf("T");
    date_str = date_time.substring(0, position);

    str_len = date_str.length();
    year_str = date_str.substring(0, 4);
    month_str = date_str.substring(5, 7);
    month = month_str.toInt();
    day_str = date_str.substring(8, str_len);
    day = day_str.toInt();

    week_day_nr = time_cl->getDay();
/*
    Serial.print("(1) time_str = '");
    Serial.print(time_str);
    Serial.println("'");
*/
    //  Adjust day for underflow to Sunday
    week_day_nr = now.dayOfTheWeek();    }

    week_day_str = week_days[week_day_nr];

    day_str = String(current_time.day);
    str_len = day_str.length();

    if (day < 10) {
      suffix = day;
    } else if (str_len == 2) {
      suffix = day_str.substring(str_len - 1, str_len - 1).toInt();
    }

    if (long_date) {
      //Serial.println("Showing long date");
/*
      Serial.print("(2) day_str = '");
      Serial.print(day_str);
      Serial.print("', week_day_str = '");
      Serial.print(week_day_str);
      Serial.println("'");
*/
      switch(suffix) {
        case 0:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
          day_suffix = "th";
          break;
        case 1:
          day_suffix = "st";
          break;
        case 2:
          day_suffix = "nd";
          break;
        case 3:
          day_suffix = "rd";
          break;
        default:
          day_suffix = "**";
          break;
      }
      day_str = day_str + day_suffix;
    } else {
      day_str = left_pad(day_str);
    }

    str_len = time_str.length();
    hours_str = time_str.substring(0, 2);
    hours = hours_str.toInt();
    min_sec_str = time_str.substring(3, str_len);
/*
    Serial.print("(3) day_str = '");
    Serial.print(day_str);
    Serial.print("', min_sec_str = '");
    Serial.print(min_sec_str);
    Serial.println("'");
*/    
    if (hours_24) {
      //Serial.println("Showing 24 hour format time");

      time_str = left_pad(String(hours)) + ":" + min_sec_str;
    } else {
      if (hours == 0) {
        hours = 12;
        am_pm = " AM";
      } else {
        if (hours > 12) {
          hours = hours - 12;
          am_pm = " PM";
        } else {
          am_pm = " AM";
        }
      }

      time_str = String(hours) + ":" + min_sec_str + am_pm;
    }
/*
    Serial.print("(4) time_str = '");
    Serial.print(time_str);
    Serial.print("', hours = ");
    Serial.println(hours);
*/
    if (!show_seconds) {;
      str_len = time_str.length();
      time_str = time_str.substring(0, str_len - 3);
/*
      Serial.println("Showing seconds");
      Serial.print("(5) time_str = '");
      Serial.print(time_str);
      Serial.println("'");
*/
    }
/*
    Serial.print("(6) min_sec_str = '");
    Serial.print(min_sec_str);
    Serial.println("'");
*/
    if (long_date) {
      //Serial.println("Showing long date 2");

      date_str = week_day_str + ", " + long_months[month - 1] + " " + day_str + ", " + year_str;
    } else {
      date_str = left_pad(month_str) + "/" + left_pad(day_str) + "/" + year_str;
    }
/*
    Serial.print("(7) date_str = '");
    Serial.print(date_str);
    Serial.print("', min_sec_str = ");
    Serial.print(min_sec_str);
    Serial.println("'");
*/
    date_time = date_str + " at " + time_str;
  } else {    
    date_time = date_str + " at " + time_str;
  }
/*
  Serial.print("(8) date_time = '");
  Serial.print(date_time);
  Serial.print("', time_str = '");
  Serial.print(time_str);
  Serial.println("'");
*/
  return date_time;
}

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 3 seconds for connection:
    delay(3000);
  }
  Serial.println("Connected to wifi");
  printWifiStatus();

  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  if (client.connect(server_url, 80)) {
    Serial.println("connected to server");
    // Make a HTTP request:
    client.println("GET /index.html HTTP/1.1");
    client.print("Host: ");
    client.println(server);
    client.println("Connection: close");
    client.println();
  }
}

void loop() {
  // if there are incoming bytes available
  // from the server, read them and print them:
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting from server.");
    client.stop();

    // do nothing forevermore:
    while (true);
  }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
