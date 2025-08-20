/***
	@file			AO_BMSModbusProcess.c
	@brief		Use Modbus Protocol, control & communicate w/ BMS (Battery Management System), via RS-485
						
						Monitoring battery sercurity. According to the following attributes:
						1:	Voltage.	Cell voltage & Battery voltage
						2:	Tmeperature.
						3:	Charging / Discharging status.
						
						Discharge Stops when :	Any cell run out of battery.
						charge Stops when :			Any cell's battery reaches full.
						
	@version	2025.08.14
	
	
	@UartSetup:	RS485, Baud(Seems to be locked, can't change): 115200bps, Data Bit: 8 bits, STOP Bit: 1 bit, Parity Bit: None.
	@ProtocolFormat: 
	Address	|	1 BYTE
	Func		|	1 BYTE	Read 0x03, Write 0x10
	Data		|	2 BYTE
	Reg	num	| 2 BYTE 
	CRC			| 2 BYTE
	
	#When host send incorrect CRC checksum, BMS will not respond
	#each register unit is 2 Byte wide.
	
	#	@TODO				Error report system	//	Half done, analyze the error rate of time scale. 
 ***/

#include	"AO_BMSModbusProcess.h"
#include	"NUC1261.h"
#include	"AO_ExternFunc.h"
#include	"AO_ModBusProcess.h"

/***	Functions	***/
void MODBUS_SendBMSCmd(uint8_t cmd);
void CmdModbus_BMS(uint8_t ModbusCmd);
void BmsPolling(void);
void BmsDataProcess(void);
void BmsSucccess(void);
void BmsTimeoutProcess(void);
void CheckBmsState(void);

/***	Variables	***/

BmsData_t  BmsData[BmsDeviceMax];
BmsError_t BmsError;

uint8_t PollingBmsID, MaxBmsDevices;
uint8_t ChargeOnOff, DishargeOnOff, BatteryBalanceOnOff, BmsPollingState;
uint8_t GotDeviceRsp;
uint8_t BmsPollingStateIndex;
uint8_t BmsReadErrorCnt;
uint16_t BmsMBCmd, ModeFlags;

_Bool CmdStateFlag = FALSE;
_Bool BmsPollingFinishedFlag;

/***	@note CellVoltage & temperature is necessary	***/
void MODBUS_SendBMSCmd(uint8_t cmd)
{
		//	BMS ID
		MeterTxBuffer[0]= PollingBmsID ; 
		GotDeviceRsp = 0xFF ;	
		//	Send BMS Cmd
		CmdModbus_BMS(cmd);
}


/***
 *	@function		BmsPolling()
 *	@brief			Poll BMS(Battery Management System) in sequence.

 *	@TODO				1.TimeOut Process
								2.Use BmsPolling to send Cmds to BMS Device

 *	Cell Voltage
 *	Battery Voltage
 *	Battery Current (maybe no need)
 *	Battery Watt
 *	Battery Status
 *	Battery	State of health
 *	Battery	State of charge
 *	Warning
 *	Mos temperature
 *	Battery	temperature (1-5)
 ***/ 

void BmsPolling(void)
{
	
		switch(BmsPollingState)
		{	
				/***	Cmds will be Process in anytime, and it will @NOT substitute the upcomming polling cmd.	***/
				case	BMS_POLLING_READY:
					
						BmsPollingState = BMS_POLLING_CELL_V_CMD + BmsPollingStateIndex;
						
						if (BmsPollingState > BMS_POLLING_BAT_TEMP5_CMD)
						{
								BmsPollingFinishedFlag = TRUE;
								PollingBmsID++;
								if (PollingBmsID > MaxBmsDevices)
								{
										PollingBmsID = 1 ;
								}
								BmsPollingState = BMS_POLLING_CELL_V_CMD;
								BmsPollingStateIndex = 0;
						}
						break;
									
				//	Get Rsp logic & Timeout Warning
				//	Change PowerMeterID to specific BMS ID
				case	BMS_POLLING_RSP:
						if (GotDeviceRsp == PollingBmsID)
            {
                BmsSucccess();
            } else {
                BmsTimeoutProcess();
            }
						break;
						
				case 	BMS_POLLING_CELL_STATUS:
						MODBUS_SendBMSCmd(MDBS_GET_CELL_STATUS);
						BmsPollingState = BMS_POLLING_RSP;
						TickPollingInterval = 0;
						break;						

				case 	BMS_POLLING_CELL_V_CMD:
						MODBUS_SendBMSCmd(MDBS_GET_CELL_V);
						BmsPollingState = BMS_POLLING_RSP;
						TickPollingInterval = 0;
						break;
				
				case	BMS_POLLING_BAT_W_CMD:
						MODBUS_SendBMSCmd(MDBS_GET_BATTERY_WATT);
						BmsPollingState = BMS_POLLING_RSP;
						TickPollingInterval = 0;				
						break;

				case	BMS_POLLING_BAT_V_CMD:
						MODBUS_SendBMSCmd(MDBS_GET_BATTERY_V);
						BmsPollingState = BMS_POLLING_RSP;
						TickPollingInterval = 0;
						break;
				
				case	BMS_POLLING_BAT_I_CMD:
						MODBUS_SendBMSCmd(MDBS_GET_BATTERY_I);
						BmsPollingState = BMS_POLLING_RSP;
						TickPollingInterval = 0;
						break;
				
				case	BMS_POLLING_BALANCE_STATUS_CMD:
						MODBUS_SendBMSCmd(MDBS_GET_SOC_STATUS);
						BmsPollingState = BMS_POLLING_RSP;
						TickPollingInterval = 0;				
						break;
				
				case	BMS_POLLING_BAT_SOH_CMD:
						MODBUS_SendBMSCmd(MDBS_GET_SOH);
						BmsPollingState = BMS_POLLING_RSP;
						TickPollingInterval = 0;				
						break;
				
				case	BMS_POLLING_CHARGING_STATUS_CMD:
						MODBUS_SendBMSCmd(MDBS_GET_CHARGE_DISCHARGE_STATUS);
						BmsPollingState = BMS_POLLING_RSP;
						TickPollingInterval = 0;				
						break;
				
				case	BMS_POLLING_WARNING_CMD:
						MODBUS_SendBMSCmd(MDBS_GET_BMS_ALARM);
						BmsPollingState = BMS_POLLING_RSP;
						TickPollingInterval = 0;				
						break;	

				case	BMS_POLLING_MOS_TEMP_CMD:
						MODBUS_SendBMSCmd(MDBS_GET_MOS_TEMP);
						BmsPollingState = BMS_POLLING_RSP;
						TickPollingInterval = 0;				
						break;
				
				case	BMS_POLLING_BAT_TEMP1_CMD:
						MODBUS_SendBMSCmd(MDBS_GET_BATTERY_TEMP1);
						BmsPollingState = BMS_POLLING_RSP;
						TickPollingInterval = 0;				
						break;
				
				case	BMS_POLLING_BAT_TEMP2_CMD:
						MODBUS_SendBMSCmd(MDBS_GET_BATTERY_TEMP2);
						BmsPollingState = BMS_POLLING_RSP;
						TickPollingInterval = 0;				
						break;
				
				case	BMS_POLLING_BAT_TEMP3_CMD:
						MODBUS_SendBMSCmd(MDBS_GET_BATTERY_TEMP3);
						BmsPollingState = BMS_POLLING_RSP;
						TickPollingInterval = 0;				
						break;
				
				case	BMS_POLLING_BAT_TEMP4_CMD:
						MODBUS_SendBMSCmd(MDBS_GET_BATTERY_TEMP4);
						BmsPollingState = BMS_POLLING_RSP;
						TickPollingInterval = 0;				
						break;				
				
				case	BMS_POLLING_BAT_TEMP5_CMD:
						MODBUS_SendBMSCmd(MDBS_GET_BATTERY_TEMP5);
						BmsPollingState = BMS_POLLING_RSP;
						TickPollingInterval = 0;				
						break;
				
				
				//	Cmds
				case	MBCMD_SET_BMS_CHARGE_STATUS:
						MODBUS_SendBMSCmd(MDBS_SET_BATTERY_CHARGEING_STATUS);
						BmsPollingState = BMS_POLLING_RSP;
						CmdStateFlag = true;
						TickPollingInterval = 0;				
						break;
				
				case	MBCMD_SET_BMS_DISCHARGE_STATUS:
						MODBUS_SendBMSCmd(MDBS_SET_BATTERY_DISCHARGING_STATUS);
						BmsPollingState = BMS_POLLING_RSP;
						CmdStateFlag = true;
						TickPollingInterval = 0;				
						break;
				
				case	MBCMD_SET_BMS_STATUS:
						MODBUS_SendBMSCmd(MDBS_SET_BATTERY_BALANCE_STATUS);
						BmsPollingState = BMS_POLLING_RSP;
						CmdStateFlag = true;
						TickPollingInterval = 0;				
						break;
				
				case	MBCMD_SET_BMS_MODES:
						MODBUS_SendBMSCmd(MDBS_SET_MODES);
						BmsPollingState = BMS_POLLING_RSP;
						CmdStateFlag = true;
						TickPollingInterval = 0;				
						break;	

				//	Set when meter initializing
				case	MBCMD_SET_BMS_ADDR:
						MODBUS_SendBMSCmd(MDBS_SET_BMS_DEVICE_ADDR);
						BmsPollingState = BMS_POLLING_RSP;
						CmdStateFlag = true;
						TickPollingInterval = 0;				
						break;				
		}
}


/***
 *	@brief	for Meter read / control BMS(Battery management system)
 ***/
void CmdModbus_BMS(uint8_t ModbusCmd)
{
	
		switch(ModbusCmd)
		{
				//	Wirte				
				case	MDBS_SET_BMS_DEVICE_ADDR:
						//	Battery charge on/ off
						BmsMBCmd = MDBS_SET_BMS_DEVICE_ADDR;
						MeterTxBuffer[1] = 0x10;
						MeterTxBuffer[2] = 0x11;
						MeterTxBuffer[3] = 0x08;
						MeterTxBuffer[4] = 0x00;
						MeterTxBuffer[5] = 0x02;
						MeterTxBuffer[6] = 0x04;
						MeterTxBuffer[7] = 0x00;
						MeterTxBuffer[8] = 0x00;
						MeterTxBuffer[9] = 0x00;				
						MeterTxBuffer[10] = PollingBmsID;		//	BMS Device ID
						CRC16(MeterTxBuffer, 11);
						MeterTxBuffer[11]=uchCRCHi;
						MeterTxBuffer[12]=uchCRCLo;
						_SendStringToMETER(MeterTxBuffer,13);   			
						break;
				
				case	MDBS_SET_MODES:
						//	Set modes
						BmsMBCmd = MDBS_SET_MODES;
						MeterTxBuffer[1] = 0x10;
						MeterTxBuffer[2] = 0x11;
						MeterTxBuffer[3] = 0x14;
						MeterTxBuffer[4] = 0x00;
						MeterTxBuffer[5] = 0x02;
						MeterTxBuffer[6] = 0x04;
						MeterTxBuffer[7] = 0x00;
						MeterTxBuffer[8] = 0x00;
						MeterTxBuffer[9] = ModeFlags << 8;
						MeterTxBuffer[10] = ModeFlags;
						CRC16(MeterTxBuffer, 11);
						MeterTxBuffer[11]=uchCRCHi;
						MeterTxBuffer[12]=uchCRCLo;
						_SendStringToMETER(MeterTxBuffer,13);   			
						break;	
				
				
				case	MDBS_SET_BATTERY_CHARGEING_STATUS:
						//	Battery charge on/ off
						BmsMBCmd = MDBS_SET_BATTERY_CHARGEING_STATUS;
						MeterTxBuffer[1] = 0x10;
						MeterTxBuffer[2] = 0x10;
						MeterTxBuffer[3] = 0x70;
						MeterTxBuffer[4] = 0x00;
						MeterTxBuffer[5] = 0x02;
						MeterTxBuffer[6] = 0x04;
						MeterTxBuffer[7] = 0x00;
						MeterTxBuffer[8] = 0x00;
						MeterTxBuffer[9] = 0x00;
						MeterTxBuffer[10] = ChargeOnOff;		// 00: off / 01: on
						CRC16(MeterTxBuffer, 11);
						MeterTxBuffer[11]=uchCRCHi;
						MeterTxBuffer[12]=uchCRCLo;
						_SendStringToMETER(MeterTxBuffer,13);   			
						break;
				
				case	MDBS_SET_BATTERY_DISCHARGING_STATUS:
						//	Battery discharge on/ off
						BmsMBCmd = MDBS_SET_BATTERY_DISCHARGING_STATUS;
						MeterTxBuffer[1] = 0x10;
						MeterTxBuffer[2] = 0x10;
						MeterTxBuffer[3] = 0x74;
						MeterTxBuffer[4] = 0x00;
						MeterTxBuffer[5] = 0x02;
						MeterTxBuffer[6] = 0x04;
						MeterTxBuffer[7] = 0x00;
						MeterTxBuffer[8] = 0x00;
						MeterTxBuffer[9] = 0x00;
						MeterTxBuffer[10] = DishargeOnOff;	// 00: off / 01: on
						CRC16(MeterTxBuffer, 11);
						MeterTxBuffer[11]=uchCRCHi;
						MeterTxBuffer[12]=uchCRCLo;
						_SendStringToMETER(MeterTxBuffer,13);   			
						break;

				case	MDBS_SET_BATTERY_BALANCE_STATUS:
						//	Battery balance on/ off
						BmsMBCmd = MDBS_SET_BATTERY_BALANCE_STATUS;
						MeterTxBuffer[1] = 0x10;
						MeterTxBuffer[2] = 0x10;
						MeterTxBuffer[3] = 0x78;
						MeterTxBuffer[4] = 0x00;
						MeterTxBuffer[5] = 0x02;
						MeterTxBuffer[6] = 0x04;
						MeterTxBuffer[7] = 0x00;
						MeterTxBuffer[8] = 0x00;
						MeterTxBuffer[9] = 0x00;
						MeterTxBuffer[10] = BatteryBalanceOnOff;	// 00: off / 01: on
						CRC16(MeterTxBuffer, 11);
						MeterTxBuffer[11]=uchCRCHi;
						MeterTxBuffer[12]=uchCRCLo;
						_SendStringToMETER(MeterTxBuffer,13);   			
						break;				
				
				//	Raed
				case MDBS_GET_CELL_V:
						BmsMBCmd = MDBS_GET_CELL_V;				//	to Process BMS Cmd use
						MeterTxBuffer[1] = 0x03;									//	Read
						MeterTxBuffer[2] = 0x12;									//	Reg num 0xHH
						MeterTxBuffer[3] = 0x00;									//	Reg num 0xLL
						MeterTxBuffer[4] = 0x00;									//	Reg adr 0xHH
						MeterTxBuffer[5] = 0x10;									//	Reg adr 0xLL
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6]=uchCRCHi;								// CRC Checksum 
						MeterTxBuffer[7]=uchCRCLo;								// CRC Checksum
						_SendStringToMETER(MeterTxBuffer,8);			//	max MeterTxBuffer length = 25;
						break;

				case MDBS_GET_CELL_STATUS:
						BmsMBCmd = MDBS_GET_CELL_STATUS;
						MeterTxBuffer[1] = 0x03;
						MeterTxBuffer[2] = 0x12;
						MeterTxBuffer[3] = 0x40;
						MeterTxBuffer[4] = 0x00;
						MeterTxBuffer[5] = 0x02;
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6]=uchCRCHi;
						MeterTxBuffer[7]=uchCRCLo;
						_SendStringToMETER(MeterTxBuffer,8);   			
						break;

				case MDBS_GET_MOS_TEMP:
						BmsMBCmd = MDBS_GET_MOS_TEMP;
						MeterTxBuffer[1] = 0x03;
						MeterTxBuffer[2] = 0x12;
						MeterTxBuffer[3] = 0x8A;
						MeterTxBuffer[4] = 0x00;
						MeterTxBuffer[5] = 0x01;
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6]=uchCRCHi;
						MeterTxBuffer[7]=uchCRCLo;
						_SendStringToMETER(MeterTxBuffer,8);   			
						break;


				case MDBS_GET_BATTERY_V:
						BmsMBCmd = MDBS_GET_BATTERY_V;
						MeterTxBuffer[1] = 0x03;
						MeterTxBuffer[2] = 0x12;
						MeterTxBuffer[3] = 0x90;
						MeterTxBuffer[4] = 0x00;
						MeterTxBuffer[5] = 0x02;
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6]=uchCRCHi;
						MeterTxBuffer[7]=uchCRCLo;
						_SendStringToMETER(MeterTxBuffer,8);   			
						break;

				case MDBS_GET_BATTERY_WATT:
						BmsMBCmd = MDBS_GET_BATTERY_WATT;
						MeterTxBuffer[1] = 0x03;
						MeterTxBuffer[2] = 0x12;
						MeterTxBuffer[3] = 0x94;
						MeterTxBuffer[4] = 0x00;
						MeterTxBuffer[5] = 0x02;
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6]=uchCRCHi;
						MeterTxBuffer[7]=uchCRCLo;
						_SendStringToMETER(MeterTxBuffer,8);   			
						break;

				case MDBS_GET_BATTERY_I:
						BmsMBCmd = MDBS_GET_BATTERY_WATT;
						MeterTxBuffer[1] = 0x03;
						MeterTxBuffer[2] = 0x12;
						MeterTxBuffer[3] = 0x98;
						MeterTxBuffer[4] = 0x00;
						MeterTxBuffer[5] = 0x02;
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6]=uchCRCHi;
						MeterTxBuffer[7]=uchCRCLo;
						_SendStringToMETER(MeterTxBuffer,8);   			
						break;

				case MDBS_GET_BATTERY_TEMP1:
						BmsMBCmd = MDBS_GET_BATTERY_TEMP1;
						MeterTxBuffer[1] = 0x03;
						MeterTxBuffer[2] = 0x12;
						MeterTxBuffer[3] = 0x9C;
						MeterTxBuffer[4] = 0x00;
						MeterTxBuffer[5] = 0x01;
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6]=uchCRCHi;
						MeterTxBuffer[7]=uchCRCLo;
						_SendStringToMETER(MeterTxBuffer,8);   			
						break;

				case MDBS_GET_BATTERY_TEMP2:
						BmsMBCmd = MDBS_GET_BATTERY_TEMP2;
						MeterTxBuffer[1] = 0x03;
						MeterTxBuffer[2] = 0x12;
						MeterTxBuffer[3] = 0x9E;		
						MeterTxBuffer[4] = 0x00;		
						MeterTxBuffer[5] = 0x01;	
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6]=uchCRCHi;      
						MeterTxBuffer[7]=uchCRCLo;      
						_SendStringToMETER(MeterTxBuffer,8);   			
						break;

				case MDBS_GET_BATTERY_TEMP3:
						BmsMBCmd = MDBS_GET_BATTERY_TEMP3;
						MeterTxBuffer[1] = 0x03;
						MeterTxBuffer[2] = 0x12;
						MeterTxBuffer[3] = 0xF8;		
						MeterTxBuffer[4] = 0x00;		
						MeterTxBuffer[5] = 0x01;	
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6]=uchCRCHi;      
						MeterTxBuffer[7]=uchCRCLo;      
						_SendStringToMETER(MeterTxBuffer,8);   			
						break;

				case MDBS_GET_BATTERY_TEMP4:
						BmsMBCmd = MDBS_GET_BATTERY_TEMP4;
						MeterTxBuffer[1] = 0x03;
						MeterTxBuffer[2] = 0x12;
						MeterTxBuffer[3] = 0xFA;		
						MeterTxBuffer[4] = 0x00;		
						MeterTxBuffer[5] = 0x01;	
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6]=uchCRCHi;      
						MeterTxBuffer[7]=uchCRCLo;      
						_SendStringToMETER(MeterTxBuffer,8);   			
						break;

				case MDBS_GET_BATTERY_TEMP5:
						BmsMBCmd = MDBS_GET_BATTERY_TEMP5;
						MeterTxBuffer[1] = 0x03;
						MeterTxBuffer[2] = 0x12;
						MeterTxBuffer[3] = 0xFC;
						MeterTxBuffer[4] = 0x00;		
						MeterTxBuffer[5] = 0x01;	
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6]=uchCRCHi;      
						MeterTxBuffer[7]=uchCRCLo;      
						_SendStringToMETER(MeterTxBuffer,8);   			
						break;

				case MDBS_GET_BMS_ALARM:
						BmsMBCmd = MDBS_GET_BMS_ALARM;
						MeterTxBuffer[1] = 0x03;
						MeterTxBuffer[2] = 0x12;
						MeterTxBuffer[3] = 0xA0;
						MeterTxBuffer[4] = 0x00;
						MeterTxBuffer[5] = 0x02;
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6]=uchCRCHi;
						MeterTxBuffer[7]=uchCRCLo;
						_SendStringToMETER(MeterTxBuffer,8);   			
						break;

				case MDBS_GET_BALANCE_CURRENT:
						BmsMBCmd = MDBS_GET_BALANCE_CURRENT;	
						MeterTxBuffer[1] = 0x03;
						MeterTxBuffer[2] = 0x12;
						MeterTxBuffer[3] = 0xA4;
						MeterTxBuffer[4] = 0x00;
						MeterTxBuffer[5] = 0x01;
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6]=uchCRCHi;
						MeterTxBuffer[7]=uchCRCLo;
						_SendStringToMETER(MeterTxBuffer,8);   			
						break;

				case MDBS_GET_SOC_STATUS:
						BmsMBCmd = MDBS_GET_SOC_STATUS;
						MeterTxBuffer[1] = 0x03;
						MeterTxBuffer[2] = 0x12;				
						MeterTxBuffer[3] = 0xA6;				
						MeterTxBuffer[4] = 0x00;				
						MeterTxBuffer[5] = 0x01;				
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6]=uchCRCHi;      
						MeterTxBuffer[7]=uchCRCLo;      
						_SendStringToMETER(MeterTxBuffer,8);   			
						break;

				case MDBS_GET_SOC_STATUS_REMAIN:
						BmsMBCmd = MDBS_GET_SOC_STATUS_REMAIN;
						MeterTxBuffer[1] = 0x03;
						MeterTxBuffer[2] = 0x12;				
						MeterTxBuffer[3] = 0xA8;				
						MeterTxBuffer[4] = 0x00;				
						MeterTxBuffer[5] = 0x02;				
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6]=uchCRCHi;      
						MeterTxBuffer[7]=uchCRCLo;      
						_SendStringToMETER(MeterTxBuffer,8);   			
						break;

				case	MDBS_GET_SOC_STATUS_CAPACITY:
						BmsMBCmd = MDBS_GET_SOC_STATUS_CAPACITY;
						MeterTxBuffer[1] = 0x03;
						MeterTxBuffer[2] = 0x12;		
						MeterTxBuffer[3] = 0xAC;		
						MeterTxBuffer[4] = 0x00;		
						MeterTxBuffer[5] = 0x02;		
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6]=uchCRCHi;  
						MeterTxBuffer[7]=uchCRCLo;  
						_SendStringToMETER(MeterTxBuffer,8);   			
						break;

				case	MDBS_GET_SOC_STATUS_CYCLE_COUNT:
						BmsMBCmd = MDBS_GET_SOC_STATUS_CYCLE_COUNT;
						MeterTxBuffer[1] = 0x03;
						MeterTxBuffer[2] = 0x12;
						MeterTxBuffer[3] = 0xB0;
						MeterTxBuffer[4] = 0x00;
						MeterTxBuffer[5] = 0x02;
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6]=uchCRCHi;
						MeterTxBuffer[7]=uchCRCLo;
						_SendStringToMETER(MeterTxBuffer,8);   			
						break;
				
				case	MDBS_GET_SOH:
						BmsMBCmd = MDBS_GET_SOH;
						MeterTxBuffer[1] = 0x03;
						MeterTxBuffer[2] = 0x12;
						MeterTxBuffer[3] = 0xB8;
						MeterTxBuffer[4] = 0x00;
						MeterTxBuffer[5] = 0x01;
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6]=uchCRCHi;
						MeterTxBuffer[7]=uchCRCLo;
						_SendStringToMETER(MeterTxBuffer,8);   			
						break;				
				
				case	MDBS_GET_CHARGE_DISCHARGE_STATUS:
						BmsMBCmd = MDBS_GET_CHARGE_DISCHARGE_STATUS;
						MeterTxBuffer[1] = 0x03;
						MeterTxBuffer[2] = 0x12;
						MeterTxBuffer[3] = 0xC0;
						MeterTxBuffer[4] = 0x00;
						MeterTxBuffer[5] = 0x01;
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6]=uchCRCHi;
						MeterTxBuffer[7]=uchCRCLo;
						_SendStringToMETER(MeterTxBuffer,8);   			
						break;		
				default:
					break;

		}
}

uint16_t CellCmdList[16];


/***
 *	@brief	No need to do Data Process, only need to store & send Data to Center.
 *	Check if mode matches Current mode.
 ***/
void BmsDataProcess(void)
{
		uint32_t u32temp;
		uint8_t BmsArrayIndex;
		
		if (GotDeviceRsp != PollingBmsID) return;	//	check the host address idx
		
		BmsArrayIndex = GotDeviceRsp -1;
	
		switch (BmsMBCmd)
		{
				case	MDBS_GET_CELL_V:
						
						for (uint8_t i = 0; i < 16; i++){
								BmsData[GotDeviceRsp].CellVolt[i] = TokenMeter[(i*2)+3] << 8 | TokenMeter[(i*2)+4];
						}
						break;

				//	The maximum use of Cell number => 16
				case	MDBS_GET_CELL_STATUS:
						BmsData[BmsArrayIndex].CellStatus = (TokenMeter[3] >> 24 | TokenMeter[4] >> 16 | 
																	TokenMeter[5] >> 8 | TokenMeter[6]);
						break;

				
				case	MDBS_GET_BATTERY_WATT:
						BmsData[BmsArrayIndex].BatWatt = (TokenMeter[3] >> 24 | TokenMeter[4] >> 16 | 
																	TokenMeter[5] >> 8 | TokenMeter[6]);
						break;				
				
				case	MDBS_GET_BATTERY_V:
						BmsData[BmsArrayIndex].BatVolt = (TokenMeter[3] >> 24 | TokenMeter[4] >> 16 | 
																TokenMeter[5] >> 8 | TokenMeter[6]);
						break;

				case	MDBS_GET_BATTERY_I:
						BmsData[BmsArrayIndex].BatCurrent = (TokenMeter[3] >> 24 | TokenMeter[4] >> 16 | 
																	TokenMeter[5] >> 8 | TokenMeter[6]);
						break;				
				
				case	MDBS_GET_MOS_TEMP:
						BmsData[BmsArrayIndex].MosTemp = (TokenMeter[3] >> 8 | TokenMeter[4]);
						break;

				case 	MDBS_GET_BATTERY_TEMP1:
						BmsData[BmsArrayIndex].BatteryTemp[0] = (TokenMeter[3] >> 8 | TokenMeter[4]);
						break;

				case	MDBS_GET_BATTERY_TEMP2:
						BmsData[BmsArrayIndex].BatteryTemp[1] = (TokenMeter[3] >> 8 | TokenMeter[4]);
						break;

				case 	MDBS_GET_BATTERY_TEMP3:
						BmsData[BmsArrayIndex].BatteryTemp[2] = (TokenMeter[3] >> 8 | TokenMeter[4]);
						break;

				case	MDBS_GET_BATTERY_TEMP4:
						BmsData[BmsArrayIndex].BatteryTemp[3] = (TokenMeter[3] >> 8 | TokenMeter[4]);
						break;

				case	MDBS_GET_BATTERY_TEMP5:
						BmsData[BmsArrayIndex].BatteryTemp[4] = (TokenMeter[3] >> 8 | TokenMeter[4]);
						break;
				
				case	MDBS_GET_SOC_STATUS:
						BmsData[BmsArrayIndex].BalanceStatus	 = TokenMeter[3];			//	2: Discharge; 1: Charge; 0: Close;
						BmsData[BmsArrayIndex].StateOfCharge  = TokenMeter[4];
						break;
				
				case	MDBS_GET_SOH:
						//	if battery health lower than 80%, need to switch the battery set
						BmsData[BmsArrayIndex].StateOfHealth  = TokenMeter[3];
						BmsData[BmsArrayIndex].PrechargeStatus  = TokenMeter[4];		//	1: Opend; 0: Closed;
						break;
				
				case	MDBS_GET_CHARGE_DISCHARGE_STATUS:
						BmsData[BmsArrayIndex].ChargeState = TokenMeter[3];				//	1: Opend; 0: Closed;
						BmsData[BmsArrayIndex].DischargeState = TokenMeter[4];			//	1: Opend; 0: Closed;
		}
}

/***
 *	@brief	Handles BMS RSP and guide polling state 
 ***/
void BmsSucccess(void)
{
		uint8_t BmsArrayIndex;
		BmsArrayIndex = GotDeviceRsp -1;
	
		BmsError.Success[BmsArrayIndex] += 1;
		BmsError.BmsDeviceNG &= (~(0x00000001 << (PollingBmsID -1)));
		if (CmdStateFlag == TRUE){
				CmdStateFlag = FALSE;
		}
		BmsPollingStateIndex++;
		BmsPollingState = BMS_POLLING_READY;	
}

void BmsTimeoutProcess(void)
{
		uint8_t BmsArrayIndex;
		BmsArrayIndex = GotDeviceRsp -1;
		//	Error report system
		BmsError.Fail[BmsArrayIndex] += 1;
    
    if ( TickPollingInterval > POLL_TIMEOUT )
    {
        BmsPollingState = BMS_POLLING_READY;							
        BmsReadErrorCnt++;
        
				if( BmsReadErrorCnt > POLL_ERROR_TIMES )
        {
						BmsError.BmsDeviceNG |= (0x00000001 << (PollingBmsID -1));
            BmsPollingStateIndex++;
            BmsReadErrorCnt = 0 ;            
            ResetMeterUART();
        }
    }
}


/***
 *	@brief	Check if MeterData is equal to Bms device's status.
 *	@Attributes	Charge, Discharge, Balance Modes.
 ***/
void CheckBmsState(void)
{
		
}