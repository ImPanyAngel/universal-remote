# Universal IR Remote

A small handheld remote that replaces several infrared remotes in my home, starting with my TV and fan. A tiny screen lets me scroll through every saved command and send it, and a few quick buttons cover the ones I use most.

## Why

I have separate remotes for different devices, and they're never where I need them. This project puts them all in one device that I build and program myself.

It's also my first main embedded project: a practical way to learn C++, wiring, GPIO, displays and infrared signals before moving on to bigger builds.

## How it works

Most home remotes communicate using **infrared (IR) light**. When you press a button, an IR LED flashes on and off very quickly (around 38,000 times a second) in a pattern that the device recognises.

The project has two halves:

1. **Capture:** an IR receiver records the signal from each button on my original remotes, and I save the codes.
2. **Playback:** an IR transmitter replays a saved code when I choose it on the remote.

Everything runs on the remote itself. It needs no Wi-Fi, no phone app and no internet.

## Features

- **Menu on an OLED screen:** scroll through all saved commands and press OK to send one
- **Quick buttons:** one press for the most-used commands, such as TV power and fan power
- **Command table:** each command (TV volume, fan speed and so on) is one entry, so adding a new device is easy
- **Works with any IR device:** TVs, fans, soundbars, air conditioners and lamps, as long as they use infrared

## Hardware

| Part | Role |
|---|---|
| Espressif ESP32-C6-DevKitC-1 | The microcontroller that runs everything |
| 0.96" OLED display (128 × 64, I2C) | Shows the command menu |
| IR receiver module (38 kHz) | Captures codes from the original remotes |
| IR transmitter module (940 nm) | Sends the codes to the devices |
| 2N3904 transistors | To boost the transmitter's range if needed |
| Resistor set | For the transistor circuit and general use |
| Tactile buttons, 6 × 6 mm | Up, Down, OK and 3 quick buttons |
| 830-point breadboard and jumper wires | For building the prototype without soldering |

**Tools**
- YIHUA 926LED-IV soldering station
- Velleman DVM010 multimeter
- Digital calipers

**Board highlights (ESP32-C6)**
- Single-core RISC-V processor at 160 MHz
- Wi-Fi 6, Bluetooth LE, Zigbee and Thread built in, useful for future smart-home features
- USB-C for power and flashing
- Genuine Espressif board with reliable documentation

## Software

- **Language:** C++
- **Tooling:** VS Code with PlatformIO
- **Framework:** Arduino for ESP32 (core 3.x, through the pioarduino platform)
- **Libraries:** IRremote for capturing and sending IR signals, U8g2 for the OLED display

## Limitations

- **Line of sight:** IR needs a clear path to the device, just like the original remotes.
- **Range:** the transmitter module may only reach a couple of metres. A transistor circuit fixes this.
- **Toggle commands:** some devices use the same signal for on and off, so the remote can't know whether a device is currently on.
- **Infrared only:** devices that use radio (RF) remotes need extra hardware.

## Roadmap

1. Set up PlatformIO and upload a first test program
2. Capture the TV and fan codes with the IR receiver
3. Show the command menu on the OLED
4. Add the navigation and quick buttons
5. Send codes with the IR transmitter and test the range
6. Boost the range with a 2N3904 circuit if needed
7. Move the circuit from the breadboard to perfboard
8. Add a LiPo battery, a charger and deep sleep
9. Design and 3D-print a case

## Possible future versions

- **More devices:** soundbar, air conditioner, lamps
- **Grouped menus:** pick a device first, then its commands
- **Learning mode:** capture and save new codes on the remote itself, without reflashing
- **RF support:** add a 433 MHz module for radio-controlled devices
- **Smart-home control:** use the C6's Wi-Fi, Zigbee or Thread to control more than infrared devices
