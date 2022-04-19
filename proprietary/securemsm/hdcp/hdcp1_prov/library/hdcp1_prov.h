#ifndef __HDCP1_PROV_H__
#define __HDCP1_PROV_H__

/** @file hdcp1_prov.h
 * @brief
 * This file contains the definitions of the constants, data structures and
 * and interfaces to the HDCP1 SW Provisioning API.
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
08/18/15    ck     Added key verification.
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

#define HDCP1_KEY_FORMAT_CHAMOMILE            2

/**
 * @brief Provision HDCP1 key.
 *
 * The key will be used for HDMI. Error return if HW fuse key is blown.
 * Input key data should be Chamomile encrypted blob.
 *
 * @param[in] key               Chamomile encrypted HDCP1 Key.
 * @param[in] key_size          'key' size.
 * @param[in] key_format        Should be HDCP1_KEY_FORMAT_CHAMOMILE
 * @param[in] *key_format_info  Key specific info per 'key_format' type.
 * @param[in] key_format_info_size  'key_format_info' size
 *
 * @return Zero on success, negative on failure.
 */
int32_t HDCP1_key_provision(
    uint8_t        *key,
    int32_t        key_size,
    uint32_t        key_format,
    void        *key_format_info,
    uint32_t        key_format_info_size);


/**
 * @brief Verify the provisioned key
 *
 * Verifies the provisioned key.
 * Returns error if the device is not provisioned or the key is invalid.
 *
 * @param[in] key_type          Reserved. Will be ignored.
 *
 * @return Zero on success, negative on failure.
 */
int32_t HDCP1_key_verify(uint32_t key_type);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__HDCP1_PROV_H__

