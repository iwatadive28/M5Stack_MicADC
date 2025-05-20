#include <M5Core2.h>
#include "arduinoFFT.h"
#include <driver/i2s.h>
#include <math.h>

#define MIC 36
#define SAMPLING_FREQUENCY 20000
const uint16_t FFTsamples = 256;

#define SPEAKER_I2S_PORT I2S_NUM_0
#define SAMPLE_RATE 16000

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

void i2sToneInit() {
  M5.Axp.SetSpkEnable(true);  // スピーカーアンプ ON

  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
      .sample_rate = SAMPLE_RATE,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
      .communication_format = I2S_COMM_FORMAT_I2S_MSB,
      .intr_alloc_flags = 0,
      .dma_buf_count = 8,
      .dma_buf_len = 64,
      .use_apll = false,
      .tx_desc_auto_clear = true,
      .fixed_mclk = 0};

  i2s_pin_config_t pin_config = {
      .bck_io_num = 12,
      .ws_io_num = 0,
      .data_out_num = 2,
      .data_in_num = I2S_PIN_NO_CHANGE};

  i2s_driver_install(SPEAKER_I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(SPEAKER_I2S_PORT, &pin_config);
  i2s_zero_dma_buffer(SPEAKER_I2S_PORT);
}

void i2sTone(uint32_t freq, uint32_t duration_ms) {
  const int samples = SAMPLE_RATE / freq;
  int16_t wave[samples];
  for (int i = 0; i < samples; i++) {
    wave[i] = (int16_t)(8000.0 * sinf(2.0 * PI * i / samples));
  }

  size_t bytes_written;
  int loops = (SAMPLE_RATE * duration_ms) / (samples * 1000);
  for (int i = 0; i < loops; i++) {
    i2s_write(SPEAKER_I2S_PORT, wave, sizeof(wave), &bytes_written, portMAX_DELAY);
  }
}

// ------------ セットアップ ------------
void setup() {
  M5.begin();
  M5.Lcd.setBrightness(30);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(5, 5);
  M5.Lcd.println("FFT Audio Monitor");

  // スピーカーアンプを有効にする
  i2sToneInit();  // 必ず最初に呼ぶ（初期化）
  i2sTone(1000, 200);  // 1kHz のピッ音（200ms）

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
    i2sTone(1000, 200);  // 1kHz のピッ音（200ms）
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
