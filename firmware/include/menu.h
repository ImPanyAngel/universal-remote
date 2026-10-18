// The OLED menu: shows the commands that have inMenu = true, lets you scroll
// through them, and shows short messages (splash, "Release to sleep").
//
// Call menu::update() every loop. Nothing is drawn until update() runs, and
// it only redraws when something changed.
#pragma once
#include <stdint.h>
#include "commands.h"

namespace menu {

void begin();                // start the display; the screen starts off
bool isOpen();               // true while the menu list is showing

void open();                 // show the list (at the last highlighted item)
void close();                // screen off; also clears any message
void up();                   // move the highlight, wrapping at the ends
void down();
CommandId selected();        // the highlighted command
void flashSelected();        // briefly flash the highlighted row (after a send)
void touch();                // counts as activity: restarts the auto-close timer

// Shows centred text. After durationMs it goes back to what was showing
// before (the list, or screen off). durationMs = 0 keeps it until the next
// showMessage() or close().
void showMessage(const char* text, uint32_t durationMs);

void sleep();                // display power-save, before deep sleep
void update(uint32_t nowMs); // timers (flash, message, auto-close) + redraw

}  // namespace menu
