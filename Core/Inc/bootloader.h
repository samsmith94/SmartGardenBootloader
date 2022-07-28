#ifndef INC_BOOTLOADER_H_
#define INC_BOOTLOADER_H_
/* Includes ------------------------------------------------------------------*/
#include "crc.h"
#include "stm32f4xx_hal.h"

#define CONF_FILENAME "crc.bin"

#define ROM_START (uint8_t *)(0x08040000)
#define ROM_END (uint8_t *)(0x0805FFFB)
#define CHECKSUM_ADDR (uint8_t *)(0x0805FFFC)
#define ROM_LEN (uint32_t)(ROM_END - ROM_START + 1u)
#define ROM_LEN_WORD (uint32_t)(ROM_LEN / 4u)

/** Automatically set vector table location before launching application */
#define SET_VECTOR_TABLE 1

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* STM32F407 Sector address */
#define ADDR_FLASH_SECTOR_0 ((uint32_t)0x08000000)  /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1 ((uint32_t)0x08004000)  /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2 ((uint32_t)0x08008000)  /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3 ((uint32_t)0x0800C000)  /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4 ((uint32_t)0x08010000)  /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5 ((uint32_t)0x08020000)  /* Base @ of Sector 5, 128 Kbytes */

#define ADDR_FLASH_SECTOR_6 ((uint32_t)0x08040000)  /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7 ((uint32_t)0x08060000)  /* Base @ of Sector 7, 128 Kbytes */
//STM32F429ZI-n dupla hely van, itt csak 512 kb!!!
//#define ADDR_FLASH_SECTOR_8 ((uint32_t)0x08080000)  /* Base @ of Sector 8, 128 Kbytes */
//#define ADDR_FLASH_SECTOR_9 ((uint32_t)0x080A0000)  /* Base @ of Sector 9, 128 Kbytes */
//#define ADDR_FLASH_SECTOR_10 ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbytes */
//#define ADDR_FLASH_SECTOR_11 ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbytes */

/* Error code */
enum
{
    FLASHIF_OK = 0,
    FLASHIF_ERASEKO,
    FLASHIF_WRITINGCTRL_ERROR,
    FLASHIF_WRITING_ERROR,
    FLASHIF_PROTECTION_ERRROR,
    BL_CHKS_ERROR
};

/* protection type */
enum
{
    FLASHIF_PROTECTION_NONE = 0,
    FLASHIF_PROTECTION_PCROPENABLED = 0x1,
    FLASHIF_PROTECTION_WRPENABLED = 0x2,
    FLASHIF_PROTECTION_RDPENABLED = 0x4,
};

/* protection update */
enum
{
    FLASHIF_WRP_ENABLE,
    FLASHIF_WRP_DISABLE
};

/* Application address, size, write protection */
//#define USER_START_ADDRESS ADDR_FLASH_SECTOR_8
#define USER_START_ADDRESS ADDR_FLASH_SECTOR_6
//#define USER_END_ADDRESS ADDR_FLASH_SECTOR_11
#define USER_END_ADDRESS ADDR_FLASH_SECTOR_7
#define USER_FLASH_SIZE ((uint32_t)(USER_END_ADDRESS - USER_START_ADDRESS))
#define USER_WRP_SECTORS (OB_WRP_SECTOR_2 | OB_WRP_SECTOR_3 | OB_WRP_SECTOR_4 | OB_WRP_SECTOR_5)

/* Exported functions ------------------------------------------------------- */
void FLASH_If_Init(void);
uint32_t FLASH_If_Erase(uint32_t StartSector);
uint32_t FLASH_If_GetWriteProtectionStatus(void);
// uint32_t FLASH_If_Write(uint32_t destination, uint32_t *p_source, uint32_t length);
uint32_t FLASH_If_Write(uint32_t data);

uint32_t FLASH_If_FlashBegin(void);
uint8_t FLASH_If_FlashEnd(void);

uint32_t FLASH_If_WriteProtectionConfig(uint32_t modifier);

void Bootloader_JumpToApplication(void);
uint8_t Bootloader_VerifyChecksum(void);

uint32_t readWord(uint32_t address);
uint8_t Bootloader_CheckForApplication(void);
#endif /* INC_BOOTLOADER_H_ */
