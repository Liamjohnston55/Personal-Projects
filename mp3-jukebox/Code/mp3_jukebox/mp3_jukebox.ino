/*
===============================================================================
Project: ESP32 MP3 Jukebox
Author: Liam Johnston
Target: ESP32-WROOM-32
Framework: Arduino ESP32 Core

Overview:
Button-driven MP3 jukebox using an ESP32 + DFPlayer Mini with OLED feedback
and analog volume control. A static track list is defined in firmware.
Selected tracks loop until stopped.

Hardware Interfaces:
- UART2  : DFPlayer Mini (9600 baud)
- I2C    : SSD1306 OLED (0x3C)
- GPIO   : Active-low pushbuttons (internal pull-ups)
- ADC    : Potentiometer volume input (0..4095 → 0..30)
- Audio  : DFPlayer → PAM8403 amplifier → speakers

Notes:
- Song titles are defined in firmware.
- DFPlayer does not support filename enumeration over UART.
===============================================================================
*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HardwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include "esp_sleep.h"

// -------- Logging --------
#ifndef LOG_ENABLE
#define LOG_ENABLE 1
#endif

#if LOG_ENABLE
  #define LOG_INFO(msg)   do { Serial.print("[INFO] ");  Serial.println(msg); } while (0)
  #define LOG_WARN(msg)   do { Serial.print("[WARN] ");  Serial.println(msg); } while (0)
  #define LOG_ERROR(msg)  do { Serial.print("[ERROR] "); Serial.println(msg); } while (0)
#else
  #define LOG_INFO(msg)   do {} while (0)
  #define LOG_WARN(msg)   do {} while (0)
  #define LOG_ERROR(msg)  do {} while (0)
#endif

// -------- OLED --------
static constexpr int SCREEN_WIDTH  = 128;
static constexpr int SCREEN_HEIGHT = 64;
static constexpr uint8_t OLED_ADDR = 0x3C;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// -------- Pin Definitions --------
static constexpr int BTN_DOWN = 27;
static constexpr int BTN_PLAY = 14;
static constexpr int BTN_BACK = 12;
static constexpr int BTN_UP   = 13;

static constexpr int VOL_PIN  = 34;

static constexpr int DF_RX_PIN = 16;  // ESP32 RX2  <- DFPlayer TX
static constexpr int DF_TX_PIN = 17;  // ESP32 TX2  -> DFPlayer RX

// -------- DFPlayer --------
HardwareSerial DFSerial(2);
DFRobotDFPlayerMini mp3;

// -------- Song List --------
struct SongNameEntry {
  uint16_t trackNum;
  const char* name;
};

static SongNameEntry SONG_NAMES[] = {
  {1, "Miku"},
  {2, "Minecraft C418"},
  {3, "Medium Waterfall"},
  {4, "Pokemon SS music"},
};

static constexpr int SONG_NAME_COUNT =
    sizeof(SONG_NAMES) / sizeof(SONG_NAMES[0]);

// -------- Timing --------
static constexpr uint32_t MENU_SLEEP_TIMEOUT_MS   = 60000;
static constexpr uint32_t BOOT_SCREEN_MS          = 5000;
static constexpr uint32_t MENU_DEBOUNCE_MS        = 120;
static constexpr uint32_t PLAY_START_DELAY_MS     = 200;
static constexpr uint32_t PLAYBACK_UI_REFRESH_MS  = 1000;

// -------- State --------
enum UiState : uint8_t {
  STATE_MENU,
  STATE_PLAYING
};

static UiState uiState = STATE_MENU;

static int      menuIndex      = 0;
static uint16_t currentTrack   = 0;
static bool     isPlaying      = false;

static uint32_t songStartMs        = 0;
static uint32_t lastInteractionMs  = 0;
static uint32_t lastUiActionMs     = 0;
static uint32_t lastPlaybackDrawMs = 0;

static int currentVolume = 20;

// -------- Button State Tracking --------
static bool lastDown  = false;
static bool lastUp    = false;
static bool lastPlay  = false;
static bool lastBack  = false;

static bool buttonFell(int pin, bool &lastState) {
  bool now = (digitalRead(pin) == LOW);
  bool pressedEdge = (now && !lastState);
  lastState = now;
  return pressedEdge;
}

// -------- Helpers --------
static const char* getSongNameByIndex(int idx) {
  if (idx < 0 || idx >= SONG_NAME_COUNT) return "?";
  return SONG_NAMES[idx].name;
}

static uint16_t getTrackNumByIndex(int idx) {
  if (idx < 0 || idx >= SONG_NAME_COUNT) return 1;
  return SONG_NAMES[idx].trackNum;
}

static bool uiRateLimitOk(uint32_t nowMs) {
  if (nowMs - lastUiActionMs < MENU_DEBOUNCE_MS)
    return false;

  lastUiActionMs = nowMs;
  return true;
}

// -------- Boot Screen --------
static void drawBootScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 4);
  display.println("Minecraft Jukebox");

  display.display();
}

// -------- Menu UI --------
static void drawMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("Select Song");

  if (SONG_NAME_COUNT == 0) {
    display.setCursor(0, 20);
    display.println("No songs listed");
    display.display();
    return;
  }

  int baseY = 20;
  int lineH = 14;

  int prevIdx = (menuIndex - 1 + SONG_NAME_COUNT) % SONG_NAME_COUNT;
  int nextIdx = (menuIndex + 1) % SONG_NAME_COUNT;

  display.setCursor(0, baseY);
  display.print("  ");
  display.println(getSongNameByIndex(prevIdx));

  display.setCursor(0, baseY + lineH);
  display.print("> ");
  display.println(getSongNameByIndex(menuIndex));

  display.setCursor(0, baseY + 2 * lineH);
  display.print("  ");
  display.println(getSongNameByIndex(nextIdx));

  display.display();
}

// -------- Playback UI --------
static void drawPlayback() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 18);
  display.println("Playing:");
  display.println(getSongNameByIndex(menuIndex));

  uint32_t elapsed = (millis() - songStartMs) / 1000;
  uint32_t min = elapsed / 60;
  uint32_t sec = elapsed % 60;

  display.print("Time: ");
  display.print(min);
  display.print(":");
  if (sec < 10) display.print("0");
  display.println(sec);

  int volPct = map(currentVolume, 0, 30, 0, 100);
  display.print("Vol: ");
  display.print(volPct);
  display.println("%");

  display.display();
}

// -------- Volume --------
static void updateVolume() {
  int raw = analogRead(VOL_PIN);
  int vol = map(raw, 0, 4095, 0, 30);

  if (abs(vol - currentVolume) >= 1) {
    currentVolume = vol;
    mp3.volume(currentVolume);
  }
}

// -------- Playback Control --------
static void startTrackByIndex(int idx) {
  uint16_t trackNum = getTrackNumByIndex(idx);

  currentTrack = trackNum;
  isPlaying = true;
  songStartMs = millis();

  LOG_INFO(String("Starting track ") + trackNum);

  mp3.play(trackNum);
  delay(PLAY_START_DELAY_MS);
  mp3.loop(trackNum);

  drawPlayback();
}

static void stopTrack() {
  LOG_INFO("Stopping playback");
  mp3.stop();
  isPlaying = false;
  currentTrack = 0;
  uiState = STATE_MENU;
  drawMenu();
}

// -------- Sleep --------
static void goToSleep() {
  LOG_INFO("Entering deep sleep (menu idle timeout)");

  esp_sleep_enable_ext1_wakeup(
    (1ULL << BTN_UP) |
    (1ULL << BTN_DOWN) |
    (1ULL << BTN_PLAY) |
    (1ULL << BTN_BACK),
    ESP_EXT1_WAKEUP_ANY_LOW
  );

  delay(100);
  esp_deep_sleep_start();
}

// -------- Setup --------
void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_UP,   INPUT_PULLUP);
  pinMode(BTN_PLAY, INPUT_PULLUP);
  pinMode(BTN_BACK, INPUT_PULLUP);
  pinMode(VOL_PIN,  INPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    LOG_ERROR("OLED init failed");
    while (true) delay(100);
  }

  drawBootScreen();
  delay(BOOT_SCREEN_MS);

  DFSerial.begin(9600, SERIAL_8N1, DF_RX_PIN, DF_TX_PIN);

  if (!mp3.begin(DFSerial, true, true)) {
    LOG_ERROR("DFPlayer not detected");
    while (true) delay(500);
  }

  mp3.outputDevice(DFPLAYER_DEVICE_SD);
  mp3.EQ(DFPLAYER_EQ_NORMAL);

  mp3.volume(currentVolume);

  drawMenu();

  lastInteractionMs  = millis();
  lastUiActionMs     = millis();
  lastPlaybackDrawMs = millis();
}

// -------- Main Loop --------
void loop() {
  uint32_t now = millis();

  updateVolume();

  bool downPressed = buttonFell(BTN_DOWN, lastDown);
  bool upPressed   = buttonFell(BTN_UP,   lastUp);
  bool playPressed = buttonFell(BTN_PLAY, lastPlay);
  bool backPressed = buttonFell(BTN_BACK, lastBack);

  if (downPressed || upPressed || playPressed || backPressed)
    lastInteractionMs = now;

  if (uiState == STATE_MENU &&
      (now - lastInteractionMs > MENU_SLEEP_TIMEOUT_MS)) {
    goToSleep();
  }

  if (uiState == STATE_MENU) {
    if (downPressed && uiRateLimitOk(now)) {
      menuIndex = (menuIndex + 1) % SONG_NAME_COUNT;
      drawMenu();
    }

    if (upPressed && uiRateLimitOk(now)) {
      menuIndex = (menuIndex - 1 + SONG_NAME_COUNT) % SONG_NAME_COUNT;
      drawMenu();
    }

    if (playPressed) {
      uiState = STATE_PLAYING;
      startTrackByIndex(menuIndex);
      lastPlaybackDrawMs = now;
    }
  }
  else {
    if (backPressed) {
      stopTrack();
    }
    else if (now - lastPlaybackDrawMs >= PLAYBACK_UI_REFRESH_MS) {
      drawPlayback();
      lastPlaybackDrawMs = now;
    }
  }

  // Small loop pacing delay to reduce CPU load
  delay(10);
}