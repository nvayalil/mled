// header file for pulse, part of cava.

#ifndef _PULSE_H_
#define _PULSE_H_

#include <pthread.h>


void *input_pulse(void *data);
void getPulseDefaultSink();

extern pthread_mutex_t lock;


#endif
