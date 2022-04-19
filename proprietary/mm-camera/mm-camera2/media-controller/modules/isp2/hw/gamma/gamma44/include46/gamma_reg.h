/* gamma_reg.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __GAMMA_REG_H__
#define __GAMMA_REG_H__

/* Macros defines */
#define GAMMA_HW_PACK_BIT             14
#define GAMMA_CHROMATIX_LUT_SIZE      512
#define DEFAULT_CONTRAST              5
#define ISP_GAMMA_NUM_ENTRIES         512
#define GAMMA_CHROMATIX_PACK_BIT      14

#define GAMMA_HW_UNPACK_MASK 0x3FFF
#define MAX_GAMMA_VALUE 0x3FFF

#define SHIFT_BITS 0

#ifdef GAMMA_TABLE_SIZE
#undef GAMMA_TABLE_SIZE
#endif
#define GAMMA_TABLE_SIZE GAMMA_CHROMATIX_LUT_SIZE

#define ISP_RGB_LUT_OFF 0x000006D0
#define ISP_RGB_LUT_LEN 1

#define GAMMA_CGC_OVERRIDE TRUE
#define GAMMA_CGC_OVERRIDE_REGISTER 0x34
#define GAMMA_CGC_OVERRIDE_BIT 3

typedef struct ISP_GammaLutSelect {
  /* LUT Bank Select Config */
  uint32_t      ch0BankSelect               : 1;
  uint32_t      ch1BankSelect               : 1;
  uint32_t      ch2BankSelect               : 1;
  uint32_t  /* reserved */ :29;
}__attribute__((packed, aligned(4))) ISP_GammaLutSelect;


#endif /* __GAMMA_REG_H__ */
