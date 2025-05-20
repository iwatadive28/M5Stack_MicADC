#include <M5Core2.h>
#include "arduinoFFT.h"

#define MIC 36
#define SAMPLING_FREQUENCY 20000
const uint16_t FFTsamples = 256;
#define SPEAKER_PIN 25  // Core2のスピーカー用ピン

double vReal[FFTsamples];
double vImag[FFTsamples];
ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, FFTsamples, SAMPLING_FREQUENCY);

unsigned int sampling_period_us;

int X0 = 30;
int Y0 = 20;
int _height = 240 - Y0;
int _width = 320;
float dmax = 5.0;

bool showFFT = true;    // 表示モード：true=FFT, false=Time
bool lastBtnA = false;  // Aボタン押下状態

// ------------ セットアップ ------------
void setup() {
  M5.begin();
  M5.Lcd.setBrightness(30);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(5, 5);
  M5.Lcd.println("FFT Audio Monitor");

  pinMode(SPEAKER_PIN, OUTPUT);  // ピッ音用
  pinMode(MIC, INPUT);           // マイク入力

  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
}

// ------------ 波形サンプリング ------------
void sample(int nsamples) {
  for (int i = 0; i < nsamples; i++) {
    unsigned long t = micros();
    vReal[i] = (double)analogRead(MIC) / 4095.0 * 3.6 + 0.1132;  // ADC補正
    vImag[i] = 0;
    while ((micros() - t) < sampling_period_us);
  }
}

// ------------ DCオフセット除去 ------------
void DCRemoval(double *vData, uint16_t samples) {
  double mean = 0;
  for (uint16_t i = 0; i < samples; i++) {
    mean += vData[i];
  }
  mean /= samples;
  for (uint16_t i = 0; i < samples; i++) {
    vData[i] -= mean;
  }
}

// ------------ FFT表示描画 ------------
void drawChart(int nsamples) {
  int band_width = floor(_width / nsamples);
  int band_pad = band_width - 1;

  for (int band = 0; band < nsamples; band++) {
    int hpos = band * band_width + X0;
    float d = vReal[band];
    if (d > dmax) d = dmax;
    int h = (int)((d / dmax) * (_height));
    M5.Lcd.fillRect(hpos, _height - h, band_pad, h, WHITE);

    if ((band % (nsamples / 4)) == 0) {
      M5.Lcd.setCursor(hpos, _height + Y0 - 10);
      M5.Lcd.printf("%.1fkHz", ((band * 1.0 * SAMPLING_FREQUENCY) / FFTsamples / 1000));
    }
  }
}

// ------------ メインループ ------------
void loop() {
  M5.update();
  bool currentBtnA = M5.BtnA.wasPressed();

  if (currentBtnA && !lastBtnA) {
    showFFT = !showFFT;
    tone(SPEAKER_PIN, 1000, 100);  // ピッ音
  }
  lastBtnA = currentBtnA;

  sample(FFTsamples);
  DCRemoval(vReal, FFTsamples);
  M5.Lcd.fillScreen(BLACK);

  if (showFFT) {
    // FFT表示
    FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.compute(FFT_FORWARD);
    FFT.complexToMagnitude();
    drawChart(FFTsamples / 2);

    M5.Lcd.setCursor(5, 5);
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.print("Mode: FFT (A: Toggle)");

  } else {
    // 時間波形表示
    int w = M5.Lcd.width();
    int h = M5.Lcd.height();
    for (int i = 1; i < FFTsamples; i++) {
      int x0 = map(i - 1, 0, FFTsamples, 0, w);
      int x1 = map(i, 0, FFTsamples, 0, w);
      int y0 = h / 2 - (vReal[i - 1] - 1.65) * 60;
      int y1 = h / 2 - (vReal[i] - 1.65) * 60;
      M5.Lcd.drawLine(x0, y0, x1, y1, CYAN);
    }

    M5.Lcd.setCursor(5, 5);
    M5.Lcd.setTextColor(CYAN);
    M5.Lcd.print("Mode: TIME (A: Toggle)");
  }

  delay(50);
}
