/*
 * at45.c
 *
 *  Created on: 11.03.2010
 *      Author: Admin
 */

#include "main.h"
#include "at45.h"
//****************************************************************************/
// Data Flash
//****************************************************************************/

//Look-up table for these sizes ->  512k, 1M, 2M, 4M, 8M, 16M, 32M, 64M
const unsigned char DF_pagebits[]  ={  9,  9,  9,  9,  9,  10,  10,  11};	//index of internal page address bits

// Globals
unsigned char PageBits;
//#define DF_SPI_RW spiSendByte

// AT91S_PIO * pPIOA = AT91C_BASE_PIOA;
unsigned char DF_SPI_RW(unsigned char data)
{
    *AT91C_SPI_TDR = data;
    while(!(*AT91C_SPI_SR & AT91C_SPI_TXEMPTY));
    return (*AT91C_SPI_RDR);
}
// Functions
/*****************************************************************************
*
*	Function name : Read_DF_status
*
*	Returns :		One status byte. Consult Dataflash datasheet for further
*					decoding info
*
*	Parameters :	None
*
*	Purpose :		Status info concerning the Dataflash is busy or not.
*					Status info concerning compare between buffer and flash page
*					Status info concerning size of actual device
*
******************************************************************************/
unsigned char Read_DF_status (void)
{
	unsigned char result,index_copy;	//make sure to toggle CS signal in order
	DF_CS_active;				//to reset dataflash command decoder
	result = DF_SPI_RW(StatusReg);		//send status register read op-code
	result = DF_SPI_RW(0x00);		//dummy write to get result
	index_copy = ((result & 0x38) >> 3);	//get the size info from status register
	PageBits   = DF_pagebits[index_copy];	//get number of internal page address bits from look-up table
	DF_CS_inactive;
	return result;				//return the read status register value
}
//----------------------------------------------------------------------------
void wait_DF_busy (void)
{
	while (!(Read_DF_status() & (1 << 7)));
}
/*****************************************************************************
*
*	Function name : Page_To_Buffer
*
*	Returns :		None
*
*	Parameters :	BufferNo	->	Decides usage of either buffer 1 or 2
*					PageAdr		->	Address of page to be transferred to buffer
*
*	Purpose :		Transfers a page from flash to dataflash SRAM buffer
*
******************************************************************************/
void Page_To_Buffer (unsigned int PageAdr, unsigned char BufferNo)
{
	                                                                //make sure to toggle CS signal in order
	DF_CS_active;							//to reset dataflash command decoder

	if (1 == BufferNo)											//transfer flash page to buffer 1
	{
		DF_SPI_RW(FlashToBuf1Transfer);							//transfer to buffer 1 op-code
		DF_SPI_RW((unsigned char)(PageAdr >> (16 - PageBits)));	//upper part of page address
		DF_SPI_RW((unsigned char)(PageAdr << (PageBits - 8)));	//lower part of page address
		DF_SPI_RW(0x00);										//don't cares
	}
	else
	if (2 == BufferNo)											//transfer flash page to buffer 2
	{
		DF_SPI_RW(FlashToBuf2Transfer);							//transfer to buffer 2 op-code
		DF_SPI_RW((unsigned char)(PageAdr >> (16 - PageBits)));	//upper part of page address
		DF_SPI_RW((unsigned char)(PageAdr << (PageBits - 8)));	//lower part of page address
		DF_SPI_RW(0x00);										//don't cares
	}


	DF_CS_inactive;							//initiate the transfer
	while(!(Read_DF_status() & 0x80));				//monitor the status register, wait until busy-flag is high
}



/*****************************************************************************
*
*	Function name : Buffer_Read_Byte
*
*	Returns :		One read byte (any value)
*
*	Parameters :	BufferNo	->	Decides usage of either buffer 1 or 2
*					IntPageAdr	->	Internal page address
*
*	Purpose :		Reads one byte from one of the dataflash
*					internal SRAM buffers
*
******************************************************************************/
unsigned char Buffer_Read_Byte (unsigned char BufferNo, unsigned int IntPageAdr)
{
	unsigned char data;
						      	   //make sure to toggle CS signal in order
	DF_CS_active;					   //to reset dataflash command decoder
	if (1 == BufferNo)				   //read byte from buffer 1
	{
		DF_SPI_RW(Buf1Read);			   //buffer 1 read op-code
		DF_SPI_RW(0x00);			   //don't cares
		DF_SPI_RW((unsigned char)(IntPageAdr>>8)); //upper part of internal buffer address
		DF_SPI_RW((unsigned char)(IntPageAdr));	   //lower part of internal buffer address
		DF_SPI_RW(0x00);			   //don't cares
		data = DF_SPI_RW(0x00);			   //read byte
	}
	else
	if (2 == BufferNo)				   //read byte from buffer 2
	{
		DF_SPI_RW(Buf2Read);			   //buffer 2 read op-code
		DF_SPI_RW(0x00);			   //don't cares
		DF_SPI_RW((unsigned char)(IntPageAdr>>8)); //upper part of internal buffer address
		DF_SPI_RW((unsigned char)(IntPageAdr));	   //lower part of internal buffer address
		DF_SPI_RW(0x00);			   //don't cares
		data = DF_SPI_RW(0x00);			   //read byte
	}
	DF_CS_inactive;
	return data;					   //return the read data byte
}
//-----------------------------------------------------------------------------
void EraseChip (void)
{
  DF_CS_active;
  		DF_SPI_RW(0xC7);			   //buffer 2 read op-code
		DF_SPI_RW(0x94);
                DF_SPI_RW(0x80);
                DF_SPI_RW(0x9A);
                //C7H, 94H, 80H, and 9AH//don't cares
  DF_CS_inactive;
}
/*****************************************************************************
*
*	Function name : Buffer_Read_Str
*
*	Returns :		None
*
*	Parameters :	BufferNo	->	Decides usage of either buffer 1 or 2
*					IntPageAdr	->	Internal page address
*					No_of_bytes	->	Number of bytes to be read
*					*BufferPtr	->	address of buffer to be used for read bytes
*
*	Purpose :		Reads one or more bytes from one of the dataflash
*					internal SRAM buffers, and puts read bytes into
*					buffer pointed to by *BufferPtr
*
******************************************************************************/
void Buffer_Read_Str (unsigned char BufferNo, unsigned int IntPageAdr, unsigned int No_of_bytes, unsigned char *BufferPtr)
{
	unsigned int i;

        BufferPtr = BufferPtr + 5;

                                                                //make sure to toggle CS signal in order
	DF_CS_active;						//to reset dataflash command decoder
	if (1 == BufferNo)					//read byte(s) from buffer 1
	{
                DF_SPI_RW(Buf1Read);				//buffer 1 read op-code
		DF_SPI_RW(0x00);				//don't cares
		DF_SPI_RW((unsigned char)(IntPageAdr>>8));      //upper part of internal buffer address
		DF_SPI_RW((unsigned char)(IntPageAdr));	        //lower part of internal buffer address
		DF_SPI_RW(0x00);				//don't cares
		//DF_SPI_RW(0x00);
                for( i=0; i<No_of_bytes; i++)
		{
			*(BufferPtr) = DF_SPI_RW(0x00);		//read byte and put it in AVR buffer pointed to by *BufferPtr
			BufferPtr++;				//point to next element in AVR buffer
		}
	}
	else
	if (2 == BufferNo)					//read byte(s) from buffer 2
	{
		DF_SPI_RW(Buf2Read);				//buffer 2 read op-code
		DF_SPI_RW(0x00);				//don't cares
		DF_SPI_RW((unsigned char)(IntPageAdr>>8));      //upper part of internal buffer address
		DF_SPI_RW((unsigned char)(IntPageAdr));	        //lower part of internal buffer address
		DF_SPI_RW(0x00);				//don't cares
                //DF_SPI_RW(0x00);
		for( i=0; i<No_of_bytes; i++)
		{
			*(BufferPtr) = DF_SPI_RW(0x00);		//read byte and put it in AVR buffer pointed to by *BufferPtr
			BufferPtr++;				//point to next element in AVR buffer
		}
	}
        DF_CS_inactive;
}
//NB : Sjekk at (IntAdr + No_of_bytes) < buffersize, hvis ikke blir det bare ball..
/*****************************************************************************
*
*	Function name : Buffer_Write_Enable
*
*	Returns :		None
*
*	Parameters :	IntPageAdr	->	Internal page address to start writing from
*			BufferAdr	->	Decides usage of either buffer 1 or 2
*
*	Purpose :	Enables continous write functionality to one of the dataflash buffers
*			buffers. NOTE : User must ensure that CS goes high to terminate
*			this mode before accessing other dataflash functionalities
*
******************************************************************************/
void Buffer_Write_Enable (unsigned char BufferNo, unsigned int IntPageAdr)
{
								//make sure to toggle CS signal in order
	DF_CS_active;						//to reset dataflash command decoder
	if (1 == BufferNo)					//write enable to buffer 1
	{
                DF_SPI_RW(Buf1Write);				//buffer 1 write op-code
		DF_SPI_RW(0x00);				//don't cares
		DF_SPI_RW((unsigned char)(IntPageAdr>>8));      //upper part of internal buffer address
		DF_SPI_RW((unsigned char)(IntPageAdr));	        //lower part of internal buffer address
	}
	else
	if (2 == BufferNo)					//write enable to buffer 2
	{
		DF_SPI_RW(Buf2Write);				//buffer 2 write op-code
		DF_SPI_RW(0x00);				//don't cares
		DF_SPI_RW((unsigned char)(IntPageAdr>>8));      //upper part of internal buffer address
		DF_SPI_RW((unsigned char)(IntPageAdr));	        //lower part of internal buffer address
	}
        DF_CS_inactive;
}
/*****************************************************************************
*
*	Function name : Buffer_Write_Byte
*
*	Returns :		None
*
*	Parameters :	IntPageAdr	->	Internal page address to write byte to
*					BufferAdr	->	Decides usage of either buffer 1 or 2
*					Data		->	Data byte to be written
*
*	Purpose :		Writes one byte to one of the dataflash
*					internal SRAM buffers
*
******************************************************************************/
void Buffer_Write_Byte (unsigned char BufferNo, unsigned int IntPageAdr, unsigned char Data)
{

								//make sure to toggle CS signal in order
	DF_CS_active;						//to reset dataflash command decoder
	if (1 == BufferNo)					//write byte to buffer 1
	{
		DF_SPI_RW(Buf1Write);				//buffer 1 write op-code
		DF_SPI_RW(0x00);				//don't cares
		DF_SPI_RW((unsigned char)(IntPageAdr>>8));      //upper part of internal buffer address
		DF_SPI_RW((unsigned char)(IntPageAdr));	        //lower part of internal buffer address
		DF_SPI_RW(Data);				//write data byte
	}
	else
	if (2 == BufferNo)					//write byte to buffer 2
	{
		DF_SPI_RW(Buf2Write);				//buffer 2 write op-code
		DF_SPI_RW(0x00);				//don't cares
		DF_SPI_RW((unsigned char)(IntPageAdr>>8));      //upper part of internal buffer address
		DF_SPI_RW((unsigned char)(IntPageAdr));	        //lower part of internal buffer address
		DF_SPI_RW(Data);				//write data byte
	}
        DF_CS_inactive;
}
/*****************************************************************************
*
*	Function name : Buffer_Write_Str
*
*	Returns :		None
*
*	Parameters :	BufferNo	->	Decides usage of either buffer 1 or 2
*			IntPageAdr	->	Internal page address
*			No_of_bytes	->	Number of bytes to be written
*			*BufferPtr	->	address of buffer to be used for copy of bytes
*			from AVR buffer to dataflash buffer 1 (or 2)
*
*	Purpose :		Copies one or more bytes to one of the dataflash
*					internal SRAM buffers from AVR SRAM buffer
*					pointed to by *BufferPtr
*
******************************************************************************/
void Buffer_Write_Str (unsigned char BufferNo, unsigned int IntPageAdr, unsigned int No_of_bytes, unsigned char *BufferPtr)
{
	unsigned int i;
									//make sure to toggle CS signal in order
	DF_CS_active;							//to reset dataflash command decoder
	if (1 == BufferNo)						//write byte(s) to buffer 1
	{
		DF_SPI_RW(AutoPageReWrBuf1);//Buf1Write);					//buffer 1 write op-code
		DF_SPI_RW(0x00);					//don't cares
		DF_SPI_RW((unsigned char)(IntPageAdr>>8));              //upper part of internal buffer address
		DF_SPI_RW((unsigned char)(IntPageAdr));	                //lower part of internal buffer address
		for( i=0; i<No_of_bytes; i++)
		{
			DF_SPI_RW(*(BufferPtr));			//write byte pointed at by *BufferPtr to dataflash buffer 1 location
			BufferPtr++;					//point to next element in AVR buffer
		}
	}
	else
	if (2 == BufferNo)						//write byte(s) to buffer 2
	{
		DF_SPI_RW(Buf2Write);					//buffer 2 write op-code
		DF_SPI_RW(0x00);					//don't cares
		DF_SPI_RW((unsigned char)(IntPageAdr>>8));              //upper part of internal buffer address
		DF_SPI_RW((unsigned char)(IntPageAdr));	                //lower part of internal buffer address
		for( i=0; i<No_of_bytes; i++)
		{
			DF_SPI_RW(*(BufferPtr));			//write byte pointed at by *BufferPtr to dataflash buffer 2 location
			BufferPtr++;					//point to next element in AVR buffer
		}
	}
        DF_CS_inactive;
}
//NB : Monitorer busy-flag i status-reg.
//NB : Sjekk at (IntAdr + No_of_bytes) < buffersize, hvis ikke blir det bare ball..
/****************************************************************************
*
*	Function name : Buffer_To_Page
*
*	Returns :		None
*
*	Parameters :	BufferAdr	->	Decides usage of either buffer 1 or 2
*			PageAdr		->	Address of flash page to be programmed
*
*	Purpose :		Transfers a page from dataflash SRAM buffer to flash
*		                Запись буфера в флеш
******************************************************************************/
void Buffer_To_Page (unsigned char BufferNo, unsigned int PageAdr)
{
									//make sure to toggle CS signal in order
	DF_CS_active;							//to reset dataflash command decoder

		if (1 == BufferNo)											//program flash page from buffer 1
	{
		DF_SPI_RW(Buf1ToFlashWE);								//buffer 1 to flash with erase op-code
		DF_SPI_RW((unsigned char)(PageAdr >> (16 - PageBits)));	//upper part of page address
		DF_SPI_RW((unsigned char)(PageAdr << (PageBits - 8)));	//lower part of page address
		DF_SPI_RW(0x00);										//don't cares
	}
	else
	if (2 == BufferNo)											//program flash page from buffer 2
	{
		DF_SPI_RW(Buf2ToFlashWE);								//buffer 2 to flash with erase op-code
		DF_SPI_RW((unsigned char)(PageAdr >> (16 - PageBits)));	//upper part of page address
		DF_SPI_RW((unsigned char)(PageAdr << (PageBits - 8)));	//lower part of page address
		DF_SPI_RW(0x00);										//don't cares
	}

	DF_CS_inactive;							//initiate flash page programming
	while(!(Read_DF_status() & 0x80));
}
/*****************************************************************************
*
*	Function name : Cont_Flash_Read_Enable
*
*	Returns :		None
*
*	Parameters :	PageAdr		->	Address of flash page where cont.read starts from
*					IntPageAdr	->	Internal page address where cont.read starts from
*
*	Purpose :		Initiates a continuous read from a location in the DataFlash
*
******************************************************************************/
void Cont_Flash_Read_Enable (unsigned int PageAdr, unsigned int IntPageAdr)
{
											//make sure to toggle CS signal in order
	DF_CS_active;									//to reset dataflash command decoder
	DF_SPI_RW(ContArrayRead);							//Continuous Array Read op-code
	DF_SPI_RW((unsigned char)(PageAdr >> (16 - PageBits)));				//upper part of page address
	DF_SPI_RW((unsigned char)((PageAdr << (PageBits - 8))+ (IntPageAdr>>8)));	//lower part of page address and MSB of int.page adr.
	DF_SPI_RW((unsigned char)(IntPageAdr));						//LSB byte of internal page address
	DF_SPI_RW(0x00);								//perform 4 dummy writes
	DF_SPI_RW(0x00);								//in order to intiate DataFlash
	DF_SPI_RW(0x00);								//address pointers
	DF_SPI_RW(0x00);
        DF_CS_inactive;
}

// *****************************[ End Of DATAFLASH.C ]*************************
