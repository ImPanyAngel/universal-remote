// Task 3 test program: prints the command table at boot to prove it compiles
// and is in the right order. Replaced by the menu test in Task 4.
#include <Arduino.h>
#include "commands.h"

void setup() {
  Serial.begin(115200);
  delay(500); // give the serial monitor a moment to connect

  Serial.println();
  Serial.printf("Command table: %d commands\n", CMD_COUNT);
  int menuCount = 0;
  for (int i = 0; i < CMD_COUNT; i++) {
    const Command& c = kCommands[i];
    if (c.inMenu) menuCount++;
    Serial.printf("%2d  %-3s %-10s %-5s %2u timings\n", i, deviceLabel(c.device), c.name,
                  c.inMenu ? "menu" : "", c.rawLen);
  }
  Serial.printf("%d in the menu\n", menuCount);
}

void loop() {}
