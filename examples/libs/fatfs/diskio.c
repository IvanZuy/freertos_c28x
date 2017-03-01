/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "uart.h"
#include "diskio.h"		/* FatFs lower layer API */
#include "SD.h"

/* Definitions of physical drive number for each drive */
#define DEV_MMC		0	/* Example: Map MMC/SD card to physical drive 1 */

#define SECTOR_SIZE       512
#define READ_BUFF_SECTORS 8
#define BUFF_INVALID      -1

#pragma DATA_SECTION(readBuff, "ramgs2");
uint8_t readBuff[READ_BUFF_SECTORS * SECTOR_SIZE];
int32_t readBuffStartSector;

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
  readBuffStartSector = BUFF_INVALID;
  return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
  if(  (readBuffStartSector == BUFF_INVALID)
     ||(sector < readBuffStartSector)
     ||(sector >= (readBuffStartSector + READ_BUFF_SECTORS)))
  {
    sd_read_multiple_block(sector, (Uint16*)readBuff, READ_BUFF_SECTORS);
    readBuffStartSector = sector;
  }
  memcpy(buff, readBuff + ((sector - readBuffStartSector) * SECTOR_SIZE), SECTOR_SIZE);

  return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
  sd_write_block(sector, (Uint16*)buff);
  if((sector >= readBuffStartSector) && (sector < (readBuffStartSector + READ_BUFF_SECTORS)))
  {
    memcpy(readBuff + ((sector - readBuffStartSector) * SECTOR_SIZE), buff, SECTOR_SIZE);
  }
  return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
  return RES_OK;
}

