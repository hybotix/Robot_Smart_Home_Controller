#include  "Arduino_GigaDisplay.h"

GigaDisplayRGB rgb;

void setup() {

  rgb.begin();
}

void loop() {
  rgb.on(255, 0, 0);
  delay(1000);
  rgb.off();
  delay(1000);
}
