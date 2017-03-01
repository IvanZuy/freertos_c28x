//############################################################################
//
// FILE:   SD_SPI_Write.c
//
// TITLE:  SD/MMC Write Data Functions
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

//############################# SD_WRITE_BLOCK ###############################
void sd_write_block(Uint16 sector, Uint16 *pBuffer)
{
	CS_LOW;							//Pull CS low
    data_manipulation = TRUE;		//Data manipulation function

	//Transmit WRITE BLOCK command
	spi_xmit_command(WRITE_BLOCK, sector, DUMMY_CRC);

	RESET_RESPONSE;					//Reset response
	while(response != STATUS_SUCCESS)		//Wait until card responds with SUCCESS response
		sd_command_response();

	//After receiving response clock must be active for 8 clock cycles
	EIGHT_CLOCK_CYCLE_DELAY;
	
	//Call sd_write_data to write to specified sector
	sd_write_data(pBuffer, SINGLE_SECTOR, 0);	
			
   	//After receiving response clock must be active for 8 clock cycles
	EIGHT_CLOCK_CYCLE_DELAY;

	//Check card status to see if data was written properly
	sd_send_status();				

	CS_HIGH;						//Pull CS high			
   	//After receiving response clock must be active for 8 clock cycles
	EIGHT_CLOCK_CYCLE_DELAY;
}
//############################# SD_WRITE_BLOCK ###############################


//######################## SD_WRITE_MULTIPLE_BLOCK ###########################
void sd_write_multiple_block(Uint16 sector, Uint16 *pBuffer, Uint16 total_sectors)
{
	CS_LOW;							//Pull CS low
	data_manipulation = TRUE;		//Data manipulation function
    	
	//Transmit WRITE MULTIPLE BLOCK command
	spi_xmit_command(WRITE_MULTIPLE_BLOCK, sector, DUMMY_CRC);

	RESET_RESPONSE;					//Reset response
	while(response != STATUS_SUCCESS)		//Wait until card responds with SUCCESS response
		sd_command_response();

	//After receiving response clock must be active for 8 clock cycles
	EIGHT_CLOCK_CYCLE_DELAY;
	
	//Call sd_write_data to write to specified sectors
	sd_write_data(pBuffer, total_sectors, MULTIPLE_WRITE);
	
	//Set response to zero to check if card is still busy
	response = BUSY;

	//Card will respond with the DATA OUT line pulled low if the card is still busy
	//erasing. Continue checking DATA OUT line until line is released high.
	while(response != DUMMY_DATA)			 
		response = spi_xmit_byte(DUMMY_DATA);
			
   	//After receiving response clock must be active for 8 clock cycles
	EIGHT_CLOCK_CYCLE_DELAY;

	//Check card status to see if data was written properly
	sd_send_status();

	CS_HIGH;						//Pull CS high			
   	//After receiving response clock must be active for 8 clock cycles
	EIGHT_CLOCK_CYCLE_DELAY;
}
//######################## SD_WRITE_MULTIPLE_BLOCK ###########################


//############################# SD_WRITE_DATA ################################
void sd_write_data(Uint16 *pBuffer, Uint16 total_sectors, Uint16 multiple_write)
{
	Uint16 i;

	RESET_RESPONSE;					//Reset response

	//Continue this write loop for the total number of sectors required
	for(i=0;i<total_sectors;i++)
	{
		//If multiple sector write is required, send data token for multiple write
		if(multiple_write == MULTIPLE_WRITE)
			spi_xmit_byte(MULTIPLE_START_BLOCK);
		//If multiple sector write is not required, send data token for write
		else
			spi_xmit_byte(START_BLOCK);	

		//Write block of data 
//		for(j=0;j<512;j++)
//			spi_xmit_byte(*pBuffer++);
		SPI_send(pBuffer, 512, 0);
		pBuffer += 512;

		//Transmit 0xFF 2 more times to send CRC. In SPI Mode CRC is disable by default
		//so application does not need to store it. 
		spi_xmit_byte(DUMMY_DATA);					
		spi_xmit_byte(DUMMY_DATA);

		//After data write, card with send a response stating if the card was accepted or
		//if an error occured
		while(response != DATA_ACCEPTED)
		{
			response = spi_xmit_byte(DUMMY_DATA);	
			response &= 0x0F00;		//Mask response to test for errors

			//If there where errors writing data, branch to error function
			if((response == CRC_ERROR) || (response == WRITE_ERROR))
				sd_error();	 
		}

		//Card will respond with the DATA OUT line pulled low if the card is still busy
		//erasing. Continue checking DATA OUT line until line is released high.
		while(response != DUMMY_DATA)			 
			response = spi_xmit_byte(DUMMY_DATA);
	}

	//If multiple sector write is required, send data token to end transmission
	if(multiple_write == MULTIPLE_WRITE)
		spi_xmit_byte(MULTIPLE_STOP_TRANSMISSION);
}
//############################# SD_WRITE_DATA ################################

