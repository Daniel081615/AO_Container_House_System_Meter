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
uint8_t GapErrorTimes[MAX_POWER_METER];
uint8_t TickMeter,MeterRelayStatus;
_Bool MeterRelayStausReady,fgMeterAddValue;
uint8_t PowerMeterReadErrorCnt;
uint8_t PollingStateIndex;
uint8_t PollingMeterID;

uint16_t Tick1S_ErrorRateGap;
uint32_t    PowerMeterNG;

uint8_t     MeterErrorRate5Min_Wp;
uint16_t MeterErrorRate5Min_Tx[MAX_POWER_METER][12];
uint16_t MeterErrorRate5Min_Rx[MAX_POWER_METER][12];

_Bool MeterPollingFinishedFlag;

uint8_t u8MeterInitState;
uint16_t Tick10mSec_PowerMeterInit;

//uint16_t TickReadPowerTime;



void MeterPolling(void)
{	   
    switch(MeterPollingState)
    {
        case MP_READY :

            if (TickReadPowerTime >= ReadMeterTime)
            {
                TickReadPowerTime = 0 ;								
                MeterPollingState = MP_POLLING_W_CMD + PollingStateIndex ;
                if ( MeterPollingState > MP_POLLING_CARD_CMD )
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
                ModBusCommand = ModBusCommandList[PollingMeterID-1] ;
                ModBusCommandList[PollingMeterID-1] = MBCMD_READY ;
                PowerMeterID = PollingMeterID ;
                switch ( ModBusCommand )	
                {
                    case MBCMD_READY :
                        break;
                    case MBCMD_RELAY_ON :
                        RelayOnOff = 1 ;
                        MeterPollingState = MP_SET_RELAY1 ;
                        ModBusCommand = MBCMD_READY;
                        break;
                    case MBCMD_RELAY_OFF :										
                        RelayOnOff = 0 ;
                        MeterPollingState = MP_SET_RELAY1 ;
                        ModBusCommand = MBCMD_READY;
                        break;
                    case MBCMD_ADD_VALUE :														
                        MeterPollingState = MP_ADD_VALUE1 ;
                        ModBusCommand = MBCMD_READY;
                        break;
                    case MBCMD_EXIT_TEST :
                        MeterPollingState = MP_EXIT_TEST1 ;
                        ModBusCommand = MBCMD_READY;
                        break;
                    case MBCMD_SET_POWER_ON :
                        RelayOnOff = 1 ;
                        MeterPollingState = MP_SET_POWER_ON1 ;
                        ModBusCommand = MBCMD_READY;
                        break;
                    case MBCMD_SET_POWER_OFF :		
                        RelayOnOff = 0 ;
                        MeterPollingState = MP_SET_POWER_OFF1 ;
                        ModBusCommand = MBCMD_READY;
                        break;                        
                    case MBCMD_SET_POWER_READER :		
                        RelayOnOff = 0 ;
                        MeterPollingState = MP_SET_POWER_READER1 ;
                        ModBusCommand = MBCMD_READY;
                        break;
                    default :
                        break;
                }
            }
            break;
        case MP_POLLING_RSP :
						//	Get rsp than go to next stage
            if (GotMeterRsp == PowerMeterID)
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
            if ( RoomMode[PollingMeterID-1] == RM_FREE_MODE_READY )
            {
                MODBUS_SendCmd(MDBS_METER_GET_BAL);
                MeterPollingState = MP_POLLING_RSP ; 
                TickPollingInterval = 0 ;			
            } else {
                MeterPollingState = MP_READY;	
                PollingStateIndex++;
                break;
            }
            break;		
        case MP_POLLING_V_CMD :			
            MODBUS_SendCmd(MDBS_METER_GET_V);
            MeterPollingState = MP_POLLING_RSP ; 
            TickPollingInterval = 0 ;			
            break;
        case MP_POLLING_I_CMD :			
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
        case MP_POLLING_PAY_CMD :
						MODBUS_SendCmd(MDBS_METER_GET_MODE);
						MeterPollingState = MP_POLLING_RSP ; 
            TickPollingInterval = 0 ;	
            break;
        case MP_POLLING_STATUS_CMD :			
            MODBUS_SendCmd(MDBS_METER_GET_STATUS);
            MeterPollingState = MP_POLLING_RSP ; 
            TickPollingInterval = 0 ;			
            break;
        case MP_POLLING_CARD_CMD :			
            MODBUS_SendCmd(MDBS_METER_GET_CARD_ID);
            MeterPollingState = MP_POLLING_RSP ; 
            TickPollingInterval = 0 ;			
            break;
        case MP_SET_RELAY1 :
            MODBUS_SendCmd(MDBS_METER_SET_PWD);
            MeterPollingState = MP_SET_RELAY2;
            DelayTime4NextCmd = 0 ;					
            break;
        case MP_SET_RELAY2 :			
            if ( DelayTime4NextCmd > 40 ) 
            {
            DelayTime4NextCmd = 0 ;				
            // BAW2A => 1 : ON   2 : OFF				
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
                // BAW2A					
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
                // BAW2A				
                MODBUS_SendCmd(MDBS_METER_EXIT_TEST);
                MeterPollingState = MP_READY ;				
            }
            break;			
        case MP_SET_POWER_ON1 :			
            DelayTime4NextCmd = 0 ;	
            Tick5mS_CheckRoomMode = 0 ;
					  MODBUS_SendCmd(MDBS_METER_SET_PWD);
            MeterPollingState = MP_SET_POWER_ON2;	
            break;
        case MP_SET_POWER_ON2 :			
            if ( DelayTime4NextCmd > 50 ) 
            {
                // BAW2A	
                DelayTime4NextCmd = 0 ;																		
                PayMode = 0x00;                 // Card reader Disable (Meter)
                MeterPollingState = MP_SET_POWER_ON3 ;
                Tick5mS_CheckRoomMode = 0 ;
                MODBUS_SendCmd(MDBS_METER_SET_MODE);
            }
            break;
         case MP_SET_POWER_ON3 :			
            if ( DelayTime4NextCmd > 50 ) 
            {
                // BAW2A	
                DelayTime4NextCmd = 0 ;																		                
                MeterPollingState = MP_SET_POWER_ON4 ;	
                Tick5mS_CheckRoomMode = 0 ;
                MODBUS_SendCmd(MDBS_METER_SET_PWD);
            }
            break;
        case MP_SET_POWER_ON4 :			
            if ( DelayTime4NextCmd > 50 ) 
            {
                // BAW2A	
                DelayTime4NextCmd = 0 ;                
                MeterPollingState = MP_SET_POWER_ON5 ;	
                Tick5mS_CheckRoomMode = 0 ;
                MODBUS_SendCmd(MDBS_METER_ADD_VALUE);
            }
            break;
        case MP_SET_POWER_ON5 :			
            if ( DelayTime4NextCmd > 50 ) 
            {
                // BAW2A	
                DelayTime4NextCmd = 0 ;																		                
                MeterPollingState = MP_SET_POWER_ON6 ;	
                Tick5mS_CheckRoomMode = 0 ;
                MODBUS_SendCmd(MDBS_METER_SET_PWD);
            }
            break;
        case MP_SET_POWER_ON6 :			
            if ( DelayTime4NextCmd > 50 ) 
            {
                // BAW2A	
                DelayTime4NextCmd = 0 ;
                RelayOnOff = 1 ; // ON
                MeterPollingState = MP_READY ;	
                Tick5mS_CheckRoomMode = 0 ;
                MODBUS_SendCmd(MDBS_METER_SET_RELAY);
            }
            break;
        case MP_SET_POWER_OFF1 :			
						MeterPollingState = MP_SET_POWER_OFF2;
            DelayTime4NextCmd = 0 ;
            Tick5mS_CheckRoomMode = 0 ;
            MODBUS_SendCmd(MDBS_METER_SET_PWD);
            break;
        case MP_SET_POWER_OFF2 :			
            if ( DelayTime4NextCmd > 50 ) 
            {
                DelayTime4NextCmd = 0 ;															
                // BAW2A			
                PayMode = 0x00;     // Card reader Disable (Meter)
                Tick5mS_CheckRoomMode = 0 ;
                MODBUS_SendCmd(MDBS_METER_SET_MODE);
                MeterPollingState = MP_SET_POWER_OFF5 ;				
            }
            break;		
        case MP_SET_POWER_OFF3 :			
            if ( DelayTime4NextCmd > 50 ) 
            {
                DelayTime4NextCmd = 0 ;															
                // BAW2A				                
                Tick5mS_CheckRoomMode = 0 ;
                MODBUS_SendCmd(MDBS_METER_SET_PWD);
                MeterPollingState = MP_SET_POWER_OFF4 ;				
            }
            break;	
        case MP_SET_POWER_OFF4 :			
            if ( DelayTime4NextCmd > 50 ) 
            {
                MeterPollingState = MP_SET_POWER_OFF5 ;
                DelayTime4NextCmd = 0 ;                
                Tick5mS_CheckRoomMode = 0 ;
                MODBUS_SendCmd(MDBS_METER_ADD_VALUE);		
            }
            break;		
        case MP_SET_POWER_OFF5 :			
            if ( DelayTime4NextCmd > 50 ) 
            {
                DelayTime4NextCmd = 0 ;															
                // BAW2A				             
                Tick5mS_CheckRoomMode = 0 ;
                MODBUS_SendCmd(MDBS_METER_SET_PWD);
                MeterPollingState = MP_SET_POWER_OFF6 ;				
            }
            break;	
        case MP_SET_POWER_OFF6 :			
            if ( DelayTime4NextCmd > 50 ) 
            {
                MeterPollingState = MP_READY ;
                DelayTime4NextCmd = 0 ;
                RelayOnOff = 0 ; // OFF
                Tick5mS_CheckRoomMode = 0 ;
                MODBUS_SendCmd(MDBS_METER_SET_RELAY);		
            }
            break;		
        case MP_SET_POWER_READER1 :						
            MODBUS_SendCmd(MDBS_METER_SET_PWD);
            MeterPollingState = MP_SET_POWER_READER2;
            DelayTime4NextCmd = 0 ;						
            break;
        case MP_SET_POWER_READER2 :			
            if ( DelayTime4NextCmd > 50 ) 
            {
                DelayTime4NextCmd = 0 ;															
                // BAW2A	
                RelayOnOff = 0 ;    // OFF
                MODBUS_SendCmd(MDBS_METER_SET_RELAY);
                MeterPollingState = MP_SET_POWER_READER3 ;						
            }
            break;		
        case MP_SET_POWER_READER3 :			
            if ( DelayTime4NextCmd > 50 ) 
            {
                DelayTime4NextCmd = 0 ;															
                // BAW2A				                
                MODBUS_SendCmd(MDBS_METER_SET_PWD);
                MeterPollingState = MP_SET_POWER_READER4 ;				
            }
            break;	
        case MP_SET_POWER_READER4 :			
            if ( DelayTime4NextCmd > 50 ) 
            {
                DelayTime4NextCmd = 0 ;
                // BAW2A		                
                PayMode = 0x01;     // Card reader Enable ( Reader )
                MODBUS_SendCmd(MDBS_METER_SET_MODE);
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
        if ( u8PowerMeterID > 0 ) 
            u8PowerMeterID = u8PowerMeterID - 1;
        else 
            return;        
        
	if ( TokenMeter[1] == 0x03 )
	{		
		// 分析電表度數 
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
                    // 2025.07.02 START
                    // MeterData[u8PowerMeterID].MeterTotalWatt = u32temp;
                    MeterPowerValue_Now = MeterData[u8PowerMeterID].MeterTotalWatt;
                    // 2025.07.02 END
			if ( PowerOnReadMeter )
			{
				PowerOnReadMeter = 0 ;
				MeterPowerValue_Now = u32temp ;
				LastMeterPower[u8PowerMeterID] = MeterPowerValue_Now ;
				MeterData[u8PowerMeterID].MeterTotalWatt = MeterPowerValue_Now ;
				//MeterData[u8PowerMeterID].RelayStatus = 1 ;
                           RoomData.Status &= (~FLAG_POWER_METER_READY) ;
                           GapErrorTimes[u8PowerMeterID] = 0 ;
			}
			
			if ( u32temp >= MeterPowerValue_Now )
			{
				if (u32temp == MeterPowerValue_Now) return;

				GapMeterPower = u32temp - MeterPowerValue_Now;
				// Check Over Value < 200 
				if ( GapMeterPower < 200 )
				{
					MeterPowerValue_Now = u32temp ;
                                  LastMeterPower[u8PowerMeterID] = MeterPowerValue_Now;
                                  MeterData[u8PowerMeterID].MeterTotalWatt = MeterPowerValue_Now ;
				       RoomData.Status &= (~FLAG_POWER_METER_READY) ;												
					GapErrorTimes[u8PowerMeterID] = 0 ;
				} else {
					GapErrorTimes[u8PowerMeterID] ++;
					if ( GapErrorTimes[u8PowerMeterID] > 3 ) 
					{
						MeterPowerValue_Now = u32temp ;
						LastMeterPower[u8PowerMeterID] = MeterPowerValue_Now;						
						GapErrorTimes[u8PowerMeterID] = 0 ;
						MeterData[u8PowerMeterID].MeterTotalWatt = MeterPowerValue_Now ;
						//WritePowerToEE();
						RoomData.Status &= (~FLAG_POWER_METER_READY) ;
					}
				}
			} else {
				//	Abnormal Condition: The meter reading is less than the previous value, which may indicate a meter replacement. The meter value will only be updated if this condition occurs for three consecutive times.
				//	Abnormal Condition: The value will only be updated after this condition occurs three consecutive times (usually due to meter replacement).
				GapErrorTimes[u8PowerMeterID] ++;
				if ( GapErrorTimes[u8PowerMeterID] > 3 )
				{
					MeterPowerValue_Now = u32temp ;
					LastMeterPower[u8PowerMeterID] = MeterPowerValue_Now;	
					MeterData[u8PowerMeterID].MeterTotalWatt = MeterPowerValue_Now ;
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
					// 台科電 DAE DEM530/540
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
					if (TokenMeter[5] & 0x01){
							MeterData[u8PowerMeterID].RelayStatus = RELAY_ON;
					} else {
					    MeterData[u8PowerMeterID].RelayStatus = RELAY_OFF;
					} 
			}						
		}
			
		// 分析電錶餘額
		if ( MeterMBCmd == MDBS_METER_GET_BAL)
		{
			switch ( MeterType )
			{
				case DAE_530n540 :
					break;
				case CIC_BAW2A :		
					MeterData[u8PowerMeterID].MeterBalance = ( (TokenMeter[3]) << 24) + ( (TokenMeter[4]) << 16) + ( (TokenMeter[5]) << 8) + (TokenMeter[6]) ;
					 
					if ( (MeterData[u8PowerMeterID].MeterBalance < 99900) && (RoomMode[u8PowerMeterID] == RM_FREE_MODE_READY))
					{
					    if ( ModBusCommandList[u8PowerMeterID] == MBCMD_READY )
							{
									ModBusCommandList[u8PowerMeterID] = MBCMD_ADD_VALUE ;
							}
					} 				
					break;
				case TUTANG_E21nE31 :				
					break;
				case DEM_510c:
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
					MeterData[u8PowerMeterID].MeterValtage = (TokenMeter[3] << 24) + (TokenMeter[4] << 16) + (TokenMeter[5] << 8) + TokenMeter[6] ;
					break;
				case TUTANG_E21nE31 :
					break;
				case DEM_510c:
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
					MeterData[u8PowerMeterID].MeterCurrent = (TokenMeter[3] << 24) + (TokenMeter[4] << 16) + (TokenMeter[5] << 8) + TokenMeter[6] ;
					break;
				case TUTANG_E21nE31 :
					break;
				case DEM_510c:
					break;
			}						
		}
		// 頻率
		if ( MeterMBCmd == MDBS_METER_GET_F )
		{
			switch ( MeterType )
			{
				case DAE_530n540 :					
					break;
				case CIC_BAW2A :
					MeterData[u8PowerMeterID].MeterFreq =  (TokenMeter[3] << 8) + TokenMeter[4] ;
					break;
				case TUTANG_E21nE31 :
					break;
				case DEM_510c:
					break;
			}						
		}
		// 有功功率		
		if ( MeterMBCmd == MDBS_METER_GET_P )
		{
			switch ( MeterType )
			{
				case DAE_530n540 :					
					break;
				case CIC_BAW2A :
					MeterData[u8PowerMeterID].MeterVA = (TokenMeter[3] << 24) + (TokenMeter[4] << 16) + (TokenMeter[5] << 8) + TokenMeter[6] ;
					break;
				case TUTANG_E21nE31 :
					break;
				case DEM_510c:
					break;
			}						
		}
		// 視在功率		
		if ( MeterMBCmd == MDBS_METER_GET_S )
		{
			switch ( MeterType )
			{
				case DAE_530n540 :					
					break;
				case CIC_BAW2A :
					MeterData[u8PowerMeterID].MeterVA = (TokenMeter[3] << 24) + (TokenMeter[4] << 16) + (TokenMeter[5] << 8) + TokenMeter[6] ;
					break;
				case TUTANG_E21nE31 :
					break;
				case DEM_510c:
					break;
			}						
		}
		// 功率因子
		if ( MeterMBCmd == MDBS_METER_GET_PF )
		{
			switch ( MeterType )
			{
				case DAE_530n540 :					
					break;
				case CIC_BAW2A :
					MeterData[u8PowerMeterID].MeterPowerFactor =  (TokenMeter[3] << 8) + TokenMeter[4] ;
					break;
				case TUTANG_E21nE31 :
					break;
				case DEM_510c:
					break;
			}						
		}
		
		if ( MeterMBCmd == MDBS_METER_GET_MODE)
		{
		    MeterData[u8PowerMeterID].PayMode =  TokenMeter[4] ;
		}
		
		if ( MeterMBCmd == MDBS_METER_GET_STATUS)
		{
		    MeterData[u8PowerMeterID].UserStatus =  TokenMeter[4] ;
		}
		
		if ( MeterMBCmd == MDBS_METER_GET_CARD_ID)
		{
		    MeterData[u8PowerMeterID].UserUID = (TokenMeter[3] << 24) + (TokenMeter[4] << 16) + (TokenMeter[5] << 8) + TokenMeter[6] ;;
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
		PowerMeterRxCounter[GotMeterRsp-1]++;
    MeterErrorRate5Min_Rx[GotMeterRsp-1][MeterErrorRate5Min_Wp]++; 
	
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



