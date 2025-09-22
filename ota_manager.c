#include "ota_manager.h"
#include "AO_MyDef.h"

FwStatus BankStatus;
FwMeta BankMeta[MaxBankCnt] = {0};

bool g_fw_metadata_ready = false;



void FwValidator(void);
void JumpToBootloader();
void WRITE_FW_STATUS_FLAG(uint32_t flag);

void FwBankSwitchProcess(_Bool BackupValid);
void Get_DualBankStatus(FwStatus *ctx, FwMeta *active, FwMeta *backup);

_Bool IsFwValid(FwMeta * Meta);
_Bool FwCheck_CRC(FwMeta *meta);
void SetFwFlags(FwMeta *meta, bool active, bool valid);
void Update_FwMetadata(bool activeValid, bool backupValid);
void Update_BankStatus(bool activeValid, bool backupValid);

int  WriteFwStatus(FwStatus *status);
int  WriteMetadata(FwMeta *meta, uint32_t meta_base);
int  WriteToFlash(void *data, uint32_t size, uint32_t base_addr, bool with_crc32, uint32_t crc_offset);
void BlinkLEDs();
void BlinkStatusLED(GPIO_T *port, uint32_t pin, uint8_t times, uint32_t delay_ms);
uint32_t CRC32_Calc(const uint8_t *pData, uint32_t len) ;

/***
 *	@brief	Handles the Fw validate process of Boot Initialization
 ***/
void FwValidator(void)
{
		SYS_UnlockReg();
		FMC_ENABLE_ISP();
	
		Get_DualBankStatus(&BankStatus, &BankMeta[Active], &BankMeta[Backup]);
		uint32_t BackupBank_addr = (BankStatus.Fw_Meta_Base == BANK1_META_BASE) ? BANK2_META_BASE : BANK1_META_BASE;
	
    _Bool activeValid = IsFwValid((FwMeta *)&BankMeta[Active]);
		_Bool backupValid = IsFwValid((FwMeta *)&BankMeta[Backup]);
	
		Update_FwMetadata(activeValid, backupValid);
		Update_BankStatus(activeValid, backupValid);
	
		BlinkStatusLED( (activeValid) ? PD : PF, (activeValid) ? 7 : 2, 10, 750);
	
		if (BankStatus.status == ALL_FW_BANK_INVALID) {
				BankStatus.Cmd = BTLD_CLEAR_CMD;
				WriteFwStatus(&BankStatus);
				JumpToBootloader();
    } else if (BankStatus.status == BACKUP_FW_BANK_VALID){
				FwBankSwitchProcess(backupValid);
		}	
		
		BankStatus.Cmd = BTLD_CLEAR_CMD;

		WriteFwStatus(&BankStatus);
    WriteMetadata(&BankMeta[Active], BankStatus.Fw_Meta_Base);
    WriteMetadata(&BankMeta[Backup], BackupBank_addr);	
		
		SYS_LockReg();
}

/***
 *	@brief	Get Bankstatus & Metadata from memory
 ***/
void Get_DualBankStatus(FwStatus *ctx, FwMeta *active, FwMeta *backup)
{
		/**	Get Meta Infos	**/
    ReadData(BANK_STATUS_BASE, BANK_STATUS_BASE + sizeof(FwStatus), (uint32_t *)ctx);
    uint32_t addr[2] = { BANK1_META_BASE, BANK2_META_BASE };
    uint32_t idx = (ctx->Fw_Meta_Base == BANK1_META_BASE) ? 0 : 1;
    ReadData(addr[idx], addr[idx] + sizeof(FwMeta), (uint32_t *)active);
    ReadData(addr[1 - idx], addr[1 - idx] + sizeof(FwMeta), (uint32_t *)backup);
}

/***
 *	@brief	Fw Bank Switch Proc. according to backup bank validility
 ***/
void FwBankSwitchProcess(_Bool BackupValid)
{
		uint32_t 	BackupBank_addr = (BankStatus.Fw_Meta_Base == BANK1_META_BASE) ? BANK2_META_BASE : BANK1_META_BASE;
	
		if (BackupValid){
				SetFwFlags(&BankMeta[Active], false, true);
				SetFwFlags(&BankMeta[Backup], true, BackupValid);
				WriteMetadata(&BankMeta[Active], BankStatus.Fw_Meta_Base);
				WriteMetadata(&BankMeta[Backup], BackupBank_addr);					
		} else {
				BankStatus.Cmd = BTLD_UPDATE_METER;
				WriteFwStatus(&BankStatus);
				JumpToBootloader();
		}
}

/***
 *	@brief	Set Meta flags up according to the Fw validation result
 ***/
void Update_FwMetadata(bool activeValid, bool backupValid)
{

		SetFwFlags(&BankMeta[Active], activeValid, activeValid);
		
    if (activeValid) {
        BankMeta[Active].trial_counter++;
        SetFwFlags(&BankMeta[Backup], false, backupValid);
    } else {
				if (backupValid)
				{
						SetFwFlags(&BankMeta[Backup], true, backupValid);
				} else 
				{
						SetFwFlags(&BankMeta[Backup], false, backupValid);
				}
    } 
}

/***
 *	@brief	Set bank status up according to the Fw validation result
 ***/
void Update_BankStatus(bool activeValid, bool backupValid)
{
    if (activeValid)
		{	
				if (backupValid)
				{
						BankStatus.status = DUAL_FW_BANK_VALID;
				} else {
						BankStatus.status = ACTIVE_FW_BANK_VALID;
				}
    } else if (backupValid) {
        BankStatus.status = BACKUP_FW_BANK_VALID;
    } 
			else {
        BankStatus.status = ALL_FW_BANK_INVALID;
    }
}

/***	@brief	Check FwMeta's integrity  ***/
_Bool IsFwValid(FwMeta * Meta)
{
		bool CrcOk = FwCheck_CRC(Meta);
    bool flagValid = ((Meta->flags == Fw_PendingFlag) || 
										 ((Meta->flags & (Fw_ValidFlag)) == (Fw_ValidFlag))) &&
											 Meta->WDTRst_counter <  MAX_WDT_TRIES;
		
		return CrcOk && flagValid;
}


void SetFwFlags(FwMeta *meta, bool active, bool valid) {
	
		meta->flags &= ~(Fw_ActiveFlag | Fw_ValidFlag | Fw_PendingFlag);
	
		if (active)
				meta->flags |= Fw_ActiveFlag;
	
		if (valid)
		{	
			meta->flags |= Fw_ValidFlag;
		} else {
			meta->flags |= Fw_InvalidFlag;
		}
}

void JumpToBootloader()
{
    WDT_Open(WDT_TIMEOUT_2POW18, WDT_RESET_DELAY_3CLK, TRUE, FALSE);
		WDT_RESET_COUNTER();
    WDT_CLEAR_RESET_FLAG();
	
    __disable_irq();
    SYS_UnlockReg();
    FMC_Open();
		
		WDT->CTL &= ~WDT_CTL_WDTEN_Msk;
    
		FMC_SetVectorPageAddr(FMC_APROM_BASE);
    NVIC_SystemReset();
}


bool FwCheck_CRC(FwMeta *meta) 
{	
    if ((meta->fw_start_addr == 0xFFFFFFFF) || (meta->fw_start_addr == 0x00000000) ||
        (meta->fw_size == 0xFFFFFFFF) || (meta->fw_size == 0x00000000) ||
        (meta->fw_crc32 == 0xFFFFFFFF) || (meta->fw_crc32 == 0x00000000)) return false;
    uint32_t crc = CRC32_Calc((const uint8_t *)meta->fw_start_addr, meta->fw_size);
    return (crc == meta->fw_crc32);	
}


void BlinkStatusLED(GPIO_T *port, uint32_t pin, uint8_t times, uint32_t delay_ms) 
{
    for (uint8_t i = 0; i < times; i++) {
        port->DOUT ^= (1 << pin);
        CLK_SysTickDelay(delay_ms * 1000);
    }
}

void BlinkLEDs()
{
    static uint8_t valid = 0;
    valid ^= 1;
    BlinkStatusLED(valid ? PD : PF, valid ? 7 : 2, 1, 2500);
}

int WriteFwStatus(FwStatus *status) {
		return WriteToFlash(status, sizeof(FwStatus), BANK_STATUS_BASE, false, 0);
}

int WriteMetadata(FwMeta *meta, uint32_t meta_base) {
		return WriteToFlash(meta, sizeof(FwMeta), meta_base, true, offsetof(FwMeta, meta_crc));
}

int WriteToFlash(void *data, uint32_t size, uint32_t base_addr, bool with_crc32, uint32_t crc_offset)
{
		FMC_Open();
    if (with_crc32) {
        uint32_t crc = CRC32_Calc((const uint8_t *)data, size - sizeof(uint32_t));
        *(uint32_t *)((uint8_t *)data + crc_offset) = crc;
    }
    
    if (FMC_Proc(FMC_ISPCMD_PAGE_ERASE, base_addr, base_addr + FMC_FLASH_PAGE_SIZE, 0) != 0) goto fail;
    if (FMC_Proc(FMC_ISPCMD_PROGRAM, base_addr, base_addr + size, (uint32_t *)data) != 0) goto fail;


    FMC_DISABLE_ISP();FMC_Close();
    return 0;

fail:
    FMC_DISABLE_ISP();FMC_Close();
    return -1;
		
}

uint32_t CRC32_Calc(const uint8_t *pData, uint32_t len) {
	
    uint32_t i, crc_result;
	
    CRC_Open(CRC_32, CRC_CHECKSUM_COM | CRC_CHECKSUM_RVS | CRC_WDATA_RVS, 0xFFFFFFFF, CRC_CPU_WDATA_32);
    for (i = 0; i + 4 <= len; i += 4) {
        uint32_t chunk;
        memcpy(&chunk, pData + i, 4);
        CRC->DAT = chunk;
    }
    if (i < len) {
        uint32_t lastChunk = 0xFFFFFFFF;
        memcpy(&lastChunk, pData + i, len - i);
        CRC->DAT = lastChunk;
    }

    crc_result = CRC_GetChecksum();
    return crc_result;
}