/*
Author: ALEXIS BOYER
Date: 10/06/2026
*/
#include "mbed.h"
#include "USBSerial.h"

#define P0_OUTSET ((uint32_t*) 0x50000508)
#define P0_OUTCLR ((uint32_t*) 0x5000050C)
#define P0_DIR    ((uint32_t*) 0x50000514)

// Port 1 registers for Part 3
#define P1_OUTSET ((uint32_t*) 0x50000808)
#define P1_DIR ((uint32_t*) 0x50000814)

#define GREEN_LED (1UL << 16)
#define RED_LED (1UL << 24)
#define SENSOR_VDD (1UL << 22)

//sensor power for part 3
#define SENSOR_POWER (1UL << 0)

#define HUMIDITY 0x1
#define TEMPERATURE 0x2

//HS3000 I2C address for Part 3
#define HS3000_ADDR (0x44 << 1)

USBSerial Serial;

//I2C initialization for Part 3
I2C i2c(I2C_SDA1, I2C_SCL1); 

EventFlags Humidity_event_flag;
EventFlags Temperature_event_flag;

Thread read_temperature_thread;
Thread read_humidity_thread;

Ticker sensor_ticker;

Mutex mutex;
//create mutex for I2C part 3
Mutex I2C_mutex; 

/*
Part 2 Code 
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

        ThisThread::sleep_for(1s);

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

        ThisThread::sleep_for(1s);

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

    sensor_ticker.attach(&ISR_sensor, 3s);

    while (true) {
        ThisThread::sleep_for(1s);
    }
}
*/

//Calibrate the humidity 
int calculate_humidity(uint32_t raw_humidity){
    return(raw_humidity * 100) / 16383; 
}

//Calibrate temperature 
int calculate_temerature(uint32_t raw_temperature){
    return(raw_temperature * 165) / 16383 - 40; 
}

//read temperatue and humidty from the sensor 
bool read_sensor(int &humidity, int &temperature){
    //create array to hold data
    char data[4];

    //lock mutex to prevent both threads from accessing I2C
    I2C_mutex.lock();

    //send measurement request to sensor 
    int result = i2c.write(HS3000_ADDR, nullptr,0);

    if (result != 0){
        I2C_mutex.unlock();
        return false; 
    }

    //wait for measurement 
    ThisThread::sleep_for(50ms);

    //read from sensor
    result = i2c.read(HS3000_ADDR, data, 4);

    if (result != 0){
        I2C_mutex.unlock();
        return false;
    }

    //Release I2C resource 
    I2C_mutex.unlock();

    //check sensor status
    uint8_t status = ((uint8_t)data[0] >> 6) & 0x03;

    //status 00 means valid new data 
    if (status != 0){
        return false; 
    }

    //get the humidity measurement (raw)
    uint32_t raw_humdity = 
        (((uint8_t)data[0] & 0x3F) << 8) | (uint8_t)data[1];

    //get temperature measurement (raw)
    uint32_t raw_temperature = 
        ((uint8_t)data[2] << 6) | ((uint8_t)data[3] >> 2);

    //convert the raw data
    humidity = calculate_humidity(raw_humdity);
    temperature = calculate_temerature(raw_temperature);

    return true; 
}


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

        int humidity;
        int temperature;

        //read sensor
        bool good_read = read_sensor(humidity, temperature);

        //turn on red led 
        *P0_OUTCLR = RED_LED;

        mutex.lock();

        if(good_read){
            Serial.printf("Reading Temperature: %d C\r\n", temperature);
        }
        else {
            Serial.printf("Temperature sensor read failure\r\n");
        }

        mutex.unlock();

        ThisThread::sleep_for(1s);

        *P0_OUTSET = RED_LED;

    }
}

void read_humidity(){
    while(true){
        Humidity_event_flag.wait_any(HUMIDITY);

        int humidity;
        int temperature;

        //read sensor
        bool good_read = read_sensor(humidity,temperature);

        //turn on green led
        *P0_OUTCLR = GREEN_LED;

        mutex.lock();

        if(good_read){
            Serial.printf("Reading Humidity: %d %\r\n", humidity);
        }
        else{
            Serial.printf("Humidity sensor read failed\r\n");
        }

        mutex.unlock();

        ThisThread::sleep_for(1s);

        *P0_OUTSET = GREEN_LED;
    }
}

//

// main() runs in its own thread in the OS
int main()
{
    *P0_DIR |= GREEN_LED | RED_LED;

    *P0_OUTSET = RED_LED | GREEN_LED;

    *P0_DIR |= SENSOR_VDD;
    *P0_OUTSET = SENSOR_VDD;

    //configure sensor output
    *P1_DIR |= SENSOR_POWER;

    //enable sensor 
    *P1_OUTSET = SENSOR_POWER;

    ThisThread::sleep_for(100ms);

    //set i2c frequency 
    i2c.frequency(100000);

    //print start message
    mutex.lock();
    Serial.printf("HS3000 Sensor Begin \r\n");
    mutex.unlock();

    read_temperature_thread.start(read_temperature);
    read_humidity_thread.start(read_humidity);

    sensor_ticker.attach(&ISR_sensor, 3s);

    while (true) {
        ThisThread::sleep_for(1s);
    }
}

