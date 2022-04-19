/*============================================================================
  Copyright (c) 2016 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================*/

#define Kgx00 0x06
#define Kgx09 0x10
#define	Kgx0A 0x11
#define Kgx0B 0x12
#define	Kgx0C 0x13
#define wDgx02 0x8414
#define Kgx0D 0x18
#define Kgx0E 0x19
#define wDgx03 0x841C
#define Kgxdr 0x8436
#define Kgx13 0x843A
#define Kgx14 0x843B
#define Kgx15 0x843C
#define wDgx06 0x843E
#define X_LMT 0x8440
#define X_TGT 0x8443
#define DigGx 0x55
#define DigGy 0x56
#define TMP_X 0x6A
#define TMP_Y 0x6B
#define EQCTL 0x847F
#define Kgy00 0x86
#define wDgy02 0x8494
#define wDgy03 0x849C
#define Kgy13 0xBA
#define Kgy14 0xBB
#define Kgy15 0x84BC
#define wDgy06 0x84BE
#define OIS_STS 0xF7

#define FactAdjName_CURDAT 0x8230
#define FactAdjName_HALOFS_X 0x8231
#define FactAdjName_HALOFS_Y 0x8232
#define FactAdjName_HX_OFS 0x841E
#define FactAdjName_HY_OFS 0x849E
#define FactAdjName_PSTXOF 0x8239
#define FactAdjName_PSTYOF 0x823B
#define FactAdjName_GX_OFS 0x8406
#define FactAdjName_GY_OFS 0x8486
#define FactAdjName_KgxHG 0x8446
#define FactAdjName_KgyHG 0x84C6
#define FactAdjName_KGXG 0x840F
#define FactAdjName_KGYG 0x848F
#define FactAdjName_SFTHAL_X 0x8231
#define FactAdjName_SFTHAL_Y 0x8232
#define FactAdjName_TMP_X 0x846A
#define FactAdjName_TMP_Y 0x846B
#define FactAdjName_KgxH0 0x8470
#define FactAdjName_KgyH0 0x8472

/* Input Frequency [kHz] of CLK/PS terminal (Depend on your system)
   RHM_HT 2013.03.19       Change 6M -> 12M*/
#define CLK_PS (unsigned short)12000

/* Target Frequency [kHz] */
#define FVCO_1 (unsigned short)27000

/* Reference Clock Frequency [kHz] */
#define FREF (unsigned short)25

/* calc DIV_N */
#define DIV_N  (unsigned short)(CLK_PS / FREF - 1)

/* calc DIV_M */
#define DIV_M (unsigned short)(FVCO_1 / FREF - 1)
