#ifndef SOC_PUBLIC_TX_TRF3720_HAL_H_
#define SOC_PUBLIC_TX_TRF3720_HAL_H_

#include "SFTypes.h"

/****************************************************************************
 *
 *   DEFINES
 *
 ****************************************************************************/
#define NUM_OF_TRF3720_REGS	8
#define TRF3720_SPI_BUS                 		0 //??
#define TRF3720_SPI_CS                  		0 //??

#define SPI_ADDRESS_OFFSET                 	0//17
#define SPI_DATA_OFFSET                    	16// 1

//-------------Register 0 Read-----------------
#define READ_ADDR_MASK    0x00000008
#define READ_ADDR_SHIFT    0U
#define READ_COUNT_MODE_MUX_SEL_MASK    0x08000000
#define READ_COUNT_MODE_MUX_SEL_SHIFT    27U
#define READ_RB_REG_MASK    0x70000000
#define READ_RB_REG_SHIFT    28U
#define READ_RB_ENABLE_MASK    0x80000000
#define READ_RB_ENABLE_SHIFT    31U
#define REG_ADDR_MASK    0x0000001F
#define REG_ADDR_SHIFT    0U

SFUINT32 TRF372017RegRead(SFUINT8 RegAddr);
void TRF372017RegWrite(SFUINT8 RegAddr, SFUINT32 RegData);


#endif /* SOC_PUBLIC_TX_TRF3720_HAL_H_ */
