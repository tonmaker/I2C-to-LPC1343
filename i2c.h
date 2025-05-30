/*==============================================================================
LIB de controle I2C para LPC1343 e similares
Weliton Santos
email:		sisaewjs@hotmail.com
Instagram:	@tonmaker70
GitHub:		https://github.com/tonmaker
==============================================================================*/
#include "LPC13xx.h"

//Definições do I2C
#define I2C_SPEED 100000  // 100kHz
#define I2C_WRITE 0
#define I2C_READ 1

#define EEPROM_ADDR 0xA0

void delay(uint32_t d);
void delay_ms(uint16_t ms);

void I2C_Init(void){
	LPC_IOCON->PIO0_4 = 0x01; 							//PIO0_4 as SCL
	LPC_IOCON->PIO0_5 = 0x01; 							//PIO0_5 as SDA
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<5);				//Enable for I2C clock
	LPC_SYSCON->PRESETCTRL |= 0x02; 					//De-asset I2C reset

	LPC_I2C->SCLL = SystemCoreClock / (I2C_SPEED * 2);	//SCL Low
	LPC_I2C->SCLH = SystemCoreClock / (I2C_SPEED * 2);	//SCL High

	LPC_I2C->CONCLR = 0x6c; 							//I2C flagClear
	LPC_I2C->CONSET = 0x40; 							//I2C enable

	LPC_I2C->CONSET = 0x20; 							//assert Start
	
	//NVIC_EnableIRQ(I2C_IRQn); 						//Enable I2C interrupt
}

void I2C_Start(void){   
	LPC_I2C->CONSET = (1 << 5);  						//Set STA Start bit
    while (!(LPC_I2C->CONSET & (1 << 3)));				//Espera flag SI
    LPC_I2C->CONCLR = (1 << 5);  						//Limpa STA após enviado
}

void I2C_Restart(void){   
	LPC_I2C->CONSET = (1 << 5);  						//Set STA Start bit
	LPC_I2C->CONCLR = (1 << 3);							//Limpa SI
	while (!(LPC_I2C->CONSET & (1 << 3)));				//Espera flag SI
	LPC_I2C->CONCLR = (1 << 5);
}

void I2C_Stop(void){
    LPC_I2C->CONSET = (1 << 4);  						//Set STO Stop bit
    LPC_I2C->CONCLR = (1 << 3);  						//Limpa SI
    while (LPC_I2C->CONSET & (1 << 4));  				//Espera STOP ser concluído
}

void I2C_SendByte(uint8_t data){
    LPC_I2C->DAT = data;
	LPC_I2C->CONCLR = (1 << 3);							//Limpa SI
	while (!(LPC_I2C->CONSET & (1 << 3))); 
}

uint8_t I2C_ReceiveByteACK(void){
	LPC_I2C->CONSET = (1 << 2);							//AA = 1 Envia ACK após receber byte
	LPC_I2C->CONCLR = (1 << 3) | (1 << 5);				//Clear SI
	while (!(LPC_I2C->CONSET & (1 << 3))); 				//Espera SI ser setado
	return LPC_I2C->DAT;
}

uint8_t I2C_ReceiveByteNACK(void){
	LPC_I2C->CONCLR = (1 << 2) | (1 << 3);				//AA = 0 Envia NACK após receber byte; Clear SI
	while (!(LPC_I2C->CONSET & (1 << 3))); 				//Espera SI ser setado
	return LPC_I2C->DAT;
}

void I2C_Write24C256(uint16_t memAddr, uint8_t Value){
	I2C_Start();
	I2C_SendByte(EEPROM_ADDR + I2C_WRITE);
	I2C_SendByte(memAddr >> 8);
	I2C_SendByte(memAddr & 0xFF);
	I2C_SendByte(Value);
	I2C_Stop();
	delay_ms(10);
}

uint8_t I2C_Read24C256(uint16_t memAddr){
    uint8_t data;
    I2C_Start();
    I2C_SendByte(EEPROM_ADDR + I2C_WRITE);
    I2C_SendByte(memAddr >> 8);
    I2C_SendByte(memAddr & 0xFF);
    I2C_Restart();
    I2C_SendByte(EEPROM_ADDR + I2C_READ);
	data = I2C_ReceiveByteNACK();
    I2C_Stop();
    return data;
}
