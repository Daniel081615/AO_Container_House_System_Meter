/***
 *	@file		AO_SoilSensor.c
 *	@breif	SoilSensor modbus cmds, polling modules
 *	@type		SN3002TR
 *	@abbreviate	SS -> SoilSensor
 ***/
 
#include	"AO_SoilSensor.h"
#include 	"NUC1261.h"
#include	"AO_ExternFunc.h"
#include	"AO_ModBusProcess.h"

void SoilSensorPolling(void);
void SoilSensorDataProcess(void);
void MODBUS_SendSoilSensorCmd(uint8_t cmd);
void CmdModbus_SoilSensor_SN3002TR(uint8_t cmd);
void SoilSensorSuccess(void);
void SoilSensorTimeoutProcess(void);

SoilSensorData_t SoilSensorData[SoilSensorMax];
SoilSensorError_t SoilSensorError;

_Bool 		SoilSensorPollingFinishedFlag;
uint8_t 	PollingSoilSensorID;
uint8_t 	SoilSensorMBCMD;
uint8_t 	SoilSensorPollingState, SoilSensorPollingStateIndex, SoilSensorModbusCmd;
uint8_t 	SoilSensorReadErrorCnt;
uint16_t 	SSNewAddr, SSNewBaudRate, N_RegValue, P_RegValue, K_RegValue;
uint16_t  Ec_COEF, Salinity_COEF, Tds_COEF;
uint16_t  Temp_CalibValue, Moisture_CalibValue, Ec_CalibValue, Ph_CalibValue;
uint16_t	Fert_Offset, N_Offset, P_Offset, K_Offset;
uint32_t	Fert_COEF, N_COEF, P_COEF, K_COEF;
/***
 *	@function		SoilSensorPolling()
 *	@brief			polling soil_sensor in sequence. and record the communication.
 *	@note				Build host structure.
 *	2025.09.01
 ***/
void SoilSensorPolling(void)
{
		switch(SoilSensorPollingState)	
		{
				case	SS_POLLING_READY:
						if (TickReadPowerTime >= ReadMeterTime)
						{
								TickReadPowerTime = 0 ;	
								SoilSensorPollingState = SS_POLLING_MOISTURE + SoilSensorPollingStateIndex;
								
								if (SoilSensorPollingState > SS_POLLING_K_OFFSET)
								{
										PollingSoilSensorID++;
										SoilSensorPollingState = SS_POLLING_MOISTURE;
										SoilSensorPollingStateIndex = 0;
								}
						} else {
								SoilSensorModbusCmd = SoilSensorCmdList[PollingSoilSensorID-1] ;
								SoilSensorCmdList[PollingSoilSensorID-1] = MBSSCMD_READY;

								switch ( SoilSensorModbusCmd )	
								{
										case MBSSCMD_READY :
											break;
										
										case MBSSCMD_SET_N_VALUE:
												SoilSensorPollingState = SS_SET_N_VALUE;
												SoilSensorModbusCmd = MBSSCMD_READY;
												break;

										case MBSSCMD_SET_P_VALUE:
												SoilSensorPollingState = SS_SET_P_VALUE;
												SoilSensorModbusCmd = MBSSCMD_READY;
												break;

										case MBSSCMD_SET_K_VALUE:
												SoilSensorPollingState = SS_SET_K_VALUE;
												SoilSensorModbusCmd = MBSSCMD_READY;
												break;

										case MBSSCMD_SET_EC_COEF:
												SoilSensorPollingState = SS_SET_EC_COEF;
												SoilSensorModbusCmd = MBSSCMD_READY;
												break;

										case MBSSCMD_SET_SALINITY_COEF:
												SoilSensorPollingState = SS_SET_SALINITY_COEF;
												SoilSensorModbusCmd = MBSSCMD_READY;
												break;

										case MBSSCMD_SET_TDS_COEF:
												SoilSensorPollingState = SS_SET_TDS_COEF;
												SoilSensorModbusCmd = MBSSCMD_READY;
												break;

										case MBSSCMD_SET_TEMP_CALIB:
												SoilSensorPollingState = SS_SET_TEMP_CALIB;
												SoilSensorModbusCmd = MBSSCMD_READY;
												break;

										case MBSSCMD_SET_MOISTURE_CALIB:
												SoilSensorPollingState = SS_SET_MOISTURE_CALIB;
												SoilSensorModbusCmd = MBSSCMD_READY;
												break;

										case MBSSCMD_SET_EC_CALIB:
												SoilSensorPollingState = SS_SET_EC_CALIB;
												SoilSensorModbusCmd = MBSSCMD_READY;
												break;

										case MBSSCMD_SET_PH_CALIB:
												SoilSensorPollingState = SS_SET_PH_CALIB;
												SoilSensorModbusCmd = MBSSCMD_READY;
												break;

										case MBSSCMD_SET_FERT_COEF:
												SoilSensorPollingState = SS_SET_FERT_COEF;
												SoilSensorModbusCmd = MBSSCMD_READY;
												break;

										case MBSSCMD_SET_FERT_OFFSET:
												SoilSensorPollingState = SS_SET_FERT_OFFSET;
												SoilSensorModbusCmd = MBSSCMD_READY;
												break;

										case MBSSCMD_SET_N_COEF:
												SoilSensorPollingState = SS_SET_N_COEF;
												SoilSensorModbusCmd = MBSSCMD_READY;
												break;

										case MBSSCMD_SET_N_OFFSET:
												SoilSensorPollingState = SS_SET_N_OFFSET;
												SoilSensorModbusCmd = MBSSCMD_READY;
												break;

										case MBSSCMD_SET_P_COEF:
												SoilSensorPollingState = SS_SET_P_COEF;
												SoilSensorModbusCmd = MBSSCMD_READY;
												break;

										case MBSSCMD_SET_P_OFFSET:
												SoilSensorPollingState = SS_SET_P_OFFSET;
												SoilSensorModbusCmd = MBSSCMD_READY;
												break;

										case MBSSCMD_SET_K_COEF:
												SoilSensorPollingState = SS_SET_K_COEF;
												SoilSensorModbusCmd = MBSSCMD_READY;
												break;

										case MBSSCMD_SET_K_OFFSET:
												SoilSensorPollingState = SS_SET_K_OFFSET;
												SoilSensorModbusCmd = MBSSCMD_READY;
												break;

										case MBSSCMD_SET_DEVICE_ADDRESS:
												SoilSensorPollingState = SS_SET_DEVICE_ADDRESS;
												SoilSensorModbusCmd = MBSSCMD_READY;
												break;

										case MBSSCMD_SET_BAUD_RATE:
												SoilSensorPollingState = SS_SET_BAUD_RATE;
												SoilSensorModbusCmd = MBSSCMD_READY;
												break;
										
										default :
                        break;
								}
						}
						break;

				case	SS_POLLING_RSP:
            if (GotDeviceRsp == SoilSensorIDArray[PollingSoilSensorID-1])
            {
                SoilSensorSuccess();
            } else {
                SoilSensorTimeoutProcess();
            }
					break;
				
				case	SS_POLLING_MOISTURE:
						MODBUS_SendSoilSensorCmd(MDBS_GET_MOISTURE);
						SoilSensorPollingState = SS_POLLING_RSP;
						TickPollingInterval = 0 ;			
					break;

				case	SS_POLLING_TEMPERATURE:
						MODBUS_SendSoilSensorCmd(MDBS_GET_TEMPERATURE);
						SoilSensorPollingState = SS_POLLING_RSP;
						TickPollingInterval = 0 ;		
					break;
				
				case	SS_POLLING_EC:
						MODBUS_SendSoilSensorCmd(MDBS_GET_EC);
						SoilSensorPollingState = SS_POLLING_RSP;
						TickPollingInterval = 0 ;			
					break;

				case	SS_POLLING_PH:
						MODBUS_SendSoilSensorCmd(MDBS_GET_PH);
						SoilSensorPollingState = SS_POLLING_RSP;
						TickPollingInterval = 0 ;			
					break;
				
				case	SS_POLLING_NPK_VALUES:
						MODBUS_SendSoilSensorCmd(MDBS_GET_NPK_VALUES);
						SoilSensorPollingState = SS_POLLING_RSP;	
						TickPollingInterval = 0 ;			
					break;

				case	SS_POLLING_SALINITY:
						MODBUS_SendSoilSensorCmd(MDBS_GET_SALINITY);
						SoilSensorPollingState = SS_POLLING_RSP;	
						TickPollingInterval = 0 ;			
					break;

				case	SS_POLLING_TDS:
						MODBUS_SendSoilSensorCmd(MDBS_GET_TDS);
						SoilSensorPollingState = SS_POLLING_RSP;
						TickPollingInterval = 0 ;			
					break;
				
				case	SS_POLLING_FERTILITY:
						MODBUS_SendSoilSensorCmd(MDBS_GET_FERTILITY);
						SoilSensorPollingState = SS_POLLING_RSP;
						TickPollingInterval = 0 ;			
					break;

				case	SS_POLLING_COEFS:
						MODBUS_SendSoilSensorCmd(MDBS_GET_COEFS);
						SoilSensorPollingState = SS_POLLING_RSP;
						TickPollingInterval = 0 ;			
					break;
				
				case	SS_POLLING_CALIB_VALUES:
						MODBUS_SendSoilSensorCmd(MDBS_GET_CALIB_VALUES);
						SoilSensorPollingState = SS_POLLING_RSP;		
						TickPollingInterval = 0 ;			
					break;

				case	SS_POLLING_FERT_COEF:
						MODBUS_SendSoilSensorCmd(MDBS_GET_FERT_COEF);
						SoilSensorPollingState = SS_POLLING_RSP;
						TickPollingInterval = 0 ;			
					break;
				
				case	SS_POLLING_FERT_OFFSET:
						MODBUS_SendSoilSensorCmd(MDBS_GET_FERT_OFFSET);
						SoilSensorPollingState = SS_POLLING_RSP;
						TickPollingInterval = 0 ;
					break;

				case	SS_POLLING_N_COEF:
						MODBUS_SendSoilSensorCmd(MDBS_GET_N_COEF);
						SoilSensorPollingState = SS_POLLING_RSP;
						TickPollingInterval = 0 ;			
					break;
				
				case	SS_POLLING_N_OFFSET:
						MODBUS_SendSoilSensorCmd(MDBS_GET_N_OFFSET);
						SoilSensorPollingState = SS_POLLING_RSP;		
						TickPollingInterval = 0 ;			
					break;

				case	SS_POLLING_P_COEF:
						MODBUS_SendSoilSensorCmd(MDBS_GET_P_COEF);
						SoilSensorPollingState = SS_POLLING_RSP;
						TickPollingInterval = 0 ;			
					break;

				case	SS_POLLING_P_OFFSET:
						MODBUS_SendSoilSensorCmd(MDBS_GET_P_OFFSET);
						SoilSensorPollingState = SS_POLLING_RSP;
						TickPollingInterval = 0 ;			
					break;
				
				case	SS_POLLING_K_COEF:
						MODBUS_SendSoilSensorCmd(MDBS_GET_K_COEF);
						SoilSensorPollingState = SS_POLLING_RSP;
						TickPollingInterval = 0 ;			
					break;

				case	SS_POLLING_K_OFFSET:
						MODBUS_SendSoilSensorCmd(MDBS_GET_K_OFFSET);
						SoilSensorPollingState = SS_POLLING_RSP;
						TickPollingInterval = 0 ;
					break;
				
				case SS_SET_N_VALUE:
						MODBUS_SendSoilSensorCmd(MDBS_SET_N_VALUE);
						SoilSensorPollingState = SS_POLLING_READY;
						break;

				case SS_SET_P_VALUE:
						MODBUS_SendSoilSensorCmd(MDBS_SET_P_VALUE);
						SoilSensorPollingState = SS_POLLING_READY;
						break;

				case SS_SET_K_VALUE:
						MODBUS_SendSoilSensorCmd(MDBS_SET_K_VALUE);
						SoilSensorPollingState = SS_POLLING_READY;
						break;

				case SS_SET_EC_COEF:
						MODBUS_SendSoilSensorCmd(MDBS_SET_EC_COEF);
						SoilSensorPollingState = SS_POLLING_READY;
						break;

				case SS_SET_SALINITY_COEF:
						MODBUS_SendSoilSensorCmd(MDBS_SET_SALINITY_COEF);
						SoilSensorPollingState = SS_POLLING_READY;
						break;

				case SS_SET_TDS_COEF:
						MODBUS_SendSoilSensorCmd(MDBS_SET_TDS_COEF);
						SoilSensorPollingState = SS_POLLING_READY;
						break;

				case SS_SET_TEMP_CALIB:
						MODBUS_SendSoilSensorCmd(MDBS_SET_TEMP_CALIB);
						SoilSensorPollingState = SS_POLLING_READY;
						break;

				case SS_SET_MOISTURE_CALIB:
						MODBUS_SendSoilSensorCmd(MDBS_SET_MOISTURE_CALIB);
						SoilSensorPollingState = SS_POLLING_READY;
						break;

				case SS_SET_EC_CALIB:
						MODBUS_SendSoilSensorCmd(MDBS_SET_EC_CALIB);
						SoilSensorPollingState = SS_POLLING_READY;
						break;

				case SS_SET_PH_CALIB:
						MODBUS_SendSoilSensorCmd(MDBS_SET_PH_CALIB);
						SoilSensorPollingState = SS_POLLING_READY;
						break;

				case SS_SET_FERT_COEF:
						MODBUS_SendSoilSensorCmd(MDBS_SET_FERT_COEF);
						SoilSensorPollingState = SS_POLLING_READY;
						break;

				case SS_SET_FERT_OFFSET:
						MODBUS_SendSoilSensorCmd(MDBS_SET_FERT_OFFSET);
						SoilSensorPollingState = SS_POLLING_READY;
						break;

				case SS_SET_N_COEF:
						MODBUS_SendSoilSensorCmd(MDBS_SET_N_COEF);
						SoilSensorPollingState = SS_POLLING_READY;
						break;

				case SS_SET_N_OFFSET:
						MODBUS_SendSoilSensorCmd(MDBS_SET_N_OFFSET);
						SoilSensorPollingState = SS_POLLING_READY;
						break;

				case SS_SET_P_COEF:
						MODBUS_SendSoilSensorCmd(MDBS_SET_P_COEF);
						SoilSensorPollingState = SS_POLLING_READY;
						break;

				case SS_SET_P_OFFSET:
						MODBUS_SendSoilSensorCmd(MDBS_SET_P_OFFSET);
						SoilSensorPollingState = SS_POLLING_READY;
						break;

				case SS_SET_K_COEF:
						MODBUS_SendSoilSensorCmd(MDBS_SET_K_COEF);
						SoilSensorPollingState = SS_POLLING_READY;
						break;

				case SS_SET_K_OFFSET:
						MODBUS_SendSoilSensorCmd(MDBS_SET_K_OFFSET);
						SoilSensorPollingState = SS_POLLING_READY;
						break;

				case SS_GET_DEVICE_ADDRESS:
						MODBUS_SendSoilSensorCmd(MDBS_GET_SS_ADDRESS);
						SoilSensorPollingState = SS_POLLING_READY;
						break;

				case SS_SET_DEVICE_ADDRESS:
						MODBUS_SendSoilSensorCmd(MDBS_SET_SS_ADDRESS);
						SoilSensorPollingState = SS_POLLING_READY;
						break;

				case SS_GET_BAUD_RATE:
						MODBUS_SendSoilSensorCmd(MDBS_GET_SS_BAUD_RATE);
						SoilSensorPollingState = SS_POLLING_READY;
						break;

				case SS_SET_BAUD_RATE:
						MODBUS_SendSoilSensorCmd(MDBS_SET_SS_BAUD_RATE);
						SoilSensorPollingState = SS_POLLING_READY;
						break;
				default:
						break;
		}
}

/***
 *	@brief			Parse Modbus massage from Soil Sensor
 ***/
void SoilSensorDataProcess(void)
{
		uint8_t SoilSensorArrayIndex;
		uint16_t u16tmp;
		uint32_t u32tmp;
	
		if (GotDeviceRsp != SoilSensorIDArray[PollingSoilSensorID-1]) return;	//	check the host address idx
		
		SoilSensorArrayIndex = PollingSoilSensorID -1;
	
		switch(SoilSensorMBCMD)
		{
        case MDBS_GET_MOISTURE:
            u16tmp = ((TokenMeter[3] << 8) | TokenMeter[4]);
            SoilSensorData[SoilSensorArrayIndex].Moisture = u16tmp;
            break;
            
        case MDBS_GET_TEMPERATURE:
            u16tmp = ((TokenMeter[3] << 8) | TokenMeter[4]);
            SoilSensorData[SoilSensorArrayIndex].Temperature = u16tmp;
            break;

        case MDBS_GET_EC:
            u16tmp = ((TokenMeter[3] << 8) | TokenMeter[4]);
            SoilSensorData[SoilSensorArrayIndex].EC = u16tmp;
            break;

        case MDBS_GET_PH:
            u16tmp = ((TokenMeter[3] << 8) | TokenMeter[4]);
            SoilSensorData[SoilSensorArrayIndex].PH = u16tmp;
            break;

        case MDBS_GET_NPK_VALUES:
            SoilSensorData[SoilSensorArrayIndex].Nitrogen 	= ((TokenMeter[3] << 8) | TokenMeter[4]);
            SoilSensorData[SoilSensorArrayIndex].Phosphorus = ((TokenMeter[5] << 8) | TokenMeter[6]);
            SoilSensorData[SoilSensorArrayIndex].Potassium 	= ((TokenMeter[7] << 8) | TokenMeter[8]);
            break;

        case MDBS_GET_SALINITY:
            u16tmp = ((TokenMeter[3] << 8) | TokenMeter[4]);
            SoilSensorData[SoilSensorArrayIndex].Salinity = u16tmp;
            break;

        case MDBS_GET_TDS:
            u16tmp = ((TokenMeter[3] << 8) | TokenMeter[4]);
            SoilSensorData[SoilSensorArrayIndex].TDS = u16tmp;
            break;

        case MDBS_GET_FERTILITY:
            u16tmp = ((TokenMeter[3] << 8) | TokenMeter[4]);
            SoilSensorData[SoilSensorArrayIndex].Fertility = u16tmp;
            break;
            
        case MDBS_GET_COEFS:
            SoilSensorData[SoilSensorArrayIndex].EC_Coef 	= ((TokenMeter[3] << 8) | TokenMeter[4]);
            SoilSensorData[SoilSensorArrayIndex].Salinity_Coef	= ((TokenMeter[5] << 8) | TokenMeter[6]);
            SoilSensorData[SoilSensorArrayIndex].TDS_Coef 			= ((TokenMeter[7] << 8) | TokenMeter[8]);
            break;
            
        case MDBS_GET_CALIB_VALUES:
            SoilSensorData[SoilSensorArrayIndex].Temp_Calib 		= ((TokenMeter[3] << 8) | TokenMeter[4]);
            SoilSensorData[SoilSensorArrayIndex].Moisture_Calib = ((TokenMeter[5] << 8) | TokenMeter[6]);
            SoilSensorData[SoilSensorArrayIndex].EC_Calib 			= ((TokenMeter[7] << 8) | TokenMeter[8]);
            SoilSensorData[SoilSensorArrayIndex].PH_Calib 			= ((TokenMeter[9] << 8) | TokenMeter[10]);
            break;

        case MDBS_GET_FERT_COEF:
            u32tmp = ((uint32_t)TokenMeter[3] << 24) | ((uint32_t)TokenMeter[4] << 16) | ((uint32_t)TokenMeter[5] << 8) | TokenMeter[6];
            SoilSensorData[SoilSensorArrayIndex].Fert_Coef = *((float*)&u32tmp);
            break;

        case MDBS_GET_FERT_OFFSET:
            SoilSensorData[SoilSensorArrayIndex].Fert_Deviation = (int16_t)((TokenMeter[3] << 8) | TokenMeter[4]);
            break;

        case MDBS_GET_N_COEF:
            u32tmp = ((uint32_t)TokenMeter[3] << 24) | ((uint32_t)TokenMeter[4] << 16) | ((uint32_t)TokenMeter[5] << 8) | TokenMeter[6];
            SoilSensorData[SoilSensorArrayIndex].Nitrogen_Coef = *((float*)&u32tmp);
            break;

        case MDBS_GET_N_OFFSET:
            SoilSensorData[SoilSensorArrayIndex].Nitrogen_Deviation = (int16_t)((TokenMeter[3] << 8) | TokenMeter[4]);
            break;
            
        case MDBS_GET_P_COEF:
            u32tmp = ((uint32_t)TokenMeter[3] << 24) | ((uint32_t)TokenMeter[4] << 16) | ((uint32_t)TokenMeter[5] << 8) | TokenMeter[6];
            SoilSensorData[SoilSensorArrayIndex].Phosphorus_Coef = *((float*)&u32tmp);
            break;

        case MDBS_GET_P_OFFSET:
            SoilSensorData[SoilSensorArrayIndex].Phosphorus_Deviation = (int16_t)((TokenMeter[3] << 8) | TokenMeter[4]);
            break;
            
        case MDBS_GET_K_COEF:
            u32tmp = ((uint32_t)TokenMeter[3] << 24) | ((uint32_t)TokenMeter[4] << 16) | ((uint32_t)TokenMeter[5] << 8) | TokenMeter[6];
            SoilSensorData[SoilSensorArrayIndex].Potassium_Coef = *((float*)&u32tmp);
            break;

        case MDBS_GET_K_OFFSET:
            SoilSensorData[SoilSensorArrayIndex].Potassium_Deviation = (int16_t)((TokenMeter[3] << 8) | TokenMeter[4]);
            break;

        default:
            break;
		}
}		

/***
 *	@brief	Send pyranometer modbus cmd
 ***/
void MODBUS_SendSoilSensorCmd(uint8_t cmd)
{
		MeterTxBuffer[0] = SoilSensorIDArray[PollingSoilSensorID-1]; 
		//	To determine if Device respond.
		GotDeviceRsp = 0xFF ;
		CmdModbus_SoilSensor_SN3002TR(cmd);		
}

/***
 *	@brief	Send pyranometer modbus cmd
 *	@Read		Solar radiation, Offset value, Device addr, Baud rate
 *	@Set		Offset value, Device addr, Baud rate
 ***/
void CmdModbus_SoilSensor_SN3002TR(uint8_t ModbusCmd)
{
		switch(ModbusCmd)
		{				
				case MDBS_GET_MOISTURE:
						SoilSensorMBCMD = MDBS_GET_MOISTURE;
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
				
				case MDBS_GET_TEMPERATURE:
						SoilSensorMBCMD = MDBS_GET_TEMPERATURE;
						MeterTxBuffer[1] = 0x03;
						MeterTxBuffer[2] = 0x00;
						MeterTxBuffer[3] = 0x01;
						MeterTxBuffer[4] = 0x00;
						MeterTxBuffer[5] = 0x01;
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;		

				case MDBS_GET_EC:
						SoilSensorMBCMD = MDBS_GET_EC;
						MeterTxBuffer[1] = 0x03;
						MeterTxBuffer[2] = 0x00;
						MeterTxBuffer[3] = 0x02;
						MeterTxBuffer[4] = 0x00;
						MeterTxBuffer[5] = 0x01;
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;					

				case MDBS_GET_PH:
						SoilSensorMBCMD = MDBS_GET_PH;
						MeterTxBuffer[1] = 0x03; 
						MeterTxBuffer[2] = 0x00; 
						MeterTxBuffer[3] = 0x03; 
						MeterTxBuffer[4] = 0x00; 
						MeterTxBuffer[5] = 0x01; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;

				case MDBS_GET_NPK_VALUES:
						SoilSensorMBCMD = MDBS_GET_NPK_VALUES;
						MeterTxBuffer[1] = 0x03; 
						MeterTxBuffer[2] = 0x00; 
						MeterTxBuffer[3] = 0x04; 
						MeterTxBuffer[4] = 0x00; 
						MeterTxBuffer[5] = 0x03; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;
				
				case MDBS_GET_SALINITY:
						SoilSensorMBCMD = MDBS_GET_SALINITY;
						MeterTxBuffer[1] = 0x03; 
						MeterTxBuffer[2] = 0x00; 
						MeterTxBuffer[3] = 0x07; 
						MeterTxBuffer[4] = 0x00; 
						MeterTxBuffer[5] = 0x01; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;
				
				case MDBS_GET_TDS:
						SoilSensorMBCMD = MDBS_GET_TDS;
						MeterTxBuffer[1] = 0x03; 
						MeterTxBuffer[2] = 0x00; 
						MeterTxBuffer[3] = 0x08;
						MeterTxBuffer[4] = 0x00; 
						MeterTxBuffer[5] = 0x01; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;
				case MDBS_GET_FERTILITY:
						SoilSensorMBCMD = MDBS_GET_FERTILITY;
						MeterTxBuffer[1] = 0x03; 
						MeterTxBuffer[2] = 0x00; 
						MeterTxBuffer[3] = 0x0c;
						MeterTxBuffer[4] = 0x00; 
						MeterTxBuffer[5] = 0x01; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;
				
				case MDBS_GET_COEFS:
						SoilSensorMBCMD = MDBS_GET_COEFS;
						MeterTxBuffer[1] = 0x03; 
						MeterTxBuffer[2] = 0x00; 
						MeterTxBuffer[3] = 0x22;
						MeterTxBuffer[4] = 0x00; 
						MeterTxBuffer[5] = 0x03; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;
				
				//	TEMP, MOISTURE, EC, PH Calibreation Values
				case MDBS_GET_CALIB_VALUES:
						SoilSensorMBCMD = MDBS_GET_CALIB_VALUES;
						MeterTxBuffer[1] = 0x03; 
						MeterTxBuffer[2] = 0x00; 
						MeterTxBuffer[3] = 0x50;
						MeterTxBuffer[4] = 0x00; 
						MeterTxBuffer[5] = 0x04; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;
				
				case MDBS_GET_FERT_COEF:
						SoilSensorMBCMD = MDBS_GET_FERT_COEF;
						MeterTxBuffer[1] = 0x03; 
						MeterTxBuffer[2] = 0x04; 
						MeterTxBuffer[3] = 0xE5;
						MeterTxBuffer[4] = 0x00; 
						MeterTxBuffer[5] = 0x02; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;				
				
				case MDBS_GET_FERT_OFFSET:
						SoilSensorMBCMD = MDBS_GET_FERT_OFFSET;
						MeterTxBuffer[1] = 0x03; 
						MeterTxBuffer[2] = 0x04; 
						MeterTxBuffer[3] = 0xE7;
						MeterTxBuffer[4] = 0x00; 
						MeterTxBuffer[5] = 0x01; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;
				
				case MDBS_GET_N_COEF:
						SoilSensorMBCMD = MDBS_GET_N_COEF;
						MeterTxBuffer[1] = 0x03; 
						MeterTxBuffer[2] = 0x04; 
						MeterTxBuffer[3] = 0xE8;
						MeterTxBuffer[4] = 0x00; 
						MeterTxBuffer[5] = 0x02; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;
				
				case MDBS_GET_N_OFFSET:
						SoilSensorMBCMD = MDBS_GET_N_OFFSET;
						MeterTxBuffer[1] = 0x03; 
						MeterTxBuffer[2] = 0x04; 
						MeterTxBuffer[3] = 0xEA;
						MeterTxBuffer[4] = 0x00; 
						MeterTxBuffer[5] = 0x01; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;

				case MDBS_GET_P_COEF:
						SoilSensorMBCMD = MDBS_GET_P_COEF;
						MeterTxBuffer[1] = 0x03; 
						MeterTxBuffer[2] = 0x04; 
						MeterTxBuffer[3] = 0xF2;
						MeterTxBuffer[4] = 0x00; 
						MeterTxBuffer[5] = 0x02; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;
				
				case MDBS_GET_P_OFFSET:
						SoilSensorMBCMD = MDBS_GET_P_OFFSET;
						MeterTxBuffer[1] = 0x03; 
						MeterTxBuffer[2] = 0x04; 
						MeterTxBuffer[3] = 0xF4;
						MeterTxBuffer[4] = 0x00; 
						MeterTxBuffer[5] = 0x01; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;
				
				case MDBS_GET_K_COEF:
						SoilSensorMBCMD = MDBS_GET_K_COEF;
						MeterTxBuffer[1] = 0x03; 
						MeterTxBuffer[2] = 0x04; 
						MeterTxBuffer[3] = 0xFC;
						MeterTxBuffer[4] = 0x00; 
						MeterTxBuffer[5] = 0x02; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;
				
				case MDBS_GET_K_OFFSET:
						SoilSensorMBCMD = MDBS_GET_K_OFFSET;
						MeterTxBuffer[1] = 0x03; 
						MeterTxBuffer[2] = 0x04; 
						MeterTxBuffer[3] = 0xFE;
						MeterTxBuffer[4] = 0x00; 
						MeterTxBuffer[5] = 0x01; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;

				case MDBS_SET_N_VALUE:
						SoilSensorMBCMD = MDBS_SET_N_VALUE;
						MeterTxBuffer[1] = 0x06; 
						MeterTxBuffer[2] = 0x00; 
						MeterTxBuffer[3] = 0x04;
						MeterTxBuffer[4] = (N_RegValue >> 8) & 0xff; 
						MeterTxBuffer[5] = N_RegValue & 0xff; ; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;

				case MDBS_SET_P_VALUE:
						SoilSensorMBCMD = MDBS_SET_P_VALUE;
						MeterTxBuffer[1] = 0x06; 
						MeterTxBuffer[2] = 0x00; 
						MeterTxBuffer[3] = 0x05;
						MeterTxBuffer[4] = (P_RegValue >> 8) & 0xff; 
						MeterTxBuffer[5] = P_RegValue & 0xff; ; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;				

				case MDBS_SET_K_VALUE:
						SoilSensorMBCMD = MDBS_SET_K_VALUE;
						MeterTxBuffer[1] = 0x06; 
						MeterTxBuffer[2] = 0x00; 
						MeterTxBuffer[3] = 0x06;
						MeterTxBuffer[4] = (K_RegValue >> 8) & 0xff; 
						MeterTxBuffer[5] = K_RegValue & 0xff; ; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;
				
				case MDBS_SET_EC_COEF:
						SoilSensorMBCMD = MDBS_SET_EC_COEF;
						MeterTxBuffer[1] = 0x06; 
						MeterTxBuffer[2] = 0x00; 
						MeterTxBuffer[3] = 0x22;
						MeterTxBuffer[4] = (Ec_COEF >> 8) & 0xff; 
						MeterTxBuffer[5] = Ec_COEF & 0xff; ; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;
				
				case MDBS_SET_SALINITY_COEF:
						SoilSensorMBCMD = MDBS_SET_SALINITY_COEF;
						MeterTxBuffer[1] = 0x06; 
						MeterTxBuffer[2] = 0x00; 
						MeterTxBuffer[3] = 0x23;
						MeterTxBuffer[4] = (Salinity_COEF >> 8) & 0xff; 
						MeterTxBuffer[5] = Salinity_COEF & 0xff; ; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;

				case MDBS_SET_TDS_COEF:
						SoilSensorMBCMD = MDBS_SET_TDS_COEF;
						MeterTxBuffer[1] = 0x06; 
						MeterTxBuffer[2] = 0x00; 
						MeterTxBuffer[3] = 0x24;
						MeterTxBuffer[4] = (Tds_COEF >> 8) & 0xff; 
						MeterTxBuffer[5] = Tds_COEF & 0xff; ; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;
				
				case MDBS_SET_TEMP_CALIB:
						SoilSensorMBCMD = MDBS_SET_TEMP_CALIB;
						MeterTxBuffer[1] = 0x06; 
						MeterTxBuffer[2] = 0x00; 
						MeterTxBuffer[3] = 0x50;
						MeterTxBuffer[4] = (Temp_CalibValue >> 8) & 0xff; 
						MeterTxBuffer[5] = Temp_CalibValue & 0xff; ; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;
				
				case MDBS_SET_MOISTURE_CALIB:
						SoilSensorMBCMD = MDBS_SET_MOISTURE_CALIB;
						MeterTxBuffer[1] = 0x06; 
						MeterTxBuffer[2] = 0x00; 
						MeterTxBuffer[3] = 0x51;
						MeterTxBuffer[4] = (Moisture_CalibValue >> 8) & 0xff; 
						MeterTxBuffer[5] = Moisture_CalibValue & 0xff; ; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;

				case MDBS_SET_EC_CALIB:
						SoilSensorMBCMD = MDBS_SET_EC_CALIB;
						MeterTxBuffer[1] = 0x06; 
						MeterTxBuffer[2] = 0x00; 
						MeterTxBuffer[3] = 0x52;
						MeterTxBuffer[4] = (Ec_CalibValue >> 8) & 0xff; 
						MeterTxBuffer[5] = Ec_CalibValue & 0xff; ; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;

				case MDBS_SET_PH_CALIB:
						SoilSensorMBCMD = MDBS_SET_PH_CALIB;
						MeterTxBuffer[1] = 0x06; 
						MeterTxBuffer[2] = 0x00; 
						MeterTxBuffer[3] = 0x53;
						MeterTxBuffer[4] = (Ph_CalibValue >> 8) & 0xff; 
						MeterTxBuffer[5] = Ph_CalibValue & 0xff; ; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;
				
				case MDBS_SET_FERT_COEF:
						SoilSensorMBCMD = MDBS_SET_FERT_COEF;
						MeterTxBuffer[1] = 0x10; 
						MeterTxBuffer[2] = 0x04; 
						MeterTxBuffer[3] = 0xE5;
						MeterTxBuffer[4] = 0x00; 
						MeterTxBuffer[5] = 0x02;
						MeterTxBuffer[6] = 0x04;
						MeterTxBuffer[7] = (Fert_COEF >> 24) & 0xff; 
						MeterTxBuffer[8] = (Fert_COEF >> 16) & 0xff; 
						MeterTxBuffer[9] = (Fert_COEF >> 8)  & 0xff; 
						MeterTxBuffer[10] =(Fert_COEF )      & 0xff; 				
						CRC16(MeterTxBuffer, 11);
						MeterTxBuffer[11] = uchCRCHi;
						MeterTxBuffer[12] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 13);
						break;
				
					case MDBS_SET_FERT_OFFSET:
						SoilSensorMBCMD = MDBS_SET_FERT_OFFSET;
						MeterTxBuffer[1] = 0x06; 
						MeterTxBuffer[2] = 0x04; 
						MeterTxBuffer[3] = 0xE7;
						MeterTxBuffer[4] = (Fert_Offset >> 8) & 0xff; 
						MeterTxBuffer[5] = Fert_Offset & 0xff; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;			

				case MDBS_SET_N_COEF:
						SoilSensorMBCMD = MDBS_SET_N_COEF;
						MeterTxBuffer[1] = 0x10; 
						MeterTxBuffer[2] = 0x04; 
						MeterTxBuffer[3] = 0xE8;
						MeterTxBuffer[4] = 0x00; 
						MeterTxBuffer[5] = 0x02;
						MeterTxBuffer[6] = 0x04;
						MeterTxBuffer[7] = (N_COEF >> 24) & 0xff; 
						MeterTxBuffer[8] = (N_COEF >> 16) & 0xff; 
						MeterTxBuffer[9] = (N_COEF >> 8)  & 0xff; 
						MeterTxBuffer[10] =(N_COEF )      & 0xff; 				
						CRC16(MeterTxBuffer, 11);
						MeterTxBuffer[11] = uchCRCHi;
						MeterTxBuffer[12] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 13);
						break;
				
					case MDBS_SET_N_OFFSET:
						SoilSensorMBCMD = MDBS_SET_N_OFFSET;
						MeterTxBuffer[1] = 0x06; 
						MeterTxBuffer[2] = 0x04; 
						MeterTxBuffer[3] = 0xEA;
						MeterTxBuffer[4] = (N_Offset >> 8) & 0xff; 
						MeterTxBuffer[5] = N_Offset & 0xff; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;	

				case MDBS_SET_P_COEF:
						SoilSensorMBCMD = MDBS_SET_P_COEF;
						MeterTxBuffer[1] = 0x10; 
						MeterTxBuffer[2] = 0x04; 
						MeterTxBuffer[3] = 0xF2;
						MeterTxBuffer[4] = 0x00; 
						MeterTxBuffer[5] = 0x02;
						MeterTxBuffer[6] = 0x04;
						MeterTxBuffer[7] = (P_COEF >> 24) & 0xff; 
						MeterTxBuffer[8] = (P_COEF >> 16) & 0xff; 
						MeterTxBuffer[9] = (P_COEF >> 8)  & 0xff; 
						MeterTxBuffer[10] =(P_COEF )      & 0xff; 				
						CRC16(MeterTxBuffer, 11);
						MeterTxBuffer[11] = uchCRCHi;
						MeterTxBuffer[12] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 13);
						break;
				
					case MDBS_SET_P_OFFSET:
						SoilSensorMBCMD = MDBS_SET_P_OFFSET;
						MeterTxBuffer[1] = 0x06; 
						MeterTxBuffer[2] = 0x04; 
						MeterTxBuffer[3] = 0xF4;
						MeterTxBuffer[4] = (P_Offset >> 8) & 0xff; 
						MeterTxBuffer[5] = P_Offset & 0xff; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;	
					
				case MDBS_SET_K_COEF:
						SoilSensorMBCMD = MDBS_SET_K_COEF;
						MeterTxBuffer[1] = 0x10; 
						MeterTxBuffer[2] = 0x04; 
						MeterTxBuffer[3] = 0xFC;
						MeterTxBuffer[4] = 0x00; 
						MeterTxBuffer[5] = 0x02;
						MeterTxBuffer[6] = 0x04;
						MeterTxBuffer[7] = (K_COEF >> 24) & 0xff; 
						MeterTxBuffer[8] = (K_COEF >> 16) & 0xff; 
						MeterTxBuffer[9] = (K_COEF >> 8)  & 0xff; 
						MeterTxBuffer[10] =(K_COEF )      & 0xff; 				
						CRC16(MeterTxBuffer, 11);
						MeterTxBuffer[11] = uchCRCHi;
						MeterTxBuffer[12] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 13);
						break;
				
					case MDBS_SET_K_OFFSET:
						SoilSensorMBCMD = MDBS_SET_K_OFFSET;
						MeterTxBuffer[1] = 0x06; 
						MeterTxBuffer[2] = 0x04; 
						MeterTxBuffer[3] = 0xFE;
						MeterTxBuffer[4] = (K_Offset >> 8) & 0xff; 
						MeterTxBuffer[5] = K_Offset & 0xff; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;						
					
				case MDBS_GET_SS_ADDRESS:
						SoilSensorMBCMD = MDBS_GET_SS_ADDRESS;
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
				
				case MDBS_SET_SS_ADDRESS:
						SoilSensorMBCMD = MDBS_SET_SS_ADDRESS;
						MeterTxBuffer[1] = 0x06; 
						MeterTxBuffer[2] = 0x07; 
						MeterTxBuffer[3] = 0xD0;
						MeterTxBuffer[4] = (SSNewAddr >> 8) & 0xff; 
						MeterTxBuffer[5] = SSNewAddr & 0xff; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;
				
				case MDBS_GET_SS_BAUD_RATE:
						SoilSensorMBCMD = MDBS_GET_SS_BAUD_RATE;
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
	
				case MDBS_SET_SS_BAUD_RATE:
						SoilSensorMBCMD = MDBS_SET_SS_BAUD_RATE;
						MeterTxBuffer[1] = 0x06; 
						MeterTxBuffer[2] = 0x07; 
						MeterTxBuffer[3] = 0xD1;
						MeterTxBuffer[4] = 0x00; 
						//	baud rate: 0x00 -> 2400 bmp, 0x01 -> 4800 bmp, 0x02 -> 9600 bmp
						MeterTxBuffer[5] = SSNewBaudRate; 
						CRC16(MeterTxBuffer, 6);
						MeterTxBuffer[6] = uchCRCHi;
						MeterTxBuffer[7] = uchCRCLo;
						_SendStringToMETER(MeterTxBuffer, 8);
						break;				
				default :
						SoilSensorMBCMD = MDBS_SoilSensor_OTHER;
						break;
		}
}

void SoilSensorSuccess(void)
{
		uint8_t SoilSensorArrayIndex;
		SoilSensorArrayIndex = PollingSoilSensorID -1;

		SoilSensorError.Success[SoilSensorArrayIndex] += 1;
		SoilSensorError.SSDeviceNG &= (~(0x00000001 << (SoilSensorArrayIndex)));
		//	Move to next pollingstate
    SoilSensorPollingStateIndex++;
		SoilSensorPollingState = SS_POLLING_READY;			
}

void SoilSensorTimeoutProcess(void)
{
		uint8_t SoilSensorArrayIndex;
		SoilSensorArrayIndex = PollingSoilSensorID -1;	
	
    if ( TickPollingInterval > POLL_TIMEOUT )
    {							
        SoilSensorReadErrorCnt++;
				//	Error report system    
				SoilSensorError.Fail[SoilSensorArrayIndex] += 1;
				SoilSensorPollingState = SS_POLLING_READY;
        if( SoilSensorReadErrorCnt > MAX_POLL_RETRY_TIMES )
        {
						SoilSensorError.SSDeviceNG |= (0x00000001 << (SoilSensorArrayIndex));
            PollingSoilSensorID++;
            SoilSensorReadErrorCnt = 0 ;            
            ResetMeterUART();
        }
    }
}
