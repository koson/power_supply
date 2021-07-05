#include "spi_slave.h"

SpiSlave* SpiSlave::pThis = nullptr;

SpiSlave::SpiSlave() {
    init();
    pThis = this;
}

void SpiSlave::setAnswerByte(uint8_t byte) {
    //while(!(SPI2->SR & SPI_SR_TXE));
    SPI2->DR = byte;
}

void SpiSlave::dmaOn() {
    DMA1_Channel4->CCR |= DMA_CCR4_EN; // enable DMA
    DMA1_Channel5->CCR |= DMA_CCR5_EN; // enable DMA
}
void SpiSlave::dmaOff() {
    DMA1_Channel4->CCR &= ~DMA_CCR4_EN; // disable DMA
    DMA1_Channel5->CCR &= ~DMA_CCR5_EN; // disable DMA
} 

void SpiSlave::init() {
    DMA_init();
    //!____ GPIO init B12_CS B13_CLK B14_MISO B15_MOSI_____________
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    //_____ B12 hardware NSS  input pull-up ___________
    GPIOB->CRH &= ~GPIO_CRH_MODE12; //0:0 input 
    GPIOB->CRH &= ~GPIO_CRH_CNF12_0;
    GPIOB->CRH |= GPIO_CRH_CNF12_1; // 1:0 - input with pull-up, pull-down
    GPIOB->ODR |= GPIO_ODR_ODR12; // pull-up

    //!____ B13 SCK input floating ____________________
    GPIOB->CRH &= ~GPIO_CRH_MODE13; //0:0 input 
    GPIOB->CRH |= GPIO_CRH_CNF13_0;
    GPIOB->CRH &= ~GPIO_CRH_CNF13_1; // 0:1 - input floating
    
    //!____ B14 MISO alt push-pull ____________________
    GPIOB->CRH |= GPIO_CRH_MODE14; //1:1 output max speed
    GPIOB->CRH &= ~GPIO_CRH_CNF14_0;
    GPIOB->CRH |= GPIO_CRH_CNF14_1; // 1:0 - alternate push pull
    
    //!____ B15 MOSI input_floating / input pull-up ____________________
    GPIOB->CRH &= ~GPIO_CRH_MODE15; //0:0 input 
    GPIOB->CRH &= ~GPIO_CRH_CNF15_0;
    GPIOB->CRH |= GPIO_CRH_CNF15_1; // 1:0 - alternate push pull
    GPIOB->ODR |= GPIO_ODR_ODR15; // pull-up

    //-------------  SPI-2  ---------------------------
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN; //clock on fast SPI-1
    //------------- предделитель SPI-1 -----------------------------
    SPI2->CR1 &= ~SPI_CR1_BR; // (000=>psc=1) 1:1:1  => 84000000/256 = 328.125 kHz - SPI-1 clk
    //------------- настройка SPI - 1 -----------------------------------------
    SPI2->CR1 &= ~SPI_CR1_BIDIMODE; // двухпроводной режим работы
    SPI2->CR1 &= ~SPI_CR1_BIDIOE;    //1: Output enabled (transmit-only mode)
    SPI2->CR1 &= ~SPI_CR1_CRCEN;    // 0: CRC calculation disabled
    SPI2->CR1 &= ~SPI_CR1_CRCNEXT;  //0: Data phase (no CRC phase)
    SPI2->CR1 &= ~SPI_CR1_DFF;      //0: 8-bit data frame format is selected for transmission/reception
    SPI2->CR1 &= ~SPI_CR1_RXONLY;   //0: Full duplex (Transmit and receive)
    SPI2->CR1 &= ~SPI_CR1_SSM;       // программное управление ведомым включено
    SPI2->CR1 &= ~SPI_CR1_SSI;       // нужно чтобы эти два бита были в 1 для правильной инициализации мастера
    SPI2->CR1 &= ~SPI_CR1_LSBFIRST; //1: LSB first //0: MSB transmitted first
    SPI2->CR1 &= ~SPI_CR1_MSTR;      //1: Master configuration
    SPI2->CR1 &= ~SPI_CR1_CPOL;      //1: CK to 1 when idle (смотреть в datasheet slave)
    SPI2->CR1 &= ~SPI_CR1_CPHA;      //1: The second clock transition is the first data capture edge (тоже)
    SPI2->CR2 |= SPI_CR2_RXNEIE; // receive not empty interrupt enable
    NVIC_EnableIRQ(SPI2_IRQn);
    //----------- включаем SPI-1 --------------------------------------------
    SPI2->CR1 |= SPI_CR1_SPE;
    setAnswerByte(99);
}

void SpiSlave::DMA_init() {
    //! Channel_4 for receiving SPI/I2S2_RX     
    DMA1_Channel4->CPAR = (uint32_t)(&SPI2->DR);  // peripheral address register
    DMA1_Channel4->CMAR = (uint32_t)(receiveBuf); // memory address register
    DMA1_Channel4->CNDTR = 4; // 4 bytes
    DMA1_Channel4->CCR &= ~DMA_CCR4_MEM2MEM; //disable memory to memory
    DMA1_Channel4->CCR |= DMA_CCR4_PL; // 1:1 Very high priority
    DMA1_Channel4->CCR &= ~DMA_CCR4_MSIZE; //0 - 8 bits memory size
    DMA1_Channel4->CCR &= ~DMA_CCR4_PSIZE; //0 - 8 bits peripheral data size
    DMA1_Channel4->CCR |= DMA_CCR4_MINC; // memory increment
    DMA1_Channel4->CCR &= ~DMA_CCR4_PINC; // 0 disable peripheral increment
    DMA1_Channel4->CCR |= DMA_CCR4_CIRC; // 1 enable circular
    DMA1_Channel4->CCR &= ~DMA_CCR4_DIR; // 0 read from peripheral (direction)
    DMA1_Channel4->CCR |= DMA_CCR4_TCIE; // transfer compete interrupt enable
    SPI2->CR2 |= SPI_CR2_RXDMAEN; // When this bit is set, the DMA request is made whenever the RXNE flag is set.
    NVIC_EnableIRQ(DMA1_Channel4_IRQn);
    //! Channel_5 for transmission SPI/I2S2_TX
    DMA1_Channel5->CPAR = (uint32_t)(&SPI2->DR);  // peripheral address register
    DMA1_Channel5->CMAR = (uint32_t)(transmitBuf); // memory address register
    DMA1_Channel5->CNDTR = 4; // 4 bytes
    DMA1_Channel5->CCR &= ~DMA_CCR5_MEM2MEM; //disable memory to memory
    DMA1_Channel5->CCR |= DMA_CCR5_PL; // 1:1 Very high priority
    DMA1_Channel5->CCR &= ~DMA_CCR5_MSIZE; //0 - 8 bits memory size
    DMA1_Channel5->CCR &= ~DMA_CCR5_PSIZE; //0 - 8 bits peripheral data size
    DMA1_Channel5->CCR |= DMA_CCR5_MINC; // memory increment
    DMA1_Channel5->CCR &= ~DMA_CCR5_PINC; // 0 disable peripheral increment
    DMA1_Channel5->CCR |= DMA_CCR5_CIRC; // 1 enable circular
    DMA1_Channel5->CCR |= DMA_CCR5_DIR; // 1 read from memory (direction)
    DMA1_Channel5->CCR |= DMA_CCR5_TCIE; // transfer compete interrupt enable

    SPI2->CR2 |= SPI_CR2_TXDMAEN; //When this bit is set, the DMA request is made whenever the TXE flag is set.
    //NVIC_EnableIRQ(DMA1_Channel5_IRQn);
}

extern "C" void SPI2_IRQHandler(void) {
    if(SPI2->SR & SPI_SR_OVR) {
        SpiSlave::pThis->sendBytes[0]=0x0;
        SpiSlave::pThis->sendBytes[1]=0x0;
        SpiSlave::pThis->sendBytes[3]=0x0;
        SpiSlave::pThis->sendBytes[4]=0x0;
        //(uint8_t)SPI2->DR;
    } else if(SPI2->SR & SPI_SR_RXNE) {
        //SpiSlave::pThis->receivedByte = SPI2->DR; 
        //while(!(SPI2->SR & SPI_SR_RXNE));
        // if(SpiSlave::pThis->bytesCounter > 3) {
        //    SpiSlave::pThis->bytesCounter = 0;
        //    SpiSlave::pThis->receivedFlag = true;
        // }        
        //SPI2->DR = SpiSlave::pThis->sendBytes[SpiSlave::pThis->bytesCounter];  
        //while(!(SPI2->SR & SPI_SR_TXE));
        //SpiSlave::pThis->receivedBytes[SpiSlave::pThis->bytesCounter++] = SpiSlave::pThis->receivedByte;                         
    }    
}

extern "C" void DMA1_Channel4_IRQHandler() {
    if (DMA1->ISR & DMA_ISR_TCIF4) {
        DMA1->IFCR |= DMA_IFCR_CTCIF4;
    }
    SpiSlave::pThis->receivedFlag = true;
}

extern "C" void DMA1_Channel5_IRQHandler() {
    if (DMA1->ISR & DMA_ISR_TCIF5) {
        DMA1->IFCR |= DMA_IFCR_CTCIF5;
    }
}