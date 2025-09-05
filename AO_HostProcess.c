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
void Host_InvDataProcess(void);

void SendHost_Ack(void);
void SendHost_SystemInformation(void);
void SendHost_PwrMtrData(void);
void SendHost_BmsData(void);
void SendHost_WtrMtrData(void);
void SendHost_PyrMtrData(void);
void SendHost_SoilData(void);
void SendHost_AirData(void);
void SendHost_InvData(void);

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
                        Host_InvDataProcess();                        
                        CmdType = METER_RSP_INV_DATA ;
                        ClearRespDelayTimer() ;	
                        break;
                    case CMD_MTR_OTA_UPDATE:
                        Host_OTAMenterProcess();
                        ClearRespDelayTimer();
                        break;
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
}

void Host_InvDataProcess(void)
{
		//	Maybe no need
    HostPollingDeviceIdx = TokenHost[3];
}

/* Get Cmd From Center
0: 0x55
1: OTA MeterID
2: Meter OTA Cmd	(0x17)
3: Sub Cmd
	0x17 : CMD_MTR_OTA_UPDATE 		 
	0x18 : CMD_MTR_SWITCH_FWVER 	
	0x19 : CMD_GET_MTR_FW_STATUS
	0x1A : CMD_MTR_FW_REBOOT
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
			  case CMD_MTR_OTA_UPDATE:
            WRITE_FW_STATUS_FLAG(OTA_UPDATE_FLAG);
            SendHost_MenterFWinfo();
            JumpToBootloader();
            break;

        case CMD_MTR_SWITCH_FWVER:
            WRITE_FW_STATUS_FLAG(SWITCH_FW_FLAG);
            SendHost_MenterFWinfo();
            MarkFwAsActive(FALSE);
            JumpToBootloader();
            break;
				
        case CMD_GET_MTR_FW_STATUS:
            SendHost_MenterFWinfo();
            break;
				
        case CMD_MTR_FW_REBOOT:
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
    HostTxBuffer[PktIdx++] = (PowerMeterNG & 0xFF000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (PowerMeterNG & 0x00FF0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (PowerMeterNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  PowerMeterNG & 0x000000FF ;
		//	BMS NG Status.
		HostTxBuffer[PktIdx++] = (BmsError.BmsDeviceNG & 0xFF000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (BmsError.BmsDeviceNG & 0x00FF0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (BmsError.BmsDeviceNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  BmsError.BmsDeviceNG & 0x000000FF ;
		//	Watermeter NG Status.
		HostTxBuffer[PktIdx++] = (WtrMtrError.WMDeviceNG & 0xFF000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (WtrMtrError.WMDeviceNG & 0x00FF0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (WtrMtrError.WMDeviceNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  WtrMtrError.WMDeviceNG & 0x000000FF ;

		//	Pyranometer NG Status.
		HostTxBuffer[PktIdx++] = (PyrMtrError.PyrDeviceNG & 0xFF000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (PyrMtrError.PyrDeviceNG & 0x00FF0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (PyrMtrError.PyrDeviceNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  PyrMtrError.PyrDeviceNG & 0x000000FF ;

		//	Soil sensor NG Status.
		HostTxBuffer[PktIdx++] = (SoilSensorError.SSDeviceNG & 0xFF000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (SoilSensorError.SSDeviceNG & 0x00FF0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (SoilSensorError.SSDeviceNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  SoilSensorError.SSDeviceNG & 0x000000FF ;

		//	Air sensor NG Status.
		HostTxBuffer[PktIdx++] = (AirSensorError.ASDeviceNG & 0xFF000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (AirSensorError.ASDeviceNG & 0x00FF0000) >> 16 ;
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
    HostTxBuffer[PktIdx++] = (PowerMeterNG & 0xFF000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (PowerMeterNG & 0x00FF0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (PowerMeterNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  PowerMeterNG & 0x000000FF ;
		//	BMS NG Status.
		HostTxBuffer[PktIdx++] = (BmsError.BmsDeviceNG & 0xFF000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (BmsError.BmsDeviceNG & 0x00FF0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (BmsError.BmsDeviceNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  BmsError.BmsDeviceNG & 0x000000FF ;
		//	Watermeter NG Status.
		HostTxBuffer[PktIdx++] = (WtrMtrError.WMDeviceNG & 0xFF000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (WtrMtrError.WMDeviceNG & 0x00FF0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (WtrMtrError.WMDeviceNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  WtrMtrError.WMDeviceNG & 0x000000FF ;

		//	Pyranometer NG Status.
		HostTxBuffer[PktIdx++] = (PyrMtrError.PyrDeviceNG & 0xFF000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (PyrMtrError.PyrDeviceNG & 0x00FF0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (PyrMtrError.PyrDeviceNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  PyrMtrError.PyrDeviceNG & 0x000000FF ;

		//	Soil sensor NG Status.
		HostTxBuffer[PktIdx++] = (SoilSensorError.SSDeviceNG & 0xFF000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (SoilSensorError.SSDeviceNG & 0x00FF0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (SoilSensorError.SSDeviceNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  SoilSensorError.SSDeviceNG & 0x000000FF ;

		//	Air sensor NG Status.
		HostTxBuffer[PktIdx++] = (AirSensorError.ASDeviceNG & 0xFF000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (AirSensorError.ASDeviceNG & 0x00FF0000) >> 16 ;
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
    uint8_t PktIdx,PwrMtrArrayIdx;
	
    PwrMtrArrayIdx = HostPollingDeviceIdx-1;
    HostTxBuffer[2] = METER_RSP_POWER_DATA ;	
    HostTxBuffer[3] = fgToHostFlag; 	
    HostTxBuffer[4] = PwrMtrArrayIdx ; 	
    PktIdx = 5;
    HostTxBuffer[PktIdx++] = MeterErrorRate[PwrMtrArrayIdx];
	
    HostTxBuffer[PktIdx++] = MeterData[PwrMtrArrayIdx].RelayStatus;
    // Total 
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrArrayIdx].TotalWatt & 0xFF000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrArrayIdx].TotalWatt & 0x00FF0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrArrayIdx].TotalWatt & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  MeterData[PwrMtrArrayIdx].TotalWatt & 0x000000FF ;

    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrArrayIdx].V & 0xFF000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrArrayIdx].V & 0x00FF0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrArrayIdx].V & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  MeterData[PwrMtrArrayIdx].V & 0x000000FF ;
	
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrArrayIdx].I & 0xFF000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrArrayIdx].I & 0x00FF0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrArrayIdx].I & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  MeterData[PwrMtrArrayIdx].I & 0x000000FF ;

    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrArrayIdx].F & 0xFF000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrArrayIdx].F & 0x00FF0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrArrayIdx].F & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  MeterData[PwrMtrArrayIdx].F & 0x000000FF ;

    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrArrayIdx].P & 0xFF000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrArrayIdx].P & 0x00FF0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrArrayIdx].P & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  MeterData[PwrMtrArrayIdx].P & 0x000000FF ;

    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrArrayIdx].S & 0xFF000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrArrayIdx].S & 0x00FF0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrArrayIdx].S & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  MeterData[PwrMtrArrayIdx].S & 0x000000FF ;
	
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrArrayIdx].PwrFactor & 0xFF000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrArrayIdx].PwrFactor & 0x00FF0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrArrayIdx].PwrFactor & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  MeterData[PwrMtrArrayIdx].PwrFactor & 0x000000FF ;

    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrArrayIdx].Balance & 0xFF000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrArrayIdx].Balance & 0x00FF0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (MeterData[PwrMtrArrayIdx].Balance & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  MeterData[PwrMtrArrayIdx].Balance & 0x000000FF ;
		
		HostTxBuffer[PktIdx++] = MeterData[PwrMtrArrayIdx].Mode;
	
    CalChecksumH();			
}

/***
 *	@brief		Send Host Bms Data
 *
 ***/
void SendHost_BmsData(void)
{
    uint8_t PktIdx,BmsArrayIdx;
    BmsArrayIdx = HostPollingDeviceIdx-1;
	
    HostTxBuffer[2] = METER_RSP_BMS_DATA ;	
    HostTxBuffer[3] = fgToHostFlag; 	
    HostTxBuffer[4] = BmsArrayIdx ; 	
    PktIdx = 5;
	
		HostTxBuffer[PktIdx++] = BmsError.ErrorRate[BmsArrayIdx];			// Communicate rate
		HostTxBuffer[PktIdx++] = BmsData[BmsArrayIdx].BalanceStatus;	// Battery mode
		HostTxBuffer[PktIdx++] = BmsData[BmsArrayIdx].StateOfCharge;
		HostTxBuffer[PktIdx++] = BmsData[BmsArrayIdx].StateOfHealth;	//	SOH
		//idx:9	Cell ststus 
		HostTxBuffer[PktIdx++] = (BmsData[BmsArrayIdx].CellStatus & 0xFF000000) >> 24 ;
		HostTxBuffer[PktIdx++] = (BmsData[BmsArrayIdx].CellStatus & 0x00FF0000) >> 16 ;
		HostTxBuffer[PktIdx++] = (BmsData[BmsArrayIdx].CellStatus & 0x0000FF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  BmsData[BmsArrayIdx].CellStatus & 0x000000FF;	
		//idx:13	Cell volt
		for (uint8_t i = 0; i < 16; i++) {
				HostTxBuffer[PktIdx++] = (BmsData[BmsArrayIdx].CellVolt[i] >> 8);
				HostTxBuffer[PktIdx++] = BmsData[BmsArrayIdx].CellVolt[i];
		}
		//idx:45	Battery watt 
		HostTxBuffer[PktIdx++] = (BmsData[BmsArrayIdx].BatWatt & 0xFF000000) >> 24 ;
		HostTxBuffer[PktIdx++] = (BmsData[BmsArrayIdx].BatWatt & 0x00FF0000) >> 16 ;
		HostTxBuffer[PktIdx++] = (BmsData[BmsArrayIdx].BatWatt & 0x0000FF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  BmsData[BmsArrayIdx].BatWatt & 0x000000FF;	
		//idx:49	Battery voltage 
		HostTxBuffer[PktIdx++] = (BmsData[BmsArrayIdx].BatVolt & 0xFF000000) >> 24 ;
		HostTxBuffer[PktIdx++] = (BmsData[BmsArrayIdx].BatVolt & 0x00FF0000) >> 16 ;
		HostTxBuffer[PktIdx++] = (BmsData[BmsArrayIdx].BatVolt & 0x0000FF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  BmsData[BmsArrayIdx].BatVolt & 0x000000FF;	
		//idx:53	Battery current 
		HostTxBuffer[PktIdx++] = (BmsData[BmsArrayIdx].BatCurrent & 0xFF000000) >> 24 ;
		HostTxBuffer[PktIdx++] = (BmsData[BmsArrayIdx].BatCurrent & 0x00FF0000) >> 16 ;
		HostTxBuffer[PktIdx++] = (BmsData[BmsArrayIdx].BatCurrent & 0x0000FF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  BmsData[BmsArrayIdx].BatCurrent & 0x000000FF;			
		//idx:63	Battery temperature [1-5]	
		for (uint8_t i = 0; i<5; i++)
		{		
				HostTxBuffer[PktIdx++] = (BmsData[BmsArrayIdx].BatteryTemp[i] >> 8);
				HostTxBuffer[PktIdx++] =  BmsData[BmsArrayIdx].BatteryTemp[i];
		}
		//idx:65 	Mos temperature 
		HostTxBuffer[PktIdx++] = (BmsData[BmsArrayIdx].MosTemp >> 8);
		HostTxBuffer[PktIdx++] =  BmsData[BmsArrayIdx].MosTemp;
		
		CalChecksumH();			
}


/***
 *	@brief		Send Host water meter Data
 *	@Send			Communication rate Valve status, Totla used water volume
 *
 ***/
void SendHost_WtrMtrData(void)
{
    uint8_t PktIdx,WtrMtrArrayIdx;
    WtrMtrArrayIdx = HostPollingDeviceIdx-1;
	
    HostTxBuffer[2] = METER_RSP_WATER_DATA ;	
    HostTxBuffer[3] = fgToHostFlag; 	
    HostTxBuffer[4] = WtrMtrArrayIdx ; 	
    PktIdx = 5;
	
	
		HostTxBuffer[PktIdx++] = WtrMtrError.ErrorRate[WtrMtrArrayIdx];			// Communicate rate
		HostTxBuffer[PktIdx++] = WMData[WtrMtrArrayIdx].ValveState;			// 0xff : closed, 0x00 : opened
	
		HostTxBuffer[PktIdx++] = (WMData[WtrMtrArrayIdx].TotalVolume & 0xFF000000) >> 24 ;
		HostTxBuffer[PktIdx++] = (WMData[WtrMtrArrayIdx].TotalVolume & 0x00FF0000) >> 16 ;
		HostTxBuffer[PktIdx++] = (WMData[WtrMtrArrayIdx].TotalVolume & 0x0000FF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  WMData[WtrMtrArrayIdx].TotalVolume & 0x000000FF;		
    
		CalChecksumH();			
}

/***
 *	@brief		Send Host  Pyranometer Data
 *	@Send			Communication rate, Offste Value, Solaer radiation
 *
 ***/
void SendHost_PyrMtrData(void)
{
    uint8_t PktIdx,PyrArrayIdx;
    PyrArrayIdx = HostPollingDeviceIdx-1;
	
    HostTxBuffer[2] =  METER_RSP_PYR_DATA;	
    HostTxBuffer[3] = fgToHostFlag; 	
    HostTxBuffer[4] = PyrArrayIdx ; 	
    PktIdx = 5;
	
		HostTxBuffer[PktIdx++] = PyrMtrError.ErrorRate[PyrArrayIdx];			// Communicate rate
		HostTxBuffer[PktIdx++] = PyrMtrData[PyrArrayIdx].OffsetValue & 0xFF00 >> 8;
		HostTxBuffer[PktIdx++] = PyrMtrData[PyrArrayIdx].OffsetValue & 0x00FF;
	
		HostTxBuffer[PktIdx++] = (PyrMtrData[PyrArrayIdx].SolarRadiation & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  PyrMtrData[PyrArrayIdx].SolarRadiation & 0x00FF;		
	
		CalChecksumH();			
}

/***
 *	@brief		Send Host Soil sensor Data
 ***/
void SendHost_SoilData(void)
{
    uint8_t PktIdx,SSArrayIdx;
    
    HostTxBuffer[2] =  METER_RSP_SOIL_DATA;	
    HostTxBuffer[3] = fgToHostFlag; 	
    HostTxBuffer[4] = SSArrayIdx ; 	
    PktIdx = 5;

		HostTxBuffer[PktIdx++] = SoilSensorError.ErrorRate[SSArrayIdx];			// Communicate rate
	
		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].Moisture & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SSArrayIdx].Moisture & 0x00FF;

		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].Temperature & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SSArrayIdx].Temperature & 0x00FF;
	
		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].EC & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SSArrayIdx].EC & 0x00FF;

		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].PH & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SSArrayIdx].PH & 0x00FF;
	
		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].Nitrogen & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SSArrayIdx].Nitrogen & 0x00FF;
	
		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].Phosphorus & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SSArrayIdx].Phosphorus & 0x00FF;

		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].Potassium & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SSArrayIdx].Potassium & 0x00FF;
		
		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].Salinity & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SSArrayIdx].Salinity & 0x00FF;
	
		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].TDS & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SSArrayIdx].TDS & 0x00FF;
	
		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].Fertility & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SSArrayIdx].Fertility & 0x00FF;
		
		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].EC_Coef & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SSArrayIdx].EC_Coef & 0x00FF;
		
		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].Salinity_Coef & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SSArrayIdx].Salinity_Coef & 0x00FF;
	
		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].TDS_Coef & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SSArrayIdx].TDS_Coef & 0x00FF;

		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].Temp_Calib & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SSArrayIdx].Temp_Calib & 0x00FF;
	
		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].Moisture_Calib & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SSArrayIdx].Moisture_Calib & 0x00FF;
		
		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].EC_Calib & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SSArrayIdx].EC_Calib & 0x00FF;
		
		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].PH_Calib & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SSArrayIdx].PH_Calib & 0x00FF;
		
		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].Fert_Coef & 0xFF000000) >> 24 ;
		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].Fert_Coef & 0x00FF0000) >> 16 ;
		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].Fert_Coef & 0x0000FF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SSArrayIdx].Fert_Coef & 0x000000FF;	
	
		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].Fert_Deviation & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SSArrayIdx].Fert_Deviation & 0x00FF;

		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].Nitrogen_Coef & 0xFF000000) >> 24 ;
		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].Nitrogen_Coef & 0x00FF0000) >> 16 ;
		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].Nitrogen_Coef & 0x0000FF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SSArrayIdx].Nitrogen_Coef & 0x000000FF;	
	
		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].Nitrogen_Deviation & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SSArrayIdx].Nitrogen_Deviation & 0x00FF;

		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].Phosphorus_Coef & 0xFF000000) >> 24 ;
		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].Phosphorus_Coef & 0x00FF0000) >> 16 ;
		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].Phosphorus_Coef & 0x0000FF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SSArrayIdx].Phosphorus_Coef & 0x000000FF;	
		
		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].Phosphorus_Deviation & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SSArrayIdx].Phosphorus_Deviation & 0x00FF;
		
		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].Potassium_Coef & 0xFF000000) >> 24 ;
		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].Potassium_Coef & 0x00FF0000) >> 16 ;
		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].Potassium_Coef & 0x0000FF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SSArrayIdx].Potassium_Coef & 0x000000FF;	
		
		HostTxBuffer[PktIdx++] = (SoilSensorData[SSArrayIdx].Potassium_Deviation & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[SSArrayIdx].Potassium_Deviation & 0x00FF;

		CalChecksumH();	
}

/***
 *	@brief		Send Host Air sensor Data
 ***/
void SendHost_AirData(void)
{
    uint8_t PktIdx,AirSnsrArrayIdx;
    
    HostTxBuffer[2] =  METER_RSP_AIR_DATA;	
    HostTxBuffer[3] = fgToHostFlag; 	
    HostTxBuffer[4] = AirSnsrArrayIdx ; 	
    PktIdx = 5;		

		HostTxBuffer[PktIdx++] = AirSensorError.ErrorRate[AirSnsrArrayIdx];			// Communicate rate
	
		HostTxBuffer[PktIdx++] = (AirSensorData[AirSnsrArrayIdx].Co2 & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  AirSensorData[AirSnsrArrayIdx].Co2 & 0x00FF;

		HostTxBuffer[PktIdx++] = (AirSensorData[AirSnsrArrayIdx].Formaldehyde & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  AirSensorData[AirSnsrArrayIdx].Formaldehyde & 0x00FF;	
	
		HostTxBuffer[PktIdx++] = (AirSensorData[AirSnsrArrayIdx].Tvoc & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  AirSensorData[AirSnsrArrayIdx].Tvoc & 0x00FF;

		HostTxBuffer[PktIdx++] = (AirSensorData[AirSnsrArrayIdx].Pm25 & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  AirSensorData[AirSnsrArrayIdx].Pm25 & 0x00FF;	

		HostTxBuffer[PktIdx++] = (AirSensorData[AirSnsrArrayIdx].Pm10 & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  AirSensorData[AirSnsrArrayIdx].Pm10 & 0x00FF;

		HostTxBuffer[PktIdx++] = (AirSensorData[AirSnsrArrayIdx].Temperature & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  AirSensorData[AirSnsrArrayIdx].Temperature & 0x00FF;	
	
		HostTxBuffer[PktIdx++] = (AirSensorData[AirSnsrArrayIdx].Humidity & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  AirSensorData[AirSnsrArrayIdx].Humidity & 0x00FF;
	
		CalChecksumH();	
}

/***
 *	@brief		Send Host Inverter Data
 ***/
void SendHost_InvData(void)
{
    uint8_t PktIdx,InvArrayIdx;
    InvArrayIdx = HostPollingDeviceIdx-1;
	
    HostTxBuffer[2] = METER_RSP_INV_DATA ;	
    HostTxBuffer[3] = fgToHostFlag; 	
    HostTxBuffer[4] = InvArrayIdx ; 	
    PktIdx = 5;
	
		HostTxBuffer[PktIdx++] = InvError.ErrorRate;			// Communicate rate
		//	Controller Data
		HostTxBuffer[PktIdx++] = CtrlData.ConnectFlag;
		HostTxBuffer[PktIdx++] = CtrlData.ChargingFlag;
		HostTxBuffer[PktIdx++] = CtrlData.FaultFlag;
		HostTxBuffer[PktIdx++] = CtrlData.WarnFlag;
		//	Inverter Data
		HostTxBuffer[PktIdx++] = InvData.ChargingFlag;	
		HostTxBuffer[PktIdx++] = InvData.FaultFlag;
		HostTxBuffer[PktIdx++] = InvData.WarnFlag;	
		//	Bat Data
		HostTxBuffer[PktIdx++] = BatData.Full;
		HostTxBuffer[PktIdx++] = BatData.LoadWarnFlag;
		HostTxBuffer[PktIdx++] = BatData.TempWarnFlag;
		HostTxBuffer[PktIdx++] = BatData.LoadTimeoutWarnFlag;	
		HostTxBuffer[PktIdx++] = BatData.LoadOverWarnFlag;
		HostTxBuffer[PktIdx++] = BatData.BatHighVoltWarnFlag;
		HostTxBuffer[PktIdx++] = BatData.BatLowVoltWarnFlag;
		HostTxBuffer[PktIdx++] = BatData.StoreDataErrWarnFlag;
		HostTxBuffer[PktIdx++] = BatData.StoreOpFailWarnFlag;
		
		HostTxBuffer[PktIdx++] = BatData.InvFuncErrWarnFlag;
		HostTxBuffer[PktIdx++] = BatData.PlanShutdownWarnFlag;
		HostTxBuffer[PktIdx++] = BatData.OutputWarnFlag;	
		
		HostTxBuffer[PktIdx++] = BatData.InvErrFaultFlag;
		HostTxBuffer[PktIdx++] = BatData.TempOverFaultFlag;
		HostTxBuffer[PktIdx++] = BatData.TempSensorFaultFlag;
		HostTxBuffer[PktIdx++] = BatData.LoadTimeoutFaultFlag;		
		HostTxBuffer[PktIdx++] = BatData.LoadErrFaultFlag;
		HostTxBuffer[PktIdx++] = BatData.LoadOverFaultFlag;
		HostTxBuffer[PktIdx++] = BatData.BatHighVoltFaultFlag;
		HostTxBuffer[PktIdx++] = BatData.BatLowVoltFaultFlag;	
		HostTxBuffer[PktIdx++] = BatData.PlanShutdownFaultFlag;
		HostTxBuffer[PktIdx++] = BatData.OutputErrFaultFlag;
		HostTxBuffer[PktIdx++] = BatData.ChipStartFailFaultFlag;
		HostTxBuffer[PktIdx++] = BatData.CurrentSensorFaultFlag;
		
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
		HostTxBuffer[2] = CMD_MTR_OTA_UPDATE;	

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
					ReadMeterTime = TokenHost[6] ;
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




