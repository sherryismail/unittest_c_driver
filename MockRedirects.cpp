#include "SFTypes.h"
#include <tx_trf3720_hal.h>
#include "Mocks/mock_tx_trf3720_hal.h"
#include <cstddef>
mock_tx_trf3720* mock_tx_trf3720::_modulator = NULL;
SFUINT32 TRF372017RegRead(SFUINT8 RegAddr)
{
	return mock_tx_trf3720::getInstance()->mRegRead(RegAddr);
}
void TRF372017RegWrite(SFUINT8 RegAddr, SFUINT32 RegData)
{
	mock_tx_trf3720::getInstance()->mRegWrite(RegAddr,RegData);
}
