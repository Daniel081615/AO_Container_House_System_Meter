/***
 *	@file		Pyranometer.c
 *	@breif	Pyranometer modbus cmds, polling modules
 ***/
 
#include	"AO_Pyranometer.h"
#include 	"NUC1261.h"
#include	"AO_ExternFunc.h"
#include	"AO_ModBusProcess.h"

void PyranometerPolling(void);
void PyrDataProcess(void);
void MODBUS_SendPYRCmd(uint8_t cmd);
void CmdModbus_PYR_SN300AL(uint8_t cmd);
void PyrMtrSuccess(void);
void PyrMtrTimeoutProcess(void);

PyrMtrError_t PyrMtrError;
PyrMtrData_t PyrMtrData[PyrMtrMax];

_Bool 		PyrPollingFinishedFlag;
uint8_t 	PollingPyrMtrID;
volatile uint8_t 	PyrMtrMBCMD, PyrMtrModbusCmd;
uint8_t 	PyrPollingState, PyrPollingStateIndex;
uint8_t 	PyrReadErrorCnt;
uint16_t 	NewDeviationValue, PyrNewAddr, PyrNewBaudRate;

/***
 *	@function		PyranometerPolling()
 *	@brief			polling Pyranometer in sequence. and record the communication.
 *	@note				Build host structure.	1. cmd Pyranometer and set param constant.
 *	2025.09.01
 ***/
void PyranometerPolling(void)
{
		switch(PyrPollingState)	
		{
				case	PYR_POLLING_READY:
						if (TickReadDeviceTime >= ReadDeviceCmdTime)
						{
								TickReadDeviceTime = 0 ;	
								PyrPollingState = PYR_GET_SOLAR_RADIATION + PyrPollingStateIndex;
								
								if (PyrPollingState > PYR_GET_DEVIATION_VALUE)
								{
										PollingPyrMtrID++;
										PyrPollingState = PYR_GET_SOLAR_RADIATION;
										PyrPollingStateIndex = 0;
								}
						} else {
								PyrMtrModbusCmd = PyrMtrCmdList[PollingPyrMtrID-1] ;
								PyrMtrCmdList[PollingPyrMtrID-1] = MBPYRCMD_READY;

								switch ( PyrMtrModbusCmd )	
								{
										case MBPYRCMD_READY :
											
											break;
										
										case MBPYRCMD_SET_DEVIATION_VALUE :
												PyrPollingState = PYR_SET_DEVIATION_VALUE;
												PyrMtrModbusCmd = MBPYRCMD_READY;
											break;
										
										case MBPYRCMD_SET_DEVICE_ADDRESS :
												PyrPollingState = PYR_SET_DEVICE_ADDRESS;
												PyrMtrModbusCmd = MBPYRCMD_READY;												
											break;
										
										case MBPYRCMD_SET_BAUD_RATE : 
												PyrPollingState = PYR_SET_BAUD_RATE;
												PyrMtrModbusCmd = MBPYRCMD_READY;
											break;
								}
						}
						break;

				case	PYR_POLLING_RSP:
            if (GotDeviceRsp == PyrMtrIDArray[PollingPyrMtrID-1])
            {
                PyrMtrSuccess();
            } else {
                PyrMtrTimeoutProcess();
            }
					break;
				
				case	PYR_GET_SOLAR_RADIATION:
						MODBUS_SendPYRCmd(MDBS_GET_SOLAR_RADIATION);
						PyrPollingState = PYR_POLLING_RSP;
						TickPollingInterval = 0 ;			
					break;

				case	PYR_GET_DEVIATION_VALUE:
						MODBUS_SendPYRCmd(MDBS_GET_DEVIATION_VALUE);
						PyrPollingState = PYR_POLLING_RSP;
						TickPollingInterval = 0 ;		
					break;
				
				case	PYR_SET_DEVIATION_VALUE:
						MODBUS_SendPYRCmd(MDBS_SET_DEVIATION_VALUE);
						PyrPollingState = PYR_POLLING_READY;
					break;

				case	PYR_SET_DEVICE_ADDRESS:
						MODBUS_SendPYRCmd(MDBS_SET_DEVICE_ADDRESS);
						PyrPollingState = PYR_POLLING_READY;
					break;
				
				case	PYR_SET_BAUD_RATE:
						MODBUS_SendPYRCmd(MDBS_SET_BAUD_RATE);
						PyrPollingState = PYR_POLLING_READY;					
					break;
		}
}

/***
 *	@brief			Parse Modbus massage from Pyranometer
 *	@ParseData	Solar Radiation(W/m^2), Offset value(W/m^2)
 ***/
void PyrDataProcess(void)
{
		uint8_t PyrMtrArrayIdx;
		uint16_t u16tmp;
		
	
		if (GotDeviceRsp != PyrMtrIDArray[PollingPyrMtrID-1]) return;	//	check the host address idx
	
		PyrMtrArrayIdx = PollingPyrMtrID-1;
	
		switch(PyrMtrMBCMD)	//	PyrMtrMBCMD Value Error, it will automatically increase...
		{
				case MDBS_GET_SOLAR_RADIATION:
						u16tmp = ((TokenMeter[3] << 8) | TokenMeter[4]);
						PyrMtrData[PyrMtrArrayIdx].SolarRadiation = u16tmp;
						break;
						
				case MDBS_GET_DEVIATION_VALUE:
						u16tmp = ((TokenMeter[3] << 8) | TokenMeter[4]);
						PyrMtrData[PyrMtrArrayIdx].OffsetValue = u16tmp;
						break;
				default:
					break;
		}
}		

/***
 *	@brief	Send pyranometer modbus cmd
 ***/
void MODBUS_SendPYRCmd(uint8_t cmd)
{
		MeterTxBuffer[0] = PyrMtrIDArray[PollingPyrMtrID-1] ; 
		//	To determine if Device respond.
		GotDeviceRsp = 0xFF ;
		CmdModbus_PYR_SN300AL(cmd);		
}

/***
 *	@brief	Send pyranometer modbus cmd
 *	@Read		Solar radiation, Offset value, Device addr, Baud rate
 *	@Set		Offset value, Device addr, Baud rate
 ***/
void CmdModbus_PYR_SN300AL(uint8_t ModbusCmd)
{
		switch(ModbusCmd)
		{
				
				case MDBS_GET_SOLAR_RADIATION:
						PyrMtrMBCMD = MDBS_GET_SOLAR_RADIATION;
						MeterTxBuffer[1] = 0x03;
						MeterTxBuffer[2] = 0x00;
						MeterTxBuffer[3] = 0x00;
						MeterTxBuffer[4] = 0x00;
						MeterTxBuffer[5] = 0x01;
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;
				
				case MDBS_GET_DEVIATION_VALUE:
						PyrMtrMBCMD = MDBS_GET_DEVIATION_VALUE;
						MeterTxBuffer[1] = 0x03;
						MeterTxBuffer[2] = 0x00;
						MeterTxBuffer[3] = 0x52;
						MeterTxBuffer[4] = 0x00;
						MeterTxBuffer[5] = 0x01;
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;		

				case MDBS_SET_DEVIATION_VALUE:
						PyrMtrMBCMD = MDBS_PYRANO_OTHER;
						MeterTxBuffer[1] = 0x06;
						MeterTxBuffer[2] = 0x00;
						MeterTxBuffer[3] = 0x52;
						MeterTxBuffer[4] = (NewDeviationValue >> 8) & 0xff;
						MeterTxBuffer[5] = NewDeviationValue & 0xff;
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;					

				case MDBS_GET_DEVICE_ADDRESS:
						PyrMtrMBCMD = MDBS_PYRANO_OTHER;
						MeterTxBuffer[1] = 0x03; 
						MeterTxBuffer[2] = 0x07; 
						MeterTxBuffer[3] = 0xD0; 
						MeterTxBuffer[4] = 0x00; 
						MeterTxBuffer[5] = 0x01; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;

				case MDBS_SET_DEVICE_ADDRESS:
						PyrMtrMBCMD = MDBS_PYRANO_OTHER;
						MeterTxBuffer[1] = 0x06; 
						MeterTxBuffer[2] = 0x07; 
						MeterTxBuffer[3] = 0xD0;
						MeterTxBuffer[4] = (PyrNewAddr >> 8) & 0xff; 
						MeterTxBuffer[5] = PyrNewAddr & 0xff; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;
				
				case MDBS_GET_BAUD_RATE:
						PyrMtrMBCMD = MDBS_PYRANO_OTHER;
						MeterTxBuffer[1] = 0x03; 
						MeterTxBuffer[2] = 0x07; 
						MeterTxBuffer[3] = 0xD1; 
						MeterTxBuffer[4] = 0x00; 
						MeterTxBuffer[5] = 0x01; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;
				
				case MDBS_SET_BAUD_RATE:
						PyrMtrMBCMD = MDBS_PYRANO_OTHER;
						MeterTxBuffer[1] = 0x06; 
						MeterTxBuffer[2] = 0x07; 
						MeterTxBuffer[3] = 0xD1;
						//	baud rate: 0x00 -> 2400 bmp, 0x01 -> 4800 bmp, 0x02 -> 9600 bmp
						MeterTxBuffer[4] = (PyrNewBaudRate >> 8) & 0xff; 
						MeterTxBuffer[5] = PyrNewBaudRate & 0xff; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;
				
				default :
						PyrMtrMBCMD = MDBS_PYRANO_OTHER;
						break;
		}
}

void PyrMtrSuccess(void)
{
		uint8_t PyrArrayIndex;
		PyrArrayIndex = PollingPyrMtrID -1;

		PyrMtrError.Success[PyrArrayIndex] += 1;
		PyrMtrError.PyrDeviceNG &= (~(0x00000001 << (PyrArrayIndex)));
		//	Move to next pollingstate
    PyrPollingStateIndex++;
		PyrPollingState = PYR_POLLING_READY;			
}

void PyrMtrTimeoutProcess(void)
{
		uint8_t PyrArrayIndex;
		PyrArrayIndex = PollingPyrMtrID -1;	
	
    if ( TickPollingInterval > POLL_TIMEOUT )
    {							
        PyrReadErrorCnt++;
				//	Error report system    
				PyrMtrError.Fail[PyrArrayIndex] += 1;
				PyrPollingState = PYR_POLLING_READY;
        if( PyrReadErrorCnt > MAX_POLL_RETRY_TIMES )
        {
						PyrMtrError.PyrDeviceNG |= (0x00000001 << (PyrArrayIndex));
            PollingPyrMtrID++;
            PyrReadErrorCnt = 0 ;            
            ResetMeterUART();
        }
    }
}
