/*=============================================================================
Copyright (c) 2019 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

/**
 * @file sp_uim_remote.h
 * @brief - APIs to be called by SPDaemon main loop for setting
 *          sp as bridge between Modem and iUICC on the
 *          SP.
 *
 */

#ifndef _SP_UIM_REMOTE_H_
#define _SP_UIM_REMOTE_H_

/*-------------------------------------------------------------------------
 * Function Definitions
 * ----------------------------------------------------------------------*/
/**
 * sp_uim_remote_create_thread()
 *
 * Creates a pthread so that sp_uim_remote can function in
 * its own independent thread.
 *
 * return: None
 */
#ifdef FEATURE_UIM_REMOTE
void sp_uim_remote_create_thread(void);
#else
static inline void sp_uim_remote_create_thread(void) {}
#endif

#endif /* _SP_UIM_REMOTE_H_ */
