/******************************************************************************
 * @file     main.c
 * @version  V1.00
 * $Revision: 2 $
 * $Date: 16/10/25 4:34p $
 * @brief    Software Development Template.
 * @note		 Add test functions in main... ( Device address affirm & revice )
 * Copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include "NUC1261.h"
#include "AO2022_MeterModule_1261.h"
#include "AO_MyDef.h"

#include "AO_EE24C.h"
#include "AO_ReaderProcess.h"
#include "AO_HostProcess.h"
#include "AO_ExternFunc.h"
#include "AO_MeterProcess.h"
#include "AO_ModBusProcess.h"
#include "AO_BMSModbusProcess.h"
#include "AO_InverterModbusProcess.h"
#include "AO_WaterMeterModbusProcess.h"
#include "AO_Pyranometer.h"
#include "AO_SoilSensor.h"
#include "AO_AirSensor.h"

#define PLLCTL_SETTING  CLK_PLLCTL_72MHz_HIRC
#define PLL_CLOCK       72000000


/*

 SERCOM0 / HOST
 SERCOM1 / READER
 SERCOM2 / METER
 PA18 ~ PA25 / input: Device ID Switch
 PA10 / Output: RS485 DIR for Reader (0:Input)
 PA11 / Output: RS485 DIR for Host (0:Input)
 PA00 / Bi- : I2C-SDA for E2PROM
 PA01 / Output: I2C-SCK for E2PROM
 PA02 / Output: E2PROM WP (0:Write protect)
 PA03 / Output: Status LED (0:LED ON)
 PB10 / Input: Meter Digital Input (Low Counter)
 PB11 / Output: SSR Control (1: SSR ON)

*/



#define DIGIT_METER	

#define POWER_100W_COUNTER		160

#define SYSTEM_ERROR_TIMEOUT		500
#define MAX_BUTTON_TIMES			5
#define EEPROM_ADDR				0x50

#define EE_ADDR_ROOM_NAME			0x0010
#define EE_ADDR_ROOM_USER			0x0014
#define EE_ADDR_ROOM_MODE			0x0015
#define EE_ADDR_ROOM_RATE			0x0016
#define EE_ADDR_ROOM_NO			0x0017
#define EE_ADDR_POWER100W		0x0018
#define EE_ADDR_STUDENT_ID		0x0020
#define EE_ADDR_USER_UID			0x0050
#define EE_ADDR_USER_VALUE		0x00A0
#define USER_CHECK_CODE_ADDR		0x00B0
#define EE_ADDR_METER_VALUE		0x0080

/*	Idx of devices 	*/
uint8_t WtrMtrIDArray[WtrMtrMax] = {0x01};
uint8_t PwrMtrIDArray[PwrMtrMax] = {0x02};
uint8_t BmsIDArray[1] = {0x03};

uint8_t SoilSensorIDArray[1] = {0x04};		/***	Already setup	***/
uint8_t AirSensorIDArray[1] = {0x05};			
uint8_t PyrMtrIDArray[1] = {0x06};				/***	Already setup	***/

uint8_t MaxPowerMeter;
STR_METER_D MeterData[PwrMtrMax];
uint8_t RoomMode[PwrMtrMax];

uint16_t MeterErrorRate_Tx[PwrMtrMax];
uint16_t MeterErrorRate_Rx[PwrMtrMax];

// Unit Cost = NTD. 0.5 ( Set 10 => NTD. 1 )
char VersionString[] = VERSION ;
uint8_t MyMeterBoardID;
uint8_t iStatus;
// System 
uint8_t iSystemTime[8]={0,0,0,0,0,0,0,0};		// Year,Month,Day,Hour,Min,Sec,Week
uint8_t UID[4];
uint8_t ReaderTxBuffer[READER_TOKEN_LENGTH];
uint8_t HostTxBuffer[HOST_TOKEN_LENGTH];
uint8_t SystemPowerStatus;
uint8_t TokenHost[HOST_TOKEN_LENGTH];
uint8_t TokenReader[READER_TOKEN_LENGTH];
uint8_t MeterTxBuffer[METER_TOKEN_LENGTH];
uint8_t TokenMeter[METER_TOKEN_LENGTH];

uint8_t HOSTTxQ[MAX_HOST_TXQ_LENGTH];
uint8_t HOSTRxQ[MAX_HOST_RXQ_LENGTH];
uint8_t READERTxQ[MAX_READER_TXQ_LENGTH];
uint8_t READERRxQ[MAX_READER_RXQ_LENGTH];
uint8_t METERRxQ[MAX_METER_RXQ_LENGTH];
uint8_t METERTxQ[MAX_METER_TXQ_LENGTH];

uint8_t HOSTRxQ_wp,HOSTRxQ_rp,HOSTRxQ_cnt;
uint8_t READERRxQ_wp,READERRxQ_rp,READERRxQ_cnt;
uint8_t HOSTTxQ_wp,HOSTTxQ_rp,HOSTTxQ_cnt;
uint8_t READERTxQ_wp,READERTxQ_rp,READERTxQ_cnt;
uint8_t METERTxQ_wp,METERTxQ_rp,METERTxQ_cnt;
uint8_t METERRxQ_wp,METERRxQ_rp,METERRxQ_cnt;
uint8_t METER_RXQLen;


uint8_t NewCardUID[4];
uint8_t NumInChargeM,NumInChargeR;
uint8_t AckResult,MemberBase;
uint8_t PacketIndex;
uint8_t LastReaderRecord_RP;

uint8_t ToReader_MemberIndex;
uint8_t HostGetRCDType,HostGetRCDIndex;
uint8_t NewRecordCounter, Record_WP, Record_RP;
uint8_t TagChargeRCD_NewCnt, TagChargeRCD_WP, TagChargeRCD_RP;
uint8_t PowerRCD_NewCnt, PowerRCD_WP, PowerRCD_RP;

uint8_t fgToReaderFlag;
uint8_t fgToReaderRSPFlag;
uint8_t fgToHostFlag;
uint8_t fgToHostRSPFlag;
uint8_t fgReaderFunction;
uint8_t fgFromHostFlag;
uint8_t fgFromReaderFlag;

uint8_t DoorRecordIndex;
uint8_t MemberIndex;
uint32_t MeterPower;
float ElectricityFee[MEMBER_MAX_NO];
uint8_t ReaderHealth ;

uint8_t MeterDI_Counter,MeterDI_Status;
uint8_t ButtonDI_Counter,ButtonStatus;

uint8_t ReaderWaitTick,PollRetryReader;
uint8_t PackageIndex1,GotReaderRSP,ReaderDeviceError;
	
volatile uint32_t   u32TimeTick2;
volatile uint8_t Tick_10mSec,Time_Sec;
uint16_t     SystemTick;
//struct adc_module adc_instance;



uint8_t 	READERTokenReady,HOSTTokenReady,MeterTokenReady;

uint16_t NowPortStatus,LastPortStatus,HostPortStatus;

uint8_t TickPollingInterval,MeterPollingState;

uint32_t 	TotalWattValue_Now;
uint32_t NowMeterPower;
uint16_t  TickReadDeviceTime,ReadDeviceCmdTime,DelayTick, TickUartChangeBaudrate;
uint8_t PwrMtrNewAddr, PwrMtrNewBaudRate, PowerMeterMode, PowerMeterDO_OnOff, PowerMeterDOLock;

uint8_t NewUser,NowUser,LastUser;
uint16_t	ConsumptionCounter;
uint8_t	Tick_20mSec,TickADC;
uint8_t ReaderPollingState;
uint8_t bSaveSettingToEE;
uint8_t TickWaitAck;
uint8_t TickReader;
uint8_t iTickDelaySendHostCMD,bDelaySendHostCMD,bValueUpdated;
uint8_t UserValue[6][5];

_Bool bRegister,TokenMeterReady;
_Bool fgDIR485_Reader,fgDIR485_HOST_In,fgDIR485_METER_In;
uint8_t 	DelaySwitchDIR1,Tick10mSec_DIR485_Reader,DelaySwitchDIR3;
uint8_t RecordWp,RecordRp,RecordCounter,TickRecord,Tick_PowerCounter;
uint8_t TickHostUart, TickMeterUart;
_Bool 	bRecoverSystem;
uint8_t TickHost;

/********	Power Meter Types	********
  * 0x01  DAE DEM530
  * 0x02  Tatung E21/E31
	* 0x03 	TUTANG_E21nE31			
	* 0x04	DEM_510c	
  ********************/
uint8_t MeterType;
uint8_t DelayTime4NextCmd;
_Bool fgSwPower,PowerOnReadMeter,PowerOnTimeSync;

#ifdef METER_TEST 
uint32_t MeterValueTest;
#endif 

//struct tc_module tc_instance1;

void GPIO_Init(void);
void DefaultValue(void);
void CalErrorRate(void);



void SystemSwitchProcess(void);
void ReadMyMeterBoardID(void);
void ResetHostUART(void);
void ResetMeterUART(void);
void RecoverSystemMoniter(void);


uint8_t CheckUserValue(uint8_t *fnMemberUID);
void ChangeRS485Direct(void);

void RoomPowerManager_Digital(void);
void MeterValueCounter(void);
void ResetReaderUART(void);
void MeterValueReset(void);

void UserBalanceCal(void);
void CheckReaderData(void);
void PowerLinkProcess(void);
void CheckModeNRelay(void);

void WriteRoomModeToEE(void);
void SaveMeterPower2EE( void );
void ReadMeterPowerFromEE( void );
void ReadPowerFromEE(void);

void ReadUserInfoFromEE(void);
void WriteUserInfoToEE(void);

uint16_t ReadUserValueFromEE_One(uint8_t fMemberIndex);
void CheckStartRecord(void);

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
volatile uint8_t g_u8DeviceAddr = 0x50;;
volatile uint8_t g_au8TxData[3];
volatile uint8_t g_u8RxData;
volatile uint8_t g_u8DataLen;
volatile uint8_t g_u8EndFlag = 0;

volatile uint32_t g_u32WDTINTCounts;
volatile uint8_t g_u8IsWDTWakeupINT;
volatile uint32_t g_u32RTCTickINT;
S_RTC_TIME_DATA_T sWriteRTC, sReadRTC;	

uint8_t  ErrorCode;

_Bool fgDirReaderRS485Out2In,fgDirMeterRS485Out2In,fgDisableRS485Tx;
uint16_t TickDirReaderRS485Delay,TickDirMeterRS485Delay,TickDisableRS485TxDelay;
uint16_t tick_MeterSetting;
uint8_t Tick1S_ResetReaderPower;
_Bool fgFirstStart;
uint8_t Tick_LED_G,Tick_LED_G1,Tick_LED_R,Tick_LED_R1;

uint8_t StepFineTuneUR2,UR2_FregOK;
uint16_t Tick10mS_FTUR2,FreqMixValue;
uint8_t RelayOffCnt;
uint8_t RoomModeCheckCounter[PwrMtrMax];

//	Devices 
/*
#define	DeviceMax								2
#define	PwrMtrDevice_START_IDX	0
#define	WtrMtrDevice_START_IDX	1
*/
//uint8_t Device[DeviceMax];

volatile uint8_t PwrMtrCmdList[PwrMtrMax], BmsCmdList[BmsMax];
volatile uint8_t WtrMtrCmdList[WtrMtrMax], PyrMtrCmdList[PyrMtrMax];
volatile uint8_t SoilSensorCmdList[SoilSensorMax], AirSensorCmdList[AirSensorMax];

//	SystemPolling
uint8_t volatile SystemPollingState;
uint8_t	SystemPollingStateIndex;

uint32_t PowerMeterRxCounter[PwrMtrMax];
uint32_t PowerMeterTxCounter[PwrMtrMax];
TotErrorRate_t TotErrorRate;

void DefaultValue(void);


void SystemSwitchProcess(void);
void ReadMyMeterBoardID(void);
void ResetHostUART(void);
void ResetMeterUART(void);
void RecoverSystemMoniter(void);


uint8_t CheckUserValue(uint8_t *fnMemberUID);
void ChangeRS485Direct(void);

void RoomPowerManager_Digital(void);
void MeterValueCounter(void);
void ResetReaderUART(void);
void MeterValueReset(void);

void UserBalanceCal(void);
void CheckReaderData(void);
void PowerLinkProcess(void);
void CheckModeNRelay(void);

void ReadUserInfoFromEE(void);
void WriteUserInfoToEE(void);

void CheckStartRecord(void);
void CheckReaderReset(void);

void ChangeDirMeterRS485(void);
void ChangeDirReaderRS485(void);
void ChangeHostRS485Tx(void);
void EnableHostUartTx(void);
void DisableHostUartTx(void);
void UART0_Init(void);
void UART1_Init(void);
void UART2_Init(uint32_t u32baudrate);
void WDT_Init(void);
void ROOM_POWER_On(void);
void ROOM_POWER_Off(void);

void SystemPolling(void);
void ModbusDataProcess(void);
void InitializePollingIDs(void);
void Delay_10ms(uint8_t ms);

/**
 * @brief       IRQ Handler for WDT Interrupt
 *
 * @param       None
 *
 * @return      None
 *
 * @details     The WDT_IRQHandler is default IRQ of WDT, declared in startup_NUC126.s.
 */
void WDT_IRQHandler(void)
{
		SYS_UnlockReg();
		FMC_Open();
	
		WDT_RESET_COUNTER();
		
		uint32_t other_addr = (g_fw_ctx.FW_meta_Addr == METADATA_FW1_BASE) ? METADATA_FW2_BASE : METADATA_FW1_BASE;
		
		if (WDT_GET_TIMEOUT_INT_FLAG())
    {
			
        WDT_CLEAR_TIMEOUT_INT_FLAG();
        g_u32WDTINTCounts++;			
			
				meta.WDTRst_counter = g_u32WDTINTCounts;
				WriteMetadata(&meta, g_fw_ctx.FW_meta_Addr);
			
				if(g_u32WDTINTCounts > MAX_WDT_TRIES)
				{
						meta.flags &= ~(FW_FLAG_ACTIVE | FW_FLAG_VALID);
						meta.flags |= FW_FLAG_INVALID;
						other.flags |= FW_FLAG_ACTIVE;
						WriteMetadata(&other, other_addr);
						WriteMetadata(&meta, g_fw_ctx.FW_meta_Addr);
						WDT_Open(WDT_TIMEOUT_2POW4, WDT_RESET_DELAY_3CLK, TRUE, FALSE);
						while (1);
				}
    }
		SYS_LockReg();
}

/*---------------------------------------------------------------------------------------------------------*/
/*  SysTick Handler                                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
void SysTick_Handler(void)
{  
    uint8_t i;
	
    DelayTick++;
    Tick_10mSec++;
    iTickDelaySendHostCMD++;
    
    ReaderWaitTick++;
    DelayTime4NextCmd++;
    u32TimeTick2++;
    TickReadDeviceTime++;
    TickPollingInterval++;
		TickUartChangeBaudrate++;
		

    if(Tick_10mSec >= 100)
    {
        Tick_10mSec = 0 ;	
        Time_Sec++;
        Tick1S_ErrorRateGap++;  
        if ( Tick1S_ErrorRateGap >= 300 )
        {
            Tick1S_ErrorRateGap = 0 ;
            MeterErrorRate5Min_Wp++;
            if ( MeterErrorRate5Min_Wp >= 12 )
            {
                MeterErrorRate5Min_Wp = 0 ;
            }
            for(i=0;i<PwrMtrMax;i++)
            {
                MeterErrorRate5Min_Tx[i][MeterErrorRate5Min_Wp]=1;
                MeterErrorRate5Min_Rx[i][MeterErrorRate5Min_Wp]=1;
            }
        }
    }
    
    SystemTick++;
    if ( SystemTick > SYSTEM_ERROR_TIMEOUT ) 
    {
        //NVIC_SystemReset(); 
        bRecoverSystem = 1 ;
        RecoverSystemMoniter();
    }
		
    if ( HOSTRxQ_cnt > 0 )
    {
        TickHostUart++;
        if ( TickHostUart > 100 )
        {	
            TickHostUart = 0 ;
            ResetHostUART() ;
        }
    }
    if ( READERRxQ_cnt > 0 )
    {
        TickMeterUart++;
        if ( TickMeterUart > 100 )
        {	
            TickMeterUart = 0 ;
            ResetReaderUART();
        }
    }
	
}


/***
 *	@func	UART0/UART2 Handler
 *	@note Revices Buff form according to Devices category
 ***/
void UART02_IRQHandler(void)
{

		uint32_t u32IntSts;
		uint8_t Rxbuf,i;

		u32IntSts = UART2->INTSTS;
		if(u32IntSts & UART_INTSTS_RDAINT_Msk)
		{

				/* Get all the input characters */
				while(UART_IS_RX_READY(UART2))
				{
						/* Get the character from UART Buffer */
						Rxbuf = UART_READ(UART2);
						
						TickMeterUart = 0 ;

						METERRxQ[METERRxQ_wp] = Rxbuf;                        
						METERRxQ_wp++;                 			
						METERRxQ_cnt++;
						if ( METERRxQ_wp >= MAX_METER_RXQ_LENGTH ) METERRxQ_wp=0; 

						//	Devices Rsp check
						switch(SystemPollingState){
							case SYSTEM_POLLING_METER:
									if (METERRxQ[0] != PwrMtrIDArray[PollingPwrMtrID-1]){
											METERRxQ_wp=0;
											METERRxQ_cnt=0; 
											METERRxQ_rp=0; 
											return;										
									}
									break;
							case SYSTEM_POLLING_BMS:
									if (METERRxQ[0] != BmsIDArray[PollingBmsID-1]){
											METERRxQ_wp=0;
											METERRxQ_cnt=0; 
											METERRxQ_rp=0; 
											return;										
									}
									break;
							case SYSTEM_POLLING_WM:
									if (METERRxQ[0] != WtrMtrIDArray[PollingWtrMtrID-1]){
											METERRxQ_wp=0;
											METERRxQ_cnt=0; 
											METERRxQ_rp=0; 
											return;										
									}								
									break;
							case SYSTEM_POLLING_PYR:
									if (METERRxQ[0] != PyrMtrIDArray[PollingPyrMtrID-1]){
											METERRxQ_wp=0;
											METERRxQ_cnt=0; 
											METERRxQ_rp=0; 
											return;										
									}								
									break;
							case SYSTEM_POLLING_SoilSensor:
									if (METERRxQ[0] != SoilSensorIDArray[PollingSoilSensorID-1]){
											METERRxQ_wp=0;
											METERRxQ_cnt=0; 
											METERRxQ_rp=0; 
											return;										
									}								
									break;
							case SYSTEM_POLLING_AirSensor:
									if (METERRxQ[0] != AirSensorIDArray[PollingAirSensorID-1]){
											METERRxQ_wp=0;
											METERRxQ_cnt=0; 
											METERRxQ_rp=0; 
											return;										
									}								
									break;		
							case SYSTEM_POLLING_INV:
									if(!((METERRxQ[0] == 'E') || (METERRxQ[0] == 'V')) &&
											((METERRxQ[1] == 'P') || (METERRxQ[1] == 'G')) &&
											 (METERRxQ[2] == 'H') && 
											 (METERRxQ[3] == 'B')) {
											METERRxQ_wp=0;
											METERRxQ_cnt=0; 
											METERRxQ_rp=0; 
											return;												 
									}
									break;
							default:
									break;
						}
						
						//	data length
						if ( (METERRxQ[1] == 0x03)) {
								if (METERRxQ_wp > 2)
								{		
										METER_RXQLen = METERRxQ[2]+5;
								}
						} else if(SystemPollingState == SYSTEM_POLLING_INV){
								METER_RXQLen = METERRxQ[4]+5;
						} else { 
								METER_RXQLen = 8;
						}

						if( METERRxQ_cnt >= METER_RXQLen )
						{		
								for(i=0;i<METER_RXQLen;i++)
								{
										TokenMeter[i]=METERRxQ[METERRxQ_rp];					
										METERRxQ_cnt--;
										METERRxQ_rp++;
										if(METERRxQ_rp >= MAX_METER_RXQ_LENGTH) METERRxQ_rp = 0 ;
								}
								METERRxQ_wp=0;
								METERRxQ_cnt=0; 
								METERRxQ_rp=0;      
								TokenMeterReady = 1 ;
						}				

				}

		}
	
	//u32IntSts = UART2->INTSTS;
	if(u32IntSts & UART_INTSTS_THREINT_Msk)
	{
		
		if(METERTxQ_cnt > 0)
		{      
			//DIR_METER_RS485_Out();	
			while(UART_IS_TX_FULL(UART2));  /* Wait Tx is not full to transmit data */
			UART_WRITE(UART2, METERTxQ[METERTxQ_rp]);      
			METERTxQ_cnt--;				                   
			METERTxQ_rp++;
			if(METERTxQ_rp >= MAX_METER_TXQ_LENGTH) METERTxQ_rp = 0 ;
			
		} else {		
			/* Disable UART RDA and THRE interrupt */
			//DIR_METER_RS485_In();	
    	UART_DisableInt(UART2, (UART_INTEN_THREIEN_Msk));
			UART_EnableInt(UART2, (UART_INTEN_RDAIEN_Msk));
			NVIC_EnableIRQ(UART02_IRQn);
			fgDirMeterRS485Out2In = 1 ;     
			TickDirMeterRS485Delay = 0 ;
			
		} 	
		
	}

	u32IntSts = UART0->INTSTS;
	if(u32IntSts & UART_INTSTS_RDAINT_Msk)
	{


		/* Get all the input characters */
		while(UART_IS_RX_READY(UART0))
		{
			/* Get the character from UART Buffer */
			Rxbuf = UART_READ(UART0);

			/* Rx completed */		
			HOSTRxQ[HOSTRxQ_wp] = Rxbuf;
			HOSTRxQ_wp++;
			HOSTRxQ_cnt++;
			
			if ( HOSTRxQ_wp >= MAX_HOST_RXQ_LENGTH ) HOSTRxQ_wp=0;
			
			if ( HOSTRxQ[0] != 0x55 ) 
			{
				HOSTRxQ_wp=0;
				HOSTRxQ_cnt=0;
				HOSTRxQ_rp=0;
				return;
			}	
			if( HOSTRxQ_cnt >= HOST_TOKEN_LENGTH )
			{
				if ( HOSTRxQ[HOST_TOKEN_LENGTH-1] != 0x0A ) {
					HOSTRxQ_wp=0;
					HOSTRxQ_cnt=0;
					HOSTRxQ_rp=0;
					return;
				}
				for(i=0;i<HOST_TOKEN_LENGTH;i++)
				{
					TokenHost[i]=HOSTRxQ[HOSTRxQ_rp];
					HOSTRxQ_cnt--;
					HOSTRxQ_rp++;
					if(HOSTRxQ_rp >= MAX_HOST_RXQ_LENGTH) HOSTRxQ_rp = 0 ;
				}
				HOSTRxQ_wp=0;
				HOSTRxQ_cnt=0;
				HOSTRxQ_rp=0;
				HOSTTokenReady = 1 ;
				
			} 
		}

	}
	
	//u32IntSts = UART0->INTSTS;
	if(u32IntSts & UART_INTSTS_THREINT_Msk)
	{
		if(HOSTTxQ_cnt > 0)
		{      
			while(UART_IS_TX_FULL(UART0));  /* Wait Tx is not full to transmit data */
			
			UART_WRITE(UART0, HOSTTxQ[HOSTTxQ_rp]);      
			HOSTTxQ_cnt--;				                   
			HOSTTxQ_rp++;
			if(HOSTTxQ_rp >= MAX_HOST_TXQ_LENGTH) HOSTTxQ_rp = 0 ;
			
		} else {	
			/* Disable UART RDA and THRE interrupt */
    			UART_DisableInt(UART0, (UART_INTEN_THREIEN_Msk));
			UART_EnableInt(UART0, (UART_INTEN_RDAIEN_Msk));
			NVIC_EnableIRQ(UART02_IRQn);
			TickDisableRS485TxDelay = 0 ;
			fgDisableRS485Tx = 1 ;	
		}                	
	}
}
		
/*---------------------------------------------------------------------------------------------------------*/
/*  UART1 Handler                                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
void UART1_IRQHandler(void)
{
	uint32_t u32IntSts;
	uint8_t Rxbuf,i;

#if 1
	
	u32IntSts = UART1->INTSTS;
	if(u32IntSts & UART_INTSTS_RDAINT_Msk)
	{
		/* Get all the input characters */
		while(UART_IS_RX_READY(UART1))
		{
			/* Get the character from UART Buffer */
			Rxbuf = UART_READ(UART1);

			/* Rx completed */		
			READERRxQ[READERRxQ_wp] = Rxbuf;
			READERRxQ_wp++;
			READERRxQ_cnt++;
			
			if ( READERRxQ_wp >= MAX_READER_RXQ_LENGTH ) READERRxQ_wp=0;
			
			if ( READERRxQ[0] != 0x55 ) 
			{
				READERRxQ_wp=0;
				READERRxQ_cnt=0;
				READERRxQ_rp=0;
			}	
			if( READERRxQ_cnt >= READER_TOKEN_LENGTH )
			{
				if ( READERRxQ[READER_TOKEN_LENGTH-1] != 0x0A ) {
					READERRxQ_wp=0;
					READERRxQ_cnt=0;
					READERRxQ_rp=0;
					return;
				}
				for(i=0;i<READER_TOKEN_LENGTH;i++)
				{
					TokenReader[i]=READERRxQ[READERRxQ_rp];
					READERRxQ_cnt--;
					READERRxQ_rp++;
					if(READERRxQ_rp >= MAX_READER_RXQ_LENGTH) READERRxQ_rp = 0 ;
				}
				READERRxQ_wp=0;
				READERRxQ_cnt=0;
				READERRxQ_rp=0;
				READERTokenReady = 1 ;
			} 
		}

	}
	
	u32IntSts = UART1->INTSTS;
	if(u32IntSts & UART_INTSTS_THREINT_Msk)
	{
		if(READERTxQ_cnt > 0)
		{      
			DIR_READER_RS485_Out();
			while(UART_IS_TX_FULL(UART1));  /* Wait Tx is not full to transmit data */
			UART_WRITE(UART1, READERTxQ[READERTxQ_rp]);      
			READERTxQ_cnt--;				                   
			READERTxQ_rp++;
			if(READERTxQ_rp >= MAX_READER_TXQ_LENGTH) 
				READERTxQ_rp = 0 ;		
		} else {
				
			/* Disable UART RDA and THRE interrupt */
			UART_DisableInt(UART1, (UART_INTEN_THREIEN_Msk));
			UART_EnableInt(UART1, (UART_INTEN_RDAIEN_Msk));
			NVIC_EnableIRQ(UART1_IRQn);
			TickDirReaderRS485Delay = 0 ;
			fgDirReaderRS485Out2In = 1 ;		
		}                	
	}
#endif 

}

// Center 
void UART0_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Reset UART0 */
    uint8_t u8UartClkSrcSel, u8UartClkDivNum;
	uint32_t u32ClkTbl[4] = {__HXT, 0, __LXT, __HIRC};
	uint32_t u32Baud_Div = 0;
	uint32_t u32baudrate = 57600 ;

	/*---------------------------------------------------------------------------------------------------------*/
	/* Init UART                                                                                               */
	/*---------------------------------------------------------------------------------------------------------*/
	/* Reset UART1 */
	SYS_ResetModule(UART0_RST);

	/* Configure UART1 and set UART1 Baudrate */
	//UART_Open(UART1, 57600);

	/* Get UART clock source selection */
	u8UartClkSrcSel = (CLK->CLKSEL1 & CLK_CLKSEL1_UARTSEL_Msk) >> CLK_CLKSEL1_UARTSEL_Pos;

	/* Get UART clock divider number */
	u8UartClkDivNum = (CLK->CLKDIV0 & CLK_CLKDIV0_UARTDIV_Msk) >> CLK_CLKDIV0_UARTDIV_Pos;

	/* Select UART function */	
        UART0->FUNCSEL = UART_FUNCSEL_UART;
        //UART0->ALTCTL = UART_ALTCTL_RS485AUD_Msk;
        
        /* Set RTS pin active level as high level active */
    	//UART1->MODEM = (UART1->MODEM & (~UART_MODEM_RTSACTLV_Msk)) | UART_RTS_IS_HIGH_LEV_ACTIVE;

        /* Set TX delay time */
    	//UART1->TOUT = 0;

	/* Set UART line configuration */
	UART0->LINE = UART_WORD_LEN_8 | UART_PARITY_NONE | UART_STOP_BIT_1;

	/* Set UART Rx and RTS trigger level */
	UART0->FIFO &= ~(UART_FIFO_RFITL_Msk | UART_FIFO_RTSTRGLV_Msk);    

	/* Get PLL clock frequency if UART clock source selection is PLL */
	if(u8UartClkSrcSel == 1)
		u32ClkTbl[u8UartClkSrcSel] = CLK_GetPLLClockFreq();

	/* Set UART baud rate */
	if(u32baudrate != 0)
	{
		u32Baud_Div = UART_BAUD_MODE2_DIVIDER((u32ClkTbl[u8UartClkSrcSel]) / (u8UartClkDivNum + 1), u32baudrate);

		if(u32Baud_Div > 0xFFFF)
			UART0->BAUD = (UART_BAUD_MODE0 | UART_BAUD_MODE0_DIVIDER((u32ClkTbl[u8UartClkSrcSel]) / (u8UartClkDivNum + 1), u32baudrate));
		else
			UART0->BAUD = (UART_BAUD_MODE2 | u32Baud_Div);
    	}
	
	/* Enable UART RDA and THRE interrupt */	
	//RT_EnableInt(UART0, (UART_INTEN_RDAIEN_Msk | UART_INTEN_THREIEN_Msk ));
	UART_EnableInt(UART0, (UART_INTEN_RDAIEN_Msk  ));
	NVIC_EnableIRQ(UART02_IRQn);
	DisableHostUartTx();
	
}

// Reader 
void UART1_Init(void)
{
     uint8_t u8UartClkSrcSel, u8UartClkDivNum;
	uint32_t u32ClkTbl[4] = {__HXT, 0, __LXT, __HIRC};
	uint32_t u32Baud_Div = 0;
	uint32_t u32baudrate = 57600 ;

	/*---------------------------------------------------------------------------------------------------------*/
	/* Init UART                                                                                               */
	/*---------------------------------------------------------------------------------------------------------*/
	/* Reset UART1 */
	SYS_ResetModule(UART1_RST);

	/* Configure UART1 and set UART1 Baudrate */
	//UART_Open(UART1, 57600);

	/* Get UART clock source selection */
	u8UartClkSrcSel = (CLK->CLKSEL1 & CLK_CLKSEL1_UARTSEL_Msk) >> CLK_CLKSEL1_UARTSEL_Pos;

	/* Get UART clock divider number */
	u8UartClkDivNum = (CLK->CLKDIV0 & CLK_CLKDIV0_UARTDIV_Msk) >> CLK_CLKDIV0_UARTDIV_Pos;

	/* Select UART function */
	//UART1->FUNCSEL = UART_FUNCSEL_UART;

	/* Select UART RS485 function mode */
    	UART1->FUNCSEL = UART_FUNCSEL_RS485;

    	/* Set RS485-Master as AUD mode */
    	/* Enable AUD mode to HW control RTS pin automatically */
    	/* It also can use GPIO to control RTS pin for replacing AUD mode */
    	UART1->ALTCTL = UART_ALTCTL_RS485AUD_Msk;

    	/* Set RTS pin active level as high level active */
    	UART1->MODEM = (UART1->MODEM & (~UART_MODEM_RTSACTLV_Msk)) | UART_RTS_IS_HIGH_LEV_ACTIVE;

    	/* Set TX delay time */
    	UART1->TOUT = 0;

	/* Set UART line configuration */
	UART1->LINE = UART_WORD_LEN_8 | UART_PARITY_NONE | UART_STOP_BIT_1;

	/* Set UART Rx and RTS trigger level */
	UART1->FIFO &= ~(UART_FIFO_RFITL_Msk | UART_FIFO_RTSTRGLV_Msk);

	/* Get PLL clock frequency if UART clock source selection is PLL */
	if(u8UartClkSrcSel == 1)
		u32ClkTbl[u8UartClkSrcSel] = CLK_GetPLLClockFreq();

	/* Set UART baud rate */
	if(u32baudrate != 0)
	{
		u32Baud_Div = UART_BAUD_MODE2_DIVIDER((u32ClkTbl[u8UartClkSrcSel]) / (u8UartClkDivNum + 1), u32baudrate);

		if(u32Baud_Div > 0xFFFF)
			UART1->BAUD = (UART_BAUD_MODE0 | UART_BAUD_MODE0_DIVIDER((u32ClkTbl[u8UartClkSrcSel]) / (u8UartClkDivNum + 1), u32baudrate));
		else
			UART1->BAUD = (UART_BAUD_MODE2 | u32Baud_Div);
    	}
	
	/* Enable UART RDA and THRE interrupt */
	//UART_EnableInt(UART1, (UART_INTEN_RDAIEN_Msk | UART_INTEN_THREIEN_Msk));
	UART_EnableInt(UART1, (UART_INTEN_RDAIEN_Msk ));
	NVIC_EnableIRQ(UART1_IRQn);
}

// Power Meter
void UART2_Init(uint32_t u32baudrate)
{
  	uint8_t u8UartClkSrcSel, u8UartClkDivNum;
	uint32_t u32ClkTbl[4] = {__HXT, 0, __LXT, __HIRC};
	uint32_t u32Baud_Div = 0;
	

	/*---------------------------------------------------------------------------------------------------------*/
	/* Init UART                                                                                               */
	/*---------------------------------------------------------------------------------------------------------*/
	/* Reset UART2 */
	SYS_ResetModule(UART2_RST);

	/* Configure UART1 and set UART1 Baudrate */
	//UART_Open(UART2, u32baudrate);
#if 1
	/* Get UART clock source selection */
	u8UartClkSrcSel = (CLK->CLKSEL1 & CLK_CLKSEL1_UARTSEL_Msk) >> CLK_CLKSEL1_UARTSEL_Pos;

	/* Get UART clock divider number */
	u8UartClkDivNum = (CLK->CLKDIV0 & CLK_CLKDIV0_UARTDIV_Msk) >> CLK_CLKDIV0_UARTDIV_Pos;

	/* Select UART function */
	//UART2->FUNCSEL = UART_FUNCSEL_UART;
	/* Select UART RS485 function mode */
    	UART2->FUNCSEL = UART_FUNCSEL_RS485;

    	/* Set RS485-Master as AUD mode */
    	/* Enable AUD mode to HW control RTS pin automatically */
    	/* It also can use GPIO to control RTS pin for replacing AUD mode */
    	UART2->ALTCTL = UART_ALTCTL_RS485AUD_Msk;

    	/* Set RTS pin active level as high level active */
    	UART2->MODEM = (UART2->MODEM & (~UART_MODEM_RTSACTLV_Msk)) | UART_RTS_IS_HIGH_LEV_ACTIVE;

    	/* Set TX delay time */
    	UART2->TOUT = 0;
		
	//UART_SelectRS485Mode(UART2,UART_ALTCTL_RS485AUD_Msk,0);

	/* Set UART line configuration */
	UART2->LINE = UART_WORD_LEN_8 | UART_PARITY_NONE | UART_STOP_BIT_1;

	/* Set UART Rx and RTS trigger level */
	UART2->FIFO &= ~(UART_FIFO_RFITL_Msk | UART_FIFO_RTSTRGLV_Msk);

	/* Get PLL clock frequency if UART clock source selection is PLL */
	if(u8UartClkSrcSel == 1)
		u32ClkTbl[u8UartClkSrcSel] = CLK_GetPLLClockFreq();

	/* Set UART baud rate */
	if(u32baudrate != 0)
	{
		u32Baud_Div = UART_BAUD_MODE2_DIVIDER((u32ClkTbl[u8UartClkSrcSel]) / (u8UartClkDivNum + 1), u32baudrate);

		if(u32Baud_Div > 0xFFFF)
			UART2->BAUD = (UART_BAUD_MODE0 | UART_BAUD_MODE0_DIVIDER((u32ClkTbl[u8UartClkSrcSel]) / (u8UartClkDivNum + 1), u32baudrate));
		else
			UART2->BAUD = (UART_BAUD_MODE2 | (u32Baud_Div));
    	}
#endif 

	/* Enable UART RDA and THRE interrupt */
	//UART_EnableInt(UART2, (UART_INTEN_RDAIEN_Msk | UART_INTEN_THREIEN_Msk));
	UART_EnableInt(UART2, (UART_INTEN_RDAIEN_Msk  ));
	NVIC_EnableIRQ(UART02_IRQn);
}

void SYS_Init(void)
{

	/* Set PF multi-function pins for X32_OUT(PF.0) and X32_IN(PF.1) */
	SYS->GPF_MFPL = (SYS->GPF_MFPL & (~SYS_GPF_MFPL_PF0MFP_Msk)) | SYS_GPF_MFPL_PF0MFP_X32_OUT;
	SYS->GPF_MFPL = (SYS->GPF_MFPL & (~SYS_GPF_MFPL_PF1MFP_Msk)) | SYS_GPF_MFPL_PF1MFP_X32_IN;

	/* Set PF multi-function pins for XT1_OUT(PF.3) and XT1_IN(PF.4) */
	//SYS->GPF_MFPL = (SYS->GPF_MFPL & (~SYS_GPF_MFPL_PF3MFP_Msk)) | SYS_GPF_MFPL_PF3MFP_XT1_OUT;
	//SYS->GPF_MFPL = (SYS->GPF_MFPL & (~SYS_GPF_MFPL_PF4MFP_Msk)) | SYS_GPF_MFPL_PF4MFP_XT1_IN;

	
	/*---------------------------------------------------------------------------------------------------------*/
	/* Init System Clock                                                                                       */
	/*---------------------------------------------------------------------------------------------------------*/

	/* Enable HIRC, HXT and LXT clock */
	/* Enable Internal RC 22.1184MHz clock */
	CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk | CLK_PWRCTL_LXTEN_Msk);

	/* Wait for HIRC, HXT and LXT clock ready */
	CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk | CLK_STATUS_LXTSTB_Msk);

	/* Select HCLK clock source as HIRC and HCLK clock divider as 1 */
	CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));

	/* Set core clock as PLL_CLOCK from PLL */
	CLK_SetCoreClock(PLL_CLOCK);

	CLK_SetSysTickClockSrc(CLK_CLKSEL0_STCLKSEL_HCLK_DIV2);

	/* Enable UART module clock */
	CLK_EnableModuleClock(UART0_MODULE);     
	CLK_EnableModuleClock(UART1_MODULE);
	CLK_EnableModuleClock(UART2_MODULE);
	
	CLK_EnableModuleClock(ISP_MODULE);
	CLK_EnableModuleClock(CRC_MODULE);
	CLK_EnableModuleClock(WDT_MODULE);

	/* Select UART module clock source as HXT and UART module clock divider as 1 */
	CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UARTSEL_HIRC, CLK_CLKDIV0_UART(1));
	CLK_SetModuleClock(UART1_MODULE, CLK_CLKSEL1_UARTSEL_HIRC, CLK_CLKDIV0_UART(1));
	CLK_SetModuleClock(UART2_MODULE, CLK_CLKSEL1_UARTSEL_HIRC, CLK_CLKDIV0_UART(1));
	CLK_SetModuleClock(WDT_MODULE, CLK_CLKSEL1_WDTSEL_HCLK_DIV2048, MODULE_NoMsk);
	
#define SYS_IRCTCTL0_AUTO_TRIM_HIRC_ENABLE 0x01
	// Auto Trim HIRC to 22.1184 MHz
	SYS->IRCTCTL0 = (SYS->IRCTCTL0 & (~SYS_IRCTCTL0_FREQSEL_Msk)) | (SYS_IRCTCTL0_AUTO_TRIM_HIRC_ENABLE << SYS_IRCTCTL0_FREQSEL_Pos);
	SYS->IRCTCTL0 = (SYS->IRCTCTL0 & (~SYS_IRCTCTL0_REFCKSEL_Msk)) | (0x00 << SYS_IRCTCTL0_REFCKSEL_Pos) ;
	SYS->IRCTCTL0 = (SYS->IRCTCTL0 & (~SYS_IRCTCTL0_LOOPSEL_Msk)) | (0x11<<SYS_IRCTCTL0_LOOPSEL_Pos) ;
	SYS->IRCTCTL0 = (SYS->IRCTCTL0 & (~SYS_IRCTCTL0_RETRYCNT_Msk)) | (0x11<<SYS_IRCTCTL0_RETRYCNT_Pos) ;

	/*---------------------------------------------------------------------------------------------------------*/
	/* Init I/O Multi-function                                                                                 */
	/*---------------------------------------------------------------------------------------------------------*/

	/* Set PD multi-function pins for UART0 RXD(PD.0) and TXD(PD.1) */
	SYS->GPD_MFPL = (SYS->GPD_MFPL & (~SYS_GPD_MFPL_PD0MFP_Msk)) | SYS_GPD_MFPL_PD0MFP_UART0_RXD;
	SYS->GPD_MFPL = (SYS->GPD_MFPL & (~SYS_GPD_MFPL_PD1MFP_Msk)) | SYS_GPD_MFPL_PD1MFP_UART0_TXD;
	/* Set PE multi-function pins for UART1 RXD(PE.13) and TXD(PE.12) */
	SYS->GPE_MFPH = (SYS->GPE_MFPH & (~SYS_GPE_MFPH_PE13MFP_Msk)) | SYS_GPE_MFPH_PE13MFP_UART1_RXD;
	SYS->GPE_MFPH = (SYS->GPE_MFPH & (~SYS_GPE_MFPH_PE12MFP_Msk)) | SYS_GPE_MFPH_PE12MFP_UART1_TXD;
	SYS->GPE_MFPH = (SYS->GPE_MFPH & (~SYS_GPE_MFPH_PE11MFP_Msk)) | SYS_GPE_MFPH_PE11MFP_UART1_nRTS;
	/* Set PC multi-function pins for UART2 RXD(PC.3) and TXD(PC.2) */
	SYS->GPC_MFPL = (SYS->GPC_MFPL & (~SYS_GPC_MFPL_PC3MFP_Msk)) | SYS_GPC_MFPL_PC3MFP_UART2_RXD;
	SYS->GPC_MFPL = (SYS->GPC_MFPL & (~SYS_GPC_MFPL_PC2MFP_Msk)) | SYS_GPC_MFPL_PC2MFP_UART2_TXD;		
	SYS->GPC_MFPL = (SYS->GPC_MFPL & (~SYS_GPC_MFPL_PC1MFP_Msk)) | SYS_GPC_MFPL_PC1MFP_UART2_nRTS;	
	
	/* Set PF multi-function pins for X32_OUT(PF.0) and X32_IN(PF.1) */
	//SYS->GPF_MFPL = (SYS->GPF_MFPL & (~SYS_GPF_MFPL_PF0MFP_Msk)) | SYS_GPF_MFPL_PF0MFP_X32_OUT;
	//SYS->GPF_MFPL = (SYS->GPF_MFPL & (~SYS_GPF_MFPL_PF1MFP_Msk)) | SYS_GPF_MFPL_PF1MFP_X32_IN;	
	
	/* Set PF multi-function pins for X32_OUT(PF.0) and X32_IN(PF.1) */
	//SYS->GPF_MFPL = (SYS->GPF_MFPL & (~SYS_GPF_MFPL_PF3MFP_Msk)) | SYS_GPF_MFPL_PF3MFP_XT1_OUT;
	//SYS->GPF_MFPL = (SYS->GPF_MFPL & (~SYS_GPF_MFPL_PF4MFP_Msk)) | SYS_GPF_MFPL_PF4MFP_XT1_IN;	

}

void GPIO_Mode_Init(void)
{

	
	PA3 = 1 ;
	PF7 = 1 ;
	
	// PA
	/* Configure PB.2 as Output mode and PE.1 as Input mode then close it */
	PA->MODE = (PA->MODE & (~GPIO_MODE_MODE0_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE0_Pos);
	PA->MODE = (PA->MODE & (~GPIO_MODE_MODE1_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE1_Pos);
	PA->MODE = (PA->MODE & (~GPIO_MODE_MODE2_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE2_Pos);
	PA->MODE = (PA->MODE & (~GPIO_MODE_MODE3_Msk)) | (GPIO_MODE_OPEN_DRAIN << GPIO_MODE_MODE3_Pos);
	// PB	
	PB->MODE = (PB->MODE & (~GPIO_MODE_MODE0_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE0_Pos);	
	PB->MODE = (PB->MODE & (~GPIO_MODE_MODE1_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE1_Pos);	
	PB->MODE = (PB->MODE & (~GPIO_MODE_MODE2_Msk)) | (GPIO_MODE_QUASI << GPIO_MODE_MODE2_Pos);
	PB->MODE = (PB->MODE & (~GPIO_MODE_MODE3_Msk)) | (GPIO_MODE_QUASI << GPIO_MODE_MODE3_Pos);
	PB->MODE = (PB->MODE & (~GPIO_MODE_MODE4_Msk)) | (GPIO_MODE_QUASI << GPIO_MODE_MODE4_Pos);
	PB->MODE = (PB->MODE & (~GPIO_MODE_MODE5_Msk)) | (GPIO_MODE_QUASI << GPIO_MODE_MODE5_Pos);
	PB->MODE = (PB->MODE & (~GPIO_MODE_MODE6_Msk)) | (GPIO_MODE_QUASI << GPIO_MODE_MODE6_Pos);
	PB->MODE = (PB->MODE & (~GPIO_MODE_MODE7_Msk)) | (GPIO_MODE_QUASI << GPIO_MODE_MODE7_Pos);
	//PC
	PC->MODE = (PC->MODE & (~GPIO_MODE_MODE0_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE0_Pos);
	//PC->MODE = (PC->MODE & (~GPIO_MODE_MODE1_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE1_Pos);
	PC->MODE = (PC->MODE & (~GPIO_MODE_MODE4_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE4_Pos);
	// PD
	PD->MODE = (PD->MODE & (~GPIO_MODE_MODE2_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE2_Pos);
	PD->MODE = (PD->MODE & (~GPIO_MODE_MODE3_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE3_Pos);
	PD->MODE = (PD->MODE & (~GPIO_MODE_MODE7_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE7_Pos);
	// PE
	PE->MODE = (PE->MODE & (~GPIO_MODE_MODE0_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE0_Pos);
	PE->MODE = (PE->MODE & (~GPIO_MODE_MODE10_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE10_Pos);
	//PE->MODE = (PE->MODE & (~GPIO_MODE_MODE11_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE11_Pos);
	// PF
	//PF->MODE = (PF->MODE & (~GPIO_MODE_MODE0_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE0_Pos);
	//PF->MODE = (PF->MODE & (~GPIO_MODE_MODE1_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE1_Pos);
	PF->MODE = (PF->MODE & (~GPIO_MODE_MODE2_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE2_Pos);
	PF->MODE = (PF->MODE & (~GPIO_MODE_MODE3_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE3_Pos);
	PF->MODE = (PF->MODE & (~GPIO_MODE_MODE4_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE4_Pos);
	PF->MODE = (PF->MODE & (~GPIO_MODE_MODE7_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE7_Pos);
	
	
}
				
//WDT Init
void WDT_Init(void)
{
		/* WDT SETUP*/
		WDT_Close();
		WDT_Open(WDT_TIMEOUT_2POW16, WDT_RESET_DELAY_130CLK, TRUE, FALSE);
		WDT_EnableInt();
		NVIC_EnableIRQ(WDT_IRQn);
		WDT_CLEAR_RESET_FLAG();
		WDT_RESET_COUNTER();
}


void TestI2C_RWEEPROM(void)
{
	uint8_t TestDataBuf[100];
	uint8_t i;
	
	for(i = 0; i < 100; i++)
	{
		soft_i2c_eeprom_write_byte(EEPROM_ADDR,i,i+10);
	}
	for(i = 0; i < 100; i++)
	{
		TestDataBuf[i] = soft_i2c_eeprom_read_byte(EEPROM_ADDR,i);
	}

	for(i = 0; i < 100; i++)
	{		if ( TestDataBuf[i] != i )
		{  
			ErrorCode = 0x01 ;
		}
	}
}

// Power Meter
void UART2_ReInitial(uint32_t u32baudrate,int FregMix)
{
  	uint8_t u8UartClkSrcSel, u8UartClkDivNum;
	uint32_t u32ClkTbl[4] = {__HXT, 0, __LXT, __HIRC};
	uint32_t u32Baud_Div = 0;
	

	/*---------------------------------------------------------------------------------------------------------*/
	/* Init UART                                                                                               */
	/*---------------------------------------------------------------------------------------------------------*/
	/* Reset UART2 */
	SYS_ResetModule(UART2_RST);

	/* Configure UART1 and set UART1 Baudrate */
	//UART_Open(UART2, u32baudrate);
#if 1
	/* Get UART clock source selection */
	u8UartClkSrcSel = (CLK->CLKSEL1 & CLK_CLKSEL1_UARTSEL_Msk) >> CLK_CLKSEL1_UARTSEL_Pos;

	/* Get UART clock divider number */
	u8UartClkDivNum = (CLK->CLKDIV0 & CLK_CLKDIV0_UARTDIV_Msk) >> CLK_CLKDIV0_UARTDIV_Pos;

	/* Get PLL clock frequency if UART clock source selection is PLL */
	if(u8UartClkSrcSel == 1)
		u32ClkTbl[u8UartClkSrcSel] = CLK_GetPLLClockFreq();

	/* Set UART baud rate */
	if(u32baudrate != 0)
	{
		u32Baud_Div = UART_BAUD_MODE2_DIVIDER((u32ClkTbl[u8UartClkSrcSel]) / (u8UartClkDivNum + 1), u32baudrate);

		if(u32Baud_Div > 0xFFFF)
			UART2->BAUD = (UART_BAUD_MODE0 | UART_BAUD_MODE0_DIVIDER((u32ClkTbl[u8UartClkSrcSel]) / (u8UartClkDivNum + 1), u32baudrate));
		else
			UART2->BAUD = (UART_BAUD_MODE2 | (u32Baud_Div-FregMix));
    	}
#endif 

	/* Enable UART RDA and THRE interrupt */
	//UART_EnableInt(UART2, (UART_INTEN_RDAIEN_Msk | UART_INTEN_THREIEN_Msk));
	UART_EnableInt(UART2, (UART_INTEN_RDAIEN_Msk  ));
	NVIC_EnableIRQ(UART02_IRQn);
}

uint32_t u32_RSTScr;

int main()
{
	
  uint8_t i,j;
	
	uint32_t rst = SYS->RSTSTS;
	SYS->RSTSTS = rst;
	//rst & SYS_RSTSTS_WDTRF_Msk


	/* Because of all bits can be written in WDT Control Register are write-protected;
	To program it needs to disable register protection first. */
	
	SYS_UnlockReg();
	SYS_Init();
	AO2022_MeterModule_1261_init();
	WDT_Init();
	FMC_Open();
	

		
	/* To check if system has been reset by WDT time-out reset or not */
	//if(WDT_GET_RESET_FLAG() == 1)
	//{
		//WDT_CLEAR_RESET_FLAG();
		//printf("*** System has been reset by WDT time-out event ***\n\n");
		//while(1);
	//}
	/* Enable WDT NVIC */
	//NVIC_EnableIRQ(WDT_IRQn);
		
	GPIO_Mode_Init();
	ROOM_POWER_On();
	//DIR_READER_RS485_In();
	//DIR_METER_RS485_In();
	
	u32_RSTScr = SYS_GetResetSrc();
	if ( (u32_RSTScr & (SYS_RSTSTS_WDTRF_Msk | SYS_RSTSTS_PINRF_Msk | SYS_RSTSTS_CPURF_Msk | SYS_RSTSTS_CPULKRF_Msk)) > 0 )
	{
		// INIT. METER
		//fgWaitInitFromCenter = 1 ;
		
		SYS->RSTSTS = 0 ;
		
	}
	
	fgFirstStart=1;
	
	ROOM_POWER_On();
	//RELAY2_High();
	//RELAY1_High();		// ROOM POWER ON 
	LED_G_On();
	LED_G_Off();
	LED_R_On();
	LED_R_Off();
	LED_G1_On();
	LED_G1_Off();
	LED_R1_On();
	LED_R1_Off();
	//READER_POWER_Off();
	READER_POWER_On();
	
	/*Test integrity of Meter FW */
//	VerifyFW(fgFirstStart);
	
	SysTick_Config(PLL_CLOCK/2/100);
	NVIC_EnableIRQ(SysTick_IRQn);
	 
	UART0_Init();
	UART1_Init();
	

	
	/* Lock protected registers */
	SYS_LockReg();

	UART2_Init(9600);

  
	DisableHostUartTx();
	NVIC_EnableIRQ(UART1_IRQn);
	NVIC_EnableIRQ(UART02_IRQn);

	
	SoftI2cMasterInit();
	//TestI2C_RWEEPROM();
	WDT_RESET_COUNTER();
	
//	STR_METER_D MD;
//	for(uint8_t i=0; i < PwrMtrMax; i++)
//	{
//		I2cReadDataStruct(i, &MD);
//		MeterData[i] = MD;
//	}
		
	//SendReader_Alive();
	//while(1);
	
	//for(i=0;i<10;i++)
		//ReaderTxBuffer[i] = i +0x30 ;
	//DIR_READER_RS485_Out();
	//_SendStringToREADER(ReaderTxBuffer,10);
	
    //MaxPowerMeter = PwrMtrMax ;
    MaxPowerMeter = 1 ;


    
    MeterErrorRate5Min_Wp = 0 ;
	
#ifdef METER_TEST 
	MeterValueTest = 10000 ;
#endif 	 	
	
		ReadMyMeterBoardID();

		Delay_10ms(75);				// Delay for System stable
		
		SoftI2cMasterInit();
    
    ReadMyMeterBoardID();		
    fgToReaderRSPFlag = 0xFF;
    fgToHostRSPFlag = 0xFF ;	        
    MeterActive = 0x01 ;
	
// Initial default 
#if 0
	SystemTick = 0 ;
	DefaultValueTest();
	TotalWattValue_Now = 0 ;
	
	
#endif 	


SystemTick = 0 ;
	
//#define TEST_EEPROM	1

#ifdef TEST_EEPROM
#define TEST_EE_ADDR 0x0000
	TestEEPROM();	
#endif 
	
	SystemTick = 0 ;	
	
	ReadMyMeterBoardID();
	DisableHostUartTx();
	fgDIR485_Reader=0;			
	//DIR485_READER_In();
	
	u32TimeTick2=0;   
	bRegister = 1 ;
	fgToReaderFlag = 0x40 ;
	
	DelayTick=0;
	
	MeterType = CIC_BAW1A ; //CIC_BAW2A ; 	// For DongHwa , Meter is 0x01 (DAE DEM530)	
	TickReadDeviceTime = 0 ;
	PowerOnReadMeter = 1 ;

#ifdef METER_TEST 
	MeterValueTest = 10000 ;
#endif 

		ReadDeviceCmdTime = 20 ;	 	// Reading Power Meter per 20 x 20 mSec

		/***	@Bms, @WtrMtr & @PwrMtr & @SoilSensor & @AirSensor & @Pyranometer Initialize function	***/
		InitializePollingIDs();

    do {
        //RoomData.RoomMode = RM_POWER_OFF_READY ;
				WDT_RESET_COUNTER();
        SystemTick = 0 ;				// Clear System S/W watchdog 		
        ModbusDataProcess();
        SystemPolling();
        ReadMyMeterBoardID();					
        HostProcess();		
        SystemTick = 0 ;		
        ChangeRS485Direct();		
        //SaveSetting2EE();							
        RecoverSystemMoniter();				
        CalErrorRate();
				
    } while (1); 
}


void EnableHostUartTx(void)
{
	SYS->GPD_MFPL &= ~(SYS_GPD_MFPL_PD1MFP_Msk | SYS_GPD_MFPL_PD0MFP_Msk);
	SYS->GPD_MFPL |= (SYS_GPD_MFPL_PD1MFP_UART0_TXD | SYS_GPD_MFPL_PD0MFP_UART0_RXD);
}

void DisableHostUartTx(void)
{
	SYS->GPD_MFPL &= ~(SYS_GPD_MFPL_PD1MFP_Msk );
	PD->MODE = (PD->MODE & (~GPIO_MODE_MODE1_Msk)) | (GPIO_MODE_QUASI << GPIO_MODE_MODE1_Pos);
}


float fErrorRate,tmpTx,tmpRx;

/***
*	@brief		Calculate error rate.
* @device		power meter, Bms, water meter, Pyranometer inverter *
 ***/
void CalErrorRate(void)
{
	  uint8_t i,j;
		float acmTx, acmRx;
	
//		//	power meter error rate calculation
//    
//		acmTx = 0; acmRx = 0;
//    for (i=0;i<PwrMtrMax;i++)
//    {
//        MeterErrorRate_Tx[i] = 0 ;
//        MeterErrorRate_Rx[i] = 0 ;
//        for ( j=0;j<12;j++)
//        {
//            MeterErrorRate_Tx[i] += MeterErrorRate5Min_Tx[i][j];
//            MeterErrorRate_Rx[i] += MeterErrorRate5Min_Rx[i][j];
//        }
//        tmpTx = (float) MeterErrorRate_Tx[i];
//        tmpRx = (float) MeterErrorRate_Rx[i];
//        fErrorRate = tmpRx/tmpTx;
//        MeterErrorRate[i] = (uint8_t) (fErrorRate*100);
//				acmTx += (float)tmpTx;
//				acmRx += (float)tmpRx;				
//		}
//		fErrorRate = acmRx/acmTx;
//		TotErrorRate.PowerMeter = (uint8_t) (fErrorRate*100);
		//	Bms error rate calculation
		
		acmTx = 0;	acmRx = 0;
		for (uint8_t i = 0; i < PwrMtrMax; i++ )
		{		
				if ((PwrMtrError.Fail[i] + PwrMtrError.Success[i]) > 0)
				{
						tmpTx = (float)(PwrMtrError.Fail[i] + PwrMtrError.Success[i]);
						tmpRx = (float)PwrMtrError.Success[i];
						fErrorRate = tmpRx/tmpTx;
						PwrMtrError.ErrorRate[i] = (uint8_t) (fErrorRate*100);
				} else {
						tmpTx = 0;
						tmpRx = 0;
						PwrMtrError.ErrorRate[i] = 0;
				}
				acmTx += tmpTx;
				acmRx += tmpRx;
		}
		fErrorRate = acmRx/acmTx;
		TotErrorRate.PowerMeter = (uint8_t) (fErrorRate*100);	
		
		//	Bms error rate calculation
		acmTx = 0;	acmRx = 0;
		for (uint8_t i = 0; i < BmsMax; i++ )
		{		
				if ((BmsError.Fail[i] + BmsError.Success[i]) > 0)
				{
						tmpTx = (float)(BmsError.Fail[i] + BmsError.Success[i]);
						tmpRx = (float)BmsError.Success[i];
						fErrorRate = tmpRx/tmpTx;
						BmsError.ErrorRate[i] = (uint8_t) (fErrorRate*100);
				} else {
						tmpTx = 0;
						tmpRx = 0;
						BmsError.ErrorRate[i] = 0;
				}
				acmTx += tmpTx;
				acmRx += tmpRx;
		}
		fErrorRate = acmRx/acmTx;
		TotErrorRate.Bms = (uint8_t) (fErrorRate*100);
		
		//	WM error rate culculation
		acmTx = 0;	acmRx = 0;
		for (uint8_t i = 0; i < WtrMtrMax; i++ )
		{
				
				if ((WtrMtrError.Fail[i] + WtrMtrError.Success[i]) > 0)
				{
						tmpTx = (float)WtrMtrError.Fail[i] + WtrMtrError.Success[i];
						tmpRx = (float)WtrMtrError.Success[i];
						fErrorRate = tmpRx/tmpTx;
						WtrMtrError.ErrorRate[i] = (uint8_t) (fErrorRate*100);
				} else {
						tmpTx = 0;
						tmpRx = 0;
						WtrMtrError.ErrorRate[i] = 0;
				}
				acmTx += tmpTx;
				acmRx += tmpRx;
		}
		fErrorRate = acmRx/acmTx;
		TotErrorRate.WaterMeter = (uint8_t) (fErrorRate*100);

		//	Pyranometer error rate culculation
		acmTx = 0;	acmRx = 0;
		for (uint8_t i = 0; i < PyrMtrMax; i++ )
		{
				
				if ((PyrMtrError.Fail[i] + PyrMtrError.Success[i]) > 0)
				{
						tmpTx = (float)PyrMtrError.Fail[i] + PyrMtrError.Success[i];
						tmpRx = (float)PyrMtrError.Success[i];
						fErrorRate = tmpRx/tmpTx;
						PyrMtrError.ErrorRate[i] = (uint8_t) (fErrorRate*100);
				} else {
						tmpTx = 0;
						tmpRx = 0;
						PyrMtrError.ErrorRate[i] = 0;
				}
				acmTx += tmpTx;
				acmRx += tmpRx;
		}
		fErrorRate = acmRx/acmTx;
		TotErrorRate.Pyranometer = (uint8_t) (fErrorRate*100);
		
		//	Soil Sensor error rate culculation
		acmTx = 0;	acmRx = 0;
		for (uint8_t i = 0; i < SoilSensorMax; i++ )
		{
				
				if ((SoilSensorError.Fail[i] + SoilSensorError.Success[i]) > 0)
				{
						tmpTx = (float)SoilSensorError.Fail[i] + SoilSensorError.Success[i];
						tmpRx = (float)SoilSensorError.Success[i];
						fErrorRate = tmpRx/tmpTx;
						SoilSensorError.ErrorRate[i] = (uint8_t) (fErrorRate*100);
				} else {
						tmpTx = 0;
						tmpRx = 0;
						SoilSensorError.ErrorRate[i] = 0;
				}
				acmTx += tmpTx;
				acmRx += tmpRx;
		}
		fErrorRate = acmRx/acmTx;
		TotErrorRate.SoilSensor = (uint8_t) (fErrorRate*100);

		//	Air Sensor error rate culculation
		acmTx = 0;	acmRx = 0;
		for (uint8_t i = 0; i < AirSensorMax; i++ )
		{
				
				if ((AirSensorError.Fail[i] + AirSensorError.Success[i]) > 0)
				{
						tmpTx = (float)AirSensorError.Fail[i] + AirSensorError.Success[i];
						tmpRx = (float)AirSensorError.Success[i];
						fErrorRate = tmpRx/tmpTx;
						AirSensorError.ErrorRate[i] = (uint8_t) (fErrorRate*100);
				} else {
						tmpTx = 0;
						tmpRx = 0;
						AirSensorError.ErrorRate[i] = 0;
				}
				acmTx += tmpTx;
				acmRx += tmpRx;
		}
		fErrorRate = acmRx/acmTx;
		TotErrorRate.AirSensor = (uint8_t) (fErrorRate*100);
		
		//	Inv error rate calculation
		if ((InvError.Fail + InvError.Success) > 0)
		{
				tmpTx = (float)InvError.Fail + InvError.Success;
				tmpRx = (float)InvError.Success;
				fErrorRate = tmpRx/tmpTx;
				InvError.ErrorRate = (uint8_t) (fErrorRate*100);
				TotErrorRate.Inverter = (uint8_t) (fErrorRate*100);
		} else {
				tmpTx = 0;
				tmpRx = 0;
				InvError.ErrorRate = 0;
				TotErrorRate.Inverter = InvError.ErrorRate;
		}
		
}


void RecoverSystemMoniter(void)
{
	if ( bRecoverSystem )
	{
		bRecoverSystem = 0 ;
		LED_G_On();
		ResetHostUART();
		ResetReaderUART();		
		LED_G_Off();
	}
}


#define RS485_DIR_DELAY_TIME 	100

void ChangeRS485Direct(void)
{

	if ( (fgDisableRS485Tx > 0) && (HOSTTxQ_cnt == 0) )
	{		
		TickDisableRS485TxDelay++;
		if ( TickDisableRS485TxDelay > RS485_DIR_DELAY_TIME )
		{ 				
			fgDisableRS485Tx = 0 ;
			TickDisableRS485TxDelay = 0 ;
			DisableHostUartTx();
		}
	}	
	
}

void ReadUserInfoFromEE(void)
{
	uint8_t i,j,addrC;
	// Ū���d��
	addrC=0;
	for(i=0;i<MEMBER_MAX_NO;i++)
	{
		for(j=0;j<4;j++)
		{
			//Members[i].UID[j] = soft_i2c_eeprom_read_byte(EEPROM_ADDR, EE_ADDR_USER_UID+addrC);
			addrC++;
		}
	}
	// Ū���Ǹ�
	addrC=0;
	for(i=0;i<MEMBER_MAX_NO;i++)
	{
		for(j=0;j<5;j++)
		{
			//Members[i].SID[j]=soft_i2c_eeprom_read_byte(EEPROM_ADDR, EE_ADDR_STUDENT_ID+addrC);
			addrC++;
		}
	}
}

void WriteUserInfoToEE(void)
{
	uint8_t i,j,addrC;
	// �g�J�d��
	addrC=0;
	for(i=0;i<MEMBER_MAX_NO;i++)
	{
		for(j=0;j<4;j++)
		{
			//soft_i2c_eeprom_write_byte(EEPROM_ADDR, EE_ADDR_USER_UID+addrC, Members[i].UID[j] );
			addrC++;
		}
	}
	// �g�J�Ǹ�
	addrC=0;
	for(i=0;i<MEMBER_MAX_NO;i++)
	{
		for(j=0;j<5;j++)
		{
			//soft_i2c_eeprom_write_byte(EEPROM_ADDR, EE_ADDR_STUDENT_ID+addrC, Members[i].SID[j] );
			addrC++;
		}
	}
}

void ReadMyMeterBoardID(void)
{	
		if ( PB2 )
		{
				MyMeterBoardID |= (0x01 << 0);
		}
		if ( PB3 )
		{
				MyMeterBoardID |= (0x01 << 1);
		}
		if ( PB4 )
		{
				MyMeterBoardID |= (0x01 << 2);
		}
		if ( PB5 )
		{
				MyMeterBoardID |= (0x01 << 3);
		}
		if ( PB6 )
		{
				MyMeterBoardID |= (0x01 << 4);
		}
		if ( PB7 )
		{
				MyMeterBoardID |= (0x01 << 5);
		}
#if 0
	MyMeterBoardID = 0x0A;
#endif 
}

void ResetHostUART(void)
{
	HOSTRxQ_cnt = 0 ; 
	HOSTRxQ_wp = 0 ;
	HOSTRxQ_rp = 0 ;	
	HOSTTxQ_cnt = 0 ; 
	HOSTTxQ_wp = 0 ;
	HOSTTxQ_rp = 0 ;	
}
void ResetReaderUART(void)
{
	READERRxQ_wp = 0 ; 
	READERRxQ_rp = 0 ;
	READERRxQ_cnt = 0 ;
	READERTxQ_wp = 0 ; 
	READERTxQ_rp = 0 ;
	READERTxQ_cnt = 0 ;
}
void ResetMeterUART(void)
{
	METERRxQ_wp = 0 ; 
	METERRxQ_rp = 0 ;
	METERRxQ_cnt = 0 ;
	METERTxQ_wp = 0 ; 
	METERTxQ_rp = 0 ;
	METERTxQ_cnt = 0 ;
}


/*
void ReadMeterPowerFromEE(void)
{
	uint32_t	U32tmpValue;
	uint8_t	U8tmpValue;
	
	U8tmpValue = soft_i2c_eeprom_read_byte(EEPROM_ADDR, EE_ADDR_METER_VALUE);
	U32tmpValue = (uint32_t) U8tmpValue << 24 ;
	U8tmpValue = soft_i2c_eeprom_read_byte(EEPROM_ADDR, EE_ADDR_METER_VALUE+1);
	U32tmpValue += (uint32_t) U8tmpValue << 16 ;
	U8tmpValue = soft_i2c_eeprom_read_byte(EEPROM_ADDR, EE_ADDR_METER_VALUE+2);
	U32tmpValue += (uint32_t) U8tmpValue << 8 ;
	U8tmpValue = soft_i2c_eeprom_read_byte(EEPROM_ADDR, EE_ADDR_METER_VALUE+3);
	U32tmpValue += (uint32_t) U8tmpValue  ;
	
	TotalWattValue_Now = U32tmpValue;
}

void SaveMeterPower2EE(void)
{	
	uint8_t	tmpValue; 	
	
	tmpValue = (uint8_t) (TotalWattValue_Now & 0xff000000) >> 24 ;
	soft_i2c_eeprom_write_byte(EEPROM_ADDR, EE_ADDR_METER_VALUE, tmpValue);	
	tmpValue = (uint8_t) (TotalWattValue_Now & 0x00ff0000) >> 16 ;
	soft_i2c_eeprom_write_byte(EEPROM_ADDR, EE_ADDR_METER_VALUE+1, tmpValue);
	tmpValue = (uint8_t) (TotalWattValue_Now & 0x0000FF00) >> 8 ;
	soft_i2c_eeprom_write_byte(EEPROM_ADDR, EE_ADDR_METER_VALUE+2, tmpValue);
	tmpValue = (uint8_t) (TotalWattValue_Now & 0x000000FF) ;
	soft_i2c_eeprom_write_byte(EEPROM_ADDR, EE_ADDR_METER_VALUE+3, tmpValue);
	
}

*/
#ifdef TEST_EEPROM
void TestEEPROM(void)
{
	uint8_t TestBuf[200];	
	uint16_t i;
	uint8_t Result;
	
	Result = 0 ;	
	for(i=0;i<200;i++)
	{
		TestBuf[i] = soft_i2c_eeprom_read_byte(EEPROM_ADDR, TEST_EE_ADDR+i);				
	}	
	for(i=0;i<200;i++)
	{
		TestBuf[i] = soft_i2c_eeprom_write_byte(EEPROM_ADDR,TEST_EE_ADDR+i, TEST_EE_ADDR+i);
	}	
	for(i=0;i<200;i++)
	{
		TestBuf[i] = soft_i2c_eeprom_read_byte(EEPROM_ADDR, TEST_EE_ADDR+i);				
	}	
	for(i=0;i<200;i++)
	{
		if ( TestBuf[i] != (TEST_EE_ADDR+i) )
		{
			Result = 1 ;			
		}
	}
	if ( Result )
	{
		Result = 0 ;
		LED_R_On();
	} else {
		LED_G_On();
	}	
	
}
#endif 
void ROOM_POWER_On(void)
{
	uint8_t i;

	
	MeterRelayStatus = RELAY_ON ;
	PF7 = 1;
	for ( i=0;i<50;i++)
		__NOP();
	PA3 = 1;
	for ( i=0;i<50;i++)
		__NOP();
		
}

void ROOM_POWER_Off(void)
{
/*	
	uint8_t i;
    
    if ( fgGotRoomMode )
    {
    
	MeterRelayStatus = RELAY_OFF;
	PF7 = 1;
	for ( i=0;i<50;i++)
		__NOP();
	PA3 = 0;
	for ( i=0;i<50;i++)
		__NOP();
    
    }
*/		
}

/***
 * @brief Polls various system devices sequentially.
 * @note The current synchronous polling approach leads to a long execution period.
 * Consider asynchronous or event-driven methods for performance optimization.
 ***/
void SystemPolling(void)
{
		if (TickUartChangeBaudrate > 40)
		{
				switch (SystemPollingState)
				{
						case SYSTEM_POLLING_READY:
								SystemPollingState = SYSTEM_POLLING_METER;
								break;

						case SYSTEM_POLLING_METER:
								if (PollingPwrMtrID > MaxPowerMeter)
								{
										PollingPwrMtrID = 1;
										MeterPollingFinishedFlag = TRUE;
								}

								if (!MeterPollingFinishedFlag)
								{
										MeterPolling();
								}
								else
								{
										MeterPollingFinishedFlag = FALSE;
										SystemPollingState = SYSTEM_POLLING_WM;
								}
								break;

						case SYSTEM_POLLING_WM:
								if (PollingWtrMtrID > WtrMtrMax)
								{
										PollingWtrMtrID = 1;
										WMPollingFinishedFlag = TRUE;
								}

								if (!WMPollingFinishedFlag)
								{
										WMPolling();
								}
								else
								{
										WMPollingFinishedFlag = FALSE;
										SystemPollingState = SYSTEM_POLLING_AirSensor;
								}
								break;

						case SYSTEM_POLLING_AirSensor:
								if (PollingAirSensorID > AirSensorMax)
								{
										PollingAirSensorID = 1;
										AirSensorPollingFinishedFlag = TRUE;
								}

								if (!AirSensorPollingFinishedFlag)
								{
										AirSensorPolling();
								}
								else
								{
										UART2_Init(2400);
										TickUartChangeBaudrate = 0;
										AirSensorPollingFinishedFlag = FALSE;
										SystemPollingState = SYSTEM_POLLING_PYR;
								}
								break;
								
						case SYSTEM_POLLING_PYR:
								if (PollingPyrMtrID > PyrMtrMax)
								{
										PollingPyrMtrID = 1;
										PyrPollingFinishedFlag = TRUE;
								}

								if (!PyrPollingFinishedFlag)
								{
										PyranometerPolling();
								}
								else
								{
										PyrPollingFinishedFlag = FALSE;
										SystemPollingState = SYSTEM_POLLING_SoilSensor;
								}
								break;								
						case SYSTEM_POLLING_SoilSensor:
								if (PollingSoilSensorID > SoilSensorMax)
								{
										PollingSoilSensorID = 1;
										SoilSensorPollingFinishedFlag = TRUE;
								}

								if (!SoilSensorPollingFinishedFlag)
								{
										SoilSensorPolling();
								}
								else
								{
										SoilSensorPollingFinishedFlag = FALSE;
										SystemPollingState = SYSTEM_POLLING_INV;
								}
								break;

						case SYSTEM_POLLING_INV:
								if (!INVPollingFinishedFlag)
								{
										INVPolling();
								}
								else
								{
										UART2_Init(115200);
										TickUartChangeBaudrate = 0;
										INVPollingFinishedFlag = FALSE;
										SystemPollingState = SYSTEM_POLLING_BMS;
								}
								break;

						case SYSTEM_POLLING_BMS:
								if (PollingBmsID > BmsMax)
								{
										PollingBmsID = 1;
										BmsPollingFinishedFlag = TRUE;
								}

								if (!BmsPollingFinishedFlag)
								{
										BmsPolling();
								}
								else
								{
										UART2_Init(9600);
										TickUartChangeBaudrate = 0;
										BmsPollingFinishedFlag = FALSE;
										SystemPollingState = SYSTEM_POLLING_READY;
								}
								break;

						default: // 處理所有未匹配到的情況
								SystemPollingState = SYSTEM_POLLING_READY;
								break;
				}
		}

}

/***
 *	@brief 	Passively	Process Data from Devices & distibute them according to Data Process State.
 *	@note		Need to know each device's address in advance, to recognize which device send the message.
 ***/
void ModbusDataProcess(void)
{
	
		if( TokenMeterReady )
		{
        TokenMeterReady = 0 ;        
        CRC16(TokenMeter, METER_RXQLen-2);
			
        if ( (TokenMeter[METER_RXQLen-2]==uchCRCHi) && (TokenMeter[METER_RXQLen-1]==uchCRCLo) )
        {		
						if ((TokenMeter[1]) > 0x80)
								return;
						
						LED_G_TOGGLE();
						GotDeviceRsp = TokenMeter[0] ;
					
						//	Dispute data according to Polling state
						if (SystemPollingState == SYSTEM_POLLING_METER) {
								MeterDataProcess();
						} else if (SystemPollingState == SYSTEM_POLLING_BMS) {
								BmsDataProcess();
						} else if (SystemPollingState == SYSTEM_POLLING_WM) {
								WMDataProcess();
						} else if (SystemPollingState == SYSTEM_POLLING_PYR) {
								PyrDataProcess();
						} else if (SystemPollingState == SYSTEM_POLLING_SoilSensor) {
								SoilSensorDataProcess();
						} else if (SystemPollingState == SYSTEM_POLLING_AirSensor) {
								AirSensorDataProcess();
						}
		
        } else if (SystemPollingState == SYSTEM_POLLING_INV){
						
						CRC16(&TokenMeter[5], METER_RXQLen-1);
						if (TokenMeter[METER_RXQLen-1]==uchCRCLo){
							
								LED_G_TOGGLE();
								GotDeviceRsp = TokenMeter[0] ;
								INVDataProcess();
						}
						

				} else {
            GotDeviceRsp = TokenMeter[0] ;
            ResetMeterUART();	
        }
			
		}
}

void InitializePollingIDs(void)
{
		//Setup PowerMeter Device
		PollingPwrMtrID = 0x01;
		PwrMtrError.PwrMtrDeviceNG = 0xFFFFFFFF;
		
		//Setup WaterMeter Device 
		PollingWtrMtrID = 0x01;
		WtrMtrError.WMDeviceNG = 0xFFFFFFFF;
	
		//Setup Pyranometer Device 
		PollingPyrMtrID =0x01;	
		PyrMtrError.PyrDeviceNG = 0xFFFFFFFF;
		
		//Setup SoilSensor Device 
		PollingSoilSensorID = 0x01;
		SoilSensorError.SSDeviceNG = 0xFFFFFFFF;
	
		//Setup AirSensor Device 
		PollingAirSensorID = 0x01;
		AirSensorError.ASDeviceNG = 0xFFFFFFFF;
	
		//Setup Bms Device 
		PollingBmsID =0x01;
		BmsError.BmsDeviceNG 	= 0xFFFFFFFF;

		InvError.InvDeviceNG 	= 0xFF;
//	Baud rate set to 9600
//		PyrNewBaudRate = 0x02;	//9600bmp
//		MODBUS_SendPYRCmd(MDBS_SET_BAUD_RATE);
//		SSNewBaudRate = 0x02;		//9600		bmp
//		MODBUS_SendSoilSensorCmd(MDBS_SET_SS_BAUD_RATE);
		
		UART2_Init(9600);
}

void Delay_10ms(uint8_t ms)
{
		u32TimeTick2 = 0;
		do {
				SystemTick = 0 ;		
		} while( u32TimeTick2 < ms ); 
		WDT_RESET_COUNTER();
}


//	For in a period of time watering, or time triggered watering
void WateringGPIO_OnOff(void)
{
		
}



/*** (C) COPYRIGHT 2022 AO Technology Corp. ***/
