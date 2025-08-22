/*--------------------------------------------------------------------------- 
 * File Name     : MAIN.C
 * Company 		 : AOTECH
 * Author Name   : Barry Su
 * Starting Date : 2015.7.1
 * C Compiler : Keil C
 * --------------------------------------------------------------------------*/
#include <stdio.h>
#include "NUC1261.h"
#include "AO2022_MeterModule_1261.h"
#include "AO_MyDef.h"
#include "AO_ExternFunc.h"
#include "AO_ExternFunc.h"
#include "AO_HostProcess.h"
#include "AO_MyDef.h"
#include "AO_ModBusProcess.h"
#include "AO_BMSModBusProcess.h"

void MeterPolling(void);
void CalChecksumM(void);
void MeterDataProcess(void);
void SendMeter_SystemSW(void);
void MeterSuccess(void);
void MeterTimeoutProcess(void);
uint8_t _SendStringToMETER(uint8_t *Str, uint8_t len);
uint8_t GapErrorTimes[PwrMeterMax];
uint8_t TickMeter,MeterRelayStatus;
_Bool MeterRelayStausReady,fgMeterAddValue;
uint8_t PowerMeterReadErrorCnt;
uint8_t PollingStateIndex;
uint8_t PollingMeterID;

uint16_t Tick1S_ErrorRateGap;
uint32_t    PowerMeterNG;

uint8_t  MeterErrorRate5Min_Wp;
uint16_t MeterErrorRate5Min_Tx[PwrMeterMax][12];
uint16_t MeterErrorRate5Min_Rx[PwrMeterMax][12];

_Bool MeterPollingFinishedFlag;

uint8_t u8MeterInitState;
uint16_t Tick10mSec_PowerMeterInit;

//uint16_t TickReadPowerTime;


/***
 *	@brief	Need to revise the Cmd Process of DEM_510c
 ***/

void MeterPolling(void)
{	   
    switch(MeterPollingState)
    {
        case MP_READY :

            if (TickReadPowerTime >= ReadMeterTime)
            {
                TickReadPowerTime = 0 ;								
                MeterPollingState = MP_POLLING_W_CMD + PollingStateIndex ;
								/***
								 *	@brief	For ESG Condition, only poll two values of PowerMeter *DEM510c, @TotalWatt, @RelayStatus
								 ***/
                if ( MeterPollingState > MP_POLLING_RELAY_CMD )
                {
                    MeterPollingState = MP_POLLING_W_CMD ;
                    PollingStateIndex = 0 ;
                    PollingMeterID++;
                    if ( PollingMeterID > MaxPowerMeter )
                    {
                        PollingMeterID = 1 ;
                    }
										MeterPollingFinishedFlag = TRUE;
                }
                PowerMeterID = PollingMeterID ;
            } else {
                // Commands 
                PwrMtrModbusCmd = PwrMeterCmdList[PollingMeterID-1] ;
                PwrMeterCmdList[PollingMeterID-1] = MBPMCMD_READY ;
                PowerMeterID = PollingMeterID ;
                switch ( PwrMtrModbusCmd )	
                {
                    case MBPMCMD_READY :
                        break;
                    case MBPMCMD_RELAY_ON :
                        RelayOnOff = 1 ;
                        MeterPollingState = MP_SET_RELAY ;
                        PwrMtrModbusCmd = MBPMCMD_READY;
                        break;
                    case MBPMCMD_RELAY_OFF :										
                        RelayOnOff = 0 ;
                        MeterPollingState = MP_SET_RELAY ;
                        PwrMtrModbusCmd = MBPMCMD_READY;
                        break;
										case MBPMCMD_SET_ADDR:
												MeterPollingState = MP_SET_ADDR ;
                        PwrMtrModbusCmd = MBPMCMD_READY;
												break;
                    default :
                        break;
                }
            }
            break;
        case MP_POLLING_RSP :
            if (GotDeviceRsp == PowerMeterID)
            {
                MeterSuccess();
            } else {
                MeterTimeoutProcess();
            }
            break;
        case  MP_POLLING_W_CMD:
            MODBUS_SendCmd(MDBS_METER_GET_WATT);
            MeterPollingState = MP_POLLING_RSP ; 
            TickPollingInterval = 0 ;
            break;		
        case MP_POLLING_RELAY_CMD :	

            MODBUS_SendCmd(MDBS_METER_GET_RELAY);
            MeterPollingState = MP_POLLING_RSP ; 
            TickPollingInterval = 0 ;			
            break;
        case MP_SET_RELAY :	
            MODBUS_SendCmd(MDBS_METER_SET_RELAY);
            MeterPollingState = MP_READY ;
            break;		
        case MP_SET_ADDR :	
            MODBUS_SendCmd(MDBS_METER_SET_ADDR);
            MeterPollingState = MP_READY ;
            break;			
        default :
            MeterPollingState = MP_READY ;
            break;
    }
}

void MeterDataProcess(void)
{
		uint32_t u32temp,GapMeterPower;
    uint8_t u8PowerMeterID;
    
		u8PowerMeterID = TokenMeter[0];
		if ( u8PowerMeterID > 0 ) 
				u8PowerMeterID = u8PowerMeterID - 1;
		else 
				return;        
        
		if ( TokenMeter[1] == 0x03 )
		{		
				if ( MeterMBCmd == MDBS_METER_GET_WATT )
				{	
						switch ( MeterType )
						{
							case DAE_530n540 :
								u32temp = (TokenMeter[5] << 24) + (TokenMeter[6] << 16) + (TokenMeter[3] << 8) + TokenMeter[4] ;
								break;
							case CIC_BAW2A :
								u32temp = (TokenMeter[3] << 24) + (TokenMeter[4] << 16) + (TokenMeter[5] << 8) + TokenMeter[6] ;
								break;
							case TUTANG_E21nE31 :
								u32temp = (TokenMeter[5] << 24) + (TokenMeter[6] << 16) + (TokenMeter[3] << 8) + TokenMeter[4] ;
								break;
							case DEM_510c:
								u32temp = (TokenMeter[5] << 24) + (TokenMeter[6] << 16) + (TokenMeter[3] << 8) + TokenMeter[4] ;
								
							default :
								u32temp = (TokenMeter[5] << 24) + (TokenMeter[6] << 16) + (TokenMeter[3] << 8) + TokenMeter[4] ;
								break;
						}
						
						TotalWattValue_Now = MeterData[u8PowerMeterID].TotalWatt;
						//	if Initialize
						if ( PowerOnReadMeter )
						{
								PowerOnReadMeter = 0 ;
								TotalWattValue_Now = u32temp ;
								LastMeterPower[u8PowerMeterID] = TotalWattValue_Now ;
								MeterData[u8PowerMeterID].TotalWatt = TotalWattValue_Now ;
								
								RoomData.Status &= (~FLAG_POWER_METER_READY) ;
								GapErrorTimes[u8PowerMeterID] = 0 ;
						}
						
						if ( u32temp >= TotalWattValue_Now )
						{
								if (u32temp == TotalWattValue_Now) return;

								GapMeterPower = u32temp - TotalWattValue_Now;
								if ( GapMeterPower < 200 )
								{
										TotalWattValue_Now = u32temp ;
										LastMeterPower[u8PowerMeterID] = TotalWattValue_Now;
										MeterData[u8PowerMeterID].TotalWatt = TotalWattValue_Now ;
										RoomData.Status &= (~FLAG_POWER_METER_READY) ;												
										GapErrorTimes[u8PowerMeterID] = 0 ;
								} else {
										GapErrorTimes[u8PowerMeterID] ++;
										if ( GapErrorTimes[u8PowerMeterID] > 3 ) 
										{
												TotalWattValue_Now = u32temp ;
												LastMeterPower[u8PowerMeterID] = TotalWattValue_Now;						
												GapErrorTimes[u8PowerMeterID] = 0 ;
												MeterData[u8PowerMeterID].TotalWatt = TotalWattValue_Now ;
												//WritePowerToEE();
												RoomData.Status &= (~FLAG_POWER_METER_READY) ;
										}
								}
						} else {
								GapErrorTimes[u8PowerMeterID] ++;
								if ( GapErrorTimes[u8PowerMeterID] > 3 )
								{
									TotalWattValue_Now = u32temp ;
									LastMeterPower[u8PowerMeterID] = TotalWattValue_Now;	
									MeterData[u8PowerMeterID].TotalWatt = TotalWattValue_Now ;
									//WritePowerToEE();
									GapErrorTimes[u8PowerMeterID] = 0 ;
									RoomData.Status &= (~FLAG_POWER_METER_READY) ;
								}		
						}		
				}
				

				if ( MeterMBCmd == MDBS_METER_GET_RELAY )
				{
						switch ( MeterType )
						{
								case DAE_530n540 :
									if (TokenMeter[4] == 0x00)
									{
										MeterRelayStatus = 0x01 ;
										MeterData[u8PowerMeterID].RelayStatus = RELAY_OFF;
									} else {					
										MeterRelayStatus = 0x00 ;
										MeterData[u8PowerMeterID].RelayStatus = RELAY_ON ;
									} 
									break;
									
								case CIC_BAW2A :
									if (TokenMeter[4] == 0x01)
									{
											MeterData[u8PowerMeterID].RelayStatus = RELAY_ON;
											MeterRelayStatus = 0x01 ;
									} 
									if (TokenMeter[4] == 0x02)
									{
											MeterData[u8PowerMeterID].RelayStatus = RELAY_OFF;
											MeterRelayStatus = 0x00 ;
									} 
									break;
									
								case TUTANG_E21nE31 :
									if (TokenMeter[4] == 0x00)
									{
											MeterData[u8PowerMeterID].RelayStatus = RELAY_OFF;
											MeterRelayStatus = 0x00 ;
									} else {			
											MeterData[u8PowerMeterID].RelayStatus = RELAY_ON;
											MeterRelayStatus = 0x01 ;
									} 
									break;
									
								case DEM_510c :
									if (TokenMeter[5] & 0x01)
									{
											MeterData[u8PowerMeterID].RelayStatus = RELAY_ON;
											PwrMeterCmdList[u8PowerMeterID] = MBPMCMD_RELAY_ON;
									} else {
											MeterData[u8PowerMeterID].RelayStatus = RELAY_OFF;
											PwrMeterCmdList[u8PowerMeterID] = MBPMCMD_RELAY_OFF;
									} 
						}						
				}
		
		STR_METER_D MD;
		I2cWriteDataStruct(u8PowerMeterID, &MeterData[u8PowerMeterID]);
		I2cReadDataStruct(u8PowerMeterID, &MD);
	}
}
void CalChecksumM(void)
{
	uint8_t i;
	uint8_t Checksum;
    
	Checksum = 0 ;
	for (i=1;i<(METER_TOKEN_LENGTH-2);i++)
	{
		Checksum += MeterTxBuffer[i];
	}	
	MeterTxBuffer[METER_TOKEN_LENGTH-2] = Checksum ;
	MeterTxBuffer[METER_TOKEN_LENGTH-1] = '\n' ;
	//DIR485_METER_Out();
	_SendStringToMETER(MeterTxBuffer,METER_TOKEN_LENGTH);
}
                

void MeterSuccess(void)
{	
		//	Record communication accuracy.
    PowerMeterNG &= (~(0x00000001 <<  (PollingMeterID-1) ) );
		PowerMeterRxCounter[GotDeviceRsp-1]++;
    MeterErrorRate5Min_Rx[GotDeviceRsp-1][MeterErrorRate5Min_Wp]++; 
	
		PollingStateIndex++;
    MeterPollingState = MP_READY;	
}
void MeterTimeoutProcess(void)
{
	
    // Record Error Status
    if ( TickPollingInterval > POLL_TIMEOUT )
    {
        MeterPollingState = MP_READY;							
        PowerMeterReadErrorCnt++;
        if( PowerMeterReadErrorCnt > POLL_ERROR_TIMES )
        {
            PowerMeterNG |=  (0x00000001 <<  (PollingMeterID-1) ) ;
            PollingStateIndex++;
            PowerMeterReadErrorCnt = 0 ;            
            ResetMeterUART();
        }
    }
}

uint8_t _SendStringToMETER(uint8_t *Str, uint8_t len)
{
	
	uint8_t idx;
    uint8_t u8MeterID;

	if( (METERTxQ_cnt+len) > MAX_METER_TXQ_LENGTH )
	{
		return 0x01 ;
	} else {
	        u8MeterID = MeterTxBuffer[0]-1;
	        PowerMeterTxCounter[u8MeterID]++;
		//DIR_METER_RS485_Out();
		for(idx=0;idx<len;idx++)
		{
			METERTxQ[METERTxQ_wp]=Str[idx];
			METERTxQ_wp++;
			if(METERTxQ_wp>=MAX_METER_TXQ_LENGTH)
			{
				METERTxQ_wp=0;
			}
			METERTxQ_cnt++;
		}        			
		UART_EnableInt(UART2, (UART_INTEN_THREIEN_Msk ));
		NVIC_EnableIRQ(UART02_IRQn);
	}                   
	return 0x00 ;
}



