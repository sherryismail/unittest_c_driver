/*
 * unittest.cpp
 *
 *  Created on: Jan 31, 2020
 *      Author: Sheheera ismail
 */
#include <gtest/gtest.h>
#include <stdio.h>

#include "Mocks/mock_tx_trf3720_hal.h"
#include "SFTypes.h"
#include "tx_trf3720_driver.h"
using ::testing::Return;
using ::testing::Invoke;
using ::testing::AtLeast;
using ::testing::_;
//Instance

SFUINT32 RFCentreFreq = 1550;
SFUINT8 RefClk = 40;

/*use cases from TRF3720.vi GUI. these will be used to verify returns from mock device
 * http://www.ti.com/tool/LOOP_FILTER-CALC
 */
//hardware team sets dev_board of IQ modulator with following setting
SFUINT32 fRF1550_Ref40[NUM_OF_TRF3720_REGS] = {0x80000008, 0x60100509, 0x88A0C1CA, 0x0B, 0x4A01000C, 0x0D03A28D, 0x958F504E,0xD041100F};
//fRF 1550MHz and reference clock is 10MHz
SFUINT32 fRF1550_Ref10[NUM_OF_TRF3720_REGS] = {0x80000008, 0x60100149, 0x88A0C1CA, 0x0B, 0x4A01000C, 0x0D23A28D, 0x9590100E,0xD041100F};
//Change fRF = 1100MHz and reference clock is 10MHz
SFUINT32 fRF1100_Ref10[NUM_OF_TRF3720_REGS] = {0x80000008, 0x601000A9, 0x88A0898A, 0x0B, 0x4A01000C, 0x0D23A28D, 0x9A90100E,0xD041100F};
//Change fRF = 2000MHz and Ref_Clk = =50 MHz
SFUINT32 fRF2000_Ref50[NUM_OF_TRF3720_REGS] = {0x80000008, 0x60100649, 0x88A0FA0A, 0x0B, 0x4A01000C, 0x0D23A28D, 0x9590100E,0xD041100F};
//if I/Q offset is changed from default 128/128  to 130/122
SFUINT32 fRF1550_Ref10_I130_Q122[NUM_OF_TRF3720_REGS] = {0x80000008, 0x60100149, 0x88A0C1CA, 0x0B, 0x4A01000C, 0x0D23A28D, 0x958F504E,0xD041100F};
//if DC offset is powered up and changed from default 150uA  to 200uA
SFUINT32 fRF1550_Ref10_DCOn[NUM_OF_TRF3720_REGS] = {0x80000008, 0x60100149, 0x88A0C1CA, 0x0B, 0x4A00000C, 0x0D23A28D, 0x958F504E,0xF041100F};

//turn DC offset Off and change offset value to minimum
SFUINT32 fRF1550_Ref10_DCOff[NUM_OF_TRF3720_REGS] = {0x80000008, 0x60100149, 0x88A0C1CA, 0x0B, 0x4A01000C, 0x0D23A28D, 0x958F504E,0x9041100F};

class TRF3720DriverTest: public testing::Test {
protected:

	virtual void SetUp() {
	}

	virtual void TearDown() {
		mock_tx_trf3720::clearInstance();
	}
};

SFUINT32 DoFakeRead(SFUINT8 RegAddr)
{
	return mock_tx_trf3720::getInstance()->TRF3720_Default[RegAddr];
}
void DoFakeWrite(SFUINT8 RegAddr,SFUINT32 RegData)
{
	mock_tx_trf3720::getInstance()->TRF3720_Default[RegAddr] = RegData;
}

SFUINT8 CompareResults(SFUINT32 mockReturn[],SFUINT32 expectedOutput[])
{
	//compare the values returned by mock device to the expected values
	for (int i=1; i < NUM_OF_TRF3720_REGS; i++)
	{
		if (mockReturn[i] != expectedOutput[i])
			return SFERROR;
	}
	return SFOK;
}

//Read default values of register and match them with datasheet values
TEST_F(TRF3720DriverTest, HardwareAlive) {
	EXPECT_CALL(*mock_tx_trf3720::getInstance(),mRegRead(_)).WillRepeatedly(Invoke(DoFakeRead));
	SFUINT32 status;
	//Read default register values
	status = TRF3720HardwareAlive();
	EXPECT_EQ(status, SFOK);
}

//If hardware is not powered up, the registers will be 0x0
TEST_F(TRF3720DriverTest, HardwareFail) {

	EXPECT_CALL(*mock_tx_trf3720::getInstance(),mRegRead(_)).WillRepeatedly(Invoke(DoFakeRead));
	SFUINT32 status;
	//create an error condition
	mock_tx_trf3720::getInstance()->TRF3720_Default[1] = 0;
	status = TRF3720HardwareAlive();
	EXPECT_EQ(status, SFERROR);
}

//Hardcode the registers with known fields and read back
TEST_F(TRF3720DriverTest, DevBoardInit) {

	EXPECT_CALL(*mock_tx_trf3720::getInstance(),mRegWrite(_,_)).WillRepeatedly(Invoke(DoFakeWrite));
	EXPECT_CALL(*mock_tx_trf3720::getInstance(),mRegRead(_)).WillRepeatedly(Invoke(DoFakeRead));

	SFUINT8 status = TRF3720DevBoardInit();
	EXPECT_EQ(status, SFOK);
	status = CompareResults(mock_tx_trf3720::getInstance()->TRF3720_Default, fRF1550_Ref40);
	EXPECT_EQ(status, SFOK);
}

/*Determine what to write based on users input.
 * This is calibration stage. It is admittedly a misnomer but in line with convention of rest of the code.
 * No hardware calibration is done. At this stage, only local struct is filled with correct values, ready to be written */
TEST_F(TRF3720DriverTest, CalibrateModulator)
{
	RFCentreFreq = 1550;
	RefClk = 40;
	TRF3720Configure(RFCentreFreq,RefClk);
	tTRF3720UserCalibrateConfig calib;
	SFINT8 status = TRF372017CalculateDividers(&calib);
	EXPECT_EQ(status, SFOK);

}

TEST_F(TRF3720DriverTest, Calibrate_with_WrongFreq)
{
	SFINT8 status;
	//Enter out of range frequency [950<f<2100]
	RFCentreFreq = 4850;
	RefClk = 40;
	TRF3720Configure(RFCentreFreq,RefClk);
	tTRF3720UserCalibrateConfig calib;
	status = TRF372017CalculateDividers(&calib);
	EXPECT_EQ(status, SFERROR);

	calib.lo_div_sel = 0;calib.pll_div_sel=0;calib.Rdiv=0;
	calib.Nint=0;calib.prescale_sel=0;
}


/* Change Rf frequency and compare them to tool provided by Texas Instruments GUI TRF3720.vi. Reg 1,2 and 6 should update.
 */
TEST_F(TRF3720DriverTest, Change_RF_Centre_Freq) {
	EXPECT_CALL(*mock_tx_trf3720::getInstance(),mRegRead(_)).WillRepeatedly(Invoke(DoFakeRead));
	EXPECT_CALL(*mock_tx_trf3720::getInstance(),mRegWrite(_,_)).WillRepeatedly(Invoke(DoFakeWrite));
	RFCentreFreq = 1550;
	RefClk = 10;

	TRF3720Configure(RFCentreFreq, RefClk);
	SFUINT8 status = TRF372017Init();
	EXPECT_EQ(status, SFOK);
	status = CompareResults(mock_tx_trf3720::getInstance()->TRF3720_Default,fRF1550_Ref10);
	EXPECT_EQ(status, SFOK);

	RFCentreFreq = 1100;
	TRF3720ChangeRFCentreFreq(RFCentreFreq);
	status = CompareResults(mock_tx_trf3720::getInstance()->TRF3720_Default, fRF1100_Ref10);
	EXPECT_EQ(status, SFOK);

	RFCentreFreq = 2000;
	RefClk = 50;
	//write new value in global config struct of device_driver for RefClk
	tx_trf3720_config.RfTxRefClk_MHz = RefClk;
	TRF3720ChangeRFCentreFreq(RFCentreFreq);
	status = CompareResults(mock_tx_trf3720::getInstance()->TRF3720_Default, fRF2000_Ref50);
	EXPECT_EQ(status, SFOK);
}

//Change I/Q offsets and DC offsets
TEST_F(TRF3720DriverTest, Change_IQ_offsets) {
	EXPECT_CALL(*mock_tx_trf3720::getInstance(),mRegRead(_)).WillRepeatedly(Invoke(DoFakeRead));
	EXPECT_CALL(*mock_tx_trf3720::getInstance(),mRegWrite(_,_)).WillRepeatedly(Invoke(DoFakeWrite));

	RFCentreFreq = 1550;
	RefClk = 10;

	TRF3720Configure(RFCentreFreq, RefClk);
	SFUINT8 status = TRF372017Init();
	status = CompareResults(mock_tx_trf3720::getInstance()->TRF3720_Default, fRF1550_Ref10);
	EXPECT_EQ(status, SFOK);

	SFUINT8 i = 130;
	SFUINT8 q = 122;
	//i and q changed by user, range [0-255]
	TRF3720ChangeIQOffset(i, q);
	status = CompareResults(mock_tx_trf3720::getInstance()->TRF3720_Default, fRF1550_Ref10_I130_Q122);
	EXPECT_EQ(status, SFOK);

	SFUINT8 dc = DCOFFSET_I_200u;
	//dc change from default DCOFFSET_I_150u to DCOFFSET_I_200u
	TRF3720ChangeDCOffset(0, dc);
	status = CompareResults(mock_tx_trf3720::getInstance()->TRF3720_Default,
			fRF1550_Ref10_DCOn);
	EXPECT_EQ(status, SFOK);

	//dc = Turn off DC PWD_DCOFFSET = 1
	dc = DCOFFSET_I_50u;
	TRF3720ChangeDCOffset(1, dc);
	status = CompareResults(mock_tx_trf3720::getInstance()->TRF3720_Default,
			fRF1550_Ref10_DCOff);
	EXPECT_EQ(status, SFOK);
}
