/*
  Five LED Blinker

  Arduino Portenta C33
*/

#include  "RaspberryPi_to_Portenta_C33.h"
#include  "Five_LED_Blinker.h"

/*
  Halt everything - used for unrecoverable errors
*/
void halt (void) {
  Serial.println();
  Serial.println("Halting...");

  //  Infinite loop
  while (true) {
    delay(1);
  }
}

/*
  Blink an LED on the selected pin.
*/
void blink_led (uint8_t pin, uint8_t blink_rate_ms=DEFAULT_BLINK_RATE_MS, uint8_t nr_cycles=DEFAULT_NR_CYCLES) {
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
  Return the state of the selected switch
*/
int8_t switch_state (uint8_t sw_nr) {
  int8_t result;

  if (sw_nr > 0 and sw_nr < NUMBER_OF_SWITCHES) {
    if (SWITCH_READINGS[sw_nr]) {
      result = 1;
    } else {
      result = 0;
    }
  } else {
    result = -1;
  }

  return result;
}

/*
  Set the LEDs according to the states of the switches.
*/
bool set_leds (uint8_t nr_of_switches=NUMBER_OF_SWITCHES, uint8_t nr_of_leds=NUMBER_OF_LEDS) {
  uint8_t index;
  bool result = true;

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

String switch_text (uint8_t nr_of_switches=NUMBER_OF_SWITCHES) {
  uint8_t index;
  String html = "", state;

  for (index=0; index < nr_of_switches; index++) {
    if (SWITCH_READINGS[index]) {
      state = "ON";
    } else {
      state ="OFF";
    }

    html = html + SWITCH_NAMES[index] + " = " + state;

    if (index < nr_of_switches - 1) {
      html = html + ", ";
    }
  }

  return html;
}

void show_switches (uint8_t nr_of_switches=NUMBER_OF_SWITCHES) {
  uint8_t index;

  Serial.print("Switch states: ");

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

  Serial.println();
}

void read_switches(uint8_t nr_of_switches=NUMBER_OF_SWITCHES) {
  uint8_t index;
  String html;

  for (index=0; index < nr_of_switches; index++) {
    SWITCH_READINGS[index] = digitalRead(SWITCH_PINS[index]);
  }
}

/*
  Read resistor voltages
*/
void read_resistors(uint8_t nr_of_resistors=NUMBER_OF_RESISTORS) {
  uint8_t index;

  Serial.print("Resistor readings: ");

  for (index=0; index < nr_of_resistors; index++) {
    RESISTOR_READINGS[index] = analogRead(RESISTOR_PINS[index]);
    RESISTOR_VOLTAGES[index] = (MAXIMUM_ANALOG_VOLTAGE / ANALOG_RESOLUTION) * RESISTOR_READINGS[index];

    Serial.print(RESISTOR_NAMES[index]);
    Serial.print(" = ");
    Serial.print(RESISTOR_VOLTAGES[index], 5);

    if (index < nr_of_resistors - 1) {
      Serial.print(", ");
    }

    delay(2);
  }

  Serial.println();

  Serial.print("Resistor voltages: ");

  for (index=0; index < nr_of_resistors; index++) {
    Serial.print(RESISTOR_NAMES[index]);
    Serial.print(" = ");
    Serial.print(RESISTOR_VOLTAGES[index], 5);
    Serial.print("V");

    if (index < nr_of_resistors - 1) {
      Serial.print(", ");
    }
  }

  Serial.println();
}

/*
  Initialize the digital LED outputs

  For the Portenta C33, LOW = Active (HIGH or ON state)
*/
void init_leds (uint8_t nr_of_leds=NUMBER_OF_LEDS, uint8_t blink_delay_ms=DEFAULT_BLINK_RATE_MS) {
  uint8_t index;

  //  Set the LED pins to OUTPUT
  for (index=0; index < nr_of_leds; index++) {
    //  Set the LED pins to OUTPUT
    pinMode(LED_PINS[index], OUTPUT);
    digitalWrite(LED_PINS[index], HIGH);

    //  Blink each of the LEDs
    blink_led(LED_PINS[index]);

    delay(500);
  }

  Serial.println();
  Serial.print("There are ");
  Serial.print(nr_of_leds);
  Serial.println(" LEDs (Digital Output)");
}

/*
  Initialize the digital pins for the switches
*/
void init_switches(uint8_t nr_of_switches=NUMBER_OF_SWITCHES) {
  uint8_t index;

  for (index=0; index < nr_of_switches; index++) {
    //  Set the analog pin to INPUT
    pinMode(SWITCH_PINS[index], INPUT_PULLDOWN);
  }

  Serial.print("There are ");
  Serial.print(nr_of_switches);
  Serial.println(" switches (Digital Input)");
}

/*
  Initialize the analog pins for the resistors
*/
void init_resistors(uint8_t nr_of_resistors=NUMBER_OF_RESISTORS) {
  uint8_t index;

  for (index=0; index < nr_of_resistors; index++) {
    //  Set the analog pin to INPUT
    pinMode(RESISTOR_PINS[index], INPUT);
  }

  Serial.print("There are ");
  Serial.print(nr_of_resistors);
  Serial.println(" resistors (Analog Input)");
}

//  Setup for the sketch
void setup(void) {
  //  Initialize serial and wait for the port to open:
  Serial.begin(115200);

  while(!Serial) {
    delay(10);
  }
  
  //  Initialize the PIEZO Buzzer
  pinMode(PIEZO_BUZZER_PIN, OUTPUT);
  analogWrite(PIEZO_BUZZER_PIN, 40);

  init_leds();

  Serial.println();
  init_switches();

  Serial.println();
  init_resistors();
}

// the loop function runs over and over again forever
void loop (void) {
  String text;
  uint8_t index = 0, looper = 0;

  while(true) {
    looper += 1;

    Serial.println();
    Serial.print("Loop #");
    Serial.println(looper);

    read_switches();
    show_switches();
    set_leds();

    text = switch_text();

    Serial.print("Switch Text = '");
    Serial.print(text);
    Serial.println("'");

    delay(DEFAULT_LOOP_DELAY_MS);
  }
}
