

#ifndef HBRIDGE_INCLUDED
#define HBRIDGE_INCLUDED

#undef __ARM_FP
#include <mbed.h>

class HBridge {
private:
    PinName pinA;
    PinName pinB;
    DigitalOut sd;
    DigitalOut sr;
    pwmout_t pwmA;
    pwmout_t pwmB;

public:
    /**
     * Make new instance of the H Bridge class. Note that pins a and b are of arbitrary
     * order; it only matters that they are the two control signals.
     */
    HBridge(PinName sd, PinName sr, PinName a, PinName b);

    /**
     * Enables the H bridge and starts the control signal at the specified frequency
     * @param frequency The frequency for one complete cycle of the H bridge, in hertz.
     */
    void start(float frequency);

    /**
     * Puts the H Bridge into a disabled state, with all IGBTs off.
     */
    void stop();
};

#endif // HBRIDGE_INCLUDED