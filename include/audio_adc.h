#ifndef AUDIO_ADC_H
#define AUDIO_ADC_H

#include <stdint.h>

#define MIC 36
#define RECORD_BUFFER_SIZE 32000
#define SAMPLE_RATE 10000

extern int16_t recordedData[RECORD_BUFFER_SIZE];
extern int recordIndex;
extern bool recording;
extern unsigned int sampling_period_us;

void recordAudioADC();
int16_t* getRecordedData();
int getRecordedLength();
bool isRecording();
void startRecording();
void stopRecording();

#endif