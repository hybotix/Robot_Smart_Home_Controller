/* Timestamp functions using a DS3231 RTC connected via I2C and Wire lib
**
** Useful for file name
**		` SD.open(time.timestamp()+".log", FILE_WRITE) `
**
**
** Created: 2015-06-01 by AxelTB
** Last Edit:
*/
#include  "Robot_Controller.h"

#include "RTClib.h"
RTC_DS3231 rtc;

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
String timestamp (RTC_DS3231 *clock, bool show_full=SHOW_FULL_DATE, bool hours_24=SHOW_12_HOURS, bool long_date=SHOW_LONG_DATE, bool show_seconds=SHOW_SECONDS) {
  DateTime current_time = clock->now();
  String date_time, date_str = "D*", time_str = "T*";
  String year_str = "Y*", month_str = "M*", day_str = "D*";
  String hours_str, min_sec_str, week_day_str = "*";
  String am_pm = " AM", day_suffix = "**";
  uint8_t position, str_len, week_day_nr, day, suffix, month, hours;

  //  Retrieve the date and time from the RTC
  //  String is in the format yyy-mm-ddThh:mm:ss
  //current_time = clock->now;
  //RTC.getTime(current_time); 
  date_time = String(current_time.timestamp(DateTime::TIMESTAMP_FULL));
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

    week_day_nr = current_time.dayOfTheWeek();
/*
    Serial.print("(1) time_str = '");
    Serial.print(time_str);
    Serial.println("'");
*/
    //  Adjust day for underflow to Sunday
    week_day_str = week_days[week_day_nr];

    day_str = current_time.timestamp();
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

void setup () {
  Serial.begin(57600);

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  // When time needs to be re-set on a previously configured device, the
  // following line sets the RTC to the date & time this sketch was compiled
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
}

void loop() {
 DateTime time = rtc.now();

 //Full Timestamp
 Serial.println(String("DateTime::TIMESTAMP_FULL:\t")+time.timestamp(DateTime::TIMESTAMP_FULL));

 //Date Only
 Serial.println(String("DateTime::TIMESTAMP_DATE:\t")+time.timestamp(DateTime::TIMESTAMP_DATE));

 //Full Timestamp
 Serial.println(String("DateTime::TIMESTAMP_TIME:\t")+time.timestamp(DateTime::TIMESTAMP_TIME));

 Serial.println();

 //Delay 5s
 delay(5000);
}
