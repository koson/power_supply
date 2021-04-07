#include "but.h"

But* But::pThis = nullptr;

But::But() {
    init();
    pThis = this;
}

void But::startFan() {
    GPIOB->BSRR |= GPIO_BSRR_BS8;
}

void But::stopFan() {
    GPIOB->BSRR |= GPIO_BSRR_BR8;
}

void But::init() {
    RCC->APB2ENR|=RCC_APB2ENR_IOPCEN;        
    GPIOC->CRH &=~ (GPIO_CRH_MODE11 | GPIO_CRH_CNF11 | GPIO_CRH_MODE12 | GPIO_CRH_CNF12); // reset
    GPIOC->CRH |= (GPIO_CRH_CNF11_1 | GPIO_CRH_CNF12_1); // CNF11[1:0] 1:0  - input mode with pull-up / pull-down
    GPIOC->ODR |= (GPIO_ODR_ODR11 | GPIO_ODR_ODR12); //pull-up
    AFIO->EXTICR[2] |= (AFIO_EXTICR3_EXTI11_PC); //11 to EXTI PC
    AFIO->EXTICR[3] |= (AFIO_EXTICR4_EXTI12_PC); //12 to EXTI PC
    EXTI->RTSR=0;
    EXTI->RTSR &=~ (EXTI_RTSR_TR11 | EXTI_RTSR_TR12); //rising
    EXTI->FTSR |= (EXTI_FTSR_TR11 | EXTI_FTSR_TR12); //falling
    EXTI->PR = (EXTI_PR_PR11 | EXTI_PR_PR12);      //clear interrupt flag
    EXTI->IMR |= (EXTI_IMR_MR11 | EXTI_IMR_MR12);   //turn on interrupt from channel EXTI
    NVIC_EnableIRQ(EXTI15_10_IRQn);  //Разрешаем прерывание в контроллере прерываний

    RCC->APB2ENR|=RCC_APB2ENR_IOPCEN;  
    RCC->APB2ENR|=RCC_APB2ENR_AFIOEN;      
    GPIOC->CRL &= ~(GPIO_CRL_MODE3 | GPIO_CRL_CNF3); // reset
    GPIOC->CRL|=GPIO_CRL_CNF3_1; // CNF0[1:0] 1:0  - input mode with pull-up / pull-down
    GPIOC->ODR |= GPIO_ODR_ODR3; //Подтяжка вверх
    AFIO->EXTICR[0] |= (AFIO_EXTICR1_EXTI3_PC); //третий канал EXTI подключен к порту PC3
    EXTI->RTSR=0;
    EXTI->RTSR &=~ EXTI_RTSR_TR3; //Прерывание по нарастанию импульса отключено
    EXTI->FTSR |= EXTI_FTSR_TR3; //Прерывание по спаду импульса
    EXTI->PR |= EXTI_PR_PR3;      //Сбрасываем флаг прерывания перед включением самого прерывания
    EXTI->IMR |= EXTI_IMR_MR3;   //Включаем прерывание 3-го канала EXTI
    NVIC_EnableIRQ(EXTI3_IRQn);  //Разрешаем прерывание в контроллере прерываний

    //! enable Fan start signal
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    GPIOB->CRH &=~ (GPIO_CRH_MODE8 | GPIO_CRH_CNF8); //11: Output mode, max speed 50 MHz.
    GPIOB->CRH |= (GPIO_CRH_MODE8); //00:  output Push-pull
    GPIOB->ODR &=~ GPIO_ODR_ODR8;
}

extern "C" void EXTI3_IRQHandler()
{
    NVIC_DisableIRQ(EXTI3_IRQn); //! отключаем прерывание от кнопки
    TIM1->CR1 |= TIM_CR1_CEN; //! включаем таймер1 для отсчета задержки от дребезга по прерыванию которого включаем прерывание от кнопки
    EXTI->PR = EXTI_PR_PR3; //Сбрасываем флаг прерывания
    But::pThis->Encoder = true;
}

extern "C" void EXTI15_10_IRQHandler()
{    
    if(EXTI->PR&EXTI_PR_PR11)
    {
        NVIC_DisableIRQ(EXTI15_10_IRQn);//! отключаем прерывание от кнопки (прерывания не будет пока не отсчитает таймер 1)
        TIM1->CR1|=TIM_CR1_CEN; //! включаем таймер1 для отсчета задержки от дребезга по прерыванию которого включаем прерывание от кнопки
        //!clear interrupt flag 
        EXTI->PR = EXTI_PR_PR11;
        But::pThis->Minus = true;   
        NVIC_ClearPendingIRQ(EXTI15_10_IRQn);     
    }
    if(EXTI->PR&EXTI_PR_PR12)
    {
        NVIC_DisableIRQ(EXTI15_10_IRQn);//! отключаем прерывание от кнопки (прерывания не будет пока не отсчитает таймер 1)
        TIM1->CR1|=TIM_CR1_CEN; //! включаем таймер1 для отсчета задержки от дребезга по прерыванию которого включаем прерывание от кнопки
        //!clear interrupt flag 
        EXTI->PR = EXTI_PR_PR12;
        But::pThis->Plus = true;        
        NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
    }
}