// Task 4 test program: drives the OLED menu from the keyboard over serial,
// since the buttons don't exist yet. Replaced by the button test in Task 5.
//
// Keys: m = open, w = up, s = down, d = select, a = close, h = test message
#include <Arduino.h>
#include "commands.h"
#include "menu.h"

void setup() {
  Serial.begin(115200);
  delay(500); // give the serial monitor a moment to connect

  menu::begin();

  Serial.println();
  Serial.println("Menu test. Keys: m=open  w=up  s=down  d=select  a=close  h=message");
}

void loop() {
  if (Serial.available() > 0) {
    char key = Serial.read();

    switch (key) {
      case 'm':
        menu::open();
        break;
      case 'w':
        menu::up();
        break;
      case 's':
        menu::down();
        break;
      case 'd': {
        // Braces give this case its own scope, so it can declare a variable.
        menu::flashSelected();
        const Command& cmd = kCommands[menu::selected()];
        Serial.printf("SELECT %s %s\n", deviceLabel(cmd.device), cmd.name);
        break;
      }
      case 'a':
        menu::close();
        break;
      case 'h':
        menu::showMessage("Hello", 1000);
        break;
      default:
        break; // ignore anything else, e.g. the newline from Enter
    }
  }

  menu::update(millis()); // every loop: runs the menu's timers and redraws if needed
}
