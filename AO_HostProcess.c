/*--------------------------------------------------------------------------- 
 * File Name     : AO_HostProcess.C
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
#include "AO_ModBusProcess.h"
#include "AO_BMSModbusProcess.h"
#include "AO_WaterMeterModbusProcess.h"
#include "AO_InverterModbusProcess.h"
#include "AO_Pyranometer.h"
#include "AO_SoilSensor.h"
#include "AO_AirSensor.h"

extern uint8_t SendHost_QWp,SendHost_QRp,SendHost_QCounter;

extern uint8_t bSaveSettingToEE;
extern uint8_t UserMemberIndex;

uint8_t CmdType,WaitTime;
_Bool bReSendPowerEnable,fgSetStartData;
uint8_t LastMessDate[3];
uint8_t CenterRoomMode;
uint8_t CenterNowUser;
uint8_t CenterUnitPrice;

uint8_t HostMeterIndex,HostMemberIndex;
uint8_t LastPackageChecksum,GetUserIndex,PkgIndex;
_Bool fgUserChangeMode;
float Min_LowBalance;
uint8_t HostPollingDeviceIdx;
uint8_t MeterActive;

void HostProcess(void);
void CalChecksumH(void);
void ClearRespDelayTimer(void);


void SystemSwitchProcess(void);
 
void Host_AliveProcess(void);
void Host_PwrMtrDataProcess(void);
void Host_BmsDataProcess(void);
void Host_WtrMtrDataProcess(void);
void Host_PyrWtrDataProcess(void);
void Host_SoilSensorDataProcess(void);
void Host_AirSensorDataProcess(void);
void Host_WateringProcess(void);

void SendHost_Ack(void);
void SendHost_SystemInformation(void);
void SendHost_PwrMtrData(void);
void SendHost_BmsData(void);
void SendHost_WtrMtrData(void);
void SendHost_PyrMtrData(void);
void SendHost_SoilData(void);
void SendHost_AirData(void);
void SendHost_InvData(void);
void SendHost_WateringStatus(void);

uint8_t _SendStringToHOST(uint8_t *Str, uint8_t len);

void GetHostRTC(void);

/*MeterOta*/
void Host_OTAMenterProcess(void);
void SendHost_MenterFWinfo(void);
void SendHost_MenterFWActivatedInfo(void);




/*********************************
 * Define : Host TokenHost 
 * byte 0 : 0x55 
 * byte 1 : MyMeterBoardID 
 * byte 2 : Command Type
 * byte 3 : System Flag 
 * byte 4 ~ 47  : Data 0 ~ 44
 * byte 48 : Checksum 
 * byte 49 : 0x0A   
  *********************************/
void HostProcess(void)
{
    uint8_t i,checksum,fgDataReady,fgFromHostRSPFlag;
	
		// When Init, Send OTA Success to  Center
		if (fgFirstStart){
				fgFirstStart = 0;
				SendHost_MenterFWActivatedInfo();
		}

    if ( TickHost == 49 )
    {
        TickHost++;
        ResetHostUART();	
    }
    if( HOSTTokenReady )
    {
        HOSTTokenReady = 0 ;
        fgDataReady = 0 ;
        checksum = 0 ;

        for(i=1;i<(HOST_TOKEN_LENGTH-2);i++)
        {
            checksum += TokenHost[i];				
        }        
        LastPackageChecksum = checksum ;
				
        if ( (TokenHost[0] == 0x55) && (TokenHost[HOST_TOKEN_LENGTH-2] == checksum) && (TokenHost[HOST_TOKEN_LENGTH-1] == 0x0A))
        {
            fgDataReady = 1 ;            
            MeterActive = TokenHost[1] ;
        }
				
        // 0x55,CenterID,CMD,....,Checksum,\n
        if (fgDataReady)
        {					

            if ( TokenHost[2] == METER_CMD_ALIVE )
            {
                Host_AliveProcess();
            }
						//	Rsp Host only when Host polls "Current" meter, @0xFF??
            if ( (TokenHost[1] == MyMeterBoardID) || (TokenHost[1] == 0xFF)  )
            {
                HostMeterIndex = TokenHost[1]-1 ;	
                fgFromHostFlag = TokenHost[3];
                fgFromHostRSPFlag = TokenHost[4];
                fgToHostFlag &= fgFromHostRSPFlag;
							
                LED_R_TOGGLE();
                fgToHostRSPFlag = 0xFF ;                
								// Token ready
                switch( TokenHost[2] )
                {
                    case METER_CMD_ALIVE :
                        // 0x55, CenterID, CMD, RoomID, ... System Time(17~22)
                        Host_AliveProcess();						
                        CmdType = METER_RSP_SYS_INFO ;
                        ClearRespDelayTimer() ;	
                        break;							
                    case METER_GET_CMD_POWER_METER :
                        Host_PwrMtrDataProcess();                        
                        CmdType = METER_RSP_POWER_DATA ;
                        ClearRespDelayTimer() ;	
                        break;
                    case METER_GET_CMD_BMS :
                        Host_BmsDataProcess();                        
                        CmdType = METER_RSP_BMS_DATA ;
                        ClearRespDelayTimer() ;	
                        break;										
                    case METER_GET_CMD_WATER_METER :
                        Host_WtrMtrDataProcess();                        
                        CmdType = METER_RSP_WATER_DATA ;
                        ClearRespDelayTimer() ;	
                        break;	
										case METER_GET_CMD_PYRANOMETER:
                        Host_PyrWtrDataProcess();                        
                        CmdType = METER_RSP_PYR_DATA ;
                        ClearRespDelayTimer();
												break;
                    case METER_GET_CMD_SOILSENSOR :
                        Host_SoilSensorDataProcess();                        
                        CmdType = METER_RSP_SOIL_DATA ;
                        ClearRespDelayTimer() ;	
                        break;
                    case METER_GET_CMD_AIRSENSOR :
                        Host_AirSensorDataProcess();                        
                        CmdType = METER_RSP_AIR_DATA ;
                        ClearRespDelayTimer() ;	
                        break;										
                    case METER_GET_CMD_INV :                     
                        CmdType = METER_RSP_INV_DATA ;
                        ClearRespDelayTimer() ;	
                        break;
                    case METER_OTA_UPDATE:
                        Host_OTAMenterProcess();
                        ClearRespDelayTimer();
                        break;
										case METER_GET_CMD_WATERING:
												Host_WateringProcess();
												CmdType = METER_RSP_WATERING_STATUS ;
												ClearRespDelayTimer();
                    default:
                        break;
                }
            }			
        } else {			
            ResetHostUART();
        }
    }

    if ( bDelaySendHostCMD )
    {
        if ( iTickDelaySendHostCMD > WaitTime)
        {
            bDelaySendHostCMD = 0 ;
            if ( (HostMeterIndex+1) == MyMeterBoardID )
            {
                switch (CmdType)
                {
                    case METER_RSP_SYS_INFO :
                        SendHost_SystemInformation();
                        break;
                    case METER_RSP_ACK:
                        SendHost_Ack();
                        break;
										
                    case METER_RSP_POWER_DATA :
                        SendHost_PwrMtrData();
                        break;
										
										case METER_RSP_BMS_DATA:
												SendHost_BmsData();
												break;
										
										case METER_RSP_WATER_DATA:
												SendHost_WtrMtrData();
												break;
										
										case METER_RSP_PYR_DATA:
												SendHost_PyrMtrData();
												break;

										case METER_RSP_SOIL_DATA:
												SendHost_SoilData();
												break;
										
										case METER_RSP_AIR_DATA:
												SendHost_AirData();
												break;
										
										case METER_RSP_INV_DATA:
												SendHost_InvData();
												break;
										
										case METER_RSP_WATERING_STATUS:
												SendHost_WateringStatus();
												break;
                    
										default :
                        break;
                }            
            }
        }
    }
}

void ClearRespDelayTimer(void)
{
	WaitTime = 1 ;
	bDelaySendHostCMD = 1 ;
	iTickDelaySendHostCMD = 0 ;	
}

/***
 *	@note	Need to revise 
 ***/

void Host_AliveProcess(void)
{
    uint8_t i;
    uint8_t u8BuferIndex;
    uint8_t HostData_P,HostData_N;
    
		GetHostRTC();

    if ( (iSystemTime[4] == 0) && (iSystemTime[5] == 0))        
    {
        for (i=0;i<PwrMtrMax;i++)
        {
            if ( PowerMeterTxCounter[i] > 10 )
            {
                PowerMeterTxCounter[i]=1;
                PowerMeterRxCounter[i]=1;
            }
        }
    }
    
}

/***	Process the device sub cmd from Center 	***/
//	Using Cmd list store the cmds, and do the Cmds in the meter board polling states
void Host_PwrMtrDataProcess(void)
{
    HostPollingDeviceIdx = TokenHost[3];
		GetHostRTC();
		if (TokenHost[4] != 0x00)
		{
				PwrMtrCmdList[HostPollingDeviceIdx-1] = TokenHost[4];
				if (PwrMtrCmdList[HostPollingDeviceIdx-1] == MBPMCMD_SET_ADDR)
				{	
						PwrMtrNewAddr = TokenHost[5];
				} else if (PwrMtrCmdList[HostPollingDeviceIdx-1] == MBPMCMD_SET_BAUDRATE){
						PwrMtrNewBaudRate = TokenHost[5];
				}
		}
}

void Host_BmsDataProcess(void)
{   
    HostPollingDeviceIdx = TokenHost[3];
		GetHostRTC();
		
		if (TokenHost[4] != 0x00)
		{
				BmsCmdList[HostPollingDeviceIdx-1] = TokenHost[4];
			
				if ((TokenHost[4]) == MBBMSCMD_MODE_SETUP)
				{		//	Bms Cmd state: Set Bms mode
						ModeFlags = ((uint32_t)TokenHost[5] << 8) |
												((uint32_t)TokenHost[6]);
				} else if ((TokenHost[4]) == MBBMSCMD_SET_ADDR){	
						//	Bms Cmd state: Set Bms Addr
						BmsNewAddr = ((uint32_t)TokenHost[5] << 24) |
												 ((uint32_t)TokenHost[6] << 16) |
												 ((uint32_t)TokenHost[7] << 8)  |
													(uint32_t)TokenHost[8];
				} else if ((TokenHost[4]) == MBBMSCMD_SET_CELLCOUNT){	
						//	Bms Cmd state: Set Cell numbers
						CellCount =  ((uint32_t)TokenHost[5] << 24) |
												 ((uint32_t)TokenHost[6] << 16) |
												 ((uint32_t)TokenHost[7] << 8)  |
													(uint32_t)TokenHost[8];
				} 
		}	
}

void Host_WtrMtrDataProcess(void)
{
    HostPollingDeviceIdx = TokenHost[3];
		GetHostRTC();
		
		if (TokenHost[4] != 0x00)
		{
				WtrMtrCmdList[HostPollingDeviceIdx-1] = TokenHost[4];
				//	Parse Data according to Cmd
				if ((TokenHost[4]) == MBWMCMD_SET_ADDR_BAUD){
						WMSetDeviceID = TokenHost[5];
						WMBaudRate 		= TokenHost[6];
				}
		}
}

void Host_PyrWtrDataProcess(void)
{
    HostPollingDeviceIdx = TokenHost[3];
		GetHostRTC();
	
		if (TokenHost[4] != 0x00)
		{
				PyrMtrCmdList[HostPollingDeviceIdx-1] = TokenHost[4];
				//	Parse Data according to Cmd
				if ((TokenHost[4]) == MBPYRCMD_SET_DEVIATION_VALUE){
						NewDeviationValue = ((uint32_t)TokenHost[5] << 8) |
																((uint32_t)TokenHost[6]);
				}	else if ((TokenHost[4]) == MBPYRCMD_SET_DEVICE_ADDRESS){
						PyrNewAddr = ((uint32_t)TokenHost[5] << 8) |
												 ((uint32_t)TokenHost[6]);
				}	else if ((TokenHost[4]) == MBPYRCMD_SET_BAUD_RATE){
						PyrNewBaudRate = ((uint32_t)TokenHost[5] << 8) |
														 ((uint32_t)TokenHost[6]);					
				}				
		}
}

void Host_SoilSensorDataProcess(void)
{
		HostPollingDeviceIdx = TokenHost[3];
		GetHostRTC();
		if (TokenHost[4] != 0x00)
		{
				SoilSensorCmdList[HostPollingDeviceIdx-1] = TokenHost[4];
			
				if ((TokenHost[4]) == MBSSCMD_SET_N_VALUE){
						N_RegValue = ((uint32_t)TokenHost[5] << 8) |
												 ((uint32_t)TokenHost[6]);	
				}	else if ((TokenHost[4]) == MBSSCMD_SET_P_VALUE){
						P_RegValue = ((uint32_t)TokenHost[5] << 8) |
												 ((uint32_t)TokenHost[6]);
				}	else if ((TokenHost[4]) == MBSSCMD_SET_K_VALUE){
						K_RegValue = ((uint32_t)TokenHost[5] << 8) |
												 ((uint32_t)TokenHost[6]);
				} else if ((TokenHost[4]) == MBSSCMD_SET_EC_COEF){
						Ec_COEF = ((uint32_t)TokenHost[5] << 8) |
											((uint32_t)TokenHost[6]);
				}	else if ((TokenHost[4]) == MBSSCMD_SET_SALINITY_COEF){
						Salinity_COEF = ((uint32_t)TokenHost[5] << 8) |
														((uint32_t)TokenHost[6]);
				}	else if ((TokenHost[4]) == MBSSCMD_SET_TDS_COEF){
						Tds_COEF = ((uint32_t)TokenHost[5] << 8) |
											 ((uint32_t)TokenHost[6]);
				}	else if ((TokenHost[4]) == MBSSCMD_SET_TEMP_CALIB){
						Temp_CalibValue = ((uint32_t)TokenHost[5] << 8) |
															((uint32_t)TokenHost[6]);
				} else if ((TokenHost[4]) == MBSSCMD_SET_MOISTURE_CALIB){
						Moisture_CalibValue = ((uint32_t)TokenHost[5] << 8) |
																	((uint32_t)TokenHost[6]);
				}	else if ((TokenHost[4]) == MBSSCMD_SET_EC_CALIB){
						Ec_CalibValue = ((uint32_t)TokenHost[5] << 8) |
														((uint32_t)TokenHost[6]);
				} else if ((TokenHost[4]) == MBSSCMD_SET_PH_CALIB){
						Ph_CalibValue = ((uint32_t)TokenHost[5] << 8) |
														((uint32_t)TokenHost[6]);
				}	else if ((TokenHost[4]) == MBSSCMD_SET_FERT_COEF){
						Fert_COEF =  ((uint32_t)TokenHost[5] << 24) |
												 ((uint32_t)TokenHost[6] << 16) |
												 ((uint32_t)TokenHost[7] << 8)  |
													(uint32_t)TokenHost[8];
				}	else if ((TokenHost[4]) == MBSSCMD_SET_FERT_OFFSET){
						Fert_Offset = ((uint32_t)TokenHost[5] << 8) |
													((uint32_t)TokenHost[6]);
				}	else if ((TokenHost[4]) == MBSSCMD_SET_N_COEF){
						N_COEF =  ((uint32_t)TokenHost[5] << 24) |
											((uint32_t)TokenHost[6] << 16) |
											((uint32_t)TokenHost[7] << 8)  |
											 (uint32_t)TokenHost[8];
				} else if ((TokenHost[4]) == MBSSCMD_SET_N_OFFSET){
						N_Offset = ((uint32_t)TokenHost[5] << 8) |
											 ((uint32_t)TokenHost[6]);
				}	else if ((TokenHost[4]) == MBSSCMD_SET_P_COEF){
						P_COEF =  ((uint32_t)TokenHost[5] << 24) |
											((uint32_t)TokenHost[6] << 16) |
											((uint32_t)TokenHost[7] << 8)  |
											 (uint32_t)TokenHost[8];
				}	else if ((TokenHost[4]) == MBSSCMD_SET_P_OFFSET){
						P_Offset = ((uint32_t)TokenHost[5] << 8) |
											 ((uint32_t)TokenHost[6]);
				} else if ((TokenHost[4]) == MBSSCMD_SET_K_COEF){
						K_COEF =  ((uint32_t)TokenHost[5] << 24) |
											((uint32_t)TokenHost[6] << 16) |
											((uint32_t)TokenHost[7] << 8)  |
											 (uint32_t)TokenHost[8];
				} else if ((TokenHost[4]) == MBSSCMD_SET_K_OFFSET){
						K_Offset = ((uint32_t)TokenHost[5] << 8) |
											 ((uint32_t)TokenHost[6]);
				}	else if ((TokenHost[4]) == MBSSCMD_SET_DEVICE_ADDRESS){
						SSNewAddr = ((uint32_t)TokenHost[5] << 8) |
												((uint32_t)TokenHost[6]);
				}	else if ((TokenHost[4]) == MBSSCMD_SET_BAUD_RATE){
						SSNewBaudRate = ((uint32_t)TokenHost[5] << 8) |
														((uint32_t)TokenHost[6]);
				}	
		}

}
void Host_AirSensorDataProcess(void)
{
		HostPollingDeviceIdx = TokenHost[3];
		GetHostRTC();
}

void Host_WateringProcess(void)
{
		GetHostRTC();
		uint16_t u16tmpNowTime;
		
		if (TokenHost[3] != 0x00)
		{		
				LED_G1_On();
		} else {
				LED_G1_Off();
		}
}

/* Get Cmd From Center
0: 0x55
1: OTA MeterID
2: Meter OTA Cmd	(0x17)
3: Sub Cmd
	0x17 : METER_OTA_UPDATE 		 
	0x18 : METER_SWITCH_FW 	
	0x19 : METER_GET_FW_STATUS
	0x1A : METER_REBOOT
*/
void Host_OTAMenterProcess(void)
{
    uint8_t i;
		SYS_UnlockReg();
		FMC_Open();
	
		GetHostRTC();
    HostPollingDeviceIdx = TokenHost[3];
		
		switch(TokenHost[3])
    {
			  case METER_OTA_UPDATE:		//0x20
            WRITE_FW_STATUS_FLAG(OTA_UPDATE_FLAG);
            SendHost_MenterFWinfo();
            JumpToBootloader();
            break;

        case METER_SWITCH_FW:	//0x21
            WRITE_FW_STATUS_FLAG(SWITCH_FW_FLAG);
            SendHost_MenterFWinfo();
            MarkFwAsActive(FALSE);
            JumpToBootloader();
            break;
				
        case METER_GET_FW_STATUS:	//0x22
            SendHost_MenterFWinfo();
            break;
				
        case METER_REBOOT:			//0x23
            WRITE_FW_STATUS_FLAG(REBOOT_FW_FLAG);
            SendHost_MenterFWinfo();
            JumpToBootloader();
            break;

        default:
            break;
		SYS_LockReg();
		}
}

/***
 *	@Not use yet(Or even no need)
 *	@brief		Send center devices status
 *  @devices 	power meter, Bms, water meter, inverter *
 ***/
void SendHost_Ack(void)
{
    uint8_t PktIdx,u8PowerMeterIdx;
    
    u8PowerMeterIdx = MyMeterBoardID-1;
    HostTxBuffer[2] = METER_RSP_SYS_INFO ;	
    HostTxBuffer[3] = fgToHostFlag; 	
    HostTxBuffer[4] = fgToHostRSPFlag; 

    PktIdx = 5 ; 
	
		//	Powermeter NG Status.
    HostTxBuffer[PktIdx++] = (PwrMtrError.PwrMtrDeviceNG & 0xff000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (PwrMtrError.PwrMtrDeviceNG & 0x00ff0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (PwrMtrError.PwrMtrDeviceNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  PwrMtrError.PwrMtrDeviceNG & 0x000000FF ;
		//	BMS NG Status.
		HostTxBuffer[PktIdx++] = (BmsError.BmsDeviceNG & 0xff000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (BmsError.BmsDeviceNG & 0x00ff0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (BmsError.BmsDeviceNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  BmsError.BmsDeviceNG & 0x000000FF ;
		//	Watermeter NG Status.
		HostTxBuffer[PktIdx++] = (WtrMtrError.WMDeviceNG & 0xff000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (WtrMtrError.WMDeviceNG & 0x00ff0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (WtrMtrError.WMDeviceNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  WtrMtrError.WMDeviceNG & 0x000000FF ;

		//	Pyranometer NG Status.
		HostTxBuffer[PktIdx++] = (PyrMtrError.PyrDeviceNG & 0xff000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (PyrMtrError.PyrDeviceNG & 0x00ff0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (PyrMtrError.PyrDeviceNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  PyrMtrError.PyrDeviceNG & 0x000000FF ;

		//	Soil sensor NG Status.
		HostTxBuffer[PktIdx++] = (SoilSensorError.SSDeviceNG & 0xff000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (SoilSensorError.SSDeviceNG & 0x00ff0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (SoilSensorError.SSDeviceNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  SoilSensorError.SSDeviceNG & 0x000000FF ;

		//	Air sensor NG Status.
		HostTxBuffer[PktIdx++] = (AirSensorError.ASDeviceNG & 0xff000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (AirSensorError.ASDeviceNG & 0x00ff0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (AirSensorError.ASDeviceNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  AirSensorError.ASDeviceNG & 0x000000FF ;

		//	INV NG Status.
    HostTxBuffer[PktIdx++] =  InvError.InvDeviceNG & 0x000000FF ;	

		//	Device Error rate
    HostTxBuffer[PktIdx++] = TotErrorRate.PowerMeter;
    HostTxBuffer[PktIdx++] = TotErrorRate.Bms;
    HostTxBuffer[PktIdx++] = TotErrorRate.WaterMeter;
    HostTxBuffer[PktIdx++] = TotErrorRate.Pyranometer;	
    HostTxBuffer[PktIdx++] = TotErrorRate.SoilSensor;
    HostTxBuffer[PktIdx++] = TotErrorRate.AirSensor;		
    HostTxBuffer[PktIdx++] = TotErrorRate.Inverter;
    
    CalChecksumH();	
    	  
}

/***
 *	@breif	Send center, devices status & communication datail
 *  @devices 	power meter, Bms, water meter, inverter, soil sensor, air sensor*
 ***/
void SendHost_SystemInformation(void)
{
    uint8_t PktIdx,u8PowerMeterIdx;
    
    u8PowerMeterIdx = MyMeterBoardID-1;
    HostTxBuffer[2] = METER_RSP_SYS_INFO ;	
    HostTxBuffer[3] = fgToHostFlag; 	
    HostTxBuffer[4] = fgToHostRSPFlag; 

    PktIdx = 5 ; 
	
		//	Powermeter NG Status.
    HostTxBuffer[PktIdx++] = (PwrMtrError.PwrMtrDeviceNG & 0xff000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (PwrMtrError.PwrMtrDeviceNG & 0x00ff0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (PwrMtrError.PwrMtrDeviceNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  PwrMtrError.PwrMtrDeviceNG & 0x000000FF ;
		//	BMS NG Status.
		HostTxBuffer[PktIdx++] = (BmsError.BmsDeviceNG & 0xff000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (BmsError.BmsDeviceNG & 0x00ff0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (BmsError.BmsDeviceNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  BmsError.BmsDeviceNG & 0x000000FF ;
		//	Watermeter NG Status.
		HostTxBuffer[PktIdx++] = (WtrMtrError.WMDeviceNG & 0xff000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (WtrMtrError.WMDeviceNG & 0x00ff0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (WtrMtrError.WMDeviceNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  WtrMtrError.WMDeviceNG & 0x000000FF ;

		//	Pyranometer NG Status.
		HostTxBuffer[PktIdx++] = (PyrMtrError.PyrDeviceNG & 0xff000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (PyrMtrError.PyrDeviceNG & 0x00ff0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (PyrMtrError.PyrDeviceNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  PyrMtrError.PyrDeviceNG & 0x000000FF ;

		//	Soil sensor NG Status.
		HostTxBuffer[PktIdx++] = (SoilSensorError.SSDeviceNG & 0xff000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (SoilSensorError.SSDeviceNG & 0x00ff0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (SoilSensorError.SSDeviceNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  SoilSensorError.SSDeviceNG & 0x000000FF ;

		//	Air sensor NG Status.
		HostTxBuffer[PktIdx++] = (AirSensorError.ASDeviceNG & 0xff000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (AirSensorError.ASDeviceNG & 0x00ff0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (AirSensorError.ASDeviceNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  AirSensorError.ASDeviceNG & 0x000000FF ;

		//	INV NG Status.
    HostTxBuffer[PktIdx++] =  InvError.InvDeviceNG & 0x000000FF ;	

		//	Device Error rate
    HostTxBuffer[PktIdx++] = TotErrorRate.PowerMeter;
    HostTxBuffer[PktIdx++] = TotErrorRate.Bms;
    HostTxBuffer[PktIdx++] = TotErrorRate.WaterMeter;
    HostTxBuffer[PktIdx++] = TotErrorRate.Pyranometer;	
    HostTxBuffer[PktIdx++] = TotErrorRate.SoilSensor;
    HostTxBuffer[PktIdx++] = TotErrorRate.AirSensor;		
    HostTxBuffer[PktIdx++] = TotErrorRate.Inverter;
    
    CalChecksumH();	
    
}

void SendHost_PwrMtrData(void)
{
    uint8_t PktIdx,PwrMtrIdx;
	
    PwrMtrIdx = HostPollingDeviceIdx-1;
    HostTxBuffer[2] = METER_RSP_POWER_DATA ;	
    HostTxBuffer[3] = fgToHostFlag; 	
    HostTxBuffer[4] = PwrMtrIdx ; 	
    PktIdx = 5;
    HostTxBuffer[PktIdx++] = PwrMtrError.ErrorRate[PwrMtrIdx];
	
    HostTxBuffer[PktIdx++] = MeterData[PwrMtrIdx].RelayStatus;
    // Total 
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrIdx].TotalWatt & 0xff000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrIdx].TotalWatt & 0x00ff0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrIdx].TotalWatt & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  MeterData[PwrMtrIdx].TotalWatt & 0x000000FF ;

    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrIdx].V & 0xff000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrIdx].V & 0x00ff0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrIdx].V & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  MeterData[PwrMtrIdx].V & 0x000000FF ;
	
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrIdx].I & 0xff000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrIdx].I & 0x00ff0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrIdx].I & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  MeterData[PwrMtrIdx].I & 0x000000FF ;

    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrIdx].F & 0xff000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrIdx].F & 0x00ff0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrIdx].F & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  MeterData[PwrMtrIdx].F & 0x000000FF ;

    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrIdx].P & 0xff000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrIdx].P & 0x00ff0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrIdx].P & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  MeterData[PwrMtrIdx].P & 0x000000FF ;

    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrIdx].S & 0xff000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrIdx].S & 0x00ff0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrIdx].S & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  MeterData[PwrMtrIdx].S & 0x000000FF ;
	
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrIdx].PwrFactor & 0xff000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrIdx].PwrFactor & 0x00ff0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrIdx].PwrFactor & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  MeterData[PwrMtrIdx].PwrFactor & 0x000000FF ;

    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrIdx].Balance & 0xff000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrIdx].Balance & 0x00ff0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrIdx].Balance & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  MeterData[PwrMtrIdx].Balance & 0x000000FF ;
	
    CalChecksumH();			
}

/***
 *	@brief		Send Host Bms Data
 *
 ***/
void SendHost_BmsData(void)
{
    uint8_t PktIdx,BmsIdx;
    BmsIdx = HostPollingDeviceIdx-1;
	
    HostTxBuffer[2] = METER_RSP_BMS_DATA ;	
    HostTxBuffer[3] = fgToHostFlag; 	
    HostTxBuffer[4] = BmsIdx ; 	
    PktIdx = 5;
	
		HostTxBuffer[PktIdx++] = BmsError.ErrorRate[BmsIdx];			// Communicate rate
		HostTxBuffer[PktIdx++] = BmsData[BmsIdx].BalanceStatus;	// Battery mode
		HostTxBuffer[PktIdx++] = BmsData[BmsIdx].StateOfCharge;
		HostTxBuffer[PktIdx++] = BmsData[BmsIdx].StateOfHealth;	//	SOH
		//idx:9	Cell ststus 
		HostTxBuffer[PktIdx++] = (BmsData[BmsIdx].CellStatus & 0xff000000) >> 24 ;
		HostTxBuffer[PktIdx++] = (BmsData[BmsIdx].CellStatus & 0x00ff0000) >> 16 ;
		HostTxBuffer[PktIdx++] = (BmsData[BmsIdx].CellStatus & 0x0000FF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  BmsData[BmsIdx].CellStatus & 0x000000FF;	
		//idx:13	Cell volt
		for (uint8_t i = 0; i < 16; i++) {
				HostTxBuffer[PktIdx++] = (BmsData[BmsIdx].CellVolt[i] >> 8);
				HostTxBuffer[PktIdx++] = BmsData[BmsIdx].CellVolt[i];
		}
		//idx:45	Battery watt 
		HostTxBuffer[PktIdx++] = (BmsData[BmsIdx].BatWatt & 0xff000000) >> 24 ;
		HostTxBuffer[PktIdx++] = (BmsData[BmsIdx].BatWatt & 0x00ff0000) >> 16 ;
		HostTxBuffer[PktIdx++] = (BmsData[BmsIdx].BatWatt & 0x0000FF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  BmsData[BmsIdx].BatWatt & 0x000000FF;	
		//idx:49	Battery voltage 
		HostTxBuffer[PktIdx++] = (BmsData[BmsIdx].BatVolt & 0xff000000) >> 24 ;
		HostTxBuffer[PktIdx++] = (BmsData[BmsIdx].BatVolt & 0x00ff0000) >> 16 ;
		HostTxBuffer[PktIdx++] = (BmsData[BmsIdx].BatVolt & 0x0000FF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  BmsData[BmsIdx].BatVolt & 0x000000FF;	
		//idx:53	Battery current 
		HostTxBuffer[PktIdx++] = (BmsData[BmsIdx].BatCurrent & 0xff000000) >> 24 ;
		HostTxBuffer[PktIdx++] = (BmsData[BmsIdx].BatCurrent & 0x00ff0000) >> 16 ;
		HostTxBuffer[PktIdx++] = (BmsData[BmsIdx].BatCurrent & 0x0000FF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  BmsData[BmsIdx].BatCurrent & 0x000000FF;			
		//idx:63	Battery temperature [1-5]	
		for (uint8_t i = 0; i<5; i++)
		{		
				HostTxBuffer[PktIdx++] = (BmsData[BmsIdx].BatteryTemp[i] >> 8);
				HostTxBuffer[PktIdx++] =  BmsData[BmsIdx].BatteryTemp[i];
		}
		//idx:65 	Mos temperature 
		HostTxBuffer[PktIdx++] = (BmsData[BmsIdx].MosTemp >> 8);
		HostTxBuffer[PktIdx++] =  BmsData[BmsIdx].MosTemp;
		
		CalChecksumH();			
}


/***
 *	@brief		Send Host water meter Data
 *	@Send			Communication rate Valve status, Totla used water volume
 *
 ***/
void SendHost_WtrMtrData(void)
{
    uint8_t PktIdx,WtrMtrIdx;
    WtrMtrIdx = HostPollingDeviceIdx-1;
	
    HostTxBuffer[2] = METER_RSP_WATER_DATA ;	
    HostTxBuffer[3] = fgToHostFlag; 	
    HostTxBuffer[4] = WtrMtrIdx ; 	
    PktIdx = 5;
	
	
		HostTxBuffer[PktIdx++] = WtrMtrError.ErrorRate[WtrMtrIdx];			// Communicate rate
		HostTxBuffer[PktIdx++] = WMData[WtrMtrIdx].ValveState;			// 0xff : closed, 0x00 : opened
	
		HostTxBuffer[PktIdx++] = (WMData[WtrMtrIdx].TotalVolume & 0xff000000) >> 24 ;
		HostTxBuffer[PktIdx++] = (WMData[WtrMtrIdx].TotalVolume & 0x00ff0000) >> 16 ;
		HostTxBuffer[PktIdx++] = (WMData[WtrMtrIdx].TotalVolume & 0x0000FF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  WMData[WtrMtrIdx].TotalVolume & 0x000000FF;		
    
		CalChecksumH();			
}

/***
 *	@brief		Send Host  Pyranometer Data
 *	@Send			Communication rate, Offste Value, Solaer radiation
 *
 ***/
void SendHost_PyrMtrData(void)
{
    uint8_t PktIdx,PyrMtrIdx;
    PyrMtrIdx = HostPollingDeviceIdx-1;
	
    HostTxBuffer[2] =  METER_RSP_PYR_DATA;	
    HostTxBuffer[3] = fgToHostFlag; 	
    HostTxBuffer[4] = PyrMtrIdx ; 	
    PktIdx = 5;
	
		HostTxBuffer[PktIdx++] = PyrMtrError.ErrorRate[PyrMtrIdx];			// Communicate rate
		HostTxBuffer[PktIdx++] = PyrMtrData[PyrMtrIdx].OffsetValue & 0xff00 >> 8;
		HostTxBuffer[PktIdx++] = PyrMtrData[PyrMtrIdx].OffsetValue & 0x00ff;
	
		HostTxBuffer[PktIdx++] = (PyrMtrData[PyrMtrIdx].SolarRadiation & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  PyrMtrData[PyrMtrIdx].SolarRadiation & 0x00ff;		
	
		CalChecksumH();			
}

/***
 *	@brief		Send Host Soil sensor Data
 ***/
void SendHost_SoilData(void)
{
    uint8_t PktIdx,SoilSensorIdx;
		SoilSensorIdx = HostPollingDeviceIdx-1;
    
    HostTxBuffer[2] =  METER_RSP_SOIL_DATA;	
    HostTxBuffer[3] = fgToHostFlag; 	
    HostTxBuffer[4] = SoilSensorIdx ; 	
    PktIdx = 5;

		HostTxBuffer[PktIdx++] = SoilSensorError.ErrorRate[SoilSensorIdx];			// Communicate rate
	
		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].Moisture & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SoilSensorIdx].Moisture & 0x00ff;

		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].Temperature & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SoilSensorIdx].Temperature & 0x00ff;
	
		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].EC & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SoilSensorIdx].EC & 0x00ff;

		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].PH & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SoilSensorIdx].PH & 0x00ff;
	
		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].Nitrogen & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SoilSensorIdx].Nitrogen & 0x00ff;
	
		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].Phosphorus & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SoilSensorIdx].Phosphorus & 0x00ff;

		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].Potassium & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SoilSensorIdx].Potassium & 0x00ff;
		
		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].Salinity & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SoilSensorIdx].Salinity & 0x00ff;
	
		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].TDS & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SoilSensorIdx].TDS & 0x00ff;
	
		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].Fertility & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SoilSensorIdx].Fertility & 0x00ff;
		
		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].EC_Coef & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SoilSensorIdx].EC_Coef & 0x00ff;
		
		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].Salinity_Coef & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SoilSensorIdx].Salinity_Coef & 0x00ff;
	
		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].TDS_Coef & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SoilSensorIdx].TDS_Coef & 0x00ff;

		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].Temp_Calib & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SoilSensorIdx].Temp_Calib & 0x00ff;
	
		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].Moisture_Calib & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SoilSensorIdx].Moisture_Calib & 0x00ff;
		
		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].EC_Calib & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SoilSensorIdx].EC_Calib & 0x00ff;
		
		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].PH_Calib & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SoilSensorIdx].PH_Calib & 0x00ff;
		
		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].Fert_Coef & 0xff000000) >> 24 ;
		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].Fert_Coef & 0x00ff0000) >> 16 ;
		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].Fert_Coef & 0x0000FF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SoilSensorIdx].Fert_Coef & 0x000000FF;	
	
		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].Fert_Deviation & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SoilSensorIdx].Fert_Deviation & 0x00ff;

		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].Nitrogen_Coef & 0xff000000) >> 24 ;
		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].Nitrogen_Coef & 0x00ff0000) >> 16 ;
		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].Nitrogen_Coef & 0x0000FF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SoilSensorIdx].Nitrogen_Coef & 0x000000FF;	
	
		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].Nitrogen_Deviation & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SoilSensorIdx].Nitrogen_Deviation & 0x00ff;

		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].Phosphorus_Coef & 0xff000000) >> 24 ;
		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].Phosphorus_Coef & 0x00ff0000) >> 16 ;
		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].Phosphorus_Coef & 0x0000FF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SoilSensorIdx].Phosphorus_Coef & 0x000000FF;	
		
		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].Phosphorus_Deviation & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SoilSensorIdx].Phosphorus_Deviation & 0x00ff;
		
		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].Potassium_Coef & 0xff000000) >> 24 ;
		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].Potassium_Coef & 0x00ff0000) >> 16 ;
		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].Potassium_Coef & 0x0000FF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SoilSensorIdx].Potassium_Coef & 0x000000FF;	
		
		HostTxBuffer[PktIdx++] = (SoilSensorData[SoilSensorIdx].Potassium_Deviation & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SoilSensorIdx].Potassium_Deviation & 0x00ff;

		CalChecksumH();	
}

/***
 *	@brief		Send Host Air sensor Data
 ***/
void SendHost_AirData(void)
{
    uint8_t PktIdx,AirSensorIdx;
	
    AirSensorIdx = HostPollingDeviceIdx-1;
	
    HostTxBuffer[2] =  METER_RSP_AIR_DATA;	
    HostTxBuffer[3] = fgToHostFlag; 	
    HostTxBuffer[4] = AirSensorIdx ; 	
    PktIdx = 5;		

		HostTxBuffer[PktIdx++] = AirSensorError.ErrorRate[AirSensorIdx];			// Communicate rate
	
		HostTxBuffer[PktIdx++] = (AirSensorData[AirSensorIdx].Co2 & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  AirSensorData[AirSensorIdx].Co2 & 0x00ff;

		HostTxBuffer[PktIdx++] = (AirSensorData[AirSensorIdx].Formaldehyde & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  AirSensorData[AirSensorIdx].Formaldehyde & 0x00ff;	
	
		HostTxBuffer[PktIdx++] = (AirSensorData[AirSensorIdx].Tvoc & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  AirSensorData[AirSensorIdx].Tvoc & 0x00ff;

		HostTxBuffer[PktIdx++] = (AirSensorData[AirSensorIdx].Pm25 & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  AirSensorData[AirSensorIdx].Pm25 & 0x00ff;	

		HostTxBuffer[PktIdx++] = (AirSensorData[AirSensorIdx].Pm10 & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  AirSensorData[AirSensorIdx].Pm10 & 0x00ff;

		HostTxBuffer[PktIdx++] = (AirSensorData[AirSensorIdx].Temperature & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  AirSensorData[AirSensorIdx].Temperature & 0x00ff;	
	
		HostTxBuffer[PktIdx++] = (AirSensorData[AirSensorIdx].Humidity & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  AirSensorData[AirSensorIdx].Humidity & 0x00ff;
	
		CalChecksumH();	
}

/***
 *	@brief		Send Host Inverter Data
 ***/
void SendHost_InvData(void)
{
    uint8_t PktIdx;
	
    HostTxBuffer[2] = METER_RSP_INV_DATA ;	
    HostTxBuffer[3] = fgToHostFlag; 		
    PktIdx = 5;
	
		HostTxBuffer[PktIdx++] = InvError.ErrorRate;			// Communicate rate
		HostTxBuffer[PktIdx++] = InvData.statusByte1;
		HostTxBuffer[PktIdx++] = InvData.statusByte3;
		HostTxBuffer[PktIdx++] = InvData.warnByte1;		
		HostTxBuffer[PktIdx++] = InvData.warnByte2;
		HostTxBuffer[PktIdx++] = InvData.faultByte1;			
		HostTxBuffer[PktIdx++] = InvData.faultByte2;
		HostTxBuffer[PktIdx++] = InvData.faultByte3;

		HostTxBuffer[PktIdx++] = (InvData.InputVolt & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  InvData.InputVolt & 0x00ff;
		HostTxBuffer[PktIdx++] = (InvData.InputFreq & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  InvData.InputFreq & 0x00ff;		

		HostTxBuffer[PktIdx++] = (InvData.OutputVolt & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  InvData.OutputVolt & 0x00ff;
		HostTxBuffer[PktIdx++] = (InvData.OutputFreq & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  InvData.OutputFreq & 0x00ff;
		
		HostTxBuffer[PktIdx++] = (InvData.BatVolt & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  InvData.BatVolt & 0x00ff;

		HostTxBuffer[PktIdx++] = InvData.BatCapacity;		
		HostTxBuffer[PktIdx++] = InvData.InvCurrent;
		HostTxBuffer[PktIdx++] = InvData.LoadPercentage;			
		HostTxBuffer[PktIdx++] = InvData.MachineTemp;
		HostTxBuffer[PktIdx++] = InvData.MachineStatusCode;
		HostTxBuffer[PktIdx++] = InvData.SysStatus;		
		
		HostTxBuffer[PktIdx++] = (InvData.PV_volt & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  InvData.PV_volt & 0x00ff;
		
		HostTxBuffer[PktIdx++] = InvData.CtrlCurrent;	
		HostTxBuffer[PktIdx++] = InvData.CtrlTemp;
		HostTxBuffer[PktIdx++] = InvData.CtrlStatusCode;	
		
		CalChecksumH();			
}

void SendHost_WateringStatus(void)
{
    HostTxBuffer[2] = METER_RSP_WATERING_STATUS ;	
    HostTxBuffer[3] = fgToHostFlag;

		HostTxBuffer[6] = ~PE0;
		CalChecksumH();	
}

/* Send Fw info to Center
0: 0x55
1: MeterID
2: Cmd (0x17)
4-19: FWstatus (16 byte)
20-51: FWMetadata1 (32 byte)
52-83: FWMetadata2 (32 byte)
*/
void SendHost_MenterFWinfo(void)
{	
	// Send Meter Fw Metadata to host, then go to meterBootloader

		uint8_t i;
		HostTxBuffer[2] = METER_OTA_UPDATE;	

		memcpy(&HostTxBuffer[4], &g_fw_ctx, sizeof(FWstatus));
    memcpy(&HostTxBuffer[20], &meta, sizeof(FWMetadata));
		memcpy(&HostTxBuffer[52], &other, sizeof(FWMetadata));
	
		CalChecksumH();
}

/* When OTA update success, send success password to Center 
0: 0x55
1: MyMeterBoardID
4: 0x0A
5: 0xBB
6: 0xC0
7: 0xDD
*/
void SendHost_MenterFWActivatedInfo(void)
{	
	// Send Meter validated Password to host, let Center Pop out of OTAMode
		HostTxBuffer[4] = 0x0A;		HostTxBuffer[5] = 0xBB;
		HostTxBuffer[6] = 0xC0;		HostTxBuffer[7] = 0xDD;
	
		CalChecksumH();
}


void CalChecksumH(void)
{
		uint8_t i;
		uint8_t Checksum;

		HostTxBuffer[0] = 0x55 ;
		HostTxBuffer[1] = MyMeterBoardID ;
		Checksum = 0 ;
		for (i=1;i<(MAX_HOST_TXQ_LENGTH-2);i++)
		{

			Checksum += HostTxBuffer[i];
		}	
		HostTxBuffer[MAX_HOST_TXQ_LENGTH-2] = Checksum ;
		HostTxBuffer[MAX_HOST_TXQ_LENGTH-1] = '\n' ; 
		EnableHostUartTx(); 
		//DIR485_HOST_Out();
		_SendStringToHOST(HostTxBuffer,MAX_HOST_TXQ_LENGTH);
	
}

void SystemSwitchProcess(void)
{
		float floatTemp;
		uint8_t * tmpAddr;
		
		// Reader SW
		if ( TokenHost[3] == 0xA5 )
		{			
				// Meter LED ON
				if ( TokenHost[4] == 0x05 )
				{
					LED_G_On();
				}
				// Meter LED OFF
				if ( TokenHost[4] == 0x50 )
				{
					LED_G_Off();
				}
				if ( TokenHost[4] == 0x06 )
				{
					//SSR_Off();
				}
				if ( TokenHost[4] == 0x60 )
				{
					//SSR_On();
				}
				if ( TokenHost[4] == 0x70 )
				{
					
				}
				if ( (TokenHost[4] == 0x85) && (TokenHost[5] == 0xBB) )

				{
					ReadDeviceCmdTime = TokenHost[6] ;
				}
				if ( (TokenHost[4] == 0x86) && (TokenHost[5] == 0xBB))

				{
					MeterType = TokenHost[6] ;
				}
				
				if ( (TokenHost[4]  == 0x5B)  && (TokenHost[5]  == 0xCC) )
				{			
					ResetHostUART();
					ResetReaderUART();		
				}
				
				if ( (TokenHost[4]  == 0x5A)  && (TokenHost[5]  == 0xBB) )
				{
					NVIC_SystemReset(); 
				}

				if ( TokenHost[20] == 0xAA )
				{		
					tmpAddr = (uint8_t*) &floatTemp;
					tmpAddr[0] = TokenHost[21];
					tmpAddr[1] = TokenHost[22];
					tmpAddr[2] = TokenHost[23];
					tmpAddr[3] = TokenHost[24];
					Min_LowBalance = floatTemp;
				}
			
		}
	
}

//	Get RTC from center
void GetHostRTC(void)
{
		for (uint8_t i = 0; i < 8; i++) {
        iSystemTime[i] = TokenHost[INX_TIME_START_YY_H + i];
    }
}

uint8_t _SendStringToHOST(uint8_t *Str, uint8_t len)
{
	
	uint8_t idx;

	if( (HOSTTxQ_cnt+len) > MAX_HOST_TXQ_LENGTH )
	{
		return 0x01 ;
	} else {
	        
		EnableHostUartTx();
		for(idx=0;idx<len;idx++)
		{
			HOSTTxQ[HOSTTxQ_wp]=Str[idx];
			HOSTTxQ_wp++;
			if(HOSTTxQ_wp>=MAX_HOST_TXQ_LENGTH)
			{
				HOSTTxQ_wp=0;
			}
			HOSTTxQ_cnt++;
		}        				
		UART_EnableInt(UART0, (UART_INTEN_THREIEN_Msk ));
		NVIC_EnableIRQ(UART02_IRQn);
               
	}  
	while (!(UART1->FIFOSTS & UART_FIFOSTS_TXEMPTYF_Msk));	
	return 0x00 ;
}




