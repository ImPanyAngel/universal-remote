// Task 5 test program: prints every button event over serial.
// Replaced by the real remote in Task 6.
#include <Arduino.h>
#include "buttons.h"
#include "menu.h"

void setup() {
  Serial.begin(115200);
  delay(500); // give the serial monitor a moment to connect

  // Only so the OLED switches off: it keeps showing its last picture across
  // ESP32 resets, because it stays powered from the rail.
  menu::begin();
  menu::update(millis());

  buttonsBegin();

  Serial.println();
  Serial.println("Button test: press, hold and release buttons.");
}

void loop() {
  ButtonEvent e;
  while (buttonsPoll(millis(), e)) {
    Serial.printf("%-9s %-7s %lu\n", buttonName(e.button), eventName(e.type), (unsigned long)e.heldMs);
  }
}
