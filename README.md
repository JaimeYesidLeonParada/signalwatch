# SignalWatch

**A smartwatch that listens for you.** SignalWatch runs a tiny neural network on an ESP32-C6 to recognize critical sounds (an ambulance, a crying baby, a fire alarm) and alerts the wearer with a message on screen and a Morse-code vibration. It is built for people with hearing loss.

![ESP-IDF 6.0](https://img.shields.io/badge/ESP--IDF-6.0-red)
![ESP32-C6](https://img.shields.io/badge/target-ESP32--C6-blue)
![LVGL 9](https://img.shields.io/badge/UI-LVGL%209-green)
![License: MIT](https://img.shields.io/badge/license-MIT-yellow)

<p align="center">
  <img src="docs/signalwatch.jpeg" alt="SignalWatch running on the Waveshare ESP32-C6 watch board" width="480">
</p>

## Features

- **On-device sound recognition.** An [Edge Impulse](https://edgeimpulse.com) model classifies 1 s of 16 kHz audio from the onboard microphone. Nothing leaves the watch.
- **Three alert classes today:** ambulance, baby crying and fire alarm. Adding a class is one row in [`alert_catalog.c`](smartwatch_project/main/alert_catalog.c).
- **Graded confidence.** A sliding window of predictions turns a score into one of three messages ("Possible...", "It looks like...", "Alert!") so the watch does not cry wolf.
- **Morse vibration.** Each class has its own letter (A, B, F) played on the vibration motor, so you can tell the alert apart without looking.
- **Live feedback on screen:** audio level meter, one icon per class, clock with date, and battery indicator.
- **Always-on clock** backed by a PCF85063 RTC.

## Hardware

| Part | Used for |
|---|---|
| [Waveshare ESP32-C6 1.69" touch LCD watch board](https://www.waveshare.com/) | MCU, 1.69" ST7789 display, CST816 touch, PCF85063 RTC, ES8311 audio codec |
| Vibration motor on GPIO 18 | Haptic alerts |
| LiPo battery | Power (voltage read through the ADC) |

## Build and flash

You need [ESP-IDF v6.0](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/get-started/). Managed components (`esp_lvgl_port`, `esp_codec_dev`) are downloaded automatically on the first build.

```bash
cd smartwatch_project
idf.py set-target esp32c6
idf.py build
idf.py -p <PORT> flash monitor
```

A [dev container](smartwatch_project/.devcontainer) is also provided.

## Architecture

The firmware follows SOLID principles. Modules do not know each other: `app_init.c` is the only place that creates them and connects them with callbacks and handles passed by parameter.

```
 microphone ─► ai_module ──(listener callbacks)──► alert_dispatcher ─┬─► ui (queue) ─► ui_task ─► widgets
              capture_task                                           │     clock · battery · vu · alerts · notification
              classify_task ─► ei_wrapper (model)                    └─► vibrator_module (Morse)
                               alert_module (sliding window)
 rtc_module ─────────(callback)────────────────────────────────────────► ui
 battery_module ─────(callback)────────────────────────────────────────► ui
```

- Only `ui_task` draws on the screen. Everything else sends messages through the queue in `ui.c`.
- `alert_catalog` holds the per-class data (label, Morse letter, glyph, texts), so the rest of the code is class-agnostic.
- `app_init.c` documents the initialization order step by step.

| Area | Files |
|---|---|
| Composition root | `app_init.c` |
| Audio and AI | `bsp_es8311`, `ai_module`, `ei_wrapper`, `alert_module`, `alert_catalog`, `alert_dispatcher` |
| Hardware | `display_hw`, `i2c_bus`, `touch_module`, `button_module`, `rtc_module`, `battery_module`, `vibrator_module` |
| UI | `ui`, `ui_clock`, `ui_battery`, `ui_vu`, `ui_alerts`, `ui_notification` |
| Pure logic (unit tested) | `morse`, `rtc_codec`, `battery_math`, `clock_format`, `audio_level`, `audio_preroll`, plus `alert_module` and `alert_catalog` |

Logic that does not need hardware lives in its own file, so it can be tested on a PC. The hardware modules only call it. `alert_dispatcher` receives its outputs (UI and vibrator) from `app_init.c` instead of including them.

## Tests

The unit tests run on the PC with gcc and [Unity](https://github.com/ThrowTheSwitch/Unity). They need no board, no ESP-IDF and no FreeRTOS, and they finish in about a second. They cover the logic that can break without anyone noticing:

| Test | What it protects |
|---|---|
| `alert_module` | Alert levels, 3-sample sliding window, per-class history |
| `alert_catalog` | Unique labels, playable Morse text, ASCII-only screen text |
| `morse` | Morse table and the timing of dots, dashes and gaps |
| `rtc_codec` | RTC register decoding and range validation |
| `battery_math` | Voltage to percentage, smoothing filter, charging threshold |
| `clock_format` | 12-hour clock and date text |
| `audio_level`, `audio_preroll` | Audio level and the pre-trigger buffer order |
| `alert_dispatcher` | When to vibrate and notify, with fake outputs |

You need gcc, CMake and Ninja. From `smartwatch_project`:

```bash
cmake -S tests -B tests/build -G Ninja
cmake --build tests/build
ctest --test-dir tests/build --output-on-failure
```

On Windows, `tests/run_tests.ps1` does the same and looks for the tools in the usual MSYS2 and ESP-IDF folders. The tests are plain CTest executables, so the Testing panel of VS Code (CMake Tools extension) shows them too.

Every test checks one behavior. Drivers, tasks, queues and drawing are not unit tested: they are checked on the watch.

## Add a new sound class

1. Train or extend the Edge Impulse model with the new label and replace the library in `smartwatch_project/components/ei_model`.
2. Add one row to `CATALOG` in `alert_catalog.c` (label, Morse letter, icon glyph, three texts).
3. Rebuild and run the [tests](#tests). The catalog tests catch a repeated label, Morse text that cannot be played, and accents in screen text (the font has none).

## Roadmap

Planned work is tracked in [Issues](https://github.com/JaimeYesidLeonParada/signalwatch/issues):

- Set date and time from the watch
- User-facing battery settings
- Take the LVGL lock when registering the touch input device
- More sound classes

## License

The firmware is released under the [MIT License](LICENSE). The Edge Impulse SDK in `smartwatch_project/components/ei_model` and the fonts keep their own licenses.
