// Capture program: point an original remote at the IR receiver, press a
// button, and copy the printed line into src/commands.cpp.
//
// Build and run with:  pio run -e capture -t upload && pio device monitor -e capture
#include <Arduino.h>
#include <ctype.h>
#include "pins.h"

// Room for long raw signals (fans and air conditioners can be long).
// Must be defined before IRremote.hpp is included.
#define RAW_BUFFER_LENGTH 750
#include <IRremote.hpp>

static uint32_t captureCount = 0;

// Protocols IRremote can decode but can't send back from address + command
// alone. For these we save the raw timings instead.
static bool needsRaw(decode_type_t protocol) {
  return protocol == UNKNOWN || protocol == PULSE_DISTANCE || protocol == PULSE_WIDTH;
}

// Prints the protocol as its C++ enum name, e.g. "Samsung" -> "SAMSUNG",
// so the line can be pasted straight into the command table.
static void printProtocolEnumName(decode_type_t protocol) {
  const char* name = (const char*)getProtocolString(protocol);
  for (const char* c = name; *c != '\0'; c++) {
    Serial.print((char)toupper(*c));
  }
}

void setup() {
  Serial.begin(115200);
  delay(500); // give the serial monitor a moment to connect

  IrReceiver.begin(PIN_IR_RX, DISABLE_LED_FEEDBACK);

  Serial.println();
  Serial.println("=== IR capture ready ===");
  Serial.println("Press each remote button 3 times, ~30 cm from the receiver.");
  Serial.println("Only use a code that is identical all 3 times.");
}

void loop() {
  if (!IrReceiver.decode()) {
    return; // nothing received yet
  }

  IRData& data = IrReceiver.decodedIRData;

  if (data.flags & IRDATA_FLAGS_IS_REPEAT) {
    // Sent while a button is held down. Not a new code, so skip it.
    Serial.println("(repeat)");
    IrReceiver.resume();
    return;
  }

  // A real remote code has dozens of pulses (NEC is ~68 timing entries).
  // A handful of entries is a glitch from light or electrical noise.
  if (data.protocol == UNKNOWN && data.rawlen < 8) {
    IrReceiver.resume();
    return;
  }

  captureCount++;
  Serial.println();
  Serial.printf("--- #%lu ---\n", (unsigned long)captureCount);

  if (data.flags & IRDATA_FLAGS_WAS_OVERFLOW) {
    Serial.println("WARNING: signal too long for the buffer, this capture is incomplete.");
  }

  IrReceiver.printIRResultShort(&Serial);

  if (needsRaw(data.protocol)) {
    // Prints e.g. "uint16_t rawIRTimings[23] = {1330, 420, ...};"
    IrReceiver.compensateAndPrintIRResultAsCArray(&Serial, true);
    Serial.println("Paste line (rename rawIRTimings to e.g. kFanSpeedRaw, make it static const):");
    Serial.println("{\"?\", DEV_FAN, true, UNKNOWN, 0, 0, kRaw, sizeof(kRaw) / sizeof(kRaw[0])},");
  } else {
    Serial.println("Paste line:");
    Serial.print("{\"?\", DEV_TV, true, ");
    printProtocolEnumName(data.protocol);
    Serial.printf(", 0x%X, 0x%X, nullptr, 0},\n", data.address, data.command);
  }

  IrReceiver.resume(); // ready for the next code
}
