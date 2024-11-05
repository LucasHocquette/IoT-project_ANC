#include "mbed.h"
#include "include/mic_sampling.h"

#define BUFFER_SIZE 1024

Thread t;
AnalogIn mic_adc(A0);

uint32_t sample_buffer[BUFFER_SIZE*2];
uint32_t freq_counter = 0;
void adc_sample(void) {
    uint32_t buffer_counter = 0;

    printf("ADC loop!\r\n");
    while (true) {
        if (buffer_counter >= BUFFER_SIZE*2) {
            buffer_counter = 0;
        }

        sample_buffer[buffer_counter] = mic_adc.read_u16();
        buffer_counter++;
        freq_counter++;
    }
}

// main() runs in its own thread in the OS
int main()
{    
    t.start(callback(adc_sample));
    printf("Init!\r\n");
    
    uint32_t last_counter = 0;
    while (true) {
        printf("counter: %d\r\n", freq_counter-last_counter);
        last_counter = freq_counter;
        ThisThread::sleep_for(1s);
    }
}

