/*
Author: ALEXIS BOYER
Date: 10/06/2026
*/
#include "mbed.h"
#include "USBSerial.h"

#define P0_OUTSET ((uint32_t*) 0x50000508)
#define P0_OUTCLR ((uint32_t*) 0x5000050C)
#define P0_DIR    ((uint32_t*) 0x50000514)

#define GREEN_LED (1UL << 16)
#define RED_LED (1UL << 24)

#define HUMIDITY 0x1
#define TEMPERATURE 0x2

USBSerial Serial;

EventFlags Humidity_event_flag;
EventFlags Temperature_event_flag;

Thread read_temperature_thread;
Thread read_humidity_thread;

Ticker sensor_ticker;

Mutex mutex;

void ISR_sensor(){
    static bool humidity_status = true;

    if(humidity_status){
        Humidity_event_flag.set(HUMIDITY);
    }
    else{
        Temperature_event_flag.set(TEMPERATURE);
    }

    humidity_status = !humidity_status;
}

void read_temperature(){
    while(true){
        Temperature_event_flag.wait_any(TEMPERATURE);

        mutex.lock();
        Serial.printf("Reading Temperature\r\n");
        mutex.unlock();

        *P0_OUTCLR = RED_LED;

        ThisThread::sleep_for(500ms);

        *P0_OUTSET = RED_LED;

    }
}

void read_humidity(){
    while(true){
        Humidity_event_flag.wait_any(HUMIDITY);

        mutex.lock();
        Serial.printf("Reading Humidity\r\n");
        mutex.unlock();

        *P0_OUTCLR = GREEN_LED;

        ThisThread::sleep_for(500ms);

        *P0_OUTSET = GREEN_LED;
    }
}

//

// main() runs in its own thread in the OS
int main()
{
    *P0_DIR |= GREEN_LED | RED_LED;

    *P0_OUTSET = RED_LED | GREEN_LED;

    read_temperature_thread.start(read_temperature);
    read_humidity_thread.start(read_humidity);

    sensor_ticker.attach(&ISR_sensor, 1s);

    while (true) {
        ThisThread::sleep_for(1s);
    }
}

