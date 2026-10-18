// Sends commands from the command table over the IR LED.
// Knows nothing about buttons or the menu.
#pragma once
#include <stdint.h>
#include "commands.h"

void irBegin();
void sendCommand(CommandId id); // blocks while sending: ~56 ms for a TV code, ~24 ms for the fan
void irUpdate(uint32_t nowMs);  // call every loop: turns the send-feedback LED off again
