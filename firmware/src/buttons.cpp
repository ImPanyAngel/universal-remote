#include "buttons.h"

#include <Arduino.h>
#include "pins.h"

// --- Settings ---
constexpr uint32_t DEBOUNCE_MS = 25;
constexpr uint32_t REPEAT_DELAY_MS = 400;
constexpr uint32_t REPEAT_INTERVAL_MS = 110;
constexpr uint32_t LONG_PRESS_MS = 3000;

constexpr uint8_t BUTTON_COUNT = static_cast<uint8_t>(ButtonId::Count);

struct ButtonConfig {
  uint8_t pin;
  bool repeats;   // sends Repeat events while held
  bool longPress; // sends one Long event at 3 s
  const char* name;
};

// One entry per ButtonId, in the same order as the enum
static const ButtonConfig kConfig[BUTTON_COUNT] = {
  {PIN_BTN_TV_POWER,  false, false, "TV Power"},  // never repeat: power is a toggle
  {PIN_BTN_FAN_POWER, false, false, "Fan Power"},
  {PIN_BTN_SCREEN,    false, true,  "Screen"},
  {PIN_BTN_OK,        true,  false, "OK"},
  {PIN_BTN_UP,        true,  false, "Up"},
  {PIN_BTN_DOWN,      true,  false, "Down"},
  {PIN_BTN_LEFT,      true,  false, "Left"},
  {PIN_BTN_RIGHT,     true,  false, "Right"},
};

struct ButtonState {
  bool rawPressed = false;      // last reading straight from the pin (may be bouncing)
  uint32_t rawChangedMs = 0;    // when that reading last changed
  bool pressed = false;         // debounced state
  uint32_t pressedAtMs = 0;     // when the current press started
  uint32_t nextRepeatAtMs = 0;  // held time at which the next Repeat is due
  bool longSent = false;
  bool suppressed = false;      // held at begin: ignore until released
};

static ButtonState states[BUTTON_COUNT];

// One step of a button's state machine. Takes the raw pin reading instead of
// reading the pin itself, so the logic has no hardware in it.
// Returns true if this step produced an event (type + heldMs filled in).
static bool stepButton(ButtonState& s, const ButtonConfig& cfg, bool rawPressed, uint32_t nowMs,
                       EventType& type, uint32_t& heldMs) {
  // 1. Watch the raw reading. Every change restarts the debounce timer.
  if (rawPressed != s.rawPressed) {
    s.rawPressed = rawPressed;
    s.rawChangedMs = nowMs;
  }

  // 2. Accept a change only once the reading has been steady for DEBOUNCE_MS.
  if (s.rawPressed != s.pressed && nowMs - s.rawChangedMs >= DEBOUNCE_MS) {
    s.pressed = s.rawPressed;

    if (s.pressed) {
      s.pressedAtMs = s.rawChangedMs; // when the finger actually went down
      s.nextRepeatAtMs = REPEAT_DELAY_MS;
      s.longSent = false;
      if (s.suppressed) return false;
      type = EventType::Press;
      heldMs = 0;
      return true;
    }

    // Released
    if (s.suppressed) {
      s.suppressed = false; // from now on this button works normally
      return false;
    }
    type = EventType::Release;
    heldMs = nowMs - s.pressedAtMs;
    return true;
  }

  // 3. While held: Repeat and Long events.
  if (s.pressed && !s.suppressed) {
    uint32_t held = nowMs - s.pressedAtMs;

    if (cfg.repeats && held >= s.nextRepeatAtMs) {
      // Scheduled from now, not from the last due time: if the loop was busy
      // (an IR send, a redraw), we skip ahead instead of firing a burst to catch up.
      s.nextRepeatAtMs = held + REPEAT_INTERVAL_MS;
      type = EventType::Repeat;
      heldMs = held;
      return true;
    }

    if (cfg.longPress && !s.longSent && held >= LONG_PRESS_MS) {
      s.longSent = true;
      type = EventType::Long;
      heldMs = held;
      return true;
    }
  }

  return false;
}

// --- Event queue ---
// A small ring buffer: scanning all 8 buttons can produce several events at
// once (e.g. two buttons released together), and buttonsPoll hands them out
// one at a time.
constexpr uint8_t QUEUE_SIZE = 16;
static ButtonEvent queue[QUEUE_SIZE];
static uint8_t queueHead = 0;  // next event to hand out
static uint8_t queueCount = 0;

static void pushEvent(const ButtonEvent& e) {
  if (queueCount == QUEUE_SIZE) return; // full: drop (can't happen with 8 buttons polled every loop)
  queue[(queueHead + queueCount) % QUEUE_SIZE] = e;
  queueCount++;
}

// --- Public functions ---

void buttonsBegin() {
  uint32_t now = millis();
  for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
    pinMode(kConfig[i].pin, INPUT_PULLUP);
  }
  delay(1); // let the pull-ups charge the pins before the first reading

  for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
    ButtonState& s = states[i];
    s = ButtonState{};
    bool held = digitalRead(kConfig[i].pin) == LOW;
    if (held) {
      // Already down: treat it as pressed-and-reported so it produces no events
      s.rawPressed = s.pressed = true;
      s.rawChangedMs = s.pressedAtMs = now;
      s.suppressed = true;
    }
  }
  queueHead = queueCount = 0;
}

bool buttonsPoll(uint32_t nowMs, ButtonEvent& out) {
  if (queueCount == 0) {
    for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
      bool rawPressed = digitalRead(kConfig[i].pin) == LOW;
      EventType type;
      uint32_t heldMs;
      if (stepButton(states[i], kConfig[i], rawPressed, nowMs, type, heldMs)) {
        pushEvent({static_cast<ButtonId>(i), type, heldMs});
      }
    }
  }

  if (queueCount == 0) return false;
  out = queue[queueHead];
  queueHead = (queueHead + 1) % QUEUE_SIZE;
  queueCount--;
  return true;
}

const char* buttonName(ButtonId b) {
  uint8_t i = static_cast<uint8_t>(b);
  return i < BUTTON_COUNT ? kConfig[i].name : "?";
}

const char* eventName(EventType t) {
  switch (t) {
    case EventType::Press:   return "Press";
    case EventType::Repeat:  return "Repeat";
    case EventType::Long:    return "Long";
    case EventType::Release: return "Release";
  }
  return "?";
}
