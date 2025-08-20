
#include "AO_MyDef.h"

#define I2C_4Byte						4
#define EEPROM_PAGE_SIZE   	64
#define STRUCT_SIZE        	36
#define METER_DATA_NUM     	31
#define EEPROM_BASE_ADDR   	0x0000

extern void a_delay_us(uint8_t uSecDelay);
extern void SoftI2cMasterInit(void);
extern void SoftI2cMasterDeInit(void);
extern uint8_t soft_i2c_eeprom_read_byte(uint8_t deviceAddr, uint16_t readAddress) ;
extern _Bool soft_i2c_eeprom_write_byte(uint8_t deviceAddr, uint16_t writeAddress, uint8_t writeByte);
extern _Bool I2cReadBytes(uint8_t deviceAddr, uint16_t readAddress, uint8_t *readBuffer, uint8_t bytestoRead);
extern _Bool I2cWriteBytes(uint8_t deviceAddr, uint16_t writeAddress, uint8_t *writeBuffer, uint8_t bytesToWrite);
extern uint16_t GetMeterDataAddress(uint8_t index);
extern void I2cWriteDataStruct(uint8_t index, STR_METER_D *data);