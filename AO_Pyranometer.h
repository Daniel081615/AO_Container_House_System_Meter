#ifndef		__AO_PYRANOMETER_H__
#define		__AO_PYRANOMETER_H__

#include "AO_MyDef.h"
#include	"stdint.h"

extern void PyranometerPolling(void);
extern void PyrDataProcess(void);
extern void MODBUS_SendPYRCmd(uint8_t cmd);

extern _Bool PyrPollingFinishedFlag;
extern uint8_t 	PollingPyrMtrID;

enum DefinePyrPollingState{
		PYR_POLLING_READY,
		PYR_POLLING_RSP,
	
		PYR_GET_SOLAR_RADIATION,
		PYR_GET_DEVIATION_VALUE,	
		
		//	Set Pyranometer Cmds
		PYR_SET_DEVIATION_VALUE,
		PYR_SET_DEVICE_ADDRESS,
		PYR_SET_BAUD_RATE,
};



enum DefineModbusPYR{
		MDBS_GET_SOLAR_RADIATION,
		MDBS_GET_DEVIATION_VALUE,
		MDBS_SET_DEVIATION_VALUE,
		MDBS_GET_DEVICE_ADDRESS,
		MDBS_SET_DEVICE_ADDRESS,
		MDBS_GET_BAUD_RATE,
		MDBS_SET_BAUD_RATE,
		MDBS_PYRANO_OTHER,
};

typedef struct {
		uint16_t SolarRadiation;
		uint16_t OffsetValue;
} PyrMtrData_t;

typedef struct {
	uint32_t PyrDeviceNG;
	uint8_t Success[PyrMtrMax];
	uint8_t Fail[PyrMtrMax];
	uint8_t ErrorRate[PyrMtrMax];
} PyrMtrError_t;



extern PyrMtrData_t PyrMtrData[PyrMtrMax];
extern PyrMtrError_t PyrMtrError;

#endif		/***	AO_Pyranometer.h	***/