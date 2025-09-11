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
uint8_t INVPollingState;
uint8_t INVPollingStateIndex;
uint8_t InvReadErrorCnt;
_Bool 	INVPollingFinishedFlag;

uint8_t PollingInvID;

void INVPolling(void)
{
		switch(INVPollingState)
		{	
				/***	Cmds will be Process in anytime, and it will @NOT substitute the upcomming polling cmd.	***/
				case	INV_POLLING_READY:
					
						INVPollingState = INV_POLLING_CMD + INVPollingStateIndex;
						
						if (INVPollingState > INV_POLLING_CMD)
						{
								PollingInvID = 1;
								INVPollingFinishedFlag = TRUE;
								INVPollingState = INV_POLLING_CMD;
								INVPollingStateIndex = 0;
						}
						break;
									
				case	INV_POLLING_RSP:
						//	If bufhead equal to 'E' or 'V', 
						if ((GotDeviceRsp == 'E') || (GotDeviceRsp == 'V'))
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

/***	Unique Inverter Cmd 
	 *	11 byte
***/
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
{		//	'E'+'P'+'H'+'B', 'V'+'G'+'H'+'B', 'V'+'P'+'H'+'B'
		if (((TokenMeter[0] == 0x45) && (TokenMeter[1] == 0x50) && (TokenMeter[2] == 0x48) && (TokenMeter[3] == 0x42)) || 
				((TokenMeter[0] == 0x56) && (TokenMeter[1] == 0x47) && (TokenMeter[2] == 0x48) && (TokenMeter[3] == 0x42)) ||
				((TokenMeter[0] == 0x56) && (TokenMeter[1] == 0x50) && (TokenMeter[2] == 0x48) && (TokenMeter[3] == 0x42)))
		{
				InvData.statusByte1 = TokenMeter[5];
				InvData.statusByte3 = TokenMeter[7];

				InvData.warnByte1   = TokenMeter[8];
				InvData.warnByte2   = TokenMeter[9];
			
				InvData.faultByte1  = TokenMeter[11];
				InvData.faultByte2  = TokenMeter[12];
				InvData.faultByte3  = TokenMeter[13];
			
				InvData.InputVolt		= TokenMeter[14] << 8 | TokenMeter[15];
				InvData.InputFreq		= TokenMeter[16] << 8 | TokenMeter[17];
				InvData.OutputVolt	= TokenMeter[18] << 8 | TokenMeter[19];
				InvData.OutputFreq	= TokenMeter[20] << 8 | TokenMeter[21];
				InvData.BatVolt			= TokenMeter[22] << 8 | TokenMeter[23];

				InvData.BatCapacity = TokenMeter[24];
				InvData.InvCurrent  = TokenMeter[25];
				InvData.LoadPercentage = TokenMeter[26];				
				InvData.MachineTemp = TokenMeter[27];
				InvData.MachineStatusCode  = TokenMeter[28];
				InvData.SysStatus 	= TokenMeter[32];			
				
				InvData.PV_volt = TokenMeter[36] << 8 | TokenMeter[37];				
				InvData.CtrlCurrent = TokenMeter[38];
				InvData.CtrlTemp  = TokenMeter[39];
				InvData.CtrlStatusCode 	= TokenMeter[40];		
		}
}

void INVSuccess(void)
{
		InvError.Success += 1;
		INVPollingStateIndex++;
		INVPollingState = INV_POLLING_READY;	
}

void INVTimeoutProcess(void)
{
		//	Error report system
		InvError.Fail += 1;
    InvError.InvDeviceNG &= (~(0x01 << (PollingInvID -1)));
	
    if ( TickPollingInterval > POLL_TIMEOUT )
    {
        INVPollingState = INV_POLLING_READY;						
        InvReadErrorCnt++;
        
				if( InvReadErrorCnt > MAX_POLL_RETRY_TIMES )
        {
						InvError.InvDeviceNG |= (0x01 << (PollingInvID -1));
            INVPollingStateIndex++;
            InvReadErrorCnt = 0 ;            
            ResetMeterUART();
        }
    }		
}