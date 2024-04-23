#ifndef TMAG5273_INCLUDED
#define TMAG5273_INCLUDED

#include "mbed.h"

class TMAG5273 {
 public:
    TMAG5273(I2C* i2c); //configures at initialization 
    bool getHallData(int* x, int* y, int* z);
    void getMagnitude(float* mag);
    void calibrate(); 

    int oversampleAmount = 0;
 private:
    I2C* i2c;
    //table 7-2 
    const int addr7bit = 0x35;      //I2C address (7-bit I2C address)
                                    // 35: 0011 0101
    const int writeAddr = 0x6A;     //I2C write address (8bit) -> 7 bit addr + 1 bit r/!w
                                    // 6A: 0110 1010 
    const int readAddr = 0x6B;     //I2C read address (8bit) -> 7 bit addr + 1 bit r/!w
                                    // 6A: 0110 1011

    int xOffset, yOffset, zOffset; 

    bool crcCheck(char crc);        
};

#endif //for TMAG5273