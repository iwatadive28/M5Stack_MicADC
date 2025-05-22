#include "shared_definitions.h"
#include <M5Core2.h>
#include "audio_adc.h"
#include "playback.h"


void recordAudioADC() {
  recordIndex = 0;
  unsigned long nextSampleTime = micros();
  while (recording && recordIndex < RECORD_BUFFER_SIZE) {
    if (micros() >= nextSampleTime) {
      int adc = analogRead(MIC);
      recordedData[recordIndex++] = map(adc, 0, 4095, -16000, 16000);
      nextSampleTime += sampling_period_us;
    }
  }
  beep();
}

int16_t* getRecordedData() { return recordedData; }
int getRecordedLength() { return recordIndex; }
bool isRecording() { return recording; }
void startRecording() { recording = true; }
void stopRecording() { recording = false; }