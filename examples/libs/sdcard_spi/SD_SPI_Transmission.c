//############################################################################
//
// FILE:   SD_SPI_Transmission.c
//
// TITLE:  SD/MMC SPI Transmission and Error Functions
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

//############################# SPI_XMIT_BYTE ################################
Uint16 spi_xmit_byte(Uint16 byte)
{
	return SPI_sendByte(byte);
}
//############################# SPI_XMIT_BYTE ################################


//############################ SPI_XMIT_COMMAND ##############################
void spi_xmit_command(Uint16 command, Uint32 data, Uint16 crc)
{
	Uint16 i, full_command[6];			
	
	//If data manipulation is requested from standard capacity card,
	//multiply data by 512 to receive full address to transmit
	if((data_manipulation == TRUE) && (high_capacity == FALSE))
		data *= BLOCK_LENGTH;					 
	
	//Set first element of full_command to the command to be transmitted
	full_command[0] = command;		

	//This loop breaks the data/address into 4 bytes to be transmitted. Since 
	//the data is brought into function as a long, it must be typecast to an unsigned
	//integer to transmit properly. The data is anded with 0xFF to get what data is located
	//in the last two bytes of data. Data is then right shifted by eight to test the next 
	//byte.  
	for(i=4;i>0;i--)
	{
		full_command[i] = ((Uint16)(Uint32)(data & 0x000000FF) << 8);
		data = data >> 8;
	}
    
	//Set last element of full_command to the crc value to be transmitted
	if(crc_enabled == TRUE)
    	full_command[5] = sd_crc7(full_command);
	else
		full_command[5] = crc;
    
    for(i=0;i<6;i++)						//Transmit full command 
    	spi_xmit_byte(full_command[i]);			
} 
//############################ SPI_XMIT_COMMAND ##############################


//################################ SD_CRC7 ###################################
Uint16 sd_crc7(Uint16 full_command[6])
{
	Uint16 i,j,command,crc = 0x0000;
   	                                           
    for (i=0;i<5;i++)
    {
		command = full_command[i];		
       	for (j=0;j<8;j++)
      	{
         	crc = crc << 1;					//Shift crc left by 1			
         	if ((command ^ crc) & 0x8000)	//Test command XOR with crc and masked with 0x8000
         		crc = crc ^ 0x0900;  		//XOR crc with 0x0900           
         	command = command << 1;			//Shift command left by 1
      	}

       	crc = crc & 0x7F00;					//Mask CRC with 0x7F00	
   	}
   
   	crc = crc << 1;							//Shift crc left by 1
   	crc |= 0x0100;							//CRC ORed with 0x0100
   
  	return(crc);							//Return CRC
}
//################################ SD_CRC7 ###################################


//########################## SD_COMMAND_RESPONSE #############################
void sd_command_response()
{
	Uint16 i;

	RESET_RESPONSE;							//Reset Response
		
	//This loop continously transmits 0xFF in order to receive response from card. 
	//The card is expected to return 00 or 01 stating that it is in idle state or 
	//it has finished it's initialization(SUCCESS). If anything is received besides
	//0x00, 0x01, or 0xFF then an error has occured. 
	for(i=0;i<8;i++)
	{
		response = spi_xmit_byte(DUMMY_DATA);
		//If response is 0x00 or 0x01 break from loop		
		if ((response == IN_IDLE_STATE) || (response == STATUS_SUCCESS))
			break;
		//If response is not 0x00, 0x01, or 0xFF branch to sd_error
		else if (response != DUMMY_DATA)
			sd_error();
	}
}
//########################## SD_COMMAND_RESPONSE #############################


//############################### SD_ERROR ###################################
void sd_error()
{	
	//For this example all errors are sent to this trap function. Specific application 
	//will have to be written to handle errors.
	 
	CS_HIGH;								//Pull CS high			
   	//After receiving response clock must be active for 8 clock cycles
	EIGHT_CLOCK_CYCLE_DELAY;

	asm(" ESTOP0");							//Emulation stop
   	for(;;){};								//Loop forever
}
//############################### SD_ERROR ###################################
