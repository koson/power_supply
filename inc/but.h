#ifndef BUT_H
#define BUT_H

#include "stm32f10x.h"

class But {
public:    
    But();
    static But* pThis;
    bool Plus = false;
    bool Minus = false;
    bool Encoder = false;
    bool Fan = false;

    void startFan();
    void stopFan();
private:
    void init();
};


#endif //BUT_H