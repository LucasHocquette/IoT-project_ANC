/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "main.h"
#include <cstdint>
#include <cstdlib>
#include <functional>

const double pi = 3.14159265358979323846;

float Signal_Generator(float t, char sig);
static void run_Loop(int ind, float* start);
static void setup_DAC(void);

// DAC Setup :
#define BLINKING_RATE     100ms
#define OSC_Frequency     800 // Hz
#define SAMPLING_Frequency 5000 // Hz

AnalogOut out(PA_5);
Ticker tic;
EventQueue queue(32*4);
Thread eventThread(osPriorityAboveNormal,65536);
us_timestamp_t SAMPLING_Period = 1000000/SAMPLING_Frequency;

int ind=0;
int loop_len = 7000;
static float values[7000]; // use a global array to increase available memory
float * start = values;
// End of DAC Setup

int main()
{
    // Fill in a LUT with the signal to loop on
    for(int i=0; i<loop_len; i++)
    {
        values[i] = Signal_Generator(float(i)/SAMPLING_Frequency, ' ');
    }

    printf("Starting blinking program !\n");

    // Start the EventQueue
    eventThread.start(callback(&queue, &EventQueue::dispatch_forever));

    // Attach the loop routine to the ticker
    auto attRunLoop = mbed::callback([&](){ind=(ind+1)%loop_len; run_Loop(ind,start);});
    auto tickerCallback = [&](){queue.call(attRunLoop);}; // use a queue to avoid Mutex errors
    tic.attach_us(tickerCallback,SAMPLING_Period);

    printf("Callback set\n");
}

float Signal_Generator(float t, char sig){
    //return 0.2f*sin(OSC_Frequency*2*pi*t)+0.22f; // Generate a test sinusoid
    return (float)rand()/RAND_MAX; // generate a random white noise
    //return 0.2f*(t*OSC_Frequency - floor(t*OSC_Frequency))+0.22f; // generate a sawtooth signal
}

static void run_Loop(int ind, float* start){
    out = *(start+ind);
}