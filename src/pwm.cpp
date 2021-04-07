#include "pwm.h"

PWM::PWM() {
    init();
}

void PWM::setFrequency(uint32_t kHz) {
    uint32_t x = 71800/kHz;
    TIM4->ARR = x;
    setDuty(50);
    Frequency = kHz;
}

void PWM::setDuty(uint8_t duty) {
    uint16_t reload = TIM4->ARR; 
    uint16_t duty_real = (duty*reload)/100;
    TIM4->CCR2 = duty_real; // channel 2
    Duty = duty;
}

void PWM::init() {
    Frequency = 1;
    //! GPIO init: PB7 - TIM4_ch2 pwm output
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    GPIOB->CRL &=~ (GPIO_CRL_MODE7 | GPIO_CRL_CNF7); //11: Output mode, max speed 50 MHz.
    GPIOB->CRL |= (GPIO_CRL_MODE7 | GPIO_CRL_CNF7_1); //10: Alternate function output Push-pull
    
    RCC->APB1ENR|=RCC_APB1ENR_TIM4EN; 
    AFIO->MAPR &=~ AFIO_MAPR_TIM4_REMAP; // 0: No remap (PB7 tim4_ch2_output)

    TIM4->CR1 |= TIM_CR1_CEN; //counter enable
    TIM4->PSC = 0;
    TIM4->ARR = 0xFFFF; //for which value is counting
    TIM4->EGR |= TIM_EGR_UG; //update generation
    TIM4->CCER |= TIM_CCER_CC2E; // enable signal on output pin OC2
    TIM4->CCMR1 &=~ TIM_CCMR1_CC2S; // 0:0 channel is configured as output
    TIM4->CCMR1 |= (TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1); // 1:1:0 - PWM1 mode (active while < CCR2) 
}