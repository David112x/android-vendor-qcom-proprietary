/*============================================================================
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <stdlib.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "camera_dbg.h"
#include "cam_intf.h"
#include "mct_controller.h"
#include "mct_profiler.h"
#include "modules.h"
#include "iface_def.h"
#include "iface.h"
#include "iface_util.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif


extern int port_iface_create_ports(iface_t *iface);

/** module_iface_process_event_func:
 *    @module: iface MCTL Module object
 *    @event: MCTL event to module
 *
 *  This function runs in MCTL thread context.
 *
 *  This function implement iface module process event method - currently empty
 *
 *  Return: FALSE
 **/
static boolean  module_iface_process_event_func(
  mct_module_t *module __unused,
  mct_event_t  *event __unused)
{
  return FALSE;
}

/** module_iface_send_event_func:
 *    @module: iface MCTL Module object
 *    @event: MCTL event to module
 *
 *  This function runs in MCTL thread context.
 *
 *  This function implement iface module send event method -
 *  currently empty
 *
 *  Return: FALSE
 **/
static boolean  module_iface_send_event_func(
  mct_module_t *module __unused,
  mct_event_t  *event __unused)
{
  return FALSE;
}

/** module_iface_set_mod_func:
 *    @module: iface MCTL Module object
 *    @module_type: MCTL module type
 *    @identity: identity
 *
 *  This function runs in MCTL thread context.
 *
 *  This function implement iface module set mod method - currently empty
 *
 *  Return: None
 **/
static void module_iface_set_mod_func(mct_module_t *module,
  unsigned int module_type,
  unsigned int identity)
{
  CDBG_HIGH("%s: set module type = %d, identity %x\n", __func__, module_type, identity);
  mct_module_add_type(module, module_type, identity);
  return;
}

/** module_iface_query_mod_func:
 *    @module: iface MCTL Module object
 *    @buf: buffer for querried info
 *    @sessionid: session ID
 *
 *  This function runs in MCTL thread context.
 *
 *  This function implement iface module query mod method - currently empty
 *
 *  Return: TRUE
 **/
static boolean  module_iface_query_mod_func(
  mct_module_t *module __unused,
  void         *buf __unused,
  unsigned int sessionid __unused)
{
  mct_pipeline_cap_t     *cap_buf;
  mct_pipeline_common_cap_t *common_cap;

  if (!buf || !module) {
    CDBG_ERROR("failed buf %p module %p", buf, module);
    return FALSE;
  }

  cap_buf = (mct_pipeline_cap_t *)buf;
  common_cap = &(cap_buf->common_cap);
  common_cap->plane_padding = mct_util_calculate_lcm(
    common_cap->plane_padding, CAM_PAD_TO_32);
  CDBG("%s: PADDING common_cap->plane_padding %d", __func__,
    common_cap->plane_padding);

  return TRUE;
}

/** module_iface_free_mod_func:
 *    @module: iface MCTL Module object
 *
 *  This function runs in MCTL thread context.
 *
 *  This function implement iface module free mod method - currently empty
 *
 *  Return: None
 **/
static void module_iface_free_mod_func(mct_module_t *module __unused)
{
  return;
}

/** module_iface_start_session:
 *    @module: iface MCTL Module object
 *    @sessionid: session ID
 *
 *  This function runs in MCTL thread context.
 *
 *  This function starts iface module session
 *
 *  Return: TRUE  - Success
 *          FALSE - Session not started
 **/
static boolean module_iface_start_session(mct_module_t *module,
  unsigned int sessionid)
{
  boolean rc = FALSE;
  iface_t *iface;
  int ret = 0;

  CDBG("%s: E\n", __func__);
  /* should not happen */
  assert(module != NULL);
  assert(module->module_private != NULL);

  iface = module->module_private;

  MCT_PROF_LOG_BEG(PROF_IFACE_START_SESSION);

  pthread_mutex_lock(&iface->mutex);
  ret = iface_start_session(iface, sessionid);
  pthread_mutex_unlock(&iface->mutex);
  rc = (ret == 0)? TRUE : FALSE;

  MCT_PROF_LOG_END();

  return rc;
}

/** module_iface_stop_session:
 *
 *  This function runs in MCTL thread context.
 *
 *  This function stops iface module session
 *
 *  Return: TRUE  - Success
 *          FALSE - Error while stopping session
 **/
static boolean module_iface_stop_session(
  mct_module_t *module, unsigned int sessionid)
{
  boolean rc = FALSE;
  int ret = 0;
  iface_t *iface = NULL;

  /* should not happen */
  assert(module != NULL);
  assert(module->module_private != NULL);
  iface = module->module_private;

  MCT_PROF_LOG_BEG(PROF_IFACE_STOP_SESSION);

  pthread_mutex_lock(&iface->mutex);

  ret = iface_stop_session(iface, sessionid);

  pthread_mutex_unlock(&iface->mutex);
  rc = (ret == 0)? TRUE : FALSE;

  MCT_PROF_LOG_END();

  return rc;
}

/** module_iface_set_session_data: set session data
 *
 *  @module: iface module handle
 *  @set_buf: set buffer handle that has session data
 *  @sessionid: session id for which session data shall be
 *            applied
 *
 *  This function provides session data that has per frame
 *  contorl parameters
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_iface_set_session_data(mct_module_t *module,
  void *set_buf, unsigned int session_id)
{
  mct_pipeline_session_data_t *session_data;
  iface_t *iface;
  boolean rc = FALSE;
  int ret = 0;

  if (!module || !set_buf) {
    CDBG_ERROR("%s:%d failed %p %p\n", __func__, __LINE__, module, set_buf);
    return FALSE;
  }

  iface = (iface_t *)module->module_private;
  if (!iface) {
    CDBG_ERROR("%s: iface NULL\n", __func__);
    return FALSE;
  }
  session_data = (mct_pipeline_session_data_t *)set_buf;

  ret = iface_set_session_data(iface, session_id, session_data);
  rc = (ret == 0) ? TRUE : FALSE;

  return rc;
}

/** module_iface_init:
 *    @name: name of initialized module
 *
 *  This function runs in MCTL thread context.
 *
 *  This function creates and initializes ispif module
 *
 *  Return: NULL  - Could not initialize module
 *          Otherwise - pointer to initialized module
 **/
mct_module_t *module_iface_init(const char *name)
{
  int rc = 0;
  mct_module_t *module_iface = NULL;
  iface_t *iface = NULL;

  #ifdef _ANDROID_
  iface_util_setloglevel();
  #endif

  CDBG("%s: E\n", __func__);
  assert(name != NULL);

  /* Create MCT module for sensor */
  module_iface = mct_module_create(name);
  if (!module_iface) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return NULL;
  }

  /*Create isp interfface instance*/
  iface = (iface_t *)malloc(sizeof(iface_t));
  if (!iface) {
    CDBG_ERROR("%s: no mem", __func__);
    goto error;
  }

  memset(iface, 0,  sizeof(iface_t));
  iface->ispif_data.ispif_hw.fd = -1;
  iface->vfe1_reserved_rdi = -1;
  rc = iface_init(iface);
  if (rc < 0) {
    CDBG_ERROR("%s: cannot create iface\n", __func__);
    goto error;
  }

  iface->module = module_iface;
  if (0 != (rc = port_iface_create_ports(iface))) {
    CDBG_ERROR("%s: create sink port error = %d", __func__, rc);
    goto error;
  }
  iface->module->process_event = NULL;
  iface->module->set_mod = module_iface_set_mod_func;
  iface->module->query_mod = module_iface_query_mod_func;
  iface->module->start_session = module_iface_start_session;
  iface->module->stop_session = module_iface_stop_session;
  iface->module->set_session_data = module_iface_set_session_data;
  iface->module->module_private = iface;

  return iface->module;

error:
  if (iface) {
    free (iface);
  }
  mct_module_destroy(module_iface);

  return NULL;
}

/** module_iface_deinit:
 *    @module: iface MCTL module object
 *
 *  This function runs in MCTL thread context.
 *
 *  This function destroys iface module - not implemented
 *
 *  Return: None
 **/
void module_iface_deinit(mct_module_t *module)
{
  iface_t *iface = NULL;

  iface = (iface_t *)module->module_private;
  if (iface == NULL) {
    CDBG_ERROR("%s: deinit fail\n", __func__);
    return;
  }

  iface_deinit(iface);

  return;
}
