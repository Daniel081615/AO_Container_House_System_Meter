#ifndef		__AO_AIRSENSOR_H__
#define		__AO_AIRSENSOR_H__

#include "AO_MyDef.h"
#include	"stdint.h"
#include <stdint.h>

extern void AirSensorPolling(void);
extern void AirSensorDataProcess(void);

extern _Bool AirSensorPollingFinishedFlag;
extern uint8_t 	PollingAirSensorID;

enum DefineAirSensorPollingState{
		AS_POLLING_READY,
		AS_POLLING_RSP,
		AS_POLLING_CMD
};


typedef struct {
    uint16_t Co2;          
    uint16_t Formaldehyde; 
    uint16_t Tvoc;         
    uint16_t Pm25;         
    uint16_t Pm10;         
    uint16_t Temperature;  
    uint16_t Humidity;

} AirSensorData_t;


typedef struct {
	uint32_t ASDeviceNG;
	uint8_t Success[AirSensorMax];
	uint8_t Fail[AirSensorMax];
	uint8_t ErrorRate[AirSensorMax];
} AirSensorError_t;



extern AirSensorData_t AirSensorData[AirSensorMax];
extern AirSensorError_t AirSensorError;

#endif		/***	AO_AirSensor.h	***/