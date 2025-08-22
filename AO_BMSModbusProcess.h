#ifndef	__AO_BMSMODBUSPROCESS__H__
#define	__AO_BMSMODBUSPROCESS__H__

#include	"stdint.h"
#include	"AO_ExternFunc.h"

extern void BmsPolling(void);
extern void BmsDataProcess(void);
//	DeviceRsp

extern _Bool BmsPollingFinishedFlag;
extern uint8_t PollingBmsID;

#define POLL_TIMEOUT 			50
#define POLL_ERROR_TIMES	5

#define MODBUS_FLAG_HEAT_ENABLE             (1 << 0)    // BIT0: Heat enable
#define MODBUS_FLAG_DISABLE_TEMP_SENSOR     (1 << 1)    // BIT1: Disable heat sensor
#define MODBUS_FLAG_GPS_HEARTBEAT           (1 << 2)    // BIT2: GPS heartbeet
#define MODBUS_FLAG_PORT_SWITCH             (1 << 3)    // BIT3: Port switch (1: RS485; 0: CAN)
#define MODBUS_FLAG_LCD_ALWAYS_ON           (1 << 4)    // BIT4: LCD always on
#define MODBUS_FLAG_SPECIAL_CHARGER         (1 << 5)    // BIT5: Special charger 
#define MODBUS_FLAG_SMART_SLEEP             (1 << 6)    // BIT6: Smart sleep
#define MODBUS_FLAG_DISABLE_PCL_MODULE      (1 << 7)    // BIT7: Disable PCL modeule
#define MODBUS_FLAG_TIMED_STORED_DATA       (1 << 8)    // BIT8: Timed stored data
#define MODBUS_FLAG_CHARGING_FLOAT_MODE     (1 << 9)    // BIT9: Charging float mode

#define ALARM_WIRE_RES                  (1 << 0)    // BIT0: 均衡線電阻過大 (AlarmWireRes)
#define ALARM_MOS_OTP                   (1 << 1)    // BIT1: MOS 過溫保護 (AlarmMosOTP)
#define ALARM_CELL_QUANTITY             (1 << 2)    // BIT2: 單體數量與設置值不符 (AlarmCellQuantity)
#define ALARM_CUR_SENSOR_ERR            (1 << 3)    // BIT3: 電流感測器異常 (AlarmCurSensorErr)
#define ALARM_CELL_OVP                  (1 << 4)    // BIT4: 單體過壓保護 (AlarmCellOVP)
#define ALARM_BAT_OVP                   (1 << 5)    // BIT5: 電池過壓保護 (AlarmBatOVP)
#define ALARM_CH_OCP                    (1 << 6)    // BIT6: 充電過流保護 (AlarmChOCP)
#define ALARM_CH_SCP                    (1 << 7)    // BIT7: 充電短路保護 (AlarmChSCP)
#define ALARM_CH_OTP                    (1 << 8)    // BIT8: 充電過溫保護 (AlarmChOTP)
#define ALARM_CH_UTP                    (1 << 9)    // BIT9: 充電低溫保護 (AlarmChUTP)
#define ALARM_CPU_AUX_COMMU_ERR         (1 << 10)   // BIT10: 內部通訊異常 (AlarmCPUAuxCommuErr)
#define ALARM_CELL_UVP                  (1 << 11)   // BIT11: 單體欠壓保護 (AlarmCellUVP)
#define ALARM_BAT_UVP                   (1 << 12)   // BIT12: 電池欠壓保護 (AlarmBatUVP)
#define ALARM_DCH_OCP                   (1 << 13)   // BIT13: 放電過流保護 (AlarmDchOCP)
#define ALARM_DCH_SCP                   (1 << 14)   // BIT14: 放電短路保護 (AlarmDchSCP)
#define ALARM_DCH_OTP                   (1 << 15)   // BIT15: 放電過溫保護 (AlarmDchOTP)
#define ALARM_CHARGE_MOS                (1 << 16)   // BIT16: 充電管異常 (AlarmChargeMOS)
#define ALARM_DISCHARGE_MOS             (1 << 17)   // BIT17: 放電管異常 (AlarmDischargeMOS)
#define ALARM_GPS_DISCONNECTED          (1 << 18)   // BIT18: GPS 斷開連接 (GPSDisconneted)
#define ALARM_MODIFY_PWD                (1 << 19)   // BIT19: 請及時修改授權密碼 (Modify PWD. in time)
#define ALARM_DISCHARGE_ON_FAILED       (1 << 20)   // BIT20: 放電開啟失敗 (Discharge On Failed)
#define ALARM_BATTERY_OVER_TEMP         (1 << 21)   // BIT21: 電池超溫警報 (Battery Over Temp Alarm)

enum Define_Bms_Polling_State
{
		BMS_POLLING_READY,
		BMS_POLLING_RSP,
		//	Polling cmds below
		BMS_POLLING_CELL_STATUS,
		BMS_POLLING_CELL_V_CMD,
		
		BMS_POLLING_BAT_W_CMD,
		BMS_POLLING_BAT_V_CMD,
		BMS_POLLING_BAT_I_CMD,

		BMS_POLLING_BALANCE_STATUS_CMD,		
		BMS_POLLING_BAT_SOH_CMD,
		BMS_POLLING_CHARGING_STATUS_CMD,
		BMS_POLLING_WARNING_CMD,
	
		BMS_POLLING_MOS_TEMP_CMD,
		BMS_POLLING_BAT_TEMP1_CMD,
		BMS_POLLING_BAT_TEMP2_CMD,
		BMS_POLLING_BAT_TEMP3_CMD,
		BMS_POLLING_BAT_TEMP4_CMD,
		BMS_POLLING_BAT_TEMP5_CMD,	//	Last polling cmd
	
		//	Cmds are down below
		BMS_SET_ADDR,
		BMS_SET_CHARGE_MODE,
		BMS_SET_DISCHARGE_MODE,		
		BMS_SET_BALANCE_MODE,
		BMS_SET_FUNC_MODES,
		BMS_SET_CELLCOUNT,
};

enum DefineModbusBMS_Cmds
{
	
		/** 	Read 	**/
		MDBS_GET_CELL_V,		//Cells
		MDBS_GET_CELL_STATUS,		
	
		MDBS_GET_BATTERY_WATT,	
		MDBS_GET_BATTERY_V,	//Battery
		MDBS_GET_BATTERY_I,
	
		MDBS_GET_MOS_TEMP,
		MDBS_GET_BATTERY_TEMP1,
		MDBS_GET_BATTERY_TEMP2,
		MDBS_GET_BATTERY_TEMP3,
		MDBS_GET_BATTERY_TEMP4,
		MDBS_GET_BATTERY_TEMP5,
		
		MDBS_GET_BMS_ALARM,
		MDBS_GET_BALANCE_CURRENT,
		
		MDBS_GET_SOC_STATUS,
		MDBS_GET_SOC_STATUS_REMAIN,		//	State Of Charge
		MDBS_GET_SOC_STATUS_CAPACITY,
		MDBS_GET_SOC_STATUS_CYCLE_COUNT,
		
		MDBS_GET_SOH,
		MDBS_GET_CHARGE_DISCHARGE_STATUS,
		
		//	Write
		MDBS_SET_BMS_DEVICE_ADDR,
		MDBS_SET_BATTERY_CHARGEING_STATUS,
		MDBS_SET_BATTERY_DISCHARGING_STATUS,
		MDBS_SET_BATTERY_BALANCE_STATUS,
		MDBS_SET_MODES,
		MDBS_SET_CELLCOUNT,
		
};



typedef struct {
	uint32_t BmsDeviceNG;
	uint8_t Success[PwrMeterMax];
	uint8_t Fail[PwrMeterMax];
	uint8_t ErrorRate;
} BmsError_t;

typedef struct {
	
		uint32_t CellStatus;
		uint16_t CellVolt[16];
		
		uint32_t BatWatt;
		uint32_t BatVolt;
		uint32_t BatCurrent;
	
		uint16_t MosTemp;
		uint16_t BatteryTemp[5];
	
		uint8_t StateOfCharge;
		uint8_t BalanceStatus;
		uint8_t	StateOfHealth;
		_Bool PrechargeStatus;
		_Bool ChargeState;
		_Bool DischargeState;
		
} BmsData_t;

extern BmsError_t BmsError;
extern BmsData_t 	BmsData[PwrMeterMax];

#endif	//	AO_BMSModbusProcess.h