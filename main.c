#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <assert.h>
#include <stdbool.h>
#include <signal.h>
#include <semaphore.h>

#include <math.h>
#include <fftw3.h>

#include "audio.h"
#include "lights.h"
#include "input/pulse.h"

#include "wave.h"
#include "defines.h"

void built_in_test(audio_t * audio)
{
    struct wav_info w_info;

    FILE* fp;

    /* Load a test file */
    fp = fopen("/home/pi/Music/MazhaSong40s.wav", "rb");
    if(fp == NULL)
    {
        printf("Unable to open wav file \r\n");
        return;
    }

    read_wav_info(&w_info, fp);

    print_wav_info(&w_info);

    int16_t buf[CHUNK*2];
    uint32_t i;
    uint32_t frame = 0;
    printf("No. of samples per channel %d \r\n",  w_info.num_samples);
    for (i = 0; i < w_info.num_samples; i+=CHUNK)
    {
        /* Read streo file, size of CHUNK * 2 */
        fread(buf, sizeof(uint16_t), CHUNK*2, fp);
        write_to_audio_buffers(buf, audio, CHUNK);
        if(find_beats(audio))
        {
            printf("Beat find at %d\r\n", frame);
        }
        frame++;
    }
    fclose(fp);
    
}

volatile bool running = true;
static void ctrl_c_handler(int signum)
{
	(void)(signum);
    running = false;
}

static void setup_handlers(void)
{
    struct sigaction sa = 
    {
        .sa_handler = ctrl_c_handler
    };
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

#define GRAVITY 1.414

void * process_audio(void * data)
{   
    int16_t color = 0;

    float count = 100.0f;
    audio_t * audio = (audio_t *) data;
    float height = (float) LED_COUNT;
    float diff;
    uint8_t mode = 0;

    // process [smoothing]: calculate gravity
    float g = GRAVITY * 1 / (count * count);

    while(running)
    {
        sem_wait(&sem_audio);
        if(find_beats(audio))
        {
            color = 255;
            height = (float) LED_COUNT;
            count = 100;
        }
        switch (mode)
        {
        case 0:
            diff = (g * count * count);
            if(diff < 1)
                diff = 1;
            height -= diff;

            if(height < 0)
                height = 0;
            count -= 1.0f;
            if(count < 0)
                count = 0;
            fill_leds((uint16_t) height, get_color(color));
            break;
        
        case 1:
            height = audio->bar * LED_COUNT;
            fill_leds((uint16_t) height, get_color(color));
            break;
        default:
            break;
        }
        color -= 2;
        if (color < 0)
            color = 0;      
    }
    audio->terminate = true;
    pthread_exit(NULL);
}

int main(void)
{
    pthread_t thr_pulse, thr_audio;
    audio_t audio;
    setup_handlers();
    /* Initialise audio buffers and tools */
    init_audio(&audio);
    //built_in_test(&audio);
    init_lights();
    audio.terminate = false;
    getPulseDefaultSink(&audio);
    printf("Starting threads \r\n");
    /* Start a new thread for listening audio */
    int err_code = pthread_create(&thr_pulse, NULL, input_pulse, (void*) &audio);
    assert(!err_code);
    #if 1
    err_code = pthread_create(&thr_audio, NULL, process_audio, (void*) &audio);
    assert(!err_code);
    pthread_join(thr_audio, NULL);
    #else
    uint16_t count = 0;
    uint32_t frames = 0;
    while(running)
    {
        sem_wait(&sem_audio);
        if(find_beats(&audio))
            printf("beat %d\r\n", count++);
        frames++;
    }
    audio.terminate = true;
    #endif
    pthread_join(thr_pulse, NULL);
    printf("Exiting\r\n");
    free_lights();
    free_audio(&audio);
    return 0;
}