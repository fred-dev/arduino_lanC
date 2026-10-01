/*
  arduino_lanC

  Five-button LANC remote: record start/stop, zoom in/out and focus near/far,
  sent to the LANC port of a video camera (Canon calls it REMOTE).

  Based on Martin Koch's Simple LANC Remote (2011), tested with a Canon XF300.
  Interface circuit:
  http://controlyourcamera.blogspot.com/2011/02/arduino-controlled-video-recording-over.html

  How LANC works: the camera sends an 8-byte frame every video field (about
  every 20 ms), each byte as a start bit, 8 data bits (least significant bit
  first) and a stop bit at 9600 baud. A remote writes its command into bytes 0
  and 1 by pulling the line low for each 1 bit, and repeats it for a few
  frames so the camera accepts it.

  HIGH on PIN_LANC_IN means the LANC line is at +5 V (idle).

  "LANC" is a registered trademark of Sony.
*/

#include <Arduino.h>

// Pins
constexpr uint8_t PIN_LANC_OUT   = 7;   // pulls the LANC line low through a transistor
constexpr uint8_t PIN_LANC_IN    = 11;  // reads the LANC line
constexpr uint8_t PIN_REC        = 6;   // buttons connect the pin to GND
constexpr uint8_t PIN_ZOOM_OUT   = 5;
constexpr uint8_t PIN_ZOOM_IN    = 4;
constexpr uint8_t PIN_FOCUS_NEAR = 3;
constexpr uint8_t PIN_FOCUS_FAR  = 2;

// Timing
constexpr unsigned int  BIT_US          = 104 - 8;  // one bit at 9600 baud, minus about 8 us for digitalWrite on a 16 MHz AVR
constexpr unsigned long FRAME_GAP_US    = 5000;     // the line stays high at least this long between frames
constexpr unsigned long SYNC_TIMEOUT_MS = 200;      // give up if no LANC frames arrive (camera off or unplugged)
constexpr uint8_t       REPEATS         = 5;        // frames to repeat each command
constexpr unsigned long DEBOUNCE_MS     = 30;

struct LancCommand {
  uint8_t byte0;
  uint8_t byte1;
};

// Commands tested with the Canon XF300
constexpr LancCommand CMD_REC        = {0x18, 0x33};  // start/stop recording
constexpr LancCommand CMD_FOCUS_NEAR = {0x28, 0x47};  // manual focus only
constexpr LancCommand CMD_FOCUS_FAR  = {0x28, 0x45};
constexpr LancCommand CMD_FOCUS_AUTO = {0x28, 0x41};

// Zoom speed 0 (slowest) to 7 (fastest)
constexpr LancCommand zoomIn(uint8_t speed)  { return {0x28, uint8_t((speed & 0x07) * 2)}; }
constexpr LancCommand zoomOut(uint8_t speed) { return {0x28, uint8_t(0x10 + (speed & 0x07) * 2)}; }
constexpr uint8_t ZOOM_SPEED = 4;

// Record is a toggle, so it needs a clean press, not a held button.
struct Button {
  uint8_t pin;
  bool stableState;
  bool lastReading;
  unsigned long lastChange;

  // True once per press, after the contacts have settled.
  bool wasPressed() {
    bool reading = digitalRead(pin);
    if (reading != lastReading) {
      lastReading = reading;
      lastChange = millis();
    }
    if (millis() - lastChange > DEBOUNCE_MS && reading != stableState) {
      stableState = reading;
      return stableState == LOW;
    }
    return false;
  }
};
Button recButton = {PIN_REC, HIGH, HIGH, 0};

bool isHeld(uint8_t pin) {
  return digitalRead(pin) == LOW;
}

// Waits for the long high pause between frames. Returns when the line has just
// dropped for the start bit of byte 0, or false if no frames arrive.
bool waitForFrameStart() {
  unsigned long start = millis();
  while (millis() - start < SYNC_TIMEOUT_MS) {
    // pulseIn returns 0 on timeout, so a missing camera can't hang the loop.
    if (pulseIn(PIN_LANC_IN, HIGH, 25000UL) >= FRAME_GAP_US) {
      return true;
    }
  }
  return false;
}

// Waits for the line to drop (start bit of the next byte).
bool waitForStartBit(unsigned long timeoutUs) {
  unsigned long start = micros();
  while (digitalRead(PIN_LANC_IN) == HIGH) {
    if (micros() - start > timeoutUs) return false;
  }
  return true;
}

// Writes 8 bits, least significant first. Interrupts are off so the timing holds.
void writeByte(uint8_t value) {
  noInterrupts();
  for (uint8_t bit = 0; bit < 8; bit++) {
    digitalWrite(PIN_LANC_OUT, (value >> bit) & 0x01);
    delayMicroseconds(BIT_US);
  }
  digitalWrite(PIN_LANC_OUT, LOW);  // release the line for the stop bit
  interrupts();
}

bool sendCommand(const LancCommand & cmd) {
  for (uint8_t i = 0; i < REPEATS; i++) {
    if (!waitForFrameStart()) return false;
    delayMicroseconds(BIT_US);       // skip the start bit of byte 0
    writeByte(cmd.byte0);
    delayMicroseconds(10);           // make sure we are into the stop bit
    if (!waitForStartBit(2000)) return false;
    delayMicroseconds(BIT_US);       // skip the start bit of byte 1
    writeByte(cmd.byte1);
    // Bytes 2-7 belong to the camera; wait for the next frame.
  }
  return true;
}

void send(const LancCommand & cmd, const char * name) {
  static unsigned long lastWarning = 0;
  if (!sendCommand(cmd) && millis() - lastWarning > 2000) {
    Serial.print(F("No LANC signal, could not send "));
    Serial.println(name);
    lastWarning = millis();
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_LANC_IN, INPUT);
  pinMode(PIN_LANC_OUT, OUTPUT);
  digitalWrite(PIN_LANC_OUT, LOW);  // leave the LANC line at +5 V

  pinMode(PIN_REC, INPUT_PULLUP);
  pinMode(PIN_ZOOM_OUT, INPUT_PULLUP);
  pinMode(PIN_ZOOM_IN, INPUT_PULLUP);
  pinMode(PIN_FOCUS_NEAR, INPUT_PULLUP);
  pinMode(PIN_FOCUS_FAR, INPUT_PULLUP);

  Serial.println(F("LANC remote ready"));
}

void loop() {
  if (recButton.wasPressed()) {
    send(CMD_REC, "record");
  }
  // Zoom and focus repeat while the button is held.
  if (isHeld(PIN_ZOOM_IN))    send(zoomIn(ZOOM_SPEED), "zoom in");
  if (isHeld(PIN_ZOOM_OUT))   send(zoomOut(ZOOM_SPEED), "zoom out");
  if (isHeld(PIN_FOCUS_NEAR)) send(CMD_FOCUS_NEAR, "focus near");
  if (isHeld(PIN_FOCUS_FAR))  send(CMD_FOCUS_FAR, "focus far");
}
