// The remote.
//
// Two modes, shown by the screen:
//   Screen off = TV mode:   arrows and OK send the TV's arrow/OK codes.
//   Screen on  = menu mode: Up/Down scroll, OK/Right send the highlighted
//                           command, Left closes the menu.
// TV Power and Fan Power always send. Tap Screen to open/close the menu;
// hold it 3 s to sleep.
#include <Arduino.h>
#include "buttons.h"
#include "commands.h"
#include "ir.h"
#include "menu.h"

constexpr uint32_t SPLASH_MS = 1000;
constexpr uint32_t SLEEP_HOLD_MS = 3000; // same as the Long press in buttons.cpp

static bool sleepPending = false; // Screen held 3 s; sleep when it's released

// The TV code each button sends in TV mode
static CommandId tvCommandFor(ButtonId button) {
  switch (button) {
    case ButtonId::Up:    return CMD_TV_UP;
    case ButtonId::Down:  return CMD_TV_DOWN;
    case ButtonId::Left:  return CMD_TV_LEFT;
    case ButtonId::Right: return CMD_TV_RIGHT;
    case ButtonId::Ok:    return CMD_TV_OK;
    default:              return CMD_TV_OK; // not reached: only called for arrows and OK
  }
}

static void sendSelected() {
  sendCommand(menu::selected());
  menu::flashSelected();
}

// Decides what each button event does. Behaviour depends on three things:
// which button, what kind of event, and whether the menu is open.
void handleButton(const ButtonEvent& e) {
  bool isPress = (e.type == EventType::Press);
  bool isPressOrRepeat = isPress || (e.type == EventType::Repeat);

  // Power buttons: the same in both modes. Press only, never repeat (toggles).
  if (e.button == ButtonId::TvPower || e.button == ButtonId::FanPower) {
    if (isPress) {
      sendCommand(e.button == ButtonId::TvPower ? CMD_TV_POWER : CMD_FAN_POWER);
      if (menu::isOpen()) menu::touch(); // counts as activity for auto-close
    }
    return;
  }

  // Screen: acts on release, so holding it for sleep doesn't toggle the menu first.
  if (e.button == ButtonId::Screen) {
    if (e.type == EventType::Long) {
      menu::showMessage("Release to sleep", 0);
      sleepPending = true;
    } else if (e.type == EventType::Release) {
      if (sleepPending) {
        sleepPending = false;
        Serial.println("SLEEP"); // Task 7 replaces this with real deep sleep
        menu::close();
      } else if (e.heldMs < SLEEP_HOLD_MS) {
        if (menu::isOpen()) {
          menu::close();
        } else {
          menu::open();
        }
      }
    }
    return;
  }

  // Arrows and OK
  if (!menu::isOpen()) {
    // TV mode: send on press, and keep sending while held
    if (isPressOrRepeat) sendCommand(tvCommandFor(e.button));
    return;
  }

  // Menu mode
  switch (e.button) {
    case ButtonId::Up:
      if (isPressOrRepeat) menu::up();
      break;
    case ButtonId::Down:
      if (isPressOrRepeat) menu::down();
      break;
    case ButtonId::Ok:
      if (isPressOrRepeat) sendSelected(); // holding OK on "Vol +" keeps raising the volume
      break;
    case ButtonId::Right:
      if (isPress) sendSelected();         // Right doesn't repeat in the menu
      break;
    case ButtonId::Left:
      if (isPress) menu::close();
      break;
    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);

  menu::begin();
  irBegin();
  buttonsBegin();

  menu::showMessage("Universal Remote", SPLASH_MS);
  Serial.println();
  Serial.println("Universal Remote ready");
}

void loop() {
  ButtonEvent e;
  while (buttonsPoll(millis(), e)) {
    handleButton(e);
  }

  uint32_t now = millis();
  menu::update(now);
  irUpdate(now);
}
