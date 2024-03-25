/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "CANMsg.h"
#include <cstdio>
#include "wirelessPins.hpp"
#include "TCA9548A.h"
#include "TMAG5273.hpp"


const unsigned int  RX_ID = 0x101;
const unsigned int  TX_ID = 0x100;


//wireles power PCB microcontroller code

float harmonicMean(float arr[], int size){
   float sum = 0;

   for (int i = 0; i < size; i++)
      sum = sum + (float)1 / arr[i];
   return (float)size/sum;
}

float trig1D(float leftMag, float rightMag) {
    //This is some experimentally found function 
    float distRight = sqrtf(((11000.0f / rightMag) - 1) + 4); //sensor 2 dist to the magnet
    float distLeft = sqrtf(((11000.0f / leftMag) - 1) + 4); //sensor 1 dist to the magnet

    float angB = acosf(-(distRight * distRight - distLeft * distLeft - 9.0f) / (2 * 3.0f * distLeft));
    float dx = distLeft * sinf(angB - 3.1415f / 2);
    float centerDist = dx + 1.5f; // positive is left

    if ((int)(angB * 1000) == 0) { //if angle is invalid, use one for distance 
        //if magnet is far enough usually angB calculation fails 
        if (distRight < distLeft) {
            centerDist = -sqrtf(distRight * distRight - 4) - 1.5f;
        } else {
            centerDist = sqrtf(distLeft * distLeft - 4) + 1.5f;
        }
    }
    //Debug statements 
    //printf("dist left: %d, right: %d\n", (int)(distLeft * 10), (int)(distRight * 10));
    //printf("ang b deg: %d\n", (int)(angB * 180.0f / 3.1415f));
    //the useful data 
    printf("distance in in 0.1: %d\n", (int)(10 * centerDist));
    printf("\n");
    return centerDist;
}



int main()
{
    //CAN related definitions
    CAN can2(PIN_CAN_RX_2, PIN_CAN_TX_2);               // CAN interface
    CANMessage msg;     
    float location; 
    char direction;

    //A0, A1, A2 are all grounded so the address is 0x70 by default 
    TCA9548A i2cMux(PIN_SDA, PIN_SCL, 0x70, PIN_NOT_RESET);

    // By default TCA9548A performs a power on reset and all downstream ports are deselected
    //First 4 channels are used for the hall sensors, rest are NC
    i2cMux.select(0);               //  select  the channel 0
    TMAG5273 hallSensor0 (PIN_SDA, PIN_SCL); //  making instance after a branch of I2C bus enabled
    wait_us(100000);
    hallSensor0.calibrate();
    hallSensor0.oversampleAmount = 9;
    wait_us(100000);

    i2cMux.select(1);               
    TMAG5273 hallSensor1 (PIN_SDA, PIN_SCL); 
    wait_us(100000);
    hallSensor1.calibrate();
    hallSensor1.oversampleAmount = 9;
    wait_us(100000);
    
    i2cMux.select(2);               
    TMAG5273 hallSensor2 (PIN_SDA, PIN_SCL); 
    wait_us(100000);
    hallSensor2.calibrate();
    hallSensor2.oversampleAmount = 9;
    wait_us(100000);

    i2cMux.select(3);               
    TMAG5273 hallSensor3 (PIN_SDA, PIN_SCL); 
    wait_us(100000);
    hallSensor3.calibrate();
    hallSensor3.oversampleAmount = 9;
    wait_us(100000);

    printf("calibration done\n");   

    //locals for hall sensor
    float s0Mag, s1Mag, s2Mag, s3Mag;
    float distances[2];
    float meanXdir, meanYdir;

    while(1) {
        
        //CHECK THIS DEPENDS ON THE WIRING AND PLACEMENT ON THE CARRIAGE 

        i2cMux.select(0);           //  select  the channel 0
        hallSensor0.getMagnitude(&s0Mag); 
        wait_us(100);
        i2cMux.select(1);           //  select  the channel 0
        hallSensor1.getMagnitude(&s1Mag); 
        wait_us(100);

        i2cMux.select(2);           //  select  the channel 0
        hallSensor2.getMagnitude(&s2Mag); 
        wait_us(100);
        i2cMux.select(3);           //  select  the channel 0
        hallSensor1.getMagnitude(&s3Mag); 
        wait_us(100);


        distances[0] = trig1D(s0Mag, s1Mag); 
        distances[1] = trig1D(s2Mag, s3Mag); 
        meanXdir = harmonicMean(distances, 2);

        distances[0] = trig1D(s0Mag, s2Mag); 
        distances[1] = trig1D(s1Mag, s3Mag); 
        meanYdir = harmonicMean(distances, 2);

        //DEBUG
        printf("Sensor 0 - Mag: %d\n", (int)s0Mag);
        printf("Sensor 1 - Mag: %d\n", (int)s1Mag);
        printf("Sensor 2 - Mag: %d\n", (int)s0Mag);
        printf("Sensor 3 - Mag: %d\n", (int)s1Mag);

        printf("Mean x dir * 10:  %d\n", (int)meanXdir * 10);
        printf("Mean y dir * 10: %d\n", (int)meanYdir * 10);

        //Now communicate the result with the other mbed via CAN

        //msg.clear();   //try to clear for the new message but it errors??
        
        msg.id = TX_ID;  // setting ID
        msg.data[0] = 'x'; // direction
        memcpy(&(msg.data[1]), &location, sizeof(float)); //alternative to << 

        // append data (total data length must not exceed 8 bytes!)
        // msg << direction;   // one byte
        // msg << location;   // four bytes //also doesnt work??
             
        if(can2.write(msg)) {       // transmit message

            printf("CAN message sent\n");

            printf("direction = %c\n", msg.data[0]);
            printf("location = %e V\r\n", location);
        }
        else {
            printf("Transmission error\n");
        }


        wait_us(5000000); //wait for carriage to be moved to closer to the target, before sending new data 

    }

    printf("Testing wireless board mbed ...\n");
}
