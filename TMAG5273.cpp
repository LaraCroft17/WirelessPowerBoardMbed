#include "TMAG5273.hpp"
#include <stdint.h>

TMAG5273::TMAG5273(I2C* i2c): i2c(i2c) {
    //set frequency
    //fast mode 400khz table 6.10
    //i2c->frequency(400000);

    //set operating mode to continuous
    char operating_mode = 0x2; //char = 1 byte 0000 0010 for cont,measure mode (table 7-9)
    char device_config2_register = 0x1; //table 7-6
    char buf[2] = {device_config2_register, operating_mode};
    i2c->write(writeAddr, buf, 2); //3rd one is length of the buff 

    char conv_crc = 0x94; //1001 0100 CONV AVG: bits 4-2 highest 5h  
    // and CRC is enabled on bit 7
    char device_config1_register = 0x0; //table 7-6
    buf[0] = device_config1_register;
    buf[1] = conv_crc;
    i2c->write(writeAddr, buf, 2); 

    //Sensor config 1 channel 
    char mag_enable = 0x70; //0111 0000 this is enabling all three channels 
    char sensor_config_1 = 0x2; //table 7-6
    buf[0] = sensor_config_1;
    buf[1] = mag_enable;
    i2c->write(writeAddr, buf, 2); 

}

bool TMAG5273::crcCheck(char crc) {
    //TO DO 
    return true;
}
bool TMAG5273::getHallData(int* x, int* y, int* z) {
    //Table 7.6
    /*Register - what to read
    12h - X_MSB_RESULT 
    13h - X_LSB_RESULT 
    14h - Y_MSB_RESULT  
    15h - Y_LSB_RESULT 
    16h - Z_MSB_RESULT 
    17h - Z_LSB_RESULT 
    */
    char x_msb = 0x12; 
    char x_lsb = 0x13; 
    char y_msb = 0x14; 
    char y_lsb = 0x15; 
    char z_msb = 0x16; 
    char z_lsb = 0x17; 

    int xT = 0, yT = 0, zT = 0;
    for (int i = 0; oversampleAmount >= i; i++) {
        if (i > 0) {
            wait_us(2500);
        }

        char buf[5];
        buf[0] = x_msb;
        i2c->write(readAddr, buf, 1);  //Reads  x msb and lsb
        i2c->read(readAddr, buf, 5);   
        xT += ((int)(int16_t)(buf[0] << 8 | buf[1])) - xOffset;

        char bufY[5];
        bufY[0] = y_msb;
        i2c->write(readAddr, bufY, 1);  //Reads  x msb and lsb
        i2c->read(readAddr, bufY, 5);   
        yT += ((int)(int16_t)(bufY[0] << 8 | bufY[1])) - yOffset;
        
        
        char bufZ[5];
        bufZ[0] = z_msb;
        i2c->write(readAddr, bufZ, 1);  //Reads  x msb and lsb
        i2c->read(readAddr, bufZ, 5);   
        zT += ((int)(int16_t)(bufZ[0] << 8 | bufZ[1])) - zOffset;
    }

    *x = xT / (oversampleAmount + 1);
    *y = yT / (oversampleAmount + 1);
    *z = zT / (oversampleAmount + 1);

    // char bufXY[5];
    // bufXY[0] = x_msb;
    // i2c.write(readAddr, bufXY, 1);  //Reads 4 registers at a time: x msb and lsb, y msb and lsb 
    // i2c.read(readAddr, bufXY, 5);   
    // //seperate the axes and concat the values
    // *x = (int)(int16_t)(bufXY[0] << 8 | bufXY[1]);
    // *y = (int)(int16_t)(bufXY[2] << 8 | bufXY[3]);

    bool successXY = true;//crcCheck(bufXY[4]); 

    // char bufZ[5];
    // bufZ[0] = z_msb;
    // i2c.write(readAddr, bufZ, 1);  //Reads Z msb and lsb
    // i2c.read(readAddr, bufZ, 5, true);   
    // *z = (int)(int16_t)(bufZ[0] << 8 | bufZ[1]);
    bool successZ = true; // crcCheck(bufZ[4]); 

    if (successXY && successZ) {
        return true;
    }
    else  {
        return false;
    }
}

void TMAG5273::getMagnitude(float* mag) {
    int x, y, z;
    getHallData(&x, &y, &z);
    *mag = sqrt((x * x) + (y * y) + (z * z));
}

void TMAG5273::calibrate() {
    int xData, yData, zData;
    int xTotal = 0, yTotal = 0, zTotal = 0;
    xOffset = 0, yOffset = 0, zOffset = 0;
    for (int i = 0; i < 100; i++) {
        getHallData(&xData, &yData, &zData);
        xTotal += xData;
        yTotal += yData;
        zTotal += zData;
        wait_us(1000);
    }
    xOffset = xTotal/100;
    yOffset = yTotal/100;
    zOffset = zTotal/100;
}