/***
	@file			AO_InverterProcess.c
	@brief		Use Modbus Protocol, control & communicate w/ BMS (Battery Management System), via RS-485
						
	@version	2025.08.14
	
	
	@UartSetup:	RS485, Baud: 2400 bps, Data Bit: 8 bits, STOP Bit: 1 bit, Parity Bit: None.
	@ProtocolFormat: 
	Address	|	1 BYTE
	Func		|	1 BYTE	
	Data		|	n BYTE
	CRC			| 2 BYTE
	
	BufHead 4BYTE
	
	#When host send incorrect CRC checksum, BMS will not respond
	#each register unit is 2 Byte wide.
	
	*@LastReviseDate 2025.08.15
	
 ***/
 
#include	"AO_InverterModbusProcess.h"
#include	"NUC1261.h"
#include	"AO_ExternFunc.h"
#include	"AO_BMSModbusProcess.h"

/*** 	Inverter variables	***/
InvError_t InvError;
InvData_t	 InvData;
BatData_t	 BatData;
CtlrData_t CtrlData;
uint8_t INVPollingState;
uint8_t INVPollingStateIndex;
uint8_t InvReadErrorCnt;
_Bool 	INVPollingFinishedFlag;

void INVPolling(void)
{
		switch(INVPollingState)
		{	
				/***	Cmds will be Process in anytime, and it will @NOT substitute the upcomming polling cmd.	***/
				case	INV_POLLING_READY:
					
						INVPollingState = INV_POLLING_CMD + INVPollingStateIndex;
						
						if (INVPollingState > INV_POLLING_CMD)
						{
								INVPollingFinishedFlag = TRUE;
								INVPollingState = INV_POLLING_CMD;
								INVPollingStateIndex = 0;
						}
						break;
									
				case	INV_POLLING_RSP:
						//	Get Rsp logic & Timeout Warn
						//	Change PowerMeterID to specific BMS ID
						if (GotDeviceRsp == PollingBmsID)
            {
                INVSuccess();
            } else {
                INVTimeoutProcess();
            }
						break;

				case 	INV_POLLING_CMD:
						MODBUS_SendINVCmd();
						INVPollingState = INV_POLLING_RSP;
						TickPollingInterval = 0;
						break;
		}
}

void MODBUS_SendINVCmd(void)
{
		MeterTxBuffer[0] = 0x43;
		MeterTxBuffer[1] = 0x4D;
		MeterTxBuffer[2] = 0x53;
		MeterTxBuffer[3] = 0x47;
		MeterTxBuffer[4] = 0x06;
		MeterTxBuffer[5] = 0x49;
		MeterTxBuffer[6] = 0x4E;
		MeterTxBuffer[7] = 0x56;
		MeterTxBuffer[8] = 0x2D;
		MeterTxBuffer[9] = 0x48;
		MeterTxBuffer[10] = 0x42;
		_SendStringToMETER(MeterTxBuffer,11); 
}

void INVDataProcess(void)
{		//	'E' 'P' 'H' 'B', 'V' 'G' 'H' 'B',
		if (((TokenMeter[0] == 0x45) && (TokenMeter[1] == 0x50) && (TokenMeter[2] == 0x48) && (TokenMeter[3] == 0x42)) || 
				((TokenMeter[0] == 0x56) && (TokenMeter[1] == 0x47) && (TokenMeter[2] == 0x48) && (TokenMeter[3] == 0x42)) ||
				((TokenMeter[0] == 0x56) && (TokenMeter[1] == 0x50) && (TokenMeter[2] == 0x48) && (TokenMeter[3] == 0x42)))
		{
				uint8_t statusByte1 = TokenMeter[5];
				uint8_t statusByte2 = TokenMeter[6];
				uint8_t statusByte3 = TokenMeter[7];
				uint8_t warnByte1   = TokenMeter[8];
				uint8_t warnByte2   = TokenMeter[9];
				uint8_t warnByte3   = TokenMeter[10];
				uint8_t faultByte1  = TokenMeter[11];
				uint8_t faultByte2  = TokenMeter[12];
				uint8_t faultByte3  = TokenMeter[13];
			
				CtrlData.ConnectFlag 	= (statusByte1 >> 3) & 0x01;
				CtrlData.ChargingFlag = (statusByte1 >> 2) & 0x01;				
				InvData.ChargingFlag  	=	(statusByte1 >> 1) & 0x01;
				BatData.Full  				=	(statusByte1 >> 0) & 0x01;
			
				CtrlData.FaultFlag 		= (statusByte3 >> 3) & 0x01;
				CtrlData.WarnFlag 		= (statusByte3 >> 2) & 0x01;				
				InvData.FaultFlag  		=	(statusByte3 >> 1) & 0x01;
				InvData.WarnFlag 			=	(statusByte3 >> 0) & 0x01;
			
				BatData.LoadWarnFlag 	= (warnByte1 >> 7) & 0x01;//
				BatData.TempWarnFlag 	= (warnByte1 >> 6) & 0x01;//
				BatData.LoadTimeoutWarnFlag =	(warnByte1 >> 5) & 0x01;
				BatData.LoadOverWarnFlag 		=	(warnByte1 >> 4) & 0x01;
				BatData.BatHighVoltWarnFlag = (warnByte1 >> 3) & 0x01;
				BatData.BatLowVoltWarnFlag 	= (warnByte1 >> 2) & 0x01;
				BatData.StoreDataErrWarnFlag 	=	(warnByte1 >> 1) & 0x01;
				BatData.StoreOpFailWarnFlag 	=	(warnByte1 >> 0) & 0x01;
				
				BatData.InvFuncErrWarnFlag 	 = (warnByte2 >> 2) & 0x01;
				BatData.PlanShutdownWarnFlag = (warnByte2 >> 1) & 0x01;
				BatData.OutputWarnFlag 			 = (warnByte2 >> 0) & 0x01;
				
				BatData.InvErrFaultFlag 		 = (faultByte1 >> 7) & 0x01;
				BatData.TempOverFaultFlag 	 = (faultByte1 >> 6) & 0x01;
				BatData.TempSensorFaultFlag  = (faultByte1 >> 5) & 0x01;
				BatData.LoadTimeoutFaultFlag = (faultByte1 >> 4) & 0x01;
				BatData.LoadErrFaultFlag 		 = (faultByte1 >> 3) & 0x01;
				BatData.LoadOverFaultFlag 	 = (faultByte1 >> 2) & 0x01;
				BatData.BatHighVoltFaultFlag = (faultByte1 >> 1) & 0x01;
				BatData.BatLowVoltFaultFlag  = (faultByte1 >> 0) & 0x01;

				BatData.PlanShutdownFaultFlag = (faultByte2 >> 1) & 0x01;
				BatData.OutputErrFaultFlag 	  = (faultByte2 >> 0) & 0x01;

				BatData.ChipStartFailFaultFlag 	= (faultByte3 >> 7) & 0x01;
				BatData.CurrentSensorFaultFlag 	= (faultByte3 >> 6) & 0x01;

		}
}

void INVSuccess(void)
{
		InvError.Success += 1;
	

		if (CmdStateFlag == TRUE){
				CmdStateFlag = FALSE;
				//	if cmd over, do polling proc. again ?				
				//BmsPollingStateIndex = 0;
		} else{
				INVPollingStateIndex++;
		}
		INVPollingState = INV_POLLING_READY;	
}

void INVTimeoutProcess(void)
{
		//	Error report system
		InvError.Fail += 1;
    //InvError.InvDeviceNG &= (~(0x00000001 << (PollingWMID -1)));
	
    if ( TickPollingInterval > POLL_TIMEOUT )
    {
        INVPollingState = INV_POLLING_READY;						
        InvReadErrorCnt++;
        
				if( InvReadErrorCnt > POLL_ERROR_TIMES )
        {
						//InvError.InvDeviceNG |= (0x00000001 << (PollingWMID -1));
            INVPollingStateIndex++;
            InvReadErrorCnt = 0 ;            
            ResetMeterUART();
        }
    }		
}