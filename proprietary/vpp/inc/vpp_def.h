/*!
 * @file vpp_def.h
 *
 * @cr
 * Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * @services
 */
#ifndef _VPP_DEF_H_
#define _VPP_DEF_H_

#define VPP_UNUSED(x) (void)(x)


enum vpp_bool {
    VPP_FALSE = 0,
    VPP_TRUE = (!VPP_FALSE),
};

typedef enum vpp_resolution t_EVppRes;

typedef enum {
    VPP_COLOR_FMT_NV12_VENUS,
    VPP_COLOR_FMT_MAX,
} t_EVppColorFmt;

typedef enum {
    VPP_IP_HVX,
    VPP_IP_GPU,
    VPP_IP_FRC,
    VPP_IP_MAX,
} t_EVppIp;

/***************************************************************************
 * Function Prototypes
 ***************************************************************************/


#endif /* _VPP_DEF_H_ */
