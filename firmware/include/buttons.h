// The 8 buttons: reads the pins, filters out contact bounce, and reports
// clean events. Knows nothing about what the buttons do.
//
// Each button is wired between its GPIO and GND, using the internal pull-up:
// the pin reads HIGH when released and LOW when pressed.
#pragma once
#include <stdint.h>

// Order must match kConfig in buttons.cpp
enum class ButtonId : uint8_t { TvPower, FanPower, Screen, Ok, Up, Down, Left, Right, Count };

enum class EventType : uint8_t {
  Press,   // went down
  Repeat,  // still held (Ok, Up, Down, Left, Right only): at 400 ms, then every 110 ms
  Long,    // held 3 s (Screen only), sent once
  Release  // went up; heldMs says for how long
};

struct ButtonEvent {
  ButtonId button;
  EventType type;
  uint32_t heldMs; // how long the button has been down (0 for Press)
};

// Sets up the pins. A button that's already held at this moment is ignored
// until it has been released (no events at all), so the press that woke the
// remote from deep sleep isn't handled twice.
void buttonsBegin();

// Returns true and fills `out` if there's an event. Call it repeatedly until
// it returns false:   while (buttonsPoll(millis(), e)) { ... }
bool buttonsPoll(uint32_t nowMs, ButtonEvent& out);

const char* buttonName(ButtonId b); // "TV Power", "Up", ...
const char* eventName(EventType t); // "Press", "Repeat", ...
