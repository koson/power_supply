#include "uart.hpp"

void USART1_IRQHandler() {
    InterruptManager::Call(USART1_IRQn);
}
void USART2_IRQHandler() {
    InterruptManager::Call(USART2_IRQn);
}



Uart* Uart::pThis = {nullptr};

Uart::Uart(uint8_t num):Interruptable() {
    uart_init(num);
    UartNum = num;
    pThis=this;
    InterruptHandle();
}

void Uart::sendByte(uint8_t byte) {
    if(UartNum == 1) {
        while(!(USART1->SR&USART_SR_TC)) {}
        USART1->DR = byte;
    }
    else if (UartNum == 2) {
        while(!(USART2->SR&USART_SR_TC)) {}
        USART2->DR = byte;
    }
    else if (UartNum == 3) {
        while(!(USART3->SR&USART_SR_TC)) {}
        USART3->DR = byte;
    }         
}

void Uart::sendStr(const char* str) {
    uint8_t len = 0;
    while(str[len]!='\0') {
        sendByte(str[len++]);
    }
}

void Uart::readByte1() {
    //USART1->SR&=~USART_SR_RXNE; // скидываем флаг
    receivedArr[0] = USART1->DR;    
    wasInterruptedby1 = true;
}
void Uart::readByte2() {
    //USART2->SR&=~USART_SR_RXNE; // скидываем флаг
    receivedArr[0] = USART2->DR;
    wasInterruptedby2 = true;
}
void Uart::InterruptHandle() {
    if(UartNum == 1) {
        InterruptManager::AddHandler(setUart1,USART1_IRQn);
    }
    else if (UartNum == 2) {
        InterruptManager::AddHandler(setUart2,USART2_IRQn);
    }
}

void Uart::setUart1() {
    pThis->readByte1();
}
void Uart::setUart2() {
    pThis->readByte2();
}

void Uart::uart_init(uint8_t num) {
    switch(num) {
        case 1: {
            // настраиваем пины
            RCC->APB2ENR|=RCC_APB2ENR_IOPAEN; // тактирование на порт А
            //Tx portA-9
            GPIOA->CRH|=GPIO_CRH_MODE9; //11: Output mode, max speed 50 MHz.
            GPIOA->CRH|=GPIO_CRH_CNF9_1;
            GPIOA->CRH&=~GPIO_CRH_CNF9_0; //10: Alternate function output Push-pull
            //Rx portA-10
            GPIOA->CRH&=~GPIO_CRH_MODE10; //00: Input mode (reset state).
            GPIOA->CRH|=GPIO_CRH_CNF10_0;
            GPIOA->CRH&=~GPIO_CRH_CNF10_1; //01: Floating input (reset state)
            //настраиваем альтернативные пины usart1
            RCC->APB2ENR|=RCC_APB2ENR_USART1EN; //подаем тактирование
            AFIO->MAPR&=~AFIO_MAPR_USART1_REMAP; // 0: No remap (TX/PA9, RX/PA10)
            AFIO->EXTICR[2]|=AFIO_EXTICR3_EXTI10_PA; //0000: PA[x] pin
            AFIO->EXTICR[2]|=AFIO_EXTICR3_EXTI9_PA;  //0000: PA[x] pin
            USART1->CR1|=USART_CR1_UE; //USART EN
            USART1->CR1&=~USART_CR1_M; //0 - 8 Data bits
            USART1->CR1|=USART_CR1_TE; // Transmission enabled
            USART1->CR1|=USART_CR1_RE; // Recieving enabled
            USART1->CR1|=USART_CR1_RXNEIE; //enable interrupt on Rx from usart1
            //USART1->BRR=0x445C; //2400 (APB1CLK - baudrate/2)/baudrate
            //USART1->BRR=0xEA6; //9600    (36000000-4800)/9600
            //USART1->BRR=0x1D4C; //9600    (72000000-4800)/9600
	        USART1->BRR=0x270; //115200 (72000000 - 57600)/115200
            NVIC_EnableIRQ(USART1_IRQn);
        } 
        break;
        case 2: {
            // настраиваем пины USART 2
            RCC->APB2ENR|=RCC_APB2ENR_IOPDEN; // тактирование на порт B
            RCC->APB2ENR|=RCC_APB2ENR_AFIOEN;
            //Tx portD-5
            GPIOD->CRL|=GPIO_CRL_MODE5; //11: Output mode, max speed 50 MHz.
            GPIOD->CRL|=GPIO_CRL_CNF5_1;
            GPIOD->CRL&=~GPIO_CRL_CNF5_0; //10: Alternate function output Push-pull
            //Rx portD-6
            GPIOD->CRL&=~GPIO_CRL_MODE6; //00: Input mode (reset state).
            GPIOD->CRL|=GPIO_CRL_CNF6_0;
            GPIOD->CRL&=~GPIO_CRL_CNF6_1; //01: Floating input (reset state)
            //настраиваем альтернативные пины usart1
            RCC->APB1ENR|=RCC_APB1ENR_USART2EN; //подаем тактирование
            AFIO->MAPR|=AFIO_MAPR_USART2_REMAP; // 1: No remap (TX/PD5, RX/PD6)
            //AFIO->EXTICR[2]|=AFIO_EXTICR3_EXTI10_PA; //0000: PA[x] pin
            //AFIO->EXTICR[2]|=AFIO_EXTICR3_EXTI9_PA;  //0000: PA[x] pin
            
            USART2->CR1&=~USART_CR1_M; //0 - 8 Data bits
            USART2->CR1|=USART_CR1_TE; // Transmission enabled
            USART2->CR1|=USART_CR1_RE; // Recieving enabled
            USART2->CR1|=USART_CR1_RXNEIE; //enable interrupt on Rx from usart1
            //USART2->BRR=0x445C; //2400 (APB1CLK - baudrate/2)/baudrate
            USART2->BRR=0xEA5; //9600    (36000000-4800)/9600
            //USART2->BRR=0x1D4C; //9600    (72000-4800)/9600
	        //USART2->BRR=0x138; //115200         
            USART2->CR1|=USART_CR1_UE; //USART EN       
            NVIC_EnableIRQ(USART2_IRQn);
        }
        break;
        case 3: {
            // настраиваем пины USART 2
            RCC->APB2ENR|=RCC_APB2ENR_IOPBEN; // тактирование на порт B
            //Tx portB-10
            GPIOB->CRH|=GPIO_CRH_MODE10; //11: Output mode, max speed 50 MHz.
            GPIOB->CRH|=GPIO_CRH_CNF10_1;
            GPIOB->CRH&=~GPIO_CRH_CNF10_0; //10: Alternate function output Push-pull
            //Rx portB-11
            GPIOB->CRH&=~GPIO_CRH_MODE11; //00: Input mode (reset state).
            GPIOB->CRH|=GPIO_CRH_CNF11_0;
            GPIOB->CRH&=~GPIO_CRH_CNF11_1; //01: Floating input (reset state)
            //настраиваем альтернативные пины usart1
            RCC->APB1ENR|=RCC_APB1ENR_USART3EN; //подаем тактирование
            //AFIO->MAPR|=AFIO_MAPR_USART2_REMAP; // 1: No remap (TX/PD5, RX/PD6)
            //AFIO->EXTICR[2]|=AFIO_EXTICR3_EXTI10_PA; //0000: PA[x] pin
            //AFIO->EXTICR[2]|=AFIO_EXTICR3_EXTI9_PA;  //0000: PA[x] pin
            
            USART3->CR1&=~USART_CR1_M; //0 - 8 Data bits
            USART3->CR1|=USART_CR1_TE; // Transmission enabled
            USART3->CR1|=USART_CR1_RE; // Recieving enabled
            USART3->CR1|=USART_CR1_RXNEIE; //enable interrupt on Rx from usart1
            //USART2->BRR=0x445C; //2400 (APB1CLK - baudrate/2)/baudrate
            USART3->BRR=0xEA5; //9600    (36000000-4800)/9600
            //USART2->BRR=0x1D4C; //9600    (72000-4800)/9600
	        //USART2->BRR=0x138; //115200         
            USART3->CR1|=USART_CR1_UE; //USART EN       
            NVIC_EnableIRQ(USART3_IRQn);
        }
        break;
    }        
}