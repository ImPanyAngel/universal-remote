// The remote.
//
// Two modes, shown by the screen:
//   Screen off = TV mode:   arrows and OK send the TV's arrow/OK codes.
//   Screen on  = menu mode: Up/Down scroll, OK/Right send the highlighted
//                           command, Left closes the menu.
// TV Power and Fan Power always send. Tap Screen to open/close the menu;
// hold it 3 s to sleep.
#include <Arduino.h>
#include <driver/rtc_io.h>
#include <esp_sleep.h>
#include "buttons.h"
#include "commands.h"
#include "ir.h"
#include "menu.h"
#include "pins.h"

constexpr uint32_t SPLASH_MS = 1000;
constexpr uint32_t SLEEP_HOLD_MS = 3000; // same as the Long press in buttons.cpp

static bool sleepPending = false; // Screen held 3 s; sleep when it's released

// The buttons that can wake the remote from deep sleep. Only GPIO 0-7 can.
struct WakeButton {
  uint8_t pin;
  ButtonId button;
};
static const WakeButton kWakeButtons[] = {
  {PIN_BTN_TV_POWER,  ButtonId::TvPower},
  {PIN_BTN_FAN_POWER, ButtonId::FanPower},
  {PIN_BTN_SCREEN,    ButtonId::Screen},
  {PIN_BTN_OK,        ButtonId::Ok},
  {PIN_BTN_UP,        ButtonId::Up},
  {PIN_BTN_DOWN,      ButtonId::Down},
};

// Deep sleep: everything stops except the wake circuit. Waking up is a full
// restart, so this function never returns; setup() runs again on wake.
static void goToSleep() {
  Serial.println("Going to sleep");
  Serial.flush(); // wait until the message has actually been sent

  menu::sleep();                 // OLED panel off
  rgbLedWrite(PIN_LED, 0, 0, 0); // in case a send blink was still on

  uint64_t wakeMask = 0;
  for (const WakeButton& w : kWakeButtons) {
    wakeMask |= 1ULL << w.pin; // one bit per GPIO
    // Keep the pull-ups on while asleep, or the pins would float and could
    // wake the chip at random. These are the low-power ("RTC") versions of
    // the pull-ups, which keep working in deep sleep.
    rtc_gpio_pullup_en(static_cast<gpio_num_t>(w.pin));
    rtc_gpio_pulldown_dis(static_cast<gpio_num_t>(w.pin));
  }
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON); // keep those pull-ups powered
  esp_sleep_enable_ext1_wakeup_io(wakeMask, ESP_EXT1_WAKEUP_ANY_LOW); // any button pressed = wake

  esp_deep_sleep_start();
}

// If this boot is a wake from deep sleep caused by a button, returns true and
// which button it was.
static bool wokenByButton(ButtonId& button) {
  if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_EXT1) return false;

  uint64_t pins = esp_sleep_get_ext1_wakeup_status(); // bit n set = GPIO n woke us
  for (const WakeButton& w : kWakeButtons) {
    if (pins & (1ULL << w.pin)) {
      button = w.button;
      return true;
    }
  }
  return false;
}

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
        // Only now, after release: Screen is a wake button, so sleeping while
        // it's still held would wake the chip straight away.
        goToSleep();
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

  // After a deep-sleep wake, the wake pins are still in their low-power (RTC)
  // mode. Hand them back to normal GPIO so the buttons code can read them.
  for (const WakeButton& w : kWakeButtons) {
    rtc_gpio_deinit(static_cast<gpio_num_t>(w.pin));
  }

  menu::begin();
  irBegin();
  buttonsBegin(); // if the wake button is still held, it's ignored until released

  ButtonId wakeButton;
  if (wokenByButton(wakeButton)) {
    // Woken by a button: no splash, act on that press straight away
    Serial.printf("Woken by %s\n", buttonName(wakeButton));
    if (wakeButton == ButtonId::Screen) {
      menu::open(); // Screen normally acts on release, which suppression hides here
    } else {
      handleButton({wakeButton, EventType::Press, 0});
    }
  } else {
    menu::showMessage("Universal Remote", SPLASH_MS);
    Serial.println();
    Serial.println("Universal Remote ready");
  }
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
