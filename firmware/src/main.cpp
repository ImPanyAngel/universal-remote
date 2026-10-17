// Task 1 test program: proves the toolchain, the upload and the serial
// monitor all work. Replaced by the real remote in later tasks.
#include <Arduino.h>

const uint8_t LED_PIN = 8;   // onboard addressable RGB LED (WS2812-style)
const uint8_t BRIGHTNESS = 40; // 0-255; the LED is very bright at full power

void setup() {
  Serial.begin(115200);
  delay(500); // give the serial monitor a moment to connect
  Serial.println("hello from universal-remote");
}

void loop() {
  rgbLedWrite(LED_PIN, BRIGHTNESS, 0, 0); // red
  delay(500);
  rgbLedWrite(LED_PIN, 0, BRIGHTNESS, 0); // green
  delay(500);
  rgbLedWrite(LED_PIN, 0, 0, BRIGHTNESS); // blue
  delay(500);
  rgbLedWrite(LED_PIN, 0, 0, 0);          // off
  delay(500);
}
