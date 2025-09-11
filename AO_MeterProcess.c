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
uint8_t GapErrorTimes[PwrMtrMax];
uint8_t TickMeter,MeterRelayStatus;
_Bool MeterRelayStausReady,fgMeterAddValue;
uint8_t PowerMeterReadErrorCnt;
uint8_t PollingStateIndex;
uint8_t PollingPwrMtrID;

uint16_t Tick1S_ErrorRateGap;

uint8_t  MeterErrorRate5Min_Wp;
uint16_t MeterErrorRate5Min_Tx[PwrMtrMax][12];
uint16_t MeterErrorRate5Min_Rx[PwrMtrMax][12];

_Bool MeterPollingFinishedFlag;

uint8_t u8MeterInitState, PwrMtrMBCmd;
uint16_t Tick10mSec_PowerMeterInit;
PwrMtrError_t PwrMtrError;

//uint16_t TickReadDeviceTime;


/***
 *	@brief	Need to revise the Cmd Process of DEM_510c
 *	@Problem encounter : When Cmd relay on, Polling process will do cmd than go poll the next device, 
												 when meter didn't do the  "get_relay" state process, meter relay will turn off in few sec.
 ***/

void MeterPolling(void)
{	   
    switch(MeterPollingState)
    {
        case MP_READY :

            if (TickReadDeviceTime >= ReadDeviceCmdTime)
            {
                TickReadDeviceTime = 0 ;
                MeterPollingState = MP_POLLING_W_CMD + PollingStateIndex ;
                if ( MeterPollingState > MP_POLLING_PF_CMD )
                {
                    MeterPollingState = MP_POLLING_W_CMD ;
                    PollingStateIndex = 0 ;
                    PollingPwrMtrID++;
                }
            } else {
                // Commands 
                PwrMtrMBCmd = PwrMtrCmdList[PollingPwrMtrID-1] ;
                PwrMtrCmdList[PollingPwrMtrID-1] = MBPMCMD_READY ;
                switch ( PwrMtrMBCmd )	
                {
                    case MBPMCMD_READY :
                        break;
                    case MBPMCMD_RELAY_ON :
                        MtrRelayOnOff = 1 ;
                        MeterPollingState = MP_SET_RELAY1 ;
                        PwrMtrMBCmd = MBPMCMD_READY;
                        break;
                    case MBPMCMD_RELAY_OFF :										
                        MtrRelayOnOff = 0 ;
                        MeterPollingState = MP_SET_RELAY1 ;
                        PwrMtrMBCmd = MBPMCMD_READY;
                        break;
                    case MBPMCMD_ADD_VALUE :
                        MeterPollingState = MP_ADD_VALUE1 ;
                        PwrMtrMBCmd = MBPMCMD_READY;
                        break;
                    case MBPMCMD_EXIT_TEST :										
                        MeterPollingState = MP_EXIT_TEST1 ;
                        PwrMtrMBCmd = MBPMCMD_READY;
                        break;
                    case MBPMCMD_SET_ADDR :										
                        MeterPollingState = MP_SET_ADDR1 ;
                        PwrMtrMBCmd = MBPMCMD_READY;
                        break;
                    case MBPMCMD_SET_BAUDRATE :
                        MeterPollingState = MP_SET_BAUDRATE1 ;
                        PwrMtrMBCmd = MBPMCMD_READY;
                        break;
                    case MBPMCMD_SET_MODE :										
                        MeterPollingState = MP_SET_MODE1 ;
                        PwrMtrMBCmd = MBPMCMD_READY;
                        break;
								
                    default :
                        break;
                }
            }
            break;
        case MP_POLLING_RSP :
            if (GotDeviceRsp == PwrMtrIDArray[PollingPwrMtrID-1])
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
        case MP_POLLING_BAL_CMD :	
            MODBUS_SendCmd(MDBS_METER_GET_BAL);
            MeterPollingState = MP_POLLING_RSP ; 
            TickPollingInterval = 0 ;			
            break;
				case MP_POLLING_V_CMD :	
            MODBUS_SendCmd(MDBS_METER_GET_V);
            MeterPollingState = MP_POLLING_RSP ; 
            TickPollingInterval = 0 ;			
            break;				
        case  MP_POLLING_I_CMD:
            MODBUS_SendCmd(MDBS_METER_GET_I);
            MeterPollingState = MP_POLLING_RSP ; 
            TickPollingInterval = 0 ;
            break;		
        case MP_POLLING_F_CMD :	
            MODBUS_SendCmd(MDBS_METER_GET_F);
            MeterPollingState = MP_POLLING_RSP ; 
            TickPollingInterval = 0 ;			
            break;
        case MP_POLLING_P_CMD :	
            MODBUS_SendCmd(MDBS_METER_GET_P);
            MeterPollingState = MP_POLLING_RSP ; 
            TickPollingInterval = 0 ;			
            break;
				case MP_POLLING_S_CMD :	
            MODBUS_SendCmd(MDBS_METER_GET_S);
            MeterPollingState = MP_POLLING_RSP ; 
            TickPollingInterval = 0 ;			
            break;		
				case MP_POLLING_PF_CMD :	
            MODBUS_SendCmd(MDBS_METER_GET_PF);
            MeterPollingState = MP_POLLING_RSP ; 
            TickPollingInterval = 0 ;			
            break;
				
				//	Cmds
        case MP_SET_RELAY1 :
            MODBUS_SendCmd(MDBS_METER_SET_PWD);
            MeterPollingState = MP_SET_RELAY2;
            DelayTime4NextCmd = 0 ;					
            break;
        case MP_SET_RELAY2 :			
            if ( DelayTime4NextCmd > 40 ) 
            {
            DelayTime4NextCmd = 0 ;							
            MODBUS_SendCmd(MDBS_METER_SET_RELAY);
            MeterPollingState = MP_READY ;
            }
            break;	
				
        case MP_ADD_VALUE1 :						
            MODBUS_SendCmd(MDBS_METER_SET_PWD);
            MeterPollingState = MP_ADD_VALUE2;
            DelayTime4NextCmd = 0 ;						
            break;
        case MP_ADD_VALUE2 :			
            if ( DelayTime4NextCmd > 30 ) 
            {
                DelayTime4NextCmd = 0 ;				
                MODBUS_SendCmd(MDBS_METER_ADD_VALUE);
                MeterPollingState = MP_READY ;				
            }
            break;		

        case MP_EXIT_TEST1 :						
            MODBUS_SendCmd(MDBS_METER_SET_PWD);
            MeterPollingState = MP_EXIT_TEST2;
            DelayTime4NextCmd = 0 ;						
            break;
        case MP_EXIT_TEST2 :			
            if ( DelayTime4NextCmd > 30 ) 
            {
                DelayTime4NextCmd = 0 ;																	
                MODBUS_SendCmd(MDBS_METER_EXIT_TEST);
                MeterPollingState = MP_READY ;				
            }

				case MP_SET_MODE1 :
            MODBUS_SendCmd(MDBS_METER_SET_PWD);
            MeterPollingState = MP_SET_MODE2;
						DelayTime4NextCmd = 0 ;	
						break;				
				case MP_SET_MODE2 :
						if ( DelayTime4NextCmd > 50 ) 
            {
								MODBUS_SendCmd(MDBS_METER_SET_MODE);
								MeterPollingState = MP_READY ;
						}
						break;
				
				case MP_SET_ADDR1 :
            MODBUS_SendCmd(MDBS_METER_SET_PWD);
            MeterPollingState = MP_SET_ADDR2;
						DelayTime4NextCmd = 0 ;	
						break;					
				case MP_SET_ADDR2 :	
						if ( DelayTime4NextCmd > 50 ) 
            {
								MODBUS_SendCmd(MDBS_METER_SET_ADDR);
								MeterPollingState = MP_READY ;
						}
            break;
				
				case MP_SET_BAUDRATE1 :
            MODBUS_SendCmd(MDBS_METER_SET_PWD);
            MeterPollingState = MP_SET_BAUDRATE2;
						DelayTime4NextCmd = 0 ;	
						break;					
				case MP_SET_BAUDRATE2 :	
						if ( DelayTime4NextCmd > 50 ) 
            {
								MODBUS_SendCmd(MDBS_METER_SET_BAUDRATE);
								MeterPollingState = MP_READY ;
						}
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
		if ( u8PowerMeterID == PwrMtrIDArray[PollingPwrMtrID-1]) 
		{
				u8PowerMeterID = PollingPwrMtrID - 1;
		} else 
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
								break;
							case CIC_BAW1A:
								u32temp = (TokenMeter[3] << 24) + (TokenMeter[4] << 16) + (TokenMeter[5] << 24) + TokenMeter[6] ;								
								break;
							default :
								u32temp = (TokenMeter[5] << 24) + (TokenMeter[6] << 16) + (TokenMeter[3] << 8) + TokenMeter[4] ;
								break;
						}
						
						TotalWattValue_Now = MeterData[u8PowerMeterID].TotalWatt;

						if ( PowerOnReadMeter )
						{
								PowerOnReadMeter = 0 ;
								TotalWattValue_Now = u32temp ;
								MeterData[u8PowerMeterID].TotalWatt = TotalWattValue_Now ;
								GapErrorTimes[u8PowerMeterID] = 0 ;
						}
						
						if ( u32temp >= TotalWattValue_Now )
						{
								if (u32temp == TotalWattValue_Now) return;

								GapMeterPower = u32temp - TotalWattValue_Now;
								if ( GapMeterPower < 200 )
								{
										TotalWattValue_Now = u32temp ;
										MeterData[u8PowerMeterID].TotalWatt = TotalWattValue_Now ;											
										GapErrorTimes[u8PowerMeterID] = 0 ;
								} else {
										GapErrorTimes[u8PowerMeterID] ++;
										if ( GapErrorTimes[u8PowerMeterID] > 3 ) 
										{
												TotalWattValue_Now = u32temp ;					
												GapErrorTimes[u8PowerMeterID] = 0 ;
												MeterData[u8PowerMeterID].TotalWatt = TotalWattValue_Now ;
												//WritePowerToEE();
										}
								}
						} else {
								GapErrorTimes[u8PowerMeterID] ++;
								if ( GapErrorTimes[u8PowerMeterID] > 3 )
								{
									TotalWattValue_Now = u32temp ;
									MeterData[u8PowerMeterID].TotalWatt = TotalWattValue_Now ;
									//WritePowerToEE();
									GapErrorTimes[u8PowerMeterID] = 0 ;
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
										MeterData[u8PowerMeterID].RelayStatus = RELAY_OFF;
									} else {					
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
									if (TokenMeter[4] == 0x01)
									{
											MeterData[u8PowerMeterID].RelayStatus = RELAY_ON;
									} else {
											MeterData[u8PowerMeterID].RelayStatus = RELAY_OFF;
									}
									break;
								
								case CIC_BAW1A :
									if (TokenMeter[4] == 0x01)
									{
											MeterData[u8PowerMeterID].RelayStatus = RELAY_ON;
									} else {
											MeterData[u8PowerMeterID].RelayStatus = RELAY_OFF;
									}
									break;
						}
				}
				
				if ( MeterMBCmd == MDBS_METER_GET_BAL)
				{
					switch ( MeterType )
					{
						case DAE_530n540 :
							break;
						
						case CIC_BAW2A :				
							MeterData[u8PowerMeterID].Balance = ( (TokenMeter[3]) << 24) + ( (TokenMeter[4]) << 16) + ( (TokenMeter[5]) << 8) + (TokenMeter[6]) ;
							 
							if (MeterData[u8PowerMeterID].Balance < 99900)
							{
									if ( PwrMtrCmdList[u8PowerMeterID] == MBPMCMD_READY )
									{
											PwrMtrCmdList[u8PowerMeterID] = MBPMCMD_ADD_VALUE ;
									}
							} 				
							break;
							
						case CIC_BAW1A :
							MeterData[u8PowerMeterID].Balance = ( (TokenMeter[3]) << 24) + ( (TokenMeter[4]) << 16) + ( (TokenMeter[5]) << 8) + (TokenMeter[6]) ;
							 
							if (MeterData[u8PowerMeterID].Balance < 99900)
							{
									if ( PwrMtrCmdList[u8PowerMeterID] == MBPMCMD_READY )
									{
											PwrMtrCmdList[u8PowerMeterID] = MBPMCMD_ADD_VALUE ;
									}
							}
							break;
							
						case TUTANG_E21nE31 :		
							break;
						
						case DEM_510c :
							break;
						
						default :
							break;
					}						
				}
				
				if ( MeterMBCmd == MDBS_METER_GET_V )
				{
					switch ( MeterType )
					{
						case DAE_530n540 :					
							break;
						case CIC_BAW2A :
								MeterData[u8PowerMeterID].V = (TokenMeter[3] << 24) + (TokenMeter[4] << 16) + (TokenMeter[5] << 8) + TokenMeter[6] ;
								break;
						case TUTANG_E21nE31 :
								break;
						case DEM_510c :
								break;
						case CIC_BAW1A :
								MeterData[u8PowerMeterID].V = (TokenMeter[3] << 24) + (TokenMeter[4] << 16) + (TokenMeter[5] << 8) + TokenMeter[6] ;						
						default :
							break;
					}						
				}
	
				if ( MeterMBCmd == MDBS_METER_GET_I )
				{
					switch ( MeterType )
					{
						case DAE_530n540 :					
							break;
						case CIC_BAW2A :
								MeterData[u8PowerMeterID].I = (TokenMeter[3] << 24) + (TokenMeter[4] << 16) + (TokenMeter[5] << 8) + TokenMeter[6] ;
								break;
						case TUTANG_E21nE31 :
								break;
						case DEM_510c :
								break;
						case CIC_BAW1A :
								MeterData[u8PowerMeterID].I = (TokenMeter[3] << 24) + (TokenMeter[4] << 16) + (TokenMeter[5] << 8) + TokenMeter[6] ;						
						default :
							break;
					}						
				}
	
				if ( MeterMBCmd == MDBS_METER_GET_F )
				{
					switch ( MeterType )
					{
						case DAE_530n540 :					
							break;
						case CIC_BAW2A :
								MeterData[u8PowerMeterID].F =  (TokenMeter[3] << 8) + TokenMeter[4] ;
							break;
						case TUTANG_E21nE31 :
							break;
						case DEM_510c :
								break;
						case CIC_BAW1A :
								MeterData[u8PowerMeterID].F = (TokenMeter[3] << 8) + TokenMeter[4] ;	
								break;
						default :
							break;						
					}						
				}
				
				if ( MeterMBCmd == MDBS_METER_GET_P )
				{
					switch ( MeterType )
					{
						case DAE_530n540 :					
							break;
						case CIC_BAW2A :
								MeterData[u8PowerMeterID].P = (TokenMeter[3] << 24) + (TokenMeter[4] << 16) + (TokenMeter[5] << 8) + TokenMeter[6] ;
								break;
						case TUTANG_E21nE31 :
							break;
						case DEM_510c :
								break;
						case CIC_BAW1A :
								MeterData[u8PowerMeterID].P = (TokenMeter[3] << 24) + (TokenMeter[4] << 16) + (TokenMeter[5] << 8) + TokenMeter[6] ;
								break;
						default :
							break;
						
					}						
				}
				
				if ( MeterMBCmd == MDBS_METER_GET_S )
				{
					switch ( MeterType )
					{
						case DAE_530n540 :					
							break;
						case CIC_BAW2A :
								MeterData[u8PowerMeterID].S = (TokenMeter[3] << 24) + (TokenMeter[4] << 16) + (TokenMeter[5] << 8) + TokenMeter[6] ;
								break;
						case TUTANG_E21nE31 :
							break;
						case DEM_510c :
								break;
						case CIC_BAW1A :
								MeterData[u8PowerMeterID].S = (TokenMeter[3] << 24) + (TokenMeter[4] << 16) + (TokenMeter[5] << 8) + TokenMeter[6] ;
								break;
						default :
							break;
						
					}						
				}
		// ¥\²v¦]¤l
				if ( MeterMBCmd == MDBS_METER_GET_PF )
				{
					switch ( MeterType )
					{
						case DAE_530n540 :					
							break;
						case CIC_BAW2A :
								MeterData[u8PowerMeterID].PwrFactor =  (TokenMeter[3] << 8) + TokenMeter[4] ;
								break;
						case TUTANG_E21nE31 :
							break;
						case DEM_510c :
								break;
						case CIC_BAW1A :
								MeterData[u8PowerMeterID].PwrFactor = (TokenMeter[3] << 8) + TokenMeter[4] ;
								break;
						default :
							break;
					}						
				}
				
//		STR_METER_D MD;
//		I2cWriteDataStruct(u8PowerMeterID, &MeterData[u8PowerMeterID]);
//		I2cReadDataStruct(u8PowerMeterID, &MD);
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
		uint8_t PwrMtrArrayIndex = PollingPwrMtrID -1;
		//	Record communication accuracy.
    PwrMtrError.PwrMtrDeviceNG &= (~(0x00000001 <<  (PwrMtrArrayIndex) ) );
		PwrMtrError.Success[PwrMtrArrayIndex]++;
    //MeterErrorRate5Min_Rx[PwrMtrArrayIndex][MeterErrorRate5Min_Wp]++; 
	
		PollingStateIndex++;
    MeterPollingState = MP_READY;	
}
void MeterTimeoutProcess(void)
{
		uint8_t PwrMtrArrayIndex = PollingPwrMtrID -1;
    // Record Error Status
    if ( TickPollingInterval > POLL_TIMEOUT )
    {
        MeterPollingState = MP_READY;							
        PowerMeterReadErrorCnt++;
				PwrMtrError.Fail[PwrMtrArrayIndex]++;
        if( PowerMeterReadErrorCnt > MAX_POLL_RETRY_TIMES )
        {
            PwrMtrError.PwrMtrDeviceNG |=  (0x00000001 <<  (PwrMtrArrayIndex) ) ;
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



