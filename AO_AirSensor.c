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
						if (TickReadPowerTime >= ReadMeterTime)
						{
								TickReadPowerTime = 0 ;	
								AirSensorPollingState = AS_POLLING_CMD + AirSensorPollingStateIndex;
								
								if (AirSensorPollingState > AS_POLLING_CMD)
								{
										PollingAirSensorID++;
										AirSensorPollingState = AS_POLLING_CMD;
										AirSensorPollingStateIndex = 0;
								}
						}
				case	AS_POLLING_RSP:
            if (GotDeviceRsp == PollingAirSensorID)
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
		uint8_t AirSensorArrayIndex;
		uint16_t u16tmp;
		uint32_t u32tmp;
		AirSensorArrayIndex = GotDeviceRsp -1;
	
		if (GotDeviceRsp != PollingAirSensorID) return;	//	check the host address idx
	
    AirSensorData[AirSensorArrayIndex].Co2 					= ((TokenMeter[3] << 8) | TokenMeter[4]);          
    AirSensorData[AirSensorArrayIndex].Formaldehyde = ((TokenMeter[5] << 8) | TokenMeter[6]); 
    AirSensorData[AirSensorArrayIndex].Tvoc 				= ((TokenMeter[7] << 8) | TokenMeter[8]);         
    AirSensorData[AirSensorArrayIndex].Pm25         = ((TokenMeter[9] << 8) | TokenMeter[10]); 
    AirSensorData[AirSensorArrayIndex].Pm10         = ((TokenMeter[11] << 8) | TokenMeter[12]); 
    AirSensorData[AirSensorArrayIndex].Temperature	= ((TokenMeter[13] << 8) | TokenMeter[14]);  
    AirSensorData[AirSensorArrayIndex].Humidity			= ((TokenMeter[15] << 8) | TokenMeter[16]);  
}		

/***
 *	@brief	Send pyranometer modbus cmd
 ***/
void MODBUS_SendAirSensorCmd(void)
{
		MeterTxBuffer[0] = PollingAirSensorID ; 
		//	To determine if Device respond.
		GotDeviceRsp = 0xFF ;
		MeterTxBuffer[1] = 0x03;
		MeterTxBuffer[2] = 0x00;
		MeterTxBuffer[3] = 0x07;
		MeterTxBuffer[4] = 0x00;
		MeterTxBuffer[5] = 0x07;
		CRC16(MeterTxBuffer, 6);
		MeterTxBuffer[6] = uchCRCHi;
		MeterTxBuffer[7] = uchCRCLo;
		_SendStringToMETER(MeterTxBuffer, 8);		
}

void AirSensorSuccess(void)
{
		uint8_t AirSensorArrayIndex;
		AirSensorArrayIndex = GotDeviceRsp -1;

		AirSensorError.Success[AirSensorArrayIndex] += 1;
		AirSensorError.ASDeviceNG &= (~(0x00000001 << (PollingAirSensorID -1)));
		//	Move to next pollingstate
    PollingAirSensorID++;
		AirSensorPollingState = AS_POLLING_READY;			
}

void AirSensorTimeoutProcess(void)
{
		uint8_t AirSensorArrayIndex;
		AirSensorArrayIndex = PollingAirSensorID -1;	
	
    if ( TickPollingInterval > POLL_TIMEOUT )
    {							
        AirSensorReadErrorCnt++;
				//	Error report system    
				AirSensorError.Fail[AirSensorArrayIndex] += 1;
				AirSensorPollingState = AS_POLLING_READY;
        if( AirSensorReadErrorCnt > MAX_POLL_RETRY_TIMES )
        {
						AirSensorError.ASDeviceNG |= (0x00000001 << (PollingAirSensorID -1));
            PollingAirSensorID++;
            AirSensorReadErrorCnt = 0 ;            
            ResetMeterUART();
        }
    }
}
