#ifndef		__AO_SOILSENSOR_H__
#define		__AO_SOILSENSOR_H__

#include "AO_MyDef.h"
#include	"stdint.h"

extern void SoilSensorPolling(void);
extern void SoilSensorDataProcess(void);
extern void MODBUS_SendSoilSensorCmd(uint8_t cmd);

extern _Bool SoilSensorPollingFinishedFlag;
extern uint8_t 	PollingSoilSensorID;

enum DefineSoilSensorPollingState{
		SS_POLLING_READY,
		SS_POLLING_RSP,
	
		SS_POLLING_MOISTURE,
    SS_POLLING_TEMPERATURE,
    SS_POLLING_EC,
    SS_POLLING_PH,
    SS_POLLING_NPK_VALUES,
    SS_POLLING_SALINITY,
    SS_POLLING_TDS,
    SS_POLLING_FERTILITY,
		SS_POLLING_COEFS,
		SS_POLLING_CALIB_VALUES,
    SS_POLLING_FERT_COEF,
    SS_POLLING_FERT_OFFSET,
    SS_POLLING_N_COEF,
    SS_POLLING_N_OFFSET,
    SS_POLLING_P_COEF,
    SS_POLLING_P_OFFSET,
    SS_POLLING_K_COEF,
    SS_POLLING_K_OFFSET,
		//	Set SoilSensor Cmds

    SS_SET_N_VALUE,
    SS_SET_P_VALUE,
    SS_SET_K_VALUE,
    SS_SET_EC_COEF,
    SS_SET_SALINITY_COEF,
    SS_SET_TDS_COEF,
    SS_SET_TEMP_CALIB,
    SS_SET_MOISTURE_CALIB,
    SS_SET_EC_CALIB,
    SS_SET_PH_CALIB,		
    SS_SET_FERT_COEF,
    SS_SET_FERT_OFFSET,
    SS_SET_N_COEF,
    SS_SET_N_OFFSET,
    SS_SET_P_COEF,
    SS_SET_P_OFFSET,
    SS_SET_K_COEF,
    SS_SET_K_OFFSET,		
    SS_GET_DEVICE_ADDRESS,
    SS_SET_DEVICE_ADDRESS,
    SS_GET_BAUD_RATE,
    SS_SET_BAUD_RATE,

};


/***
 *	@brief	enum : DefineModbusSoilSensor
 *	@note 	Can be simplify
 ***/
typedef enum DefineModbusSoilSensor
{
    MDBS_GET_MOISTURE,
    MDBS_GET_TEMPERATURE,
    MDBS_GET_EC,
    MDBS_GET_PH,
    MDBS_GET_NPK_VALUES,
    MDBS_GET_SALINITY,
    MDBS_GET_TDS,
    MDBS_GET_FERTILITY,
		MDBS_GET_COEFS,
		MDBS_GET_CALIB_VALUES,
    MDBS_GET_FERT_COEF,
    MDBS_GET_FERT_OFFSET,
    MDBS_GET_N_COEF,
    MDBS_GET_N_OFFSET,
    MDBS_GET_P_COEF,
    MDBS_GET_P_OFFSET,
    MDBS_GET_K_COEF,
    MDBS_GET_K_OFFSET,
	
	

    MDBS_SET_N_VALUE,
    MDBS_SET_P_VALUE,
    MDBS_SET_K_VALUE,
    MDBS_SET_EC_COEF,
    MDBS_SET_SALINITY_COEF,
    MDBS_SET_TDS_COEF,
    MDBS_SET_TEMP_CALIB,
    MDBS_SET_MOISTURE_CALIB,
    MDBS_SET_EC_CALIB,
    MDBS_SET_PH_CALIB,		
    MDBS_SET_FERT_COEF,
    MDBS_SET_FERT_OFFSET,
    MDBS_SET_N_COEF,
    MDBS_SET_N_OFFSET,
    MDBS_SET_P_COEF,
    MDBS_SET_P_OFFSET,
    MDBS_SET_K_COEF,
    MDBS_SET_K_OFFSET,		
    MDBS_GET_SS_ADDRESS,
    MDBS_SET_SS_ADDRESS,
    MDBS_GET_SS_BAUD_RATE,
    MDBS_SET_SS_BAUD_RATE,
		MDBS_SoilSensor_OTHER,

} DefineModbusPYR;


#include <stdint.h>


typedef struct {
    uint16_t Moisture;
    uint16_t Temperature;
    uint16_t EC;
    uint16_t PH;
    uint16_t Nitrogen;
    uint16_t Phosphorus;
    uint16_t Potassium;
    uint16_t Salinity;
    uint16_t TDS;
    uint16_t Fertility;

    uint16_t EC_Coef;
    uint16_t Salinity_Coef;
    uint16_t TDS_Coef;

    uint16_t Temp_Calib;
    uint16_t Moisture_Calib;
    uint16_t EC_Calib;
    uint16_t PH_Calib;

    uint32_t Fert_Coef;
    int16_t Fert_Deviation;

    uint32_t Nitrogen_Coef;
    int16_t Nitrogen_Deviation;

    uint32_t Phosphorus_Coef;
    int16_t Phosphorus_Deviation;

    uint32_t Potassium_Coef;
    int16_t Potassium_Deviation;

} SoilSensorData_t;


typedef struct {
	uint32_t SSDeviceNG;
	uint8_t Success[SoilSensorMax];
	uint8_t Fail[SoilSensorMax];
	uint8_t ErrorRate[SoilSensorMax];
} SoilSensorError_t;



extern SoilSensorData_t SoilSensorData[SoilSensorMax];
extern SoilSensorError_t SoilSensorError;

#endif		/***	AO_SoilSensor.h	***/