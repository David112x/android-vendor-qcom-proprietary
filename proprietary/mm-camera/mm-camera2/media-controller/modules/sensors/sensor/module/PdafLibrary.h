/*
Copyright (c) 2016 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

Copyright (c)  2016, Sony Corporation All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __PDAF_LIBRARY_H__
#define __PDAF_LIBRARY_H__

#include "pdaf_lib.h"

/* For PhaseDifference */
/* This value is different by each sensor type */
/* Please set value according to your environment */
/* IMX230, IMX298, IMX330, IMX338 : -32 */
/* Other sensor type : -64 */
#define D_PD_ERROR_VALUE                            (-64)

/* For DensityOfPhasePix */
/* Density of phase detection pixel of mode 0 */
#define D_PD_LIB_DENSITY_SENS_MODE0                 (2304)
/* Density of phase detection pixel of mode 1 */
#define D_PD_LIB_DENSITY_SENS_MODE1                 (2304)
/* Density of phase detection pixel of mode 2 */
#define D_PD_LIB_DENSITY_SENS_MODE2                 (2304/2)
/* Density of phase detection pixel of mode 3 */
#define D_PD_LIB_DENSITY_SENS_MODE3                 (2304/2)
/* Density of phase detection pixel of mode 4 */
#define D_PD_LIB_DENSITY_SENS_MODE4                 (2304/4)

/* For AdjCoeffSlope */
/* Adjustment coefficient of slope */
#define D_PD_LIB_SLOPE_ADJ_COEFF_SENS_MODE0         (2304)
#define D_PD_LIB_SLOPE_ADJ_COEFF_SENS_MODE1         (2304)
#define D_PD_LIB_SLOPE_ADJ_COEFF_SENS_MODE2         (2304)
#define D_PD_LIB_SLOPE_ADJ_COEFF_SENS_MODE3         (2304)
#define D_PD_LIB_SLOPE_ADJ_COEFF_SENS_MODE4         (2304)

#define D_PD_LIB_E_OK                               (0)
#define D_PD_LIB_E_NG                               (-1)

 /* NCW determines Defocus OK/NG by disable this functionality */
#define ENCWDDON                                    (2)
/* Input error value of phase difference data */
#define EPDVALERR                                   (3)
/* XSizeOfImage Input out of range */
#define EINXSOI                                     (40)
/* YSizeOfImage Input out of range */
#define EINYSOI                                     (41)
/* PDAFWindowsX Input out of range */
#define EINPDAFWX                                   (42)
 /* PDAFWindowsY Input out of range */
#define EINPDAFWY                                   (43)
 /* SlopeOffset Input out of range */
#define EINSO                                       (44)
 /* AdjCoeffSlope Input out of range */
#define EINACS                                      (45)
 /* SlopeOffsetXAddressKnot Input out of range */
#define EINSOXAK                                    (46)
/* SlopeOffsetYAddressKnot Input out of range */
#define EINSOYAK                                    (47)
 /* Invalid of Disable Confidence Judgement */
#define EINVALDISCONFJ                              (48)
 /* Invalid of Disable compensation relation with image height */
#define EINVALDISIHC                                (49)
  /* DefocusOKNGThrPointNum Input out of range */
#define EINDONTPN                                   (50)
  /* DefocusOKNGXAddressKnot Input out of range */
#define EINDONXAK                                   (51)
 /* DefocusOKNGYAddressKnot Input out of range */
#define EINDONYAK                                   (52)
 /* DensityOfPhasePix Input out of range */
#define EINDOP                                      (53)
 /* Low DefocusConfidenceLevel */
#define ELDCL                                       (80)

/* ------- PdLibGetVersion API */
#ifdef __cplusplus
extern "C" {
#endif

/* ------- PdLibGetDefocus API */
#if defined __GNUC__
__attribute__ ((visibility ("default"))) signed long PdLibGetDefocus
#elif defined(_DLL)
__declspec( dllexport ) signed long PdLibGetDefocus
#else
/* Get defocus data according to a PDAF window. */
extern signed long PdLibGetDefocus
#endif
(
    void    *input,        /* Input data needed for defocus data output. */
    void    *output        /* Defocus data. */
);

#ifdef __cplusplus
}
#endif          /* __cplusplus */

#endif
