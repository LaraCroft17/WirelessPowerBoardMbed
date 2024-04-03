#include "HBridge.hpp"

extern __IO uint32_t *PWM_MATCH[7];

/**
    * Make new instance of the H Bridge class. Note that pins a and b are of arbitrary
    * order; it only matters that they are the two control signals.
    */
HBridge::HBridge(PinName sd, PinName sr, PinName a, PinName b): sd(sd), sr(sr) {
    this->pinA = a;
    this->pinB = b;

    this->sd.write(0);
    this->sr.write(0);
}

/**
    * Enables the H bridge and starts the control signal at the specified frequency
    * @param frequency The frequency for one complete cycle of the H bridge, in hertz.
    */
void HBridge::start(float frequency) {
    // configuring PWM
    // PWM is 96MHz (CCLK) / 4 = 24 MHz
    pwmout_init(&this->pwmA, this->pinA);
    pwmout_init(&this->pwmB, this->pinB);

    LPC_PWM_TypeDef* pwm1 = LPC_PWM1;
    pwm1->TCR = 0x2; // resetting TC and stopping what pwmout_init started

    // setting up prescaler
    pwm1->PR = (uint32_t)24000000 / (frequency * 16);
    uint32_t mcr = pwm1->MCR;
    mcr &= ~(1<<2); // clearing the bit for stopping on match 3
    mcr |= 1<<1;
    pwm1->MCR = mcr;

    wait_us(10000); // give bootstrap capacitor time to charge
    this->sr.write(1);
    wait_us(3);
    this->sd.write(1);
    this->sr.write(0);

    // starting PWM
    pwm1->PCR |= 1<<3; // setting double edge control mode for channel 3

    // match 0 is the rising edge for A
    // match 1 is the falling edge for A
    // match 2 is the rising edge for B
    // match 3 is the falling edge for B
    pwm1->MR0 = 16; // timer resets to 1
    pwm1->MR1 = 8;
    pwm1->MR2 = 8;
    pwm1->MR3 = 16;
    pwm1->LER = 0xF; // committing the match register values

    // enabling PWM
    pwm1->TCR = 0x9; // enable counter and PWM
}

/**
    * Puts the H Bridge into a disabled state, with all IGBTs off.
    */
void HBridge::stop() {
    this->sd.write(0);

    // disabling PWM - setting it to stop after last match
    uint32_t mcr = LPC_PWM1->MCR;
    mcr &= ~(1<<1); // clearing the bit for resetting 
    mcr |= 1<<2; // setting bit for stopping on match 3
    LPC_PWM1->MCR = mcr;
}