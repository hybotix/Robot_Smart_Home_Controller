/*
 Multiple Blinks

 Demonstrates the use of the Scheduler library for the boards:
 
 - Arduino Nano 33 BLE, or
 - Arduino Portenta H7, or
 - Arduino Nano RP2040 Connect

 Hardware required :
 * None (LEDs are already conencted to RGB LED)

 ATTENTION: LEDs polarity is reversed (so loop3 will turn the LED off by writing 1)

 created 8 Oct 2012
 by Cristian Maglie
 Modified by
 Scott Fitzgerald 19 Oct 2012

 This example code is in the public domain

 http://www.arduino.cc/en/Tutorial/MultipleBlinks
*/
// Include Scheduler since we want to manage multiple tasks.
#include <Scheduler.h>

int led_R = LEDR;
int led_G = LEDG;
int led_B = LEDB;

#include  "Robot_Controller.h"
//#include  "Web_Server_Control.h"
#include  "Secrets.h"

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

  //  Infinite loop
  while (true) {
    //blink_led_raspi(LED_RASPI_HALT_PIN);
    delay(100);
  }
}

/*
  Convert the Celsius temperature to Fahrenheit

  Returns: (float) temperature in fahrenheit
*/
float to_fahrenheit (float celsius) {
  return celsius * 1.8 + 32;
}

void setup() {
  Serial.begin(115200);

  // Setup the 3 pins as OUTPUT
  pinMode(led_R, OUTPUT);
  pinMode(led_G, OUTPUT);
  pinMode(led_B, OUTPUT);

  // Add "loop2" and "loop3" to scheduling.
  // "loop" is always started by default.
  Scheduler.startLoop(loop2);
  Scheduler.startLoop(loop3);
}

// Task no.1: blink LED with 1 second delay.
void loop() {
  digitalWrite(led_R, HIGH);

  // IMPORTANT:
  // When multiple tasks are running 'delay' passes control to
  // other tasks while waiting and guarantees they get executed.
  delay(1000);

  digitalWrite(led_R, LOW);
  delay(1000);
}

// Task no.2: blink LED with 0.1 second delay.
void loop2() {
  digitalWrite(led_B, HIGH);
  delay(500);
  digitalWrite(led_B, LOW);
  delay(500);
}

// Task no.3: accept commands from Serial port
// '0' turns off LED
// '1' turns on LED
void loop3() {
  if (Serial.available()) {
    char c = Serial.read();

    if (c == '0') {
      digitalWrite(led_G, HIGH);
      Serial.println("Green LED turned OFF!");
    } else if (c == '1') {
      digitalWrite(led_G, LOW);
      Serial.println("Green LED turned ON!");
    } else if ((c == 'r')  or (c == 'R')) { 
      blink_rgb(Red, DEFAULT_BLINK_RATE_MS, DEFAULT_NR_CYCLES);
      Serial.println("Blink Red!");
    } else if ((c == 'g')  or (c == 'G')) {
      blink_rgb(Green, DEFAULT_BLINK_RATE_MS, DEFAULT_NR_CYCLES);
      Serial.println("Blink Green!");
    } else if ((c == 'b')  or (c == 'B')) { 
      blink_rgb(Blue, DEFAULT_BLINK_RATE_MS, DEFAULT_NR_CYCLES);
      Serial.println("Blink Blue!");
    } else {
      Serial.println("Invalid Input!");
    }
  }

  // IMPORTANT:
  // We must call 'yield' at a regular basis to pass
  // control to other tasks.
  yield();
}
