

#ifndef _TX_TRF3720_DRIVER_H_
#define _TX_TRF3720_DRIVER_H_
#include "SFTypes.h"
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
 *
 *   DEFINES
 *
 ****************************************************************************/

#define FREQ_N_MAX_MHZ			375
#define NINT_MAX_INTEGER_MODE	72
#define NINT_MIN_INTEGER_MODE	20
#define FREQ_PM_MIN_MHZ	2800
/* fPM must not exceed 3000. In GUI, it is set to 2800 so use 2800. For 600<fRF<1199, lo_div_sel is selected as 4. This calculates fVCO as high as 4796. For 1200<fRF<2399, lo_div_sel is selected as 2. this calculates fVCO as high as 4798. */
#define FREQ_PM_MAX_MHZ	4800
/* DIV8=3 will never arise under normal operation because frequencies < 600MHz are beyond L-band range */
#define DIV_SEL_DIV1	0
#define DIV_SEL_DIV2	1
#define DIV_SEL_DIV4 	2
#define DIV_SEL_DIV8 	3

#define DIV_BIAS_25u 0
#define DIV_BIAS_37u 1
#define DIV_BIAS_50u 2
#define DIV_BIAS_62u 3

#define VCO_SEL_DEFAULT	2
#define VCO_SEL_LOW		0
#define VCO_SEL_HIGH	3

#define ICP_CHARGE_PUMP_MAX	0
#define ICP_CHARGE_PUMP_DEFAULT	0xA
#define VCO_BIAS_RECOMMEND	0xF //600uA for VCC=5V
#define VCO_BIAS_DEFAULT	0x8	//460uA
#define VCOMUX_BIAS_RECOMMEND	3
#define VCO_CAL_REF_RECOMMEND	2
#define VCO_AMPL_CTRL_RECOMMEND	3
#define VCO_VB_CTRL_RECOMMEND	0
#define BUFOUT_BIAS_300u	0
#define BUFOUT_BIAS_600u	3
#define DCOFFSET_I_50u	0
#define DCOFFSET_I_100u	1
#define DCOFFSET_I_150u	2
#define DCOFFSET_I_200u	3

#define    CAL_CLK_SEL_128X    0
#define    CAL_CLK_SEL_64X    1
#define    CAL_CLK_SEL_32X    2
#define    CAL_CLK_SEL_16X    3
#define    CAL_CLK_SEL_8X    4
#define    CAL_CLK_SEL_4X    5
#define    CAL_CLK_SEL_2X    6
#define    CAL_CLK_SEL_1    8 //7 not used
#define    CAL_CLK_SEL_HALF    9
#define    CAL_CLK_SEL_4TH    10
#define    CAL_CLK_SEL_8TH    11
#define    CAL_CLK_SEL_16TH    12
#define    CAL_CLK_SEL_32TH    13
#define    CAL_CLK_SEL_64TH    14
#define    CAL_CLK_SEL_128TH    15

#define TRF372017_R0   0x0
#define TRF372017_R1   0x1
#define TRF372017_R2   0x2
#define TRF372017_R3   0x3
#define TRF372017_R4   0x4
#define TRF372017_R5   0x5
#define TRF372017_R6   0x6
#define TRF372017_R7   0x7

//-------------Register 0 Write-----------------
#define R0_CHIP_ID_MASK    0x00000060
#define R0_CHIP_ID_SHIFT    5U
#define R0_NU_MASK    0x00000F80
#define R0_NU_SHIFT    7U
#define R0_R_SAT_ERR_MASK    0x00001000
#define R0_R_SAT_ERR_SHIFT    12U
#define R0_COUNT_NU_MASK    0x00006000
#define R0_COUNT_NU_SHIFT    13U
#define R0_COUNT_VCO_TRIM_MASK    0x001F8000
#define R0_COUNT_VCO_TRIM_SHIFT    15U
#define R0_COUNT_8_NU_MASK    0x00200000
#define R0_COUNT_8_NU_SHIFT    21U
#define R0_COUNT_VCO_SEL_MASK    0x00C00000
#define R0_COUNT_VCO_SEL_SHIFT    22U
#define R0_COUNT_N_MASK    0x7F000000
#define R0_COUNT_N_SHIFT    24U
#define R0_COUNT_MODE_MUX_SEL_MASK    0x80000000
#define R0_COUNT_MODE_MUX_SEL_SHIFT    31U

//-------------Register 1-----------------
#define R1_RDIV_MASK    0x0007FFE0
#define R1_RDIV_SHIFT    5U
#define R1_REF_INV_MASK    0x00080000
#define R1_REF_INV_SHIFT    19U
#define R1_NEG_VCO_MASK    0x00100000
#define R1_NEG_VCO_SHIFT    20U
#define R1_ICP_MASK    0x03E00000
#define R1_ICP_SHIFT    21U
#define R1_ICPDOUBLE_MASK 0x04000000
#define R1_ICPDOUBLE_SHIFT 26U
#define R1_CAL_CLK_SEL_MASK 0x78000000
#define R1_CAL_CLK_SEL_SHIFT 27U

//-------------Register 2-----------------
#define R2_NINT_MASK    0x001FFFE0
#define R2_NINT_SHIFT    5U
#define R2_PLL_DIV_SEL_MASK    0x00600000
#define R2_PLL_DIV_SEL_SHIFT    21U
/*div-by-1=0, div-by-2=1 and div-by-4 = 2. Two options to implement this
 * 1.For correct translation of PLL_DIV_SEL value to its representation in register, _SHIFT step is reduced by 1 to 20U
 * 2. Introduce another variable in _config to save a _DIV2 */
#define R2_PRSC_SEL_MASK    0x00800000
#define R2_PRSC_SEL_SHIFT    23U
#define R2_VCO_SEL_MASK    0x0C000000
#define R2_VCO_SEL_SHIFT    26U
#define R2_VCOSEL_MODE_MASK    0x10000000
#define R2_VCOSEL_MODE_SHIFT    28U
#define R2_CAL_ACC_MASK    0x60000000
#define R2_CAL_ACC_SHIFT    29U
#define R2_EN_CAL_MASK    0x80000000
#define R2_EN_CAL_SHIFT    31U

//-------------Register 3-----------------
#define R3_NFRAC_MASK    0x3FFFFFE0
#define R3_NFRAC_SHIFT    5U

//-------------Register 4-----------------
#define R4_PWD_PLL_MASK    0x00000020
#define R4_PWD_PLL_SHIFT    5U
#define R4_PWD_CP_MASK    0x00000040
#define R4_PWD_CP_SHIFT    6U
#define R4_PWD_VCO_MASK    0x00000080
#define R4_PWD_VCO_SHIFT    7U
#define R4_PWD_VCOMUX_MASK    0x00000100
#define R4_PWD_VCOMUX_SHIFT    8U
#define R4_PWD_DIV124_MASK    0x00000200
#define R4_PWD_DIV124_SHIFT    9U
#define R4_PWD_PRESC_MASK    0x00000400
#define R4_PWD_PRESC_SHIFT    10U
#define R4_PWD_OUT_BUFF_MASK    0x00001000
#define R4_PWD_OUT_BUFF_SHIFT    12U
#define R4_PWD_LO_DIV_MASK    0x00002000
#define R4_PWD_LO_DIV_SHIFT    13U
#define R4_PWD_TX_DIV_MASK    0x00004000
#define R4_PWD_TX_DIV_SHIFT    14U
#define R4_PWD_BB_VCM_MASK    0x00008000
#define R4_PWD_BB_VCM_SHIFT    15U
#define R4_PWD_DC_OFF_MASK    0x00010000
#define R4_PWD_DC_OFF_SHIFT    16U
#define R4_EN_EXTVCO_MASK    0x00020000
#define R4_EN_EXTVCO_SHIFT    17U
#define R4_EN_ISOURCE_MASK    0x00040000
#define R4_EN_ISOURCE_SHIFT    18U
#define R4_LD_ANA_PRESC_MASK    0x00080000
#define R4_LD_ANA_PRESC_SHIFT    19U
#define R4_CP_TRISTATE_MASK    0x00300000
#define R4_CP_TRISTATE_SHIFT    21U
#define R4_SPEEDUP_MASK    0x00800000
#define R4_SPEEDUP_SHIFT    23U
#define R4_LD_DIG_PREC_MASK    0x01000000
#define R4_LD_DIG_PREC_SHIFT    24U
#define R4_EN_DITH_MASK    0x02000000
#define R4_EN_DITH_SHIFT    25U
#define R4_MOD_ORD_MASK    0x0C000000
#define R4_MOD_ORD_SHIFT    26U
#define R4_DITH_SEL_MASK    0x10000000
#define R4_DITH_SEL_SHIFT    28U
#define R4_DEL_SD_CLK_MASK    0x60000000
#define R4_DEL_SD_CLK_SHIFT    29U
#define R4_EN_FRAC_MASK    0x80000000
#define R4_EN_FRAC_SHIFT    31U

//-------------Register 5-----------------
#define R5_VCOBIAS_RTRIM_MASK    0x000000E0
#define R5_VCOBIAS_RTRIM_SHIFT    5U
#define R5_PLLBIAS_RTRIM_MASK    0x00000300
#define R5_PLLBIAS_RTRIM_SHIFT    8U
#define R5_VCO_BIAS_MASK    0x00003C00
#define R5_VCO_BIAS_SHIFT    10U
#define R5_VCOBUF_BIAS_MASK    0x0000C000
#define R5_VCOBUF_BIAS_SHIFT    14U
#define R5_VCOMUX_BIAS_MASK    0x00030000
#define R5_VCOMUX_BIAS_SHIFT    16U
#define R5_BUFOUT_BIAS_MASK    0x000C0000
#define R5_BUFOUT_BIAS_SHIFT    18U
#define R5_VCO_CAL_IB_MASK    0x00400000
#define R5_VCO_CAL_IB_SHIFT    22U
#define R5_VCO_CAL_REF_MASK    0x03800000
#define R5_VCO_CAL_REF_SHIFT    23U
#define R5_VCO_AMPL_CTRL_MASK    0x0C000000
#define R5_VCO_AMPL_CTRL_SHIFT    26U
#define R5_VCO_VB_CTRL_MASK    0x30000000
#define R5_VCO_VB_CTRL_SHIFT    28U
#define R5_EN_LD_ISOURCE_MASK    0x80000000
#define R5_EN_LD_ISOURCE_SHIFT    31U

//-------------Register 6-----------------
#define R6_IOFF_MASK    0x00001FE0
#define R6_IOFF_SHIFT    5U
#define R6_QOFF_MASK    0x001FE000
#define R6_QOFF_SHIFT    13U
#define R6_VREF_SEL_MASK    0x00E00000
#define R6_VREF_SEL_SHIFT    21U
/*div1=0, div2=1, div4 = 2 and div8 = 3 (never used)
 * Even though correct _SHIFT is 24U or 26U but for correct translation of TX_DIV_SEL and LO_DIV_SEL values to their representation in register, _SHIFT can be changed to one step lower.
 * Alternative method is an extra assignment and one extra field in Configuration struct
 */
#define R6_TX_DIV_SEL_MASK    0x03000000
#define R6_TX_DIV_SEL_SHIFT    24U //ideally 24U
#define R6_LO_DIV_SEL_MASK    0x0C000000
#define R6_LO_DIV_SEL_SHIFT    26U //ideally 26U
#define R6_TX_DIV_BIAS_MASK    0x30000000
#define R6_TX_DIV_BIAS_SHIFT    28U
#define R6_LO_DIV_BIAS_MASK    0xC0000000
#define R6_LO_DIV_BIAS_SHIFT    30U

//-------------Register 7-----------------
#define R7_VCO_TRIM_MASK    0x00001F80
#define R7_VCO_TRIM_SHIFT    7U
#define R7_VCO_TEST_MODE_MASK    0x00004000
#define R7_VCO_TEST_MODE_SHIFT    14U
#define R7_CAL_BYPASS_MASK    0x00008000
#define R7_CAL_BYPASS_SHIFT    15U
#define R7_MUX_CTRL_MASK    0x00070000
#define R7_MUX_CTRL_SHIFT    16U
#define R7_ISOURCE_SINK_MASK    0x00080000
#define R7_ISOURCE_SINK_SHIFT    19U
#define R7_ISOURCE_TRIM_MASK    0x00700000
#define R7_ISOURCE_TRIM_SHIFT    20U
#define R7_PD_TC_MASK    0x01800000
#define R7_PD_TC_SHIFT    23U
#define R7_IB_VCM_SEL_MASK    0x02000000
#define R7_IB_VCM_SEL_SHIFT    25U
#define R7_DC_OFFSET_I_MASK    0x60000000
#define R7_DC_OFFSET_I_SHIFT    29U
#define R7_VCO_BIAS_SEL_MASK    0x80000000
#define R7_VCO_BIAS_SEL_SHIFT    31U

/****************************************************************************
 *
 *   STRUCTURES AND TYPES
 *
 ****************************************************************************/
typedef enum {
	WRITE_MOD 	          	= 0, //param1 Register Address, param2 = Bits Value
	WRITE_MASK_MOD 	        = 1, //param1 = Register Address, param2 = Bits Mask, param3 = value to write
	READ_BACK	        = 2, //param1 = Register Address
	WAIT_MS_MOD		    = 3	//param1 = Delay [ms]
}tTRF3720Operations;

typedef struct {
	tTRF3720Operations operation;
	SFUINT16			     param1;//register address
	SFUINT8				     param2;//mask
	SFUINT8				     param3;//value
} tTRF3720operationsArguments;

typedef struct {
	SFUINT32 	RfTxCenterFrequencyLBand_MHz;	// range: [950 MHz : 2400 MHz]
	SFUINT16	icp;
	SFUINT8		RfTxRefClk_MHz;	// range: 10, 50 or 40[MHz]
	SFUINT8		RfStepsize;		//1MHz or 2MHz
	SFUINT8		cal_clk_sel;	//phase detector freq : VCO calib clock
	SFUINT8		tx_div_sel;
	SFUINT8		lo_div_sel;
	SFUINT8		pll_div_sel;
	SFUINT8		vco_bias;
	SFUINT8		bufout_bias;
	SFUINT8 	en_extvco;
	SFUINT8 	vco_sel;
	SFUINT8		i_offset;
	SFUINT8		q_offset;
	SFUINT8		pwd_dc_offset;
	SFUINT8		dc_offset_i;
	SFUINT8		tx_div_bias;
	SFUINT8		lo_div_bias;
}tSfRfTxTRF3720Configuration;

typedef struct {
	SFUINT8		lo_div_sel;
	SFUINT16 	pll_div_sel;
	SFUINT16	Rdiv;	//reference divide value Range 1 to 8191
	SFUINT16	Nint;
	SFUINT8		prescale_sel;
	;
} tTRF3720UserCalibrateConfig;

/****************************************************************************
 *
 *   MACROS
 *
 ****************************************************************************/
extern tSfRfTxTRF3720Configuration tx_trf3720_config;
SFUINT8 TRF372017Init();
SFUINT8 TRF372017CalculateDividers(tTRF3720UserCalibrateConfig * extcalib);
void TRF3720ChangeRFCentreFreq(SFUINT32 fRF);
void TRF3720ChangeIQOffset(SFUINT16 i, SFUINT16 q);
void TRF3720ChangeDCOffset(SFUINT8 OnOff, SFUINT8 dc);
SFUINT32 TRF3720HardwareAlive();
SFUINT8 TRF3720DevBoardInit();
void TRF3720PrintAllRegs();
void TRF3720Configure(SFUINT32 fRF, SFUINT8 RefClk);
#ifdef __cplusplus
}
#endif

#endif /* _TX_TRF3720_DRIVER_H_ */
