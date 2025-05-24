#include <M5Core2.h>
#include "input.h"
#include "display.h"
#include "audio_adc.h"
#include "audio_i2s.h"
#include "playback.h"
#include "shared_definitions.h"

void handleDisplayToggle() {
  if (M5.BtnA.wasPressed()) {
    currentMode = (currentMode == MODE_ADC) ? MODE_I2S : MODE_ADC;
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(5, 5);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.printf("Input Mode: %s", currentMode == MODE_ADC ? "ADC" : "I2S");
    beep();
    delay(300);
  }
}

void handleRecording() {
  if (M5.BtnB.wasPressed()) {
    if (!recording) {
      startRecording();
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(5, 100);
      M5.Lcd.setTextColor(RED);
      M5.Lcd.print("[REC]");
      if (currentMode == MODE_ADC) {
        recordAudioADC();
      } else {
        recordAudioI2S();
      }
      stopRecording();
    }
  }
}

void handlePlayback() {
  if (M5.BtnC.wasPressed()) {
    if (!isRecording() && getRecordedLength() > 0) {
      startPlayback();
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(5, 100);
      M5.Lcd.setTextColor(GREEN);
      M5.Lcd.print("[PLAY]");
      playAudio();
      stopPlayback();
    }
  }
}

void handleInputSwitch() {
  // do nothing (touch disabled)
}