/*
 * IncFile1.h
 *
 * Created: 2018/6/18 上午 10:27:04
 *  Author: Barry
 */ 


#ifndef _AO_EXTERN_FUNC_H_
#define _AO_EXTERN_FUNC_H_

#include "AO_MyDef.h"
#include "AO_MeterProcess.h"

extern void ReadParameterFromEE(void);
extern void WriteParameterFromEE(void);
extern void ReadUserInfoFromEE(void);
extern void WriteUserInfoToEE(void);
extern void ReadUserValueFromEE(void);
extern void WriteUserValueToEE(void);
extern void WriteRoomInfoToEE(void);
extern void ReadRoomInfoFromEE(void);
extern void PowerCostCal(void);
extern void ResetHostUART(void);
extern void ResetMeterUART(void);
extern uint8_t CheckUserValue(uint8_t *fnMemberUID);
extern void SendReader_ValueFailure(void);
extern void WriteUserInfoToEE_One(uint8_t fMemberIndex);
extern void WriteUserValueToEE_One(uint8_t fMemberIndex);
extern uint16_t ReadUserValueFromEE_One(uint8_t fMemberIndex);
extern void SendHost_MeterStatus(void);
extern void InitSystemMode(void);
extern void WritePowerToEE(void);
extern void SendHost_RspReaderInformation(void);
extern void SendHost_RspReadEE(void);
extern void SendReader_SystemSW(void);
extern void RecordStopProcess(void);
extern void RoomPowerManager_Pluse(void);
extern void RoomPowerManager_Digital(void);

extern void ReaderPolling(void);
extern void MeterPolling(void);

extern void MeterValueCounter(void);
extern void SendReader_Alive(void);
extern void ResetReaderUART(void);
extern void MeterValueReset(void);
extern void WriteRoomModeToEE(void);
extern void SendReader_GetUser(uint8_t UserIndex);
extern void TurnOnPowerProcess(void);
extern void TurnOffPowerProcess(void);
extern void SaveMeterPower2EE( void );
extern void ReadMeterPowerFromEE( void );
extern void MODBUS_CmdProcess(void);
extern void DigitMeterPowerON(void);
extern void DigitMeterPowerOFF(void);

extern void SendHost_MeterStatus(void);
extern void UserCGRModeJoin(uint8_t *fnMemberUID);
extern void UserCGRModeExit(uint8_t *fnMemberUID);
extern void EnableHostUartTx(void);
extern void DisableHostUartTx(void);
extern uint8_t _SendStringToMETER(uint8_t *Str, uint8_t len);
extern uint8_t PollingPwrMtrID;
extern uint8_t MaxPowerMeter;
extern uint16_t Tick1S_ErrorRateGap;
extern uint8_t PayMode;
extern uint8_t RoomMode[PwrMtrMax];

extern uint8_t PwrMtrIDArray[];
extern uint8_t BmsIDArray[];
extern uint8_t WtrMtrIDArray[];
extern uint8_t PyrMtrIDArray[];
extern uint8_t SoilSensorIDArray[];
extern uint8_t AirSensorIDArray[];

extern uint16_t MeterErrorRate_Tx[PwrMtrMax];
extern uint16_t MeterErrorRate_Rx[PwrMtrMax];
extern uint8_t     MeterErrorRate5Min_Wp;
extern uint16_t MeterErrorRate5Min_Tx[PwrMtrMax][12];
extern uint16_t MeterErrorRate5Min_Rx[PwrMtrMax][12];

extern uint8_t ReaderPollingState,ReaderWaitTick,PollRetryReader;
extern uint8_t PackageIndex1,GotReaderRSP,ReaderDeviceError;
extern uint8_t RecordWp,RecordRp,RecordCounter,TickRecord;
extern uint8_t MeterActive;

//extern STR_UserInfo Members[MEMBER_MAX_NO];
extern STR_METER_D MeterData[PwrMtrMax];
extern uint8_t ControlState;
extern uint8_t MyMeterBoardID;
extern uint8_t iStatus;
extern uint8_t CheckUserResult;
extern uint8_t ReaderStatus;

extern uint8_t UID[4];
extern uint8_t NewCardUID[4];
extern uint8_t iKey,fgTimeSync;							// 加密基數
extern uint8_t TokenHost[HOST_TOKEN_LENGTH];
extern uint8_t TokenMeter[METER_TOKEN_LENGTH];
extern uint8_t MeterTxBuffer[METER_TOKEN_LENGTH];
extern uint8_t HostTxBuffer[HOST_TOKEN_LENGTH];
extern uint8_t NewUser,NowUser,LastUser;
extern uint8_t fgSendFinish;
extern uint8_t MemberIndex;
extern uint8_t MeterID;
extern uint32_t  u32PowerCounter100W;
extern uint8_t 	READERTokenReady,HOSTTokenReady;
extern _Bool fgUpdateValueMsg,fgHostAckOK,fgReaderAckOK,SendDelayFlag,bNonAck,bInitSystemMode ;
extern uint8_t PollingMode;
extern uint8_t TickReader,fgReportPower,bValueUpdated,bSystemTimeReady;

extern uint8_t iTickDelaySendHostCMD,bDelaySendHostCMD;
extern uint8_t TickPollWating;
extern uint32_t u32UserPower100W[MEMBER_MAX_NO];
extern uint8_t ReaderDataSync;
extern uint8_t ReaderState,ReaderTick;
extern uint8_t bSendReaderCommand,ReaderCommandType;
extern uint8_t LastReaderRecord_RP;

extern uint8_t TickHostUart, TickMeterUart;
extern uint8_t 	iSystemTime[8];
extern _Bool TokenMeterReady;
extern uint8_t TickHost;
extern uint8_t PacketIndex;
extern uint8_t fgFromHostFlag,fgToHostFlag,fgToHostRSPFlag,fgReaderFunction;
extern uint8_t fgFromReaderFlag, fgToReaderFlag, fgToReaderRSPFlag;
extern uint8_t ToReader_MemberIndex;
extern uint8_t HostGetRCDType,HostGetRCDIndex;
extern uint8_t NewRecordCounter, Record_WP, Record_RP;

/***
 @note : Devices variables extern
 ***/
//	Powermeter
extern uint8_t PwrMtrNewAddr;
extern uint8_t PwrMtrNewBaudRate, PowerMeterMode, PowerMeterDO_OnOff, PowerMeterDOLock;
extern PwrMtrError_t PwrMtrError;
//	Bms
extern uint32_t BmsNewAddr, CellCount;
extern uint16_t ModeFlags;
//	Watermeter
extern uint8_t WMBaudRate, WMSetDeviceID;
//	Pyranmeter
extern uint16_t NewDeviationValue, PyrNewAddr, PyrNewBaudRate;
//	Soil Sensor
extern uint16_t SSNewAddr, SSNewBaudRate, N_RegValue, P_RegValue, K_RegValue;
extern uint16_t Ec_COEF, Salinity_COEF, Tds_COEF;
extern uint16_t Temp_CalibValue, Moisture_CalibValue, Ec_CalibValue, Ph_CalibValue;
extern uint16_t	Fert_Offset, N_Offset, P_Offset, K_Offset;
extern uint32_t	Fert_COEF, N_COEF, P_COEF, K_COEF;

extern uint8_t uchCRCHi;  /* high byte of CRC initialized */
extern uint8_t uchCRCLo;  /* low byte of CRC initialized */
extern uint8_t MeterType;
extern uint32_t NowMeterPower;
extern uint32_t TotalWattValue_Now;
extern uint8_t MeterMBCmd,MeterRelayStatus;
extern _Bool fgSwPower,PowerOnReadMeter,PowerOnTimeSync;
extern uint8_t MeterPollingState,MeterError,TickPollingInterval;
extern uint16_t  TickReadDeviceTime,ReadDeviceCmdTime;

extern uint8_t  DelayTime4NextCmd,MtrRelayOnOff,CenterRoomMode;

extern uint8_t ButtonStatus,METER_RXQLen;
extern uint8_t AckResult,MemberBase;
extern uint8_t NewRecordCounter, Record_WP, Record_RP;
extern uint8_t TagChargeRCD_NewCnt, TagChargeRCD_WP, TagChargeRCD_RP;
extern uint8_t PowerRCD_NewCnt, PowerRCD_WP, PowerRCD_RP;
extern float Min_LowBalance;

extern uint8_t PollingStateIndex;
extern _Bool fgDIR485_Reader;
extern uint32_t PowerMeterRxCounter[PwrMtrMax];
extern uint32_t PowerMeterTxCounter[PwrMtrMax];
extern uint8_t HOSTTxQ[MAX_HOST_TXQ_LENGTH];
extern uint8_t HOSTRxQ[MAX_HOST_RXQ_LENGTH];
extern uint8_t METERRxQ[MAX_METER_RXQ_LENGTH];
extern uint8_t METERTxQ[MAX_METER_TXQ_LENGTH];

extern uint8_t HOSTRxQ_wp,HOSTRxQ_rp,HOSTRxQ_cnt;
extern uint8_t READERRxQ_wp,READERRxQ_rp,READERRxQ_cnt;
extern uint8_t HOSTTxQ_wp,HOSTTxQ_rp,HOSTTxQ_cnt;
extern uint8_t READERTxQ_wp,READERTxQ_rp,READERTxQ_cnt;
extern uint8_t METERTxQ_wp,METERTxQ_rp,METERTxQ_cnt;
extern uint8_t METERRxQ_wp,METERRxQ_rp,METERRxQ_cnt;


extern uint8_t GotDeviceRsp;
extern volatile uint8_t PwrMtrCmdList[PwrMtrMax], BmsCmdList[BmsMax];
extern volatile uint8_t WtrMtrCmdList[WtrMtrMax], PyrMtrCmdList[PyrMtrMax];
extern volatile uint8_t SoilSensorCmdList[SoilSensorMax];

extern TotErrorRate_t TotErrorRate;


extern _Bool fgFirstStart;

extern uint16_t GetMeterDataAddress(uint8_t index);
extern void I2cWriteDataStruct(uint8_t index, STR_METER_D *data);
extern void I2cReadDataStruct(uint8_t index, STR_METER_D *data);

extern void CmdModbus_BMS(uint8_t ModbusCmd);
extern void CmdModbus_WM(uint8_t ModbusCmd);
extern void CmdModBus_DEM5x0(uint8_t ModBusCmd);


extern void UART2_Init(uint32_t u32baudrate);
extern void MeterDEM_510c_Init(void);
extern void Bms_Init(void);
extern void WtrMeter_Init(void);
extern void Delay_10ms(uint8_t ms);

extern uint16_t SystemTick;
extern volatile uint32_t u32TimeTick2;
extern volatile uint8_t SystemPollingState;

extern void CRC16(unsigned char *puchMsg,unsigned short usDataLen);

//extern _Bool fgUartReaderBusy;
#ifdef METER_TEST
extern uint32_t MeterValueTest;
#endif
#endif /* INCFILE1_H_ */
