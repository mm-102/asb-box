# Snake Charmer - Box Firmware

This repository contains the firmware for the "Box" module of the Snake Charmer project. It runs on an ESP32-S2 microcontroller using the Zephyr RTOS and a custom board definition (`sumo_6`) - [board repository](https://github.com/mm-102/zephyr-minisumo2022).

The Box acts as a secure CoAPS server and a Wi-Fi Access Point, processing incoming sensor data from the "Flute" (instrument), managing the game's state machine, and providing visual feedback via a 74HC595-driven LED strip.

##  Setup & Building

This project is built using Zephyr's `west` meta-tool. The repository contains a `west.yml` manifest file at its root.

### 1. Initialize the Workspace

First, set up a Python virtual environment and install `west`. Then, initialize the workspace directly from this repository's URL:

```bash
python -m venv zephyrproject/.venv
source zephyrproject/.venv/bin/activate

pip install west

west init -m https://github.com/mm-102/asb-box asb-box
cd asb-box

west update

```

### 2. Build and Flash

The project uses a custom board definition named `sumo_6`. To build and flash the firmware:

```bash
cd asb-box

west build -b sumo_6

west flash

```

## Configuration

All essential network and security configurations are located in the `prj.conf` file. You can modify the following parameters there before building:

* **Wi-Fi AP Settings:** IP address and Pre-Shared Key (PSK) for the Access Point hosted by the Box.
* **CoAPS Security:** CoAP DTLS identity (ID) and Pre-Shared Key (PSK).

## Usage & Game Modes

When you power up the ESP32 board for the very first time (or when the NVS flash is completely empty), the box will automatically boot into **Code Write Mode**.

### Code Write Mode (Recording a Sequence)

In this mode, you create the secret combination of movements and button presses using the Flute.

* **LED Animation:** For every entry successfully recorded, one LED turns fully ON. The remaining LEDs display a shifting loop animation indicating the system is waiting for input (e.g., if 1 entry is present on a 4-LED setup, the pattern shifts like: `1100` -> `1010` -> `1001` -> `1100`...).
* **Saving:** Once you are done entering the sequence via the instrument, press the **Save button (Small Red Button)** to save the code to the non-volatile storage (NVS).
* **Closing:** Press the **Close button (Big Black Button)** to shift the box into Code Check Mode. *Warning: If you press Close without pressing Save first, the newly entered code will NOT be saved!*

### Code Check Mode (Playing the Game)

In this mode, the player must recreate the exact combination stored in the Box.

* **LED Animation:** For every correctly guessed part of the sequence, an LED turns fully ON. The LED representing the *next* required step will blink rapidly (e.g., if 1 entry is guessed on a 4-LED setup: `1000` -> `1100` -> `1000` -> `1100`...).
* **Gameplay:** Enter combinations via the instrument.
* **Correct guess:** Progress advances, and the next LED lights up.
* **Mistake:** An error animation plays, and the progress resets to 0.


* **Victory:** If you manage to input the entire sequence correctly, a special victory animation will play!
* **Resetting/Reprogramming:** While in Code Check Mode, pressing the **Save button (Small Red Button)** will immediately switch the system back into **Code Write Mode**. *(Note: In the final physical design, this button is supposed to be accessible only when the box is already open).*

## CoAP Endpoints (Debug)

The Box exposes several CoAPS endpoints. These are primarily intended for internal system communication with the Flute, but can be triggered manually for debugging purposes.

| Endpoint | Method | Payload | Description |
| --- | --- | --- | --- |
| `/state` | `GET` | *None* | Returns the current game state and progress. |
| `/reset` | `GET` | *None* | Resets the player's current progress to 0. |
| `/entry` | `POST` | `entry_msg_t` | Input a code element (simulates an instrument action). |
| `/set_state` | `POST` | `state_msg_t` | Manually force the game into a specific state. |
| `/save` | `POST` | *None* | Manually trigger the save action to write the current sequence to NVS. |

*For detailed instructions on how to interact with these endpoints (including payload structures and python client examples), please refer to the [Flute (asb-rpi) repository](https://github.com/mm-102/asb-rpi).*