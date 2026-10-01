# arduino_lanC

Arduino LANC remote for video cameras: five buttons for record start/stop, zoom in, zoom out, focus near and focus far, sent to the camera's LANC port (Canon calls it REMOTE).

Based on Martin Koch's *Simple LANC Remote* (2011) and tested with a Canon XF300. Updated in 2026 to current Arduino style, with debounced record, timeouts when no camera is connected, and serial status messages at 115200 baud.

## Wiring

| Pin | Use |
|---|---|
| 11 | LANC line in (through the interface circuit) |
| 7 | LANC line out (drives a transistor that pulls the line low) |
| 6 | Record button to GND |
| 5 | Zoom out button to GND |
| 4 | Zoom in button to GND |
| 3 | Focus near button to GND |
| 2 | Focus far button to GND |

The interface circuit is described [here](http://controlyourcamera.blogspot.com/2011/02/arduino-controlled-video-recording-over.html) and shown in `LANC_REMOTE.png`. Buttons use the internal pull-ups, so no resistors are needed.

Zoom speed (0-7) is `ZOOM_SPEED` in the sketch. Focus commands only work with the camera in manual focus.

## Build

Open `arduino_lanC.ino` in the Arduino IDE and upload to an Uno or any 16 MHz AVR board. No libraries needed. The bit timing assumes a 16 MHz AVR; other boards need `BIT_US` adjusted.

`LANC Documenation/` has reference notes on the LANC protocol and command codes.
