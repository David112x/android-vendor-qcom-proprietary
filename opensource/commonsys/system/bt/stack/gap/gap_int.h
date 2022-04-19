/******************************************************************************
 *
 *  Copyright (C) 2009-2013 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

#ifndef GAP_INT_H
#define GAP_INT_H

#include "bt_common.h"
#include "bt_target.h"
#include "gap_api.h"
#include "gatt_api.h"
#include "osi/include/fixed_queue.h"
#define GAP_MAX_BLOCKS 2 /* Concurrent GAP commands pending at a time*/
/* Define the Generic Access Profile control structure */
typedef struct {
  void* p_data;             /* Pointer to any data returned in callback */
  tGAP_CALLBACK* gap_cback; /* Pointer to users callback function */
  tGAP_CALLBACK* gap_inq_rslt_cback; /* Used for inquiry results */
  uint16_t event;                    /* Passed back in the callback */
  uint8_t index; /* Index of this control block and callback */
  bool in_use;   /* True when structure is allocated */
} tGAP_INFO;

/* The control block for FindAddrByName (Only 1 active at a time) */
typedef struct {
  tGAP_CALLBACK* p_cback;
  /* Pointer to the current inquiry database entry */
  tBTM_INQ_INFO* p_cur_inq;
  tGAP_FINDADDR_RESULTS results;
  bool in_use;
} tGAP_FINDADDR_CB;

typedef struct {
#if (AMP_INCLUDED == TRUE)
  tAMP_APPL_INFO reg_info;
#else
  tL2CAP_APPL_INFO reg_info; /* L2CAP Registration info */
#endif
  tGAP_CCB ccb_pool[GAP_MAX_CONNECTIONS];
} tGAP_CONN;

#define GAP_MAX_CHAR_NUM 4

typedef struct {
  uint16_t handle;
  uint16_t uuid;
  tGAP_BLE_ATTR_VALUE attr_value;
} tGAP_ATTR;
/**********************************************************************
 * M A I N   C O N T R O L   B L O C K
 **********************************************************************/

#define GAP_MAX_CL GATT_CL_MAX_LCB

typedef struct {
  uint16_t uuid;
  tGAP_BLE_CMPL_CBACK* p_cback;
} tGAP_BLE_REQ;

typedef struct {
  RawAddress bda;
  tGAP_BLE_CMPL_CBACK* p_cback;
  uint16_t conn_id;
  uint16_t cl_op_uuid;
  bool in_use;
  bool connected;
  fixed_queue_t* pending_req_q;

} tGAP_CLCB;

typedef struct {
  tGAP_INFO blk[GAP_MAX_BLOCKS];
  tBTM_CMPL_CB* btm_cback[GAP_MAX_BLOCKS];
  uint8_t trace_level;
  tGAP_FINDADDR_CB
      findaddr_cb; /* Contains the control block for finding a device addr */
  tBTM_INQ_INFO* cur_inqptr;

#if (GAP_CONN_INCLUDED == TRUE)
  tGAP_CONN conn;
#endif

  /* LE GAP attribute database */
  tGAP_ATTR gatt_attr[GAP_MAX_CHAR_NUM];
  tGAP_CLCB clcb[GAP_MAX_CL]; /* connection link*/
  tGATT_IF gatt_if;
} tGAP_CB;

extern tGAP_CB gap_cb;
#if (GAP_CONN_INCLUDED == TRUE)
extern void gap_conn_init(void);
#endif
extern void gap_attr_db_init(void);
#endif
