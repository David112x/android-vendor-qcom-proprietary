/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

EnginePluginLibAPI.h

GENERAL DESCRIPTION
Interface file for Engine Plug-in Lib API

=============================================================================*/


#ifndef ENGINE_PLUGIN_LIB_API_H
#define ENGINE_PLUGIN_LIB_API_H

#include <EnginePluginAPI.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Initialize Engine Plugin Lib Interface.
 *        This is a blocking Call.
 *
 * @param[in] pEpIface : Pointer to EPInterface object.
 *
 * @return EPCallbacks* : Pointer to Engine Plugin Callbacks.
*/

const struct EPCallbacks* init_ep_lib_interface(const struct EPInterface* pEpIface);

/**
 * @brief De-initialize Engine Plugin Lib Interface.
 *        This is a blocking Call.
 *
 * @return bool : True/false.
*/
bool deinit_ep_lib_interface();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /** ENGINE_PLUGIN_LIB_API_H */
