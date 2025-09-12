/**--------------------------------------------------------------------------- 
 * File Name     : ITRI_GPL2_SensorHub.h
 * Company 		 : AOTECH
 * Author Name   : Barry Su
 * Starting Date : 2020.4.24
 * C Compiler : 
 
  SERCOM0 / Reader
  SERCOM1 / Host
  SERCOM2 / Debug
  PA18 ~ PA25 / input: Device ID Switch
  PA10 / Output: RS485 DIR for Reader (0:Input)
  PA11 / Output: RS485 DIR for Host (0:Input)
  
  PA00 / ADC for Light Sensor
  PA01 / Bi- : I2C-SDA for E2PROM
  PA02 / Output: I2C-SCK for E2PROM
  PA03 / Output: E2PROM WP (0:Write protect)
  PB08 / Output: Status LED (0:LED ON)
  PB10 / Input: Meter Digital Input (Low Counter)
  PB11 / Output: SSR Control (1: SSR ON)
 
	* @ESG Container	Meter 'Define' Code
 * -------------------------------------------------------------------------- 
*/
#ifndef __AO_MY_DEF__
#define __AO_MY_DEF__

#define VERSION	 "V2020-04-24-01"

#include "stdint.h"
#include "ota_manager.h"

// Device Per Meter Board

#define PwrMtrMax 		1
#define BmsMax				1
#define WtrMtrMax 		1
#define InvMax				1
#define	PyrMtrMax 		1
#define	SoilSensorMax 1
#define	AirSensorMax 	1

//wdt max retry times
#define MAX_WDT_TRIES				4

//#define METER_TEST

#define MEMBER_MAX_NO		2

#define POLL_TIMEOUT 					25
#define MAX_POLL_RETRY_TIMES	5

#define MAX_METER_TXQ_LENGTH    	25
#define MAX_METER_RXQ_LENGTH    	25
#define METER_TOKEN_LENGTH			50

#define HOST_TOKEN_LENGTH			100
#define MAX_HOST_TXQ_LENGTH    	100
#define MAX_HOST_RXQ_LENGTH		100

#define SYSTEM_ERROR_TIMEOUT		500
#define MAX_BUTTON_TIMES				5
#define EEPROM_ADDR					0x50


#define EE_ADDR_ROOM_NAME			0x0010
#define EE_ADDR_ROOM_MODE			0x0015
#define EE_ADDR_ROOM_RATE			0x0016
#define EE_ADDR_ROOM_NO				0x0017
#define EE_ADDR_POWER100W			0x0018
#define EE_ADDR_STUDENT_ID			0x0020
#define EE_ADDR_USER_UID				0x0050
#define EE_ADDR_TIME_TAG				0x0075
#define EE_ADDR_USER_VALUE			0x00A0
#define EE_ADDR_BOOTLOADER_SWITCH	0x00B7
#define USER_CHECK_CODE_ADDR			0x00B0
#define EE_ADDR_PORT					0x00F0


#define RELAY1_Low()				(PA3 = 0)
#define RELAY1_High()				(PA3 = 1)
#define RELAY2_Low()				(PF7 = 0)
#define RELAY2_High()				(PF7 = 1)
#define LED_G_On()				(PD7 = 0)
#define LED_G_Off()				(PD7 = 1)
#define LED_G_TOGGLE()			(PD7 ^= 1)
#define LED_R_On()				(PF2 = 0)
#define LED_R_Off()				(PF2 = 1)
#define LED_R_TOGGLE()			(PF2 ^= 1)
#define LED_G1_On()				(PE0 = 0)
#define LED_G1_Off()				(PE0 = 1)
#define LED_G1_TOGGLE()			(PE0 ^= 1)
#define LED_R1_On()				(PC4 = 0)
#define LED_R1_Off()				(PC4 = 1)
#define LED_R1_TOGGLE()			(PC4 ^= 1)




// SERCOM 2
#define DIR_METER_RS485_In()		//(PC1 = 0)
#define DIR_METER_RS485_Out()		//(PC1 = 1)

#define SET_RECORD_READY() 		fgToHostFlag |= 0x40


#define SYSTEM_POWER_ON		0x01
#define SYSTEM_POWER_OFF		0x02

#define DAE_530n540				0x01 
#define CIC_BAW2A				0x02
#define TUTANG_E21nE31			0x03
#define DEM_510c					0x04
#define CIC_BAW1A				0x05

enum DEFINE_POWER_METER_STATE {
	MP_READY,	
	MP_POLLING_RSP,
	MP_POLLING_W_CMD,	
	MP_POLLING_RELAY_CMD,	
	MP_POLLING_BAL_CMD,		
	MP_POLLING_V_CMD,	
	MP_POLLING_I_CMD,
	MP_POLLING_F_CMD,	
	MP_POLLING_P_CMD,	
	MP_POLLING_S_CMD,	
	MP_POLLING_PF_CMD,
			
//	MP_SET_DO_ON,
//	MP_SET_DO_OFF,
//	MP_SET_DO_LOCK_ON,
//	MP_SET_DO_LOCK_OFF,	
	MP_SET_RELAY1,
	MP_SET_RELAY2,	
	MP_ADD_VALUE1,
	MP_ADD_VALUE2,
	MP_EXIT_TEST1,
	MP_EXIT_TEST2,
	MP_SET_MODE1,	
	MP_SET_MODE2,	
	MP_SET_ADDR1,
	MP_SET_ADDR2,
	MP_SET_BAUDRATE1,
	MP_SET_BAUDRATE2,
	MP_ERROR,
};



// Time Index
#define INX_YEAR_H		0
#define INX_YEAR_L		1
#define INX_MON			2
#define INX_DAY			3
#define INX_HOUR		4
#define INX_MIN			5
#define INX_SEC			6
#define INX_WEEK		7

#define INX_TIME_START_YY_H	(MAX_HOST_RXQ_LENGTH-10)
#define INX_TIME_START_YY_L	(MAX_HOST_RXQ_LENGTH-9)
#define INX_TIME_START_M	(MAX_HOST_RXQ_LENGTH-8)
#define INX_TIME_START_D	(MAX_HOST_RXQ_LENGTH-7)
#define INX_TIME_START_H	(MAX_HOST_RXQ_LENGTH-6)
#define INX_TIME_START_MN	(MAX_HOST_RXQ_LENGTH-5)
#define INX_TIME_START_S	(MAX_HOST_RXQ_LENGTH-4)
#define INX_TIME_START_W	(MAX_HOST_RXQ_LENGTH-3)


#define RELAY_ON	0x01
#define RELAY_OFF	0x00


enum DEFINE_RS485_METER_TOKEN {

METER_CMD_ALIVE=0x10,				//0x10			
METER_GET_CMD_POWER_METER,
METER_GET_CMD_BMS,
METER_GET_CMD_WATER_METER,
METER_GET_CMD_PYRANOMETER,	//0x14
METER_GET_CMD_SOILSENSOR,	
METER_GET_CMD_AIRSENSOR,	
METER_GET_CMD_INV,
METER_GET_CMD_WATERING,	
METER_OTA_CMD,

METER_RSP_ACK=0x30,			//0x30
METER_RSP_SYS_INFO,				//0x31
METER_RSP_USER_DATA,		//0x32
METER_RSP_POWER_DATA,			//0x33
METER_RSP_BMS_DATA,			//0x34
METER_RSP_WATER_DATA,				//0x35
METER_RSP_PYR_DATA,			//0x36
METER_RSP_SOIL_DATA,				//0x37
METER_RSP_AIR_DATA,			//0x38
METER_RSP_INV_DATA,					//0x39
METER_RSP_WATERING_STATUS,

METER_OTA_UPDATE = 0x40,
METER_SWITCH_FW,
METER_GET_FW_STATUS,
METER_REBOOT,

};

// State Modet
enum DEFINE_STATE_POLL {						
PL_NORM=0,								
PL_STATUS,
PL_STATUS_RSP,
PL_INIT,
PL_INIT_RSP,
PL_INIT1,
PL_INIT_RSP1,
PL_CHG_BAL,
PL_CHG_BAL_RSP,
PL_GET_USER_MODE,
PL_GET_USER_MODE_RSP,
PL_GET_USER_INFO,
PL_GET_USER_INFO_RSP,
PL_GET_DOOR_RCD,
PL_GET_DOOR_RCD_RSP,
};

// State Mode
enum DEFINE_STATE_MODE {
STM_INIT=0,								// 0
STM_NORM,								// 1
STM_POWER_ON_READY,
STM_POWER_OFF_READY,
STM_CHARGE_FREE_WAIT,
STM_CHARGE_STOP_WAIT,
STM_POWER_ON,
STM_POWER_OFF,
STM_CHARGE_FREE,
STM_CHARGE_STOP,
STM_WAIT_ON_ACK,
STM_WAIT_OFF_ACK,
STM_WAIT_MODE,
STM_DELAY_SEND_ON,
STM_DELAY_SEND_OFF,
STM_REGISTER,
STM_DELAY_SEND_FAILURE,
        
};

enum DefineDevicePolling{
	SYSTEM_POLLING_READY,
	SYSTEM_POLLING_METER,
	SYSTEM_POLLING_WM,
	SYSTEM_POLLING_PYR,	
	SYSTEM_POLLING_SoilSensor,
	SYSTEM_POLLING_AirSensor,
	SYSTEM_POLLING_INV,
	SYSTEM_POLLING_BMS,
};

#define RS_NORM			0x00
#define RS_DATA_SYNC_ONE	0x01
#define RS_DATA_SYNC_ALL	0x02
#define RS_DATA_CHECK		0x03
#define RS_SEND_DATA		0x04

#define NODE2_CMD_ACK				0x00
#define NODE2_CMD_POWER_COUNTER		0x01
#define NODE2_CMD_METER_VALUE		0x02
#define NODE2_CMD_METER_VALUE_ALL	0x03
#define NODE2_CMD_METER_STATUS		0x04
#define NODE2_CMD_VALAUE_UPDATED		0x05
#define NODE2_CMD_GET_RECORD			0x06
#define NODE2_CMD_GET_INFO			0x07 

#define NODE1_CMD_POWER_ENABLE		0x01
#define NODE1_CMD_POWER_DISABLE	0x02
#define NODE1_CMD_VALUE_UPDATED	0x03
#define NODE1_CMD_LED_OFF			0x04

#define SHOW_WAIT_DELAY		0x00
#define SHOW_USER_INFO			0x01
#define SHOW_TIME				0x02
#define SHOW_SYSTEM_OFF			0x03
#define SHOW_VERSION			0x04
#define SHOW_LOW_VALUE			0x05
#define SHOW_LOGO				0x06
#define SHOW_INVALID			0x07
#define SHOW_SETTING			0x08
#define SHOW_FAILURE			0x09
#define SHOW_SYSTEM_FREE		0x0A

#define SET_POWER_ON 	1
#define SET_POWER_OFF	2

typedef struct STR_DOOR_RECORD {	
    uint8_t Status;	// 0 : Enter / 1: Exit     
    uint8_t UID[4];
    uint8_t DateTime[6];
	
} STR_DOOR_RCD ;

typedef struct STR_RECORD {	
	uint8_t UID[4];
	uint8_t StartTime[6];
	uint8_t EndTime[6];
	uint32_t StartMeter;
	uint32_t EndMeter;
	float  StartBalance;
	float  EndBalance;
} STR_POWER_RCD ;

typedef struct STR_CHG_RECORD {	
	uint8_t NumInCharge;
	uint8_t DateTime[6];	
	uint32_t PowerMeter;
} STR_TAG_CHG ;

typedef struct strUserInfo {
	uint8_t mode;				//  0x01 :  �p�O��
	uint8_t SID[4];				//�Ǹ�
	uint8_t UID[4];				// �d��	
	float  Balance;				//�l�B		
	uint8_t StartTime[6];		// ���ڶ}�l
	uint32_t StartMeter;		//���ڵ���	
	float StartBalance;			//���ڵ���	
} STR_UserInfo;

typedef struct STR_TIME_FORMAT {    
	uint8_t tYear;
	uint8_t tMonth;
	uint8_t tDay;
	uint8_t tHour;
	uint8_t tMin;
	uint8_t tSec;
	uint8_t t10mSec;	
} STR_TIME;

typedef struct STR_METER_TEABLE {    
    uint8_t BoardID;
    uint8_t PortID;    
} STR_METER_TB;

typedef struct STR_METER_DATA {   
uint32_t TotalWatt;	
uint8_t RelayStatus;
uint32_t V;
uint32_t I;
uint32_t F;
uint32_t P;
uint32_t S;
uint32_t PwrFactor;
uint32_t Balance;
uint8_t Mode;
} STR_METER_D;

typedef struct {
	uint8_t PowerMeter;
	uint8_t Bms;
	uint8_t WaterMeter;
	uint8_t Pyranometer;
	uint8_t	SoilSensor;
	uint8_t	AirSensor;	
	uint8_t Inverter;
}TotErrorRate_t;
#endif 


