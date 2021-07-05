#ifndef SPI_SLAVE
#define SPI_SLAVE

#include "stm32f10x.h"

class SpiSlave {
public:
    SpiSlave();
    static SpiSlave* pThis;
    bool receivedFlag = false;
    uint8_t receivedByte = 0;
    uint8_t receivedBytes[4] = {0};
    volatile uint8_t sendBytes[5] = {10,11,12,13,100}; //last byte = first byte for continious answer
    volatile uint8_t bytesCounter = 0;
    void setAnswerByte(uint8_t byte);
private:
    void init();

};


#endif //SPI_SLAVE