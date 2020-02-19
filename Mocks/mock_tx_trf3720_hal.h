#ifndef HOST_LINUX_APPS_UNITTEST_MOCKMODULATOR_H_
#define HOST_LINUX_APPS_UNITTEST_MOCKMODULATOR_H_

#include "SFTypes.h"
#include <cstddef>
#include <gmock/gmock.h>
#define NUM_OF_TRF3720_REGS	8
class mock_tx_trf3720
{
public:
	//Singleton class
	static mock_tx_trf3720* getInstance()
	{
		if (_modulator == NULL){
			_modulator = new mock_tx_trf3720();
		}
		return _modulator;
	}
	static void clearInstance()
	{
		if(_modulator != NULL)
		{
			delete _modulator;
			_modulator = NULL;
		}
	}

	SFUINT32 TRF3720_Default[NUM_OF_TRF3720_REGS] = {0x80000008, 0x41500029, 0x08A0100A, 0x0B, 0x4A01F00C, 0xAA2AA28D, 0xA090100E,0x5041100F};
	MOCK_METHOD1(mRegRead, SFUINT32(SFUINT8));
	MOCK_METHOD2(mRegWrite, void(SFUINT8,SFUINT32));
private:
	static mock_tx_trf3720 *_modulator;
	mock_tx_trf3720() {};
	~mock_tx_trf3720() {};
	mock_tx_trf3720(mock_tx_trf3720 const&);
	mock_tx_trf3720& operator=(mock_tx_trf3720 const&);
};

#endif /* HOST_LINUX_APPS_UNITTEST_MOCKMODULATOR_H_ */
