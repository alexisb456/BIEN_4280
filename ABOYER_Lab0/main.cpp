#include "mbed.h"
#include "USBSerial.h"
#include "ABoyer_binaryutils.hpp"

USBSerial pc;
// main() runs in its own thread in the OS
int main()
{
    uint32_t solo = 0;

    //set bit 24
    setbit(&solo, 24);

    //setbits 16 and 17
    setbit(&solo, 16);
    setbit(&solo, 17);

    //set bits 0 through 11
    setbits(&solo, 0x00000FFF);

    // clear bit 11
    clearbit(&solo, 11);

    //clearbits 4 through 7
    clearbits(&solo, 0x000000F0);

    ThisThread::sleep_for(2s); 

    //header message
    pc.printf("Binary Solo:\r\n");

    //binary representation of solo
    char* binStr = display_binary(solo);
    pc.printf("%s\n\r", binStr);

    while (true) {

    }
}
