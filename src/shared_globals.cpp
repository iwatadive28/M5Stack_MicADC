#include "shared_definitions.h"

int16_t recordedData[RECORD_BUFFER_SIZE];
int recordIndex = 0;
bool recording = false;
unsigned int sampling_period_us = 1000000 / SAMPLE_RATE;

double vReal[FFTsamples];
double vImag[FFTsamples];
float dmax = 5.0;

InputMode currentMode = MODE_ADC;