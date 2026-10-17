// The command table: every IR code the remote can send.
//
// All codes were captured with the capture program (src/capture/) and are
// stored as raw timings: mark, space, mark, space, ... in microseconds.
//
// TV (TCL): RCA protocol. 4 ms + 4 ms header, then 24 bits: 4-bit address,
//   8-bit command, then the same 12 bits inverted. IRremote has no RCA sender,
//   so we replay the raw timings. Captured with the TV unplugged, because the
//   remote uses Bluetooth while it's paired and only falls back to IR without it.
// Fan (Honeywell): its own pulse-width protocol, 12 pulses (long = 1, short = 0).
#include <stddef.h> // size_t
#include "commands.h"

// Returns the number of elements in a C array, worked out by the compiler.
template <size_t N>
constexpr uint16_t rawLen(const uint16_t (&)[N]) { return N; }

// --- TV (RCA, address 0xF) ---
static const uint16_t kTvPowerRaw[] = {  // RCA command 0x2A
    4030,3970, 530,1970, 480,1970, 480,2020, 480,1970, 480,1020, 480,1020, 480,2020,
    480,970, 530,1970, 480,970, 530,1970, 530,970, 530,970, 530,970, 530,970,
    530,970, 530,1970, 480,1970, 480,1020, 480,2020, 480,1020, 480,1970, 480,1020,
    480,1970, 530
};

static const uint16_t kTvUpRaw[] = {  // RCA command 0x59
    3980,4020, 480,1970, 480,2020, 480,1970, 480,2020, 430,1020, 480,2020, 480,1020,
    480,2020, 430,2020, 480,1020, 480,1020, 480,2020, 430,1020, 480,1020, 480,1020,
    480,1020, 480,2020, 480,970, 530,1970, 480,1020, 480,1020, 480,2020, 430,2020,
    480,1020, 480
};

static const uint16_t kTvDownRaw[] = {  // RCA command 0x58
    3930,4070, 430,2020, 480,2020, 430,2020, 480,1970, 480,1020, 480,2020, 480,970,
    530,1970, 480,2020, 480,970, 480,1020, 480,1020, 480,1020, 480,1020, 480,1020,
    480,1020, 480,2020, 480,1020, 480,1970, 480,1020, 480,1020, 480,2020, 480,1970,
    480,2020, 480
};

static const uint16_t kTvLeftRaw[] = {  // RCA command 0x56
    3980,4020, 480,2020, 480,1970, 480,2020, 430,2020, 480,1020, 480,2020, 430,1020,
    480,2020, 480,1020, 480,1970, 480,2020, 480,1020, 480,1020, 480,970, 480,1020,
    480,1020, 480,2020, 480,1020, 480,2020, 430,1020, 480,2020, 480,1020, 480,1020,
    480,2020, 430
};

static const uint16_t kTvRightRaw[] = {  // RCA command 0x57
    3980,4020, 480,1970, 480,2020, 480,1970, 480,2020, 430,1020, 480,2020, 480,1020,
    480,2020, 430,1020, 480,2020, 480,1970, 480,2020, 480,1020, 480,970, 530,970,
    530,970, 530,1970, 480,1020, 480,2020, 430,1020, 480,2020, 480,1020, 480,1020,
    480,1020, 480
};

static const uint16_t kTvOkRaw[] = {  // RCA command 0xF4
    3980,4020, 480,2020, 430,2020, 480,1970, 480,2020, 430,2020, 480,2020, 430,2020,
    480,2020, 430,1020, 480,2020, 480,1020, 480,1020, 480,1020, 480,1020, 480,1020,
    480,1020, 480,1020, 480,1020, 480,970, 480,1020, 480,2020, 480,1020, 480,2020,
    430,2020, 480
};

static const uint16_t kTvBackRaw[] = {  // RCA command 0x27
    4030,3970, 530,1970, 480,1970, 530,1970, 480,1970, 480,1020, 480,1020, 480,2020,
    480,970, 530,970, 530,1970, 480,1970, 530,1970, 480,970, 530,970, 530,970,
    530,970, 530,1970, 480,2020, 430,1020, 480,2020, 480,2020, 430,1020, 480,1020,
    480,1020, 480
};

static const uint16_t kTvHomeRaw[] = {  // RCA command 0x08
    3980,4020, 480,1970, 530,1970, 480,1970, 530,1970, 480,970, 530,970, 530,970,
    530,970, 530,1970, 480,1020, 480,1020, 480,1020, 480,1020, 480,1020, 480,1020,
    480,1020, 480,2020, 480,1970, 480,1970, 530,1970, 480,970, 530,1970, 530,1970,
    480,1970, 480
};

static const uint16_t kTvVolUpRaw[] = {  // RCA command 0x2F
    3980,4020, 480,1970, 530,1970, 480,1970, 530,1970, 480,970, 530,970, 530,1970,
    480,1020, 480,2020, 480,1970, 480,2020, 480,1970, 480,970, 530,970, 530,970,
    530,970, 530,1970, 530,1970, 480,970, 530,1970, 480,1020, 480,1020, 480,1020,
    480,1020, 480
};

static const uint16_t kTvVolDownRaw[] = {  // RCA command 0x2E
    3980,4020, 480,1970, 480,2020, 480,1970, 480,2020, 480,1020, 480,1020, 480,1970,
    480,1020, 480,2020, 480,1970, 480,2020, 480,1020, 480,1020, 480,1020, 480,970,
    530,970, 530,1970, 480,1970, 530,970, 530,1970, 480,970, 530,970, 530,970,
    530,1970, 480
};

static const uint16_t kTvMuteRaw[] = {  // RCA command 0x3F
    3980,4020, 480,2020, 430,2020, 480,2020, 430,2020, 480,1020, 480,1020, 480,2020,
    430,2020, 480,2020, 430,2020, 480,1970, 480,2020, 430,1020, 480,1020, 480,1020,
    480,1020, 480,2020, 480,2020, 430,1020, 480,1020, 480,1020, 480,1020, 480,1020,
    480,1020, 480
};

static const uint16_t kTvInputRaw[] = {  // RCA command 0xA3
    4030,3970, 530,1970, 480,1970, 530,1970, 480,1970, 530,1970, 480,1020, 480,1970,
    530,970, 530,970, 480,1020, 480,2020, 480,1970, 480,1020, 480,1020, 480,1020,
    480,1020, 480,1020, 480,1970, 530,970, 530,1970, 480,1970, 530,1970, 480,1020,
    480,1020, 480
};

// --- Fan (Honeywell) ---
static const uint16_t kFanPowerRaw[] = {
    1330,420, 1330,420, 430,1320, 1280,420, 1330,420, 430,1320, 430,1320, 380,1370,
    380,1320, 1330,370, 480,1320, 380
};

static const uint16_t kFanSpeedRaw[] = {
    1330,370, 1330,420, 430,1320, 1330,420, 1280,420, 1330,420, 430,1320, 430,1270,
    430,1320, 1330,420, 1330,370, 480
};

static const uint16_t kFanOscRaw[] = {
    1330,370, 1330,420, 430,1320, 1330,420, 1280,420, 1330,420, 430,1320, 430,1320,
    430,1320, 380,1320, 1330,420, 1280
};

// Must list every CommandId, in the same order as the enum in commands.h.
const Command kCommands[CMD_COUNT] = {
  // name         device   inMenu protocol  addr cmd  raw              length
  {"Power",       DEV_TV,  false, UNKNOWN,  0,   0,   kTvPowerRaw,     rawLen(kTvPowerRaw)},     // CMD_TV_POWER
  {"Up",          DEV_TV,  false, UNKNOWN,  0,   0,   kTvUpRaw,        rawLen(kTvUpRaw)},        // CMD_TV_UP
  {"Down",        DEV_TV,  false, UNKNOWN,  0,   0,   kTvDownRaw,      rawLen(kTvDownRaw)},      // CMD_TV_DOWN
  {"Left",        DEV_TV,  false, UNKNOWN,  0,   0,   kTvLeftRaw,      rawLen(kTvLeftRaw)},      // CMD_TV_LEFT
  {"Right",       DEV_TV,  false, UNKNOWN,  0,   0,   kTvRightRaw,     rawLen(kTvRightRaw)},     // CMD_TV_RIGHT
  {"OK",          DEV_TV,  false, UNKNOWN,  0,   0,   kTvOkRaw,        rawLen(kTvOkRaw)},        // CMD_TV_OK
  {"Power",       DEV_FAN, false, UNKNOWN,  0,   0,   kFanPowerRaw,    rawLen(kFanPowerRaw)},    // CMD_FAN_POWER
  {"Back",        DEV_TV,  true,  UNKNOWN,  0,   0,   kTvBackRaw,      rawLen(kTvBackRaw)},      // CMD_TV_BACK
  {"Home",        DEV_TV,  true,  UNKNOWN,  0,   0,   kTvHomeRaw,      rawLen(kTvHomeRaw)},      // CMD_TV_HOME
  {"Vol +",       DEV_TV,  true,  UNKNOWN,  0,   0,   kTvVolUpRaw,     rawLen(kTvVolUpRaw)},     // CMD_TV_VOL_UP
  {"Vol -",       DEV_TV,  true,  UNKNOWN,  0,   0,   kTvVolDownRaw,   rawLen(kTvVolDownRaw)},   // CMD_TV_VOL_DOWN
  {"Mute",        DEV_TV,  true,  UNKNOWN,  0,   0,   kTvMuteRaw,      rawLen(kTvMuteRaw)},      // CMD_TV_MUTE
  {"Input",       DEV_TV,  true,  UNKNOWN,  0,   0,   kTvInputRaw,     rawLen(kTvInputRaw)},     // CMD_TV_INPUT
  {"Speed",       DEV_FAN, true,  UNKNOWN,  0,   0,   kFanSpeedRaw,    rawLen(kFanSpeedRaw)},    // CMD_FAN_SPEED
  {"Oscillate",   DEV_FAN, true,  UNKNOWN,  0,   0,   kFanOscRaw,      rawLen(kFanOscRaw)},      // CMD_FAN_OSC
};

// Fails the build if an entry is added or removed without updating CommandId.
static_assert(sizeof(kCommands) / sizeof(kCommands[0]) == CMD_COUNT,
              "kCommands must have exactly one entry per CommandId");

const char* deviceLabel(Device d) {
  switch (d) {
    case DEV_TV:  return "TV";
    case DEV_FAN: return "Fan";
  }
  return "?";
}
