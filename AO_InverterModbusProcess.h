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
	
	_Bool ChargingFlag;	// 0: not charging, 1: charging
	_Bool FaultFlag;		// 0: no,	1: yes
	_Bool WarnFlag;			// 0: No, 1: Yes
} InvData_t;

typedef struct {
	
	_Bool ConnectFlag;	// 0:	disconnect, 	1: connected
	_Bool ChargingFlag;	// 0: not charging, 1: charging
	_Bool FaultFlag;		// 0: no,	1: yes
	_Bool WarnFlag;	// 0: No, 1: Yes
} CtlrData_t;

typedef struct {
	_Bool Full;		// 0: not full,		1: full
	
	_Bool LoadWarnFlag;
	_Bool TempWarnFlag;
	_Bool LoadTimeoutWarnFlag;
	_Bool LoadOverWarnFlag;
	_Bool BatHighVoltWarnFlag;
	_Bool BatLowVoltWarnFlag;
	_Bool StoreDataErrWarnFlag;
	_Bool StoreOpFailWarnFlag;
	
	_Bool InvFuncErrWarnFlag;
	_Bool PlanShutdownWarnFlag;
	_Bool OutputWarnFlag;
	
	_Bool InvErrFaultFlag;
	_Bool TempOverFaultFlag;
	_Bool TempSensorFaultFlag;
	_Bool LoadTimeoutFaultFlag;
	_Bool LoadErrFaultFlag;
	_Bool LoadOverFaultFlag;
	_Bool BatHighVoltFaultFlag;
	_Bool BatLowVoltFaultFlag;
	_Bool PlanShutdownFaultFlag;
	_Bool OutputErrFaultFlag;
	_Bool ChipStartFailFaultFlag;
	_Bool CurrentSensorFaultFlag;

} BatData_t;


extern InvError_t InvError;
extern InvData_t	InvData;
extern BatData_t	BatData;
extern CtlrData_t CtrlData;

#endif