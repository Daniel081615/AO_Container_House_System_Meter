/***
 *	@file		AO_AirSensor.c
 *	@breif	AirSensor modbus cmds, polling modules
 *	@type		SN3002TR
 *	@abbreviate	AS -> AirSensor
 ***/
 
#include	"AO_AirSensor.h"
#include 	"NUC1261.h"
#include	"AO_ExternFunc.h"
#include	"AO_ModBusProcess.h"

void AirSensorPolling(void);
void AirSensorDataProcess(void);
void MODBUS_SendAirSensorCmd(void);
void AirSensorSuccess(void);
void AirSensorTimeoutProcess(void);

AirSensorData_t AirSensorData[AirSensorMax];
AirSensorError_t AirSensorError;

_Bool 		AirSensorPollingFinishedFlag;
uint8_t 	PollingAirSensorID;
uint8_t 	AirSensorMBCMD;
uint8_t 	AirSensorPollingState, AirSensorPollingStateIndex, AirSensorModbusCmd;
uint8_t 	AirSensorReadErrorCnt;
/***
 *	@function		AirSensorPolling()
 *	@brief			polling soil_sensor in sequence. and record the communication.
 *	@note				Build host structure.
 *	2025.09.02
 ***/
void AirSensorPolling(void)
{
		switch(AirSensorPollingState)	
		{
				case	AS_POLLING_READY:

						AirSensorPollingState = AS_POLLING_CMD + AirSensorPollingStateIndex;
				
						if (AirSensorPollingState > AS_POLLING_CMD)
						{
								PollingAirSensorID++;
								AirSensorPollingState = AS_POLLING_CMD;
								AirSensorPollingStateIndex = 0;
						}
						break;
						
				case	AS_POLLING_RSP:
            if (GotDeviceRsp == AirSensorIDArray[PollingAirSensorID-1])
            {
                AirSensorSuccess();
            } else {
                AirSensorTimeoutProcess();
            }
						
					break;
				
				case	AS_POLLING_CMD:
						MODBUS_SendAirSensorCmd();
						AirSensorPollingState = AS_POLLING_RSP;
						TickPollingInterval = 0 ;	
					break;
				
		}
}

/***
 *	@brief			Parse Modbus massage from Air Sensor
 ***/
void AirSensorDataProcess(void)
{
		uint8_t AirSensorIdx;
		
		if (GotDeviceRsp != AirSensorIDArray[PollingAirSensorID-1]) return;	//	check the host address idx
		
		AirSensorIdx = PollingAirSensorID -1;
	
    AirSensorData[AirSensorIdx].Co2 					= ((TokenMeter[3] << 8) | TokenMeter[4]);          
    AirSensorData[AirSensorIdx].Formaldehyde = ((TokenMeter[5] << 8) | TokenMeter[6]); 
    AirSensorData[AirSensorIdx].Tvoc 				= ((TokenMeter[7] << 8) | TokenMeter[8]);         
    AirSensorData[AirSensorIdx].Pm25         = ((TokenMeter[9] << 8) | TokenMeter[10]); 
    AirSensorData[AirSensorIdx].Pm10         = ((TokenMeter[11] << 8) | TokenMeter[12]); 
    AirSensorData[AirSensorIdx].Temperature	= ((TokenMeter[13] << 8) | TokenMeter[14]);  
    AirSensorData[AirSensorIdx].Humidity			= ((TokenMeter[15] << 8) | TokenMeter[16]);  
}		

/***
 *	@brief	Send AirSensor modbus cmd
 ***/
void MODBUS_SendAirSensorCmd(void)
{
		GotDeviceRsp = 0xff;
		MeterTxBuffer[0] = AirSensorIDArray[PollingAirSensorID-1] ; 
		//	To determine if Device respond.
		MeterTxBuffer[1] = 0x03;
		MeterTxBuffer[2] = 0x00;
		MeterTxBuffer[3] = 0x02;
		MeterTxBuffer[4] = 0x00;
		MeterTxBuffer[5] = 0x07;
		CRC16(MeterTxBuffer, 6);
		MeterTxBuffer[6] = uchCRCHi;
		MeterTxBuffer[7] = uchCRCLo;
		_SendStringToMETER(MeterTxBuffer, 8);		
}

void AirSensorSuccess(void)
{
		uint8_t AirSensorIdx;
		AirSensorIdx = PollingAirSensorID -1; 	

		AirSensorError.Success[AirSensorIdx] += 1;
		AirSensorError.ASDeviceNG &= (~(0x00000001 << (AirSensorIdx)));
		//	Move to next pollingstate
		AirSensorPollingStateIndex++;    
		AirSensorPollingState = AS_POLLING_READY;			
}

void AirSensorTimeoutProcess(void)
{
		uint8_t AirSensorIdx;
		AirSensorIdx = PollingAirSensorID -1; 	
	
    if ( TickPollingInterval > POLL_TIMEOUT )
    {							
        AirSensorReadErrorCnt++;
				AirSensorError.Fail[AirSensorIdx] += 1;
				AirSensorPollingState = AS_POLLING_READY;
        if( AirSensorReadErrorCnt > MAX_POLL_RETRY_TIMES )
        {
						AirSensorError.ASDeviceNG |= (0x00000001 << (AirSensorIdx));
            PollingAirSensorID++;
            AirSensorReadErrorCnt = 0 ;            
            ResetMeterUART();
        }
    }
}
