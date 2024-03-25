#ifndef PINS_INCLUDED
#define PINS_INCLUDED

#undef __ARM_FP
#include <mbed.h>

//between p15 to VCC 10M ohm. between p15 to GND 10k ohm
#define PIN_ZERO_CROSS p15

//H-Bridge signals 
//https://www.onsemi.com/download/data-sheet/pdf/fad6263-d.pdf

//for U8
#define PIN_SHUTDOWN_ACTIVE_LOW_1 p5 //has 1k pull up to 5V (SD bar pin 2)
#define PIN_SHUTDOWN_RESET_1 p6 //sr pin 3
#define PIN_PWM_IN_1 p26 // pin 1 
//for U9
#define PIN_SHUTDOWN_ACTIVE_LOW_2 p7 //has 1k pull up to 5V (SD bar pin 2)
#define PIN_SHUTDOWN_RESET_2 p8 //sr pin 3
#define PIN_PWM_IN_2 p24 // pin 1 

//ignore
#define PIN_RFID p14 //not used 

//CAN between motion board and wireless power board
#define PIN_CAN_RX_2 p30
#define PIN_CAN_TX_2 p29

//I2C Multiplexer TCA9458APWR
//A0, A1, A2  
//both sda and scl are connected with 2k pull up resistor 
#define PIN_SDA p28
#define PIN_SCL p27 
#define PIN_NOT_RESET p25 //connects to pin 3 of the multiplexer

#endif // PINS_INCLUDED