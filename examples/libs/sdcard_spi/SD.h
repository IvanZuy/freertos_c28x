//############################################################################
//
// FILE:   SD.h
//
// TITLE:  Command, Data Token, Global Prototypes, and Global Variables
//
//############################################################################
// Author: Tim Love
// Release Date: July 2007 
//############################################################################


/* ***********************************************************
* THIS PROGRAM IS PROVIDED "AS IS". TI MAKES NO WARRANTIES OR
* REPRESENTATIONS, EITHER EXPRESS, IMPLIED OR STATUTORY, 
* INCLUDING ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS 
* FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR 
* COMPLETENESS OF RESPONSES, RESULTS AND LACK OF NEGLIGENCE. 
* TI DISCLAIMS ANY WARRANTY OF TITLE, QUIET ENJOYMENT, QUIET 
* POSSESSION, AND NON-INFRINGEMENT OF ANY THIRD PARTY 
* INTELLECTUAL PROPERTY RIGHTS WITH REGARD TO THE PROGRAM OR 
* YOUR USE OF THE PROGRAM.
*
* IN NO EVENT SHALL TI BE LIABLE FOR ANY SPECIAL, INCIDENTAL, 
* CONSEQUENTIAL OR INDIRECT DAMAGES, HOWEVER CAUSED, ON ANY 
* THEORY OF LIABILITY AND WHETHER OR NOT TI HAS BEEN ADVISED 
* OF THE POSSIBILITY OF SUCH DAMAGES, ARISING IN ANY WAY OUT 
* OF THIS AGREEMENT, THE PROGRAM, OR YOUR USE OF THE PROGRAM. 
* EXCLUDED DAMAGES INCLUDE, BUT ARE NOT LIMITED TO, COST OF 
* REMOVAL OR REINSTALLATION, COMPUTER TIME, LABOR COSTS, LOSS 
* OF GOODWILL, LOSS OF PROFITS, LOSS OF SAVINGS, OR LOSS OF 
* USE OR INTERRUPTION OF BUSINESS. IN NO EVENT WILL TI'S 
* AGGREGATE LIABILITY UNDER THIS AGREEMENT OR ARISING OUT OF 
* YOUR USE OF THE PROGRAM EXCEED FIVE HUNDRED DOLLARS 
* (U.S.$500).
*
* Unless otherwise stated, the Program written and copyrighted 
* by Texas Instruments is distributed as "freeware".  You may, 
* only under TI's copyright in the Program, use and modify the 
* Program without any charge or restriction.  You may 
* distribute to third parties, provided that you transfer a 
* copy of this license to the third party and the third party 
* agrees to these terms by its first use of the Program. You 
* must reproduce the copyright notice and any other legend of 
* ownership on each copy or partial copy, of the Program.
*
* You acknowledge and agree that the Program contains 
* copyrighted material, trade secrets and other TI proprietary 
* information and is protected by copyright laws, 
* international copyright treaties, and trade secret laws, as 
* well as other intellectual property laws.  To protect TI's 
* rights in the Program, you agree not to decompile, reverse 
* engineer, disassemble or otherwise translate any object code 
* versions of the Program to a human-readable form.  You agree 
* that in no event will you alter, remove or destroy any 
* copyright notice included in the Program.  TI reserves all 
* rights not specifically granted under this license. Except 
* as specifically provided herein, nothing in this agreement 
* shall be construed as conferring by implication, estoppel, 
* or otherwise, upon you, any license or other right under any 
* TI patents, copyrights or trade secrets.
*
* You may not use the Program in non-TI devices.
* ********************************************************* */

#ifndef __SD_H__
#define __SD_H__

#include "spi.h"


//SD Standard Commands
#define GO_IDLE_STATE               0x4000
#define SEND_OP_COND                0x4100
#define SWITCH_FUNC                 0x4600
#define SEND_IF_COND                0x4800
#define SEND_CSD                    0x4900
#define SEND_CID                    0x4A00
#define STOP_TRANSMISSION           0x4C00
#define SEND_STATUS                 0x4D00
#define SET_BLOCKLEN                0x5000
#define READ_SINGLE_BLOCK           0x5100
#define READ_MULTIPLE_BLOCK         0x5200
#define WRITE_BLOCK                 0x5800
#define WRITE_MULTIPLE_BLOCK        0x5900
#define PROGRAM_CSD                 0x5B00
#define SET_WRITE_PROT              0x5C00
#define CLR_WRITE_PROT              0x5D00
#define SEND_WRITE_PROT             0x5E00
#define ERASE_WR_BLK_START_ADDR     0x6000
#define ERASE_WR_BLK_END_ADDR       0x6100
#define ERASE                       0x6600
#define LOCK_UNLOCK                 0x6A00
#define APP_CMD                     0x7700
#define GEN_CMD                     0x7800
#define READ_OCR                    0x7A00
#define CRC_ON_OFF                  0x7B00

//SD Application Specific Commands
#define SD_STATUS                   0x4D00
#define SEND_NUM_WR_BLOCKS          0x5600
#define SET_WR_BLK_ERASE_COUNT      0x5700
#define SD_SEND_OP_COND             0x6900
#define SET_CLR_CARD_DETECT         0x6A00
#define SEND_SCR                    0x7300

//Data Tokens
#define INITIAL_CRC                 0x9500
#define DUMMY_CRC                   0xFF00
#define DUMMY_DATA                  0xFF00
#define START_BLOCK                 0xFE00
#define MULTIPLE_START_BLOCK        0xFC00
#define MULTIPLE_STOP_TRANSMISSION  0xFD00 
#define SECTOR_ZERO                 0x0000
#define STUFF_BITS                  0x0000
#define BUSY                        0x0000
#define SINGLE_SECTOR               0x0001
#define MULTIPLE_WRITE              0x0001
#define CRC_ON                      0x0001
#define CRC_OFF                     0x0000
#define BLOCK_LENGTH                0x0200
#define TRUE                        0x0001
#define FALSE                       0x0000
#define CHECK_PATTERN               0xAA00
#define SUPPLY_VOLTAGE              0x0100
#define HIGH_CAPACITY               0x4000
#define SUPPORTED_VOLTAGE           0xFF00
#define INTERFACE_COND              426
#define VER2_OP_COND                1073741824

//Status and Error Codes
#define IN_IDLE_STATE               0x0100
#define STATUS_SUCCESS              0x0000
#define DATA_ACCEPTED               0x0500
#define CRC_ERROR                   0x0B00
#define WRITE_ERROR                 0x0D00
#define ERROR                       0x0100
#define CC_ERROR                    0x0200
#define CARD_ECC_FAILED             0x0400
#define OUT_OF_RANGE                0x0800  
#define ILLEGAL_COMMAND_IDLE_STATE  0x0500 

//Continuous Commands
#define CS_HIGH SPI_setCsHigh();
#define CS_LOW  SPI_setCsLow();
#define RESET_RESPONSE response = 0xFF00;
#define EIGHT_CLOCK_CYCLE_DELAY spi_xmit_byte(DUMMY_DATA);

//Global Prototypes

//Functions located in SD_SPI_Erase.c
void sd_erase_block(Uint16 starting_sector, Uint16 total_sectors);

//Functions located in SD_SPI_Initialization.c
void spi_initialization();
void led_initialization();
void sd_card_insertion();
void sd_initialization();
void sd_version1_initialization();
void sd_version2_initialization();

//Functions located in SD_SPI_Read.c
void sd_read_block(Uint16 sector, Uint16 *pBuffer);
void sd_read_multiple_block(Uint16 sector, Uint16 *pBuffer, Uint16 total_sectors);
void sd_data_response(Uint16 *pBuffer, Uint16 total_sectors);

//Functions located in SD_SPI_Registers.c
void sd_read_register(Uint16 command);
void sd_ocr_response();
void sd_cid_csd_response(Uint16 command);
void sd_send_status();
void sd_switch_function(Uint16 *pBuffer);

//Functions located in SD_SPI_Transmission.c
Uint16 spi_xmit_byte(Uint16 byte);
void spi_xmit_command(Uint16 command, Uint32 data, Uint16 crc);
Uint16 sd_crc7(Uint16 full_command[6]);
void sd_command_response();
void sd_error();

//Functions located in SD_SPI_Write.c
void sd_write_block(Uint16 sector, Uint16 *pBuffer);
void sd_write_multiple_block(Uint16 sector, Uint16 *pBuffer, Uint16 total_sectors);
void sd_write_data(Uint16 *pBuffer, Uint16 total_sectors, Uint16 multiple_write);

//Global Variables
extern Uint16 response, ocr_contents[5], csd_contents[16], cid_contents[16];
extern Uint16 card_status[2], data_manipulation, high_capacity, crc_enabled;

#endif // __SD_H__
