# K530 QMK + Bluetooth (reverse-engineered) + OpenRGB

Custom QMK firmware for the **Redragon K530 Draconic** (Sonix SN32F248B + PixArt PAR2801QN-GHVC BLE module), built on top of the [SonixQMK/qmk_firmware](https://github.com/SonixQMK/qmk_firmware) fork, branch `sn32_master_openrgb`.

> **Status: experimental / work in progress.** Flash at your own risk. Keep a backup of your stock firmware and know how to enter the bootloader before flashing (see below).

## Goals

1. **Full per-key RGB control via OpenRGB / Skydimo** (direct mode), instead of the limited stock EVision preset-only RGB.
2. **Custom keymap**: Fn / Fn2 layers, Magic Fn (Caps Lock hold), custom-recorded macros with dedicated LED status indicators, reactive per-key lighting.
3. **Reverse-engineered Bluetooth support** so the keyboard can keep BLE connectivity while running fully custom QMK firmware (community alternative firmwares drop BT entirely).

## Current status

| Feature | Status |
|---|---|
| Base QMK build (USB, matrix, RGB matrix) | Working |
| OpenRGB direct-mode protocol | Working (see OPENRGB_ENABLE in rules.mk) |
| Skydimo custom HID plugin | Working |
| Custom keymap (Fn/Fn2/macros/reactive RGB) | Working |
| Bluetooth (SPI0 link to PAR2801QN) | In progress - transport layer (SPI0 init/IRQ, both physical toggles) confirmed working on hardware; RX frame synchronization still being debugged |

## Hardware background

- Main MCU: Sonix SN32F248B (marked VS11K09A-1 on board)
- BLE module: PixArt PAR2801QN-GHVC (aka PAJ2801UA-40), connected over SPI (module = SPI master, MCU = SPI slave)
- 2 physical toggles: BT on/off (GPIO1.13) and host slot 1/2/3 (GPIO0.6 + GPIO0.7)
- No Jumploader is required for this chip family (SN32F248B) - flash at offset 0x00

Full reverse-engineering notes (register maps, disassembled functions, protocol frame layout) will be written up in /docs.

## Building

```
qmk setup SonixQMK/qmk_firmware -b sn32_master_openrgb   # or clone this repo directly
qmk compile -kb redragon/k530 -km default
```

Flashing: use Sonix Flasher (https://github.com/SonixQMK/sonix-flasher) or the official EVision tool, offset 0x00.

## Bootloader / recovery

If a build behaves unexpectedly, the chip can be forced into bootloader mode by shorting the BOOT pin (pin 3 on the MCU) to GND while powering on - this works independently of whatever firmware is flashed.

## Credits

- Base platform: SonixQMK/qmk_firmware (https://github.com/SonixQMK/qmk_firmware)
- K530 keyboard support: original QMK community contributors
- Bluetooth protocol reverse-engineering, custom keymap, and OpenRGB/Skydimo integration: this repository

## Disclaimer

This is a hobby reverse-engineering project, not an officially supported product. No warranty. Bluetooth support is incomplete/experimental at this stage - see Status above.
