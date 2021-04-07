#ifndef DAC_H
#define DAC_H

#include "stm32f10x.h"

class Dac {
public:
    Dac();
    void setVoltage(float voltage);
    float voltage = 0;
private:
    void init();
};

#endif //DAC_H