/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file     ril_utf_defs.h
  @brief    RIL UTF definitions

  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef _QCRIL_UNITTEST_DEFS_H
#define _QCRIL_UNITTEST_DEFS_H

#ifndef RIL_UTF_IOE
#include "cutils/properties.h"
#include "qcrili.h"
#include "qcril_qmi_client.h"
#include "qcril_log.h"
#include "qcrili.h"
#include "qcril_qmi_client.h"
#include "qcril_log.h"
#endif

#include "qmi_i.h"
#include "qmi.h"
#include "qmi_service.h"
#include "qmi_qmux_if.h"
#include "qmi_client.h"
#include "qmi_idl_lib.h"
#include "qmi_util.h"
#include "qmi_platform.h"

#include "qmi_idl_lib_internal.h"
#include "qmi_idl_lib.h"
#include <sys/types.h>
#include <string.h>

//===============================================================
//  QMI dependency functions
//===============================================================
typedef struct qmi_client_struct {
    int service_user_handle;
    qmi_idl_service_object_type p_service;
}qmi_client_struct_type;

//===============================================================
// Data dependency definitions
//===============================================================
#define boolean unsigned char
#define byte    char

#ifndef QCRIL_QMI_NAS_H
typedef unsigned int  qcril_data_hndl_t;

/* Event notifications */
typedef enum
{
  QCRIL_DATA_EVT_CALL_CONNECTED,
  QCRIL_DATA_EVT_CALL_PHYSLINK_UP,
  QCRIL_DATA_EVT_CALL_PHYSLINK_DOWN,
  QCRIL_DATA_EVT_CALL_RELEASED,
  QCRIL_DATA_EVT_MAX
} qcril_data_net_evt_t;

typedef void          qcril_data_evt_payload_t;

/* Client event notification callback function */
typedef void (*qcril_data_net_ev_cb)( qcril_data_hndl_t         hndl,
                                      void                     *user_data,
                                      qcril_data_net_evt_t      evt,
                                      qcril_data_evt_payload_t *payload );
#endif
/* Format: xxx.xxx.xxx.xxx/yy */
#define DS_CALL_INFO_ADDR_IPV4_MAX_LEN (18)
/* Format: xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx/yyy */
#define DS_CALL_INFO_ADDR_IPV6_MAX_LEN (43)

#define DS_CALL_INFO_ADDR_IPV4V6_MAX_LEN (DS_CALL_INFO_ADDR_IPV4_MAX_LEN +  \
                                          DS_CALL_INFO_ADDR_IPV6_MAX_LEN + 1)

#define DS_CALL_INFO_APN_MAX_LEN 101
#define DSI_CALL_INFO_DEVICE_NAME_MAX_LEN 12


#ifndef QCRIL_QMI_NAS_H
typedef struct
{
  char                fmtstr[ DS_CALL_INFO_ADDR_IPV4V6_MAX_LEN + 1 ];
} qcril_data_addr_string_t;

#ifndef RIL_UTF_IOE
typedef struct qcril_data_active_call_info_s
{
  int                   call_id;

  RIL_RadioTechnology   radioTech;                             /* Technology */

  qcril_data_addr_string_t     address;                        /* IPv4 & IPv6 */

  char            apn[ DS_CALL_INFO_APN_MAX_LEN + 1 ];         /* Access Point Name */

  char            dev_name[ DSI_CALL_INFO_DEVICE_NAME_MAX_LEN + 1]; /* Network Interface */

  /* CALL_INACTIVE / CALL_ACTIVE_PHYSLINK_DOWN / CALL_ACTIVE_PHYSLINK_UP */
  int             active;                                      /* State of the data call */

} qcril_data_active_call_info_t;
#endif
/* Switch to toggle data limited system status indications */
typedef enum
{
  LIMITED_SYS_INDICATIONS_MIN = 0x01,
  LIMITED_SYS_INDICATIONS_OFF = LIMITED_SYS_INDICATIONS_MIN,
  LIMITED_SYS_INDICATIONS_ON  = 0x02,
  LIMITED_SYS_INDICATIONS_MAX = LIMITED_SYS_INDICATIONS_ON
}qcril_data_limited_sys_ind_switch_type;


/*Switch to toggle Dormancy indications*/
typedef enum
{
  DORMANCY_INDICATIONS_MIN = 0x01,
  DORMANCY_INDICATIONS_OFF = DORMANCY_INDICATIONS_MIN,
  DORMANCY_INDICATIONS_ON  = 0x02,
  DORMANCY_INDICATIONS_MAX = DORMANCY_INDICATIONS_ON
}qcril_data_dormancy_ind_switch_type;
#endif

/* property_get: returns the length of the value which will never be
** greater than PROPERTY_VALUE_MAX - 1 and will always be zero terminated.
** (the length does not include the terminating zero).
**
** If the property read fails or returns an empty value, the default
** value is used (if nonnull).
*/
//===============================================================
// Data dependency definitions
//===============================================================


typedef struct qmi_sys_event_client_data_type
{
  struct qmi_sys_event_client_data_type   *next;

  qmi_sys_event_rx_hdlr                   sys_event_rx_hdlr;
  void                                    *sys_event_user_data;
  int                                     init_client_handle;
} qmi_sys_event_client_data_type;

//===============================================================
// QCRIL UNITTEST local redefinition of QMI vars
//===============================================================
#endif
