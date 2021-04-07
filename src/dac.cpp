#include "dac.h"

Dac::Dac() {
    init();
}

void Dac::setVoltage(float voltage) {
    this->voltage = voltage;
    uint16_t x = (uint16_t)(4095*(voltage/3));
    DAC->DHR12R1 = x;
}

//_________________________ private _______________________________________
void Dac::init() {
    //! GPIO init PA4 DAC1 output
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    GPIOA->CRL &=~ (GPIO_CRL_MODE4 | GPIO_CRL_CNF4); // PB0 analog imode (for DAC output)
    //! DAC init
    RCC->APB1ENR |= RCC_APB1ENR_DACEN;
    DAC->CR |= DAC_CR_EN1;
}