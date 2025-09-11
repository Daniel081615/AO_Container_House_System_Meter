#ifndef	__AO_INVERTERMODBUSPROCESS_H__
#define	__AO_INVERTERMODBUSPROCESS_H__

#include	"stdint.h"

/*** 	Inverter Functions	***/
void INVPolling(void);
void INVDataProcess(void);
void MODBUS_SendINVCmd(void);
void INVSuccess(void);
void INVTimeoutProcess(void);

extern void INVPolling(void);
extern void INVDataProcess(void);
extern _Bool INVPollingFinishedFlag;

typedef enum {
	
		MDBS_GET_INV_STATUS,
}DefineModbusINVCmds_t;

typedef enum {
		INV_POLLING_READY,
		INV_POLLING_RSP,
		//	Polling cmds below
		INV_POLLING_CMD

}INV_Polling_State_t;

typedef struct {
	uint8_t InvDeviceNG;
	uint8_t Success;
	uint8_t Fail;
	uint8_t ErrorRate;
} InvError_t;

typedef struct {
	uint8_t statusByte1;
	uint8_t statusByte3;
	uint8_t warnByte1;
	uint8_t warnByte2;
	uint8_t faultByte1;
	uint8_t faultByte2;
	uint8_t faultByte3;

	uint16_t InputVolt;
	uint16_t InputFreq;
	uint16_t OutputVolt;
	uint16_t OutputFreq;
	
	uint16_t BatVolt;
	uint8_t BatCapacity;
	uint8_t InvCurrent;
	uint8_t LoadPercentage;
	uint8_t MachineTemp;
	uint8_t MachineStatusCode;
	uint8_t SysStatus;
	
	uint16_t PV_volt;
	uint8_t CtrlCurrent;
	uint8_t CtrlTemp;
	uint8_t CtrlStatusCode;
	
} InvData_t;


extern InvError_t InvError;
extern InvData_t	InvData;

#endif