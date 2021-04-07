#ifndef PWM_H
#define PWM_H

#include "stm32f10x.h"

class PWM {
public: 
    PWM();
    void setFrequency(uint32_t kHz);
    void setDuty(uint8_t duty);
    uint8_t Frequency;
    uint8_t Duty;
private:
    void init();
};

#endif //PWM_H