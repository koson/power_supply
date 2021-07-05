#ifndef TIMER3_H_
#define TIMER3_H_

#include "stm32f10x.h"

/*!
    \brief Timers class
    Tim1 - first timer - 100 ms for exclude button rattling
    Tim4 - 1 second timer interval
    --- Tim2+Tim3  - 32bits counter, counts on ETR input ---
    Tim3 - slave
*/

class Timers
{
public:
    Timers(uint8_t init);
    static volatile bool timer_mSecFlag;
    uint16_t encoderValue();
    void checkValue();
private:
    void init(uint8_t init);        
};

#endif //TIMER3_H_
