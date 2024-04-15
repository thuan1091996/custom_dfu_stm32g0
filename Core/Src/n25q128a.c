/*******************************************************************************
* Title                 :    
* Filename              :   n25q128a.c
* Origin Date           :   2023/10/30
* Version               :   0.0.0
* Compiler              :   ESP-IDF V5.0.2
* Target                :   ESP32 
* Notes                 :   None
*******************************************************************************/

/*************** MODULE REVISION LOG ******************************************
*
*    Date       Software Version	Initials	Description
*  2023/10/30       0.0.0	         ItachiVN      Module Created.
*
*******************************************************************************/

/** \file n25q128a.c
 *  \brief This module contains the
 */
/******************************************************************************
* Includes
*******************************************************************************/
#include "../Inc/main.h"
/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/

/******************************************************************************
* Module Preprocessor Macros
*******************************************************************************/
#if (FLASH_N25_DBG_MSG_EN != 0)
#define testprintf(...)         printf(__VA_ARGS__)
#define dbgprintf(...)          printf(__VA_ARGS__)
#else
#define testprintf(...)         (void)(0)
#define dbgprintf(...)          (void)(0)
#endif /* End of (FLASH_N25_DBG_MSG_EN != 0) */


#define SlaveSelect()           HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, GPIO_PIN_RESET)
#define SlaveDeSelect()         HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, GPIO_PIN_SET)
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

/*
 * Copyright [2016] [Riccardo Pozza]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author:
 * Riccardo Pozza <r.pozza@surrey.ac.uk>
 */
#include <stdbool.h>

#include "n25q128a.h"
#include "spi.h"


#define SPI_MAX_TIMEOUT     3000

int m_SPI__writebyte(uint8_t data) {
	uint8_t transmit_byte = data;
    if (HAL_SPI_Transmit(&hspi2, (uint8_t *)&transmit_byte, 1, SPI_MAX_TIMEOUT) != HAL_OK) {
        return -1;
    }
    return 0;
}

int m_SPI__ReadNBytes(uint8_t * rxBuffer, int length) {
    if (HAL_SPI_Receive(&hspi2, (uint8_t *)rxBuffer, length, SPI_MAX_TIMEOUT) != HAL_OK) {
        return -1;
    }
    return 0;
}

int m_SPI__ReadByte(void) {
    uint8_t rxBuffer;
    if (HAL_SPI_Receive(&hspi2, (uint8_t *)&rxBuffer, 1, SPI_MAX_TIMEOUT) != HAL_OK) {
        return -1;
    }
    return rxBuffer;
}

void N25Q_ClearFlagStatusRegister(void) {
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	SlaveSelect();
	m_SPI__writebyte(CLEAR_FLAG_STATUS_REG_CMD);
	SlaveDeSelect();

	testprintf("Ended!\r\n");
}

int N25Q_ReadStatusRegister(void) {
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	SlaveSelect();
	m_SPI__writebyte(READ_STATUS_REG_CMD);
	int retval = m_SPI__ReadByte();
	SlaveDeSelect();

	testprintf("Ended!\r\n");
	return retval;
}

bool N25Q_isBusy(void) {
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	int retval = N25Q_ReadStatusRegister();
	retval &= 0x01;

	if (retval){
		testprintf("Ended!\r\n");
		return true;
	}

	testprintf("Ended!\r\n");
	return false;
}


void N25Q_WriteEnable(void) {
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	dbgprintf("Write Enable\r\n");

	SlaveSelect();
	m_SPI__writebyte(WRITE_ENABLE_CMD);
	SlaveDeSelect();

	testprintf("Ended!\r\n");
}

void N25Q_ReadID(uint8_t * id_string, int length) {
    
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	dbgprintf("Reading Identification Data\r\n");
	SlaveSelect();
	m_SPI__writebyte(READ_ID_CMD);
	m_SPI__ReadNBytes(id_string,length);
	SlaveDeSelect();

	testprintf("Ended!\r\n");
}

void N25Q_ReadDataFromAddress(uint8_t * dataBuffer, int startingAddress, int length) {
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	dbgprintf("Reading Data From ");

	SlaveSelect();
	m_SPI__writebyte(READ_CMD);
	dbgprintf("Starting Address: ");
	for (int i=2; i>=0; i--){
		int addressByte = (startingAddress >> 8*i) & 0xFF;
		dbgprintf("%X ", addressByte);
		m_SPI__writebyte(addressByte);
	}
	dbgprintf("\r\n");
	m_SPI__ReadNBytes(dataBuffer,length);
	SlaveDeSelect();

	testprintf("Ended!\r\n");
}

void N25Q_ProgramFromAddress(uint8_t* dataBuffer, int startingAddress, int length){
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	dbgprintf("Writing Data From");

	N25Q_ClearFlagStatusRegister();

	N25Q_WriteEnable();

	SlaveSelect();
	m_SPI__writebyte(PAGE_PROG_CMD);
	dbgprintf("Starting Address: ");
	for (int i=2; i>=0; i--){
		int addressByte = (startingAddress >> 8*i) & 0xFF;
		dbgprintf("%X ", addressByte);
		m_SPI__writebyte(addressByte);
	}
	dbgprintf("\r\n");
	dbgprintf("Data Written:\r\n");
	for (int z=0; z<length; z++){
		dbgprintf("%X ", dataBuffer[z]);
		m_SPI__writebyte(dataBuffer[z]);
	}
	dbgprintf("\r\n");
	SlaveDeSelect();

	while (N25Q_isBusy());

	testprintf("Ended!\r\n");
}

void N25Q_NonBlockingProgramFromAddress(uint8_t * dataBuffer, int startingAddress, int length){
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	dbgprintf("Writing Data From");

	N25Q_ClearFlagStatusRegister();

	N25Q_WriteEnable();

	SlaveSelect();
	m_SPI__writebyte(PAGE_PROG_CMD);
	dbgprintf("Starting Address: ");
	for (int i=2; i>=0; i--){
		int addressByte = (startingAddress >> 8*i) & 0xFF;
		dbgprintf("%X ", addressByte);
		m_SPI__writebyte(addressByte);
	}
	dbgprintf("\r\n");
	dbgprintf("Data Written:\r\n");
	for (int z=0; z<length; z++){
		dbgprintf("%X ", dataBuffer[z]);
		m_SPI__writebyte(dataBuffer[z]);
	}
	dbgprintf("\r\n");
	SlaveDeSelect();

	testprintf("Ended!\r\n");
}

void N25Q_WriteDisable(void) {
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	dbgprintf("Write Disable\r\n");

	SlaveSelect();
	m_SPI__writebyte(WRITE_DISABLE_CMD);
	SlaveDeSelect();

	testprintf("Ended!\r\n");
}


void N25Q_WriteStatusRegister(int status_mask) {
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	N25Q_WriteEnable();

	SlaveSelect();
	m_SPI__writebyte(WRITE_STATUS_REG_CMD);
	m_SPI__writebyte(status_mask);  // only bits 7..2 (1 and 0 are not writable)
	SlaveDeSelect();

	while (N25Q_isBusy()){
	}

	testprintf("Ended!\r\n");
}

int N25Q_ReadLockRegister(int startingAddress) {
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	SlaveSelect();
	m_SPI__writebyte(READ_LOCK_REG_CMD);
	dbgprintf("Read Lock Register From Starting Address: ");
	for (int i=2; i>=0; i--){
		int addressByte = (startingAddress >> 8*i) & 0xFF;
		dbgprintf("%X ", addressByte);
		m_SPI__writebyte(addressByte);
	}
	dbgprintf("\r\n");
	int retval = m_SPI__ReadByte();
	SlaveDeSelect();

	testprintf("Ended!\r\n");
	return retval;
}

void N25Q_WriteLockRegister(int startingAddress, int lock_mask) {
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	N25Q_WriteEnable();

	SlaveSelect();
	m_SPI__writebyte(WRITE_LOCK_REG_CMD);
	for (int i=2; i>=0; i--){
		int addressByte = (startingAddress >> 8*i) & 0xFF;
		m_SPI__writebyte(addressByte);
	}
	m_SPI__writebyte(lock_mask);
	SlaveDeSelect();

	testprintf("Ended!\r\n");
}

int N25Q_ReadFlagStatusRegister(void) {
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	SlaveSelect();
	m_SPI__writebyte(READ_FLAG_STATUS_REG_CMD);
	dbgprintf("Read Flag Status Register ");
	int retval = m_SPI__ReadByte();
	dbgprintf("\r\n");
	SlaveDeSelect();

	testprintf("Ended!\r\n");
	return retval;

}

void N25Q_SubSectorErase(int startingAddress) {
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	N25Q_WriteEnable();

	SlaveSelect();
	m_SPI__writebyte(SUBSECTOR_ERASE_CMD);
	dbgprintf("Subsector Erase From Starting Address: ");
	for (int i=2; i>=0; i--){
		int addressByte = (startingAddress >> 8*i) & 0xFF;
		dbgprintf("%X ", addressByte);
		m_SPI__writebyte(addressByte);
	}
	dbgprintf("\r\n");
	SlaveDeSelect();

	while (N25Q_isBusy()){
	}

	testprintf("Ended!\r\n");
}

void N25Q_SectorErase(int startingAddress) {
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	N25Q_WriteEnable();

	SlaveSelect();
	m_SPI__writebyte(SECTOR_ERASE_CMD);
	dbgprintf("Sector Erase From Starting Address: ");
	for (int i=2; i>=0; i--){
		int addressByte = (startingAddress >> 8*i) & 0xFF;
		dbgprintf("%X ", addressByte);
		m_SPI__writebyte(addressByte);
	}
	dbgprintf("\r\n");
	SlaveDeSelect();

	while (N25Q_isBusy()){
	}

	testprintf("Ended!\r\n");
}

void N25Q_BulkErase(void) {
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	N25Q_WriteEnable();

	SlaveSelect();
	dbgprintf("Bulk Erase!\r\n");
	m_SPI__writebyte(BULK_ERASE_CMD);
	SlaveDeSelect();

	while (N25Q_isBusy()){
	}

	testprintf("Ended!\r\n");
}

void N25Q_NonBlockingBulkErase(void) {
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	N25Q_WriteEnable();

	SlaveSelect();
	dbgprintf("Bulk Erase!\r\n");
	m_SPI__writebyte(BULK_ERASE_CMD);
	SlaveDeSelect();

	testprintf("Ended!\r\n");
}



#if defined(TARGET_ARCH_PRO)
SPI * N25Q::m_SPI = new SPI(P0_9, P0_8, P0_7);
DigitalOut * N25Q::m_SSEL = new DigitalOut(P0_6);
#endif
