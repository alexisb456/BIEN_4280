#include "mbed.h"
#include "USBSerial.h"

//register addresses
#define P0_OUTSET  ((uint32_t*) 0x50000508)
#define P0_OUTCLR  ((uint32_t*) 0x5000050C)
#define P0_DIR     ((uint32_t*) 0x50000514)

// LED pins
#define BUILT_IN_LED (1UL << 13)

USBSerial serial;

int timer; 

Ticker foo; 
Thread ticker_thread;

void foo{
    timer = timer + 1; 
}

void ticker{
    while(true){
        if (timer >= 3){
             *P0_OUTSET |= BUILT_IN_LED;
             thread_sleep_for(500);

             *P0_OUTCLR |= BUILT_IN_LED;
             thread_sleep_for(500);
        }
    }
}

// main() runs in its own thread in the OS
int main()
{
    while (true) {

    }
}

