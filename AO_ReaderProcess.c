/*--------------------------------------------------------------------------- 
 * File Name     : MAIN.C
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
#include "AO_HostProcess.h"
#include "AO_MyDef.h"

#define POLLING_TIMEOUT 	60
#define ERROR_RETRY_MAX 	15

uint8_t ReaderMode,ReaderUser;
uint8_t ReaderUID[4];
uint8_t ErrorTimes,ModeSyncError,PackageMax,ReaderResult;
_Bool fgReaderInitCompleted;
uint8_t ELockDataErrorCounter;
uint8_t ErrorCounter;
uint8_t GetMemberInfoIndex;
uint8_t GotRecordNewCounter;
uint8_t ReaderStatus;
uint8_t LastRecordWP,LastNewRecordCounter;

//extern _Bool fgUartReaderBusy;
extern uint8_t fgReaderSync;

void ReaderProcess(void);
void CalChecksumR(void);
void SendReader_UserInformation(uint8_t DataIndex);
void SendReader_Alive(void);
void ReaderPolling(void);
void UserCGRModeJoin(uint8_t *fnMemberUID);
void UserCGRModeExit(uint8_t *fnMemberUID);
void RecordJoinCGR(uint8_t fnUserIndex);
void SendReader_GetRecord(void);
void Reader_RSP_SystemInformation(void);
void Reader_AckProcess(void);
void SendReader_ChangeMode(void);
void SendReader_ChangeBalance(uint8_t fnPackageIndex);
void Reader_RecordProcess(void);
uint8_t _SendStringToREADER(uint8_t *Str, uint8_t len);

/*

1. Polling Status : if any data changed, meter get data from reader	
2. Polling Update reader data

*/
void ReaderPolling(void)
{	
    
}

void ReaderProcess(void)
{
  
	
}
    	
    
	

void CalChecksumR(void)
{
	uint8_t i;
	uint8_t Checksum;

	//DIR485_READER_Out();
	fgDIR485_Reader = 1 ;
	
	ReaderTxBuffer[0] = 0x55 ;
	ReaderTxBuffer[1] = 0x01 ;

	Checksum = 0 ;
	for (i=1;i<(MAX_READER_TXQ_LENGTH-2);i++)
	{
		Checksum += ReaderTxBuffer[i];
	}	
	ReaderTxBuffer[MAX_READER_TXQ_LENGTH-2] = Checksum ;
	ReaderTxBuffer[MAX_READER_TXQ_LENGTH-1] = '\n' ;	
	
	_SendStringToREADER(ReaderTxBuffer,MAX_READER_TXQ_LENGTH);
}

uint8_t _SendStringToREADER(uint8_t *Str, uint8_t len)
{
	
	uint8_t idx;

	if( (READERTxQ_cnt+len) > MAX_READER_TXQ_LENGTH )
	{
		return 0x01 ;
	} else {
		DIR_READER_RS485_Out();
		for(idx=0;idx<len;idx++)
		{
			READERTxQ[READERTxQ_wp]=Str[idx];
			READERTxQ_wp++;
			if(READERTxQ_wp>=MAX_READER_TXQ_LENGTH)
			{
				READERTxQ_wp=0;
			}
			READERTxQ_cnt++;
		}        				
		UART_EnableInt(UART1, (UART_INTEN_THREIEN_Msk ));
		NVIC_EnableIRQ(UART1_IRQn);
	}                   
	return 0x00 ;
	
}


