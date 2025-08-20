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
#include "AO_BMSModbusProcess.h"
#include "AO_WaterMeterModbusProcess.h"
#include "AO_InverterModbusProcess.h"

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
void SendHost_SystemInformation(void);
void Host_UserInfoUpdated(void);
void SendHost_ErrorTable(void);
void SendHost_PowerMeter(void);
void SendHost_Ack(void);
void SystemSwitchProcess(void);
void Host_AliveProcess(void);
void SendHost_UserBalance(uint8_t fnUserIndex);
void GetUserProcess(void);
void Host_RoomSettingUpdated(void);
void C2M_FlagProcess(uint8_t u8Flag);
void Host_UserBalanceUpdated(void);
void Host_OpenDoorProcess(void);
void SendHost_PowerMeterData(void);
void Host_UserChangeMode(void);
void SetStartInfor(uint8_t fnMemberIndex);
void SendHost_UserData(void);
void SendHost_PowerData(void);
void Host_ChangeRoomData(void);
 void Host_ChangeUserData(void);
 
void Host_GetDeviceDataProcess(void);


uint8_t _SendStringToHOST(uint8_t *Str, uint8_t len);

extern uint8_t fgReaderSync;
extern uint8_t ModeSyncError;
uint8_t LastDataUpdated;

/*MeterOta*/
void Host_OTAMenterProcess(void);
void SendHost_MenterFWinfo(void);
void SendHost_MenterFWActivatedInfo(void);

/***	Bms, WM, Inv SendHost module	***/
void SendHost_BmsData(void);
void SendHost_WMData(void);
void SendHost_InvData(void);

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
                    case METER_CMD_GET_POWER_DATA :
                        Host_GetDeviceDataProcess();                        
                        CmdType = METER_RSP_POWER_DATA ;
                        ClearRespDelayTimer() ;	
                        break;
                    case METER_CMD_GET_BMS_DATA :
                        Host_GetDeviceDataProcess();                        
                        CmdType = METER_RSP_BMS_DATA ;
                        ClearRespDelayTimer() ;	
                        break;										
                    case METER_CMD_GET_WM_DATA :
                        Host_GetDeviceDataProcess();                        
                        CmdType = METER_RSP_WM_DATA ;
                        ClearRespDelayTimer() ;	
                        break;	
                    case METER_CMD_GET_INV_DATA :
                        Host_GetDeviceDataProcess();                        
                        CmdType = METER_RSP_INV_DATA ;
                        ClearRespDelayTimer() ;	
                        break;

										 	// MTR	ota commamd
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
                        SendHost_PowerData();
                        break;
										
										case METER_RSP_BMS_DATA:
												SendHost_BmsData();
												break;
										
										case METER_RSP_WM_DATA:
												SendHost_WMData();
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


void Host_AliveProcess(void)
{
    uint8_t i;
    uint8_t u8BuferIndex;
    uint8_t HostData_P,HostData_N;
    //uint8_t CmdOpenDoor1,CmdOpenDoor2;
    
    for (i=0;i<7;i++)
    {
        iSystemTime[i] = TokenHost[INX_TIME_START_Y+i];
    }
    
    HostData_P = TokenHost[5];
    HostData_N =  TokenHost[6];
    if ( HostData_P == (255-HostData_N) )
    {
        MaxPowerMeter = HostData_P;
    }
    HostData_P = TokenHost[7];
    HostData_N =  TokenHost[8];
    if ( HostData_P == (255-HostData_N) )
    {
        if ( HostData_P == 0x30 )
        {
            u8BuferIndex = 9 ;
            for (i=0;i<MAX_POWER_METER;i++)
            {
                RoomMode[i] = TokenHost[u8BuferIndex++];
            }            
        }
    }

    if ( (iSystemTime[4] == 0) && (iSystemTime[5] == 0))        
    {
        for (i=0;i<MAX_POWER_METER;i++)
        {
            if ( PowerMeterTxCounter[i] > 10 )
            {
                PowerMeterTxCounter[i]=1;
                PowerMeterRxCounter[i]=1;
            }
        }
    }
    
}

void Host_GetDeviceDataProcess(void)
{
    uint8_t i;
	  //u8BuferIndex;
    //uint8_t HostData_P,HostData_N;
    //uint8_t CmdOpenDoor1,CmdOpenDoor2;
    
    for (i=0;i<7;i++)
    {
        iSystemTime[i] = TokenHost[INX_TIME_START_Y+i];
    }
    HostPollingDeviceIdx = TokenHost[3];
/*    
    HostData_P = TokenHost[3];
    HostData_N =  TokenHost[4];
    if ( HostData_P == (255-HostData_N) )
    {
        MaxPowerMeter = HostData_P;
    }
    HostData_P = TokenHost[5];
    HostData_N =  TokenHost[6];
    if ( HostData_P == (255-HostData_N) )
    {
        for (i=0;i<MAX_POWER_METER;i++)
        MeterErrorRate[i] = 100;
        MeterErrorRate_Tx[i] = 1;
        MeterErrorRate_Rx[i] = 1;
    }
*/    
    
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
	
    for (i=0;i<7;i++)
    {
        iSystemTime[i] = TokenHost[INX_TIME_START_Y+i];
    }
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


void SendHost_SystemInformation(void)
{
    uint8_t fnPacketIndex,u8MeterIDIndex;
    
    u8MeterIDIndex = MyMeterBoardID-1;
    HostTxBuffer[2] = METER_RSP_SYS_INFO ;	
    HostTxBuffer[3] = fgToHostFlag; 	
    HostTxBuffer[4] = fgToHostRSPFlag; 

    fnPacketIndex = 5 ; 
    HostTxBuffer[fnPacketIndex++] = (PowerMeterError & 0xFF000000) >> 24 ;
    HostTxBuffer[fnPacketIndex++] = (PowerMeterError & 0x00FF0000) >> 16 ;
    HostTxBuffer[fnPacketIndex++] = (PowerMeterError & 0x0000FF00) >> 8 ;
    HostTxBuffer[fnPacketIndex++] = PowerMeterError & 0x000000FF ;
	
    HostTxBuffer[fnPacketIndex++] = (PowerMeterTxCounter[u8MeterIDIndex] & 0xFF000000) >> 24 ;
    HostTxBuffer[fnPacketIndex++] = (PowerMeterTxCounter[u8MeterIDIndex] & 0x00FF0000) >> 16 ;
    HostTxBuffer[fnPacketIndex++] = (PowerMeterTxCounter[u8MeterIDIndex] & 0x0000FF00) >> 8 ;
    HostTxBuffer[fnPacketIndex++] = PowerMeterTxCounter[u8MeterIDIndex] & 0x000000FF ;    
	
    HostTxBuffer[fnPacketIndex++] = (PowerMeterRxCounter[u8MeterIDIndex] & 0xFF000000) >> 24 ;
    HostTxBuffer[fnPacketIndex++] = (PowerMeterRxCounter[u8MeterIDIndex] & 0x00FF0000) >> 16 ;
    HostTxBuffer[fnPacketIndex++] = (PowerMeterRxCounter[u8MeterIDIndex] & 0x0000FF00) >> 8 ;
    HostTxBuffer[fnPacketIndex++] = PowerMeterRxCounter[u8MeterIDIndex] & 0x000000FF ;
    
    CalChecksumH();	
    
}

void SendHost_PowerData(void)
{
    uint8_t fnPacketIndex,u8PowerMeterID;
    //uint8_t *tmpAddr;
    u8PowerMeterID = HostPollingDeviceIdx-1;
    HostTxBuffer[2] = METER_RSP_POWER_DATA ;	
    HostTxBuffer[3] = fgToHostFlag; 	
    HostTxBuffer[4] = u8PowerMeterID ; 	
    fnPacketIndex = 5;
    HostTxBuffer[fnPacketIndex++] = MeterData[u8PowerMeterID].PayMode ;
    HostTxBuffer[fnPacketIndex++] = MeterErrorRate[u8PowerMeterID];    
    HostTxBuffer[fnPacketIndex++] = MeterData[u8PowerMeterID].RelayStatus;
    HostTxBuffer[fnPacketIndex++] = MeterData[u8PowerMeterID].UserStatus;
    // Total 
    HostTxBuffer[fnPacketIndex++] = (MeterData[u8PowerMeterID].MeterTotalWatt & 0xFF000000) >> 24 ;
    HostTxBuffer[fnPacketIndex++] = (MeterData[u8PowerMeterID].MeterTotalWatt & 0x00FF0000) >> 16 ;
    HostTxBuffer[fnPacketIndex++] = (MeterData[u8PowerMeterID].MeterTotalWatt & 0x0000FF00) >> 8 ;
    HostTxBuffer[fnPacketIndex++] = MeterData[u8PowerMeterID].MeterTotalWatt & 0x000000FF ;
    // 49 Power Meter : Voltage	
    HostTxBuffer[fnPacketIndex++] = (MeterData[u8PowerMeterID].MeterValtage & 0xFF000000) >> 24 ;
    HostTxBuffer[fnPacketIndex++] = (MeterData[u8PowerMeterID].MeterValtage & 0x00FF0000) >> 16 ;
    HostTxBuffer[fnPacketIndex++] = (MeterData[u8PowerMeterID].MeterValtage & 0x0000FF00) >> 8 ;
    HostTxBuffer[fnPacketIndex++] = MeterData[u8PowerMeterID].MeterValtage & 0x000000FF ;
    // 53 Power Meter : Current	
    HostTxBuffer[fnPacketIndex++] = (MeterData[u8PowerMeterID].MeterCurrent & 0xFF000000) >> 24 ;
    HostTxBuffer[fnPacketIndex++] = (MeterData[u8PowerMeterID].MeterCurrent & 0x00FF0000) >> 16 ;
    HostTxBuffer[fnPacketIndex++] = (MeterData[u8PowerMeterID].MeterCurrent & 0x0000FF00) >> 8 ;
    HostTxBuffer[fnPacketIndex++] = MeterData[u8PowerMeterID].MeterCurrent & 0x000000FF ;
    // 57 Power Meter : Freq. 		
    HostTxBuffer[fnPacketIndex++] = (MeterData[u8PowerMeterID].MeterFreq & 0x0000FF00) >> 8 ;
    HostTxBuffer[fnPacketIndex++] = MeterData[u8PowerMeterID].MeterFreq & 0x000000FF ;
    // 59 Power Meter : Power Factor		
    HostTxBuffer[fnPacketIndex++] = (MeterData[u8PowerMeterID].MeterPowerFactor & 0x0000FF00) >> 8 ;
    HostTxBuffer[fnPacketIndex++] = MeterData[u8PowerMeterID].MeterPowerFactor & 0x000000FF ;
    // 61 Power Meter : Active Power	
    HostTxBuffer[fnPacketIndex++] = (MeterData[u8PowerMeterID].MeterActPower & 0xFF000000) >> 24 ;
    HostTxBuffer[fnPacketIndex++] = (MeterData[u8PowerMeterID].MeterActPower & 0x00FF0000) >> 16 ;
    HostTxBuffer[fnPacketIndex++] = (MeterData[u8PowerMeterID].MeterActPower & 0x0000FF00) >> 8 ;
    HostTxBuffer[fnPacketIndex++] = MeterData[u8PowerMeterID].MeterActPower & 0x000000FF ;
    // 65 Power Meter : VA	
    HostTxBuffer[fnPacketIndex++] = (MeterData[u8PowerMeterID].MeterVA & 0xFF000000) >> 24 ;
    HostTxBuffer[fnPacketIndex++] = (MeterData[u8PowerMeterID].MeterVA & 0x00FF0000) >> 16 ;
    HostTxBuffer[fnPacketIndex++] = (MeterData[u8PowerMeterID].MeterVA & 0x0000FF00) >> 8 ;
    HostTxBuffer[fnPacketIndex++] = MeterData[u8PowerMeterID].MeterVA & 0x000000FF ;
    // 69 Power Meter Balance	
    HostTxBuffer[fnPacketIndex++] = (MeterData[u8PowerMeterID].MeterBalance& 0xFF000000) >> 24 ;
    HostTxBuffer[fnPacketIndex++] = (MeterData[u8PowerMeterID].MeterBalance & 0x00FF0000) >> 16 ;
    HostTxBuffer[fnPacketIndex++] = (MeterData[u8PowerMeterID].MeterBalance & 0x0000FF00) >> 8 ;
    HostTxBuffer[fnPacketIndex++] = MeterData[u8PowerMeterID].MeterBalance & 0x000000FF ;
    // 73 Power Meter : User UID	
    HostTxBuffer[fnPacketIndex++] = (MeterData[u8PowerMeterID].UserUID & 0xFF000000) >> 24 ;
    HostTxBuffer[fnPacketIndex++] = (MeterData[u8PowerMeterID].UserUID & 0x00FF0000) >> 16 ;
    HostTxBuffer[fnPacketIndex++] = (MeterData[u8PowerMeterID].UserUID & 0x0000FF00) >> 8 ;
    HostTxBuffer[fnPacketIndex++] = MeterData[u8PowerMeterID].UserUID & 0x000000FF ;

    CalChecksumH();			
	
}

/***
 *	@brief		Send Host Bms Data
 *
 ***/
void SendHost_BmsData(void)
{
    uint8_t fnPacketIndex,u8BmsID;
    u8BmsID = HostPollingDeviceIdx-1;
    HostTxBuffer[2] = METER_RSP_BMS_DATA ;	
    HostTxBuffer[3] = fgToHostFlag; 	
    HostTxBuffer[4] = u8BmsID ; 	
    fnPacketIndex = 5;
	
		HostTxBuffer[fnPacketIndex++] = BmsError.ErrorRate;			// Communicate rate
		HostTxBuffer[fnPacketIndex++] = BmsData[u8BmsID].BalanceStatus;	// Battery mode
		HostTxBuffer[fnPacketIndex++] = BmsData[u8BmsID].StateOfCharge;
		HostTxBuffer[fnPacketIndex++] = BmsData[u8BmsID].StateOfHealth;	//	SOH
		//idx:9	Cell ststus 
		HostTxBuffer[fnPacketIndex++] = (BmsData[u8BmsID].CellStatus & 0xFF000000) >> 24 ;
		HostTxBuffer[fnPacketIndex++] = (BmsData[u8BmsID].CellStatus & 0xFF000000) >> 16 ;
		HostTxBuffer[fnPacketIndex++] = (BmsData[u8BmsID].CellStatus & 0xFF000000) >> 8 ;
		HostTxBuffer[fnPacketIndex++] =  BmsData[u8BmsID].CellStatus & 0xFF000000;	
		//idx:13	Cell volt
		for (uint8_t i = 0; i < 16; i++) {
				HostTxBuffer[fnPacketIndex++] = (BmsData[u8BmsID].CellVolt[i] >> 8);
				HostTxBuffer[fnPacketIndex++] = BmsData[u8BmsID].CellVolt[i];
		}
		//idx:45	Battery watt 
		HostTxBuffer[fnPacketIndex++] = (BmsData[u8BmsID].BatWatt & 0xFF000000) >> 24 ;
		HostTxBuffer[fnPacketIndex++] = (BmsData[u8BmsID].BatWatt & 0xFF000000) >> 16 ;
		HostTxBuffer[fnPacketIndex++] = (BmsData[u8BmsID].BatWatt & 0xFF000000) >> 8 ;
		HostTxBuffer[fnPacketIndex++] =  BmsData[u8BmsID].BatWatt & 0xFF000000;	
		//idx:49	Battery voltage 
		HostTxBuffer[fnPacketIndex++] = (BmsData[u8BmsID].BatVolt & 0xFF000000) >> 24 ;
		HostTxBuffer[fnPacketIndex++] = (BmsData[u8BmsID].BatVolt & 0xFF000000) >> 16 ;
		HostTxBuffer[fnPacketIndex++] = (BmsData[u8BmsID].BatVolt & 0xFF000000) >> 8 ;
		HostTxBuffer[fnPacketIndex++] =  BmsData[u8BmsID].BatVolt & 0xFF000000;	
		//idx:53	Battery current 
		HostTxBuffer[fnPacketIndex++] = (BmsData[u8BmsID].BatCurrent & 0xFF000000) >> 24 ;
		HostTxBuffer[fnPacketIndex++] = (BmsData[u8BmsID].BatCurrent & 0xFF000000) >> 16 ;
		HostTxBuffer[fnPacketIndex++] = (BmsData[u8BmsID].BatCurrent & 0xFF000000) >> 8 ;
		HostTxBuffer[fnPacketIndex++] =  BmsData[u8BmsID].BatCurrent & 0xFF000000;			
		//idx:63	Battery temperature [1-5]	
		for (uint8_t i = 0; i<5; i++)
		{		
				HostTxBuffer[fnPacketIndex++] = (BmsData[u8BmsID].BatteryTemp[i] >> 8);
				HostTxBuffer[fnPacketIndex++] =  BmsData[u8BmsID].BatteryTemp[i];
		}
		//idx:65 	Mos temperature 
		HostTxBuffer[fnPacketIndex++] = (BmsData[u8BmsID].MosTemp >> 8);
		HostTxBuffer[fnPacketIndex++] =  BmsData[u8BmsID].MosTemp;
		
		CalChecksumH();			
}


/***
 *	@brief		Send Host water meter Data
 *	@Send			Communication rate Valve status, Totla used water volume
 *
 ***/
void SendHost_WMData(void)
{
    uint8_t fnPacketIndex,u8WMID;
    u8WMID = HostPollingDeviceIdx-1;
    HostTxBuffer[2] = METER_RSP_WM_DATA ;	
    HostTxBuffer[3] = fgToHostFlag; 	
    HostTxBuffer[4] = u8WMID ; 	
    fnPacketIndex = 5;
	
		HostTxBuffer[fnPacketIndex++] = WMError.ErrorRate;			// Communicate rate
		HostTxBuffer[fnPacketIndex++] = WMData[u8WMID].ValveState;			// 0xff : closed, 0x00 : opened
	
		HostTxBuffer[fnPacketIndex++] = (WMData[u8WMID].TotalVolume & 0xFF000000) >> 24 ;
		HostTxBuffer[fnPacketIndex++] = (WMData[u8WMID].TotalVolume & 0xFF000000) >> 16 ;
		HostTxBuffer[fnPacketIndex++] = (WMData[u8WMID].TotalVolume & 0xFF000000) >> 8 ;
		HostTxBuffer[fnPacketIndex++] =  WMData[u8WMID].TotalVolume & 0xFF000000;		
    
		CalChecksumH();			
}

/***
 *	@brief		Send Host Inverter Data
 ***/
void SendHost_InvData(void)
{
    uint8_t fnPacketIndex,u8WMID;
    u8WMID = HostPollingDeviceIdx-1;
    HostTxBuffer[2] = METER_RSP_INV_DATA ;	
    HostTxBuffer[3] = fgToHostFlag; 	
    HostTxBuffer[4] = u8WMID ; 	
    fnPacketIndex = 5;
	
		HostTxBuffer[fnPacketIndex++] = InvError.ErrorRate;			// Communicate rate
		//	Controller Data
		HostTxBuffer[fnPacketIndex++] = CtrlData.ConnectFlag;
		HostTxBuffer[fnPacketIndex++] = CtrlData.ChargingFlag;
		HostTxBuffer[fnPacketIndex++] = CtrlData.FaultFlag;
		HostTxBuffer[fnPacketIndex++] = CtrlData.WarnFlag;
		//	Inverter Data
		HostTxBuffer[fnPacketIndex++] = InvData.ChargingFlag;	
		HostTxBuffer[fnPacketIndex++] = InvData.FaultFlag;
		HostTxBuffer[fnPacketIndex++] = InvData.WarnFlag;	
		//	Bat Data
		HostTxBuffer[fnPacketIndex++] = BatData.Full;
		HostTxBuffer[fnPacketIndex++] = BatData.LoadWarnFlag;
		HostTxBuffer[fnPacketIndex++] = BatData.TempWarnFlag;
		HostTxBuffer[fnPacketIndex++] = BatData.LoadTimeoutWarnFlag;	
		HostTxBuffer[fnPacketIndex++] = BatData.LoadOverWarnFlag;
		HostTxBuffer[fnPacketIndex++] = BatData.BatHighVoltWarnFlag;
		HostTxBuffer[fnPacketIndex++] = BatData.BatLowVoltWarnFlag;
		HostTxBuffer[fnPacketIndex++] = BatData.StoreDataErrWarnFlag;
		HostTxBuffer[fnPacketIndex++] = BatData.StoreOpFailWarnFlag;
		
		HostTxBuffer[fnPacketIndex++] = BatData.InvFuncErrWarnFlag;
		HostTxBuffer[fnPacketIndex++] = BatData.PlanShutdownWarnFlag;
		HostTxBuffer[fnPacketIndex++] = BatData.OutputWarnFlag;	
		
		HostTxBuffer[fnPacketIndex++] = BatData.InvErrFaultFlag;
		HostTxBuffer[fnPacketIndex++] = BatData.TempOverFaultFlag;
		HostTxBuffer[fnPacketIndex++] = BatData.TempSensorFaultFlag;
		HostTxBuffer[fnPacketIndex++] = BatData.LoadTimeoutFaultFlag;		
		HostTxBuffer[fnPacketIndex++] = BatData.LoadErrFaultFlag;
		HostTxBuffer[fnPacketIndex++] = BatData.LoadOverFaultFlag;
		HostTxBuffer[fnPacketIndex++] = BatData.BatHighVoltFaultFlag;
		HostTxBuffer[fnPacketIndex++] = BatData.BatLowVoltFaultFlag;	
		HostTxBuffer[fnPacketIndex++] = BatData.PlanShutdownFaultFlag;
		HostTxBuffer[fnPacketIndex++] = BatData.OutputErrFaultFlag;
		HostTxBuffer[fnPacketIndex++] = BatData.ChipStartFailFaultFlag;
		HostTxBuffer[fnPacketIndex++] = BatData.CurrentSensorFaultFlag;
		
		CalChecksumH();			
}

/************************************
0: 0x55
1: MyMeterBoardID
2: Command
3: fgToHostFlag
4:  Result
5 ~ 9 : Meter status ( 4 Bytes )
10 ~ 14 : Meter status ( 4 Bytes )
15 ~ 19 : Meter status ( 4 Bytes )
20 ~ 24 : Meter status ( 4 Bytes )
24 ~ 29 : Meter status ( 4 Bytes )
Members Data's checksum (MEMBER MAX )
49: Checksum
50: 0x0A (\n)
*/
void SendHost_Ack(void)
{
    uint8_t fnPacketIndex;
    //uint8_t *tmpAddr;

    HostTxBuffer[2] = METER_RSP_ACK ;	
    HostTxBuffer[3] = fgToHostFlag; 
    HostTxBuffer[4] = fgToHostRSPFlag; 
    fnPacketIndex = 5 ; 
	
		//	Got Devices status
		//	PowerMeter NG Status.
    HostTxBuffer[fnPacketIndex++] = (PowerMeterError & 0xFF000000) >> 24 ;
    HostTxBuffer[fnPacketIndex++] = (PowerMeterError & 0x00FF0000) >> 16 ;
    HostTxBuffer[fnPacketIndex++] = (PowerMeterError & 0x0000FF00) >> 8 ;
    HostTxBuffer[fnPacketIndex++] =  PowerMeterError & 0x000000FF ;
		//	BMS NG Status.
		HostTxBuffer[fnPacketIndex++] = (BmsError.BmsDeviceNG & 0xFF000000) >> 24 ;
    HostTxBuffer[fnPacketIndex++] = (BmsError.BmsDeviceNG & 0x00FF0000) >> 16 ;
    HostTxBuffer[fnPacketIndex++] = (BmsError.BmsDeviceNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[fnPacketIndex++] =  BmsError.BmsDeviceNG & 0x000000FF ;
		//	WM NG Status.
		HostTxBuffer[fnPacketIndex++] = (WMError.WMDeviceNG & 0xFF000000) >> 24 ;
    HostTxBuffer[fnPacketIndex++] = (WMError.WMDeviceNG & 0x00FF0000) >> 16 ;
    HostTxBuffer[fnPacketIndex++] = (WMError.WMDeviceNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[fnPacketIndex++] =  WMError.WMDeviceNG & 0x000000FF ;
		//	INV NG Status.
		HostTxBuffer[fnPacketIndex++] = (InvError.InvDeviceNG & 0xFF000000) >> 24 ;
    HostTxBuffer[fnPacketIndex++] = (InvError.InvDeviceNG & 0x00FF0000) >> 16 ;
    HostTxBuffer[fnPacketIndex++] = (InvError.InvDeviceNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[fnPacketIndex++] =  InvError.InvDeviceNG & 0x000000FF ;
		
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




