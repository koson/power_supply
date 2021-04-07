#ifndef UART_HPP
#define UART_HPP

#include "irq.hpp"

extern "C" void USART1_IRQHandler();
extern "C" void USART2_IRQHandler();

class Uart: public Interruptable {
public:
    Uart(uint8_t num = 1);
    void sendByte(uint8_t byte);
    void sendStr(const char* str);
    void readByte1();
    void readByte2();
    void InterruptHandle() override;
    static void setUart1();
    static void setUart2();
    bool wasInterruptedby1=false;
    bool wasInterruptedby2=false;
    uint8_t receivedArr[1]={0};
    static Uart* pThis;
private:
    void uart_init(uint8_t num);
    uint8_t UartNum=1;    
};

#endif //UART_HPP