/*!
 * \file main file with main function from which program is started after initialization procedure in startup.cpp
 * 
 */
#include "main.h"
#include <cmath>
void *__dso_handle = nullptr; // dummy "guard" that is used to identify dynamic shared objects during global destruction. (in fini in startup.cpp)
void delay(uint32_t x) {
	while(x--);
}
int main()
{    
	//__enable_irq();
	//! disable JTAG pins for normal lcd work
	RCC->APB2ENR|=RCC_APB2ENR_IOPBEN;
	RCC->APB2ENR|=RCC_APB2ENR_AFIOEN;
	AFIO->MAPR|=AFIO_MAPR_SWJ_CFG_JTAGDISABLE;
	//!____________ objects ___________________
	RCCini rcc;	//! 72 MHz
	SpiLcd lcd;
	lcd.fillScreen(0xff00);
	Font_16x16 font;
	ADC adc;
	Dac dac;
	//dac.setVoltage(2);
	PWM pwm;
	
	pwm.setFrequency(1);
	Timers antiRattle(1);
	Timers encoder(3);
	Timers msec100(4);
	But but;
	//Uart uart(2);
	//LED13 led;	
	//__________________________________________
	__enable_irq();
	int x; 	
	float xdac=0;
	float K_DAC_to_ampere = 1;
	dac.setVoltage(0.230); // no current flow through MOSFET in this case (2.5 V on gate)
	pwm.setFrequency(10); //10 kHz

	//_____________ PIDR ___________________________
	float K_proportional = 1.F/20.F;
	float K_integral = 1.F/400.F;
	float K_differential = 1.F/200.F;
	float PIDR_accumulator = 0;
	float Err_previous = 0;
	float Err = 0;
	float I_dac = 0;
	float dac_set = 0.240;
	float dac_set_new = dac_set;
	float I_set = 0;
	bool PIDR_start = false;
	//_____________________________________________
	uint32_t decrease_count = 0;
	while(1) {	
		encoder.checkValue();
		//pwm.setFrequency(encoder.encoderValue());
		pwm.setDuty(encoder.encoderValue());
		//!_________ Buttons ________________________________
		if(but.Plus) {
			if(I_set <= 1.5) {
				I_set += 0.01;	
			}
			but.Plus = false;
		}
		if(but.Minus) {
			if(I_set > 0.010) {
				I_set -= 0.01;	
			}
			but.Minus = false;
		}
		if(but.Encoder) {
			if(!but.Fan) {
				but.Fan = true;
				but.startFan();
			} else {
				but.Fan = false;
				but.stopFan();
			}						
			but.Encoder = false;
		}
		adc.start();	
		font.intToChar(encoder.encoderValue());
		font.print(0,0,0xFF00,font.arr,5);		
		
		// __________________ Amperage measurements ____________________
		float amperage = (3.3*(float)(adc.amperage + 1)/4095.F);
		font.floatTochar(amperage); 
		font.print(0,120,font.RED,font.arrFloat,5); font.print(160,120,font.RED,"A",5); 
		
		// __________________ Voltage ____________________________
		//font.intToChar(adc.data[1]);
		float voltage = (3.3*(float)(adc.voltage + 8)/4095.F)*100.F;
		font.floatTochar(voltage); 
		font.print(0,140,font.BLUE,font.arrFloat,5); font.print(160,140,font.BLUE,"V",1);

		// __________________ Temperature ________________________
		uint16_t V = adc.temperature;
		float temperature = 0.035*V + 5.5;
		font.floatTochar(temperature);
		font.print(0,200,0xFF00,font.arrFloat,5);
		
//**********************************************************************************
//**********************************  PIDR  ****************************************
//**********************************************************************************
		//! __________________ Amperage settings _______________________
		font.floatTochar(dac_set);
		font.print(0,20,font.GREEN,font.arrFloat,5); font.print(160,20,font.GREEN,"V",1);
		font.floatTochar(I_set);
		font.print(0,40,font.CYAN,font.arrFloat,5); font.print(160,40,font.CYAN,"A",1);
		font.floatTochar(dac.voltage);
		font.print(0,60,font.GREEN,font.arrFloat,5); font.print(160,60,font.GREEN,"V",1);
		

		if(amperage>1.2){dac.setVoltage(0.230); continue;} //go to start of while (protection from short circuit)
		if(msec100.timer_mSecFlag) {
			if(PIDR_start) {
				Err_previous = Err;
				Err = amperage - I_set;		//measured minus setted 	
				PIDR_accumulator += (Err);
				font.floatTochar(PIDR_accumulator);
				font.print(0,80,font.CYAN,font.arrFloat,5); font.print(160,80,font.CYAN,"A",1);
				dac_set_new = dac.voltage - (Err*K_proportional) - PIDR_accumulator*K_integral
				 			  -(Err_previous-Err)*K_differential;
				dac.setVoltage(dac_set_new); //set new U_dac to reduce I through MOSFET
					
				if(PIDR_accumulator<-2){
					PIDR_start = false;
					PIDR_accumulator = 0;
				}			
			} else {
				if(amperage>I_set){
					PIDR_start = true;						
				} else {
					//PIDR_start = false;
				}
				PIDR_accumulator = 0;
				if(dac.voltage < 1) {
					dac_set_new = dac.voltage + 0.002;
					dac.setVoltage(dac_set_new); //set new U_dac to reduce I through MOSFET			
				}
				font.print(0,80,font.CYAN,"0",5); font.print(160,80,font.CYAN,"A",1);
			}
			msec100.timer_mSecFlag = false;
		}
	}
    return 0;
}
