// Every GPIO used by the remote, in one place.
// If a wire moves on the breadboard, change it here and nowhere else.
#pragma once
#include <stdint.h>

// Buttons (right side, J1). GPIO 0-7 can wake the chip from deep sleep.
constexpr uint8_t PIN_BTN_TV_POWER  = 0;
constexpr uint8_t PIN_BTN_FAN_POWER = 1;
constexpr uint8_t PIN_BTN_SCREEN    = 2;
constexpr uint8_t PIN_BTN_OK        = 3;
constexpr uint8_t PIN_BTN_UP        = 6;
constexpr uint8_t PIN_BTN_DOWN      = 7;
constexpr uint8_t PIN_BTN_LEFT      = 10;
constexpr uint8_t PIN_BTN_RIGHT     = 11;

// Modules (left side, J3)
constexpr uint8_t PIN_OLED_SDA = 23;
constexpr uint8_t PIN_OLED_SCL = 22;
constexpr uint8_t PIN_IR_RX    = 21;
constexpr uint8_t PIN_IR_TX    = 20;

// Onboard addressable RGB LED
constexpr uint8_t PIN_LED = 8; // not PIN_RGB_LED: the Arduino core already #defines that name

// Free for later (e.g. volume or Back buttons)
constexpr uint8_t PIN_SPARE_1 = 19;
constexpr uint8_t PIN_SPARE_2 = 18;
