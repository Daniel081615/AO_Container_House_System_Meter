/*--------------------------------------------------------------------------- 
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
 
 * -------------------------------------------------------------------------- 
*/
#ifndef __AO_MY_DEF__
#define __AO_MY_DEF__

#define VERSION	 "V2020-04-24-01"

#include "stdint.h"
#include "ota_manager.h"

//	Per Meter Board
#define PwrMeterMax 1
#define BmsMax			1
#define WtrMeterMax 1
#define InvMax			1

//#define METER_TEST

#define MAX_MANAGER_E		5

#define MEMBER_MAX_NO		2
#define RECORD_MAX		2
#define POWER_RCD_MAX		3
#define TAG_CHG_MAX		3


//wdt max retry times
#define MAX_WDT_TRIES				4


// SystemMode : 
// 	bit 0 = 1 => Multi-User / 0 => Sign User 
// 	bit 1 = 1 => Auto Lock / 0 => Lock
// 	bit 2 = 1 => Inside Lock Enable / 0 => Disable
// 	bit 3 = 1 =>  
// 	bit 4 = 1 => 
// 	bit 5 = 1 => LCD Sleep Enable / 0 => Disable 
// 	bit 6 = 1 => AC Power Loss / 0 => AC Power
// 	bit 7 = 1 => No Power Control / 0 => Power Control 
#define SYSTEM_SINGLE_MODE				0x00
#define SYSTEM_MULTI_MODE					(0x01<<0)
#define SYSTEM_AUTO_LOCK_ENABLE			(0x01 << 1)
#define SYSTEM_INSIDE_LOCK_ENABLE			(0x01 << 2)

#define SYSTEM_LCD_SLEEP					(0x01<<5)
#define SYSTEM_AC_LOSS						(0x01<<6)
#define SYSTEM_POWER_ALWAYS				(0x01<<7)
#define SYSTEM_POWER_CONTROL				0x00


#define FLAG_IN_CHARGE			(0x01 << 0)
#define FLAG_PWD_ENABLED			(0x01 << 6)
#define FLAG_SUPER_USER			(0x01 << 7)

#define FLAG_USER_DATA_READY            (0x01 << 0 )
#define FLAG_SYSTEM_TIME_READY        (0x01 << 1 )
#define FLAG_POWER_METER_READY        (0x01 << 2 )

// FLAG : From Reader  
#define FLAG_READER_CARD			(0x01 << 0)
#define FLAG_READER_DOOR		(0x01 << 1)
#define FLAG_METER_CHG_BALANCE	(0x01 << 2)
#define FLAG_METER_GET_RCD		(0x01 << 3)
//#define FLAG_METER_INIT_BIT		(0x01 << 4)
//#define FLAG_METER_INIT_BIT		(0x01 << 5)
//#define FLAG_METER_INIT_BIT		(0x01 << 6)
//#define FLAG_METER_INIT_BIT		(0x01 << 7)

// fgFromHostFlag : Flag From Host 
#define TO_METER_CHG_MODE				(0x01 << 0)
#define TO_METER_CHG_BALANCE				(0x01 << 1)
#define TO_METER_OPEN_DOOR				(0x01 << 2)
#define TO_METER_USER_JOIN_CGR			(0x01 << 3)
#define TO_METER_USER_EXIT_CGR			(0x01 << 4)
#define TO_METER_CHG_USER_MODE			(0x01 << 5)
#define FLAG_SYS_RST						(0x01 << 6)
#define FLAG_SYS_INIT						(0x01 << 7)


// fgToMeterFlag : Send to Meter
#define TO_METER_CHG_MODE				(0x01 << 0)
#define TO_METER_CHG_BALANCE				(0x01 << 1)
#define TO_METER_OPEN_DOOR				(0x01 << 2)
#define TO_METER_USER_JOIN_CGR			(0x01 << 3)
#define TO_METER_USER_EXIT_CGR			(0x01 << 4)
//#define FLAG_METER_SHOW_MSG				(0x01 << 5)
#define FLAG_SYS_RST						(0x01 << 6)
#define FLAG_SYS_INIT						(0x01 << 7)


//#define FLAG_METER_SHOW_MSG			(0x01 << 5)
//#define FLAG_METER_CLEAR_FLAG			(0x01 << 6)
//#define FLAG_METER_SYS_RESET			(0x01 << 7)



// FLAG for Meter System Operation
#define FLAG_METER_CHR_UPDATED			(0x01 << 0)
#define FLAG_METER_ADD_BAL				(0x01 << 1)
#define FLAG_METER_DEC_BAL				(0x01 << 2)
#define FLAG_METER_CHG_ROOM_MODE		(0x01 << 3)
#define FLAG_METER_OPEN_ROOM_DOOR		(0x01 << 4)
#define FLAG_METER_SHOW_MSG			(0x01 << 5)
#define FLAG_METER_CLEAR_FLAG			(0x01 << 6)
#define FLAG_METER_SYS_RESET				(0x01 << 7)


// FLAG for Reader Function 
#define FLAG_READER_INIT			(0x01 << 0)
#define FLAG_READER_CHG_MODE		(0x01 << 1)
#define FLAG_READER_CHG_DATA	(0x01 << 2)
#define FLAG_READER_GET_RCD		(0x01 << 3)

//#define FLAG_METER_INIT_BIT		(0x01 << 4)
//#define FLAG_METER_INIT_BIT		(0x01 << 5)
//#define FLAG_METER_INIT_BIT		(0x01 << 6)
//#define FLAG_METER_INIT_BIT		(0x01 << 7)



// Flag : From Reader Flag 
#define FLAG_READER_USER_JOIN				(0x01 << 0)
#define FLAG_READER_USER_EXIT				(0x01 << 1)
#define FLAG_READER_DOOR_RCD_NEW		(0x01 << 2)
#define FLAG_READER_ROOM_MODE_UPDATED	(0x01 << 3)
//#define FLAG_READER_OPEN_ROOM_DOOR		(0x01 << 4)
//#define FLAG_READER_SHOW_MSG				(0x01 << 5)
//#define FLAG_READER_CLEAR_FLAG				(0x01 << 6)
#define FLAG_READER_SYS_RESET				(0x01 << 7)


// FLAG fgToMeterRSPFlag ( From Host )
#define TO_METER_RSP_CLEAR_DATAUPDATED 	(0x01 << 7)

// FLAG : DataUpdated 
#define FLAG_DATA_PWR_RCD_NEW			(0x01 << 0)
#define FLAG_DATA_DOOR_RCD_NEW			(0x01 << 1)
#define FLAG_DATA_TAG_RCD_NEW			(0x01 << 2)
#define FLAG_NUM_CGR_UPDATED			(0x01 << 3)
#define FLAG_DATA_CHG_BAL				(0x01 << 4)
#define FLAG_METER_VALUE_UPDATED		(0x01 << 5)
#define FLAG_DATA_CHG_MODE				(0x01 << 6)


// FLAG fgToHostFlag : To Center 
#define TO_CENTER_CHG_BALANCE		(0x01 << 0)
#define TO_CENTER_CHG_MODE			(0x01 << 1)
#define TO_CENTER_CHG_USER_MODE		(0x01 << 2)
#define TO_CENTER_PWR_RCD_NEW		(0x01 << 4)
#define TO_CENTER_DOOR_RCD_NEW		(0x01 << 5)
#define TO_CENTER_INIT_METER                (0x01 << 6)
#define TO_CENTER_CHG_USER_INFO		(0x01 << 7)


// FLAG fgToReaderFlag : To Reader
#define TO_READER_CHG_BALANCE		(0x01 << 0)
#define TO_READER_CHG_MODE			(0x01 << 1)
#define TO_READER_CHG_USER_MODE		(0x01 << 2)
#define TO_READER_OPEN_DOOR			(0x01 << 3)
#define TO_READER_GOT_DOOR_RCD		(0x01 << 4)
//#define FLAG_READER_SHOW_MSG			(0x01 << 5)
#define TO_READER_SYS_RST			(0x01 << 6)
#define TO_READER_SYS_INIT			(0x01 << 7)



#define DOOR_RCD		0x01
#define POWER_RCD		0x02
#define TAG_CHG_RCD		0x03
#define DOOR_RCD_DIR	0x04
#define POWER_RCD_DIR	0x05
#define TAG_CHG_RCD_DIR	0x06


#define METER_MAX1 	10
#define METER_MAX2 	10
#define METER_MAX3 	10

#define TC_VALUE	0x2800

#define METER_POLL_INTERVAL	50

#define RS485_TIMEOUT_TIME		25
#define WAIT_TIMEOUT_TIME			10
#define POLL_TIMEOUT_TIME			25
#define MAX_RETRY_TIMES			5
#define POLL_DELAY_TIME				2

#define MAX_METER_TXQ_LENGTH    	25
#define MAX_METER_RXQ_LENGTH    	16
#define METER_TOKEN_LENGTH			25

#define HOST_TOKEN_LENGTH			100
#define MAX_HOST_TXQ_LENGTH    	100
#define MAX_HOST_RXQ_LENGTH		100


#define MAX_READER_TXQ_LENGTH     	100
#define MAX_READER_RXQ_LENGTH     	100
#define READER_TOKEN_LENGTH		100

#define POWER_100W_COUNTER		160

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


//#define ROOM_POWER_On()		(PF7 = 1); (PA3 = 1)
//#define ROOM_POWER_Off()		(PF7 = 1); (PA3 = 0)

// SERCOM 1
#define DIR_READER_RS485_In()		//(PE11 = 0)
#define DIR_READER_RS485_Out()		//(PE11 = 1)
// SERCOM 2
#define DIR_METER_RS485_In()		//(PC1 = 0)
#define DIR_METER_RS485_Out()		//(PC1 = 1)
// Reader Power Switch
#define READER_POWER_On()			(PD3 = 1)
#define READER_POWER_Off()			(PD3 = 0)
//#define READER_POWER_On()			(PB0 = 1)
//#define READER_POWER_Off()			(PB0 = 0)



#define SET_RECORD_READY() 		fgToHostFlag |= 0x40


#define SYSTEM_POWER_ON		0x01
#define SYSTEM_POWER_OFF		0x02

#define DAE_530n540				0x01 
#define CIC_BAW2A				0x02
#define TUTANG_E21nE31			0x03
#define DEM_510c					0x04

enum DEFINE_RS485_READER_TOKEN {
	READER_CMD_ALIVE=0x10,			//0x10			
	READER_CMD_CHG_MODE, 			//0x11
	READER_CMD_CHG_BALANCE,			//0x12
	READER_CMD_CHG_USER_INFO,		//0x13
	READER_CMD_OPEN_DOOR,			//0x14
	READER_CMD_GET_SYS_INFO, 			//0x15	
	READER_CMD_GET_DOOR_RCD,		//0x16
	READER_CMD_SHOW_MSG,			//0x17
	READER_CMD_SHOW_ID,				//0x18			
	READER_CMD_LCD_SHOW,			//0x19
	READER_CMD_SYS_SW,				//0x1A
	READER_CMD_CLEAR_FLAG,			//0x1B
	
	
	READER_RSP_ACK=0x30, 			//0x30
	READER_RSP_ALIVE,				//0x31
	READER_RSP_SYS_INFO, 				//0x32
	READER_RSP_USER_BAL, 				//0x33
	READER_RSP_DOOR_RCD,			//0x34
	
};

enum DEFINE_RS485_CENTER_TOKEN {

CTR_ALIVE=0x10,				//0x10			
CTR_GET_POWER_ALL,			//0x11
CTR_GET_ERROR_TABLE,			//0x12
CTR_SYS_SW,					//0x13
CTR_RSP_SYS_INFO,				//0x14
CTR_RSP_GET_ERROR_TABLE,	//0x15
CTR_READER_INIT,				//0x16
CTR_READER_INIT_ONE,			//0x17
CTR_SET_USER_INFO,			//0x18
CTR_RSP_ACK,					//0x19
CTR_RSP_POWER_ALL,			//0x1A
CTR_RSP_ERROR_TABLE,			//0x1B
CTR_ADD_VALUE,				//0x1C
CTR_DEC_VALUE,					//0x1D
CTR_CHG_ROOM_SETTING,		//0x1E
CTR_POWER_ON,				//0x1F
CTR_POWER_OFF,				//0x20
CTR_GET_RECORD,				//0x21
CTR_RSP_GET_RECORD,			//0x22
CTR_ALIVE_INIT,					//0x23
CTR_BOOTLOADER_INIT,			//0x24
CTR_BOOTLOADER_ACK,			//0x25
CTR_ReBotSuccess_ACK,			//0x26
CTR_ReBotFalid_ACK, 			//0x27
CTR_SAVE_INFO,					//0x28
CTR_GET_USER_INFO,			//0x29
CTR_RSP_USER_INFO,			//0x2A
CTR_METER_RESET,				//0x2B
CTR_FIRST_RESET_STATUS, 		//0x2C
CTR_OPEN_DOOR,				//0x2D

};

enum DEFINE_POWER_METER_STATE {
	MP_READY,	
	MP_POLLING_RSP,
	MP_POLLING_W_CMD,	
	MP_POLLING_RELAY_CMD,	
			
	MP_SET_RELAY,
	MP_SET_ADDR,
	
	MP_ERROR,
};


#define INIT_S1	0
#define INIT_S2 	1
#define INIT_S3 	2
#define INIT_S4 	3
#define INIT_S5 	4
#define INIT_S6 	5
#define INIT_S7 	6
#define INIT_S8	7
#define INIT_S9	8
#define INIT_S10	9

// Room Mode
#define RM_NULL					0
#define RM_POWER_OFF_READY		1
#define RM_POWER_ON_READY		2
#define RM_FREE_MODE_READY		3
#define RM_STOP_MODE_READY		4
#define RM_SUPER_MODE_READY		5
#define RM_ERROR					6

// Time Index
#define INX_YEAR		0
#define INX_MON			1
#define INX_DAY			2
#define INX_HOUR		3
#define INX_MIN			4
#define INX_SEC			5
#define INX_WEEK		6
#define INX_TIME_START_Y	(READER_TOKEN_LENGTH-9)
#define INX_TIME_START_M	(READER_TOKEN_LENGTH-8)
#define INX_TIME_START_D	(READER_TOKEN_LENGTH-7)
#define INX_TIME_START_H	(READER_TOKEN_LENGTH-6)
#define INX_TIME_START_MN	(READER_TOKEN_LENGTH-5)
#define INX_TIME_START_S	(READER_TOKEN_LENGTH-4)
#define INX_TIME_START_W	(READER_TOKEN_LENGTH-3)

#define NODE_CLEAR_CARD_FLAG			0x01
#define NODE_CLEAR_BUTTON_FLAG		0x02
#define NODE_UPDATE_USER_INFO		0x04
#define NODE_SAVE_USER_INFO			0x08
#define NODE_UPDATE_ROOM_INFO		0x10
#define NODE_DATA_DIRECT				0x80

#define NODE_TIME_SYNC				0x40

#define RELAY_ON	0x01
#define RELAY_OFF	0x00

// Meter => Center

#define MIN_METER_DI_TIMES		10
#define SEND_NODE2_Q_MAX		5

#define MIN_BUTTON_DI_TIMES	250

// Status Code (Host)
#define STATUS_MY_TOKEN_FINISH			0xD0
#define STATUS_MY_TOKEN_NC				0xD1
#define STATUS_NOT_MY_TOKEN				0xE0

#define STATUS_ERROR					0xF0
#define STATUS_ERROR_NODE2_UART			0xF1
#define STATUS_ERROR_NODE1_UART		0xF2

// Response Code (Reader)
#define STATUS_NODE1_TOKEN_FINISH		0xA0
#define STATUS_NODE1_TOKEN_NC			0xA1



// Response Code (Host)
#define RSP_CMD_RESULT				0xD0
#define RSP_REQ_RESULT				0xD1
#define RSP_MY_TOKEN_FINISH			0xD2
#define RSP_MY_TOKEN_NC				0xD3

#define RSP_NOT_MY_TOKEN			0xE0

#define RSP_ERROR					0xF0
#define RSP_ERROR_NODE2_UART			0xF1
#define RSP_ERROR_METER_UART		0xF2

// Response Code (METER)
#define RSP_METER_TOKEN_FINISH		0xA0
#define RSP_METER_TOKEN_NC			0xA1

#define CHARGE_MODE_FREE			0x01
#define CHARGE_MODE_CHARGE			0x02
#define CHARGE_MODE_STOP			0x03

enum DEFINE_RS485_METER_TOKEN {

METER_CMD_ALIVE=0x10,				//0x10			
METER_GET_CMD_POWER_METER,
METER_GET_CMD_BMS,
METER_GET_CMD_WATER_METER,
METER_GET_CMD_INV,	

CMD_MTR_OTA_UPDATE = 0x20,	
CMD_MTR_SWITCH_FWVER,
CMD_GET_MTR_FW_STATUS,
CMD_MTR_FW_REBOOT,

METER_RSP_ACK=0x30,			//0x30
METER_RSP_SYS_INFO,				//0x31
METER_RSP_USER_DATA,		//0x32
METER_RSP_POWER_DATA,			//0x33
METER_RSP_BMS_DATA,			//0x34
METER_RSP_WM_DATA,				//0x35
METER_RSP_INV_DATA,			//0x36

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
	uint8_t mode;				//  0x01 :  計費中
	uint8_t SID[4];				//學號
	uint8_t UID[4];				// 卡號	
	float  Balance;				//餘額		
	uint8_t StartTime[6];		// 扣款開始
	uint32_t StartMeter;		//扣款結束	
	float StartBalance;			//扣款結束	
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
} STR_METER_D;
#endif 


