#include <M5Core2.h>
#include "arduinoFFT.h"
#include <driver/i2s.h>
#include <math.h>

#define MIC 36
#define SAMPLE_RATE 10000
#define FFTsamples 256
#define RECORD_BUFFER_SIZE 32000  // 約2秒

#define I2S_PORT I2S_NUM_0
#define I2S_BCK 12
#define I2S_WS 0
#define I2S_DO 2

double vReal[FFTsamples];
double vImag[FFTsamples];
ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, FFTsamples, SAMPLE_RATE);

int16_t recordedData[RECORD_BUFFER_SIZE];
int recordIndex = 0;
bool recording = false;
bool playing = false;
bool showFFT = true;

unsigned int sampling_period_us;
int X0 = 30, Y0 = 20;
int _height = 240 - Y0, _width = 320;
float dmax = 5.0;

// スピーカー用I2S初期化
void InitI2SSpeaker() {
  i2s_driver_uninstall(I2S_PORT);
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
    .fixed_mclk = 0
  };
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_DO,
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
  i2s_zero_dma_buffer(I2S_PORT);
}

// ピー音
void beep() {
  InitI2SSpeaker();
  const int samples = SAMPLE_RATE / 1000;
  int16_t wave[samples];
  for (int i = 0; i < samples; i++) {
    wave[i] = (int16_t)(8000.0 * sinf(2.0 * PI * i / samples));
  }
  size_t bytes_written;
  for (int i = 0; i < 200; i++) {
    i2s_write(I2S_PORT, wave, sizeof(wave), &bytes_written, portMAX_DELAY);
  }
  i2s_driver_uninstall(I2S_PORT);
}

// 録音（ADC方式）
void recordAudioADC() {
  recordIndex = 0;
  unsigned long nextSampleTime = micros();
  while (recording && recordIndex < RECORD_BUFFER_SIZE) {
    if (micros() >= nextSampleTime) {
      int adc = analogRead(MIC);
      int16_t sample = map(adc, 0, 4095, -16000, 16000);
      recordedData[recordIndex++] = sample;
      nextSampleTime += sampling_period_us;  // 固定周期で更新
    }
  }
  beep();  // 録音終了時
}
// 再生（I2S）
void playAudio() {
  InitI2SSpeaker();
  size_t bytes_written;
  i2s_write(I2S_PORT, (const char *)recordedData, recordIndex * sizeof(int16_t), &bytes_written, portMAX_DELAY);
  i2s_driver_uninstall(I2S_PORT);
}

// サンプル取得（ADC）
void sample(int nsamples) {
  for (int i = 0; i < nsamples; i++) {
    unsigned long t = micros();
    vReal[i] = (double)analogRead(MIC) / 4095.0 * 3.6 + 0.1132;
    vImag[i] = 0;
    while ((micros() - t) < sampling_period_us);
  }
}

void DCRemoval(double *vData, uint16_t samples) {
  double mean = 0;
  for (int i = 0; i < samples; i++) mean += vData[i];
  mean /= samples;
  for (int i = 0; i < samples; i++) vData[i] -= mean;
}

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
      M5.Lcd.printf("%.1fkHz", ((band * 1.0 * SAMPLE_RATE) / FFTsamples / 1000));
    }
  }
}

void setup() {
  M5.begin();
  M5.Lcd.setBrightness(30);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(5, 5);
  M5.Lcd.println("FFT Audio Monitor");
  sampling_period_us = round(1000000.0 / SAMPLE_RATE);
  beep();
}

void loop() {
  M5.update();

  if (M5.BtnA.wasPressed()) {
    showFFT = !showFFT;
    beep();
  }

  if (M5.BtnB.wasPressed()) {
    if (!recording) {
      recording = true;
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(5, 100);
      M5.Lcd.setTextColor(RED);
      M5.Lcd.print("[REC]");
      recordAudioADC();
      recording = false;
    }
  }

  if (M5.BtnC.wasPressed()) {
    if (!recording && recordIndex > 0) {
      playing = true;
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(5, 100);
      M5.Lcd.setTextColor(GREEN);
      M5.Lcd.print("[PLAY]");
      playAudio();
      playing = false;
    }
  }

  // 通常表示（録音・再生中はスキップ）
  if (!recording && !playing) {
    sample(FFTsamples);
    DCRemoval(vReal, FFTsamples);
    M5.Lcd.fillScreen(BLACK);
    if (showFFT) {
      FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
      FFT.compute(FFT_FORWARD);
      FFT.complexToMagnitude();
      drawChart(FFTsamples / 2);
      M5.Lcd.setCursor(5, 5);
      M5.Lcd.setTextColor(GREEN);
      M5.Lcd.print("Mode: FFT (A: Toggle)");
    } else {
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
  }

  delay(50);
}
