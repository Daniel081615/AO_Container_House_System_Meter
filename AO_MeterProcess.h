/*--------------------------------------------------------------------------- 
 * File Name     : MAIN.C
 * Company 		 : AOTECH
 * Author Name   : Barry Su
 * Starting Date : 2015.7.1
 * C Compiler : Keil C
 * -------------------------------------------------------------------------- 
*/
#ifndef __AO_METER_PROCESS_H__
#define __AO_METER_PROCESS_H__



//enum DefineHostCommandType {       
//};

typedef struct {
	uint32_t PwrMtrDeviceNG;
	uint8_t Success[PwrMtrMax];
	uint8_t Fail[PwrMtrMax];
	uint8_t ErrorRate[PwrMtrMax];
} PwrMtrError_t;

extern void MeterDataProcess(void);

#endif
