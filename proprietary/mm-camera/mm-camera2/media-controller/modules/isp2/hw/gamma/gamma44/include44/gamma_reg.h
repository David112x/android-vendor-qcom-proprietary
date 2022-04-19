/* gamma_reg.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __GAMMA_REG_H__
#define __GAMMA_REG_H__

/* Macros defines */
#define GAMMA_HW_PACK_BIT             12  /*delta << 12 | base_value*/
#define GAMMA_CHROMATIX_LUT_SIZE      512
#define DEFAULT_CONTRAST              5
#define ISP_GAMMA_NUM_ENTRIES         256
#define GAMMA_CHROMATIX_PACK_BIT      14

#define GAMMA_HW_UNPACK_MASK 0xFFF
#define MAX_GAMMA_VALUE 0xFFF

#define SHIFT_BITS 2
#ifdef GAMMA_TABLE_SIZE
#undef GAMMA_TABLE_SIZE
#endif
#define GAMMA_TABLE_SIZE GAMMA_CHROMATIX_LUT_SIZE

#define ISP_RGB_LUT_OFF 0x00000638
#define ISP_RGB_LUT_LEN 1

#define GAMMA_CGC_OVERRIDE FALSE
#define GAMMA_CGC_OVERRIDE_REGISTER 0x0
#define GAMMA_CGC_OVERRIDE_BIT 0

typedef struct ISP_GammaLutSelect {
  /* LUT Bank Select Config */
  uint32_t      ch0BankSelect               : 1;
  uint32_t      ch1BankSelect               : 1;
  uint32_t      ch2BankSelect               : 1;
  uint32_t  /* reserved */ :29;
}__attribute__((packed, aligned(4))) ISP_GammaLutSelect;


#endif /* __GAMMA_REG_H__ */
