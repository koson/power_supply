#ifndef IRQ_HPP
#define IRQ_HPP

#include "stm32f10x.h"

//void(*IsrVectors[42])();
//void AddHandler(void (*IsrVectors[41])(),IRQn y)
//{
//    IsrVectors[y]=*IsrVectors;
//}
//------------------ class interrupt manager -----------------
class InterruptManager
{
public:
    InterruptManager(){}
    static void (*IsrVectors[41])(void);
    static void DefaultHandler(void) {asm volatile ("nop"::);}
    static void AddHandler(void (*IsrVec)(), IRQn_Type irq)         
    {   IsrVectors[irq] = IsrVec;    } // IsrVectors[TIM3_IRQn] = blink  func pointer
    static void RemoveHandler(IRQn_Type irq)
    {   IsrVectors[irq] = DefaultHandler;    }
    static void Call(IRQn_Type irq)  //call function
    {   IsrVectors[irq]();}
    //`static void (*IsrVectors[41])(void);
};
void (*InterruptManager::IsrVectors[41])(void)={InterruptManager::DefaultHandler};
//--------------------- class Interruptable ------------------------------------------
class Interruptable
{
public:
    Interruptable(){}
    virtual ~Interruptable(){}
    virtual void InterruptHandle(){}  // need to override
};
//------------------------- Interrupt classes manager ------------------------
template <IRQn_Type irqN>
class InterruptSubject
{
public:
    InterruptSubject(){}
    static void IrqHandle()
    {
        for(uint8_t i=0;i<10;i++)
        {
            if(ArrOffInterrupteClasses[i]!=nullptr)
            {
                ArrOffInterrupteClasses[i]->InterruptHandle();
            }
        }
    }   
    static void SetVector(){InterruptManager::AddHandler(IrqHandle,irqN);}
    static void setInterrupt(Interruptable* pInterrupt)
    {
        for(uint8_t i=0;i<10;i++)
        {
            if(ArrOffInterrupteClasses[i]==nullptr)
            {
                ArrOffInterrupteClasses[i]=pInterrupt;
                break;
            }
        }
    }
    static void removeInterrupt(Interruptable* pInterrupt)
    {
        for(uint8_t i=0;i<10;i++)
        {
            if(ArrOffInterrupteClasses[i]==pInterrupt)
            {
                ArrOffInterrupteClasses[i]=nullptr;
                break;
            }
        }
    }
private:
    static Interruptable* ArrOffInterrupteClasses[10];
};
template <IRQn_Type irqN>
Interruptable* InterruptSubject<irqN>::ArrOffInterrupteClasses[10]={nullptr};

//-----------------  example classes -----------------------------------------
//class Blink10
//{
//public:
//    Blink10(){pThis=this;}
//    static void setFunc1(void) {pThis->ledOn();}
//   static void setFunc2(void) {pThis->ledOff();}
//    void setActiveIrq(void) {pThis=this;}
//    void InterruptHandle()
//    {
//        InterruptManager::AddHandler(Blink10::setFunc1,TIM2_IRQn);
//	    InterruptManager::AddHandler(Blink10::setFunc2,TIM3_IRQn);
//    }
//private:
//    void ledOn()
//    {GPIOC->BSRR|=GPIO_BSRR_BR13;}
//    void ledOff()
//    {GPIOC->BSRR|=GPIO_BSRR_BS13;}
//    static Blink10* pThis;
//};
//Blink10* Blink10::pThis=nullptr;

class Blink:public Interruptable
{
public: 
    Blink(){led_ini();}
    ~Blink()override{}
    void InterruptHandle() override
    {
        toggle();
    }
    void toggle() {GPIOC->ODR^=GPIO_ODR_ODR13;}

private:
    void led_ini()
    {
        RCC->APB2ENR|=RCC_APB2ENR_IOPCEN;
	    GPIOC->CRH&=~GPIO_CRH_CNF13;
	    GPIOC->CRH|=GPIO_CRH_MODE13;//50MHz
    }
};

#endif //IRQ_HPP