/*******************************************************************************
 * Title                 :
 * Filename              :   dfu.c
 * Origin Date           :   2023/11/14
 * Version               :   0.0.0
 * Compiler              :   nRF connect SDK V2.4.0
 * Target                :   nRF52840DK
 * Notes                 :   None
 *******************************************************************************/

/** \file dfu.c
 *  \brief This module contains the DFU driver
 */
/******************************************************************************
 * Includes
 *******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "dfu.h"
#include "crc32.h"
#include "n25q128a.h"

/******************************************************************************
 * Module Preprocessor Constants
 *******************************************************************************/

/******************************************************************************
 * Module Preprocessor Macros
 *******************************************************************************/

/******************************************************************************
* Module Typedefs

*******************************************************************************/

/******************************************************************************
 * Module Variable Definitions
 *******************************************************************************/

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/

/******************************************************************************
 * Function Definitions
 *******************************************************************************/

/******************************************************************************
 * Flash HAL Functions
 *******************************************************************************/
#if (DFU_STORAGE_SPI_ZEPHYR == 1)
static const struct device *storage_device_handle = NULL;

typedef struct
{
    uint32_t flash_size;     // Number of flash size in bytes
    uint16_t sector_size;    // Number of sector (or smallest eraseable unit) in bytes
    uint16_t sector_count;   // Number of sector
    uint8_t write_size;      // Smallest write size in bytes
    uint8_t erase_default;   // Default value after erase
} flash_info_t;

static flash_info_t flash_storage_device_info = {0};

long flash_get_flash_size()
{
    return flash_storage_device_info.flash_size != 0 ? flash_storage_device_info.flash_size : -1;
}

int flash_get_sector_size()
{
    return flash_storage_device_info.sector_size != 0 ? flash_storage_device_info.sector_size : -1;
}

int flash_get_sector_count()
{
    return flash_storage_device_info.sector_count != 0 ? flash_storage_device_info.sector_count : -1;
}

int flash_get_write_size()
{
    return flash_storage_device_info.write_size != 0 ? flash_storage_device_info.write_size : -1;
}

int flash_get_erase_value()
{
    return flash_storage_device_info.erase_default;
}

int dfu_storage_read(uint32_t addr, uint8_t *data, uint32_t len)
{
    IS_STORAGE_DEV_RDY();
    return flash_read(storage_device_handle, addr, data, len);
}

int dfu_storage_write(uint32_t addr, uint8_t *data, uint32_t len)
{
    IS_STORAGE_DEV_RDY();

    // Write the number of by that is multiple of "FLASH WRITE SIZE"
    int write_size = flash_get_write_size();
    if (write_size < 0)
    {
        LOG_ERR("Failed to get flash write size");
        return -1;
    }
    uint32_t write_len = ceil((float) len / write_size) * write_size;
    if (0 != flash_write(storage_device_handle, addr, data, write_len))
    {
        LOG_ERR("Failed to write storage");
        return -1;
    }
    LOG_INF("Write %d bytes to storage at address: 0X%X", write_len, addr);
    return 0;
}

int dfu_storage_erase(uint32_t addr, uint32_t len)
{
    IS_STORAGE_DEV_RDY();
    // Find the smallest erase size that is multiple of "PAGE SECTOR SIZE"
    int erase_size = flash_get_sector_size();
    if (erase_size < 0)
    {
        LOG_ERR("Failed to get flash sector size");
        return -1;
    }
    uint32_t erase_len = ceil((float) len / erase_size) * erase_size;
    if (flash_erase(storage_device_handle, addr, erase_len) != 0)
    {
        LOG_ERR("Failed to erase %dB storage at address: 0X%X", erase_len, addr);
        return -1;
    }
    LOG_INF("Erase %dB storage at address: 0X%X", erase_len, addr);
    return 0;
}

int dfu_storage_flash_init(const struct device *storage_dev)
{
    // Storage device initialization
    if (!device_is_ready(storage_dev))
    {
        LOG_ERR("%s: DFU Storage device not ready", storage_dev->name);
        return -1;
    }
    storage_device_handle = storage_dev;

    // Get storage device info
    const struct flash_parameters *flash_info = flash_get_parameters(storage_device_handle);
    if (flash_info == NULL)
    {
        LOG_ERR("Failed to get storage device info");
        return -1;
    }

    // Get storage device page info
    struct flash_pages_info page_info;
    if (0 != flash_get_page_info_by_offs(storage_device_handle, 0, &page_info))
    {
        LOG_ERR("Failed to get page info\r\n");
        return -1;
    }
    // Update flash device info
    flash_storage_device_info.write_size = flash_info->write_block_size;
    flash_storage_device_info.erase_default = flash_info->erase_value;
    flash_storage_device_info.sector_size = page_info.size;
    flash_storage_device_info.sector_count = flash_get_page_count(storage_device_handle);
    flash_storage_device_info.flash_size =
        flash_storage_device_info.sector_size * flash_storage_device_info.sector_count;

    LOG_INF("Storage device info: write_size: %dB, flash_size: %dKB, sector_size: %dB, sector_count: %d\r\n",
            flash_storage_device_info.write_size, flash_storage_device_info.flash_size / 1024,
            flash_storage_device_info.sector_size, flash_storage_device_info.sector_count);
    return 0;
}

/*
 * @brief init the DFU module including the storage device
 * @param storage_dev: storage device handle use by DFU module
 * @return 0 on success, negative value otherwise
 */
int dfu_init(const struct device *storage_dev)
{
#if ((DFU_STORAGE_SPI_ZEPHYR == 1) && (DFU_STORAGE_SPI_STM32 != 1))
    if (0 != dfu_storage_flash_init(storage_dev))
    {
        LOG_ERR("Failed to init DFU flash storage\r\n");
        return -1;
    }
#else
#warning "Implement appropreate driver"
#endif /* End of (DFU_STORAGE_SPI_ZEPHYR == 1) */
    return 0;
}

#elif (DFU_STORAGE_SPI_STM32 == 1) && (DFU_STORAGE_SPI_MX25 == 1)
#include "MX25Series.h"
extern MX25Series_t flash_test;
int dfu_storage_read(uint32_t addr, uint8_t *data, uint32_t len)
{
    if (MX25Series_read_stored_data(&flash_test, true, addr, len, data) != MX25Series_status_ok)
    {
        LOG_ERR("Failed to read storage\r\n");
        return -1;
    }
    return 0;
}
#elif (DFU_STORAGE_SPI_STM32 == 1) && (DFU_STORAGE_SPI_N25Q == 1)

#include "n25q128a.h"
int dfu_storage_read(uint32_t addr, uint8_t *data, uint32_t len)
{
    N25Q_ReadDataFromAddress(data, addr, len);
    return 0;
}

int dfu_storage_erase(uint32_t addr, uint32_t len)
{
    // Calculate number of sectors to erase
    uint32_t num_sector = ceil((float) len / N25Q128A_SECTOR_SIZE);
    // Erase sectors
    for (uint32_t i = 0; i < num_sector; i++)
    {
        N25Q_SectorErase(addr + i * N25Q128A_SECTOR_SIZE);
    }
    return 0;
}

int dfu_storage_write(uint32_t addr, uint8_t *data, uint32_t len) {
    uint32_t remaining_len = len;
    uint32_t current_addr = addr;
    int result = 0;

    while (remaining_len > 0) {
        uint32_t write_len = (remaining_len > FLASH_N25_MAX_WRITE_SIZE) ? FLASH_N25_MAX_WRITE_SIZE : remaining_len;
        uint32_t page_start_addr = current_addr & ~(FLASH_N25_MAX_WRITE_SIZE - 1);

        // Check if the write operation crosses a page boundary
        if ((current_addr + write_len) > (page_start_addr + FLASH_N25_MAX_WRITE_SIZE))
        {
            // Write the data in two parts to avoid crossing the page boundary
            uint32_t first_part_len = FLASH_N25_MAX_WRITE_SIZE - (current_addr - page_start_addr);
            uint32_t second_part_len = write_len - first_part_len;

            // Write the first part
            N25Q_ProgramFromAddress(data, current_addr, first_part_len);

            if (result != 0) {
                return result;
            }

            // Write the second part
            N25Q_ProgramFromAddress(data + first_part_len, page_start_addr + FLASH_N25_MAX_WRITE_SIZE, second_part_len);

            if (result != 0) {
                return result;
            }
        }
        else
        {
            // Write the data in a single operation
            N25Q_ProgramFromAddress(data, current_addr, write_len);

            if (result != 0) {
                return result;
            }
        }
        // Readback and verify
        uint8_t read_data[FLASH_N25_MAX_WRITE_SIZE];
        N25Q_ReadDataFromAddress(read_data, current_addr, write_len);
        if (memcmp(data, read_data, write_len) != 0)
        {
            LOG_ERR("Failed to write %dB storage at address: 0X%X", write_len, current_addr);
            return -1;
        }
        remaining_len -= write_len;
        current_addr += write_len;
        data += write_len;
    }
    return 0;
}

#else /* !(DFU_STORAGE_SPI_ZEPHYR == 1) */
#endif /* End of (DFU_STORAGE_SPI_ZEPHYR == 1) */

/******************************************************************************
 * DFU Functions
 *******************************************************************************/

/*
 * @brief read image header at the given address
 * @param img_start_addr: start address of the image header
 * @param[out] img_header_data: pointer to store read image header
 * @return int 0 on success, negative value otherwise
 */
int dfu_image_read_header(uint32_t img_start_addr, image_header_t *img_header_data)
{
    assert(img_header_data != NULL);
    // Read image header
    if (0 != dfu_storage_read(img_start_addr, (uint8_t *) img_header_data, sizeof(image_header_t)))
    {
        LOG_ERR("Failed to read image header\r\n");
        return -1;
    }
    return 0;
}

/*
 * @brief: This function is used to validate the image data by compare CRC value in the header and the calculated CRC
 * value
 * @param img_start_addr
 * @return int 0 if the image data is valid, negative value otherwise
 */
int dfu_image_validate_data_content(uint32_t img_start_addr)
{
    // Read image header
    image_header_t image_header = {0};
    if (dfu_image_read_header(img_start_addr, &image_header) != 0)
    {
        LOG_ERR("Failed to read image header\r\n");
        return -1;
    }

    // Calculate CRC of the image inside the storage
    uint32_t crc = 0;
    uint8_t read_data_buf[image_header.img_data_size];
    memset(read_data_buf, 0, image_header.img_data_size);
    if (dfu_storage_read(image_header.img_data_start_addr, read_data_buf, image_header.img_data_size) != 0)
    {
        LOG_ERR("Failed to read %dB image data at address: 0X%X\r\n", image_header.img_data_size,
                image_header.img_data_start_addr);
        return -1;
    }
    crc = crc32(read_data_buf, image_header.img_data_size);

    if (crc != image_header.image_data_crc)
    {
        LOG_ERR("Image data CRC is invalid: %x instead of %x\r\n", crc, image_header.image_data_crc);
        return -1;
    }
    LOG_INF("Found valid image\r\n");
    return 0;
}

/*
 * @brief: Clear the image header to default erase value
 * @param img_start_addr 
 * @return int: 0 if the image header is cleared, negative value otherwise
 
 */
int dfu_image_clear(uint32_t img_start_addr)
{
    uint8_t header_len = sizeof(image_header_t);
    uint8_t header_buf[header_len];
    memset(header_buf, flash_get_erase_value(), header_len);
    // Erase the image header
    if (0 != dfu_storage_write(img_start_addr, header_buf, header_len))
    {
        LOG_ERR("Failed to clear image header at address: 0X%X\r\n", img_start_addr);
        return -1;
    }
    return 0;
}

/**
 * @brief: Commit image header, write header to storage at given address after validating image data
 * @param img_header_data: pointer to image header data
 * @param dest_img_addr: destination address to write image header
 * @return int: 0 if the image header is committed, negative value otherwise
 */
int dfu_image_commit(image_header_t *img_header_data, uint32_t hdr_addr)
{
    assert(img_header_data != NULL);
    uint8_t read_data_buf[img_header_data->img_data_size];
    memset(read_data_buf, 0, sizeof(read_data_buf));
    // Calculate CRC of the image inside the storage
    if (dfu_storage_read(img_header_data->img_data_start_addr, read_data_buf, img_header_data->img_data_size) != 0)
    {
        LOG_ERR("dfu_image_commit() failed to read %dB image data at address: 0X%X\r\n", img_header_data->img_data_size,
                img_header_data->img_data_start_addr);
        return -1;
    }
    uint32_t crc_storage = crc32(read_data_buf, img_header_data->img_data_size);
    // Check CRC
    if (img_header_data->image_data_crc != crc_storage)
    {
        LOG_ERR("dfu_image_commit() Image data CRC is invalid: %x instead of %x\r\n", img_header_data->image_data_crc,
                crc_storage);
        return -1;
    }

    // Write image header
    if (0 != dfu_storage_write(hdr_addr, (uint8_t *) img_header_data, sizeof(image_header_t)))
    {
        LOG_ERR(" dfu_image_commit() Failed to write image header at address: 0X%X\r\n", hdr_addr);
        return -1;
    }
    return 0;
}

/**
 * @brief: Update image at given address with new image data
 * @param img_meta_data: pointer to new image header data
 * @param p_data: pointer to new image data
 * @param data_len: length of new image data
 * @param dest_img_addr: destination address to write new image
 * @return int: 0 if the image is updated, negative value otherwise
 */
int dfu_image_update(image_header_t *img_meta_data, uint8_t *p_data, uint32_t data_len, uint32_t dest_img_addr)
{
    assert(img_meta_data != NULL);

    // Prepare storage for new image
    uint32_t img_total_size = data_len + sizeof(image_header_t);

    // Erase the image area
    if (0 != dfu_storage_erase(dest_img_addr, img_total_size))
    {
        LOG_ERR("Failed to erase %dB image area at address: 0X%X\r\n", img_total_size, dest_img_addr);
        return -1;
    }

    // Write image content
    if (0 != dfu_storage_write(dest_img_addr, p_data, data_len))
    {
        LOG_ERR("Failed to write %dB image data at address: 0X%X\r\n", data_len, dest_img_addr);
        return -1;
    }

    // Update image header based on new image data
    img_meta_data->img_data_size = data_len;
    img_meta_data->img_data_start_addr = dest_img_addr;
    // Calcuate CRC of new image data
    uint32_t crc_new_data = crc32(p_data, data_len);
    img_meta_data->image_data_crc = crc_new_data;

    // Commit image
    if (0 != dfu_image_commit(img_meta_data, dest_img_addr + data_len))
    {
        LOG_ERR("Failed to commit image\r\n");
        return -1;
    }
    return 0;
}

/*
 * @brief: Check if the image at the given address is valid by comparing header values
 *         with image default value (default_image_header)
 * @param addr: address of the image header
 * @return int: 0 if the image is valid, negative value otherwise 
 */
int dfu_image_is_valid(uint32_t addr)
{
    // Retrieve image header
    image_header_t read_header = {0};
    if (dfu_image_read_header(addr, &read_header) != 0)
    {
        LOG_ERR("Failed to read image header\r\n");
        return -1;
    }

    if (read_header.image_magic != IMAGE_MAGIC_NUMBER)
    {
        LOG_WRN("Invalid image magic number: 0x%X\r\n", read_header.image_magic);
        return -1;
    }

    // Check version
	if (IMAGE_FIRMWARE_MAJOR_VERSION    != read_header.image_data_version_major ||
		IMAGE_FIRMWARE_MINOR_VERSION    != read_header.image_data_version_minor ||
		IMAGE_FIRMWARE_REVISION_VERSION != read_header.image_data_version_revision )
	{
		LOG_WRN("Current version: %d.%d.%d is mismatch with %d.%d.%d in storage\r\n",
		IMAGE_FIRMWARE_MAJOR_VERSION, IMAGE_FIRMWARE_MINOR_VERSION, IMAGE_FIRMWARE_REVISION_VERSION,
			read_header.image_data_version_major, read_header.image_data_version_minor, read_header.image_data_version_revision);
		return -1;
	}

	// Check image type
	if (IMAGE_TYPE_RFIC_FIRMWARE != read_header.image_data_type)
	{
		LOG_WRN("Current image type: %d is mismatch with %d in storage\r\n",
			IMAGE_TYPE_RFIC_FIRMWARE, read_header.image_data_type);
		return -1;
	}

    // Check CRC of the image data in the storage
    if (dfu_image_validate_data_content(addr) != 0)
    {
        LOG_WRN("CRC check failed\r\n");
        return -1;
    }

    LOG_INF("Valid image found at 0x%X, type: %d, length: %d \r\n", 
        read_header.img_data_start_addr, read_header.image_data_type, read_header.img_data_size);
    LOG_INF("Version: %d.%d.%d \r\n",
		read_header.image_data_version_major, read_header.image_data_version_minor, read_header.image_data_version_revision);
    LOG_INF("CRC: 0x%X \r\n", read_header.image_data_crc);
#if (DFU_DUMP_IMAGE_DATA != 0)
    LOG_INF("Data content: \r\n");
    uint8_t read_data_buf[read_header.img_data_size];
    memset(read_data_buf, 0, read_header.img_data_size);
    if (dfu_storage_read(read_header.img_data_start_addr, read_data_buf, read_header.img_data_size) != 0)
    {
        LOG_ERR("Failed to read %dB image data at address: 0X%X\r\n", read_header.img_data_size,
                read_header.img_data_start_addr);
        return -1;
    }
    for (int i = 0; i < read_header.img_data_size; i++)
    {
        LOG_INF("%02X ", read_data_buf[i]);
    }
    LOG_INF("\r\n");
#endif /* End of (DFU_DUMP_IMAGE_DATA != 0 */)
    return 0;
}

/**
 * @brief Update the firmware image in the storage
 * @param addr: address of the image header
 * @return int 
 */
int dfu_fw_image_update(uint8_t* fw_data, uint32_t fw_len, uint32_t addr)
{
	int retval = 0;
    uint32_t header_addr = addr + fw_len;
    image_header_t default_image_header = {
        .image_magic = IMAGE_MAGIC_NUMBER,
        .image_data_type = IMAGE_TYPE_RFIC_FIRMWARE,
        .image_data_version_major = IMAGE_FIRMWARE_MAJOR_VERSION,
        .image_data_version_minor = IMAGE_FIRMWARE_MINOR_VERSION,
        .image_data_version_revision = IMAGE_FIRMWARE_REVISION_VERSION,
        .img_data_size = fw_len,
        .img_data_start_addr = addr,
    };
    //Check and update new img if needed
    if (dfu_image_is_valid(header_addr))
	{
        uint8_t img_update_retry = 5;
		LOG_WRN("Invalid image, perform DFU update");
		// Try to update image with max img_update_retry attempts
		for (uint8_t retry = 0; retry <= img_update_retry; retry++)
		{
			if (retry == img_update_retry)
			{
				LOG_ERR("dfu_fw_image_update() Failed to update image after %d attempts", retry);
				retval = -1;
				break;
			}

			if (dfu_image_update(&default_image_header, fw_data, default_image_header.img_data_size, default_image_header.img_data_start_addr) != 0)
			{
				LOG_ERR("dfu_fw_image_update() Failed to update image, (%d/%d)", retry + 1, img_update_retry);
			}
			else
			{
				LOG_INF("Image updated successfully");
				retval = 0;
				break;
			}
		}
	}
    return retval;
}
