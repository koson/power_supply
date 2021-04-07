#include "adc.h"

ADC::ADC() {
    init();
}

void ADC::start() {
    ADC1->CR2 |= ADC_CR2_JSWSTART;
    while(!(ADC1->SR & ADC_SR_JEOC)); //while not ending all conversions
    data[0] = ADC1->JDR1; //ch8  B0 - Termistor
    data[1] = ADC1->JDR2; //ch14 C4 - Voltage
    data[2] = ADC1->JDR3; //ch15 C5 - Amperage
    average();
}

void ADC::average() {
    sum_amperage -= arr_amperage[count];
    sum_amperage += data[2];
    arr_amperage[count] = data[2];
    amperage = sum_amperage >> 4;

    sum_voltage -= arr_voltage[count];
    sum_voltage += data[1];
    arr_voltage[count] = data[1];
    voltage = sum_voltage >> 4;

    sum_temperature -= arr_temperature[count];
    sum_temperature += data[0];
    arr_temperature[count] = data[0];
    temperature = sum_temperature >> 4;
    
    count == num-1 ? count = 0 : count++;
}

//!__________________ private _________________________
void ADC::init() {
    //! gpio init PC4-in14 PC5-in15 PB0-in8
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN ; // разрешение тактирования порта A
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN ;
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN ;
    GPIOC->CRL &=~ (GPIO_CRL_MODE4 | GPIO_CRL_MODE5| GPIO_CRL_CNF4 | GPIO_CRL_CNF5); // PC4_PC5 analog mode
    GPIOB->CRL &=~ (GPIO_CRL_MODE0 | GPIO_CRL_CNF0); // PB0 analog mode
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN; //clock adc1
    RCC->CFGR |= RCC_CFGR_ADCPRE_DIV2; //Биты 15:14 ADCPRE [1:0]. Предделитель АЦП.    
	
	ADC1->CR2 &=~ ADC_CR2_CONT; //0-single conversion,                  1 - continious conversion	
    ADC1->CR1 |= ADC_CR1_SCAN; // enable scan mode
    ADC1->SMPR2 |= ADC_SMPR2_SMP8; // 111:480 cycles in ch8;
    ADC1->SMPR1 |= ADC_SMPR1_SMP14; // 111:480 cycles in ch14;
    ADC1->SMPR1 |= ADC_SMPR1_SMP15; // 111:480 cycles in ch15;
	//! injected channel sequence
    ADC1->CR2 |= ADC_CR2_JSWSTART; //event start conversion injected channel from software JSWSTART
    ADC1->JSQR |= ADC_JSQR_JL_1; // 1:0 - 3 conversion (J2,J3,J4 - ch8,ch14,ch15)
    ADC1->JSQR |= ((8<<5)|(14<<10)|(15<<15));

    
	ADC1->CR2 |= ADC_CR2_JEXTSEL;//111 start conversion on software external signal by rising edge
    ADC1->CR2 |= ADC_CR2_JEXTTRIG; // enable conversion on external trigger
	//ADC1->SMPR1|=ADC_SMPR1_SMP16; // 111:480 cycles;
	//ADC1->CR1|=ADC_CR1_AWDCH_4; //wadchdog for 10000 adc 16 channel for temp sensor
	//ADC1->CR1&=~ADC_CR1_AWDCH_3;ADC1->CR1&=~ADC_CR1_AWDCH_2;ADC1->CR1&=~ADC_CR1_AWDCH_1;ADC1->CR1&=~ADC_CR1_AWDCH_0;
	//ADC1->SQR1&=~ADC_SQR1_L; // 0000 - 1 conversion on a channel
	//ADC1->SQR3|=ADC_SQR3_SQ1_4; //10000 - 16 channel on 1 conversion 
	//ADC1->SQR3&=~ADC_SQR3_SQ1_3;ADC1->SQR3&=~ADC_SQR3_SQ1_2;ADC1->SQR3&=~ADC_SQR3_SQ1_1;ADC1->SQR3&=~ADC_SQR3_SQ1_0;   
    ADC1->CR2 |= ADC_CR2_CAL; //start calibrating
    ADC1->CR2|=ADC_CR2_ADON; //enable ADC
    while((ADC1->CR2 & ADC_CR2_CAL));     
}