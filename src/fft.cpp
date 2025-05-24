#include "fft.h"
#include "shared_definitions.h"
#include "arduinoFFT.h"
#include <M5Core2.h>

extern double vReal[FFTsamples];
extern double vImag[FFTsamples];
extern float dmax;

void sampleADC() {
  for (int i = 0; i < FFTsamples; i++) {
    unsigned long t = micros();
    vReal[i] = (double)analogRead(MIC) / 4095.0 * 3.6 + 0.1132;
    vImag[i] = 0;
    while ((micros() - t) < sampling_period_us);
  }
}

void removeDC(double *vData, int len) {
  double mean = 0;
  for (int i = 0; i < len; i++) mean += vData[i];
  mean /= len;
  for (int i = 0; i < len; i++) vData[i] -= mean;
}

void applyFFT() {
  ArduinoFFT<double> FFT(vReal, vImag, FFTsamples, SAMPLE_RATE);
  FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(FFT_FORWARD);
  FFT.complexToMagnitude();
}

void drawFFT() {
  int band_width = floor(320 / (FFTsamples / 2));
  for (int i = 0; i < FFTsamples / 2; i++) {
    float d = vReal[i];
    if (d > dmax) d = dmax;
    int h = (int)((d / dmax) * 200);
    M5.Lcd.fillRect(i * band_width, 240 - h, band_width - 1, h, WHITE);
  }
}

void drawWaveform() {
  int w = M5.Lcd.width();
  int h = M5.Lcd.height();
  for (int i = 1; i < FFTsamples; i++) {
    int x0 = map(i - 1, 0, FFTsamples, 0, w);
    int x1 = map(i, 0, FFTsamples, 0, w);
    int y0 = h / 2 - (vReal[i - 1] - 1.65) * 60;
    int y1 = h / 2 - (vReal[i] - 1.65) * 60;
    M5.Lcd.drawLine(x0, y0, x1, y1, CYAN);
  }
}