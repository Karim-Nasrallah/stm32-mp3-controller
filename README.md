# stm32-mp3-controller

A bare metal MP3 playback controller built on an STM32F091RC. It drives a DFPlayer Mini module over UART using a single button for play, pause and track selection. Written in C with no HAL or vendor libraries. Every peripheral is configured by writing directly to registers.

**Demo:** https://youtube.com/shorts/EOvRSBnXCg0

---

## How it works

One button does everything. A short press toggles between play and pause. A long press skips to the next track and cycles through three tracks before wrapping back to the first.

The firmware waits a moment on startup for the DFPlayer to boot then sends a volume command before entering the main loop.

| Input | Action |
|---|---|
| Short press | Toggles play and pause |
| Long press | Advances to the next track and wraps from 3 back to 1 |

---

## Hardware

- STM32 NUCLEO-F091RC development board
- DFPlayer Mini MP3 module
- microSD card with three MP3 files
- Speaker
- Pushbutton

### Pin assignments

| Pin | Function |
|---|---|
| PA9 | USART1 TX to the DFPlayer RX pin |
| PC13 | Button (input, active low) |

---

## Implementation notes

**Serial setup.** USART1 runs at 9600 baud which is what the DFPlayer expects. The baud rate register is loaded with 833 since the 8 MHz system clock divided by 9600 gives 833. PA9 is switched to alternate function mode and AF1 is selected to route USART1 TX onto that pin.

**Command protocol.** Every DFPlayer command is a 10 byte packet built from the datasheet. It opens with a start byte then a version byte then a length byte then the command and its two data bytes. The last three bytes are a two byte checksum and a stop byte. The checksum is the two's complement of the sum of the six bytes between the start and the checksum itself so the module can reject a corrupted packet.

Commands used in this project:

| Command | Purpose |
|---|---|
| `0x03` | Play a specific track number |
| `0x06` | Set volume |
| `0x0D` | Play or resume |
| `0x0E` | Pause |

**Button timing.** The button is polled rather than interrupt driven. When a press is detected the firmware counts how long the button stays held and compares that count against a threshold to decide whether it was a short press or a long press. Debounce delays run on either side of the press so a noisy contact does not register twice.

**Volume.** Set once at startup. The DFPlayer accepts a range of 0 to 30 and this project uses 20.

---

## Known issues

- The firmware only transmits. The DFPlayer can send status back over its own TX line but nothing is wired to receive it so there is no way to confirm a command was accepted.
- Play state is tracked in a software variable rather than read from the module. If a track finishes on its own the firmware still thinks it is playing so the next short press sends a pause that does nothing.
- Press timing uses a software counting loop instead of a timer so the long press threshold shifts with compiler optimization settings.
- The main loop blocks while the button is held down.

---

## Building

Built and flashed with STM32CubeIDE. Connect the board over USB and use the IDE's run/debug configuration to flash. The on-board ST-LINK handles programming so no external programmer is needed.
