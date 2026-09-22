#include<cstdint>

//sets the bit at the specified position in the 32-bit integer
void setbit(uint32_t *addr, uint8_t whichbit){
    *addr = (*addr | (1<< whichbit));
}

//clears the bit at the specfied position in the 32-bit integer
void clearbit(uint32_t *addr, uint8_t whichbit){
    *addr = (*addr &  ~(1 << whichbit));
}

//sets all bits specified by the bitmask
void setbits(uint32_t *addr, uint32_t bitmask){
    *addr = *addr | bitmask;
}

//clears all bits specified by the birthmark
void clearbits(uint32_t *addr, uint32_t bitmask){
    *addr = *addr & ~ bitmask;
}

//converts a 32-bit unsigned integer into a dynamically allocated null-terminated binary string
char *display_binary(uint32_t num){
    char *result = new char[33];

    for(int i = 31; i >= 0; i--){
        if(num & (1 << i)){
            result[31 - i] = '1';
        }
        else {
            result[31 -i] = '0';
        }
    }

    result[32] = '\0';

    return result;
}
