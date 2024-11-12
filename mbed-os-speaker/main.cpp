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
static void on_init_rise();
static void on_rec_rise();
static void on_release();

// DAC Setup :
#define BLINKING_RATE     100ms
#define OSC_Frequency     800 // Hz
#define SAMPLING_Frequency 5000 // Hz

AnalogOut out(PA_5);
InterruptIn command(PC_13);
Ticker tic;
EventQueue queue(32*4);
Thread eventThread(osPriorityAboveNormal,65536);
us_timestamp_t SAMPLING_Period = 1000000/SAMPLING_Frequency;

int ind=0;
int loop_len = 7000;
static float values[7000]; // use a global array to increase available memory
float * start = values;
int button = 1;
bool DAC_status = false;

bool flag_init = false;
bool flag_rec = false;
// End of DAC Setup

int main()
{
    bool prev_flag_init = flag_init;
    bool prev_flag_rec = flag_rec;
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

    command.fall(on_init_rise);
    command.rise(on_release);

    while(true){
        button = command.read(); // Fetch the command signal
        if(flag_init != prev_flag_init){
            printf("Detaching init routine ...\n");
            prev_flag_init = flag_init;
            tic.detach();
            if(flag_init){
                printf("Attaching init routine ...\n");
                tic.attach_us(tickerCallback,SAMPLING_Period);
            }
        } else if (flag_rec != prev_flag_rec) {
            printf("Detaching rec routine ...\n");
            prev_flag_rec = flag_rec;
            tic.detach();
            if(flag_rec){
                printf("Attaching rec routine ...\n");
                tic.attach_us(tickerCallback,SAMPLING_Period);
            }
        }
    }

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

static void on_init_rise() {
    flag_init = true;
    flag_rec = false;
}

static void on_rec_rise() {
    flag_init = false;
    flag_rec = true;
}

static void on_release() {
    flag_init = false;
    flag_rec = false;
}