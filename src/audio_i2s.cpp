#include "shared_definitions.h"
#include <M5Core2.h>
#include <driver/i2s.h>
#include "audio_i2s.h"
#include "playback.h"

void InitI2SMic() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_MSB,
    .intr_alloc_flags = 0,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };
  i2s_pin_config_t pin_config = {
    .bck_io_num = 0, .ws_io_num = 2, .data_out_num = I2S_PIN_NO_CHANGE, .data_in_num = 34
  };
  i2s_driver_uninstall(I2S_PORT);
  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
}

void recordAudioI2S() {
  recordIndex = 0;
  InitI2SMic();
  int32_t sample32;
  size_t bytesRead;
  while (recording && recordIndex < RECORD_BUFFER_SIZE) {
    i2s_read(I2S_PORT, &sample32, sizeof(sample32), &bytesRead, portMAX_DELAY);
    if (bytesRead > 0) recordedData[recordIndex++] = sample32 >> 14;
  }
  i2s_driver_uninstall(I2S_PORT);
  beep();
}