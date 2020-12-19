#include <fftw3.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>


#include <string.h>

#include "defines.h"
#include "audio.h"
#include "pulse.h"

#define MAVG_FILT_SIZE  4

#ifndef M_PI
#define M_PI 3.1415926535897932385
#endif

sem_t sem_audio;

void init_audio(audio_t *audio)
{
    audio->buf = fftwf_alloc_real(FFT_TRANF_SIZE);
    audio->fft_in = fftwf_alloc_real(FFT_TRANF_SIZE);
    audio->fft_out = fftwf_alloc_complex((FFT_TRANF_SIZE/2)+1);   
    
    audio->window = fftwf_alloc_real(FFT_TRANF_SIZE);
    //audio->plan_fft = fftwf_plan_dft_r2c_1d(FFT_TRANF_SIZE, audio->fft_in, audio->fft_out, FFTW_MEASURE);
    audio->plan_fft = fftwf_plan_dft_r2c_1d(FFT_TRANF_SIZE, audio->fft_in, audio->fft_out, FFTW_ESTIMATE);
    audio->spectrum = fftwf_alloc_real(FFT_TRANF_SIZE);

    audio->format = 16;
    audio->source = NULL;
    
    uint16_t i;
    /* Initially, fill the the buffer with zeros */
    for (i = 0; i < FFT_TRANF_SIZE; i++)
        audio->fft_in[i] = 0.0f;
    /* Initialize window */
    for (int i = 0; i < FFT_TRANF_SIZE; i++) 
        audio->window[i] = 0.5 * (1 - cos(2 * M_PI * i / (FFT_TRANF_SIZE - 1)));

    /* Shared between processes (non zero), init with value zero */
    sem_init(&sem_audio, 0, 0);
}

void free_audio(audio_t *audio)
{
    fftwf_free(audio->buf);
    fftwf_free(audio->fft_in);
    fftwf_free(audio->fft_out);
    
    fftwf_free(audio->window);
    fftwf_free(audio->spectrum);

    fftwf_destroy_plan(audio->plan_fft);
    free(audio->source);

    sem_destroy(&sem_audio);
}

bool find_beats(audio_t *audio)
{
    static float running_sum;
    static int hold_count = 0;
    static float th_max = 0.0f;
    //TODO: Init with zeros
    static float mva_filt[MAVG_FILT_SIZE] = {0.f, 0.0f, 0.0f, 0.0f};
    static float th_scaling = 0.998;

    static uint32_t lo_cut_off = 2;
    static uint32_t filt_indx = 0;
    static bool onset = false;
    static bool prev_onset = false;

    uint16_t i; // General purpose variable
    static uint16_t hold_th = (48000 * 60) / (2*MAX_BPM * CHUNK);
    float f; 
    // TODO: Windowing 
    pthread_mutex_lock(&lock);
    for (i = 0; i < FFT_TRANF_SIZE; i++)
        audio->fft_in[i] = audio->buf[i] * audio->window[i];
        
    fftwf_execute(audio->plan_fft);
    pthread_mutex_unlock(&lock);
    float * spectrum = audio->spectrum;
    /* Find magnitude of the specturm 
        TODO: Limit the calculations only for interested bands    */
    
    for (i = 0; i <= 10; i++)
    {
        *spectrum++ = sqrtf((audio->fft_out[i][0] * audio->fft_out[i][0]) + (audio->fft_out[i][1] * audio->fft_out[i][1]));
        //printf("x[%d] = %f, X[%d] = %f\r\n", i, audio->fft_in[i], i, audio->spectrum[i]);
    }
        
    /*
        Find the magnitude of the spectrum of interest.
        Approximately 0 to 50Hz, excluded the d.c.
    */
    f = 0.0f;
    for (i = 1; i <= lo_cut_off; i ++)
        f += audio->spectrum[i];
    /* Moving average filter over the freq. magnitude */
    if(++filt_indx >= MAVG_FILT_SIZE)
        filt_indx = 0;

    running_sum -= mva_filt[filt_indx];
    running_sum += f;
    mva_filt[filt_indx] = f;

    f = running_sum / MAVG_FILT_SIZE;

    /* Find the max and set as a threshold */
    if(f > th_max)
        th_max = f;

    audio->bar = f / th_max;

    th_max *= th_scaling;   // Decrement threshold

    if(++hold_count > hold_th)
    {
        if(f > (th_max * 0.4))
        {
            if (onset == false)
            {
                onset = true;
                hold_count = 0;
            }
        }
        if(f < (th_max * 0.4))
        {
            if(onset == true)
            {
                onset = false;
                hold_count = 0;
            }
        }
    }
    if(prev_onset == false)
    {
        prev_onset = onset;
        return onset;
    }
    prev_onset = onset;
    return false;
}

void write_to_audio_buffers(int16_t *buf, audio_t * audio, uint16_t frames)
{
    uint16_t i;
    float * ptr;
    /* 
        Move previous set of data from the last section of the buffer to
        initial portion of the buffer. Here assume hop size is same as the
        CHUNK size and take FFT of two times of CHUNK size
    */
    // dest <- source
    #if 0
    memcpy((void *) audio->fft_in, (void*) (&audio->fft_in[CHUNK]), sizeof(float)*CHUNK);
    // Fill the new data in the second half of the buffer
    ptr = &audio->fft_in[CHUNK];
    //
    if(frames != CHUNK)
        printf("Fatal Error: No read size other than %d currently spported\r\n", CHUNK);
    
    for (i = 0; i < frames; i++)
    {
        *ptr++ = (buf[2*i] + buf[2*i+1]) / 65536.0f;
    }
    #else
    memcpy((void *) audio->buf, (void*) (&audio->buf[CHUNK]), sizeof(float)*CHUNK);
    // Fill the new data in the second half of the buffer
    ptr = &audio->buf[CHUNK];
    //
    if(frames != CHUNK)
        printf("Fatal Error: No read size other than %d currently spported\r\n", CHUNK);
    
    for (i = 0; i < frames; i++)
    {
        *ptr++ = (buf[2*i] + buf[2*i+1]) / 65536.0f;
    }
    #endif
    sem_post(&sem_audio);   // Signal to 
}