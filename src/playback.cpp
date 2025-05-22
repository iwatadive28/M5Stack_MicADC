#include "shared_definitions.h"
#include <M5Core2.h>
#include <driver/i2s.h>
#include "playback.h"
#include "audio_adc.h"

bool playing = false;

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
    .bck_io_num = I2S_BCK, .ws_io_num = I2S_WS, .data_out_num = I2S_DO, .data_in_num = I2S_PIN_NO_CHANGE
  };
  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
  i2s_zero_dma_buffer(I2S_PORT);
}

void beep() {
  InitI2SSpeaker();
  const int samples = SAMPLE_RATE / 1000;
  int16_t wave[samples];
  for (int i = 0; i < samples; i++)
    wave[i] = (int16_t)(8000.0 * sinf(2.0 * PI * i / samples));
  size_t bytes_written;
  for (int i = 0; i < 200; i++)
    i2s_write(I2S_PORT, wave, sizeof(wave), &bytes_written, portMAX_DELAY);
  i2s_driver_uninstall(I2S_PORT);
}

void playAudio() {
  InitI2SSpeaker();
  size_t bytes_written;
  i2s_write(I2S_PORT, (const char*)getRecordedData(), getRecordedLength() * sizeof(int16_t), &bytes_written, portMAX_DELAY);
  i2s_driver_uninstall(I2S_PORT);
}

bool isPlaying() { return playing; }
void startPlayback() { playing = true; }
void stopPlayback() { playing = false; }