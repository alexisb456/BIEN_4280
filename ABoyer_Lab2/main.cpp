#include "mbed.h"

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




// main() runs in its own thread in the OS
int main()
{
    while (true) {

    }
}

