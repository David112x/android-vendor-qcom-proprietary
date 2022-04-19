/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __GAMMA40_REG_H__
#define __GAMMA40_REG_H__

#define ISP_RGB_LUT_OFF 0x00000638
#define ISP_RGB_LUT_LEN 1

/* Macros defines */
#define GAMMA_HW_PACK_BIT             8  /*delta << 8 | base_value*/
#define GAMMA_NUM_ENTRIES             64
#define GAMMA_CHROMATIX_LUT_SIZE      512
#define MAX_GAMMA_TABLE_SIZE          512
#define DEFAULT_CONTRAST              5
#define ISP_GAMMA_NUM_ENTRIES         64
#define GAMMA_CHROMATIX_PACK_BIT      8

#define GAMMA_HW_UNPACK_MASK    0xFF /*Coded as per PACK_BIT*/
#define MAX_GAMMA_VALUE         0xFF


typedef struct ISP_GammaLutSelect {
  /* LUT Bank Select Config */
  uint32_t      ch0BankSelect             : 1;
  uint32_t      ch1BankSelect             : 1;
  uint32_t      ch2BankSelect             : 1;
  uint32_t     /* reserved */              : 29;
}__attribute__((packed, aligned(4))) ISP_GammaLutSelect;


#endif //__GAMMA40_REG_H__
