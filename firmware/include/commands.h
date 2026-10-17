// The command table: every IR code the remote can send.
// The codes themselves live in src/commands.cpp.
#pragma once
#include <stdint.h>
#include <IRProtocol.h> // just the decode_type_t enum (NEC, UNKNOWN, ...), not all of IRremote

enum Device : uint8_t { DEV_TV, DEV_FAN };

// One ID per command. kCommands[] in commands.cpp must list them in exactly
// this order. Menu order is the order of the inMenu entries.
enum CommandId : uint8_t {
  // Buttons only (not in the menu)
  CMD_TV_POWER,
  CMD_TV_UP,
  CMD_TV_DOWN,
  CMD_TV_LEFT,
  CMD_TV_RIGHT,
  CMD_TV_OK,
  CMD_FAN_POWER,
  // Menu, top to bottom
  CMD_TV_BACK,
  CMD_TV_HOME,
  CMD_TV_VOL_UP,
  CMD_TV_VOL_DOWN,
  CMD_TV_MUTE,
  CMD_TV_INPUT,
  CMD_FAN_SPEED,
  CMD_FAN_OSC,

  CMD_COUNT // not a command: the number of commands
};

struct Command {
  const char* name;       // shown in the menu, e.g. "Vol +"
  Device device;
  bool inMenu;
  // Decoded protocols (e.g. NEC): protocol, address and command are used, raw is nullptr.
  decode_type_t protocol;
  uint16_t address;
  uint16_t command;
  // Codes IRremote can't send from address + command: protocol is UNKNOWN and
  // raw holds mark/space durations in microseconds, sent at 38 kHz.
  const uint16_t* raw;
  uint16_t rawLen;
};

extern const Command kCommands[CMD_COUNT];

// "TV" or "Fan", for the menu
const char* deviceLabel(Device d);
