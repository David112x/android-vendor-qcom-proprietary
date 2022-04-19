/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "DataCommon.h"
#include "MessageCommon.h"
#include "qmi_client.h"
#include "MessageCommon.h"
#include "WDSModemEndPoint.h"
#include "UnSolMessages/KeepAliveIndMessage.h"
#include "request/StartKeepAliveRequestMessage.h"

namespace rildata {

#define QCRIL_DATA_IPV6_ADDR_LEN QMI_WDS_IPV6_ADDR_LEN_V01
#define MAX_INADDR_LEN 16

 typedef uint32_t qcril_data_ka_v4_address_t;
 typedef uint16_t qcril_data_ka_port_t;

struct keepalive_request_t{
  KeepaliveType_t ka_type;
  qcril_data_ka_v4_address_t v4_dest_addr;
  uint8_t dest_ipv6_address[QCRIL_DATA_IPV6_ADDR_LEN];
  qcril_data_ka_v4_address_t v4_src_addr;
  uint8_t src_ipv6_address[QCRIL_DATA_IPV6_ADDR_LEN];
  qcril_data_ka_port_t src_port;
  qcril_data_ka_port_t dest_port;
  uint32_t timer;
  char apn_name[QMI_WDS_APN_NAME_MAX_V01 + 1];
  uint32_t call_id;
};

struct ma_ka_list_elem_t{
    KeepaliveType_t type;
    vector<uint8_t> sourceAddress;      /* Source address in network-byte order */
    int sourcePort;                          /* Source port if applicable, or 0x7FFFFFFF;
                                                the maximum value is 65535 */
    vector<uint8_t> destinationAddress; /* Destination address in network-byte order */
    int destinationPort;                     /* Destination port if applicable or 0x7FFFFFFF;
                                                the maximum value is 65535 */
    uint32_t handle; /* handle as received from wds_modem_assisted_ka_start_resp_msg_v01 */
    int cid; /* call id for which request is being made */

    KeepaliveStatus_t status;
    struct ma_ka_list_elem_t *next;
};

enum KeepAliveEventEnum : int {
  KeepAliveStartReq = 0,
  KeepAliveStopReq,
  KeepAliveInd,
  KeepAliveCleanCallState,
};

struct KeepAliveEventType{
  std::shared_ptr<Message> msg;
  KeepAliveEventEnum event;
  void *data;
};

struct keep_alive_start_request{
  KeepaliveRequest_t * ril_ka_req;
  string apn;
};

struct keep_alive_ind{
  KeepAliveResult_t status;
  uint32_t handle;
};

class KeepAliveHandler {
private:
    ma_ka_list_elem_t* ma_ka_list_head = NULL;
    std::shared_ptr<WDSModemEndPoint> wds_endpoint;

    ResponseError_t  sendKeepAliveStartRequestMessage( keep_alive_start_request *ka_req, uint32_t *handle);
    void generateStartKeepAliveRequest(const KeepaliveRequest_t* in, wds_modem_assisted_ka_start_req_msg_v01* out, string apn);
    ResponseError_t  sendKeepAliveStopRequestMessage( uint32_t handle , bool bSendUnsol);
    ma_ka_list_elem_t *getKeepAliveElementByHandle(uint32_t handle);
    ma_ka_list_elem_t *createKeepAliveElement(uint32_t handle);
    void insertKeepAliveElement(ma_ka_list_elem_t* elem);
    void deleteKeepAliveElementByCid(int cid);
    void deleteKeepAliveElementByHandle(uint32_t handle);
    void copyKeepAliveElement(ma_ka_list_elem_t *ele, KeepaliveRequest_t* req);
    boolean DetectKeepAliveDuplicateReq(const KeepaliveRequest_t* req);
    boolean matchAddress(const vector<uint8_t>& a, const vector<uint8_t>& b);
    boolean matchPort(int a, int b);
    boolean matchKeepAliveType(KeepaliveType_t a, KeepaliveType_t b);
    void sendKeepAliveUnsol(uint32_t handle, KeepaliveStatusCode status_code);

public:
    KeepAliveHandler();
    void processEvent(const KeepAliveEventType eve);
};
}
