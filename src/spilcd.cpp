#include "spilcd.h"

uint16_t Spi1_interface::spi_read() {
    SPI1->DR = 0;                      //start exchange
    while (!(SPI1->SR & SPI_SR_RXNE)); 
    return SPI1->DR;                   
}

SpiLcd::SpiLcd() {
    spi1_ini(); tft_ini(240,240);
}

void SpiLcd::spi1_ini() {
    //-------------  RESET (display reset) PE2-------------------
    RCC->APB2ENR |= RCC_APB2ENR_IOPEEN;
    GPIOE->CRL |= GPIO_CRL_MODE2; // output mode 1:1 max speed
    GPIOE->CRL &= ~GPIO_CRL_CNF2_0;
    GPIOE->CRL &= ~GPIO_CRL_CNF2_1; // 0:0 - output push pull
    GPIOE->BSRR = 0x0000ffff;       // low level

    //-------------  DC (data command) PE3-------------------
    GPIOE->CRL |= GPIO_CRL_MODE3; // output mode 1:1 max speed
    GPIOE->CRL &= ~GPIO_CRL_CNF3_0;
    GPIOE->CRL &= ~GPIO_CRL_CNF3_1; // 0:0 - output push pull

    //-------------  CS (chip select NSS) PA4-------------------
    GPIOE->CRL |= GPIO_CRL_MODE4; // output mode 1:1 max speed
    GPIOE->CRL &= ~GPIO_CRL_CNF4_0;
    GPIOE->CRL &= ~GPIO_CRL_CNF4_1; // 0:0 - output push pull

    //-------------  SPI:  PB3-_SCK, PB4-MISO(not realized), PB5-MOSI :::AF5
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    AFIO->MAPR |= AFIO_MAPR_SPI1_REMAP;
    GPIOB->CRL |= GPIO_CRL_MODE3; // output mode 1:1 max speed
    GPIOB->CRL &= ~GPIO_CRL_CNF3_0;
    GPIOB->CRL |= GPIO_CRL_CNF3_1; // 1:0 - alternate push pull
    GPIOB->CRL |= GPIO_CRL_MODE5;  // output mode 1:1 max speed
    GPIOB->CRL &= ~GPIO_CRL_CNF5_0;
    GPIOB->CRL |= GPIO_CRL_CNF5_1; // 1:0 - alternate push pull

    //------------- SPI-1  ---------------------------
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN; //clock on fast SPI-1

    SPI1->CR1 &= ~SPI_CR1_BR; // (000=>psc=1) 1:1:1  => 84000000/256 = 328.125 kHz - SPI-1 clk
    SPI1->CR1 &= ~SPI_CR1_BIDIMODE; // 
    SPI1->CR1 |= SPI_CR1_BIDIOE;    //1: Output enabled (transmit-only mode)
    SPI1->CR1 &= ~SPI_CR1_CRCEN;    // 0: CRC calculation disabled
    SPI1->CR1 &= ~SPI_CR1_CRCNEXT;  //0: Data phase (no CRC phase)
    SPI1->CR1 &= ~SPI_CR1_DFF;      //0: 8-bit data frame format is selected for transmission/reception
    SPI1->CR1 &= ~SPI_CR1_RXONLY;   //0: Full duplex (Transmit and receive)
    SPI1->CR1 |= SPI_CR1_SSM;       // Software slave management
    SPI1->CR1 |= SPI_CR1_SSI;       // Internal slave select. The value of this bit is forced onto the NSS pin and the I/O value of the NSS pin is ignored 
    SPI1->CR1 &= ~SPI_CR1_LSBFIRST; //1: LSB first //0: MSB transmitted first
    SPI1->CR1 |= SPI_CR1_MSTR;      //1: Master configuration
    SPI1->CR1 |= SPI_CR1_CPOL;      //1: CK to 1 when idle (in datasheet slave)
    SPI1->CR1 |= SPI_CR1_CPHA;      //1: The second clock transition is the first data capture edge (тоже)
    //NVIC_EnableIRQ(SPI1_IRQn);
    SPI1->CR1 |= SPI_CR1_SPE;
}

void SpiLcd::tft_ini(uint16_t w_size, uint16_t h_size) {
    reset_active();
    delay(36000);
    reset_idle();
    ST7789_SendCommand(0x01);
    delay(36000);
    ST7789_SendCommand(0x11); //SLPOUT sleep out
    delay(36000);             // 100 ms
    //---------------------------------------------------
    ST7789_SendCommand(0x3A); //COLMOD RGB444(12bit) 0x03, RGB565(16bit) 0x05,
    ST7789_SendData(0x05);    //RGB666(18bit) 0x06
    //---------------------------------------------------
    //Memory Acsess Control - rotation
    ST7789_SendCommand(0x36); //Memory Data Access Control
    ST7789_SendData(0x00);    // 1-полубайт ориентация (через 2) - 2-ой цветовая схема (0 или 8)
    
    ST7789_SendCommand(0x21); //inv on
    ST7789_SendCommand(0x13); //Normal display Mode on
    //delay(3600000); //100 ms

    ST7789_SendCommand(0x29); //display on
    //ST7789_SendData(TFT9341_ROTATION);

    delay(3600000); //100 ms
}

Font_interface::Font_interface() {}

uint32_t Font_interface::char_to_int(char* str,uint8_t size) {
    uint32_t x;
    for(uint8_t i=0;i<size;i++) {
        uint8_t dec;
        if (str[i]==48){dec=0;} if (str[i]==49){dec=1;}
        if (str[i]==50){dec=2;} if (str[i]==51){dec=3;}
        if (str[i]==52){dec=4;} if (str[i]==53){dec=5;}
        if (str[i]==54){dec=6;} if (str[i]==55){dec=7;}
        if (str[i]==56){dec=8;} if (str[i]==57){dec=9;}
        x+=dec*pow(10,size-i);           
    }
    return x;
}
void Font_interface::intToChar(uint32_t x) {
    uint32_t y=x;        
    uint8_t count=0;
    while (y>0)
    {  y=y/10;  count++; }//считаем количество цифр
    y=x;
    arrSize=count;
    if(x==0) {arrSize=1;arr[arrSize-1]='0';arr[arrSize]='\0';return;} 
    for(uint8_t i=0;i<arrSize;i++) {            
        int x=y%10;
        if(x==0) {arr[arrSize-1-i]='0';} if(x==1) {arr[arrSize-1-i]='1';}
        if(x==2) {arr[arrSize-1-i]='2';} if(x==3) {arr[arrSize-1-i]='3';}
        if(x==4) {arr[arrSize-1-i]='4';} if(x==5) {arr[arrSize-1-i]='5';}
        if(x==6) {arr[arrSize-1-i]='6';} if(x==7) {arr[arrSize-1-i]='7';}
        if(x==8) {arr[arrSize-1-i]='8';} if(x==9) {arr[arrSize-1-i]='9';}
        y=y/10;        
    }
    if(arrSize+1<10) {
        //strcat(arr+arrSize,'\0'); 
        arr[arrSize]='\0';
    }
}
void Font_interface::floatTochar(float x) {        
    sprintf(arrFloat, "%.3f", x);           
}


Font_8x8::Font_8x8(){}

void Font_8x8::symbol(uint16_t x, uint16_t y, uint16_t col,const uint8_t* data) {
    tft_setColumn(x,x+8);
    tft_setRow(y,y+8);
    ST7789_SendCommand(0x2C);
    for(uint16_t i=0; i <y+8-y;i++) {
        for(uint16_t j=0; j <x+8-x+1;j++) {
            if((j==0)&&(data[i]&0x80)) {spi_sendWord(col);}//else spi_sendWord(0xF800);
            else if((j==1)&&(data[i]&0x40)) {spi_sendWord(col);}//else spi_sendWord(0xF800);
            else if((j==2)&&(data[i]&0x20)) {spi_sendWord(col);}//else spi_sendWord(0xF800);
            else if((j==3)&&(data[i]&0x10)) {spi_sendWord(col);}//else spi_sendWord(0xF800);
            else if((j==4)&&(data[i]&0x08)) {spi_sendWord(col);}//else spi_sendWord(0xF800);
            else if((j==5)&&(data[i]&0x04)) {spi_sendWord(col);}//else spi_sendWord(0xF800);
            else if((j==6)&&(data[i]&0x02)) {spi_sendWord(col);}//else spi_sendWord(0xF800);
            else if((j==7)&&(data[i]&0x01)) {spi_sendWord(col);}
            else spi_sendWord(0x0000);
        }
    }
}

void Font_8x8::print(uint16_t x, uint16_t y,uint16_t col,char* str) {
    for(uint8_t i=0;i<strlen(str);i++) {
        if (str[i]==48){symbol(x, y, col, data0);}
        if (str[i]==49){symbol(x, y, col, data1);}
        if (str[i]==50){symbol(x, y, col, data2);}
        if (str[i]==51){symbol(x, y, col, data3);}
        if (str[i]==52){symbol(x, y, col, data4);}
        if (str[i]==53){symbol(x, y, col, data5);}
        if (str[i]==54){symbol(x, y, col, data6);}
        if (str[i]==55){symbol(x, y, col, data7);}
        if (str[i]==56){symbol(x, y, col, data8);}
        if (str[i]==57){symbol(x, y, col, data9);}
        x+=9;            
    }
}    

Font_16x16::Font_16x16(){}

void Font_16x16::symbol(uint16_t x, uint16_t y, uint16_t col,const uint16_t* data) {        
    tft_setColumn(x,x+16);
    tft_setRow(y,y+16);
    ST7789_SendCommand(0x2C);
    for(uint16_t i=0; i <y+16-y;i++) {
        //spi_sendWord(col);
        for(uint16_t j=0; j <x+16-x+1;j++) {
            if((j==0)&&(data[i]&0x8000)) {spi_sendWord(col);}
            else if((j==1)&&(data[i]&0x4000)) {spi_sendWord(col);}
            else if((j==2)&&(data[i]&0x2000)) {spi_sendWord(col);}
            else if((j==3)&&(data[i]&0x1000)) {spi_sendWord(col);}
            else if((j==4)&&(data[i]&0x0800)) {spi_sendWord(col);}
            else if((j==5)&&(data[i]&0x0400)) {spi_sendWord(col);}
            else if((j==6)&&(data[i]&0x0200)) {spi_sendWord(col);}
            else if((j==7)&&(data[i]&0x0100)) {spi_sendWord(col);}
            else if((j==8)&&(data[i]&0x0080)) {spi_sendWord(col);}
            else if((j==9)&&(data[i]&0x0040)) {spi_sendWord(col);}
            else if((j==10)&&(data[i]&0x0020)) {spi_sendWord(col);}
            else if((j==11)&&(data[i]&0x0010)) {spi_sendWord(col);}
            else if((j==12)&&(data[i]&0x0008)) {spi_sendWord(col);}
            else if((j==13)&&(data[i]&0x0004)) {spi_sendWord(col);}
            else if((j==14)&&(data[i]&0x0002)) {spi_sendWord(col);}
            else if((j==15)&&(data[i]&0x0001)) {spi_sendWord(col);}
            else spi_sendWord(0x0000);
        }
    }       
}

void Font_16x16::clearString(uint16_t x, uint16_t y,uint8_t size) {
    tft_setColumn(x,x+16*size);
    tft_setRow(y,y+16);
    ST7789_SendCommand(0x2C);
    for(uint16_t i=0; i <(y+16+1)*(x+16+1);i++) {
        spi_sendWord(0x0000);
    }
}

void Font_16x16::print(uint16_t x, uint16_t y,uint16_t col,const char* str,uint8_t numDigits) {        
    for(uint8_t i=0;i<strlen(str);i++) {
        if (str[i]==48){symbol(x, y, col, data0);}
        if (str[i]==49){symbol(x, y, col, data1);}
        if (str[i]==50){symbol(x, y, col, data2);}
        if (str[i]==51){symbol(x, y, col, data3);}
        if (str[i]==52){symbol(x, y, col, data4);}
        if (str[i]==53){symbol(x, y, col, data5);}
        if (str[i]==54){symbol(x, y, col, data6);}
        if (str[i]==55){symbol(x, y, col, data7);}
        if (str[i]==56){symbol(x, y, col, data8);}
        if (str[i]==57){symbol(x, y, col, data9);}
        if (str[i]==58){symbol(x, y, col, dataColon);}
        if (str[i]==46){symbol(x, y, col, dataPoint);}
        if (str[i]==0x41){symbol(x, y, col, dataAmper);}
        if (str[i]==0x56){symbol(x, y, col, dataVolt);}
        else{x+=17;}                        
    }
    for(uint8_t i=strlen(str);i<strlen(str)+numDigits-1;i++) {
        {symbol(x, y, col, dataSpace);}
        x+=17;            
    }
}