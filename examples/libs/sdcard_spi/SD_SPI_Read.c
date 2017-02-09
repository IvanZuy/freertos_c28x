//############################################################################
//
// FILE:   SD_SPI_Read.c
//
// TITLE:  SD/MMC Read Data Functions
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

//############################# SD_READ_BLOCK ################################
void sd_read_block(Uint16 sector, Uint16 *pBuffer)
{
	CS_LOW;							//Pull CS low
	data_manipulation = TRUE;		//Data manipulation function

	//Transmit READ SINGLE BLOCK command
	spi_xmit_command(READ_SINGLE_BLOCK, sector, DUMMY_CRC);	
	
	RESET_RESPONSE;					//Reset Response
	while(response != STATUS_SUCCESS)		//Wait until card responds with SUCCESS response
		sd_command_response();

	//After receiving response clock must be active for 8 clock cycles
	EIGHT_CLOCK_CYCLE_DELAY;

	//Call sd_data_response to read specified sector
	sd_data_response(pBuffer, SINGLE_SECTOR);	

	CS_HIGH;						//Pull CS high			
   	//After receiving response clock must be active for 8 clock cycles
	EIGHT_CLOCK_CYCLE_DELAY;
}
//############################# SD_READ_BLOCK ################################


//######################## SD_READ_MULTIPLE_BLOCK ############################
void sd_read_multiple_block(Uint16 sector, Uint16 *pBuffer, Uint16 total_sectors)
{
	Uint16 i;

	CS_LOW;							//Pull CS low
	data_manipulation = TRUE;		//Data manipulation function

	//Transmit READ MULTIPLE BLOCK command
	spi_xmit_command(READ_MULTIPLE_BLOCK, sector, DUMMY_CRC);	
	
	RESET_RESPONSE;					//Reset response
	while(response != STATUS_SUCCESS)		//Wait until card responds with SUCCESS response
		sd_command_response();

	//After receiving response clock must be active for 8 clock cycles
	EIGHT_CLOCK_CYCLE_DELAY;
	
	//Call sd_data_response to read specified sectors
	sd_data_response(pBuffer, total_sectors);
	
	//Transmit STOP TRANSMISSION command			
	spi_xmit_command(STOP_TRANSMISSION, STUFF_BITS, DUMMY_CRC);
	
	RESET_RESPONSE;					//Reset response

	//Wait 64 clock cycles or until card responds with SUCCESS response to move on
	for(i=0;i<8;i++)
	{
		response = spi_xmit_byte(DUMMY_DATA);
		if (response == STATUS_SUCCESS)
		 	break;
	}
	
	CS_HIGH;						//Pull CS high			
   	//After receiving response clock must be active for 8 clock cycles
	EIGHT_CLOCK_CYCLE_DELAY;
}
//######################## SD_READ_MULTIPLE_BLOCK ############################


//########################### SD_DATA_RESPONSE ###############################
void sd_data_response(Uint16 *pBuffer, Uint16 total_sectors)
{
	Uint16 i;

	data_manipulation = TRUE;		//Data manipulation function

	//Continue until all total sectors requested are transmitted
	for(i=0;i<total_sectors;i++)	
	{
		RESET_RESPONSE;				//Reset response
		//This loop continues receiving a response from the card until a START_BLOCK
		//or and error response is transmitted. If response is START_BLOCK then 
		//data will be transmitted. If response is an error the data will not be
		//transmitted.
		while(response != START_BLOCK)
		{
			response = spi_xmit_byte(DUMMY_DATA);

			if((response == ERROR) || (response == CC_ERROR) || \
			(response == CARD_ECC_FAILED) || (response == OUT_OF_RANGE))
				sd_error();
		}
			
//		for(j=0;j<512;j++)			//Read block of data
//			//Store data to address referred to by pBuffer
//			*pBuffer++ = spi_xmit_byte(DUMMY_DATA);

		SPI_receive(pBuffer, 512, portMAX_DELAY);
		pBuffer += 512;

		//Transmit 0xFF 2 more times to receive CRC. In SPI Mode CRC is disabled by default
		//so application does not need to store it. 
		spi_xmit_byte(DUMMY_DATA);					
		spi_xmit_byte(DUMMY_DATA);
	}
	
	//After receiving response clock must be active for 8 clock cycles
	EIGHT_CLOCK_CYCLE_DELAY;			
}
//########################### SD_DATA_RESPONSE ###############################

