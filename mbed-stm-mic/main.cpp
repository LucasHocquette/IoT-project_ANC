#include "ThisThread.h"
#include "mbed.h"
#include "cloud.h"

/*
    CONSTANTS
*/
#define BUFFER_SIZE 15000
#define BURST_TIME_MS 2000

/*
    DECLARATION
*/
Timer burst_timer;
Timer sample_timer;
AnalogIn mic_adc(A0);
DigitalIn start_button(PC_13);
DigitalOut synchro_init(D0);
DigitalOut synchro_record(D1);
DigitalOut init_led(LED1);
DigitalOut record_led(LED2);

/*
    PROGRAM
*/
uint16_t sample_buffer[BUFFER_SIZE];
uint16_t sample_size = 0;
us_timestamp_t adc_burst(void) {
    sample_size = 0;
    burst_timer.reset();
    burst_timer.start();
    while (burst_timer.read_ms() < BURST_TIME_MS) {
        sample_timer.reset();
        sample_timer.start();
        if (sample_size >= BUFFER_SIZE) {
            printf("Buffer OVERFLOW !!\r\n");
            break;
        }

        sample_buffer[sample_size] = mic_adc.read_u16();
        sample_size++;
        while (sample_timer.read_us() <= 300);
        sample_timer.stop();
    }
    burst_timer.stop();
    return burst_timer.read_high_resolution_us();
}

/*
    MAIN
*/
int main()
{
    printf("\r\nBoard start up!\r\n");
    init_led.write(0);
    record_led.write(0);
    synchro_init.write(0);

    if (!cloud_init()){
        printf("Cloud init err!\r\n");
        return 1;
    }

    while (start_button.read() == 1);
    printf("Start Init!\r\n");
    init_led.write(1);
    synchro_init.write(1);
    us_timestamp_t time_length = adc_burst();
    printf("Burst completed: %llu us, %d!\r\n", time_length, sample_size);
    if (cloud_send(sample_buffer, 50) <= 0){
        printf("Cloud init err!\r\n");
        return 1;
    }
    synchro_init.write(0);
    init_led.write(0);

    while (start_button.read() == 1);
    printf("Start Loop!\r\n");
    record_led.write(1);
    synchro_record.write(1);
    while (true) {
        us_timestamp_t time_length = adc_burst();
        printf("Burst completed: %llu us, %d!\r\n", time_length, sample_size);
        
        if (cloud_send(sample_buffer, 50) <= 0){
            printf("Cloud init err!\r\n");
            return 1;
        }

        ThisThread::sleep_for(15000);
    }
}
