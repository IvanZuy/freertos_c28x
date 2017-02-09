//############################################################################
//
// FILE:   SD_SPI_Registers.c
//
// TITLE:  SD/MMC Register Manipulation Functions
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


#include "SD.h"                     //SD Include File       

//############################ SD_READ_REGISTER ##############################
void sd_read_register(Uint16 command)
{
    CS_LOW;                         //Pull CS low
    data_manipulation = FALSE;      //Register manipulation function

    //Transmit register command requested
    spi_xmit_command(command, STUFF_BITS, DUMMY_CRC);               
    
    RESET_RESPONSE;                 //Reset response
    while(response != STATUS_SUCCESS)      //Wait until card responds with SUCCESS response
        sd_command_response();
    
    if(command == READ_OCR)         //Test if OCR register is requested                 
        sd_ocr_response();          //Call sd_ocr_response to read OCR register
    else
        //Call sd_cid_csd_response to read CID or CSD registers
        sd_cid_csd_response(command);

    CS_HIGH;                        //Pull CS high          
    //After receiving response clock must be active for 8 clock cycles
    EIGHT_CLOCK_CYCLE_DELAY;
}
//############################ SD_READ_REGISTER ##############################


//############################ SD_OCR_RESPONSE ###############################
void sd_ocr_response()
{
    Uint16 i;
    
    //OCR register is five bytes. Transmit 0xFF 5 times to receive the contents
    //of OCR register and store them.
    for(i=0;i<4;i++)                         
        ocr_contents[i] = spi_xmit_byte(DUMMY_DATA);

    //After receiving response clock must be active for 8 clock cycles
    EIGHT_CLOCK_CYCLE_DELAY;
}
//############################ SD_OCR_RESPONSE ###############################


//########################## SD_CID_CSD_RESPONSE #############################
void sd_cid_csd_response(Uint16 command)
{
    Uint16 i;

    RESET_RESPONSE;                 //Reset Response
    while(response != START_BLOCK)  //Wait until START BLOCK is received
        response = spi_xmit_byte(DUMMY_DATA);

    //CID and CSD Registers are 16 bytes. Transmit 0xFF 16 times to receive the contents
    //of CID or CSD Register and store them.
    for(i=0;i<16;i++)
    {
        if(command == SEND_CSD)
            csd_contents[i] = spi_xmit_byte(DUMMY_DATA);
        else
            cid_contents[i] = spi_xmit_byte(DUMMY_DATA);            
    }

    //Transmit 0xFF 2 more times to receive CRC. In SPI Mode CRC is disable by default
    //so application does not need to store it. 
    spi_xmit_byte(DUMMY_DATA);                  
    spi_xmit_byte(DUMMY_DATA);
    
    //After receiving response clock must be active for 8 clock cycles
    EIGHT_CLOCK_CYCLE_DELAY;                
}
//########################## SD_CID_CSD_RESPONSE #############################


//############################# SD_SEND_STATUS ###############################
void sd_send_status()
{
    data_manipulation = FALSE;      //Register manipulation function

    //Transmit SEND STATUS command
    spi_xmit_command(SEND_STATUS, STUFF_BITS, DUMMY_CRC);

    RESET_RESPONSE;                 //Reset response
    while(response == DUMMY_DATA)   //Wait until response is not 0xFF
        response = spi_xmit_byte(DUMMY_DATA);
    
    card_status[0] = response;                  //Store first byte of Status register
    card_status[1] = spi_xmit_byte(DUMMY_DATA); //Store second byte of status register

    //If either byte of CARD STATUS is not SUCCESS, error was generated.
    if((card_status[0] != STATUS_SUCCESS) || (card_status[1] != STATUS_SUCCESS))
        sd_error();                             //Branch to error function

    //After receiving response clock must be active for 8 clock cycles
    EIGHT_CLOCK_CYCLE_DELAY;
}
//############################# SD_SEND_STATUS ###############################

