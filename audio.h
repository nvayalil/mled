#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <stdbool.h>
#include <fftw3.h>
#include <stdint.h>
#include <semaphore.h>

typedef struct
{
    char * source; // alsa device, fifo path or pulse source
    /* Use single precision real FFT tramsform */
    fftwf_plan plan_fft;
    float * buf;
    float * fft_in;
    float * window;
    fftwf_complex * fft_out;
    float * spectrum;
    float bar;
    int format;
    bool terminate;
} audio_t;

extern sem_t sem_audio;

void init_audio(audio_t *audio);
void free_audio(audio_t *audio);
bool find_beats(audio_t *audio);
void write_to_audio_buffers(int16_t *buf, audio_t * audio, uint16_t frames);
#endif