#ifndef		__AO_PYRANOMETER_H__
#define		__AO_PYRANOMETER_H__

#include "AO_MyDef.h"
#include	"stdint.h"

extern void PyranometerPolling(void);
extern void PyrDataProcess(void);

extern _Bool PyrPollingFinishedFlag;
extern uint8_t 	PollingPyrID;

enum DefinePyrPollingState{
		PYR_POLLING_READY,
		PYR_POLLING_RSP,
	
		PYR_GET_SOLAR_RADIATION,
		PYR_GET_OFFSET_VALUE,	
		
		//	Set Pyranometer Cmds
		PYR_SET_OFFSET_VALUE,
		PYR_SET_DEVICE_ADDRESS,
		PYR_SET_BAUD_RATE,
};

enum DefineModbusPyrCmd{
		
		MBPYRCMD_SET_OFFSET_VALUE,
		MBPYRCMD_SET_DEVICE_ADDRESS,
		MBPYRCMD_SET_BAUD_RATE,
		MBPYRCMD_READY
};



enum DefineModbusPYR{
		MDBS_GET_SOLAR_RADIATION,
		MDBS_GET_OFFSET_VALUE,
		MDBS_SET_OFFSET_VALUE,
		MDBS_GET_DEVICE_ADDRESS,
		MDBS_SET_DEVICE_ADDRESS,
		MDBS_GET_BAUD_RATE,
		MDBS_SET_BAUD_RATE,
		MDBS_PYRANO_OTHER,
};

typedef struct {
		uint16_t SolarRadiation;
		uint16_t OffsetValue;
} PyrMeterData_t;

typedef struct {
	uint32_t PyrDeviceNG;
	uint8_t Success[PyrMeterMax];
	uint8_t Fail[PyrMeterMax];
	uint8_t ErrorRate[PyrMeterMax];
} PyrError_t;



extern PyrMeterData_t PyrMeterData[PyrMeterMax];
extern PyrError_t PyrError;

#endif		/***	AO_Pyranometer.h	***/