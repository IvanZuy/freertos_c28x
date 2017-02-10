//############################################################################
//
// FILE:   SD_SPI_Erase.c
//
// TITLE:  SD/MMC Erase Function
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


#include "SD.h"						//SD Include File		

//############################# SD_ERASE_BLOCK ###############################
void sd_erase_block(Uint16 starting_sector, Uint16 total_sectors)
{
	CS_LOW;							//Pull CS low
	data_manipulation = TRUE;		//Data manipulation function 

	//Transmit Erase Start Block Command 
	spi_xmit_command(ERASE_WR_BLK_START_ADDR, starting_sector, DUMMY_CRC);
	
	RESET_RESPONSE;					//Reset response 
	while(response != STATUS_SUCCESS)		//Wait until card responds with SUCCESS response
		sd_command_response();

	//After receiving response clock must be active for 8 clock cycles
	EIGHT_CLOCK_CYCLE_DELAY;					

	//Add starting sector to total number of sectors to find ending address 
	total_sectors += starting_sector;

	//Transmit Erase End Block command 		
	spi_xmit_command(ERASE_WR_BLK_END_ADDR, total_sectors, DUMMY_CRC);
	
	RESET_RESPONSE;					//Reset response
	while(response != STATUS_SUCCESS)		//Wait until card responds with SUCCESS response
		sd_command_response();

	//After receiving response clock must be active for 8 clock cycles
	EIGHT_CLOCK_CYCLE_DELAY;

	//Transmit ERASE command
	spi_xmit_command(ERASE, STUFF_BITS, DUMMY_CRC);
	
	RESET_RESPONSE;					//Reset response	
	while(response != STATUS_SUCCESS)		//Wait until card responds with SUCCESS response
		sd_command_response();

	//Card will respond with the DATA OUT line pulled low if the card is still busy
	//erasing. Continue checking DATA OUT line until line is released high.
	while(response != DUMMY_DATA)			 
		response = spi_xmit_byte(DUMMY_DATA);
	
	CS_HIGH;						//Pull CS high			
   	//After receiving response clock must be active for 8 clock cycles
	EIGHT_CLOCK_CYCLE_DELAY;
}
//############################# SD_ERASE_BLOCK ###############################
