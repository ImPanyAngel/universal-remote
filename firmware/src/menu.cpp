#include "menu.h"

#include <Arduino.h>
#include <U8g2lib.h>
#include "pins.h"

namespace menu {

// --- Settings ---
constexpr uint8_t VISIBLE_ROWS = 4;
constexpr uint32_t AUTO_CLOSE_MS = 15000;
constexpr uint32_t FLASH_MS = 150;
constexpr uint32_t I2C_CLOCK_HZ = 400000; // 400 kHz: a full redraw takes ~25 ms instead of ~100 ms

// Screen layout (128 x 64): a 12 px header, then 4 rows of 13 px = 64 px.
constexpr uint8_t HEADER_HEIGHT = 12;
constexpr uint8_t ROW_HEIGHT = 13;

// SSD1306, full frame buffer in RAM ("F"), hardware I2C on our pins.
// U8g2 calls Wire.begin(SDA, SCL) for us with these pins.
static U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE, PIN_OLED_SCL, PIN_OLED_SDA);

// --- State (static = private to this file) ---
static CommandId items[CMD_COUNT]; // the inMenu commands, in table order
static uint8_t itemCount = 0;
static uint8_t selectedIndex = 0;  // kept across close()/open(), so the menu remembers where you were
static uint8_t topIndex = 0;       // first item shown in the 4-row window

static bool listOpen = false;
static bool dirty = true;          // something changed since the last redraw
static uint32_t lastActivityMs = 0;

static bool flashing = false;
static uint32_t flashStartMs = 0;

static const char* messageText = nullptr; // nullptr = no message
static uint32_t messageStartMs = 0;
static uint32_t messageDurationMs = 0;    // 0 = until replaced or closed

// Timers store a start time and compare elapsed time ("now - start >= duration").
// That stays correct when millis() wraps back to 0 after ~49 days, unlike
// storing an end time and checking "now >= end".

// Keeps the highlighted item inside the visible window. Pure maths, no
// drawing, so it's easy to check by hand:
//   count 8, rows 4: selected 0..3 -> top 0; selected 7 -> top 4.
static uint8_t windowTop(uint8_t selected, uint8_t top, uint8_t count) {
  if (count <= VISIBLE_ROWS) return 0;
  if (selected < top) return selected;                                  // scrolled above the window
  if (selected >= top + VISIBLE_ROWS) return selected - (VISIBLE_ROWS - 1); // scrolled below it
  return top;                                                           // already visible
}

static void markActivity() {
  lastActivityMs = millis();
  dirty = true;
}

// --- Drawing ---

static void drawMessage() {
  display.setFont(u8g2_font_7x13_tr);
  int16_t x = (display.getDisplayWidth() - display.getStrWidth(messageText)) / 2;
  display.drawStr(x < 0 ? 0 : x, 37, messageText); // baseline near the vertical middle
}

static void drawList() {
  // Header: "MENU" on the left, "n/8" on the right, with a line under it
  display.setFont(u8g2_font_6x10_tr);
  display.drawStr(0, 9, "MENU");
  char position[8];
  snprintf(position, sizeof(position), "%u/%u", selectedIndex + 1, itemCount);
  display.drawStr(display.getDisplayWidth() - display.getStrWidth(position), 9, position);
  display.drawHLine(0, HEADER_HEIGHT - 1, display.getDisplayWidth());

  // Rows
  display.setFont(u8g2_font_7x13_tr);
  display.setFontMode(1); // transparent text, so it can be drawn on top of the highlight box
  for (uint8_t row = 0; row < VISIBLE_ROWS && topIndex + row < itemCount; row++) {
    uint8_t index = topIndex + row;
    const Command& cmd = kCommands[items[index]];
    uint8_t rowTop = HEADER_HEIGHT + row * ROW_HEIGHT;

    // The highlighted row is inverted (white box, black text). While
    // flashing, it's drawn normally for a moment, which reads as a blink.
    bool inverted = (index == selectedIndex) && !flashing;
    if (inverted) {
      display.setDrawColor(1);
      display.drawBox(0, rowTop, display.getDisplayWidth(), ROW_HEIGHT);
      display.setDrawColor(0);
    }

    char label[24];
    snprintf(label, sizeof(label), "%-3s %s", deviceLabel(cmd.device), cmd.name); // "TV  Vol +", "Fan Speed"
    display.drawStr(2, rowTop + 10, label);
    display.setDrawColor(1);
  }
}

static void render() {
  if (messageText == nullptr && !listOpen) {
    display.setPowerSave(1); // nothing to show: panel off (saves power and the OLED)
    return;
  }
  display.setPowerSave(0);
  display.clearBuffer();
  if (messageText != nullptr) {
    drawMessage();
  } else {
    drawList();
  }
  display.sendBuffer(); // the only slow part: sends 1 KB over I2C
}

// --- Public functions ---

void begin() {
  display.setBusClock(I2C_CLOCK_HZ);
  display.begin();

  itemCount = 0;
  for (uint8_t i = 0; i < CMD_COUNT; i++) {
    if (kCommands[i].inMenu) items[itemCount++] = static_cast<CommandId>(i);
  }

  dirty = true; // first update() turns the panel off
}

bool isOpen() { return listOpen; }

void open() {
  listOpen = true;
  markActivity();
}

void close() {
  listOpen = false;
  messageText = nullptr;
  flashing = false;
  dirty = true;
}

void up() {
  if (!listOpen || itemCount == 0) return;
  selectedIndex = (selectedIndex == 0) ? itemCount - 1 : selectedIndex - 1;
  topIndex = windowTop(selectedIndex, topIndex, itemCount);
  markActivity();
}

void down() {
  if (!listOpen || itemCount == 0) return;
  selectedIndex = (selectedIndex + 1 == itemCount) ? 0 : selectedIndex + 1;
  topIndex = windowTop(selectedIndex, topIndex, itemCount);
  markActivity();
}

CommandId selected() { return items[selectedIndex]; }

void flashSelected() {
  flashing = true;
  flashStartMs = millis();
  markActivity();
}

void touch() { lastActivityMs = millis(); }

void showMessage(const char* text, uint32_t durationMs) {
  messageText = text;
  messageStartMs = millis();
  messageDurationMs = durationMs;
  dirty = true;
}

void sleep() { display.setPowerSave(1); }

void update(uint32_t nowMs) {
  if (flashing && nowMs - flashStartMs >= FLASH_MS) {
    flashing = false;
    dirty = true;
  }

  if (messageText != nullptr && messageDurationMs > 0 && nowMs - messageStartMs >= messageDurationMs) {
    messageText = nullptr;
    lastActivityMs = nowMs; // don't let the menu auto-close the instant a message ends
    dirty = true;
  }

  if (listOpen && messageText == nullptr && nowMs - lastActivityMs >= AUTO_CLOSE_MS) {
    close();
  }

  if (dirty) {
    render();
    dirty = false;
  }
}

}  // namespace menu
