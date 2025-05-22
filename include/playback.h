#ifndef PLAYBACK_H
#define PLAYBACK_H

#define I2S_PORT I2S_NUM_0
#define I2S_BCK 12
#define I2S_WS 0
#define I2S_DO 2

void InitI2SSpeaker();
void beep();
void playAudio();
bool isPlaying();
void startPlayback();
void stopPlayback();

#endif