#ifndef __OTA_MANAGER_H__
#define	__OTA_MANAGER_H__

#include "fmc.h"
#include "fmc_user.h"
#include "stdbool.h"
#include "string.h"
#include "stddef.h"

#define Active	0
#define	Backup	1
#define MaxBankCnt	2

// Firmware flags
#define Fw_InvalidFlag          (1 << 0)
#define Fw_ValidFlag            (1 << 1)
#define Fw_PendingFlag          (1 << 2)
#define Fw_ActiveFlag           (1 << 3)
#define Fw_MeterFwFlag		 			(1 << 4)


// Memory addresses
#define BANK1_BASE             0x00002000
#define BANK2_BASE             0x00010000
#define BANK_STATUS_BASE       0x0001E800
#define BANK1_META_BASE        0x0001F000
#define BANK2_META_BASE        0x0001F800

 enum {
	 //Bootloader Cmds
	 BTLD_CLEAR_CMD = 0,
	 BTLD_UPDATE_CENTER,
	 BTLD_UPDATE_METER,	 
	 
	 //	FwBank status
	 DUAL_FW_BANK_VALID = 0x10,
	 ACTIVE_FW_BANK_VALID,
	 BACKUP_FW_BANK_VALID,
	 METER_OTA_UPDATEING,
	 ALL_FW_BANK_INVALID
	 
 } ;



typedef struct __attribute__((packed)) {
    uint32_t flags;
    uint32_t fw_crc32;
    uint32_t fw_version;
    uint32_t fw_start_addr;
    uint32_t fw_size;
    uint32_t trial_counter;
    uint16_t WDTRst_counter;
		uint16_t MeterID;
    uint32_t meta_crc;
} FwMeta;


typedef struct {
    uint32_t Fw_Base;
    uint32_t Fw_Meta_Base;
    uint16_t status;
		uint16_t Cmd;
	  uint32_t FwVersion;
} FwStatus;


extern void BlinkLEDs();

extern void FwValidator(void);
extern void JumpToBootloader();

extern void Get_DualBankStatus(FwStatus *ctx, FwMeta *active, FwMeta *backup);
extern void FwBankSwitchProcess(_Bool BackupValid);

extern _Bool IsFwValid(FwMeta * Meta);
extern int  WriteFwStatus(FwStatus *status);
extern int  WriteMetadata(FwMeta *meta, uint32_t meta_base);
extern int  WriteToFlash(void *data, uint32_t size, uint32_t base_addr, bool with_crc32, uint32_t crc_offset);
extern void BlinkStatusLED(GPIO_T *port, uint32_t pin, uint8_t times, uint32_t delay_ms);
extern uint32_t CRC32_Calc(const uint8_t *pData, uint32_t len) ;

extern FwStatus BankStatus;
extern FwMeta BankMeta[MaxBankCnt];

extern bool g_fw_metadata_ready;

#endif