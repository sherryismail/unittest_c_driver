
/*************************************  INCLUDE FILES*****************************************/
#include "tx_trf3720_driver.h"
#include "tx_trf3720_hal.h"
#include "SFTypes.h"
#include "trace.h"
#include <stdio.h>
#include <math.h>
SFUINT32 TRF3720_Default[NUM_OF_TRF3720_REGS] = {0x80000008, 0x41500029, 0x08A0100A, 0x0B, 0x4A01F00C, 0xAA2AA28D, 0xA090100E,0x5041100F};

/***************************************  DEFINES *******************************************/

/**********************************  STRUCTURE DECLARATION **********************************/

/*************************************  GLOBAL VARIABLES ************************************/
tSfRfTxTRF3720Configuration tx_trf3720_config;

/*************************************  FUNCTION DECLARATIONS ***********************************/

/*************************************  FUNCTION DEFINITIONS ************************************/


void TRF3720Configure(SFUINT32 fRF, SFUINT8 RefClk)
{
	//This function sets default config parameters for global struct of TRF3720 device settings. No hardware is accessed yet. This should be called before initialisation.
	tx_trf3720_config.RfTxCenterFrequencyLBand_MHz = fRF;
	tx_trf3720_config.RfTxRefClk_MHz = RefClk;
	tx_trf3720_config.RfStepsize = 1;
	tx_trf3720_config.icp = ICP_CHARGE_PUMP_MAX;
	tx_trf3720_config.cal_clk_sel = CAL_CLK_SEL_16TH;
	tx_trf3720_config.vco_bias = VCO_BIAS_DEFAULT;
	tx_trf3720_config.bufout_bias = BUFOUT_BIAS_300u;
	tx_trf3720_config.en_extvco = 0;
	tx_trf3720_config.i_offset = 128;
	tx_trf3720_config.q_offset = 128;
	tx_trf3720_config.pwd_dc_offset = 1;//0 is ON, 1 is OFF
	tx_trf3720_config.dc_offset_i = DCOFFSET_I_150u;
	tx_trf3720_config.vco_sel = VCO_SEL_DEFAULT;
	tx_trf3720_config.tx_div_bias = DIV_BIAS_37u;
	tx_trf3720_config.lo_div_bias = DIV_BIAS_50u;
}

SFUINT8 TRF372017CalculateDividers(tTRF3720UserCalibrateConfig * calib)
{
	/*  _Configuration is only used but not altered
	 * _UserCalibrateConfig is updated based on users parameters.
	 * This is why pointer to _UserCalibrateConfig is passed to this function
	 *
	 * Initialization sequence:
	 *  start from RF front end and work backwards. Given fRF, fREF and fStepsize.
	 *  1. Pick lo_div_sel for correct fRF range
	 *  2. calculate fVCO = fRF * lo_div_sel
	 *  3. Select pll_div_sel from min(1,2,4) to have fPM <=3000MHz. fPM= fVCO/ pll_div_sel
	 *  4. Select max fPFD that allows required fStepsize
	 *  5. Calculate rdiv = fREF/fPFD
	 *  6. Calculate Nint
	 *  7. Calculate NFRAC (not used)
	 *  8. Select p/p+1 prescale
	 *  9. Verify that fNMax is less than FREQ_N_MAX, if not then increase pll_div_sel and recalculate steps 3-9.
	 */
	/*
	 * TODO:
	 * 1. To introduce the logic of calculating fPFD based on arbitrary RF step and leading to Nint can be done as improvement feature later. Under the current constraints of step Size = 1MHz, this algorithm works fine.
	 * 2. Change all frequencies to kHz to introduce the smallest StepSize of 15kHz. Below 15kHz Nint overflows.
	 * 3. To lead to Nint<20, some floating point calculation is needed that Sx3000b cannot perform. If user wants these settings, the driver can allow user-space to perform floating point calculations, determine precise register values and write the register values directly to hardware */
	SFUINT32 fRF;
	SFUINT32 fVCO;
	SFUINT32 fPFD;
	SFUINT8 prescale;
	//Enter the initialization loop the first time assuming that fNMax is high and set up all reg_data fields.
	SFUINT32 fNMax = FREQ_N_MAX_MHZ + 1;
	fRF = tx_trf3720_config.RfTxCenterFrequencyLBand_MHz;
	if (fRF >= 2400 && fRF < 4800)
	{
		calib->lo_div_sel = 1;
		tx_trf3720_config.lo_div_sel = DIV_SEL_DIV1;
	}
	else if (fRF >= 1200 && fRF < 2400)
	{
		calib->lo_div_sel = 2;
		tx_trf3720_config.lo_div_sel = DIV_SEL_DIV2;
	}
	else if (fRF >= 600 && fRF < 1200)
	{
		calib->lo_div_sel = 4; //do not select 3, will result in fraction
		tx_trf3720_config.lo_div_sel = DIV_SEL_DIV4;
	}
	else if (fRF > 300 && fRF < 600)
	{
		calib->lo_div_sel = 8;
		tx_trf3720_config.lo_div_sel = DIV_SEL_DIV8;
	}
	else {
		calib->lo_div_sel = 2; //default
		tx_trf3720_config.lo_div_sel = DIV_SEL_DIV2;
		printf("TRF3720 - RF freq = %d is out of range\n", fRF);
		return SFERROR;
	}
	tx_trf3720_config.tx_div_sel = tx_trf3720_config.lo_div_sel;
	//-------------fVCO = LO_DIV_SEL * fRF
	fVCO = calib->lo_div_sel * fRF;
	/* PLL_DIV_SEL = Ceiling ( LO_DIV_SEL x fRF / 3000MHz ) for fPM.
	 * Inconsistency between datasheet and GUI to select pll_div_sel in un-used band of 600-700MHz
	 */
	if (fVCO <= FREQ_PM_MIN_MHZ)
	{
		calib->pll_div_sel = 1;
		tx_trf3720_config.pll_div_sel = DIV_SEL_DIV1;
	}
	else if (fVCO > FREQ_PM_MIN_MHZ && fVCO <= FREQ_PM_MAX_MHZ)
	{
		calib->pll_div_sel = 2;
		tx_trf3720_config.pll_div_sel = DIV_SEL_DIV2;
	}
	//-------------set NINT, RDIV, PRSC_SEL and NFRAC (not used)
	while (fNMax > FREQ_N_MAX_MHZ)
	{
		fPFD = tx_trf3720_config.RfStepsize * calib->lo_div_sel
				/ calib->pll_div_sel;
		//-------------set NINT, RDIV, PRSC_SEL and NFRAC (not used)
		calib->Rdiv = tx_trf3720_config.RfTxRefClk_MHz / fPFD;
		calib->Nint = (fVCO * calib->Rdiv)
				/ (tx_trf3720_config.RfTxRefClk_MHz * calib->pll_div_sel);
		if (calib->Nint >= NINT_MAX_INTEGER_MODE) {
			calib->prescale_sel = 1; //for prescale p/p+1 = 8/9
			prescale = 8;
		} else if ((calib->Nint < NINT_MAX_INTEGER_MODE)
				&& (calib->Nint > NINT_MIN_INTEGER_MODE)) {
			calib->prescale_sel = 0;
			prescale = 4;
		} else {
			/*the constraints of user input frequency 900<f<1800 and ref_clk = 10/50 and StepSize <= 1MHz do not lead to 0 <= Nint < 20.
			 * the user only wants improved resolution of RF,Stepsize
			 */
			printf("TRF3720 - Nint = %d is out of range\n", calib->Nint);
			return SFERROR;
		}
		fNMax = fVCO / (calib->pll_div_sel * prescale);

		if (fNMax <= FREQ_N_MAX_MHZ) {
			//all values in  acceptable range
			break;
		} else {
			//recalculate all values. If pll_div=1 make it 2. If pll_div=2 make it 4.
			calib->pll_div_sel = calib->pll_div_sel * 2;
			tx_trf3720_config.pll_div_sel = (calib->pll_div_sel) / 2;
		}
	}
	return SFOK;
}

SFUINT8 TRF372017Init()
{
	//Call TRF3720Configure() before calling this
	tTRF3720UserCalibrateConfig calib;
	SFUINT8 i;
	SFUINT32 reg_data[NUM_OF_TRF3720_REGS];

	TRF372017CalculateDividers((tTRF3720UserCalibrateConfig *)&calib);

	//Reg 1 REF_INV = 0, VCO_POLARITY = 1, ICP=0, ICPDOUBLE = 0, CAL_CLK_SEL = 1/16th
	reg_data[TRF372017_R1] = TRF372017RegRead(TRF372017_R1);
	//clear existing fields from default
	reg_data[TRF372017_R1] &= (~R1_RDIV_MASK) & (~R1_ICP_MASK)
			& (~R1_CAL_CLK_SEL_MASK);
	//enter users values
	reg_data[TRF372017_R1] |= ((calib.Rdiv << R1_RDIV_SHIFT) & R1_RDIV_MASK)
			| ((tx_trf3720_config.icp << R1_ICP_SHIFT) & R1_ICP_MASK)
			| ((tx_trf3720_config.cal_clk_sel << R1_CAL_CLK_SEL_SHIFT)
					& R1_CAL_CLK_SEL_MASK);

	//Reg 2 VCO_SEL = 2, VCOSEL_MODE = 0, CAL_ACC = 0, Execute VCO calib
	reg_data[TRF372017_R2] = TRF372017RegRead(TRF372017_R2);
	//clear existing fields from default
	reg_data[TRF372017_R2] &= (~R2_NINT_MASK) & (~R2_PRSC_SEL_MASK)
			& (~R2_PLL_DIV_SEL_MASK) & (~R2_VCO_SEL_MASK);
	//enter users values
	reg_data[TRF372017_R2] |=
			((calib.Nint << R2_NINT_SHIFT) & R2_NINT_MASK)
			| ((tx_trf3720_config.pll_div_sel << R2_PLL_DIV_SEL_SHIFT) & R2_PLL_DIV_SEL_MASK)
			| ((calib.prescale_sel << R2_PRSC_SEL_SHIFT) & R2_PRSC_SEL_MASK)
			| (R2_EN_CAL_MASK)
			| ((tx_trf3720_config.vco_sel << R2_VCO_SEL_SHIFT) & R2_VCO_SEL_MASK);

	//Reg 3 No fractional setting
	reg_data[TRF372017_R3] = TRF372017RegRead(TRF372017_R3);
	reg_data[TRF372017_R3] &= (~R3_NFRAC_MASK);

	//Reg 4 PWD_OUT_BUFF, PWD_LO_DIV, PWD_TX_DIV, PWD_BB_VCM,PWD_DC_OFF = ON
	reg_data[TRF372017_R4] = TRF372017RegRead(TRF372017_R4);
	reg_data[TRF372017_R4] = (reg_data[TRF372017_R4]
			| ((tx_trf3720_config.en_extvco << R4_EN_EXTVCO_SHIFT) & R4_EN_EXTVCO_MASK)
			| ((tx_trf3720_config.pwd_dc_offset << R4_PWD_DC_OFF_SHIFT)& R4_PWD_DC_OFF_MASK))
			& (~R4_PWD_OUT_BUFF_MASK)
			& (~R4_PWD_LO_DIV_MASK) & (~R4_PWD_TX_DIV_MASK)
			& (~R4_PWD_BB_VCM_MASK);

	//Reg 5 VCOMUX_BIAS = 11, VCO_AMPL_CTRL = 11, Disable ISOURCE (not needed in integer mode), VCO_VB_CTL = 00
	//VCOMUX_BIAS_RECOMMEND sets 600uA and VCC_VCO2 = 5V
	reg_data[TRF372017_R5] = TRF372017RegRead(TRF372017_R5);
	//clear existing fields from default
	reg_data[TRF372017_R5] &= (~R5_VCO_BIAS_MASK) & (~R5_VCOMUX_BIAS_MASK)
			& (~R5_BUFOUT_BIAS_MASK) & (~R5_VCO_CAL_REF_MASK)
			& (~R5_VCO_AMPL_CTRL_MASK) & (~R5_VCO_VB_CTRL_MASK)
			& (~R5_EN_LD_ISOURCE_MASK);
	//enter user defined values
	reg_data[TRF372017_R5] |=
					 ((tx_trf3720_config.vco_bias << R5_VCO_BIAS_SHIFT) & R5_VCO_BIAS_MASK)
					| ((VCOMUX_BIAS_RECOMMEND << R5_VCOMUX_BIAS_SHIFT) & R5_VCOMUX_BIAS_MASK)
					| ((tx_trf3720_config.bufout_bias << R5_BUFOUT_BIAS_SHIFT) & R5_BUFOUT_BIAS_MASK)
					| ((VCO_CAL_REF_RECOMMEND << R5_VCO_CAL_REF_SHIFT) & R5_VCO_CAL_REF_MASK)
					| ((VCO_AMPL_CTRL_RECOMMEND << R5_VCO_AMPL_CTRL_SHIFT) & R5_VCO_AMPL_CTRL_MASK)
					| ((VCO_VB_CTRL_RECOMMEND << R5_VCO_VB_CTRL_SHIFT) & R5_VCO_VB_CTRL_MASK);

	//Reg 6
	reg_data[TRF372017_R6] = TRF372017RegRead(TRF372017_R6);
	//clear existing fields from default
	reg_data[TRF372017_R6] &= (~R6_IOFF_MASK) & (~R6_QOFF_MASK)
			& (~R6_TX_DIV_SEL_MASK) & (~R6_LO_DIV_SEL_MASK)
			& (~R6_TX_DIV_BIAS_MASK) & (~R6_LO_DIV_BIAS_MASK);
	//enter use defined values
	reg_data[TRF372017_R6] |=
			 ((tx_trf3720_config.i_offset << R6_IOFF_SHIFT) & R6_IOFF_MASK)
			| ((tx_trf3720_config.q_offset << R6_QOFF_SHIFT) & R6_QOFF_MASK)
			| ((tx_trf3720_config.tx_div_sel << R6_TX_DIV_SEL_SHIFT) & R6_TX_DIV_SEL_MASK)
			| ((tx_trf3720_config.lo_div_sel << R6_LO_DIV_SEL_SHIFT) & R6_LO_DIV_SEL_MASK)
			| ((tx_trf3720_config.tx_div_bias << R6_TX_DIV_BIAS_SHIFT) & R6_TX_DIV_BIAS_MASK)
			| ((tx_trf3720_config.lo_div_bias << R6_LO_DIV_BIAS_SHIFT) & R6_LO_DIV_BIAS_MASK);

	//Reg 7 VCO_TRIM_SEL - use SPI not EEPROM
	reg_data[TRF372017_R7] = TRF372017RegRead(TRF372017_R7);
	//clear some fields
	reg_data[TRF372017_R7] &= (~R7_DC_OFFSET_I_MASK);
	reg_data[TRF372017_R7] |= R7_VCO_BIAS_SEL_MASK | ((tx_trf3720_config.dc_offset_i <<R7_DC_OFFSET_I_SHIFT)&R7_DC_OFFSET_I_MASK);

	//Write all the prepared fields to SPI bus
	for (i= TRF372017_R1; i < NUM_OF_TRF3720_REGS; i++)
	{
		TRF372017RegWrite(i, reg_data[i]);
	}
	return SFOK;

}

SFUINT8 TRF3720DevBoardInit()
{
	if (TRF372017RegRead(1) != TRF3720_Default[1])
		return SFERROR;

	//fRF is 1550MHz
	TRF372017RegWrite(TRF372017_R1, 0x60100509);
	TRF372017RegWrite(TRF372017_R2, 0x88A0C1CA);
	TRF372017RegWrite(TRF372017_R3, 0x0000000B);
	TRF372017RegWrite(TRF372017_R4, 0x4A01000C);
	TRF372017RegWrite(TRF372017_R5, 0x0D03A28D);//bits [21:20] reserved
	TRF372017RegWrite(TRF372017_R6, 0x958F504E);
	TRF372017RegWrite(TRF372017_R7, 0xD041100F);

	if (TRF372017RegRead(1) == 0x60100509)
		return SFOK;
	else
		return SFERROR;
}

SFUINT32 TRF3720HardwareAlive()
{
	SFUINT8 i = 1;
	SFUINT32 regValue;

	for (i = 1; i < NUM_OF_TRF3720_REGS; i++)
	{
		regValue = TRF372017RegRead(i);
		if (regValue ==TRF3720_Default[i])
			continue;
		else
			return SFERROR;
	}
	return SFOK;
}

void TRF3720ChangeRFCentreFreq(SFUINT32 fRF)
{
	//edit the registers that change centre RF frequency and reset the modulator
	SFUINT32 reg_data[NUM_OF_TRF3720_REGS];
	tTRF3720UserCalibrateConfig calib;
	tx_trf3720_config.RfTxCenterFrequencyLBand_MHz = fRF;

	TRF372017CalculateDividers(&calib);
	//only Reg 1 and reg 2 change
	//disable output RFOUT and LO
	reg_data[TRF372017_R4] = TRF372017RegRead(TRF372017_R4);
	reg_data[TRF372017_R4] |= R4_PWD_LO_DIV_MASK | R4_PWD_TX_DIV_MASK | R4_PWD_OUT_BUFF_MASK;
	TRF372017RegWrite(TRF372017_R4, reg_data[TRF372017_R4]);

	//Reg 1
	reg_data[TRF372017_R1] = TRF372017RegRead(TRF372017_R1);
	//clear existing fields from default
	reg_data[TRF372017_R1] &= (~R1_RDIV_MASK)
			& (~R1_ICP_MASK)
			& (~R1_CAL_CLK_SEL_MASK);
	//enter users values
	reg_data[TRF372017_R1] |= ((calib.Rdiv << R1_RDIV_SHIFT) & R1_RDIV_MASK)
			| ((tx_trf3720_config.icp << R1_ICP_SHIFT) & R1_ICP_MASK)
			| ((tx_trf3720_config.cal_clk_sel << R1_CAL_CLK_SEL_SHIFT)
					& R1_CAL_CLK_SEL_MASK);

	//Reg 2
	reg_data[TRF372017_R2] = TRF372017RegRead(TRF372017_R2);
	//clear existing fields from default
	reg_data[TRF372017_R2] = reg_data[TRF372017_R2] & (~R2_NINT_MASK)
			& (~R2_PRSC_SEL_MASK) & (~R2_PLL_DIV_SEL_MASK) & (~R2_VCO_SEL_MASK);
	//enter users values
	reg_data[TRF372017_R2] =
			reg_data[TRF372017_R2]
					| ((calib.Nint << R2_NINT_SHIFT) & R2_NINT_MASK)
					| ((tx_trf3720_config.pll_div_sel << R2_PLL_DIV_SEL_SHIFT)
							& R2_PLL_DIV_SEL_MASK)
					| ((calib.prescale_sel << R2_PRSC_SEL_SHIFT)
							& R2_PRSC_SEL_MASK) | (R2_EN_CAL_MASK)
					| ((tx_trf3720_config.vco_sel << R2_VCO_SEL_SHIFT)
							& R2_VCO_SEL_MASK);


	//Reg 6
		reg_data[TRF372017_R6] = TRF372017RegRead(TRF372017_R6);
		//clear existing fields from default
		reg_data[TRF372017_R6] = reg_data[TRF372017_R6] & (~R6_TX_DIV_SEL_MASK) & (~R6_LO_DIV_SEL_MASK);
		//enter new values
		reg_data[TRF372017_R6] = reg_data[TRF372017_R6]
				| ((tx_trf3720_config.tx_div_sel << R6_TX_DIV_SEL_SHIFT) & R6_TX_DIV_SEL_MASK)
				| ((tx_trf3720_config.lo_div_sel << R6_LO_DIV_SEL_SHIFT) & R6_LO_DIV_SEL_MASK);

	//Reg 4 - enable output RFOUT and LO
		reg_data[TRF372017_R4] = TRF372017RegRead(TRF372017_R4);
		reg_data[TRF372017_R4] = reg_data[TRF372017_R4]
					& (~R4_PWD_LO_DIV_MASK) & (~R4_PWD_TX_DIV_MASK) & (~R4_PWD_OUT_BUFF_MASK);

		TRF372017RegWrite(TRF372017_R1, reg_data[TRF372017_R1]);
		TRF372017RegWrite(TRF372017_R2, reg_data[TRF372017_R2]);
		TRF372017RegWrite(TRF372017_R6, reg_data[TRF372017_R6]);
		//enable RFOUT and LO in the end
		TRF372017RegWrite(TRF372017_R4, reg_data[TRF372017_R4]);

}

void TRF3720ChangeIQOffset(SFUINT16 i, SFUINT16 q)
{
	tx_trf3720_config.i_offset = i;
	tx_trf3720_config.q_offset = q;
	SFUINT32 reg_data;
	//Reg 6
	reg_data = TRF372017RegRead(TRF372017_R6);
	//clear existing fields
	reg_data &= (~R6_IOFF_MASK) & (~R6_QOFF_MASK);
	//enter use defined values
	reg_data |= ((tx_trf3720_config.i_offset << R6_IOFF_SHIFT) & R6_IOFF_MASK)
			| ((tx_trf3720_config.q_offset << R6_QOFF_SHIFT) & R6_QOFF_MASK);
	TRF372017RegWrite(TRF372017_R6, reg_data);

}

void TRF3720ChangeDCOffset(SFUINT8 OnOff,SFUINT8 dc)
{
	//values allowed are DCOFFSET_I_50u, DCOFFSET_I_100u etc
	tx_trf3720_config.pwd_dc_offset = OnOff; //0 is ON , 1 is OFF
	tx_trf3720_config.dc_offset_i = dc;
	SFUINT32 reg_data;
	//write a value in Reg 7
	reg_data = TRF372017RegRead(TRF372017_R7);
	//clear the field if it had any value
	reg_data &= (~R7_DC_OFFSET_I_MASK);
	reg_data |= ((tx_trf3720_config.dc_offset_i << R7_DC_OFFSET_I_SHIFT) & R7_DC_OFFSET_I_MASK);
	TRF372017RegWrite(TRF372017_R7, reg_data);
	//power up DC offset block
	reg_data = TRF372017RegRead(TRF372017_R4);
	reg_data &= (~R4_PWD_DC_OFF_MASK);
	reg_data |= ((tx_trf3720_config.pwd_dc_offset << R4_PWD_DC_OFF_SHIFT) & R4_PWD_DC_OFF_MASK);
	TRF372017RegWrite(TRF372017_R4, reg_data);
}

void TRF3720PrintAllRegs()
{
	SFUINT8 i;
	SFUINT32 reg_data;
	printf("\n");
	for (i = TRF372017_R1; i < NUM_OF_TRF3720_REGS; i++)
	{
		reg_data = TRF372017RegRead(i);
		printf("Reg[%d] = 0x%x\n",i,reg_data);
	}
}

