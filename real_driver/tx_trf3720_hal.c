#include "tx_trf3720_driver.h"//../Public/
#include "SFTypes.h"
#include "spi_driver.h"
#include "tx_trf3720_hal.h"

#define NUM_OF_BYTES	4
SFUINT32 TRF372017RegRead(SFUINT8 RegAddr)
{
	SFUINT32	spi_wr_data;
	SFUINT32	spi_rd_data;
	SFUINT8 ReadMode = 1;
	SFUINT8 WriteMode = 0;
	//Write 4 bytes into register 0 to turn on Read_back mode, provide register address to be read
	//Read back 4 bytes from the bus
	/* Note: The Read command is formed of two consecutive transactions:
	 * 1. Host write bits [31]ReadBackMode - [30:28](1 to 7) -  [27] Count mode Mux select
	 * 2. Terminal write bits [7:0] - Data [7:0] */
	/*    23                20                                                             8    7                                  0   */
	/* | R/W | A14 | A13 | A12 | A11 | A10 | A9 | A8 | A7 | A6 | A5 | A4 | A3 | A2 | A1 | A0 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | */
	spi_wr_data = (READ_RB_ENABLE_MASK) | (READ_RB_REG_MASK & (RegAddr << READ_RB_REG_SHIFT));
	spi_rd_data = SfSpiWriteBytesReadBytes(TRF3720_SPI_BUS, TRF3720_SPI_CS, SPI_POL1, spi_wr_data, NUM_OF_BYTES, NUM_OF_BYTES);

	return spi_rd_data;
}

void TRF372017RegWrite(SFUINT8 RegAddr, SFUINT32 RegData)
{
	SFUINT32 spi_wr_data;

    /* 3-wire SPI (clk,data, Latch enable). 4th pin RDBK (readback)
     * little endian, LSB first.
     * Note: The Write command is formed of a single transaction:
     * 1. Control fields [31:6] - Internal regAddr [5:0]
     * | D31 | D30 | D29 | D28 | D27 | D26 | D25 | D24 | D23 | D22 | D21 | D20 | D19 | D18 | D17 | D16 | D15 | D14 | D13 | D12 | D10 | D9 | D8 | D7 | D6 | A5 | A4 | A3 | A2 | A1 | A0 | */

	/* Build spi transaction*/
    spi_wr_data = ((RegAddr + NUM_OF_TRF3720_REGS) & REG_ADDR_MASK) |  RegData;

    SfSpiWrite(TRF3720_SPI_BUS, TRF3720_SPI_CS, SPI_POL1, SFTRUE, NUM_OF_BYTES, spi_wr_data);
}
