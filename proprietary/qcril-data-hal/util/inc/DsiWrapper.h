/*===========================================================================

  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#pragma once

#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "dsi_netctrl.h"
#include <framework/Log.h>
#include "legacy/qcril_data.h"

#define DS_CALL_INFO_ADDR_PCSCF_MAX_LEN (200)

#define AddressFormatIPv4WithSubnetMask(data,mask,str,len)    \
        (void)snprintf( str, len,                                       \
                        "%d.%d.%d.%d/%d%c",                             \
                        ((char*)data)[ 0],((char*)data)[ 1],            \
                        ((char*)data)[ 2],((char*)data)[ 3],            \
                        mask, 0 );

#define AddressFormatIPv6WithPrefixLength(data,mask,str,len) \
        (void)snprintf( str, len,                                            \
                        "%.02x%.02x:%.02x%.02x:%.02x%.02x:%.02x%.02x:"       \
                        "%.02x%.02x:%.02x%.02x:%.02x%.02x:%.02x%.02x/%d%c",  \
                        ((char*)data)[ 0],((char*)data)[ 1],                 \
                        ((char*)data)[ 2],((char*)data)[ 3],                 \
                        ((char*)data)[ 4],((char*)data)[ 5],                 \
                        ((char*)data)[ 6],((char*)data)[ 7],                 \
                        ((char*)data)[ 8],((char*)data)[ 9],                 \
                        ((char*)data)[10],((char*)data)[11],                 \
                        ((char*)data)[12],((char*)data)[13],                 \
                        ((char*)data)[14],((char*)data)[15], mask, 0 );

#define AddressFormatIPv4(data,str,len)                          \
        (void)snprintf( str, len,                                       \
                        "%d.%d.%d.%d%c",                                \
                        ((char*)data)[ 0],((char*)data)[ 1],            \
                        ((char*)data)[ 2],((char*)data)[ 3], 0 );

#define AddressFormatIPv6(data,str,len)                          \
        (void)snprintf( str, len,                                       \
                        "%.02x%.02x:%.02x%.02x:%.02x%.02x:%.02x%.02x:"  \
                        "%.02x%.02x:%.02x%.02x:%.02x%.02x:%.02x%.02x%c",\
                        ((char*)data)[ 0],((char*)data)[ 1],            \
                        ((char*)data)[ 2],((char*)data)[ 3],            \
                        ((char*)data)[ 4],((char*)data)[ 5],            \
                        ((char*)data)[ 6],((char*)data)[ 7],            \
                        ((char*)data)[ 8],((char*)data)[ 9],            \
                        ((char*)data)[10],((char*)data)[11],            \
                        ((char*)data)[12],((char*)data)[13],            \
                        ((char*)data)[14],((char*)data)[15], 0 );

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

/**
 * @brief [brief description]
 * @details [long description]
 *
 * @param dsiHandle [description]
 */
void dsiStopDataCall(dsi_hndl_t dsiHandle);

/**
 * @brief [brief description]
 * @details [long description]
 *
 * @param DsiHandle [description]
 * @param deviceName [description]
 *
 * @return [description]
 */
int dsiGetDeviceName(dsi_hndl_t DsiHandle, std::string &deviceName);

/**
 * @brief [brief description]
 * @details [long description]
 *
 * @param DsiHandle [description]
 * @param ipAddresses [description]
 * @param gatewayAddresses [description]
 * @param dnsAddresses [description]
 * @return [description]
 */
unsigned int dsiGetAddresses(dsi_hndl_t DsiHandle, bool &v4Connected, bool &v6Connected,
    std::string &ipAddresses, std::string &gatewayAddresses, std::string &dnsAddresses);

/**
 * @brief [brief description]
 * @details [long description]
 *
 * @param DsiHandle [description]
 * @return [description]
 */
unsigned int dsiGetMtu(dsi_hndl_t DsiHandle);

/**
 * @brief [brief description]
 * @details [long description]
 *
 * @param DsiHandle [description]
 * @param pcscfAddresses [description]
 *
 * @return [description]
 */
int dsiGetPcscfAddresses(dsi_hndl_t DsiHandle, std::string &pcscfAddresses);


/**
 * @brief [brief description]
 * @details [long description]
 *
 * @param DsiHandle [description]
 * @return [description]
 */
int dsiGetCallEndReason(dsi_hndl_t DsiHandle);

dsi_ce_reason_t dsiGetVerboseCallEndReason(dsi_hndl_t DsiHandle);

bool dsiGetDualIpCallEndReason(dsi_hndl_t DsiHandle, dsi_ce_reason_t &v4Reason,
    dsi_ce_reason_t &v6Reason);

dsi_call_tech_type dsiGetCallTech(dsi_hndl_t DsiHandle);

int dsiTmgiActivate(
  dsi_hndl_t                          hndl,
  char                               *tmgi,
  unsigned int                       *earfcnlist,
  unsigned char                       earfcnlist_len,
  unsigned long                       preemption_priority,
  unsigned int                       *sailist,
  unsigned char                       sailist_len,
  int                                 dbg_trace_id
);

int dsiGetTmgiDeactivateReason (
  dsi_hndl_t                               hndl,
  dsi_embms_tmgi_status_field_type         status_field,
  int                                     *dsi_tmgi_deactivate_reason
);

int dsiTmgiDeactivate (
  dsi_hndl_t hndl,
  char      *tmgi,
  int       dbg_trace_id
);

int dsiTmgiActivateDeactivate (
  dsi_hndl_t                         hndl,
  char                              *activate_tmgi,
  unsigned int                      *earfcnlist,
  unsigned char                      earfcnlist_len,
  unsigned long                      preemption_priority,
  char                              *deactivate_tmgi,
  unsigned int                      *sailist,
  unsigned char                      sailist_len,
  int                                dbg_trace_id
);

int dsiContentDescUpdate (
  dsi_hndl_t                    hndl,
  char                         *content_desc_update_tmgi,
  unsigned char                 content_desc_valid,
  unsigned int                  content_desc_len,
  dsi_embms_content_desc_type  *content_desc,
  int                           dbg_trace_id
);

int dsiGetEmbmsTmgiListquery (
 dsi_hndl_t                               hndl,
 wds_embms_tmgi_list_type_enum_v01        embms_tmgi_list_query,
 wds_embms_tmgi_list_query_resp_msg_v01  *list_info,
 int                                      dbg_trace_id
);

int dsiEmbmsSvcInterestList (
  dsi_hndl_t          hndl,
  char              **svc_interest_tmgi_list,
  unsigned int        svc_interest_tmgi_list_len,
  int                 dbg_trace_id
);
