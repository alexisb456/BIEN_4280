#include "mbed.h"
#include "USBSerial.h"

//GPIO register addresses for Port 0
#define P0_OUTSET ((uint32_t*) 0x50000508) //set GPIO pins HIGH
#define P0_OUTCLR ((uint32_t*) 0x5000050C) //set GPIO pins LOW
#define P0_DIR ((uint32_t*) 0x50000514)    //configure GPIO pin direction

//GPIO register addresses for Port 1
#define P1_OUTSET ((uint32_t*) 0x50000808) //set GPIO pins HIGH
#define P1_DIR ((uint32_t*) 0x50000814)    //configure GPIO pin direction

//LED bitmasks
#define RED_LED (1UL << 24)   //red LED on Port 0 pin 24

//Sensor power pins
#define SENSOR_VDD (1UL << 22)    //sensor power on Port 0 pin 22
#define SENSOR_PULLUP (1UL << 0)  //sensor pullup on Port 1 pin 0

//I2C sensor addresses
#define SENSOR_WRITE 0x72 //sensor address for writing
#define SENSOR_READ 0x73  //sensor address for reading

//Sensor register addresses
#define ID_REG 0x92       //register containing sensor ID
#define PROX_ENABLE 0x80  //register used to enable proximity sensing
#define PROX_DATA 0x9C    //register containing proximity measurement
#define PROX_GAIN 0x9F    //register used for proximity gain settings
#define GAIN_CONTROL 0x8F //register used to control gain

//initialize Serial communication 
USBSerial Serial;

//initialize I2C communication using SDA and SCL
I2C i2c(I2C_SDA1, I2C_SCL1);

//initialize threads 
Thread proximity_thread;
Thread LED_thread; 

//create levels of power for brightness 
enum PowerLevel {
    OFF, 
    ONE,
    TWO,
    THREE,
    FOUR,
    FIVE
};

//Struct to send power levels between the threads 
typedef struct {
    PowerLevel state; //state stores current LED power level 
} ProximityMessage;

//mail box holds 5 proximity messages
Mail<ProximityMessage, 5>mail_box; 

void proximity_read() {

    char sensor_id = 0;
    char id_reg = ID_REG; //adding because the value of an int cannot be taken (error fix)

    //perform handshake with sensor 
    //write ID register address to the sensore
    int connectWrite = i2c.write(SENSOR_WRITE, &id_reg, 1, true);

    //read one byte containing the sensor ID (should be first byte)
    int connectRead = i2c.read(SENSOR_READ, &sensor_id, 1);

    //check for succesfull connection with I2C (debugging)
    if (connectWrite == 0 && connectRead == 0){
        Serial.printf("Handshake successful\r\n"); //print succes 
        //print the sensor id to determine connecting with correct sensor (more debugging)
        Serial.printf("Sensor ID: %02X\r\n", (uint8_t)sensor_id);
    } else {
        Serial.printf("Handshake Failed\r\n");
    }

    //enable proximity sensore 
    char enable[] = {PROX_ENABLE, 5};
    i2c.write(SENSOR_WRITE, enable, 2);

    //configure proximity gain 
    char gain_enable[] = {PROX_GAIN, 32};
    i2c.write(SENSOR_WRITE, gain_enable, 2);
    
    //set gain control 
    char gain[] = {GAIN_CONTROL, 12};
    i2c.write(SENSOR_WRITE, gain, 2); 

    //array for stored proximity measurements 
    char data[1];
    
    //store raw proximity 
    uint8_t proximity;

    //stores calculated distance in centimeters
    float calibrated_proximity; 

    //read from the sensor continuously 
    while(true) {

        char prox_data = PROX_DATA; //adding becasue cannot take value of int (error fixing)
        int write = i2c.write(SENSOR_WRITE, &prox_data, 1, true);

        //read
        int read = i2c.read(SENSOR_READ, data, 1);

        //check for sensor communication fail (debugging)
        if (write != 0 || read != 0){ 
            Serial.printf("Proximity read failed\r\n");
            thread_sleep_for(100);
            continue;
        }

        //convert data into unsigned int
        proximity = (uint8_t)data[0];

        //calculate the distance as long as proximity is greater then minimum 
        if (proximity > 4) {

            calibrated_proximity = 8.35 * exp(-0.0065 * proximity);

            //seperate the float into ints for serial
            int distance = (int)calibrated_proximity; 
            int decimal = (int)(calibrated_proximity * 100) % 100;

            Serial.printf("Distance: %d.%02d cm\r\n", distance, decimal);

        }

        //allocate memory for a proximity message 
        ProximityMessage *mail = mail_box.try_alloc();

        //check mailbox allocation 
        if (mail == nullptr){
            thread_sleep_for(100);
            continue;
        }

        //determine LED power based on proximity 
        if(proximity >4 && proximity <= 51) {
            mail->state = ONE;
        } else if (proximity > 51 && proximity <= 102){
            mail->state = TWO;
        }else if (proximity > 102 && proximity <= 153){
            mail->state = THREE;
        }else if (proximity > 153 && proximity <= 204){
            mail->state = FOUR;
        }else if (proximity > 204){
            mail->state = FIVE;
        }else {
            mail->state = OFF;
        }

        //send powerlevel to LED using mail
        mail_box.put(mail);

        //wait 100ms
        thread_sleep_for(100);
    }
}

void LED() {

    PwmOut blue(p6);
    PwmOut red(P0_24);

    //set period
    red.period_ms(1);
    blue.period_ms(1);

    //check the mailbox for incoming new powerlevels 
    while(true){

        osEvent event = mail_box.get(0);

        if (event.status == osEventMail){

            ProximityMessage *mail = (ProximityMessage*)event.value.v;

            //set the led brightness based on power level 
            if (mail->state == OFF) {
                blue.write(1.0);
                red.write(0.0);
            } else if (mail->state == ONE) {
                blue.write(0.8);
                red.write(1.0);
            }else if (mail->state == TWO){
                blue.write(0.6);
                red.write(1.0);
            }else if (mail->state == THREE){
                blue.write(0.4);
                red.write(1.0);
            }else if (mail->state == FOUR){
                blue.write(0.2);
                red.write(1.0);
            }else if (mail->state == FIVE){
                blue.write(0.0);
                red.write(1.0);
            }

            mail_box.free(mail);
        }

        thread_sleep_for(10);
    }
}

// main() runs in its own thread in the OS
int main()
{
    *P0_DIR |= SENSOR_VDD;
    *P1_DIR |= SENSOR_PULLUP;

    *P0_OUTSET = SENSOR_VDD;
    *P1_OUTSET = SENSOR_PULLUP;

    thread_sleep_for(100);

    proximity_thread.start(callback(proximity_read));

    thread_sleep_for(100);

    LED_thread.start(callback(LED));


    while (true) {
        thread_sleep_for(1000);
    }
}

