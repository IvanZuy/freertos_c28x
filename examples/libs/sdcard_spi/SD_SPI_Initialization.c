//############################################################################
//
// FILE:   SD_SPI_Initialization.c
//
// TITLE:  SD/MMC Initialization Functions
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


#include "SD.h"                         //SD Include File       

//Global Variables
Uint16 response, ocr_contents[5], csd_contents[16], cid_contents[16];
Uint16 card_status[2], data_manipulation = TRUE, high_capacity = FALSE, crc_enabled = FALSE;

//######################### CHECK_CARD_INSERTION #############################
void sd_card_insertion()
{
    Uint16 i;

    //After Card Detection, SD protocol states that card needs 74 clock 
    //cycles with the DATA IN line high for chip to stabilize. CS does not 
    //need to be active for this action.

    CS_HIGH;                            //Pull CS high
    for(i=0;i<10;i++)                   //Transmit 0xFF for 80 clock cycles
        spi_xmit_byte(DUMMY_DATA); 
}
//######################### CHECK_CARD_INSERTION #############################


//########################## SD_INITIALIZATION ###############################
void sd_initialization()
{
    CS_LOW;                                 //Pull CS low
    data_manipulation = FALSE;              //Register manipulation function

    //Transmit GO IDLE STATE command to card with CS low to select SPI mode 
    //and put the card in IDLE mode.
    spi_xmit_command(GO_IDLE_STATE, STUFF_BITS, INITIAL_CRC);   
    
    RESET_RESPONSE;                         //Reset response    
    while(response != IN_IDLE_STATE)        //Wait until card responds with IDLE response
        sd_command_response();
    
    RESET_RESPONSE;                         //Reset response            
    //After receiving response clock must be active for 8 clock cycles
    EIGHT_CLOCK_CYCLE_DELAY;

    crc_enabled = TRUE;                     //CRC is always enabled for SEND_IF_COND command
    spi_xmit_command(SEND_IF_COND, INTERFACE_COND, DUMMY_CRC);  //Transmit SEND_IF_COND command
    crc_enabled = FALSE;                    //CRC is disabled for SPI mode

    //Wait until card responds with IDLE response
    while((response != IN_IDLE_STATE) && (response != ILLEGAL_COMMAND_IDLE_STATE))  
        response = spi_xmit_byte(DUMMY_DATA);

    //If SEND_IF_COND returned illegal command call sd_version1_initialization for
    //standard capacity card initialization. Otherwise call sd_version2_initialization 
    //for high capacity card initialization
    if(response == ILLEGAL_COMMAND_IDLE_STATE)
        sd_version1_initialization();   
    else if(response == IN_IDLE_STATE)
        sd_version2_initialization();

    CS_HIGH;                                //Pull CS high
    //After receiving response clock must be active for 8 clock cycles
    EIGHT_CLOCK_CYCLE_DELAY;
}
//########################## SD_INITIALIZATION ###############################


//###################### SD_VERSION1_INITIALIZATION ##########################
void sd_version1_initialization()
{

    RESET_RESPONSE;                     //Reset response
    //After receiving response clock must be active for 8 clock cycles
    EIGHT_CLOCK_CYCLE_DELAY;

    spi_xmit_command(READ_OCR, STUFF_BITS, DUMMY_CRC);  //Transmit READ_OCR command
    //Wait until card responds with IN_IDLE_STATE response
    while(response != IN_IDLE_STATE)        
        sd_command_response();

    sd_ocr_response();                  //Call OCR response function

    //If host voltage is not compatible with card voltage, do not communicate 
    //further with card
    if(ocr_contents[1] != SUPPORTED_VOLTAGE)        
        sd_error();

    RESET_RESPONSE;                     //Reset response
    //After receiving response clock must be active for 8 clock cycles
    EIGHT_CLOCK_CYCLE_DELAY;

    while(response != STATUS_SUCCESS)
    {
        RESET_RESPONSE;                 //Reset response
        //After receiving response clock must be active for 8 clock cycles
        EIGHT_CLOCK_CYCLE_DELAY;
        
        //Before transmitting application specific commands, the APP_CMD
        //must be transmitted
        spi_xmit_command(APP_CMD, STUFF_BITS, DUMMY_CRC);
        //Wait until card responds with IN_IDLE_STATE response
        while(response != IN_IDLE_STATE)     
            sd_command_response();

        RESET_RESPONSE;                 //Reset response
        //After receiving response clock must be active for 8 clock cycles
        EIGHT_CLOCK_CYCLE_DELAY;    
        
        //Transmit SEND OP COND command                                 
        spi_xmit_command(SD_SEND_OP_COND, STUFF_BITS, DUMMY_CRC);                 
        sd_command_response();          //Receive response
    }
    
    RESET_RESPONSE;                     //Reset response
    //After receiving response clock must be active for 8 clock cycles
    EIGHT_CLOCK_CYCLE_DELAY;            
}
//###################### SD_VERSION1_INITIALIZATION ##########################


//###################### SD_VERSION2_INITIALIZATION ##########################
void sd_version2_initialization()
{
    Uint16 i, send_if_cond_response[4], ccs_check;
    
    //Receive SEND_IF_COND response
    for(i=0;i<4;i++)
        send_if_cond_response[i] = spi_xmit_byte(DUMMY_DATA);

    //If voltage accepted or check pattern does not match, do not communicate further
    if((send_if_cond_response[2] != SUPPLY_VOLTAGE) || (send_if_cond_response[3] != CHECK_PATTERN))
        sd_error();

    RESET_RESPONSE;                     //Reset response
    //After receiving response clock must be active for 8 clock cycles
    EIGHT_CLOCK_CYCLE_DELAY;
    
    while(response != STATUS_SUCCESS)
    {
        RESET_RESPONSE;                 //Reset response
        //After receiving response clock must be active for 8 clock cycles
        EIGHT_CLOCK_CYCLE_DELAY;

        //Before transmitting application specific commands, the APP_CMD
        //must be transmitted
        spi_xmit_command(APP_CMD, STUFF_BITS, DUMMY_CRC);
        while(response != IN_IDLE_STATE)//Wait until card responds with IN_IDLE_STATE response 
            sd_command_response();

        RESET_RESPONSE;                 //Reset response
        //After receiving response clock must be active for 8 clock cycles
        EIGHT_CLOCK_CYCLE_DELAY;
        
        //Transmit SEND_OP_COND command                                 
        spi_xmit_command(SD_SEND_OP_COND, VER2_OP_COND, DUMMY_CRC);               
        sd_command_response();          //Receive response
    }
    
    RESET_RESPONSE;                     //Reset response
    //After receiving response clock must be active for 8 clock cycles
    EIGHT_CLOCK_CYCLE_DELAY;
        
    spi_xmit_command(READ_OCR, STUFF_BITS, DUMMY_CRC);  //Transmit READ_OCR command
    //Wait until card responds with STATUS_SUCCESS response
    while(response != STATUS_SUCCESS)      
        sd_command_response();

    sd_ocr_response();                  //Call OCR response function
    
    ccs_check = ocr_contents[0] & HIGH_CAPACITY; //Mask ocr_contents to test for High Capacity

    if(ccs_check == HIGH_CAPACITY)      //Check if card is High Capacity
        high_capacity = TRUE;                
    
    RESET_RESPONSE;                     //Reset response
    //After receiving response clock must be active for 8 clock cycles
    EIGHT_CLOCK_CYCLE_DELAY;            
}
//###################### SD_VERSION2_INITIALIZATION ##########################
