#include <Wire.h>
#include <tusb-hid.h>
#include <tusb.h>
#include <Keyboard.h>
#include <Mouse.h>
#include <Adafruit_MCP23X17.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 5
#define ROW0_PIN 6
#define ROW1_PIN 7
#define ROW2_PIN 8
#define ROW3_PIN 9
#define ROW4_PIN 10
#define ROW5_PIN 11
#define ROW6_PIN 12
#define ROW7_PIN 13
#define ENC_A_PIN 14
#define ENC_B_PIN 15
#define RGB_DATA_PIN 16
#define JOY_BUTTON_PIN 22
#define JOY_X_PIN 26
#define JOY_Y_PIN 27

constexpr uint8_t ROWS = 8;
constexpr uint8_t COLS = 16;
constexpr uint8_t OLED_ADDR = 0x3C;
constexpr uint8_t MCP_A_ADDR = 0x20;
constexpr uint8_t MCP_B_ADDR = 0x21;
constexpr uint16_t RGB_LED_COUNT = 128;
constexpr uint8_t RGB_BRIGHTNESS = 48; // 19% cap: approximately 1.5 A worst-case LED load.

const uint8_t rowPins[ROWS] = {ROW0_PIN, ROW1_PIN, ROW2_PIN, ROW3_PIN, ROW4_PIN, ROW5_PIN, ROW6_PIN, ROW7_PIN};

Adafruit_MCP23X17 columnsA;
Adafruit_MCP23X17 columnsB;
Adafruit_SSD1306 display(128, 64, &Wire, -1);
Adafruit_NeoPixel pixels(RGB_LED_COUNT, RGB_DATA_PIN, NEO_GRB + NEO_KHZ800);

constexpr uint16_t DEBOUNCE_MS = 8;
constexpr uint16_t CONSUMER_PREVIOUS_TRACK = 0x00B6;
constexpr uint16_t CONSUMER_PLAY_PAUSE = 0x00CD;
constexpr uint16_t CONSUMER_NEXT_TRACK = 0x00B5;

// USB HID Keyboard/Keypad-page usages. These are deliberately different from
// the main-row ASCII digits so Windows identifies this cluster as a numpad.
constexpr uint8_t KEYPAD_NUM_LOCK = 0x53;
constexpr uint8_t KEYPAD_DIVIDE = 0x54;
constexpr uint8_t KEYPAD_MULTIPLY = 0x55;
constexpr uint8_t KEYPAD_MINUS = 0x56;
constexpr uint8_t KEYPAD_PLUS = 0x57;
constexpr uint8_t KEYPAD_ENTER = 0x58;
constexpr uint8_t KEYPAD_1 = 0x59;
constexpr uint8_t KEYPAD_2 = 0x5A;
constexpr uint8_t KEYPAD_3 = 0x5B;
constexpr uint8_t KEYPAD_4 = 0x5C;
constexpr uint8_t KEYPAD_5 = 0x5D;
constexpr uint8_t KEYPAD_6 = 0x5E;
constexpr uint8_t KEYPAD_7 = 0x5F;
constexpr uint8_t KEYPAD_8 = 0x60;
constexpr uint8_t KEYPAD_9 = 0x61;
constexpr uint8_t KEYPAD_0 = 0x62;
constexpr uint8_t KEYPAD_DECIMAL = 0x63;

bool stableMatrix[ROWS][COLS] = {};
bool rawMatrix[ROWS][COLS] = {};
uint32_t matrixChangedAt[ROWS][COLS] = {};
bool rawJoyButton = false;
bool stableJoyButton = false;
uint32_t joyChangedAt = 0;
bool matrixMiddleHeld = false;
bool joystickMiddleHeld = false;
int lastEncoderA = HIGH;
uint32_t lastEncoderAt = 0;
int lastJoyXZone = 0;
int lastJoyYZone = 0;
uint32_t lastDisplayUpdate = 0;
uint32_t consumerReleaseAt = 0;
bool consumerReportHeld = false;

// Physical switch map for revision A. A zero is the one intentionally unpopulated
// spare intersection; every other position has one switch and one 1N4148W diode.
// The 17 numpad keys occupy R5C4-R5C6, R5C14-R5C15, and R7C3-R7C14.
const uint8_t keyMap[ROWS][COLS] = {
  {KEY_ESC, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12, KEY_PRINT_SCREEN, KEY_SCROLL_LOCK, KEY_PAUSE},
  {'`', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', KEY_BACKSPACE, KEY_INSERT, KEY_HOME},
  {KEY_TAB, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\\', KEY_DELETE, KEY_END},
  {KEY_CAPS_LOCK, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', KEY_RETURN, KEY_MENU, KEY_RIGHT_GUI, KEY_PAGE_UP},
  {KEY_LEFT_SHIFT, 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', KEY_RIGHT_SHIFT, KEY_UP_ARROW, 0, KEY_RIGHT_CTRL, KEY_PAGE_DOWN},
  {KEY_LEFT_CTRL, KEY_LEFT_GUI, KEY_LEFT_ALT, ' ', KEYPAD_NUM_LOCK, KEYPAD_DIVIDE, KEYPAD_MULTIPLY, KEY_RIGHT_ALT, KEY_RIGHT_GUI, KEY_MENU, KEY_RIGHT_CTRL, KEY_LEFT_ARROW, KEY_DOWN_ARROW, KEY_RIGHT_ARROW, KEYPAD_0, KEYPAD_DECIMAL},
  {'w', 'a', 's', 'd', KEY_UP_ARROW, KEY_LEFT_ARROW, KEY_DOWN_ARROW, KEY_RIGHT_ARROW, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, KEYPAD_MINUS, KEYPAD_7, KEYPAD_8, KEYPAD_9, KEYPAD_PLUS, KEYPAD_4, KEYPAD_5, KEYPAD_6, KEYPAD_1, KEYPAD_2, KEYPAD_3, KEYPAD_ENTER, 0}
};

// Special positions are deliberately separate from keyMap to allow future remapping.
constexpr uint8_t MOUSE_LEFT_ROW = 6, MOUSE_LEFT_COL = 8;
constexpr uint8_t MOUSE_MIDDLE_ROW = 6, MOUSE_MIDDLE_COL = 9;
constexpr uint8_t MOUSE_RIGHT_ROW = 6, MOUSE_RIGHT_COL = 10;
constexpr uint8_t MEDIA_PREV_ROW = 6, MEDIA_PREV_COL = 11;
constexpr uint8_t MEDIA_PLAY_ROW = 6, MEDIA_PLAY_COL = 12;
constexpr uint8_t MEDIA_NEXT_ROW = 6, MEDIA_NEXT_COL = 13;
constexpr uint8_t CHATGPT_ROW = 6, CHATGPT_COL = 14;
constexpr uint8_t DISCORD_ROW = 6, DISCORD_COL = 15;
constexpr uint8_t INSTAGRAM_ROW = 7, INSTAGRAM_COL = 0;
constexpr uint8_t CHESS_ROW = 7, CHESS_COL = 1;
constexpr uint8_t ORION_ROW = 7, ORION_COL = 2;
constexpr uint8_t ENCODER_SW_ROW = 7, ENCODER_SW_COL = 15;

bool isPosition(uint8_t row, uint8_t col, uint8_t wantedRow, uint8_t wantedCol) {
  return row == wantedRow && col == wantedCol;
}

void showStatus() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("ORION keyboard"));
  display.println(F("Profile: Windows HID"));
  display.println(F("RGB: capped at 19%"));
  display.print(F("Joy X/Y: "));
  display.print(analogRead(JOY_X_PIN));
  display.print('/');
  display.println(analogRead(JOY_Y_PIN));
  display.display();
}

void setRgbStartupColor() {
  for (uint16_t i = 0; i < RGB_LED_COUNT; ++i) {
    pixels.setPixelColor(i, pixels.Color(0, 20, 35));
  }
  pixels.show();
}

void sendConsumerTap(uint16_t usage) {
  // Arduino-Pico's built-in Keyboard device declares Consumer Control as HID
  // report ID 2. Send a Consumer-page usage followed by a separate empty report.
  // Keyboard.write() only emits Keyboard/Keypad-page reports and must not be used
  // for media controls.
  if (!tud_hid_ready()) return;
  tud_hid_report(2, &usage, sizeof(usage));
  consumerReportHeld = true;
  consumerReleaseAt = millis() + 12;
}

void serviceConsumerRelease() {
  if (!consumerReportHeld || millis() < consumerReleaseAt || !tud_hid_ready()) return;
  const uint16_t noConsumerUsage = 0;
  tud_hid_report(2, &noConsumerUsage, sizeof(noConsumerUsage));
  consumerReportHeld = false;
}

void sendShortcut(uint8_t slot) {
  // Windows can map these five standard chords without relying on unsupported
  // F13-F17 reports: Ctrl+Alt+F1 through Ctrl+Alt+F5.
  Keyboard.press(KEY_LEFT_CTRL);
  Keyboard.press(KEY_LEFT_ALT);
  Keyboard.write((uint8_t)(KEY_F1 + slot));
  Keyboard.releaseAll();
}

void updateMiddleMouseButton() {
  if (matrixMiddleHeld || joystickMiddleHeld) Mouse.press(MOUSE_MIDDLE);
  else Mouse.release(MOUSE_MIDDLE);
}

void handleMatrixChange(uint8_t row, uint8_t col, bool pressed) {
  if (isPosition(row, col, MOUSE_LEFT_ROW, MOUSE_LEFT_COL)) { if (pressed) Mouse.press(MOUSE_LEFT); else Mouse.release(MOUSE_LEFT); return; }
  if (isPosition(row, col, MOUSE_MIDDLE_ROW, MOUSE_MIDDLE_COL)) {
    matrixMiddleHeld = pressed;
    updateMiddleMouseButton();
    return;
  }
  if (isPosition(row, col, MOUSE_RIGHT_ROW, MOUSE_RIGHT_COL)) { if (pressed) Mouse.press(MOUSE_RIGHT); else Mouse.release(MOUSE_RIGHT); return; }
  if (pressed && isPosition(row, col, MEDIA_PREV_ROW, MEDIA_PREV_COL)) { sendConsumerTap(CONSUMER_PREVIOUS_TRACK); return; }
  if (pressed && isPosition(row, col, MEDIA_PLAY_ROW, MEDIA_PLAY_COL)) { sendConsumerTap(CONSUMER_PLAY_PAUSE); return; }
  if (pressed && isPosition(row, col, MEDIA_NEXT_ROW, MEDIA_NEXT_COL)) { sendConsumerTap(CONSUMER_NEXT_TRACK); return; }
  if (pressed && isPosition(row, col, CHATGPT_ROW, CHATGPT_COL)) { sendShortcut(0); return; }
  if (pressed && isPosition(row, col, DISCORD_ROW, DISCORD_COL)) { sendShortcut(1); return; }
  if (pressed && isPosition(row, col, INSTAGRAM_ROW, INSTAGRAM_COL)) { sendShortcut(2); return; }
  if (pressed && isPosition(row, col, CHESS_ROW, CHESS_COL)) { sendShortcut(3); return; }
  if (pressed && isPosition(row, col, ORION_ROW, ORION_COL)) { sendShortcut(4); return; }
  // Ctrl+Alt+F6 is the host-side binding for encoder push (mute by default).
  if (pressed && isPosition(row, col, ENCODER_SW_ROW, ENCODER_SW_COL)) { sendShortcut(5); return; }

  const uint8_t key = keyMap[row][col];
  if (key != 0) {
    if (pressed) Keyboard.press(key); else Keyboard.release(key);
  }
}

bool readColumn(uint8_t col) {
  return col < 8 ? columnsA.digitalRead(col) == LOW : columnsB.digitalRead(col - 8) == LOW;
}

void scanMatrix() {
  const uint32_t now = millis();
  for (uint8_t row = 0; row < ROWS; ++row) {
    digitalWrite(rowPins[row], LOW);
    delayMicroseconds(30);
    for (uint8_t col = 0; col < COLS; ++col) {
      const bool sampledPressed = readColumn(col);
      if (sampledPressed != rawMatrix[row][col]) {
        rawMatrix[row][col] = sampledPressed;
        matrixChangedAt[row][col] = now;
      }
      if (rawMatrix[row][col] != stableMatrix[row][col] && now - matrixChangedAt[row][col] >= DEBOUNCE_MS) {
        stableMatrix[row][col] = rawMatrix[row][col];
        handleMatrixChange(row, col, stableMatrix[row][col]);
      }
    }
    digitalWrite(rowPins[row], HIGH);
  }
}

void scanEncoder() {
  const int encoderA = digitalRead(ENC_A_PIN);
  const uint32_t now = millis();
  if (encoderA != lastEncoderA && now - lastEncoderAt >= DEBOUNCE_MS) {
    lastEncoderAt = now;
    if (encoderA == LOW) {
      if (digitalRead(ENC_B_PIN) != encoderA) Mouse.move(0, 0, 1); else Mouse.move(0, 0, -1);
    }
    lastEncoderA = encoderA;
  }
}

void scanJoystick() {
  const bool sampledJoyPressed = digitalRead(JOY_BUTTON_PIN) == LOW;
  const uint32_t now = millis();
  if (sampledJoyPressed != rawJoyButton) {
    rawJoyButton = sampledJoyPressed;
    joyChangedAt = now;
  }
  if (rawJoyButton != stableJoyButton && now - joyChangedAt >= DEBOUNCE_MS) {
    stableJoyButton = rawJoyButton;
    joystickMiddleHeld = stableJoyButton;
    updateMiddleMouseButton();
  }

  const int x = analogRead(JOY_X_PIN);
  const int y = analogRead(JOY_Y_PIN);
  const int xZone = x < 1100 ? -1 : (x > 3000 ? 1 : 0);
  const int yZone = y < 1100 ? -1 : (y > 3000 ? 1 : 0);
  if (xZone != lastJoyXZone || yZone != lastJoyYZone) {
    Mouse.move(xZone * 5, yZone * 5, 0);
    lastJoyXZone = xZone;
    lastJoyYZone = yZone;
  }
}

void setup() {
  Wire.setSDA(I2C_SDA_PIN);
  Wire.setSCL(I2C_SCL_PIN);
  Wire.begin();

  for (uint8_t rowPin : rowPins) {
    pinMode(rowPin, OUTPUT);
    digitalWrite(rowPin, HIGH);
  }
  pinMode(ENC_A_PIN, INPUT_PULLUP);
  pinMode(ENC_B_PIN, INPUT_PULLUP);
  pinMode(JOY_BUTTON_PIN, INPUT_PULLUP);

  columnsA.begin_I2C(MCP_A_ADDR, &Wire);
  columnsB.begin_I2C(MCP_B_ADDR, &Wire);
  for (uint8_t pin = 0; pin < 8; ++pin) {
    columnsA.pinMode(pin, INPUT_PULLUP);
    columnsB.pinMode(pin, INPUT_PULLUP);
  }

  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  pixels.begin();
  pixels.setBrightness(RGB_BRIGHTNESS);
  setRgbStartupColor();
  Keyboard.begin();
  Mouse.begin();
  lastEncoderA = digitalRead(ENC_A_PIN);
  lastEncoderAt = millis();
  showStatus();
}

void loop() {
  serviceConsumerRelease();
  scanMatrix();
  scanEncoder();
  scanJoystick();
  if (millis() - lastDisplayUpdate >= 500) {
    lastDisplayUpdate = millis();
    showStatus();
  }
  delay(1);
}
