# project specific files
SRC = ../../../drivers/led/sn32/matrix_sn32f24xx.c
SRC += config_led.c
SRC += keymaps/default/bluetooth.c

# MCU name
MCU = SN32F248BF

# Build Options
#   comment out to disable the options.
#
LTO_ENABLE = no
BACKLIGHT_ENABLE = no
MAGIC_ENABLE = yes
MAGIC_KEYCODE_ENABLE = yes
BOOTMAGIC_ENABLE = yes # Virtual DIP switch configuration
MOUSEKEY_ENABLE = no    # Mouse keys
EXTRAKEY_ENABLE = no   # Audio control and System control
CONSOLE_ENABLE = yes    # Console for debug — ВКЛЮЧЕНО для bring-up тестирования
                          # Bluetooth-драйвера (см. bluetooth.c, план из 4 шагов).
                          # Верните в "no" после того, как связь заработает
                          # стабильно, если консоль больше не нужна.
COMMAND_ENABLE = no     # Commands for debug and configuration
SLEEP_LED_ENABLE = no   # Breathing sleep LED during USB suspend
NKRO_ENABLE = no        # USB Nkey Rollover
AUDIO_ENABLE = no
RGBLIGHT_ENABLE = no
SERIAL_LINK_ENABLE = no
WAIT_FOR_USB = no
CUSTOM_MATRIX = yes
KEYBOARD_SHARED_EP = yes

# Custom RGB matrix handling
RGB_MATRIX_ENABLE = yes
RGB_MATRIX_DRIVER = SN32F24xB
OPENRGB_ENABLE = no
DYNAMIC_MACRO_ENABLE = yes

# --- Bluetooth (custom-driver, реализован вручную в keymaps/default/bluetooth.c,
# НЕ через BLUETOOTH_ENABLE/BLUETOOTH_DRIVER=custom — эта инфраструктура НЕ
# подтверждена как существующая в данном форке SonixQMK, поэтому интеграция
# сделана напрямую через keyboard_post_init_user()/matrix_scan_user() в
# keymap.c, без опоры на возможно отсутствующий core-механизм) ---
OPT_DEFS += -DK530_BT_DEBUG   # включает вывод в консоль (см. план bring-up
                                # в bluetooth.c) — уберите после отладки,
                                # если печать в консоль больше не нужна.
