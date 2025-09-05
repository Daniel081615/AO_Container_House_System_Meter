/*---------------------------------------------------------------------------------------
 * File Name     : AO_EE24C.C
 * Company 		 : AOTECH
 * Author Name   : Barry Su
 * Starting Date : 2018.5.8
 * C Compiler : AtmelStudio 7.0
 * --------------------------------------------------------------------------------------

 *------------------------------------------------------*/
#include <stdio.h>
#include "NUC1261.h"
#include "AO2022_MeterModule_1261.h"
#include "AO_MyDef.h"
#include "AO_ExternFunc.h"
#include "AO_EE24C.h"
#include "AO_MyDef.h"


// PORTC4 & PORTC5
#define TWI_SDA_PIN     4
#define TWI_SCL_PIN      5

#define LOW 0
#define HIGH 1
#define false 0
#define true 1

#define I2C_DELAY_USEC		2
#define I2C_READ 1
#define I2C_WRITE 0

#define EE_SDA_DIR_Out()		PF->MODE = (PF->MODE & (~GPIO_MODE_MODE4_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE4_Pos) 
#define EE_SDA_DIR_In()			PF->MODE = (PF->MODE & (~GPIO_MODE_MODE4_Msk)) | (GPIO_MODE_QUASI << GPIO_MODE_MODE4_Pos);
#define EE_SCL_DIR_Out()		PF->MODE = (PF->MODE & (~GPIO_MODE_MODE3_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE3_Pos) 
#define EE_SCL_DIR_In()			PF->MODE = (PF->MODE & (~GPIO_MODE_MODE3_Msk)) | (GPIO_MODE_QUASI << GPIO_MODE_MODE3_Pos) 

#define EE_SDA_PIN_High()		(PF4 = 1)
#define EE_SDA_PIN_Low()		(PF4 = 0) 
#define EE_SCL_PIN_High()		(PF3 = 1)
#define EE_SCL_PIN_Low()		(PF3 = 0)
#define EE_WP_PIN_High()		(PC0= 1)	
#define EE_WP_PIN_Low()			(PC0= 0)	
#define EE_SDA_is_High()			(PF4)


void a_delay_us(uint8_t uSecDelay);
void SoftI2cMasterInit(void);
void SoftI2cMasterDeInit(void);
uint8_t soft_i2c_eeprom_read_byte(uint8_t deviceAddr, uint16_t readAddress) ;
_Bool soft_i2c_eeprom_write_byte(uint8_t deviceAddr, uint16_t writeAddress, uint8_t writeByte);
uint8_t SoftI2cMasterRead(uint8_t last);
_Bool SoftI2cMasterRestart(uint8_t addressRW);
_Bool SoftI2cMasterStart(uint8_t addressRW);
void SoftI2cMasterStop(void);
_Bool SoftI2cMasterWrite(uint8_t data);
_Bool I2cReadBytes(uint8_t deviceAddr, uint16_t readAddress, uint8_t *readBuffer, uint8_t bytestoRead);
_Bool I2cWriteBytes(uint8_t deviceAddr, uint16_t writeAddress, uint8_t *writeBuffer, uint8_t bytesToWrite);
uint16_t GetMeterDataAddress(uint8_t index);
void I2cWriteDataStruct(uint8_t index, STR_METER_D *data);
void I2cReadDataStruct(uint8_t index, STR_METER_D *data);

void a_delay_us(uint8_t uSecDelay)
{
    uint8_t i,k;

    for(i=0;i<uSecDelay;i++)
    {        
    	for(k=0;k<50;k++) __NOP();
    }
}

// Initialize SCL/SDA pins and set the bus high
void SoftI2cMasterInit(void) 
{	
	
	EE_SCL_DIR_Out();
	EE_SCL_PIN_High();
	EE_SDA_DIR_Out();
	EE_SDA_PIN_High();
	
}

// De-initialize SCL/SDA pins and set the bus low
void SoftI2cMasterDeInit(void) 
{
	
	EE_SCL_DIR_In();
	EE_SCL_PIN_Low();
	EE_SDA_DIR_In();
	EE_SDA_PIN_Low();

}

// Read a byte from I2C and send Ack if more reads follow else Nak to terminate read
uint8_t SoftI2cMasterRead(uint8_t last) 
{
	uint8_t i;
	uint8_t b = 0;
	
	// Make sure pull-up enabled
	EE_SDA_PIN_High();
	EE_SDA_DIR_In();
	
	// Read byte
	for (i = 0; i < 8; i++) {
		// Don't change this loop unless you verify the change with a scope
		b <<= 1;
		a_delay_us(I2C_DELAY_USEC);
		EE_SCL_PIN_High();
		a_delay_us(I2C_DELAY_USEC);
		if ( EE_SDA_is_High() )	b |= 1 ;
		a_delay_us(I2C_DELAY_USEC);
		EE_SCL_PIN_Low();
	}
	// Send Ack or Nak
	EE_SDA_DIR_Out();
	if (last) { 
		EE_SDA_PIN_High();
	} else { 
		EE_SDA_PIN_Low();
	}  
	EE_SCL_PIN_High();
	a_delay_us(I2C_DELAY_USEC);
	EE_SCL_PIN_Low();
	EE_SDA_PIN_Low();
	
	return b;
}

// Write a byte to I2C
_Bool SoftI2cMasterWrite(uint8_t data) 
{
	uint8_t m;
	uint8_t rtn;
	
	EE_SDA_DIR_Out();
	// Write byte
	for (m = 0x80; m != 0; m >>= 1) 
	{
		// Don't change this loop unless you verify the change with a scope
		a_delay_us(I2C_DELAY_USEC);
		if (m & data) 
		{ 
			EE_SDA_PIN_High();
		} else { 
			EE_SDA_PIN_Low();
		}
		a_delay_us(I2C_DELAY_USEC);
		EE_SCL_PIN_High();
		a_delay_us(I2C_DELAY_USEC);
		EE_SCL_PIN_Low();
		
	}      
	// Enable pullup
	EE_SDA_PIN_High();
	EE_SDA_DIR_In();
	EE_SCL_PIN_High();
    a_delay_us(I2C_DELAY_USEC+2);
	rtn = EE_SDA_is_High() ;		//  get Ack or Nak, SDA = 0, ACK; SDA = 1, NACK;
	EE_SCL_PIN_Low();
	EE_SDA_DIR_Out();
	EE_SDA_PIN_Low();
	a_delay_us(I2C_DELAY_USEC);
	
	return rtn == 0;
}

// Issue a start condition
_Bool SoftI2cMasterStart(uint8_t addressRW) 
{
 
	EE_SCL_DIR_Out();
	EE_SDA_DIR_Out();
	EE_SDA_PIN_Low();			// SCL High, SDA high -> low => Start signal
    a_delay_us(I2C_DELAY_USEC);
	EE_SCL_PIN_Low();			// SCL Low, Ready to Read or Write Data
    a_delay_us(I2C_DELAY_USEC);
	
    return SoftI2cMasterWrite(addressRW);
}

// Issue a restart condition
_Bool SoftI2cMasterRestart(uint8_t addressRW) 
{
	
	EE_SCL_DIR_Out();
	EE_SDA_DIR_Out();
	EE_SDA_PIN_High();
	EE_SCL_PIN_High();
    a_delay_us(I2C_DELAY_USEC);
	
    return SoftI2cMasterStart(addressRW);
}

// Issue a stop condition
void SoftI2cMasterStop(void) 
{
	EE_SDA_DIR_Out();
	EE_SDA_PIN_Low();
	a_delay_us(I2C_DELAY_USEC);
	EE_SCL_PIN_High();
	a_delay_us(I2C_DELAY_USEC);
	EE_SDA_PIN_High();		// SCL High, SDA low -> high => Stop Signal
	a_delay_us(I2C_DELAY_USEC);
}

// Read 1 byte from the EEPROM device and return it
uint8_t soft_i2c_eeprom_read_byte(uint8_t deviceAddr, uint16_t readAddress) {
	uint8_t byteRead = 0;
	
	// Issue a start condition, send device address and write direction bit
	if (!SoftI2cMasterStart((deviceAddr<<1) | I2C_WRITE)) return false;
       a_delay_us(I2C_DELAY_USEC);
	// Send the address to read, 8 bit or 16 bit
	//if (readAddress > 255) {
		if (!SoftI2cMasterWrite((readAddress >> 8))) return false; // MSB		=> if NACK, return false
		if (!SoftI2cMasterWrite((readAddress & 0xFF))) return false; // LSB
	//}
	//else {
	//	if (!SoftI2cMasterWrite(readAddress)) return false; // 8 bit
	//}

	// Issue a repeated start condition, send device address and read direction bit
	if (!SoftI2cMasterRestart((deviceAddr<<1) | I2C_READ)) return false;
	a_delay_us(I2C_DELAY_USEC);
	// Read the byte
	byteRead = SoftI2cMasterRead(1);

	// Issue a stop condition
	SoftI2cMasterStop();
   	
	return byteRead;
}

// Write 1 byte to the EEPROM
_Bool soft_i2c_eeprom_write_byte(uint8_t deviceAddr, uint16_t writeAddress, uint8_t writeByte) 
{
	uint8_t j;
	// WP = Low (Disable Write Protection)
	EE_WP_PIN_Low();
	a_delay_us(I2C_DELAY_USEC);
	// Issue a start condition, send device address and write direction bit
	if (!SoftI2cMasterStart((deviceAddr<<1) | I2C_WRITE)) return false;
        a_delay_us(I2C_DELAY_USEC);
	// Send the address to write, 8 bit or 16 bit
	//if ( writeAddress > 255) {
		if (!SoftI2cMasterWrite((writeAddress >> 8))) return false; // MSB
		if (!SoftI2cMasterWrite((writeAddress & 0xFF))) return false; // LSB
	//}
	//else {
	//	if (!SoftI2cMasterWrite(writeAddress)) return false; // 8 bit
	//}

	// Write the byte
	if (!SoftI2cMasterWrite(writeByte)) return false;

	// Issue a stop condition
	SoftI2cMasterStop();
	
	// Delay 10ms for the write to complete, depends on the EEPROM you use
	//_delay_ms(10);
	for ( j=0;j<10;j++) 
	{
		a_delay_us(100); 
	}
	// WP = HIGH (Enable Write Protection)
	EE_WP_PIN_High();
	
	return true;
}

// Read more than 1 byte from a device
_Bool I2cReadBytes(uint8_t deviceAddr, uint16_t readAddress, uint8_t *readBuffer, uint8_t bytestoRead) 
{
		
		// Issue a start condition, send device address and write direction bit
		if (!SoftI2cMasterStart((deviceAddr<<1))) return false;

		// Send the address to read
		if (!SoftI2cMasterWrite((readAddress >> 8))) return false; // MSB		=> if NACK, return false
		if (!SoftI2cMasterWrite((readAddress & 0xFF))) return false; // LSB

		// Issue a repeated start condition, send device address and read direction bit
		if (!SoftI2cMasterRestart((deviceAddr<<1) | I2C_READ)) return false;

		// Read data from the device
		for (uint8_t i = 0; i < bytestoRead; i++) {
			// Send Ack until last byte then send Ack
			readBuffer[i] = SoftI2cMasterRead(i == (bytestoRead-1));
		}
		SoftI2cMasterStop();
	
		return true;
}

_Bool I2cWriteBytes(uint8_t deviceAddr, uint16_t writeAddress, uint8_t *writeBuffer, uint8_t bytesToWrite) 
{
	
		uint8_t j;
		EE_WP_PIN_Low();						// WP = Low (Disable Write Protection)
		a_delay_us(I2C_DELAY_USEC);
	
    if (!SoftI2cMasterStart((deviceAddr << 1) | I2C_WRITE)) return false;

    // 寫入 EEPROM address (High + Low byte)
    if (!SoftI2cMasterWrite((writeAddress >> 8) & 0xFF)) return false; // MSB
    if (!SoftI2cMasterWrite(writeAddress & 0xFF)) return false;        // LSB

    // 寫入資料
    for (uint8_t i = 0; i < bytesToWrite; i++) {
        if (!SoftI2cMasterWrite(writeBuffer[i])) return false;
    }

    SoftI2cMasterStop();

		// Delay 10ms for the write to complete, depends on the EEPROM you use
		//_delay_ms(10);
		for ( j=0;j<10;j++) 
		{
			a_delay_us(100); 
		}
		// WP = HIGH (Enable Write Protection)
		EE_WP_PIN_High();

    return true;
}



/*	Write Data Structure in EEPROM	
		STRUCT_SIZE = 0x36
*/

//	Get the StartAddr of each Data structure
uint16_t GetMeterDataAddress(uint8_t index) {
    return (STRUCT_SIZE * index);
}

/* Write Data structure to EEPROM
@param index
*/
void I2cWriteDataStruct(uint8_t index, STR_METER_D *data) 
{
    uint16_t addr = GetMeterDataAddress(index);
    uint8_t *buf = (uint8_t *)data;
    uint8_t bytes_remaining = STRUCT_SIZE;

    while (bytes_remaining > 0) {
        uint8_t offset_in_page = addr % EEPROM_PAGE_SIZE;
        uint8_t bytes_left_in_page = EEPROM_PAGE_SIZE - offset_in_page;
        uint8_t chunk_size = (bytes_remaining > bytes_left_in_page) ? bytes_left_in_page : bytes_remaining;

        I2cWriteBytes(EEPROM_ADDR, addr, buf, chunk_size);

        bytes_remaining -= chunk_size;
        addr += chunk_size;
        buf += chunk_size;
    }
}


#define METER_DATA_ADDR_BASE   0x0000  // EEPROM 開始儲存位置
#define METER_DATA_SIZE        sizeof(STR_METER_D)


void I2cReadDataStruct(uint8_t index, STR_METER_D *data)
{
    uint16_t addr;

    if (index >= PwrMtrMax || data == NULL)
        return;

    addr = METER_DATA_ADDR_BASE + (index * METER_DATA_SIZE);

    I2cReadBytes(EEPROM_ADDR, addr, (uint8_t *)data, METER_DATA_SIZE);
}


