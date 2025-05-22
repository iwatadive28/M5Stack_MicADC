#include "shared_definitions.h"
#include <M5Core2.h>
#include "display.h"
#include "audio_adc.h"
#include "audio_i2s.h"
#include "playback.h"
#include "fft.h"
#include "input.h"

void setup() {
  M5.begin();
  M5.Lcd.setBrightness(30);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(5, 5);
  M5.Lcd.println("FFT Audio Monitor");
  beep();
}

void loop() {
  M5.update();
  handleDisplayToggle();
  handleRecording();
  handlePlayback();
  if (!isRecording() && !isPlaying()) {
    updateDisplay();
  }
  handleInputSwitch();  // タッチでADC/I2S切替
  delay(50);
}