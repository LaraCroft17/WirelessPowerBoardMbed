/**
 *  TCA9548A library
 *
 *  @author Akash Vibhute
 *  @author < akash . roboticist [at] gmail . com >
 *  @version 0.1
 *  @date May/24/2016
 */
 
#include "tca9548a.h"
 
TCA9548A::TCA9548A( I2C* i2c, uint8_t i2c_address, PinName resetPin) : i2c_(i2c), reset_pin(resetPin)
{
    i2c_addr = i2c_address;
    reset_pin.write(1);
}

 
void TCA9548A::select( uint8_t channel )
{
    if(channel >= 8) return;
    char data[1];
    data[0] = 1 << channel;
    int err = i2c_->write(0xE0, data, 1);
    if (err) {
        printf("failed to write to the mux: %d\n", err);
    }
    wait_us(20000);
}

char TCA9548A::check_set_channel()
{
    char data[1];
    int err = i2c_->read(0xE1, data, 1);
    return data[0];
}
 
void TCA9548A::reset( )
{
    reset_pin = 0;   // asserting low to reset the system
    wait_us(1000); //1ms = 1000us
    reset_pin = 1; //open it back
}
 