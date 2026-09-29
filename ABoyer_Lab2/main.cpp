#include "mbed.h"

#define P0_OUTSET ((uint32_t*) 0x50000508)
#define P0_OUTCLR ((uint32_t*) 0x5000050C)
#define P0_DIR    ((uint32_t*) 0x50000514)

#define GREEN_LED (1UL << 16)
//create the queue to represent dutycycle
typedef struct {
    float dutyCycle;
} PWM;

//queue holding up to 9 pointers
Queue<PWM, 9> pwm_queue;

//create memory pool that provide memory for queue items
MemoryPool<PWM, 9> pwm_mem;

//producer thread 
Thread producer_thread;
Thread vanilla_thread; 
Thread chocolate_thread; 

void producer(){

    //duty cycyles 
    int dutyCycle[9] = {
        33, 75, 50,
        33, 75, 50,
        33, 75, 50
    };

    for(int i = 0; i<9; i++){
        
        //allocate memory for a PWM 
        PWM *message = pwm_mem.try_alloc();

        if(message != nullptr){

            //store duty cycle
            message->dutyCycle = dutyCycle[i];

            //push message into Queue
            pwm_queue.try_put(message);
        }
    }
}

void vanilla(){
    PWM *message;

    while(true) {

        //wait for duty cycle
        pwm_queue.try_get(&message);
        
        int dutyCycle = message->dutyCycle;

        //return memory
        pwm_mem.free(message);

        //10ms PWM period (100Hz)
        int period_us = 10000;

        int on_time = (period_us * dutyCycle) / 100;
        int off_time = period_us - on_time;

        //produce PWM until another value
        while(true) {

            //Green LED on
            *P0_OUTCLR = GREEN_LED;
            wait_us(on_time);

            //Green LED off
            *P0_OUTSET = GREEN_LED;
            wait_us(off_time);
        }
    }
}

void chocolate(){

    //Mbed PWM output for blue
    PwmOut blue(p6);

    PWM *message;

    while (true) {
        
        //get duty cycle from queue
        if (pwm_queue.try_get(&message)){

            float dutyCycle = message->dutyCycle;

            //return memory
            pwm_mem.free(message);

            blue.period_us(10000);

            blue.write(1.0 - (dutyCycle / 100.0));
        }
    }
}

/*
Main for Part 2a
// main() runs in its own thread in the OS
int main()
{
    green LED as output
    *P0_DIR |= GREEN_LED;

    green LED starts off
    *P0_OUTSET = GREEN_LED;

    producer_thread.start(producer);
    vanilla_thread.start(vanilla);
    while (true) {
        ThisThread::sleep_for(1s);
    }
}
*/

//Main for Part 2b
int main()
{
    // FOR TESTING PART 2b ONLY:

    producer_thread.start(producer);
    chocolate_thread.start(chocolate);

    while (true) {
        ThisThread::sleep_for(1s);
    }
}
