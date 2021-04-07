#ifndef ADC_H_
#define ADC_H

#include "stm32f10x.h"

class ADC
{
public:
    ADC();    
    uint16_t data[3] = {0};
    uint16_t voltage = 0;
    uint16_t amperage = 0;
    uint16_t temperature = 0;
    void start();
private: 
    void init();    
    void average();
    
    static constexpr uint8_t num = 16;
    
    uint16_t sum_voltage = 0;
    uint16_t sum_amperage = 0;
    uint16_t sum_temperature = 0;
    uint16_t arr_voltage[num] = {0};
    uint16_t arr_amperage[num] = {0};
    uint16_t arr_temperature[num] = {0};
    uint8_t count = 0;

    
};



#endif //ADC_H