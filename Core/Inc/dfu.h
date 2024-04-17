/****************************************************************************
* Title                 :    header file
* Filename              :   dfu.h
* Author                :   ItachiVN
* Origin Date           :   2023/11/15
* Version               :   v0.0.0
* Compiler              :   ESP-IDF V5.0.2
* Target                :   ESP32 
* Notes                 :   None
*****************************************************************************/

/*************** INTERFACE CHANGE LIST **************************************
*
*    Date    	Software Version    Initials   	Description
*  2023/11/15    v0.0.0         	ItachiVN      Interface Created.
*
*****************************************************************************/

/** \file dfu.h
 *  \brief This module contains .
 *
 *  This is the header file for 
 */
#ifndef DFU_H_
#define DFU_H_

/******************************************************************************
* Includes
*******************************************************************************/
#include <stdint.h>
#include <stdio.h>

/******************************************************************************
* Preprocessor Constants
*******************************************************************************/

/* Application configs */
#define DFU_STORAGE_SPI_ZEPHYR			    (0) // 1: DFU use zephyr external flash driver
#define DFU_STORAGE_SPI_STM32			    (1) // 1: DFU use STM driver
#define DFU_DUMP_IMAGE_DATA                 (0) // 1: Dump image data

#if (DFU_STORAGE_SPI_STM32 != 0)
#define DFU_STORAGE_SPI_MX25                (0)
#define DFU_STORAGE_SPI_N25Q                (1)
#endif /*(DFU_STORAGE_SPI_STM32 != 0) */


#define IMAGE_MAGIC_NUMBER 			        (0xBADCAFE)
#define IMAGE_TYPE_RFIC_FIRMWARE            (7)
#define IMAGE_FIRMWARE_MAJOR_VERSION        (0)
#define IMAGE_FIRMWARE_MINOR_VERSION        (0)
#define IMAGE_FIRMWARE_REVISION_VERSION     (1)

#define FLASH_N25_MAX_WRITE_SIZE            (256)
#define FLASH_N25_FW_START_ADDR            	(0)


/******************************************************************************
* Configuration Constants
*******************************************************************************/


/******************************************************************************
* Macros
*******************************************************************************/
#if !(DFU_STORAGE_SPI_ZEPHYR == 1)
#define LOG_ERR(...) printf("[ERR] "__VA_ARGS__); printf("\r\n");
#define LOG_WRN(...) printf("[WRN] "__VA_ARGS__); printf("\r\n");
#define LOG_INF(...) printf("[INF] "__VA_ARGS__); printf("\r\n");
#endif /* End of (DFU_STORAGE_SPI_ZEPHYR == 1) */

/******************************************************************************
* Typedefs
*******************************************************************************/
typedef struct __attribute__((packed)){
    // Image header
    uint32_t image_magic;

    // Image info
    uint32_t img_data_size;
    uint32_t img_data_start_addr;
    uint8_t image_data_type;
    uint8_t image_data_version_major;
    uint8_t image_data_version_minor;
    uint8_t image_data_version_revision;
    uint32_t image_data_crc;    
    uint32_t reserved;

}image_header_t;

/******************************************************************************
* Variables
*******************************************************************************/


/******************************************************************************
* Function Prototypes
*******************************************************************************/

#ifdef __cplusplus
extern "C"{
#endif

int dfu_init(const struct device *storage_dev);
int dfu_image_is_valid(uint32_t addr);
int dfu_image_validate_header(uint32_t img_start_addr);
int dfu_image_validate_data_content(uint32_t img_start_addr);
int dfu_image_clear(uint32_t img_start_addr);
int dfu_image_commit(image_header_t* img_header_data, uint32_t hdr_addr);
int dfu_image_read_header(uint32_t img_start_addr, image_header_t* img_header_data);
int dfu_image_update(image_header_t* img_meta_data, uint8_t* p_data, uint32_t data_len, uint32_t dest_img_addr);
int dfu_fw_image_update(uint8_t* fw_data, uint32_t fw_len, uint32_t addr);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // DFU_H_

/*** End of File **************************************************************/
