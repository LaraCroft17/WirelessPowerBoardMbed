/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#undef __ARM_FP
#include "mbed.h"
#include "CANMsg.h"
#include <cstdio>
#include "wirelessPins.hpp"
#include "TCA9548A.h"
#include "TMAG5273.hpp"
#include "HBridge.hpp"


const unsigned int  RX_ID = 0x101;
const unsigned int  TX_ID = 0x100;


//wireles power PCB microcontroller code

float harmonicMean(float arr[], int size){
   float sum = 0;

   for (int i = 0; i < size; i++)
      sum = sum + (float)1 / arr[i];
   return (float)size/sum;
}

float harmonicMean2(float a, float b){
    float arr[] = {a, b};
    return harmonicMean(arr, 2);
}

float trig1DTest(float leftMag, float rightMag) {
    return 0;
}

float trig1D(float leftMag, float rightMag) {
    //printf("left: %d, right: %d\n", (int)(leftMag * 100), (int)(rightMag * 100));
    float zDist = 2;
    float distBetweenSensors = 5.0f;

    //This is some experimentally found function 
    float distRight = sqrtf(((5000.0f / rightMag) - 1) + (zDist * zDist)); //sensor 2 dist to the magnet
    float distLeft = sqrtf(((5000.0f / leftMag) - 1) + (zDist * zDist)); //sensor 1 dist to the magnet
    //printf("dLeft: %d, dRight: %d\n", (int)(distLeft * 100), (int)(distRight * 100));

    float angB = acosf(-(distRight * distRight - distLeft * distLeft - distBetweenSensors * distBetweenSensors) / (2 * distBetweenSensors * distLeft));
    float dx = distLeft * sinf(angB - 3.1415f / 2);
    float centerDist = dx + distBetweenSensors / 2; // positive is left
    //printf("dx: %d, angB: %d\n", (int)(dx * 10), (int)(angB * 180 / 3.1415f));

    if ((int)(angB * 1000) == 0) { //if angle is invalid, use one for distance 
        //if magnet is far enough usually angB calculation fails 
        if (distRight < distLeft) {
            centerDist = -sqrtf(distRight * distRight - (zDist * zDist)) - (distBetweenSensors / 2);
        } else {
            centerDist = sqrtf(distLeft * distLeft - (zDist * zDist)) + (distBetweenSensors / 2);
        }
    }
    //Debug statements 
    //printf("dist left: %d, right: %d\n", (int)(distLeft * 10), (int)(distRight * 10));
    //printf("ang b deg: %d\n", (int)(angB * 180.0f / 3.1415f));
    //the useful data 
    //printf("distance in in 0.1: %d\n", (int)(10 * centerDist));
    //printf("\n");
    return centerDist;
}

bool isMagnetStable(float* lastXs, float* lastYs, int numLasts) {
    // min - max must be less than 2
    float minX = lastXs[0];
    float maxX = lastXs[0];
    float minY = lastYs[0];
    float maxY = lastYs[0];

    for (int i = 1; numLasts > i; i++) {
        if (lastXs[i] < minX) {
            minX = lastXs[i];
        }
        if (lastXs[i] > maxX) {
            maxX = lastXs[i];
        }
        if (lastYs[i] < minY) {
            minY = lastYs[i];
        }
        if (lastYs[i] > maxY) {
            maxY = lastYs[i];
        }
    }

    bool ret = (maxX - minX) < 2 && (maxY - minY) < 2 && !((maxX - minX) < 2 && (maxY - minY) < 2 && maxX < 1 && maxY < 1);
    // if (ret) {
    //     printf("maxX: %d minX: %d\n", (int)(maxX * 10), (int)(minX * 10));
    //     printf("maxY: %d minY: %d\n", (int)(maxY * 10), (int)(minY * 10));
    // }

    return ret;
}

bool isMagnetReached(float* lastXs, float* lastYs, int numLasts) {
    // min - max must be less than 2
    float minX = lastXs[0];
    float maxX = lastXs[0];
    float minY = lastYs[0];
    float maxY = lastYs[0];

    for (int i = 1; numLasts > i; i++) {
        if (lastXs[i] < minX) {
            minX = lastXs[i];
        }
        if (lastXs[i] > maxX) {
            maxX = lastXs[i];
        }
        if (lastYs[i] < minY) {
            minY = lastYs[i];
        }
        if (lastYs[i] > maxY) {
            maxY = lastYs[i];
        }
    }

    return (maxX - minX) < 1 && (maxY - minY) < 1 && maxX < 0.5 && maxY < 0.5;
}

int main()
{
    // HBridge hBridge(p5, p6, p24, p26);

    // while(true) {
    //     hBridge.start(25000);
    //     wait_us(1000000);
    //     hBridge.stop();
    //     wait_us(1000000);
    // }
    //CAN related definitions
    printf("is it still alive?\n");

    CAN can2(PIN_CAN_RX_2, PIN_CAN_TX_2);               // CAN interface
    can2.frequency(100000);
    CANMessage msg; 

    float location; 
    char direction;

    I2C i2c(PIN_SDA, PIN_SCL);
    i2c.frequency(100000);

    //A0, A1, A2 are all grounded so the address is 0x70 by default 
    TCA9548A i2cMux(&i2c, 0x70, PIN_NOT_RESET);

    // By default TCA9548A performs a power on reset and all downstream ports are deselected
    //First 4 channels are used for the hall sensors, rest are NC
    printf("sending select\n");
    i2cMux.select(0);               //  select  the channel 0
    TMAG5273 hallSensor0 (&i2c); //  making instance after a branch of I2C bus enabled
    wait_us(100000);
    hallSensor0.calibrate();
    hallSensor0.oversampleAmount = 10;
    wait_us(100000);

    i2cMux.select(1);               
    TMAG5273 hallSensor1 (&i2c); 
    wait_us(100000);
    hallSensor1.calibrate();
    hallSensor1.oversampleAmount = 10;
    wait_us(100000);
    
    i2cMux.select(2);               
    TMAG5273 hallSensor2 (&i2c); 
    wait_us(100000);
    hallSensor2.calibrate();
    hallSensor2.oversampleAmount = 10;
    wait_us(100000);

    i2cMux.select(3);               
    TMAG5273 hallSensor3 (&i2c); 
    wait_us(100000);
    hallSensor3.calibrate();
    hallSensor3.oversampleAmount = 10;
    wait_us(100000);

    printf("calibration done\n");   

    //locals for hall sensor
    float s0Mag, s1Mag, s2Mag, s3Mag;
    float distances[2];
    float meanXdir, meanYdir;

    float lastXs[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    float lastYs[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int lastIdx = 0;

    bool chasingMagnet = false;

    DigitalOut led1(LED1);

    while(1) {
        
        //CHECK THIS DEPENDS ON THE WIRING AND PLACEMENT ON THE CARRIAGE 

        i2cMux.select(0);           //  select  the channel 0
        hallSensor0.getMagnitude(&s0Mag);
        s0Mag = s0Mag * 5000.0f / 6380.0f;
        wait_us(1000);
       // printf("magnitude obtained: %d\n", (int)(s0Mag*100));
        i2cMux.select(1);           //  select  the channel 0
        hallSensor1.getMagnitude(&s1Mag); 
        s1Mag = s1Mag * 5000.0f / 6610.0f;
        wait_us(1000);

        i2cMux.select(2);           //  select  the channel 0
        hallSensor2.getMagnitude(&s2Mag);
        s2Mag = s2Mag * 5000.0f / 6690.0f;
        wait_us(1000);
        i2cMux.select(3);           //  select  the channel 0
        hallSensor3.getMagnitude(&s3Mag);
        s3Mag = s3Mag * 5000.0f / 6450.0f;
        wait_us(1000);

        // printf("mags: %d %d %d %d\n", (int)(s0Mag * 100), (int)(s1Mag * 100), (int)(s2Mag * 100), (int)(s3Mag * 100));

        // meanXdir = trig1D(harmonicMean2(s1Mag, s2Mag), harmonicMean2(s0Mag, s3Mag));
        // meanYdir = trig1D(harmonicMean2(s0Mag, s1Mag), harmonicMean2(s2Mag, s3Mag));

        if (s1Mag * s0Mag > s2Mag * s3Mag) {
            meanXdir = trig1D(s1Mag, s0Mag);
        } else {
            meanXdir = trig1D(s2Mag, s3Mag);
        }

        if (s3Mag * s0Mag > s1Mag * s2Mag) {
            meanYdir = trig1D(s3Mag, s0Mag);
        } else {
            meanYdir = trig1D(s2Mag, s1Mag);
        }

        lastXs[lastIdx] = meanXdir;
        lastYs[lastIdx] = meanYdir;
        lastIdx = (lastIdx + 1) % 10;

        if (chasingMagnet) {
            if (isMagnetReached(lastXs, lastYs, 10)) {
                chasingMagnet = false;
                led1.write(0);
                printf("reached the magnet\n");
            }

            memcpy(&(msg.data[0]), &meanXdir, sizeof(float));
            memcpy(&(msg.data[4]), &meanYdir, sizeof(float));

            // // append data (total data length must not exceed 8 bytes!)
            // //  msg << direction;   // one byte
            // // msg << location;   // four bytes //also doesnt work??
                
            if(can2.write(msg)) {       // transmit message
                printf("CAN message sent\n");
            }
            else {
                printf("Transmission error\n");
            }
        } else if (isMagnetStable(lastXs, lastYs, 10)) {
            chasingMagnet = true;
            led1.write(1);
            printf("Chase is on!\n");
        }

        // //DEBUG
        // printf("Sensor 0 - Mag: %d\n", (int)s0Mag);
        // printf("Sensor 1 - Mag: %d\n", (int)s1Mag);
        // printf("Sensor 2 - Mag: %d\n", (int)s0Mag);
        // printf("Sensor 3 - Mag: %d\n", (int)s1Mag);

        printf("Mean x dir * 10: %d\n", (int)(meanXdir * 10));

        printf("Mean y dir * 10: %d\n", (int)(meanYdir * 10));

        //Now communicate the result with the other mbed via CAN

        //msg.clear();   //try to clear for the new message but it errors??
        
        // msg.id = TX_ID;  // setting ID


        wait_us(50000); //wait for carriage to be moved to closer to the target, before sending new data 

    }

    printf("Testing wireless board mbed ...\n");
}
