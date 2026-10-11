# ORION — Advanced RGB Gaming Keyboard

> **Command more. Do more.**  
> A custom, feature-packed mechanical keyboard concept for Windows.

![ORION RGB Gaming Keyboard Concept](ORION%20RGB%20Gaming%20Keyboard%20Concept.png)

## Overview

ORION is a concept for a full-size mechanical keyboard that combines conventional typing with gaming controls, media controls, programmable shortcuts, RGB lighting, and a compact status display.

The design aims to bring multiple controls into one configurable desktop device while retaining a familiar full-size keyboard layout.

## Design highlights

- **Full-size keyboard layout** with function keys, navigation cluster, arrow keys, and numeric keypad.
- **Secondary WASD cluster** for gaming.
- **Secondary arrow cluster** for additional navigation or gaming control.
- **Built-in analog joystick** for configurable input.
- **Mouse controls** with left, middle, and right buttons.
- **Rotary encoder** for scrolling, volume, or other configurable actions, with a push function.
- **Media controls** for previous track, play/pause, and next track.
- **Quick-launch keys** intended for ChatGPT, Discord, Instagram, World Chess/FIDE, and the Orion control panel.
- **Top OLED/status bar** concept for time, system information, app status, RGB mode, and custom widgets.
- **RGB lighting zones**, including per-key lighting and side/underglow accents.

## Electronics architecture

The current proposed architecture uses:

- Raspberry Pi Pico as the main controller.
- Two MCP23017 I²C GPIO expanders for the 16-column key-matrix interface.
- An 8-row × 16-column matrix architecture, providing up to 128 matrix positions.
- Individual switch diodes for matrix isolation.
- An I²C SSD1306 128 × 64 OLED module.
- A dual-axis analog joystick with push switch.
- A rotary encoder with push switch.
- WS2812B-compatible addressable RGB LEDs.
- A 74AHCT125 buffer for interfacing the Pico's 3.3 V RGB data signal to the 5 V LED data input.
- A separately protected 5 V RGB power input, with fuse and bulk capacitance.

These are the proposed components and interfaces documented during the design stage. Exact footprints, physical switch coordinates, connector pinouts, and electrical implementation must be verified in the final CAD design.

## Firmware goals

The firmware is intended to support:

- USB HID keyboard input.
- Numeric keypad keys using keypad-specific HID usages.
- Mouse-button input.
- Joystick analog input and push switch.
- Rotary encoder input and push action.
- Media-control Consumer Control HID reports.
- Configurable shortcut key combinations.
- OLED status output.
- Addressable RGB lighting effects.

Application shortcuts may require user configuration on Windows; the keyboard alone cannot guarantee that a particular application will launch unless the corresponding shortcut is configured.

## Design files

Add the current project artifacts here as they become available:

| File | Purpose |
|---|---|
| `README.md` | Project overview and design status |
| `ORION RGB Gaming Keyboard Concept.png` | Visual concept reference |
| `Orion_BOM.csv` | Bill of Materials |
| `Orion_Connections.csv` | Pin-by-pin connection summary |
| KiCad/EasyEDA schematic | Editable electrical schematic, when available |
| PCB layout | Physical board layout, when available |
| Firmware source | Controller firmware |

## Current status

The concept, component selection, connection plan, and firmware structure are being developed. The next steps are to complete the editable schematic and PCB layout, run electrical/design-rule checks, compile the firmware for the selected Pico core, and test the design.

**This repository documents a design in progress.** Unless explicitly updated with test evidence, the hardware should not be considered manufactured, electrically validated, or production-ready.

## Safety and verification notes

- Keep the Pico's GPIO at 3.3 V logic.
- Do not connect the external 5 V RGB supply to the Pico's 3.3 V rail.
- Use the level shifter between the Pico's RGB data output and the LED data input.
- Share ground between the controller and external RGB supply.
- Verify the fuse, connector, wire/trace capacity, and LED power budget for the final number of LEDs.
- Confirm diode orientation matches the firmware's matrix scan polarity.
- Check for existing I²C pull-ups on modules before adding additional pull-ups.
- Run schematic ERC and PCB DRC before manufacturing.

## License

No license has been specified yet. Add a license before inviting others to reuse or distribute the design.
