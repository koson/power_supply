.PHONY: all,clean,load

CC = arm-none-eabi-gcc
OBJC = arm-none-eabi-objcopy
OBJD = arm-none-eabi-objdump
AS = arm-none-eabi-as
LD = arm-none-eabi-ld

TARGET = src/main.cpp
INC = inc/
SRC = src/
BLD = build/
LIB = lib/
FRS = freeRTOS/src/
FRH = freeRTOS/inc/

CPPFLAGS = -mcpu=cortex-m3 -mthumb -march=armv7-m -c -g -O0 -Wall -nostartfiles -lstdc++ \
	 -Wno-pointer-arith -mfloat-abi=soft \
	-u_printf_float -ffast-math -fno-math-errno -ffunction-sections -fdata-sections \
	-fno-rtti -fno-exceptions -specs=nosys.specs -specs=nano.specs -fno-common -D"assert_param(x)=" \
	-L"C:/Program Files (x86)/GNUArmEmbeddedToolchain/9_2020-q2-update/arm-none-eabi/lib" 
CFLAGS = -Wall -g -O0 -specs=nosys.specs -specs=nano.specs \
	 -c -fno-common -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -Wno-pointer-arith 
LCPPFLAGS = -mcpu=cortex-m3 -mthumb -nostartfiles -lm -lc -lgcc -lstdc++ \
		 -specs=nano.specs -specs=nosys.specs -fno-exceptions -fno-rtti \
		 -u_printf_float -mfloat-abi=soft -fno-use-cxa-atexit \
		 -L"C:/Program Files (x86)/GNUArmEmbeddedToolchain/9_2020-q2-update/arm-none-eabi/lib/thumb/v7-m/nofp" \
		 -L"C:/Program Files (x86)/GNUArmEmbeddedToolchain/9_2020-q2-update/arm-none-eabi/lib" \
		 -Xlinker -Map=$(BLD)main.map -z muldefs 
LDFLAGS =  -marmelf --gc-sections -lgcc -lm -lc \
	-L/usr/lib/gcc/arm-none-eabi/7.3.1/ -L/usr/lib/gcc/arm-none-eabi/7.3.1/thumb/v7-m/
	
load: $(BLD)main.bin
	openocd -f lib/stlink.cfg -f lib/stm32f1x.cfg -c "program $(BLD)main.bin verify reset exit 0x08000000"

all: $(BLD)main.bin $(BLD)main.elf $(BLD)main.lst
$(BLD)main.bin: $(BLD)main.elf
	$(OBJC) $(BLD)main.elf $(BLD)main.bin -O binary
$(BLD)main.lst: $(BLD)main.elf
	$(OBJD) -D $(BLD)main.elf > $(BLD)main.lst
$(BLD)main.elf: $(BLD)startup.o $(BLD)main.o $(BLD)uart.o $(BLD)adc.o $(BLD)dac.o
$(BLD)main.elf: $(BLD)but.o $(BLD)pwm.o $(BLD)timers.o
	$(CC) -o $(BLD)main.elf -T$(LIB)stm32f107.ld $(BLD)startup.o $(BLD)main.o \
	$(BLD)but.o $(BLD)pwm.o $(BLD)uart.o $(BLD)adc.o $(BLD)dac.o $(BLD)timers.o \
	-I$(LIB) -I$(FRH) $(LCPPFLAGS)
	arm-none-eabi-size $(BLD)main.elf
$(BLD)startup.o: $(LIB)startup.cpp
	$(CC) $(LIB)startup.cpp -o $(BLD)startup.o $(CPPFLAGS)
$(BLD)pwm.o: $(SRC)pwm.cpp
	$(CC) $(SRC)pwm.cpp -o $(BLD)pwm.o -I$(INC) -I$(LIB) -I$(FRH) $(CPPFLAGS)
$(BLD)timers.o: $(SRC)timers.cpp
	$(CC) $(SRC)timers.cpp -o $(BLD)timers.o -I$(INC) -I$(LIB) -I$(FRH) $(CPPFLAGS)
$(BLD)but.o: $(SRC)but.cpp
	$(CC) $(SRC)but.cpp -o $(BLD)but.o -I$(INC) -I$(LIB) -I$(FRH) $(CPPFLAGS)			
$(BLD)adc.o: $(SRC)adc.cpp
	$(CC) $(SRC)adc.cpp -o $(BLD)adc.o -I$(INC) -I$(LIB) -I$(FRH) $(CPPFLAGS)
$(BLD)dac.o: $(SRC)dac.cpp
	$(CC) $(SRC)dac.cpp -o $(BLD)dac.o -I$(INC) -I$(LIB) -I$(FRH) $(CPPFLAGS)
$(BLD)uart.o: $(SRC)uart.cpp
	$(CC) $(SRC)uart.cpp -o $(BLD)uart.o -I$(INC) -I$(LIB) -I$(FRH) $(CPPFLAGS)
$(BLD)main.o: $(TARGET)
	$(CC) $(TARGET) -o $(BLD)main.o -I$(INC) -I$(LIB) -I$(FRH) $(CPPFLAGS)
	
clean:
	rm -rf $(BLD)*.o $(BLD)*.elf $(BLD)*.lst $(BLD)*.bin $(BLD)*.map 


	
