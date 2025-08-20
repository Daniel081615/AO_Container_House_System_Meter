/*--------------------------------------------------------------------------- 
 * File Name     : MODBusProcess.C
 * Description   : RS485 Comucation with 阿拉斯加熱交機
 * Company 		 : AOTECH
 * Author Name   : Barry Su
 * Starting Date : 2015.7.1
 * C Compiler : Keil C
 * --------------------------------------------------------------------------*/

/*********************************************************************************************************/
#include <stdio.h>
#include "NUC1261.h"
#include "AO2022_MeterModule_1261.h"
#include "AO_MyDef.h"
#include "AO_ExternFunc.h"
#include "AO_ModBusProcess.h"
#include "AO_MyDef.h"
#include "AO_ExternFunc.h"
#include "AO_MeterProcess.h"

void BAW2A_GetWatt(void);
void BAW2A_GetRelayStatus(void);
void BAW2A_GetSetRelay(void);
void BAW2A_GetBalance(void);
//#include "My.h"


/* Table of CRC values for high–order byte */
const unsigned char auchCRCHi[] = {
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40
};

/* Table of CRC values for low–order byte */
const char auchCRCLo[] = {
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
0x40
};

uint8_t uchCRCHi = 0xFF;  /* high byte of CRC initialized */
uint8_t uchCRCLo = 0xFF;  /* low byte of CRC initialized */

uint8_t bReadPowerMeterValue,ModBusStep,ModBusCommand;
uint8_t PowerMeterBaudRate,RelayOnOff;
uint8_t MeterMBCmd;
uint8_t PayMode,u8Year,u8Month,u8Day,u8Hour,u8Min,u8Sec,u8Week;

void MODBUS_CmdProcess(void);
void MODBUS_SendCmd(uint8_t cmd);
void DigitalMeterRelay(uint8_t fnOnOff);
void CmdModBus_DEM5x0(uint8_t ModBusCmd);
void CmdModBus_BAW2A(uint8_t ModBusCmd);
void CmdModBus_E21nE31(uint8_t ModBusCmd);
void CmdModBus_DEM_510c(uint8_t ModBusCommand);
void DigitMeterPowerON(void);
void DigitMeterPowerOFF(void);

void BAW2A_GetCardID(void);
void BAW2A_GetWatt(void);
void BAW2A_GetBalance(void);
void BAW2A_GetRelayStatus(void);
void BAW2A_SetRelay(void);
void BAW2A_SetAddr(void);
void BAW2A_SetBuadrate(void);
void BAW2A_SetPassword(void);
void BAW2A_AddBalance(void);
void BAW2A_ExitTestMode(void);

void BAW2A_Get_V(void);
void BAW2A_Get_I(void);
void BAW2A_Get_F(void);
void BAW2A_Get_P(void);
void BAW2A_Get_S(void);
void BAW2A_Get_PF(void);
void BAW2A_SetPayMode(void);
void BAW2A_SetPowerOn(void);
void BAW2A_SetTime(void);
void BAW2A_GetPayMode(void);
void BAW2A_GetStatus(void);
void BAW2A_GetCardID(void);

void CRC16(unsigned char *puchMsg,unsigned short usDataLen);


void DigitMeterPowerON(void)
{
	ModBusCommand = MBCMD_RELAY_ON ;	
}
void DigitMeterPowerOFF(void)
{	
	ModBusCommand = MBCMD_RELAY_OFF ;		
}

/*******************************************************************************
* Function Name  : MODBUS_SendCmd
* Description : Send command to RS485 
* Input   : None
* Output  : None
* Return  : None
*******************************************************************************/
void MODBUS_SendCmd(uint8_t cmd)
{
	
		//DIR485_METER_Out() ;
		PowerMeterID = PollingMeterID ;
		MeterTxBuffer[0]= PowerMeterID ; // Node ID
		GotMeterRsp = 0xFF ;
		MeterErrorRate5Min_Tx[PowerMeterID-1][MeterErrorRate5Min_Wp]++; 
	
		switch ( MeterType )
		{
			case DAE_530n540 :
				// 台科電 DAE DEM530/540
				CmdModBus_DEM5x0(cmd);
				break;
			case CIC_BAW2A :
				// 巧力 BAW-2A
				CmdModBus_BAW2A(cmd);
				break;
			case TUTANG_E21nE31 :
				// 大同 E21/E31
				CmdModBus_E21nE31(cmd);
				break;
			case DEM_510c:
				break;
		}	    
}

// DAE DEM510
void CmdModBus_DEM5x0(uint8_t ModBusCommand)
{
	// 010300000003
	switch ( ModBusCommand )
	{		
		case MDBS_READY :
			ModBusStep = 0 ;
			break;
		case MDBS_METER_GET_WATT :
			MeterMBCmd = MDBS_METER_GET_WATT ;
			bReadPowerMeterValue = 1 ;						
			MeterTxBuffer[1]=0x03;          // Function 
			MeterTxBuffer[2]=0x00;          // Data 0
			MeterTxBuffer[3]=0x00;          // Data 1
			MeterTxBuffer[4]=0x00;          // Data 2
			MeterTxBuffer[5]=0x02;          // Data 3
			CRC16(MeterTxBuffer,6);
			MeterTxBuffer[6]=uchCRCHi;      // // CRC Checksum 
			MeterTxBuffer[7]=uchCRCLo;      // // CRC Checksum 
			_SendStringToMETER(MeterTxBuffer,8);   			
			break;
		case MDBS_METER_GET_RELAY :
			MeterMBCmd = MDBS_METER_GET_RELAY ;
			bReadPowerMeterValue = 1 ;			
			MeterTxBuffer[1]=0x03;          // Function 
			MeterTxBuffer[2]=0x00;          // Data 0
			MeterTxBuffer[3]=0x03;          // Data 1
			MeterTxBuffer[4]=0x00;          // Data 2
			MeterTxBuffer[5]=0x01;          // Data 3
			CRC16(MeterTxBuffer,6);
			MeterTxBuffer[6]=uchCRCHi;      // // CRC Checksum 
			MeterTxBuffer[7]=uchCRCLo;      // // CRC Checksum 
			_SendStringToMETER(MeterTxBuffer,8);   					
			break;
		case MDBS_METER_SET_RELAY : 
			MeterMBCmd = MDBS_METER_OTHER ;
			MeterTxBuffer[1]=0x05;          // Function 
			MeterTxBuffer[2]=0x00;          // Data 0
			MeterTxBuffer[3]=0x30;          // Data 1
			MeterTxBuffer[4]=0x00;          // Data 2
			MeterTxBuffer[5]=0x00;          // Data 3      
			CRC16(MeterTxBuffer,6);
			MeterTxBuffer[6]=uchCRCHi;      // // CRC Checksum 
			MeterTxBuffer[7]=uchCRCLo;      // // CRC Checksum 
			_SendStringToMETER(MeterTxBuffer,8);   
			break;		
		case MDBS_METER_SET_ADDR : 
			MeterMBCmd = MDBS_METER_OTHER ;
			MeterTxBuffer[1]=0x10;          // Function 
			MeterTxBuffer[2]=0x00;          // Data 0
			MeterTxBuffer[3]=0x30;          // Data 1
			MeterTxBuffer[4]=0x00;          // Data 2
			MeterTxBuffer[5]=0x01;          // Data 3 
			MeterTxBuffer[6]=0x02;          // Data 3 
			MeterTxBuffer[7]=PowerMeterNewAddr;          // Data 3 
			MeterTxBuffer[8]=0x00;          // Data 3 
			CRC16(MeterTxBuffer,9);
			MeterTxBuffer[9]=uchCRCHi;      // // CRC Checksum 
			MeterTxBuffer[10]=uchCRCLo;      // // CRC Checksum 
			_SendStringToMETER(MeterTxBuffer,11);   
			break;
		case MDBS_METER_SET_BAUDRATE : 
			MeterMBCmd = MDBS_METER_OTHER ;
			MeterTxBuffer[1]=0x05;          // Function 
			MeterTxBuffer[2]=0x00;          // Data 0
			MeterTxBuffer[3]=0x30;          // Data 1
			MeterTxBuffer[4]=0xFF;          // Data 2
			MeterTxBuffer[5]=0x00;          // Data 3            
			CRC16(MeterTxBuffer,6);
			MeterTxBuffer[6]=uchCRCHi;      // // CRC Checksum 
			MeterTxBuffer[7]=uchCRCLo;      // // CRC Checksum 
			_SendStringToMETER(MeterTxBuffer,8);   
			break;
		case MDBS_METER_SET_PWD :
			MeterMBCmd = MDBS_METER_OTHER ;
			MeterTxBuffer[1]=0x05;          // Function 
			MeterTxBuffer[2]=0x00;          // Data 0
			MeterTxBuffer[3]=0x30;          // Data 1
			MeterTxBuffer[4]=0xFF;          // Data 2
			MeterTxBuffer[5]=0x00;          // Data 3            
			CRC16(MeterTxBuffer,6);
			MeterTxBuffer[6]=uchCRCHi;      // // CRC Checksum 
			MeterTxBuffer[7]=uchCRCLo;      // // CRC Checksum 
			_SendStringToMETER(MeterTxBuffer,8);   
			break;
		case MDBS_METER_SET_EXIT :
			MeterMBCmd = MDBS_METER_OTHER ;
			MeterTxBuffer[1]=0x05;          // Function 
			MeterTxBuffer[2]=0x00;          // Data 0
			MeterTxBuffer[3]=0x30;          // Data 1
			MeterTxBuffer[4]=0xFF;          // Data 2
			MeterTxBuffer[5]=0x00;          // Data 3            
			CRC16(MeterTxBuffer,6);
			MeterTxBuffer[6]=uchCRCHi;      // // CRC Checksum 
			MeterTxBuffer[7]=uchCRCLo;      // // CRC Checksum 
			_SendStringToMETER(MeterTxBuffer,8);   
			break;
		default :
			MeterMBCmd = MDBS_METER_OTHER ;
			break;
	}
}
void BAW2A_GetWatt(void)
{
    MeterMBCmd = MDBS_METER_GET_WATT ;
    bReadPowerMeterValue = 1 ;			
    MeterTxBuffer[1]=0x03;          // Function 
    MeterTxBuffer[2]=0x00;          // Data 0
    MeterTxBuffer[3]=0x06;          // Data 1
    MeterTxBuffer[4]=0x00;          // Data 2
    MeterTxBuffer[5]=0x02;          // Data 3
    CRC16(MeterTxBuffer,6);
    MeterTxBuffer[6]=uchCRCHi;      // // CRC Checksum 
    MeterTxBuffer[7]=uchCRCLo;      // // CRC Checksum 
    _SendStringToMETER(MeterTxBuffer,8);   					
}
void BAW2A_GetBalance(void)
{
    MeterMBCmd = MDBS_METER_GET_BAL ;
    bReadPowerMeterValue = 1 ;			
    MeterTxBuffer[1]=0x03;          // Function 
    MeterTxBuffer[2]=0x00;          // Data 0
    MeterTxBuffer[3]=0x0A;          // Data 1
    MeterTxBuffer[4]=0x00;          // Data 2
    MeterTxBuffer[5]=0x02;          // Data 3
    CRC16(MeterTxBuffer,6);
    MeterTxBuffer[6]=uchCRCHi;      // // CRC Checksum 
    MeterTxBuffer[7]=uchCRCLo;      // // CRC Checksum 
    _SendStringToMETER(MeterTxBuffer,8);   			
}
void BAW2A_GetRelayStatus(void)
{

    MeterMBCmd = MDBS_METER_GET_RELAY ;
    bReadPowerMeterValue = 1 ;			
    MeterTxBuffer[1]=0x03;          // Function 
    MeterTxBuffer[2]=0x00;          // Data 0
    MeterTxBuffer[3]=0x02;          // Data 1
    MeterTxBuffer[4]=0x00;          // Data 2
    MeterTxBuffer[5]=0x01;          // Data 3
    CRC16(MeterTxBuffer,6);
    MeterTxBuffer[6]=uchCRCHi;      // // CRC Checksum 
    MeterTxBuffer[7]=uchCRCLo;      // // CRC Checksum 
    _SendStringToMETER(MeterTxBuffer,8);   				

}

void BAW2A_SetRelay(void)
{
    MeterMBCmd = MDBS_METER_OTHER ;
    MeterTxBuffer[1]=0x10;          // Function 
    MeterTxBuffer[2]=0x00;          // Data 0
    MeterTxBuffer[3]=0xD2;          // Data 1
    MeterTxBuffer[4]=0x00;          // Data 2
    MeterTxBuffer[5]=0x01;          // Data 3 
    MeterTxBuffer[6]=0x02;          // Data 3 
    MeterTxBuffer[7]=0x00;          // Data 3 
    if ( RelayOnOff )
    {
    MeterTxBuffer[8]=0x01;          // Data 3 
    } else {
    MeterTxBuffer[8]=0x02;          // Data 3 
    }
    CRC16(MeterTxBuffer,9);
    MeterTxBuffer[9]=uchCRCHi;      // // CRC Checksum 
    MeterTxBuffer[10]=uchCRCLo;      // // CRC Checksum 
    _SendStringToMETER(MeterTxBuffer,11);   
}
void BAW2A_SetAddr(void)
{
    MeterMBCmd = MDBS_METER_OTHER ;
    MeterTxBuffer[1]=0x10;          // Function 
    MeterTxBuffer[2]=0x00;          // Data 0
    MeterTxBuffer[3]=0x0E;          // Data 1
    MeterTxBuffer[4]=0x00;          // Data 2
    MeterTxBuffer[5]=0x01;          // Data 3 
    MeterTxBuffer[6]=0x02;          // Data 3 
    MeterTxBuffer[7]=PowerMeterNewAddr;          // Data 3 
    MeterTxBuffer[8]=0x00;          // Data 3 
    CRC16(MeterTxBuffer,9);
    MeterTxBuffer[9]=uchCRCHi;      // // CRC Checksum 
    MeterTxBuffer[10]=uchCRCLo;      // // CRC Checksum 
    _SendStringToMETER(MeterTxBuffer,11);   
}
void BAW2A_SetBuadrate(void)
{
    MeterMBCmd = MDBS_METER_OTHER ;
    MeterTxBuffer[1]=0x10;          // Function 
    MeterTxBuffer[2]=0x00;          // Data 0
    MeterTxBuffer[3]=0xD3;          // Data 1
    MeterTxBuffer[4]=0x00;          // Data 2
    MeterTxBuffer[5]=0x01;          // Data 3    
    MeterTxBuffer[6]=0x02;          // Data 3 
    MeterTxBuffer[7]=0x00;          // Data 3 
    MeterTxBuffer[8]=PowerMeterBaudRate;          // Data 3 
    CRC16(MeterTxBuffer,9);
    MeterTxBuffer[9]=uchCRCHi;      // // CRC Checksum 
    MeterTxBuffer[10]=uchCRCLo;      // // CRC Checksum 
    _SendStringToMETER(MeterTxBuffer,11);  
}
void BAW2A_SetPassword(void)
{
    MeterMBCmd = MDBS_METER_OTHER ;
    MeterTxBuffer[1]=0x16;          // Function 
    MeterTxBuffer[2]=0x08;          // Data 0
    MeterTxBuffer[3]=0x08;          // Data 1
    MeterTxBuffer[4]=0x00;          // Data 2
    MeterTxBuffer[5]=0x02;          // Data 3          
    MeterTxBuffer[6]=0x04;          // Data 3 			
    MeterTxBuffer[7]=0x1D;          // Data 3 
    MeterTxBuffer[8]=0x2C;          // Data 3 
    MeterTxBuffer[9]=0x3B;          // Data 3 
    MeterTxBuffer[10]=0x4A;          // Data 3 		
    CRC16(MeterTxBuffer,11);
    MeterTxBuffer[11]=uchCRCHi;      // // CRC Checksum 
    MeterTxBuffer[12]=uchCRCLo;      // // CRC Checksum 
    _SendStringToMETER(MeterTxBuffer,13);
}
void BAW2A_AddBalance(void)
{
    MeterMBCmd = MDBS_METER_OTHER ;
    MeterTxBuffer[1]=0x10;          // Function 
    MeterTxBuffer[2]=0x00;          // Data 0
    MeterTxBuffer[3]=0xD4;          // Data 1
    MeterTxBuffer[4]=0x00;          // Data 2
    MeterTxBuffer[5]=0x02;          // Data 3    
    MeterTxBuffer[6]=0x04;          // Data 3 
    // Add 50000.00
    MeterTxBuffer[7]=0x33;          // Data 3 	( 實際加值為減 -0x33 = 0x00)
    MeterTxBuffer[8]=0x7F;          // Data 3  ( 實際加值為減 -0x33 = 0x4C)
    MeterTxBuffer[9]=0x7E;          // Data 3  ( 實際加值為減 -0x33 = 0x4B)
    MeterTxBuffer[10]=0x73;        // Data 3 	( 實際加值為減 -0x33 = 0x40)
    CRC16(MeterTxBuffer,11);
    MeterTxBuffer[11]=uchCRCHi;      // // CRC Checksum 
    MeterTxBuffer[12]=uchCRCLo;      // // CRC Checksum 
    _SendStringToMETER(MeterTxBuffer,13);   
}
void BAW2A_ExitTestMode(void)
{
    MeterMBCmd = MDBS_METER_OTHER ;
    MeterTxBuffer[1]=0x10;          // Function 
    MeterTxBuffer[2]=0x00;          // Data 0
    MeterTxBuffer[3]=0x49;          // Data 1
    MeterTxBuffer[4]=0x00;          // Data 2
    MeterTxBuffer[5]=0x01;          // Data 3    
    MeterTxBuffer[6]=0x02;          // Data 3 
    MeterTxBuffer[7]=0x00;          // Data 3 
    MeterTxBuffer[8]=0x00;          // Data 3 
    CRC16(MeterTxBuffer,9);
    MeterTxBuffer[9]=uchCRCHi;      // // CRC Checksum 
    MeterTxBuffer[10]=uchCRCLo;      // // CRC Checksum 
    _SendStringToMETER(MeterTxBuffer,11);   
}
void BAW2A_Get_V(void)
{
    MeterMBCmd = MDBS_METER_GET_V ;
    MeterTxBuffer[1]=0x03;          // Function 
    MeterTxBuffer[2]=0x00;          // Data 0
    MeterTxBuffer[3]=0xD6;          // Data 1
    MeterTxBuffer[4]=0x00;          // Data 2
    MeterTxBuffer[5]=0x02;          // Data 3
    CRC16(MeterTxBuffer,6);
    MeterTxBuffer[6]=uchCRCHi;      // // CRC Checksum 
    MeterTxBuffer[7]=uchCRCLo;      // // CRC Checksum 
    _SendStringToMETER(MeterTxBuffer,8); 
}
void BAW2A_Get_I(void)
{
    MeterMBCmd = MDBS_METER_GET_I ;
    MeterTxBuffer[1]=0x03;          // Function 
    MeterTxBuffer[2]=0x00;          // Data 0
    MeterTxBuffer[3]=0xD8;          // Data 1
    MeterTxBuffer[4]=0x00;          // Data 2
    MeterTxBuffer[5]=0x02;          // Data 3
    CRC16(MeterTxBuffer,6);
    MeterTxBuffer[6]=uchCRCHi;      // // CRC Checksum 
    MeterTxBuffer[7]=uchCRCLo;      // // CRC Checksum 
    _SendStringToMETER(MeterTxBuffer,8); 
}
void BAW2A_Get_F(void)
{
    MeterMBCmd = MDBS_METER_GET_F ;
    MeterTxBuffer[1]=0x03;          // Function 
    MeterTxBuffer[2]=0x00;          // Data 0
    MeterTxBuffer[3]=0xDA;          // Data 1
    MeterTxBuffer[4]=0x00;          // Data 2
    MeterTxBuffer[5]=0x01;          // Data 3
    CRC16(MeterTxBuffer,6);
    MeterTxBuffer[6]=uchCRCHi;      // // CRC Checksum 
    MeterTxBuffer[7]=uchCRCLo;      // // CRC Checksum 
    _SendStringToMETER(MeterTxBuffer,8);   		
}
void BAW2A_Get_P(void)
{
    MeterMBCmd = MDBS_METER_GET_P ;
    MeterTxBuffer[1]=0x03;          // Function 
    MeterTxBuffer[2]=0x00;          // Data 0
    MeterTxBuffer[3]=0xDB;          // Data 1 ( 有功功率 )
    MeterTxBuffer[4]=0x00;          // Data 2
    MeterTxBuffer[5]=0x02;          // Data 3
    CRC16(MeterTxBuffer,6);
    MeterTxBuffer[6]=uchCRCHi;      // // CRC Checksum 
    MeterTxBuffer[7]=uchCRCLo;      // // CRC Checksum 
    _SendStringToMETER(MeterTxBuffer,8);
}
void BAW2A_Get_S(void)
{
    MeterMBCmd = MDBS_METER_GET_S ;
    MeterTxBuffer[1]=0x03;          // Function 
    MeterTxBuffer[2]=0x00;          // Data 0
    MeterTxBuffer[3]=0xDD;          // Data 1 ( 視在功率 )
    MeterTxBuffer[4]=0x00;          // Data 2
    MeterTxBuffer[5]=0x02;          // Data 3
    CRC16(MeterTxBuffer,6);					// 
    MeterTxBuffer[6]=uchCRCHi;      // // CRC Checksum 
    MeterTxBuffer[7]=uchCRCLo;      // // CRC Checksum 
    _SendStringToMETER(MeterTxBuffer,8);   			
}
void BAW2A_Get_PF(void)
{
    MeterMBCmd = MDBS_METER_GET_PF ;
    MeterTxBuffer[1]=0x03;          // Function 
    MeterTxBuffer[2]=0x00;          // Data 0
    MeterTxBuffer[3]=0xDF;          // Data 1 ( 功率因子 )
    MeterTxBuffer[4]=0x00;          // Data 2
    MeterTxBuffer[5]=0x01;          // Data 3
    CRC16(MeterTxBuffer,6);
    MeterTxBuffer[6]=uchCRCHi;      // // CRC Checksum 
    MeterTxBuffer[7]=uchCRCLo;      // // CRC Checksum 
    _SendStringToMETER(MeterTxBuffer,8);   	
}
void BAW2A_SetPayMode(void)
{
    MeterMBCmd = MDBS_METER_OTHER ;
    MeterTxBuffer[1]=0x10;          // Function 
    MeterTxBuffer[2]=0x00;          // Data 0
    MeterTxBuffer[3]=0xE0;          // Data 1
    MeterTxBuffer[4]=0x00;          // Data 2
    MeterTxBuffer[5]=0x01;          // Data 3 
    MeterTxBuffer[6]=0x02;          // Data 3 
    MeterTxBuffer[7]=0x00;          // Data 3 
    MeterTxBuffer[8]=PayMode;          // Data 3 
    CRC16(MeterTxBuffer,9);
    MeterTxBuffer[9]=uchCRCHi;      // // CRC Checksum 
    MeterTxBuffer[10]=uchCRCLo;      // // CRC Checksum 
    _SendStringToMETER(MeterTxBuffer,11);
}
void BAW2A_SetPowerOn(void)
{
    MeterMBCmd = MDBS_METER_OTHER ;
    MeterTxBuffer[1]=0x10;          // Function 
    MeterTxBuffer[2]=0x00;          // Data 0
    MeterTxBuffer[3]=0xE2;          // Data 1
    MeterTxBuffer[4]=0x00;          // Data 2
    MeterTxBuffer[5]=0x01;          // Data 3    
    MeterTxBuffer[6]=0x02;          // Data 3 
    MeterTxBuffer[7]=0x00;          // Data 3 
    MeterTxBuffer[8]=PayMode;          // 扣款模式
    CRC16(MeterTxBuffer,9);
    MeterTxBuffer[9]=uchCRCHi;      // // CRC Checksum 
    MeterTxBuffer[10]=uchCRCLo;      // // CRC Checksum 
    _SendStringToMETER(MeterTxBuffer,11);   
}

void BAW2A_SetTime(void)
{
    MeterMBCmd = MDBS_METER_OTHER ;
    MeterTxBuffer[1]=0x10;          // Function 
    MeterTxBuffer[2]=0x00;          // Data 0
    MeterTxBuffer[3]=0x12;          // Data 1
    MeterTxBuffer[4]=0x00;          // Data 2
    MeterTxBuffer[5]=0x04;          // Data 3    
    MeterTxBuffer[6]=0x08;          // Data 3 

    MeterTxBuffer[7]=0x00;          // Data 3 
    MeterTxBuffer[8]=u8Year;          // 扣款模式
    MeterTxBuffer[9]=u8Month;          // Data 3 
    MeterTxBuffer[10]=u8Day;          // 扣款模式
    MeterTxBuffer[11]=u8Hour;          // Data 3 
    MeterTxBuffer[12]=u8Min;          // 扣款模式
    MeterTxBuffer[13]=u8Sec;          // Data 3 
    MeterTxBuffer[14]=u8Week;          // 扣款模式
    CRC16(MeterTxBuffer,15);
    MeterTxBuffer[16]=uchCRCHi;      // // CRC Checksum 
    MeterTxBuffer[17]=uchCRCLo;      // // CRC Checksum 
    _SendStringToMETER(MeterTxBuffer,18);   
}

void BAW2A_GetPayMode(void)
{
    MeterMBCmd = MDBS_METER_GET_MODE ;
    MeterTxBuffer[1]=0x03;          // Function 
    MeterTxBuffer[2]=0x00;          // Data 0
    MeterTxBuffer[3]=0xE0;          // Data 1 
    MeterTxBuffer[4]=0x00;          // Data 2
    MeterTxBuffer[5]=0x01;          // Data 3
    CRC16(MeterTxBuffer,6);
    MeterTxBuffer[6]=uchCRCHi;      // // CRC Checksum 
    MeterTxBuffer[7]=uchCRCLo;      // // CRC Checksum 
    _SendStringToMETER(MeterTxBuffer,8);   			
}
void BAW2A_GetStatus(void)
{
    MeterMBCmd = MDBS_METER_GET_STATUS ;
    MeterTxBuffer[1]=0x03;          // Function 
    MeterTxBuffer[2]=0x01;          // Data 0
    MeterTxBuffer[3]=0x22;          // Data 1 
    MeterTxBuffer[4]=0x00;          // Data 2
    MeterTxBuffer[5]=0x01;          // Data 3
    CRC16(MeterTxBuffer,6);
    MeterTxBuffer[6]=uchCRCHi;      // // CRC Checksum 
    MeterTxBuffer[7]=uchCRCLo;      // // CRC Checksum 
    _SendStringToMETER(MeterTxBuffer,8);   
}
void BAW2A_GetCardID(void)
{
    MeterMBCmd = MDBS_METER_GET_CARD_ID ;
    MeterTxBuffer[1]=0x03;          // Function 
    MeterTxBuffer[2]=0x01;          // Data 0
    MeterTxBuffer[3]=0x23;          // Data 1 ( 功率因子 )
    MeterTxBuffer[4]=0x00;          // Data 2
    MeterTxBuffer[5]=0x04;          // Data 3
    CRC16(MeterTxBuffer,6);
    MeterTxBuffer[6]=uchCRCHi;      // CRC Checksum 
    MeterTxBuffer[7]=uchCRCLo;     // CRC Checksum 
    _SendStringToMETER(MeterTxBuffer,8);   	
}

//巧力預付型電表 
void CmdModBus_BAW2A(uint8_t ModBusCommand)
{
	// 010300000003	
	switch ( ModBusCommand )
	{		
		case MDBS_READY :
			ModBusStep = 0 ;			
			break;
		case MDBS_METER_GET_WATT :
			BAW2A_GetWatt();
			break;
		case MDBS_METER_GET_RELAY :
			BAW2A_GetRelayStatus();	
			break;
		case MDBS_METER_GET_BAL :
			BAW2A_GetBalance();
			break;
		case MDBS_METER_SET_RELAY : 			
			BAW2A_SetRelay();
			break;
		case MDBS_METER_SET_ADDR : 
			BAW2A_SetAddr();
			break;
		case MDBS_METER_SET_BAUDRATE : 
			 BAW2A_SetBuadrate();
			break;
		case MDBS_METER_SET_PWD :
			BAW2A_SetPassword();
			break;
		case MDBS_METER_ADD_VALUE :
			BAW2A_AddBalance();
			break;
		case MDBS_METER_EXIT_TEST : 
			BAW2A_ExitTestMode();
			break;
		case MDBS_METER_GET_V : 
			BAW2A_Get_V();  			
			break;
		case MDBS_METER_GET_I : 
			BAW2A_Get_I();  			
			break;
		case MDBS_METER_GET_F : 
			BAW2A_Get_F();	
			break;
		case MDBS_METER_GET_P: 
			BAW2A_Get_P();   			
			break;		
		case MDBS_METER_GET_S: 
			BAW2A_Get_S();
			break;
		case MDBS_METER_GET_PF: 
			BAW2A_Get_PF();		
			break;
		case MDBS_METER_SET_MODE : 			
			BAW2A_SetPayMode();			   
			break;
		case MDBS_METER_SET_POWER_ON: 
			BAW2A_SetPowerOn();   
			break;
		case MDBS_METER_SET_TIME: 
			BAW2A_SetTime();
			break;
		case MDBS_METER_GET_MODE: 
			BAW2A_GetPayMode();
			break;
		case MDBS_METER_GET_STATUS: 
			BAW2A_GetStatus();			
			break;
		case MDBS_METER_GET_CARD_ID: 
			BAW2A_GetCardID();		
			break;            
		default :
			MeterMBCmd = MDBS_METER_OTHER ;
			break;
	}
}

// Tatung E21/E31
void CmdModBus_E21nE31(uint8_t ModBusCmd)
{
	// Tatung E21/E31 
	switch ( ModBusCmd )
	{		
		case MDBS_READY :
			ModBusStep = 0 ;
			break;
		case MDBS_METER_GET_WATT :
			MeterMBCmd = MDBS_METER_GET_WATT ;
			bReadPowerMeterValue = 1 ;									
			MeterTxBuffer[1]=0x03;          // Function 
			MeterTxBuffer[2]=0x01;          // Data 0
			MeterTxBuffer[3]=0x04;          // Data 1
			MeterTxBuffer[4]=0x00;          // Data 2
			MeterTxBuffer[5]=0x02;          // Data 3
			CRC16(MeterTxBuffer,6);
			MeterTxBuffer[6]=uchCRCHi;      // // CRC Checksum 
			MeterTxBuffer[7]=uchCRCLo;      // // CRC Checksum 
			_SendStringToMETER(MeterTxBuffer,8);   						
			break;
		case MDBS_METER_GET_RELAY :
			MeterMBCmd = MDBS_METER_GET_RELAY ;
			bReadPowerMeterValue = 1 ;			
			MeterTxBuffer[1]=0x03;          // Function 
			MeterTxBuffer[2]=0x01;          // Data 0
			MeterTxBuffer[3]=0x00;          // Data 1
			MeterTxBuffer[4]=0x00;          // Data 2
			MeterTxBuffer[5]=0x01;          // Data 3
			CRC16(MeterTxBuffer,6);
			MeterTxBuffer[6]=uchCRCHi;      // // CRC Checksum 
			MeterTxBuffer[7]=uchCRCLo;      // // CRC Checksum 
			_SendStringToMETER(MeterTxBuffer,8);   					
			break;
		case MDBS_METER_SET_RELAY : 
			MeterMBCmd = MDBS_METER_OTHER ;
			MeterTxBuffer[1]=0x10;          // Function 
			MeterTxBuffer[2]=0x00;          // Data 0
			MeterTxBuffer[3]=0x03;          // Data 1
			MeterTxBuffer[4]=0x00;          // Data 2
			MeterTxBuffer[5]=0x01;          // Data 3 
			MeterTxBuffer[6]=0x02;          // Data 3 
			MeterTxBuffer[7]=RelayOnOff;          // Data 3 
			MeterTxBuffer[8]=0x00;          // Data 3 
			CRC16(MeterTxBuffer,9);
			MeterTxBuffer[9]=uchCRCHi;      // // CRC Checksum 
			MeterTxBuffer[10]=uchCRCLo;      // // CRC Checksum 
			_SendStringToMETER(MeterTxBuffer,11);   
			break;
		case MDBS_METER_SET_ADDR : 
			MeterMBCmd = MDBS_METER_OTHER ;
			MeterTxBuffer[1]=0x10;          // Function 
			MeterTxBuffer[2]=0x00;          // Data 0
			MeterTxBuffer[3]=0x05;          // Data 1
			MeterTxBuffer[4]=0x00;          // Data 2
			MeterTxBuffer[5]=0x01;          // Data 3 
			MeterTxBuffer[6]=0x02;          // Data 3 
			MeterTxBuffer[7]=PowerMeterNewAddr;          // Data 3 
			MeterTxBuffer[8]=0x00;          // Data 3 
			CRC16(MeterTxBuffer,9);
			MeterTxBuffer[9]=uchCRCHi;      // // CRC Checksum 
			MeterTxBuffer[10]=uchCRCLo;      // // CRC Checksum 
			_SendStringToMETER(MeterTxBuffer,11);   
			break;
		case MDBS_METER_SET_BAUDRATE : 
			// No Support , Default is 9600
			MeterMBCmd = MDBS_METER_OTHER ;
			break;
		default :
			MeterMBCmd = MDBS_METER_OTHER ;
			break;
	}
}

void CmdModBus_DEM_510c(uint8_t ModBusCommand)
{
	switch ( ModBusCommand )
	{		
		case MDBS_READY :
			ModBusStep = 0 ;
			break;
		case MDBS_METER_GET_WATT :
			MeterMBCmd = MDBS_METER_GET_WATT ;
			bReadPowerMeterValue = 1 ;						
			MeterTxBuffer[1]=0x03;          // Function 
			MeterTxBuffer[2]=0x00;          // Data 0
			MeterTxBuffer[3]=0x00;          // Data 1
			MeterTxBuffer[4]=0x00;          // Data 2
			MeterTxBuffer[5]=0x02;          // Data 3
			CRC16(MeterTxBuffer,6);
			MeterTxBuffer[6]=uchCRCHi;      // // CRC Checksum 
			MeterTxBuffer[7]=uchCRCLo;      // // CRC Checksum 
			_SendStringToMETER(MeterTxBuffer,8);   			
			break;
		case MDBS_METER_GET_RELAY :
			MeterMBCmd = MDBS_METER_GET_RELAY ;
			bReadPowerMeterValue = 1 ;			
			MeterTxBuffer[1]=0x03;          // Function 
			MeterTxBuffer[2]=0x00;          // Data 0
			MeterTxBuffer[3]=0x03;          // Data 1
			MeterTxBuffer[4]=0x00;          // Data 2
			MeterTxBuffer[5]=0x01;          // Data 3
			CRC16(MeterTxBuffer,6);
			MeterTxBuffer[6]=uchCRCHi;      // // CRC Checksum 
			MeterTxBuffer[7]=uchCRCLo;      // // CRC Checksum 
			_SendStringToMETER(MeterTxBuffer,8);   					
			break;
		case MDBS_METER_SET_RELAY : 
			MeterMBCmd = MDBS_METER_OTHER ;
			MeterTxBuffer[1]=0x05;          // Function 
			MeterTxBuffer[2]=0x00;          // Data 0
			MeterTxBuffer[3]=0x00;          // Data 1
			MeterTxBuffer[4]=0x00;          // Data 2
			if ( RelayOnOff ){
				MeterTxBuffer[5]=0xFF;          // Data 3 
			} else {
				MeterTxBuffer[5]=0x00;          // Data 3 
			}
			CRC16(MeterTxBuffer,6);
			MeterTxBuffer[6]=uchCRCHi;      // // CRC Checksum 
			MeterTxBuffer[7]=uchCRCLo;      // // CRC Checksum 
			_SendStringToMETER(MeterTxBuffer,8);   
			break;		
		case  MDBS_METER_SET_ADDR_EN: 
			MeterMBCmd = MDBS_METER_OTHER ;
			MeterTxBuffer[1]=0x05;          
			MeterTxBuffer[2]=0x00;          
			MeterTxBuffer[3]=0x30;          
			MeterTxBuffer[4]=0x00;         
			MeterTxBuffer[5]=0x00;          
			CRC16(MeterTxBuffer,6);
			MeterTxBuffer[6]=uchCRCHi;      // // CRC Checksum 
			MeterTxBuffer[7]=uchCRCLo;      // // CRC Checksum 
			_SendStringToMETER(MeterTxBuffer,8);   
			break;
		case  MDBS_METER_SET_ADDR_DIS: 
			MeterMBCmd = MDBS_METER_OTHER ;
			MeterTxBuffer[1]=0x05;          
			MeterTxBuffer[2]=0x00;          
			MeterTxBuffer[3]=0x30;          
			MeterTxBuffer[4]=0xFF;         
			MeterTxBuffer[5]=0x00;          
			CRC16(MeterTxBuffer,6);
			MeterTxBuffer[6]=uchCRCHi;      // // CRC Checksum 
			MeterTxBuffer[7]=uchCRCLo;      // // CRC Checksum 
			_SendStringToMETER(MeterTxBuffer,8);   
			break;
		case MDBS_METER_SET_ADDR : 
			MeterMBCmd = MDBS_METER_OTHER ;
			MeterTxBuffer[1]=0x10;          // Function 
			MeterTxBuffer[2]=0x00;          // Data 0
			MeterTxBuffer[3]=0x30;          // Data 1
			MeterTxBuffer[4]=0x00;          // Data 2
			MeterTxBuffer[5]=0x01;          // Data 3 
			MeterTxBuffer[6]=0x02;          // Data 3 
			MeterTxBuffer[7] = PowerMeterNewAddr;          // Data 3 
			MeterTxBuffer[8]=0x00;          // Data 3 
			CRC16(MeterTxBuffer,9);
			MeterTxBuffer[9]=uchCRCHi;      // // CRC Checksum 
			MeterTxBuffer[10]=uchCRCLo;      // // CRC Checksum 
			_SendStringToMETER(MeterTxBuffer,11);   
			break;
		case MDBS_METER_SET_BAUDRATE : 
			MeterMBCmd = MDBS_METER_OTHER ;
			MeterTxBuffer[1]=0x10;          // Function 
			MeterTxBuffer[2]=0x00;          // Data 0
			MeterTxBuffer[3]=0x37;          // Data 1
			MeterTxBuffer[4]=0x00;          // Data 2
			MeterTxBuffer[5]=0x01;          // Data 3            
			MeterTxBuffer[6]=0x02;          // Data 1
			MeterTxBuffer[7]=0x00;          // Data 2
			MeterTxBuffer[8]=0x00;          // Data 3    
		CRC16(MeterTxBuffer,6);
			MeterTxBuffer[6]=uchCRCHi;      // // CRC Checksum 
			MeterTxBuffer[7]=uchCRCLo;      // // CRC Checksum 
			_SendStringToMETER(MeterTxBuffer,8);   
			break;
		case MDBS_METER_SET_PWD :
			MeterMBCmd = MDBS_METER_OTHER ;
			MeterTxBuffer[1]=0x05;          // Function 
			MeterTxBuffer[2]=0x00;          // Data 0
			MeterTxBuffer[3]=0x30;          // Data 1
			MeterTxBuffer[4]=0xFF;          // Data 2
			MeterTxBuffer[5]=0x00;          // Data 3            
			CRC16(MeterTxBuffer,6);
			MeterTxBuffer[6]=uchCRCHi;      // // CRC Checksum 
			MeterTxBuffer[7]=uchCRCLo;      // // CRC Checksum 
			_SendStringToMETER(MeterTxBuffer,8);   
			break;
		case MDBS_METER_SET_EXIT :
			MeterMBCmd = MDBS_METER_OTHER ;
			MeterTxBuffer[1]=0x05;          // Function 
			MeterTxBuffer[2]=0x00;          // Data 0
			MeterTxBuffer[3]=0x30;          // Data 1
			MeterTxBuffer[4]=0xFF;          // Data 2
			MeterTxBuffer[5]=0x00;          // Data 3            
			CRC16(MeterTxBuffer,6);
			MeterTxBuffer[6]=uchCRCHi;      // // CRC Checksum 
			MeterTxBuffer[7]=uchCRCLo;      // // CRC Checksum 
			_SendStringToMETER(MeterTxBuffer,8);   
			break;
		default :
			MeterMBCmd = MDBS_METER_OTHER ;
			break;
	}
}

void CRC16(uint8_t *puchMsg, uint16_t usDataLen)
{    
    unsigned uIndex ; /* will index into CRC lookup table */
    uchCRCHi = 0xFF;  /* high byte of CRC initialized */
    uchCRCLo = 0xFF;  /* low byte of CRC initialized */
    while (usDataLen--) /* pass through message buffer */
    {
        uIndex = uchCRCHi ^ *puchMsg++ ; /* calculate the CRC */
        uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex] ;
        uchCRCLo = auchCRCLo[uIndex] ;
    }

    //return ( ((uchCRCHi << 8) | uchCRCLo) ) ;
}

/*
CMD: 01 03 00 00 00 64 44 21
EVENT: 
01 03 C8 00 0A 00 00 00 00 00 
00 00 FA 03 20 01 90 01 A8 00 
01 00 00 00 11 00 01 00 00 00 
00 07 E0 00 0A 00 19 00 04 00 
0B 00 1D 01 18 01 10 01 94 00 
00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 
00 00 

CMD: 01 03 00 00 00 64 44 21
EVENT: 
01 03 C8 00 0A 00 00 00 00 00 
00 00 FA 03 20 01 90 01 A8 00 
01 00 00 00 11 00 01 00 00 00 
00 07 DF 00 0A 00 19 00 04 00 
0B 00 39 01 19 01 12 01 7D 00 
00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 
00 00 01 03 00 00 00 64 44 21 
00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 8F 53 00 00 
00 00 00 01 1C 92 00 20 84 8B 
00 20 01 00 00 00 3A F2 E5 00 
3C 00 00 20 00 B3 00 20 F4 92 
00 20 34 00 00 20 0A 00 00 00 
00 00 00 00 00 00 00 00 F4 92 
00 20 00 00 00 00 55 41 52 54 
31 00 00 00 8F 53 00 00 38 B5 
FD 00 00 00 00 00 7B 74 FF DF 
00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 
00 00 00 AA 1A 

*/

