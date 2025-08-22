/*--------------------------------------------------------------------------- 
 * File Name     : MAIN.C
 * Company 		 : AOTECH
 * Author Name   : Barry Su
 * Starting Date : 2015.7.1
 * C Compiler : Keil C
 * -------------------------------------------------------------------------- 
*/
#ifndef __AO_READER_PROCESS_H__
#define __AO_READER_PROCESS_H__




//enum DefineHostCommandType {       
//};
extern uint8_t ReaderUser;
extern void ReaderProcess(void);
extern void SendCommandToHost(uint8_t cmd);
extern void SendReader_UserOpenPower(void);
extern void SendReader_UserClosePower(void);
extern void SendReader_SetRoomMember(uint8_t UserIndex);
extern void SendReader_SetRoomInfo(void);
extern void SendReader_TimeSync(void);
extern void SendReader_ValueUpdated(void);
extern void SendReader_StartupStart(void);
extern void SendReader_StartupExit(void);
extern void SendReader_PowerEnable(void);
extern void SendReader_ValueFailure(void);
extern void SendReader_Information(uint8_t DataIndex);
extern void SendReader_ChangeMode(void);
extern void SendReader_TurnOffReaderLED(void);
extern void SendReader_UserInformation(uint8_t DataIndex);
extern void SendReader_Alive(void);








#endif
