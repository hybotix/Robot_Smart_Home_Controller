#include  "Robot_Controller.h"
#include  "Arduino_HAT_Carrier.h"
#include  "Secrets.h"

#if defined(ARDUINO_PORTENTA_C33)
#include <WiFiC3.h>
#elif defined(ARDUINO_UNOWIFIR4)
#include <WiFiS3.h>
#endif

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

void show_switches_raspi (uint8_t nr_of_switches=NUMBER_OF_SWITCHES) {
  uint8_t index;

  Serial.print("Switch states: ");


  if (digitalRead(SWITCH_WHITE_PIN)) {
    Serial.print("ON");
  } else {
    Serial.print("OFF");
  }

/*
  for (index=0; index < nr_of_switches; index++) {
    Serial.print(SWITCH_NAMES[index]);
    Serial.print(" = ");

    if (SWITCH_READINGS[index]) {
      Serial.print("ON");
    } else {
      Serial.print("OFF");
    }

    if (index < nr_of_switches - 1) {
      Serial.print(", ");
    }
  }
*/
  Serial.println();
}

/*
  Set the LEDs according to the states of the switches.
*/
bool set_leds_raspi (uint8_t nr_of_switches=NUMBER_OF_SWITCHES, uint8_t nr_of_leds=NUMBER_OF_LEDS) {
  uint8_t index;
  bool result = true;

  if (digitalRead(SWITCH_RASPI_TEST_PIN)) {
    digitalWrite(LED_RASPI_TEST_PIN, HIGH);
  } else {
    digitalWrite(LED_RASPI_TEST_PIN, LOW);
  }

  if (nr_of_switches == nr_of_leds) {
    for (index=0; index < nr_of_switches; index++) {
      //Serial.print("index = ");
      //Serial.print(index);
      //Serial.print(", switch state = ");
      //Serial.println(SWITCH_READINGS[index]);

      if (SWITCH_READINGS[index]) {
        digitalWrite(LED_PINS[index], LOW);
      } else {
        digitalWrite(LED_PINS[index], HIGH);
      }
    }
  } else {
    result = false;
  }

}


/*
  Initialize the digital LED outputs

  For the Portenta C33, LOW = Active (HIGH or ON state)
*/
void init_leds (uint8_t nr_of_leds=NUMBER_OF_LEDS, uint8_t blink_delay_ms=DEFAULT_BLINK_RATE_MS) {
  uint8_t index;

  //  Initialize the RGB LED - Set pins to be outputs
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  //  Turn the RGB LED OFF
  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);


  //  Set the status LED pins to OUTPUT
  for (index=0; index < nr_of_leds; index++) {
    //  Set the LED pins to OUTPUT
    pinMode(LED_PINS[index], OUTPUT);
    digitalWrite(LED_PINS[index], HIGH);

    //  Blink each of the LEDs
    blink_led_c33(LED_PINS[index]);
    delay(100);
  }

  Serial.println();
  Serial.print("There are ");
  Serial.print(nr_of_leds);
  Serial.println(" LEDs (Digital Output)");

  //  Initialize the Raspberry Pi GPIO pins
  pinMode(LED_WHITE_PIN, OUTPUT);             //  ON = Good WiFi connection
  digitalWrite(LED_WHITE_PIN, LOW);
  blink_led_raspi(LED_WHITE_PIN, 250, 1);

  pinMode(LED_RASPI_CONNECT_PIN, OUTPUT);             //  ON = Good WiFi connection
  digitalWrite(LED_RASPI_CONNECT_PIN, LOW);
  blink_led_raspi(LED_RASPI_CONNECT_PIN, 250, 1);

  pinMode(LED_RASPI_WIFI_PIN, OUTPUT);                //  When connecting to WiFi, blinks the number
  digitalWrite(LED_RASPI_WIFI_PIN, LOW);              //    of attempts that have been made if > 1
  blink_led_raspi(LED_RASPI_WIFI_PIN, 250, 1);

  pinMode(LED_RASPI_HALT_PIN, OUTPUT);                //  Blinks steady when a WiFi connection
  digitalWrite(LED_RASPI_HALT_PIN, LOW);              //    was not made
  blink_led_raspi(LED_RASPI_HALT_PIN, 250, 1);

  pinMode(LED_RASPI_TEST_PIN, OUTPUT);                //  Blinks steady when a WiFi connection
  digitalWrite(LED_RASPI_TEST_PIN, LOW);              //    was not made
  blink_led_raspi(LED_RASPI_TEST_PIN, 250, 1);
}

void read_switches_raspi (uint8_t nr_of_switches=NUMBER_OF_SWITCHES) {
  uint8_t index;
  String html;

  for (index=0; index < nr_of_switches; index++) {
    SWITCH_READINGS[index] = digitalRead(SWITCH_PINS[index]);
  }
}

/*
  Initialize the digital pins for the switches
*/
void init_switches_raspi(uint8_t nr_of_switches=NUMBER_OF_SWITCHES) {
  uint8_t index;

  pinMode(SWITCH_RASPI_TEST_PIN, INPUT_PULLUP);

/*
  for (index=0; index < nr_of_switches; index++) {
    //  Set the analog pin to INPUT
    pinMode(SWITCH_PINS[index], INPUT_PULLDOWN);
  }
*/
  Serial.print("There are ");
  Serial.print(nr_of_switches);
  Serial.println(" switches (Digital Input)");
}

void setup() {
  Serial.begin(115200);
  delay(7000);

  Serial.println();
  Serial.println("Starting the Raspberry Pi Tester");
  Serial.println();
  
  init_leds(12);
  init_switches_raspi(1);
}

void loop() {
  uint16_t  looper;

  //  Heartbeat
  blink_rgb(blue);

  Serial.println();
  Serial.print("Loop #");
  looper++;
  Serial.println(looper);

  read_switches_raspi(1);
  show_switches_raspi(1);
  set_leds_raspi(4);
}
