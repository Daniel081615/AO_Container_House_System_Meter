#ifndef	__AO_WATERMETERMODBUSPROCESS_H__
#define __AO_WATERMETERMODBUSPROCESS_H__

#include "stdint.h"
#include "AO_ExternFunc.h"

extern void WMPolling(void);
extern void WMDataProcess(void);

extern _Bool WMPollingFinishedFlag;
extern uint8_t PollingWMID;

enum Define_WM_Polling_State
{
		WM_POLLING_READY,
		WM_POLLING_RSP,
		//	Polling cmds below
	
		WM_POLLING_WATER_CONSUMPTION_CMD,
		WM_POLLING_VALVE_STATUS_CMD,
	
		//	Cmds are down below
		MBCMD_SET_WM_ADDR_AND_BAUDRATE,
		MBCMD_SET_WM_VALVE_OnOff,
};

enum DefineModbusWM_Cmds
{
		MDBS_GET_VALVE_STATUS,		
		MDBS_GET_TOTAL_WATER_CONSUMPTION,
		MDBS_SET_WM_DEVICE_ADDR_AND_BAUDRATE,
		MDBS_SET_VALVE_OnOff,
} ;

typedef struct{
	uint32_t WMDeviceNG;
	uint8_t Success[WtrMeterMax];
	uint8_t Fail[WtrMeterMax];
	uint8_t ErrorRate;
}WMError_t;

typedef struct {
		uint8_t ValveState;		
		uint32_t TotalVolume;
} WMData_t;

extern WMError_t WMError;
extern WMData_t  WMData[WtrMeterMax];

#endif	//AO_WaterMeterModbusProcess.h