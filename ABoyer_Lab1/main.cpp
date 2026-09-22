/*
Author: Alexis Boyer
Date: 09/08/2026
Code talks to Arduino Nano Sense, telling it to blink
at different speeds based on the severity of an error. 
*/

#include "mbed.h"
#include <Mail.h>
#include <cstdint>

//register addresses
#define P0_OUTSET  ((uint32_t*) 0x50000508)
#define P0_OUTCLR  ((uint32_t*) 0x5000050C)
#define P0_DIR     ((uint32_t*) 0x50000514)

// LED pins
#define BUILT_IN_LED (1UL << 13)
#define RED_LED      (1UL << 24)
#define GREEN_LED (1UL << 16)


    // Diagnostic states
 enum DiagnosticState {
  NO_ERROR,
  ATTN_REQ,
  FATAL_ERROR
 };

typedef struct {
    DiagnosticState state;
}DiagnosticMessage;

Mail<DiagnosticMessage, 4> mail_box; 

//initiate threads 
Thread led_thread;
Thread tester_thread; 




/*
Part 1 code: 

int main(void){
    int builtinLED n= 8192;
    *P0_DIR |= builtinLED;

    while(true){
        *P0_OUT |= builtinLED;
        thread_sleep_for(500);

        *P0_OUTCLR |= builtinLED;
        thread_sleep_for(500);
    }
}

*/

/*
Code for part 2: 
All cosntants are defined above 
// Diagnostic states
enum DiagnosticState {
    NO_ERROR,
    ATTN_REQ,
    FATAL_ERROR
};

int main(void)
{
    // LEDs as outputs
    *P0_DIR |= BUILT_IN_LED;
    *P0_DIR |= RED_LED;

    DiagnosticState state = NO_ERROR;

    while (true)
    {
        int delay;

        // Choose blinking speed based on diagnostic state
        switch (state)
        {
            case NO_ERROR:
                delay = 500;
                break;

            case ATTN_REQ:
                delay = 250;
                break;

            case FATAL_ERROR:
                delay = 100;
                break;
        }

        // Blink for approximately 2 seconds
        int elapsedTime = 0;

        while (elapsedTime < 2000)
        {
            // Built-in LED ON
            *P0_OUTSET |= BUILT_IN_LED;

            // Red LED ON
            *P0_OUTCLR |= RED_LED;

            thread_sleep_for(delay);

            // Built-in LED OFF
            *P0_OUTCLR |= BUILT_IN_LED;

            // Red LED OFF
            *P0_OUTSET |= RED_LED;

            thread_sleep_for(delay);

            elapsedTime += 2 * delay;
        }

        // Move to the next diagnostic state
        if (state == NO_ERROR){
            state = ATTN_REQ;
        }
        else if (state == ATTN_REQ) {
            state = FATAL_ERROR;
        }
        else {
            state = NO_ERROR;
        }
    }
}
*/

/*
//Part 3
// Struct, mail, and threads are all created/declared at the top
void led_diag_handler(){
    DiagnosticState currentState = NO_ERROR;

    while(true){
    //wait for incomoing mail 
    osEvent evt = mail_box.get(0);

    if(evt.status == osEventMail){
        DiagnosticMessage *message = (DiagnosticMessage *)evt.value.p;

        currentState = message->state;

        mail_box.free(message);
    }


    int delay;

    *P0_OUTSET = RED_LED | GREEN_LED;

    switch(currentState){
       
        case NO_ERROR:
            //*P0_OUTSET = RED_LED;
            *P0_OUTCLR = GREEN_LED; 
            delay = 500;
            break;

        case ATTN_REQ:
            *P0_OUTCLR = GREEN_LED | RED_LED;
           // *P0_OUTCLR = RED_LED;
            delay = 250;
            break;

        case FATAL_ERROR:
            *P0_OUTCLR = RED_LED; 
           // *P0_OUTSET = GREEN_LED;
            delay = 100; 
            break;
    }

    thread_sleep_for(delay);  

    //turn leds off
    *P0_OUTSET = RED_LED | GREEN_LED;
   // *P0_OUTSET = GREEN_LED;

    thread_sleep_for(delay);
    }
}

void diag_tester(){

    DiagnosticState state = NO_ERROR;

    while(true){
        DiagnosticMessage *message = mail_box.alloc();

        if(message != NULL){
            message ->state = state;

            mail_box.put(message);
        }

        //wait 5 seconds before changing states
        thread_sleep_for(5000);


  if (state == NO_ERROR){
    state = ATTN_REQ;
  }
  else if (state == ATTN_REQ){
     state = FATAL_ERROR;
  }
  else{
     state = NO_ERROR;
  }
 }
}

int main(void){

    //set outputs
    *P0_DIR |= RED_LED;
    *P0_DIR |= GREEN_LED; 

    //start with led off
    *P0_OUTSET = RED_LED;
    *P0_OUTSET = GREEN_LED;
    
    //start threads
    led_thread.start(led_diag_handler);
    tester_thread.start(diag_tester);

    //yield CPU
    while(true){
        thread_sleep_for(1000);
    }
    
}
*/

//Part 4
void led_diag_handler(){
    DiagnosticState currentState = NO_ERROR;

    while(true){
    //wait for incomoing mail 
    osEvent evt = mail_box.get(0);

    if(evt.status == osEventMail){
        DiagnosticMessage *message = (DiagnosticMessage *)evt.value.p;

        currentState = message->state;

    //Print message whenever state changes
    switch(currentState){
        case NO_ERROR:
            printf("State changed to NO_ERROR. Blink rate: 500ms \r\n");
            break;
        case ATTN_REQ:
            printf("State changed to ATTN_REQ. Blink rate: 250ms \r\n");
            break;
        case FATAL_ERROR:
            printf("State changed to FATAL_ERROR. Blink rate: 100ms \r\n");
            break;
    }
        mail_box.free(message);
    }

    int delay;

    //turn off LEDs 
    *P0_OUTSET = RED_LED | GREEN_LED;

    switch(currentState){
       
        case NO_ERROR:
            //*P0_OUTSET = RED_LED;
            *P0_OUTCLR = GREEN_LED; 
            delay = 500;
            break;

        case ATTN_REQ:
            *P0_OUTCLR = GREEN_LED | RED_LED;
           // *P0_OUTCLR = RED_LED;
            delay = 250;
            break;

        case FATAL_ERROR:
            *P0_OUTCLR = RED_LED; 
           // *P0_OUTSET = GREEN_LED;
            delay = 100; 
            break;
    }

    thread_sleep_for(delay);  

    //turn leds off
    *P0_OUTSET = RED_LED | GREEN_LED;
   // *P0_OUTSET = GREEN_LED;

    thread_sleep_for(delay);
    }
}

void diag_tester(){

    DiagnosticState state = NO_ERROR;

    while(true){
        DiagnosticMessage *message = mail_box.alloc();

        if(message != NULL){
            message ->state = state;

            mail_box.put(message);
        }

        //wait 5 seconds before changing states
        thread_sleep_for(5000);


  if (state == NO_ERROR){
    state = ATTN_REQ;
  }
  else if (state == ATTN_REQ){
     state = FATAL_ERROR;
  }
  else{
     state = NO_ERROR;
  }
 }
}

int main(void){

    //set outputs
    *P0_DIR |= RED_LED;
    *P0_DIR |= GREEN_LED; 

    //start with led off
    *P0_OUTSET = RED_LED;
    *P0_OUTSET = GREEN_LED;
    
    //start threads
    led_thread.start(led_diag_handler);
    tester_thread.start(diag_tester);

    //initialization complete
    printf("Initializaiton complete. \r\n");
    //yield CPU
    while(true){

        printf("Main thread is running. \r\n");
        thread_sleep_for(1000);
    }
    
}

