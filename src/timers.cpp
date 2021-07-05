#include "timers.h"

volatile bool Timers::timer_mSecFlag = false;

Timers::Timers(uint8_t i) {
    init(i);
}

uint16_t Timers::encoderValue() {
    return TIM3->CNT;
}

void Timers::checkValue() {
    uint32_t x = TIM3->CNT; 
    if(TIM3->CNT >= 99 && TIM3->CNT <= 150) {
        TIM3->CNT = 99;
    } else if (TIM3->CNT < 1 || TIM3->CNT >= 150 ) {
        TIM3->CNT = 1;
    }
}

//_________________________________ private _________________________________________________________
void Timers::init(uint8_t init) {
    switch(init) {
        case 1: 
            /*! **************** \brief TIMER_1 - time counting to 100 milisecond  **************************************/
            RCC->APB2ENR|=RCC_APB2ENR_TIM1EN; //тактирование на таймер  ("НА ТАЙМЕР ЧАСТОТА ПРИХОДИТ БОЛЬШАЯ В ДВА РАЗА")
            TIM1->PSC=64000; //0  //делить частоту шины apb1(64MHz*2 при SysClk -128MHz) на 64000 => частота 2kHz 
            TIM1->ARR=200-1; //считаем до 2000 => прерывание раз в 100 мс  period                 
            TIM1->CR1|=TIM_CR1_ARPE;  // задействуем регистр auto reload
	        TIM1->DIER|=TIM_DIER_UIE; //включаем прерывание по таймеру -   1: Update interrupt enabled.
            //TIM1->CR1|=TIM_CR1_CEN; //включаем таймер
	        NVIC_EnableIRQ(TIM1_UP_IRQn); //включаем обработку прерывания по таймеру 1
        break;
        case 2: {
             //**************  TIM2_ETR - PA15 ***********************************
            RCC->APB2ENR|=RCC_APB2ENR_IOPAEN;
            RCC->APB2ENR|=RCC_APB2ENR_AFIOEN;
            GPIOA->CRH&=~GPIO_CRH_MODE15;
            GPIOA->CRH|=GPIO_CRH_CNF15_0; // 0:1 floating input (reset state)
            AFIO->MAPR|=AFIO_MAPR_TIM2_REMAP; //full remap (CH1/ETR/PA15, CH2/PB3, CH3/PB10, CH4/PB11)
            //***************** TIMER_2 - master at external clock low two bytes ****************************
            RCC->APB1ENR|=RCC_APB1ENR_TIM2EN; //тактирование на таймер  ("НА ТАЙМЕР ЧАСТОТА ПРИХОДИТ БОЛЬШАЯ В ДВА РАЗА")
            TIM2->PSC=1-1; //0  //делить частоту шины apb1(36MHz*2 при SysClk -72MHz) на 36 => частота 2kHz 
            TIM2->SMCR|=TIM_SMCR_ECE; // external clock enable
            TIM2->CR2&=~TIM_CR2_MMS_2;
            TIM2->CR2|=TIM_CR2_MMS_1;
            TIM2->CR2&=~TIM_CR2_MMS_0; // 0:1:0 update
            TIM2->EGR|=TIM_EGR_UG; //update generation
            
            //TIM3->ARR=1000-1; //update mode
            TIM2->CR1|=TIM_CR1_ARPE;  // задействуем регистр auto reload
	        //TIM3->DIER|=TIM_DIER_UIE; //включаем прерывание по таймеру -   1: Update interrupt enabled.
            TIM2->CNT=0;
            TIM2->CR1|=TIM_CR1_CEN; //включаем таймер
	        //NVIC_EnableIRQ(TIM3_IRQn); //включаем обработку прерывания по таймеру 3
        }break;
        //!< encoder initialization
        case 3: {
            //!< PA6 PA7 ports initialization
            RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
            AFIO->MAPR |= AFIO_MAPR_TIM3_REMAP_NOREMAP;
            GPIOA->CRL |= GPIO_CRL_CNF6_1 | GPIO_CRL_CNF7_1; //!< 1:0 input mode 
            GPIOA->CRL &=~ (GPIO_CRL_CNF6_0 | GPIO_CRL_CNF7_0); //!< 1:0 input mode 
            GPIOA->CRL &=~ (GPIO_CRL_MODE6|GPIO_CRL_MODE7); //!< 0:0 input mode
            GPIOA->ODR |= (GPIO_ODR_ODR6 | GPIO_ODR_ODR7); //!< pull-up
            
           //******************** TIMER_3 -  **************************************
            RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; //тактирование на таймер  ("НА ТАЙМЕР ЧАСТОТА ПРИХОДИТ БОЛЬШАЯ В ДВА РАЗА")
            //!< 1:1:1:1  Fdts/32  (72MHz/32=2.25MHz) digital filter sampling >
            //TIM2->PSC=0xffff-1; //0  
            //TIM3->SMCR=0;
            TIM3->CCMR1 = 0;
            TIM3->CCER = 0;
            TIM3->CCMR1 |= (TIM_CCMR1_IC1F | TIM_CCMR1_IC2F); 
            TIM3->CCMR1 |= (TIM_CCMR1_CC1S_1 | TIM_CCMR1_CC2S_1); //!< enable Ti1 and Ti2
            TIM3->CCER &=~ (TIM_CCER_CC1P | TIM_CCER_CC2P); //!< capture on rising edge
            TIM3->CCER |= (TIM_CCER_CC2P ); //!< capture on rising edge
            TIM3->SMCR &=~ (TIM_SMCR_SMS_2 | TIM_SMCR_SMS_1 | TIM_SMCR_SMS_0);
            TIM3->SMCR |= (TIM_SMCR_SMS_1 | TIM_SMCR_SMS_0); //!< 0:1:1 encoder mode 3 up/down on two channels
            TIM3->ARR = 201-1; //считаем до 100 - max value
            TIM3->CCR1 = 100;   
            TIM3->EGR |= TIM_EGR_CC1G;
            TIM3->CCMR1 |= TIM_CCMR1_CC1S_1; // input on TI1              
            TIM3->DIER |= TIM_DIER_UIE; //включаем прерывание по таймеру -   1: Update interrupt enabled.
            TIM3->DIER |= TIM_DIER_CC1IE; // interrupt on CCR1 compare match
            //TIM3->CR1|=TIM_CR1_ARPE;  // задействуем регистр auto reload	            
            TIM3->CNT = 1;
            //NVIC_EnableIRQ(TIM3_IRQn);
            TIM3->CR1 |= TIM_CR1_CEN; //включаем таймер                
        }break;
        case 4: {
            //***************** TIMER_4 - time counting to 1 second  **************************************
            RCC->APB1ENR|=RCC_APB1ENR_TIM4EN; //тактирование на таймер  ("НА ТАЙМЕР ЧАСТОТА ПРИХОДИТ БОЛЬШАЯ В ДВА РАЗА")
            TIM4->PSC=3600; //0  //делить частоту шины apb1(64MHz*2 при SysClk -128MHz) на 64000 => частота 2kHz 
            TIM4->ARR=400-1; //считаем до 200 => прерывание раз в 100 мс  period                 
            TIM4->CR1|=TIM_CR1_ARPE;  // задействуем регистр auto reload
	        TIM4->DIER|=TIM_DIER_UIE; //включаем прерывание по таймеру -   1: Update interrupt enabled.
            TIM4->CR1|=TIM_CR1_CEN; //включаем таймер
	        NVIC_EnableIRQ(TIM4_IRQn); //включаем обработку прерывания по таймеру 3
        }break;
        default: break;
    }
}

extern "C" void TIM3_IRQHandler(void) {
    TIM3->SR &=~ TIM_SR_UIF; //!< скидываем флаг прерывания
    TIM3->SR &=~ TIM_SR_CC1IF;
    
}

extern "C" void TIM4_IRQHandler(void) //!обработчик прерывания раз в 1 с
{
	TIM4->SR &=~ TIM_SR_UIF; //!< скидываем флаг прерывания
	Timers::timer_mSecFlag=true;
    //low=TIM2->CNT;
	//high=TIM3->CNT;
	//TIM3->CNT=0;
	//TIM2->CNT=0;
    //freq=(high<<16|low);   //!< frequency accounting by summarizing two timers count registers 
	//NVIC_ClearPendingIRQ(TIM4_IRQn); //! скидываем флаг ожидания прерывания
}
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
/*! antirattle timer */
extern "C" void TIM1_UP_IRQHandler(void) //!обработчик прерывания раз в 100 ms
{
    TIM1->CR1 &=~ TIM_CR1_CEN; 
	TIM1->SR &=~ TIM_SR_UIF; // clear interrupt flag
    NVIC_EnableIRQ(EXTI15_10_IRQn);   //! PC11 PC12
    NVIC_EnableIRQ(EXTI3_IRQn);   //! PC11 PC12
	//NVIC_ClearPendingIRQ(TIM1_UP_IRQn); //! скидываем флаг ожидания прерывания, очищая бит NVIC_ICPRx (бит очереди)    
}