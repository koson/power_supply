#ifndef SPILCD_H_
#define SPILCD_H_

#include "main.h"
#include <cmath>
#include "string.h"
#include "stdio.h"

class Spi1_interface
{
public:
    inline void reset_active(){GPIOE->BSRR|=GPIO_BSRR_BR2;}
    inline void reset_idle(){GPIOE->BSRR|=GPIO_BSRR_BS2;}
    inline void cs_active(){GPIOE->BSRR|=GPIO_BSRR_BR4;}
    inline void cs_idle(){GPIOE->BSRR|=GPIO_BSRR_BS4;}
    inline void dc_command(){GPIOE->BSRR|=GPIO_BSRR_BR3;}
    inline void dc_data(){GPIOE->BSRR|=GPIO_BSRR_BS3;}
    
    inline void ST7789_SendCommand(uint8_t cmd) {
        while((SPI1->SR&SPI_SR_BSY)){}
        dc_command();
        spi_sendByte(cmd);  
        delay(40);
    }
    inline void ST7789_SendData(uint8_t data) {        
        dc_data();
        spi_sendByte(data);
    }    
    inline void spi_sendByte(uint8_t data) {
        while(!(SPI1->SR&SPI_SR_TXE)){}        
        SPI1->DR=data;
        while((SPI1->SR&SPI_SR_BSY)){}
    }
    inline void spi_sendWord(uint16_t data) {
        while((SPI1->SR&SPI_SR_BSY)){}
        dc_data();
        ST7789_SendData(data>>8);
        while((SPI1->SR&SPI_SR_BSY)){}
        ST7789_SendData(data&0x00FF);
    }
    inline void tft_setColumn(int StartCol, int EndCol) {
        ST7789_SendCommand(0x2A);                                                     // Column Command address
        spi_sendWord(StartCol);
        spi_sendWord(EndCol);
    }
    inline void tft_setRow(int StartRow, int EndRow) {
        ST7789_SendCommand(0x2B);                                                  // Column Command address
        spi_sendWord(StartRow);
        spi_sendWord(EndRow);
    }
    inline void tft_setXY(int poX, int poY) {
        tft_setColumn(poX, poX);
        tft_setRow(poY, poY);
        ST7789_SendCommand(0x2C);
    }
    inline void tft_setPixel(int poX, int poY, int color) {
        tft_setXY(poX, poY);
        spi_sendWord(color);
        spi_sendWord(color);
    }
    inline void swap(uint16_t x1, uint16_t x2) {
        if(x2>x1){uint16_t z=x2;x2=x1;x1=z;}
    }
    inline void checkXYswap(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2) {
        swap(x1,x2); swap(y1,y2);
    }    
    inline void fillScreen(uint16_t color) {
        tft_setRow(0, 240);
        tft_setColumn(0,240);        
        ST7789_SendCommand(0x2C); //memory write
        for(volatile uint32_t i=0; i<57600; i++) {           
            spi_sendWord(color);           
        }
    }        
    inline void delay(volatile uint32_t x) {
        for(volatile uint32_t i=0;i<x;i++){;}
    }
    uint16_t spi_read();
};

class SpiLcd:public Spi1_interface
{
public:
    SpiLcd();

private:
    void spi1_ini();
     void tft_ini(uint16_t w_size, uint16_t h_size);
};
// ///////////////////////////////////////////////////////////////////////////
//*********** FONT IMTERFACE ********************
class Font_interface:public Spi1_interface
{
public:
     enum col
    {
        RED        =    0xf800,
        GREEN      =    0x07e0,
        BLUE       =    0x001f,
        BLACK      =    0x0000,
        YELLOW     =    0xffe0,
        WHITE      =    0xffff,
        CYAN       =    0x07ff,
        BRIGHT_RED =    0xf810,
        GRAY1      =    0x8410,
        GRAY2      =    0x4208
    };
public:
    Font_interface();
    uint32_t char_to_int(char* str,uint8_t size);
    char arr[20];
    char arrFloat[20]{0};
    volatile uint8_t arrSize=0;

    void intToChar(uint32_t x);
    void floatTochar(float x);
};
class Font_8x8: public Font_interface
{
public:
    Font_8x8();
    void symbol(uint16_t x, uint16_t y, uint16_t col,const uint8_t* data);    
    void print(uint16_t x, uint16_t y,uint16_t col,char* str);
    const uint8_t data0[8]={0x7C,0xFE,0xC6,0xC6,0xC6,0xC6,0xFE,0x7C}; //0         
    const uint8_t data1[8]={0x30,0x70,0xF0,0x30,0x30,0x30,0x30,0x30}; //1         
    const uint8_t data2[8]={0x78,0xFC,0xCC,0x1C,0x38,0x70,0xFC,0xFC}; //2         
    const uint8_t data3[8]={0x78,0xFC,0x0C,0x78,0xFC,0x0C,0xFC,0x78}; //3         
    const uint8_t data4[8]={0x1C,0x3C,0x6C,0xCC,0xFC,0x0C,0x0C,0x0C}; //4         
    const uint8_t data5[8]={0xFC,0xC0,0xC0,0xF8,0x0C,0xCC,0xFC,0x78}; //5         
    const uint8_t data6[8]={0x7E,0xC6,0xC0,0xFC,0xC6,0xC6,0xFE,0x7C}; //6         
    const uint8_t data7[8]={0xFE,0xFE,0x06,0x0E,0x1C,0x38,0x70,0x60}; //7         
    const uint8_t data8[8]={0x7C,0xFE,0xC6,0x7C,0x7C,0xC6,0xFE,0x7C}; //8         
    const uint8_t data9[8]={0x7C,0xFE,0xC6,0x7E,0x06,0x86,0xFE,0x7C}; //9 
};

class Font_16x16: public Font_interface
{
public:
    Font_16x16();
    const char strArr[10]="opa";
    void symbol(uint16_t x, uint16_t y, uint16_t col,const uint16_t* data);    
    void clearString(uint16_t x, uint16_t y,uint8_t size);
    void print(uint16_t x, uint16_t y,uint16_t col,const char* str,uint8_t numDigits);

    const uint16_t data0[16]={0x0FE0,0x1FF0,0x3FF8,0x7C7C,0x783C,0xF01E,0xF01E,0xF01E,          
                              0xF01E,0xF01E,0x783C,0x7C7C,0x3FF8,0x1FF0,0x0FE0,0x0000}; //0         
    const uint16_t data1[16]={0x07F8,0x0FF8,0x1FF8,0x3FF8,0x7FF8,0xFFF8,0xFDF8,0xF9F8,          
                              0x01F8,0x01F8,0x01F8,0x01F8,0x01F8,0x01F8,0x01F8,0x0000}; //1         
    const uint16_t data2[16]={0x3FF0,0x7FF8,0xFFFC,0xF0FC,0xF0FC,0xE1F8,0x03F0,0x07E0,         
                              0x0FC0,0x1F80,0x3F00,0x7E00,0xFFFC,0xFFFC,0xFFFC,0x0000}; //2         
    const uint16_t data3[16]={0x3FF0,0x7FF8,0xFFFC,0xE07C,0x003C,0x007C,0x3FF8,0x7FF0,          
                              0x3FF8,0x007C,0x003C,0xE07C,0xFFFC,0x7FF8,0x3FF0,0x0000}; //3         
    const uint16_t data4[16]={0x03FC,0x07FC,0x0FFC,0x1E7C,0x3C7C,0x787C,0xF07C,0xFFFC,          
                              0xFFFC,0xFFFC,0x007C,0x007C,0x007C,0x007C,0x007C,0x0000}; //4         
    const uint16_t data5[16]={0xFFFC,0xFFFC,0xFFFC,0xE000,0xE000,0xFFE0,0xFFF0,0xFFF8,          
                              0x007C,0x003C,0xE03C,0xF07C,0xFFF8,0x7FF0,0x3FE0,0x0000}; //5         
    const uint16_t data6[16]={0x3FF8,0x7FFC,0xFFFC,0xF03C,0xE000,0xF000,0xFFF0,0xFFF8,          
                              0xFFFC,0xF03C,0xE01C,0xF03C,0xFFF8,0x7FF0,0x3FE0,0x0000}; //6         
    const uint16_t data7[16]={0x7FFC,0xFFFC,0xFFFC,0xFFFC,0x003C,0x007C,0x00FC,0x01F8,          
                              0x03F0,0x07E0,0x0FC0,0x1F80,0x3F00,0x7E00,0xFC00,0x0000}; //7         
    const uint16_t data8[16]={0x1FE0,0x3FF0,0x7FF8,0xF87C,0xF03C,0xF87C,0x7FF8,0x3FF0,          
                              0x7FF8,0xF87C,0xF03C,0xF87C,0x7FF8,0x3FF0,0x1FE0,0x0000}; //8         
    const uint16_t data9[16]={0x1FE0,0x3FF0,0x7FF8,0xF03C,0xF03C,0xF03C,0xFFFC,0x7FFC,          
                              0x3FFC,0x003C,0xE07C,0xF0F8,0xFFF0,0x7FE0,0x3FC0,0x0000}; //9
    const uint16_t dataColon[16]={0x0000,0x0000,0x0780,0x0780,0x0780,0x0780,0x0000,0x0000,          
                              0x0000,0x0780,0x0780,0x0780,0x0780,0x0000,0x0000,0x0000};  //:  
    const uint16_t dataSpace[16]={0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,          
                              0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000};  //Space  
    const uint16_t dataPoint[16]={0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,          
                              0x0000,0x0000,0x0000,0x03C0,0x03C0,0x03C0,0x03C0,0x0000};  //.
    const uint16_t dataVolt[16]={0x0000,0x783C,0x3838,0x3C78,0x1C70,0x1C70,0x1C70,0x0EE0,          
                              0x0EE0,0x0EE0,0x07C0,0x07C0,0x07C0,0x0380,0x0380,0x0000};  //V  
    const uint16_t dataAmper[16]={0x0000,0x0000,0x03C0,0x03C0,0x07E0,0x07E0,0x07E0,0x0E70,          
                              0x0E70,0x0E70,0x1FF8,0x01FF8,0x1C38,0x3E3E,0x381E,0x0000};  //A                            
};
class Time:Font_16x16
{
public:
    Time(){}
    static volatile bool colon;
    void printTime(uint16_t x, uint16_t y,uint16_t col,uint8_t hour, uint8_t min, uint8_t sec)
    {
        if(Time::colon){Time::colon=false;}
        else {Time::colon=true;}
        if(hour<10)    // печатаем часы
        {
            intToChar(hour);
            print(x,y,col,"0",1);
            print(x+17,y,col,arr,1);
        }
        else
        {
            intToChar(hour);
            print(x,y,col,arr,2);
        }
        if(colon){print(x+17*2,y,col,":",1);}
        else{symbol(x+17*2,y,col,dataSpace);}
        

        if(min<10)      // печатаем минуты      
        {
            intToChar(min);
            print(x+17*3,y,col,"0",1);
            print(x+17*4,y,col,arr,1);
        }
        else
        {
            intToChar(min);
            print(x+17*3,y,col,arr,2);
        }
        if(colon){print(x+17*5,y,col,":",1);}
        else{symbol(x+17*5,y,col,dataSpace);}
        

        if(sec<10)       //печатаем секунды
        {
            intToChar(sec);
            print(x+17*6,y,col,"0",1);
            print(x+17*7,y,col,arr,1);
        }
        else
        {
            intToChar(sec);
            print(x+17*6,y,col,arr,2);
        }
    }
};

class Pixel:public Spi1_interface
{
public:
    Pixel(uint16_t x=0,uint16_t y=0,uint16_t col=BLACK)
    {
        //setPixel(x,y,col);  
    }
    enum col
    {
        RED        =    0xf800,
        GREEN      =    0x07e0,
        BLUE       =    0x001f,
        BLACK      =    0x0000,
        YELLOW     =    0xffe0,
        WHITE      =    0xffff,
        CYAN       =    0x07ff,
        BRIGHT_RED =    0xf810,
        GRAY1      =    0x8410,
        GRAY2      =    0x4208
    };

    void setPixel(uint16_t X, uint16_t Y, uint16_t color)
    {
        tft_setXY(X, Y);
        spi_sendWord(color);
        //spi_sendWord(color);
    }
};
class Rect:public Spi1_interface//, public Pixel
{
public:
    enum col
    {
        RED        =    0xf800,
        GREEN      =    0x07e0,
        BLUE       =    0x001f,
        BLACK      =    0x0000,
        YELLOW     =    0xffe0,
        WHITE      =    0xffff,
        CYAN       =    0x07ff,
        BRIGHT_RED =    0xf810,
        GRAY1      =    0x8410,
        GRAY2      =    0x4208
    };
public: 
    Rect(uint16_t x1=0,uint16_t x2=0,uint16_t y1=0,uint16_t y2=0,uint16_t col=BLACK)
    {        
    }   
    void setRect(uint16_t x1, uint16_t y1, uint16_t x2,uint16_t y2,uint16_t col) 
    {
        checkXYswap(x1,y1,x2,y2);
        tft_setColumn(x1,x2);
        tft_setRow(y1,y2);
        ST7789_SendCommand(0x2C);
        for(uint32_t i=0; i<=uint32_t((x2-x1+1)*(y2-y1+1));i++)
        {
            spi_sendWord(col);
        }
    }
};
class Line:public Pixel
{
public:
    Line(uint16_t x1=0,uint16_t y1=0,uint16_t x2=0,uint16_t y2=0,uint16_t col=BLACK)
    {
        //SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));  /* set CP10 and CP11 Full Access */ //FPU enable
        //checkXYswap(x1,y1,x2,y2);
        //if(x2!=x1)
        //{
        //    angle = atanf((y2-y1)/(x2-x1));
        //}
        //else if((x2==x1)&&y2!=y1)
        //{
        //    angle=90.0f*3.14159265f/90.0f ;
        //}
        //else {angle=0;}        
        length = float(sqrtf((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1))); //single float
        
        for(uint16_t i=0;i<uint32_t(length);i++) //uint16_t(length)
        {     
            //setPixel(x1+i,y1+sqrtf(i),col);
            setPixel(x1+uint16_t(i*((x2-x1)/length)),y1+uint16_t(i*((y2-y1)/length)),col);            
        }
    }
    void setLine(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t col)
    {
        float length1 = sqrtf((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
        for(uint16_t i=0;i<uint32_t(length1);i++) //uint16_t(length)
        {     
            //setPixel(x1+i,y1+sqrtf(i),col);       
            setPixel(x1+i*(x2-x1)/length1,y1+i*(y2-y1)/length1,col);            
        }
    }
    void setDiscretLine(uint16_t X1,uint16_t Y1,uint16_t X2,uint16_t Y2,uint16_t col)
    {
        volatile uint16_t x1=X1; 
        volatile uint16_t y1=Y1;
        volatile uint16_t x2=X2;
        volatile uint16_t y2=Y2;   
        volatile uint16_t step = std::abs(y2-y1)>std::abs(x2-x1);
        if(step){swap(&x1,&y1);swap(&x2,&y2);} //меняем систему координат и отсчитываем в реальности по y
        //volatile uint16_t dx;
        //volatile uint16_t dy; 
        //dx=x2-x1; dy=std::abs(y2-y1);
        volatile uint16_t err=0;
        err = (x2-x1)/2; //точка перегиба
        volatile uint16_t ystep=0;
        if(y1<y2) {ystep=1;}
        else {ystep=-1;}     // если линия идет вверх меняем направление y

        for(;x1<=x2; x1++) //увеличиваем координату x1
        {
            if(step) {setPixel(y1,x1,col);} //если была поменяна система коодинат
            else     {setPixel(x1,y1,col);}
            err=err-(y2-y1);
            if(err>0)  //увеличиваем координату y
            {
                y1=y1+ystep+1;
                err=err+x2-x1;
            }
        }        
    }
private:
    float angle{0.0F};
    float length{0.0F}; //число точек - line length
    void swap(volatile uint16_t* x1,volatile uint16_t* x2)
    {
        volatile uint16_t z = *x2;*x1=*x2;*x2=z;
    }
};
class Elipse:public Pixel
{
public:
    Elipse(uint16_t x=0,uint16_t y=0,float R=0,uint16_t col=BLACK)
    {
        //SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));  /* set CP10 and CP11 Full Access */ //FPU enable
        for (uint16_t i=0;i<=2*R;i++)        
        {
            setPixel(x+i,y+sqrtf(R*R - (i-R)*(i-R))+R,col);
            setPixel(x+i,y-sqrtf(R*R - (i-R)*(i-R))+R,col);
            setPixel(x+sqrtf(R*R - (i-R)*(i-R))+R,y+i,col);
            setPixel(x-sqrtf(R*R - (i-R)*(i-R))+R,y+i,col);
        }
    }
private:
    float R{0.f};
};

#endif //SPILCD_H_