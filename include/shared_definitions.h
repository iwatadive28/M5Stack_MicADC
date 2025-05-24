#ifndef SHARED_DEFINITIONS_H
#define SHARED_DEFINITIONS_H

#include <stdint.h>
#include <Arduino.h>

#define MIC 36
#define SAMPLE_RATE 10000
#define FFTsamples 256
#define RECORD_BUFFER_SIZE 32000

#define I2S_PORT I2S_NUM_0
#define I2S_BCK 12
#define I2S_WS 0
#define I2S_DO 2

extern int16_t recordedData[RECORD_BUFFER_SIZE];
extern int recordIndex;
extern bool recording;
extern unsigned int sampling_period_us;

extern double vReal[FFTsamples];
extern double vImag[FFTsamples];
extern float dmax;

enum InputMode { MODE_ADC, MODE_I2S };
extern InputMode currentMode;

#endif