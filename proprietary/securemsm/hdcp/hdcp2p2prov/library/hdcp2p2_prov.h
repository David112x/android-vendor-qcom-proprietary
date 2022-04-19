#ifndef __HDCP2P2_PROV_H__
#define __HDCP2P2_PROV_H__

/** @file hdcp2p2_prov.h
 * @brief
 * This file contains the definitions of the constants, data structures and
 * and interfaces to the HDCP2P2 SW Provisioning API.
 */
/*===========================================================================
  Copyright (c) 2015, 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE

when       who     what, where, why
--------   ---     ----------------------------------------------------------
10/11/18    tk     Moved location
04/16/15    ck     Created

===========================================================================*/

/*===========================================================================

                     INCLUDE FILES FOR MODULE

===========================================================================*/

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

/**
 * @return Zero on success, negative on failure.
 */
int32_t HDCP2P2_key_provision(
   uint8_t    *key,
   int32_t    key_size,
   uint8_t    *dps,
   int32_t    dps_size);

/**
 * @brief Verify the provisioned key
 *
 * Verifies the provisioned key.
 * Returns error if the device is not provisioned or the key is invalid.
 *
 *
 * @return Zero on success, negative on failure.
 */
int32_t HDCP2P2_key_verify();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__HDCP2P2_PROV_H__

