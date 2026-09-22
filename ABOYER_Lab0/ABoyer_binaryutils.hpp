/* 
    Author: Alexis Boyer 
    Date: 09/01/2026
    This header file defines a collection of functions for manipulation
    Individiual bits and gorups of bits in 32-bit unsigned integers.
    It also defines a function for converting a 32-bit unsigned itneger
    into a null-terminated binary string. 
*/

#include <cstdint>


//Sets the bit at the specified postion in the 32-bit integer
void setbit(uint32_t* addr,uint8_t whichbit);

//clears the bit at the specified position in the 32-bit integer
void clearbit(uint32_t* addr, uint8_t whichbit);

//Sets all bits in the integer that correspond to 1s in the bitmask
void setbits(uint32_t* addr, uint32_t bitmask);

//Clears all bits in the integer that correspond to 1s in the bitmask
void clearbits(uint32_t* addr, uint32_t bitmask);

//Converts a 32-bit unsigned integer into a binary string
char *display_binary(uint32_t num);

