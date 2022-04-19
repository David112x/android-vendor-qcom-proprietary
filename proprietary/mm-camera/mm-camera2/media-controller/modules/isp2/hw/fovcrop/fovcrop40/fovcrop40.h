/* fovcrop40.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __FOVCROP40_H__
#define __FOVCROP40_H__

/* isp headers */
#include "module_fovcrop40.h"

boolean fovcrop40_init(fovcrop40_t *mod);

int fovcrop40_destroy(void *mod_ctrl);

boolean fovcrop40_handle_isp_private_event(mct_module_t *module,
  fovcrop40_t *fovcrop, void *data, uint32_t identity);

boolean fovcrop40_config(fovcrop40_t *fovcrop);

boolean fovcrop40_config_crop_window(fovcrop40_t *fovcrop);

void fovcrop40_reset(fovcrop40_t *mod);

boolean fovcrop40_trigger_update(fovcrop40_t *mod);

void fov40_reset(fovcrop40_t *fovcrop);

#endif //__FOVCROP40_H__
