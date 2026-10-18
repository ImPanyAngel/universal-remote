#include "ir.h"

#include <Arduino.h>
#include "pins.h"

// The remote only sends; this leaves IRremote's receiving code out of the build.
#define DISABLE_CODE_FOR_RECEIVER
// IRremote.hpp contains the library's code, not just declarations, so it may
// be included in only one .cpp file per program. In the remote, that's here.
#include <IRremote.hpp>

constexpr uint8_t CARRIER_KHZ = 38;
constexpr uint32_t LED_BLINK_MS = 60;
constexpr uint8_t LED_BRIGHTNESS = 40;

static bool ledOn = false;
static uint32_t ledOnSinceMs = 0;

void irBegin() {
  IrSender.begin(PIN_IR_TX);
}

void sendCommand(CommandId id) {
  const Command& cmd = kCommands[id];
  Serial.printf("SEND %s %s\n", deviceLabel(cmd.device), cmd.name);

  // Feedback: blue blink, turned off by irUpdate() without blocking
  rgbLedWrite(PIN_LED, 0, 0, LED_BRIGHTNESS);
  ledOn = true;
  ledOnSinceMs = millis();

  if (cmd.raw != nullptr) {
    IrSender.sendRaw(cmd.raw, cmd.rawLen, CARRIER_KHZ);
  } else if (cmd.protocol != UNKNOWN) {
    IrSender.write(cmd.protocol, cmd.address, cmd.command, NO_REPEATS);
  } else {
    Serial.println("NOT CAPTURED"); // a table entry with no code yet
  }
}

void irUpdate(uint32_t nowMs) {
  if (ledOn && nowMs - ledOnSinceMs >= LED_BLINK_MS) {
    rgbLedWrite(PIN_LED, 0, 0, 0);
    ledOn = false;
  }
}
