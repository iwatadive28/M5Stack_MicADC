#include <M5Core2.h>
#include "display.h"
#include "fft.h"
#include "shared_definitions.h"

void updateDisplay() {
  // バックアップ用コピー
  double vRealCopy[FFTsamples];
  for (int i = 0; i < FFTsamples; i++) {
    vRealCopy[i] = vReal[i] = (double)analogRead(MIC) / 4095.0 * 3.6 + 0.1132;
    vImag[i] = 0;
  }

  removeDC(vReal, FFTsamples);
  applyFFT();

  M5.Lcd.fillScreen(BLACK);

  // FFT表示（上半分）
  int band_width = floor(320 / (FFTsamples / 2));
  for (int i = 0; i < FFTsamples / 2; i++) {
    float d = vReal[i];
    if (d > dmax) d = dmax;
    int h = (int)((d / dmax) * 90);  // 下げて余白確保
    M5.Lcd.fillRect(i * band_width, 120 - h, band_width - 1, h, WHITE);

    // 周波数ラベル（4箇所だけ、位置を上に）
    if ((i % (FFTsamples / 8)) == 0) {
      M5.Lcd.setCursor(i * band_width, 95);  // ← 以前は110、被ってた
      M5.Lcd.setTextColor(GREEN);
      M5.Lcd.printf("%.1fk", (i * 1.0 * SAMPLE_RATE) / FFTsamples / 1000);
    }
  }

  // 波形表示（下半分）スケール縮小
  for (int i = 1; i < FFTsamples; i++) {
    int x0 = map(i - 1, 0, FFTsamples, 0, 320);
    int x1 = map(i, 0, FFTsamples, 0, 320);
    int y0 = 120 + 90 - (vRealCopy[i - 1] - 1.65) * 30;  // ← スケール40→30に
    int y1 = 120 + 90 - (vRealCopy[i] - 1.65) * 30;
    M5.Lcd.drawLine(x0, y0, x1, y1, CYAN);
  }

  // モード表示
  M5.Lcd.setCursor(5, 5);
  M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.printf("Input: %s", currentMode == MODE_ADC ? "ADC" : "I2S");
}
