#include "bootloader.h"

static uint32_t GetSector(uint32_t Address);

/* Clear flags */
void FLASH_If_Init(void)
{
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                           FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
    HAL_FLASH_Lock();
}

/* Erase flash memory */
uint32_t FLASH_If_Erase(uint32_t start)
{
    uint32_t FirstSector, NbOfSectors, SectorError;
    FLASH_EraseInitTypeDef pEraseInit;
    HAL_StatusTypeDef status = HAL_OK;

    HAL_FLASH_Unlock();

    /* Erase from SECTOR 2~5 */
    FirstSector = GetSector(USER_START_ADDRESS);
    NbOfSectors = GetSector(USER_END_ADDRESS) - FirstSector;

    pEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    pEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    pEraseInit.Sector = FirstSector;
    pEraseInit.NbSectors = NbOfSectors;
    status = HAL_FLASHEx_Erase(&pEraseInit, &SectorError);

    HAL_FLASH_Lock();

    if (status != HAL_OK)
    {
        return FLASHIF_ERASEKO;
    }

    return FLASHIF_OK;
}

static uint32_t flash_ptr = USER_START_ADDRESS;

uint32_t FLASH_If_FlashBegin(void)
{
    flash_ptr = USER_START_ADDRESS;
    HAL_FLASH_Unlock();
    return FLASHIF_OK;
}

/* Write flash memory */
uint32_t FLASH_If_Write(uint32_t data)
{
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, flash_ptr, data) == HAL_OK)
    {
        /* Validate the written value */
        if (*(uint32_t *)flash_ptr != data)
        {
            HAL_FLASH_Lock();
            return FLASHIF_WRITINGCTRL_ERROR;
        }

        /* Increase WORD length */
        flash_ptr += 4;
    }
    else
    {
        HAL_FLASH_Lock();
        return FLASHIF_WRITING_ERROR;
    }

    return FLASHIF_OK;
}

uint8_t FLASH_If_FlashEnd(void)
{
    /* Lock flash */
    HAL_FLASH_Lock();

    return FLASHIF_OK;
}

/* Check write protection */
uint32_t FLASH_If_GetWriteProtectionStatus(void)
{
    uint32_t ProtectedSector = FLASHIF_PROTECTION_NONE;
    FLASH_OBProgramInitTypeDef OptionsBytesStruct;

    HAL_FLASH_Unlock();
    HAL_FLASHEx_OBGetConfig(&OptionsBytesStruct);
    HAL_FLASH_Lock();

    /* If sectors are protected, WRPSector bits are zero, so it needs to be inverted */
    ProtectedSector = ~(OptionsBytesStruct.WRPSector) & USER_WRP_SECTORS;

    if (ProtectedSector != 0)
    {
        return FLASHIF_PROTECTION_WRPENABLED;
    }

    return FLASHIF_PROTECTION_NONE;
}

/* Configure write protection */
uint32_t FLASH_If_WriteProtectionConfig(uint32_t protectionstate)
{
    FLASH_OBProgramInitTypeDef OBInit;
    HAL_StatusTypeDef status = HAL_OK;

    HAL_FLASH_OB_Unlock();
    HAL_FLASH_Unlock();

    /* Configure sector write protection */
    OBInit.OptionType = OPTIONBYTE_WRP;
    OBInit.Banks = FLASH_BANK_1;
    OBInit.WRPState = (protectionstate == FLASHIF_WRP_ENABLE ? OB_WRPSTATE_ENABLE : OB_WRPSTATE_DISABLE);
    OBInit.WRPSector = USER_WRP_SECTORS;

    HAL_FLASHEx_OBProgram(&OBInit);
    status = HAL_FLASH_OB_Launch();

    HAL_FLASH_OB_Lock();
    HAL_FLASH_Lock();

    if (status != HAL_OK)
    {
        return FLASHIF_PROTECTION_ERRROR;
    }
    else
    {
        return FLASHIF_OK;
    }
}

/* Get sector number by address */
static uint32_t GetSector(uint32_t Address)
{
    uint32_t sector = 0;

    if ((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
    {
        sector = FLASH_SECTOR_0;
    }
    else if ((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
    {
        sector = FLASH_SECTOR_1;
    }
    else if ((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
    {
        sector = FLASH_SECTOR_2;
    }
    else if ((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
    {
        sector = FLASH_SECTOR_3;
    }
    else if ((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
    {
        sector = FLASH_SECTOR_4;
    }
    else if ((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
    {
        sector = FLASH_SECTOR_5;
    }
    else if ((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
    {
        sector = FLASH_SECTOR_6;
    }
    else
    {
    	sector = FLASH_SECTOR_7;
    }

    return sector;
}

typedef void (*pFunction)(void); /*!< Function pointer definition */

void Bootloader_JumpToApplication(void)
{
    uint32_t JumpAddress = *(__IO uint32_t *)(USER_START_ADDRESS + 4);
    pFunction Jump = (pFunction)JumpAddress;

    HAL_RCC_DeInit();
    HAL_DeInit();

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

#if (SET_VECTOR_TABLE)
    SCB->VTOR = USER_START_ADDRESS;
#endif

    __set_MSP(*(__IO uint32_t *)USER_START_ADDRESS);
    Jump();
}

uint8_t Bootloader_VerifyChecksum(void)
{
    uint32_t calculated_crc;
    calculated_crc = HAL_CRC_Calculate(&hcrc, (uint32_t *)ROM_START, (uint32_t)ROM_LEN_WORD);

    uint8_t crc_array[4];
    crc_array[0] = calculated_crc >> 24;
    crc_array[1] = calculated_crc >> 16;
    crc_array[2] = calculated_crc >> 8;
    crc_array[3] = calculated_crc;
    printf("Calculated CRC: %X:%X:%X:%X\r\n", crc_array[3], crc_array[2], crc_array[1], crc_array[0]);

    uint32_t crc;
    crc = *(uint32_t *)CHECKSUM_ADDR;
    crc_array[0] = crc >> 24;
    crc_array[1] = crc >> 16;
    crc_array[2] = crc >> 8;
    crc_array[3] = crc;
    printf("CRC appended application.bin: %X:%X:%X:%X\r\n", crc_array[3], crc_array[2], crc_array[1], crc_array[0]);

    if ((*(uint32_t *)CHECKSUM_ADDR) == calculated_crc)
    {
        return 0;
    }
    return BL_CHKS_ERROR;
}

uint32_t readWord(uint32_t address)
{
    uint32_t read_data;
    read_data = *(uint32_t *)(address);
    return read_data;
}

uint8_t Bootloader_CheckForApplication(void)
{
    // Check if the application is there
    uint8_t emptyCellCount = 0;
    for (uint8_t i = 0; i < 10; i++)
    {
        uint32_t word = readWord(ROM_START + (i * 4));

        printf("Word: %d\r\n", word);

        if (word == 0xFFFF)
        {
            emptyCellCount++;
        }
    }

    if (emptyCellCount != 10)
    {
        return 0;
    }
    else
    {
        return -100;
    }
}
