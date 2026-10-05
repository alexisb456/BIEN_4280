#include "mbed.h"
#include "nrf_pwm.h"

//GPIO registers
#define P0_OUTSET ((uint32_t*) 0x50000508)
#define P0_OUTCLR ((uint32_t*) 0x5000050C)
#define P0_DIR    ((uint32_t*) 0x50000514)

//Bitmeask for LED pins 
#define GREEN_LED (1UL << 16)
#define RED_LED (1UL << 24)

//structure used to pass duty cycle between threads
typedef struct {
    float dutyCycle; //stores desired duty cycles 
} PWM;

/*
//Queue and mempool for all Part 2
//queue holding up to 9 pointers
Queue<PWM, 9> pwm_queue;

//create memory pool that provide memory for queue items
MemoryPool<PWM, 9> pwm_mem;
*/


//seperate queues for part 3 
Queue<PWM, 9>vanilla_queue;
Queue<PWM, 9>chocolate_queue;
Queue<PWM, 9>strawberry_queue; 

//seperate memory pools for part 3
MemoryPool<PWM, 9>vanilla_mem;
MemoryPool<PWM, 9>chocolate_mem;
MemoryPool<PWM, 9>strawberry_mem;


//Define ticker 
Ticker vanilla_ticker;

//threads created
Thread producer_thread;
Thread vanilla_thread; 
Thread chocolate_thread; 
Thread strawberry_thread;

//define global variables
volatile int pwmCounter = 0;

volatile int vanillaDutycycle = 33;

volatile int vanillaDutycycle_part3 = 10;


/*
Producer for part 2
void producer(){

    //array of duty cycles for Vanilla, Chocolate, and Strawberry
    int dutyCycle[9] = {
        33, 75, 50,
        33, 75, 50,
        33, 75, 50
    };

    //Go through each duty cycle in the array
    for(int i = 0; i<9; i++){
        
        //allocate memory for a PWM 
        PWM *message = pwm_mem.try_alloc();

        //if memory was correctly allocated continue
        if(message != nullptr){

            //store duty cycle
            message->dutyCycle = dutyCycle[i];

            //push message into Queue
            pwm_queue.try_put(message);
        }
    }
}
*/

/*
//producer for Part 2a
void producer(){

    //Allocate memory for one message
    PWM *message = pwm_mem.try_alloc();

    //ensure successful allocation 
    if(message != nullptr){

        //vanilla at 1/3rd brightness 
        //to test can also different brightness at 10 and 90
        message->dutyCycle = 90;

        //place message into the queue
        if(!pwm_queue.try_put(message)){
            pwm_mem.free(message);
        }
    }
}
*/

/*
//Producer for Part 2b
void producer() {

    //allcoate memory for one message
    PWM *message = pwm_mem.try_alloc();

    //ensure correct allocation 
    if (message != nullptr) {

        //chocalate at 75% brightness
        message->dutyCycle = 75;

        //place message in queue
        if (!pwm_queue.try_put(message)) {
            pwm_mem.free(message);
        }
    }
}
*/

/*
//Producer for part 2c
void producer() {

    //allocate memory 
    PWM *message = pwm_mem.try_alloc();

    //ensure correct allocaiton 
    if (message != nullptr) {

        //strawberry at 50%
        message->dutyCycle = 50;

        if (!pwm_queue.try_put(message)) {
            pwm_mem.free(message);
        }
    }
}
*/


//Producer for part 3
void producer() {

    //starting brightness of each LED (all at 10%)
    int greenDuty = 10;
    int blueDuty = 10;
    int redDuty = 10;

    //The amount each duty cycle changes at producer updates
    //different values for different change rates 
    int greenDirection = 5;
    int blueDirection = 3;
    int redDirection = 2;

    
    while(true){

        //Vanilla
        //allocate memory for vanilla duty cycle
        PWM *greenMessage = vanilla_mem.try_alloc();

        //ensure correct allocation 
        if(greenMessage != nullptr){

            //store green brightness current level
            greenMessage->dutyCycle = greenDuty;

            //send message to vanillia thread
            if(!vanilla_queue.try_put(greenMessage))
                //free if queue if full
                vanilla_mem.free(greenMessage);
        }


    //Chocolate
    //same process as vanilla but using chocolate

    PWM *blueMessage = chocolate_mem.try_alloc();

    if(blueMessage != nullptr){
        blueMessage->dutyCycle = blueDuty;

        if(!chocolate_queue.try_put(blueMessage)){
            chocolate_mem.free(blueMessage);
        }
    }

    //Strawberry
    //same process as vanilla but using strawberry

    PWM *redMessage = strawberry_mem.try_alloc();

    if(redMessage != nullptr){
        redMessage->dutyCycle = redDuty;

        if(!strawberry_queue.try_put(redMessage)){
            strawberry_mem.free(redMessage);
        }
    }

    //Increase or decrease LED's current brightness by the designated amount
    greenDuty += greenDirection;
    blueDuty += blueDirection;
    redDuty += redDirection;

    //Reverse direction at limits
    //if LEDs reach either brightness limit, reverse the direction
    //positive becomes negative, negative becomes positive
    if(greenDuty >= 90 || greenDuty <= 10){
        greenDirection = -greenDirection;
    }

    if(blueDuty >= 90 || blueDuty <= 10){
        blueDirection = -blueDirection;
    }

    if(redDuty >= 90 || redDuty <= 10){
        redDirection = -redDirection;
    }

    //wait before changing brightness
    ThisThread::sleep_for(50ms);

    }
}


/*
//ticker for part 2a
void vanilla_tick(){

 if(pwmCounter < vanillaDutycycle){
     //turn green on 
     *P0_OUTCLR = GREEN_LED;
 }
 else {
     //Green LED off
     *P0_OUTSET = GREEN_LED;
 }

 pwmCounter++;

 if(pwmCounter >= 100){
     pwmCounter = 0;
 }
}
*/

//ticker for part 3
void vanilla_tick(){

    if(pwmCounter < vanillaDutycycle_part3){
        //turn green on 
         *P0_OUTCLR = GREEN_LED;
    }
    else {
      //Green LED off
         *P0_OUTSET = GREEN_LED;
     }

    pwmCounter++;

    if(pwmCounter >= 100){
         pwmCounter = 0;
    }
}

/*
void vanilla(){

    PWM *message;

    vanilla_ticker.attach(&vanilla_tick, 100us);

    while(true){
        
        //check queue
        if(pwm_queue.try_get(&message)){

            //save requested duty cycle
            vanillaDutycycle = message->dutyCycle;

            //return message memory to Memory pool
            pwm_mem.free(message);
        }

        ThisThread::sleep_for(1ms);
    }
}

void chocolate(){

    //Mbed PWM output for blue
    PwmOut blue(p6);

    //create pointer to receive messages
    PWM *message;

    //keep consumer running 
    while (true) {
        
        //get duty cycle from queue
        if (pwm_queue.try_get(&message)){

            //get duty cycle percentage 
            float dutyCycle = message->dutyCycle;

            //return memory
            pwm_mem.free(message);

            //set PWM period
            blue.period_us(10000);

            //convert percentage 
            blue.write(1.0 - (dutyCycle / 100.0));
        }
    }
}

void strawberry() {

    //pointer to recieve messages
    PWM *message; 

    // pointer to PWM1 hardware
    NRF_PWM_Type *pwm = NRF_PWM1;

    // Assign channel 0 to P24(the red light)
    uint32_t pwm_pins[4] = {
        24,
        NRF_PWM_PIN_NOT_CONNECTED,
        NRF_PWM_PIN_NOT_CONNECTED,
        NRF_PWM_PIN_NOT_CONNECTED
    };

    //PWM peripheral pins to output pins
    nrf_pwm_pins_set(pwm, pwm_pins);

    // 1 MHz clock, 10 ms period
    //the timing 
    nrf_pwm_configure(
        pwm,
        NRF_PWM_CLK_1MHz,
        NRF_PWM_MODE_UP,
        10000
    );

    //Interpret sequence data 
    nrf_pwm_decoder_set(
        pwm,
        NRF_PWM_LOAD_COMMON,
        NRF_PWM_STEP_AUTO
    );

    //turn on PWM1
    nrf_pwm_enable(pwm);

    // keep variable in memory 
    static uint16_t pwmValue;

    // structure to describe PWM sequence
    nrf_pwm_sequence_t sequence;

    //Point sequence at PWM value
    sequence.values.p_common = &pwmValue;

    //one PWM value in sequence, do not repeat within sequence, do not add a delay
    sequence.length = 1;
    sequence.repeats = 0;
    sequence.end_delay = 0;

    //load into sequence slot 0
    nrf_pwm_sequence_set(
        pwm,
        0,
        &sequence
    );

    // Automatically restart sequence
    nrf_pwm_loop_set(pwm, 1);

    //restart with loop ends
    nrf_pwm_shorts_set(
        pwm,
        NRF_PWM_SHORT_LOOPSDONE_SEQSTART0_MASK
    );

    //check for new duty cycles 
    while (true) {

        //check strawberry message availability
        if (pwm_queue.try_get(&message)) {

            //get dutycycle from message
            float dutyCycle = message->dutyCycle;
            pwm_mem.free(message);

            // Convert percentage to PWM value
            pwmValue =
                (uint16_t)((dutyCycle / 100.0) * 10000);

            // PWM polarity bit for the low-active LED
            pwmValue |= 0x8000;

            // Start PWM
            nrf_pwm_task_trigger(
                pwm,
                NRF_PWM_TASK_SEQSTART0
            );
        }
    }
}
*/


void vanilla_part_3(){
    PWM *message;

    //start ticker 
    vanilla_ticker.attach(&vanilla_tick, 100us);

   while(true){

       //check for updated duty cycle
       if(vanilla_queue.try_get(&message)){

           //save new green duty cycle
           vanillaDutycycle_part3 = message->dutyCycle;

           //return memory to pool 
           vanilla_mem.free(message);
       }

       //allow other threads to run 
       ThisThread::sleep_for(1ms);
   }
}

void chocolate_part_3(){

    //Mbed PWM output for blue
    PwmOut blue(p6);

    //create pointer to receive messages
    PWM *message;

    //set PWM period
     blue.period_us(10000);

    //keep consumer running 
    while (true) {
        
        //get duty cycle from queue
        if (chocolate_queue.try_get(&message)){

            //get duty cycle percentage 
            float dutyCycle = message->dutyCycle;

            //return memory
            chocolate_mem.free(message);


            //convert percentage 
            blue.write(1.0 - (dutyCycle / 100.0));
        }
    }
}

void strawberry_part_3() {

    //pointer to recieve messages
    PWM *message; 

    // pointer to PWM1 hardware
    NRF_PWM_Type *pwm = NRF_PWM1;

    // Assign channel 0 to P24(the red light)
    uint32_t pwm_pins[4] = {
        24,
        NRF_PWM_PIN_NOT_CONNECTED,
        NRF_PWM_PIN_NOT_CONNECTED,
        NRF_PWM_PIN_NOT_CONNECTED
    };

    //PWM peripheral pins to output pins
    nrf_pwm_pins_set(pwm, pwm_pins);

    // 1 MHz clock, 10 ms period
    //the timing 
    nrf_pwm_configure(
        pwm,
        NRF_PWM_CLK_1MHz,
        NRF_PWM_MODE_UP,
        10000
    );

    //Interpret sequence data 
    nrf_pwm_decoder_set(
        pwm,
        NRF_PWM_LOAD_COMMON,
        NRF_PWM_STEP_AUTO
    );

    //turn on PWM1
    nrf_pwm_enable(pwm);

    // keep variable in memory 
    static uint16_t pwmValue;

    // structure to describe PWM sequence
    nrf_pwm_sequence_t sequence;

    //Point sequence at PWM value
    sequence.values.p_common = &pwmValue;

    //one PWM value in sequence, do not repeat within sequence, do not add a delay
    sequence.length = 1;
    sequence.repeats = 0;
    sequence.end_delay = 0;

    //load into sequence slot 0
    nrf_pwm_sequence_set(
        pwm,
        0,
        &sequence
    );

    // Automatically restart sequence
    nrf_pwm_loop_set(pwm, 1);

    //restart with loop ends
    nrf_pwm_shorts_set(
        pwm,
        NRF_PWM_SHORT_LOOPSDONE_SEQSTART0_MASK
    );

    //check for new duty cycles 
    while (true) {

        //check strawberry message availability
        if (strawberry_queue.try_get(&message)) {

            //get dutycycle from message
            float dutyCycle = message->dutyCycle;
            strawberry_mem.free(message);

            // Convert percentage to PWM value
            pwmValue =
                (uint16_t)((dutyCycle / 100.0f) * 10000);

            // PWM polarity bit for the low-active LED
            pwmValue |= 0x8000;

            // Start PWM
            nrf_pwm_task_trigger(
                pwm,
                NRF_PWM_TASK_SEQSTART0
            );
        }

        ThisThread::sleep_for(1ms);
    }
}


/*
//Main for Part 2a
// main() runs in its own thread in the OS
int main()
{
    //green LED as output
    *P0_DIR |= GREEN_LED;

    //green LED starts off
    *P0_OUTSET = GREEN_LED;

    producer_thread.start(producer);
    vanilla_thread.start(vanilla);
    while (true) {
        ThisThread::sleep_for(1s);
    }
}
*/

/*
//Main for Part 2b
int main()
{

    producer_thread.start(producer);
    chocolate_thread.start(chocolate);

    while (true) {
        ThisThread::sleep_for(1s);
    }
}
*/

/*
//Main for part 2c
int main()
{
    producer_thread.start(producer);
    strawberry_thread.start(strawberry);

    while (true) {
        ThisThread::sleep_for(1s);
    }
}
*/


//Main for Part 3
int main(){
    //Green LED 
    *P0_DIR |= GREEN_LED;
    *P0_OUTSET = GREEN_LED;

    vanilla_thread.start(vanilla_part_3);
    chocolate_thread.start(chocolate_part_3);
    strawberry_thread.start(strawberry_part_3);

    producer_thread.start(producer);

    while(true){
        ThisThread::sleep_for(1s);
    }
}
