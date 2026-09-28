#include "mbed.h"
#include "USBSerial.h"

//register addresses
#define P0_OUTSET  ((uint32_t*) 0x50000508)
#define P0_OUTCLR  ((uint32_t*) 0x5000050C)
#define P0_DIR     ((uint32_t*) 0x50000514)

// LED pins
#define RED_LED (1UL << 24)

USBSerial serial;

volatile int timer = 0;


Ticker foo; 
Thread ticker_thread;

void foo_function(){
    timer++; 
}

void ticker(){
    while(true){
        if (timer >= 3){
             *P0_OUTCLR = RED_LED;
             thread_sleep_for(500);

             *P0_OUTSET = RED_LED;
             thread_sleep_for(500);

             //reset timer
             timer = 0;
        }

        thread_sleep_for(1);
    }
}

// main() runs in its own thread in the OS
int main()
{
    *P0_DIR |= RED_LED;

    *P0_OUTSET = RED_LED;

    foo.attach(&foo_function, 1s);
    //start
    ticker_thread.start(ticker);


    while (true) {
        thread_sleep_for(10000);
    }
}
