/* color_correct46_reg.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __COLOR_CORRECT46_REG_H__
#define __COLOR_CORRECT46_REG_H__

#define ISP_COLOR_COR46_OFF 0x0000069C
#define ISP_COLOR_COR46_LEN 13


/* Color Correction Config */
typedef struct ISP_ColorCorrectionCfgCmdType {
  /* Color Corr. Coefficient 0 Config */
  int32_t      C0                          :12;
  int32_t     /* reserved */               :4;
  int32_t      RIGHT_C0                    :12;
  int32_t     /* reserved */               :4;
  /* Color Corr. Coefficient 1 Config */
  int32_t      C1                          :12;
  int32_t     /* reserved */               :4;
  int32_t      RIGHT_C1                    :12;
  int32_t     /* reserved */               :4;
  /* Color Corr. Coefficient 2 Config */
  int32_t      C2                          :12;
  int32_t     /* reserved */               :4;
  int32_t      RIGHT_C2                    :12;
  int32_t     /* reserved */               :4;
  /* Color Corr. Coefficient 3 Config */
  int32_t      C3                          :12;
  int32_t     /* reserved */               :4;
  int32_t      RIGHT_C3                    :12;
  int32_t     /* reserved */               :4;
  /* Color Corr. Coefficient 4 Config */
  int32_t      C4                          :12;
  int32_t     /* reserved */               :4;
  int32_t      RIGHT_C4                    :12;
  int32_t     /* reserved */               :4;
  /* Color Corr. Coefficient 5 Config */
  int32_t      C5                          :12;
  int32_t     /* reserved */               :4;
  int32_t      RIGHT_C5                    :12;
  int32_t     /* reserved */               :4;
  /* Color Corr. Coefficient 6 Config */
  int32_t      C6                          :12;
  int32_t     /* reserved */               :4;
  int32_t      RIGHT_C6                    :12;
  int32_t     /* reserved */               :4;
  /* Color Corr. Coefficient 7 Config */
  int32_t      C7                          :12;
  int32_t     /* reserved */               :4;
  int32_t      RIGHT_C7                    :12;
  int32_t     /* reserved */               :4;
  /* Color Corr. Coefficient 8 Config */
  int32_t      C8                          :12;
  int32_t     /* reserved */               :4;
  int32_t      RIGFHT_C8                   :12;
  int32_t     /* reserved */               :4;
  /* Color Corr. Offset 0 Config */
  int32_t      K0                          :11;
  int32_t     /* reserved */               :5;
  int32_t      RIGHT_K0                    :11;
  int32_t     /* reserved */               :5;
  /* Color Corr. Offset 1 Config */
  int32_t      K1                          :11;
  int32_t     /* reserved */               :5;
  int32_t      RIGHT_K1                    :11;
  int32_t     /* reserved */               :5;
  /* Color Corr. Offset 2 Config */
  int32_t      K2                          :11;
  int32_t     /* reserved */               :5;
  int32_t      RIGHT_K2                    :11;
  int32_t     /* reserved */               :5;
  /* Color Corr. Coefficient Q Config */
  uint32_t      coefQFactor                : 2;
  uint32_t     /* reserved */              :14;
  uint32_t      RIGHT_coefQFactor          : 2;
  uint32_t     /* reserved */              :14;
}__attribute__((packed, aligned(4))) ISP_ColorCorrectionCfgCmdType;

#endif /* __COLOR_CORRECT46_REG_H__ */
