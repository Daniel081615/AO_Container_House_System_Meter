/***
	@file			AO_WaterMeterProcess.c
	@brief		Use Modbus Protocol, control & communicate w/ BMS (Battery Management System), via RS-485
						
						LSB

						
	@version	2025.08.14
	
	
	@UartSetup:	RS485, Baud: 9600/ 4800/ 2400/ 1200 bps, Data Bit: 8 bits, STOP Bit: 1 bit, Parity Bit: 1.
	@ProtocolFormat: 
	Address	|	1 BYTE
	Func		|	1 BYTE	
	Data		|	n BYTE
	CRC			| 2 BYTE
	
	@Valve 
	func code:	
		Valve control 		: 0x05
		Read valve status	: 0x01
		Write	reg					: 0x10
		Read	reg					: 0x03
	addr : 0x0001
	
	#When host send incorrect CRC checksum, BMS will not respond
	#each register unit is 2 Byte wide.
	
	 *	@note		need to add Next Device Polling Decision. @Date 2025.08.14
	
 ***/

#include	"AO_WaterMeterModbusProcess.h"
#include	"AO_ExternFunc.h"
#include	"AO_ModBusProcess.h"
#include	"AO_BMSModbusProcess.h"
#include	"NUC1261.h"

/*** 	Water Meter Functions	***/
void WMPolling(void);
void WMDataProcess(void);
void CmdModbus_WM(uint8_t ModbusCmd);
void MODBUS_SendWMCmd(uint8_t cmd);
void WMSuccess(void);
void WMTimeoutProcess(void);
void CheckWMState(void);

void WtrMeter_Init(void);
_Bool ScannSetWTRMeterAddr(int baudrate);

/*** 	Water Meter Variables	***/
WMError_t WMError;
WMData_t WMData[WtrMeterMax];
uint8_t PollingWMID, WMSetDeviceID;
uint8_t WMPollingState, WMPollingStateIndex;
uint8_t WMMBCMD;
uint8_t VlaveOnOff;
uint8_t	WMBaudRate = 0x03;		// 0x01: 9600bps, 0x02: 4800bps, 0x03: 2400bps, 0x04: 1200bps
uint8_t WMReadErrorCnt;
uint8_t u8tempStatus;
uint8_t tries;

uint32_t u32tempStatus;
_Bool WMPollingFinishedFlag;

/***	init Setup Cmd	
	WMPollingState = MBCMD_SET_WM_ADDR_AND_BAUDRATE;
***/

void WMPolling(void)
{
		switch(WMPollingState)
		{	
			
				case	WM_POLLING_READY:
						if (TickReadPowerTime >= ReadMeterTime)
            {
								TickReadPowerTime = 0 ;
								WMPollingState = WM_POLLING_WATER_CONSUMPTION_CMD + WMPollingStateIndex;
								
								if (WMPollingState > WM_POLLING_VALVE_STATUS_CMD)
								{
										WMPollingState = WM_POLLING_WATER_CONSUMPTION_CMD;
										WMPollingStateIndex = 0;
										PollingWMID++;
								}
						} else {
								WtrMtrModbusCmd =  WtrMeterCmdList[PollingWMID-1];
								WtrMeterCmdList[PollingWMID-1] = MBWMCMD_READY ;
							
								switch ( WtrMtrModbusCmd )	
                {
                    case	MBWMCMD_READY :
                        break;
                    case	MBWMCMD_VALVE_OPEN :
                        VlaveOnOff = 1 ;
                        WMPollingState = MBCMD_SET_WM_VALVE_OnOff ;
                        PwrMtrModbusCmd = MBWMCMD_READY;
                        break;
                    case	MBWMCMD_VALVE_CLOSE :										
                        VlaveOnOff = 0 ;
                        WMPollingState = MBCMD_SET_WM_VALVE_OnOff ;
                        PwrMtrModbusCmd = MBWMCMD_READY;
                        break;
                    default :
                        break;
                }	
						}
						break;
				
				case	WM_POLLING_RSP:
						//	Get Rsp logic & Timeout Warn
						//	Change PowerMeterID to specific BMS ID
						if (GotDeviceRsp == PollingWMID)
            {
								WMSuccess();
            } else {
								WMTimeoutProcess();
            }
						break;
						
				case	WM_POLLING_WATER_CONSUMPTION_CMD:
						MODBUS_SendWMCmd(MDBS_GET_TOTAL_WATER_CONSUMPTION);
						WMPollingState = WM_POLLING_RSP;
						TickPollingInterval = 0;
						break;
				
				case	WM_POLLING_VALVE_STATUS_CMD:
						MODBUS_SendWMCmd(MDBS_GET_VALVE_STATUS);
						WMPollingState = WM_POLLING_RSP;
						TickPollingInterval = 0;
						break;
				
				case	MBCMD_SET_WM_ADDR_AND_BAUDRATE:
						WMSetDeviceID = 0x01;		//#	When needed to set
						MODBUS_SendWMCmd(MDBS_SET_WM_DEVICE_ADDR_AND_BAUDRATE);
						WMPollingState = WM_POLLING_RSP;
						TickPollingInterval = 0;
						break;
				
				case	MBCMD_SET_WM_VALVE_OnOff:
						MODBUS_SendWMCmd(MDBS_SET_VALVE_OnOff);
						WMPollingState = WM_POLLING_RSP;
						TickPollingInterval = 0;
						break;				
		}
}

void WMDataProcess(void)
{
		uint8_t WMArrayIndex;
		//	Read Valve, register ststus respond function code
		if (GotDeviceRsp != PollingWMID) return;	//	check the host address idx
	
		WMArrayIndex = GotDeviceRsp -1;
	
		switch(WMMBCMD)
		{
				case MDBS_GET_VALVE_STATUS:
						u8tempStatus = TokenMeter[5];	// 0xff Open, 0x00 Close
						if (WMData[GotDeviceRsp].ValveState != u8tempStatus)
						{
							 if (u8tempStatus == 0xff){
										VlaveOnOff = 0x01;
							 } else if (u8tempStatus == 0x00){
										VlaveOnOff = 0x00;
							 }
								WMPollingState = MBCMD_SET_WM_VALVE_OnOff;//	Set Valve to temp status.
								WMData[GotDeviceRsp].ValveState = u8tempStatus;
						}
						break;
						
				case MDBS_GET_TOTAL_WATER_CONSUMPTION:
						u32tempStatus = (TokenMeter[3] << 24) + (TokenMeter[4] << 16) + (TokenMeter[5] << 8) + (TokenMeter[6]);
						//	If total water consumption lower than before, then check three times
						if (u32tempStatus < WMData[GotDeviceRsp].TotalVolume)
						{
								tries++;
								if (tries > 3)
										WMData[GotDeviceRsp].TotalVolume = u32tempStatus;
						} else 
								WMData[GotDeviceRsp].TotalVolume = u32tempStatus;
						break;
						
				default:
					break;
		}
}


void MODBUS_SendWMCmd(uint8_t cmd)
{
		MeterTxBuffer[0] = PollingWMID ; 
		//	To determine if Device respond.
		GotDeviceRsp = 0xFF ;
		CmdModbus_WM(cmd);
}

void CmdModbus_WM(uint8_t ModbusCmd)
{
		switch(ModbusCmd)
		{
				
				case	MDBS_GET_VALVE_STATUS:
						WMMBCMD = MDBS_GET_VALVE_STATUS;
						MeterTxBuffer[1] = 0x01;	//Read Valve
						MeterTxBuffer[2] = 0x00;
						MeterTxBuffer[3] = 0x01;
						MeterTxBuffer[4] = 0x00;
						MeterTxBuffer[5] = 0x01;
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6]=uchCRCHi;
						MeterTxBuffer[7]=uchCRCLo;
						_SendStringToMETER(MeterTxBuffer,8);   			
						break;
				
				case	MDBS_GET_TOTAL_WATER_CONSUMPTION:
						WMMBCMD = MDBS_GET_TOTAL_WATER_CONSUMPTION;
						MeterTxBuffer[1] = 0x03;	//Read register
						MeterTxBuffer[2] = 0x00;
						MeterTxBuffer[3] = 0x00;
						MeterTxBuffer[4] = 0x00;
						MeterTxBuffer[5] = 0x02;
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6]=uchCRCHi;
						MeterTxBuffer[7]=uchCRCLo;
						_SendStringToMETER(MeterTxBuffer,8);   			
						break;				

				case	MDBS_SET_VALVE_OnOff:
						WMMBCMD = MDBS_SET_VALVE_OnOff;
						MeterTxBuffer[1] = 0x05;	//Set vslve
						MeterTxBuffer[2] = 0x00;
						MeterTxBuffer[3] = 0x01;
						MeterTxBuffer[4] = 0x00;
						if (VlaveOnOff){
								MeterTxBuffer[5] = 0xFF;
						} else {
								MeterTxBuffer[5] = 0x00;
						}
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6]=uchCRCHi;
						MeterTxBuffer[7]=uchCRCLo;
						_SendStringToMETER(MeterTxBuffer,8);   			
						break;
				
				case	MDBS_SET_WM_DEVICE_ADDR_AND_BAUDRATE:
						WMMBCMD = MDBS_SET_WM_DEVICE_ADDR_AND_BAUDRATE;
						MeterTxBuffer[1] = 0x10;	//Write multiple register
						MeterTxBuffer[2] = 0x00;
						MeterTxBuffer[3] = 0x15;
						MeterTxBuffer[4] = 0x00;
						MeterTxBuffer[5] = 0x01;	//Write	Reg amount
						MeterTxBuffer[6] = 0x02;	//Write byte
						MeterTxBuffer[7] = WMSetDeviceID;
						MeterTxBuffer[8] = WMBaudRate;
						CRC16(MeterTxBuffer, 9);
						MeterTxBuffer[9]=uchCRCHi;
						MeterTxBuffer[10]=uchCRCLo;
						_SendStringToMETER(MeterTxBuffer,11);   			
						break;	
		}
}
/***
 *	@brief	Handles Device Rsps.
 *	@note		need to add Next Device Polling Decision. @Date 2025.08.14
 ***/
void WMSuccess(void)
{
		uint8_t WMArrayIndex;
		WMArrayIndex = GotDeviceRsp -1;

		WMError.Success[WMArrayIndex] += 1;
		WMError.WMDeviceNG &= (~(0x00000001 << (PollingWMID -1)));
		//	Move to next BMSpollingstate
    WMPollingStateIndex++;
		WMPollingState = WM_POLLING_READY;			
}

void WMTimeoutProcess(void)
{
		uint8_t WMArrayIndex;
		WMArrayIndex = GotDeviceRsp -1;	
	
    if ( TickPollingInterval > POLL_TIMEOUT )
    {							
        WMReadErrorCnt++;
				//	Error report system    
				WMError.Fail[WMArrayIndex] += 1;
				WMPollingState = WM_POLLING_READY;
        if( WMReadErrorCnt > POLL_ERROR_TIMES )
        {
						WMError.WMDeviceNG |= (0x00000001 << (PollingWMID -1));
            WMPollingStateIndex++;
            WMReadErrorCnt = 0 ;            
            ResetMeterUART();
        }
    }
}

void WtrMeter_Init(void)
{
    const int baudrates[] = {9600, 4800, 2400, 1200};
    const int numBaudrates = sizeof(baudrates) / sizeof(baudrates[0]);
		
    for (int i = 0; i < numBaudrates; i++)
    {
        if (ScannSetWTRMeterAddr(baudrates[i]))
				{
						//	Success flag
				} else {
						//	Fail flag
				}
    }
		
}


_Bool ScannSetWTRMeterAddr(int baudrate)
{
    UART2_Init(baudrate);
	
    for (uint8_t i = 1; i < 247; i++)
    {
        PollingWMID = i;
				WMSetDeviceID = 0x01;
				WMBaudRate= 0x03;			// 2400 baud
        MODBUS_SendWMCmd(MDBS_SET_WM_DEVICE_ADDR_AND_BAUDRATE);
			
				Delay_10ms(20);
			
        if (TokenMeterReady != 0x00)
        {
            PollingMeterID = 1;
						TokenMeterReady = 0x00;
            return 1;
        }
    }

    return 0;
}