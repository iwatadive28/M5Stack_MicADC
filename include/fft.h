#ifndef FFT_H
#define FFT_H

void sampleADC();
void removeDC(double *data, int len); 
void applyFFT();
void drawFFT();
void drawWaveform();

#endif