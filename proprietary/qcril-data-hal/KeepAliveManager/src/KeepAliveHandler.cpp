/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include "KeepAliveHandler.h"
#include "qcril_log.h"
#include "qmi_client.h"
#include "UnSolMessages/KeepAliveIndMessage.h"
#include "modules/qmi/ModemEndPointFactory.h"
#include "MessageCommon.h"
#include "request/StartKeepAliveRequestMessage.h"
#include "request/StopKeepAliveRequestMessage.h"
#include "UnSolMessages/RadioKeepAliveStatusIndMessage.h"

#ifdef QMI_RIL_UTF
#include <arpa/inet.h>
#endif
using namespace rildata;

KeepAliveHandler::KeepAliveHandler()
{
  wds_endpoint = ModemEndPointFactory<WDSModemEndPoint>::getInstance().buildEndPoint();
}

void KeepAliveHandler::processEvent(const KeepAliveEventType e)
{
  switch(e.event) {
      case KeepAliveStartReq:
      {
        Log::getInstance().d("processEvent(): handling KeepAliveStartReq");
        ResponseError_t status = ResponseError_t::INTERNAL_ERROR;
        StartKeepAliveResp_t res;
        res.handle = 0;
        res.status = KeepaliveStatus_t::NONE;
        res.error = status;
        if(e.data != nullptr)
        {
          keep_alive_start_request *ka_req = static_cast<keep_alive_start_request *>(e.data);
          uint32_t handle = 0;
          status = sendKeepAliveStartRequestMessage(ka_req, &handle);
          if (status == ResponseError_t::NO_ERROR) {
            ma_ka_list_elem_t *ele = createKeepAliveElement(handle);
            if(ele != NULL)
            {
              copyKeepAliveElement(ele, ka_req->ril_ka_req);
              insertKeepAliveElement(ele);
              res.handle = ele->handle;
              res.status = ele->status;
            }
            else
            {
              Log::getInstance().d("KeepAliveStartReq : Failed create element");
              status =  ResponseError_t::INTERNAL_ERROR;
            }
          }
        }
        else
        {
          status = ResponseError_t::INVALID_ARGUMENT;
        }
        res.error = status;
        std::shared_ptr<StartKeepAliveRequestMessage> m = std::static_pointer_cast<StartKeepAliveRequestMessage>(e.msg);
        if(m != nullptr)
        {
          auto resp = std::make_shared<StartKeepAliveResp_t>(res);
          m->sendResponse(e.msg, Message::Callback::Status::SUCCESS, resp);
        }
        else
        {
          Log::getInstance().d("StartKeepAliveRequestMessage Failed");
        }
      break;
    }

      case KeepAliveStopReq:
      {
        Log::getInstance().d("processEvent(): handling KeepAliveStopReq");
        ResponseError_t status;
        uint32_t *handle = static_cast<uint32_t*>(e.data);
        if(e.data!=NULL)
        {
          if(handle != NULL && getKeepAliveElementByHandle(*handle) != NULL)
          {
            deleteKeepAliveElementByHandle(*handle);
            status = sendKeepAliveStopRequestMessage(*handle, false);
          } else {
            status = ResponseError_t::INVALID_ARGUMENT;
          }
        } else {
          status = ResponseError_t::INVALID_ARGUMENT;
        }
        std::shared_ptr<StopKeepAliveRequestMessage> m = std::static_pointer_cast<StopKeepAliveRequestMessage>(e.msg);
        if( m != nullptr)
        {
          auto resp = std::make_shared<ResponseError_t>(status);
          m->sendResponse(e.msg, Message::Callback::Status::SUCCESS, resp);
        }
        else
        {
          Log::getInstance().d("StopKeepAliveRequestMessage Failed");
        }
        break;
      }

      case KeepAliveInd:
      {
        Log::getInstance().d("processEvent(): handling KeepAliveInd");
        keep_alive_ind *ka_ind = static_cast<keep_alive_ind *>(e.data);
        KeepaliveStatusCode code = KeepaliveStatusCode::INACTIVE;
        if (ka_ind->status == KeepAliveResult_t::KEEP_ALIVE_RESULT_OK) {
          code = KeepaliveStatusCode::ACTIVE;
          Log::getInstance().d("Keepalive active for handle " + std::to_string(ka_ind->handle));
          ma_ka_list_elem_t *e = getKeepAliveElementByHandle(ka_ind->handle);
          if (e) {
            e->status = KeepaliveStatus_t::RUNNING;
          }
        } else {
          deleteKeepAliveElementByHandle(ka_ind->handle);
        }
        sendKeepAliveUnsol(ka_ind->handle, code);
      break;
      }

      case KeepAliveCleanCallState:
      {
        Log::getInstance().d("processEvent(): handling KeepAliveCleanCallState");
        int *cid =  static_cast<int *>(e.data);
        if(*cid >= 0) {
          deleteKeepAliveElementByCid(*cid);
        } else {
          while (ma_ka_list_head != NULL) {
            ma_ka_list_elem_t *next = ma_ka_list_head->next;
            Log::getInstance().d("Cleanup for handle "+ std::to_string(ma_ka_list_head->handle));
            sendKeepAliveUnsol(ma_ka_list_head->handle, KeepaliveStatusCode::INACTIVE);
            free(ma_ka_list_head);
            ma_ka_list_head = next;
          }
        }
      break;
    }
  }
}

/*===========================================================================
  FUNCTION  sendKeepAliveStartRequestMessage
===========================================================================*/
/*!
@brief
  Send request to WDS for starting keepalive timer on modem

@return
  int - E_SUCCESS on successful operation,
        E_FAILURE if there was an error sending QMI message

*/
ResponseError_t KeepAliveHandler::sendKeepAliveStartRequestMessage( keep_alive_start_request *ka_req, uint32_t *handle)
{
  Log::getInstance().d("sendKeepAliveStartRequestMessage() : ENTRY");
  wds_modem_assisted_ka_start_req_msg_v01 qmi_req;
  memset( &qmi_req, 0x0, sizeof(wds_modem_assisted_ka_start_req_msg_v01) );
  ResponseError_t rc = ResponseError_t::INTERNAL_ERROR;

  if(DetectKeepAliveDuplicateReq(ka_req->ril_ka_req))
  {
    Log::getInstance().d("duplicate request is detected");
    rc = ResponseError_t::INVALID_ARGUMENT;
    return rc;
  }

  generateStartKeepAliveRequest(ka_req->ril_ka_req , &qmi_req, ka_req->apn);

  wds_modem_assisted_ka_start_resp_msg_v01 resp;
  memset( &resp, 0x0, sizeof(wds_modem_assisted_ka_start_resp_msg_v01) );

  auto retVal = wds_endpoint->sendRawSync(QMI_WDS_MODEM_ASSISTED_KA_START_REQ_V01,
                           (void *)&qmi_req, sizeof(qmi_req),
                           (void *)&resp, sizeof(resp),
                           QCRIL_DATA_QMI_TIMEOUT);

  if ( retVal == QMI_NO_ERR && resp.resp.result == QMI_RESULT_SUCCESS_V01)
  {
    if (resp.keep_alive_handle_valid) {
      Log::getInstance().d("Keep Alive handle is valid");
     *handle = resp.keep_alive_handle;
      rc = ResponseError_t::NO_ERROR;
    }
  }
  else
    Log::getInstance().d("sendKeepAliveStartRequestMessage():send stop for handle failed");

  return rc;
}

/*=========================================================================

  FUNCTION:  generateStartKeepAliveRequest

===========================================================================*/
void KeepAliveHandler::generateStartKeepAliveRequest(const KeepaliveRequest_t* in, wds_modem_assisted_ka_start_req_msg_v01* out, string apnName)
{
  Log::getInstance().d("generateStartKeepAliveRequest() : ENTRY");
  out->keep_alive_type = WDS_KEEPALIVE_TYPE_NAT_V01;
  if (in->type == KeepaliveType_t::NATT_IPV4) {
    Log::getInstance().d("Received Keep alive request with NATT_IPV4 type");
    // data is in network byte order (big-endian), convert to host order (little-endian)
    uint32_t src = 0;
    uint32_t dest = 0;
    memcpy(&src, in->sourceAddress.data(), sizeof(uint32_t));
    memcpy(&dest, in->destinationAddress.data(), sizeof(uint32_t));
    out->source_ipv4_address = ntohl(src);
    out->dest_ipv4_address = ntohl(dest);
    out->dest_ipv4_address_valid = 1;
    out->source_ipv4_address_valid = 1;
  } else {
    // no conversion needed for IPv6
    memcpy(&out->source_ipv6_address, in->sourceAddress.data(), QMI_WDS_IPV6_ADDR_LEN_V01*sizeof(uint8_t));
    memcpy(&out->dest_ipv6_address, in->destinationAddress.data(), QMI_WDS_IPV6_ADDR_LEN_V01*sizeof(uint8_t));
    out->dest_ipv6_address_valid = 1;
    out->source_ipv6_address_valid = 1;
  }
  if (in->maxKeepaliveIntervalMillis > 0) {
    out->timer_value = in->maxKeepaliveIntervalMillis;
    out->timer_value_valid = 1;
  }
  out->dest_port = (uint16_t) in->destinationPort;
  out->dest_port_valid = 1;
  out->source_port = (uint16_t) in->sourcePort;
  out->source_port_valid = 1;
  strlcpy(out->apn_name, apnName.c_str(), QMI_WDS_APN_NAME_MAX_V01);
  out->apn_name_valid = 1;
}

void KeepAliveHandler::copyKeepAliveElement(ma_ka_list_elem_t *ele, KeepaliveRequest_t* req)
{
  Log::getInstance().d("copyKeepAliveElement() : ENTRY");
  ele->type = req->type;
  ele->sourcePort = req->sourcePort;
  ele->sourceAddress = req->sourceAddress;
  ele->destinationPort = ele->destinationPort;
  ele->destinationAddress = req->destinationAddress;
  ele->cid = req->cid;
  ele->status = KeepaliveStatus_t::REQUESTED;
}

/*=========================================================================

  FUNCTION:  sendKeepAliveStopRequestMessage

===========================================================================*/
ResponseError_t KeepAliveHandler::sendKeepAliveStopRequestMessage( uint32_t handle, bool bSendUnsol )
{
  Log::getInstance().d("sendKeepAliveStopRequestMessage() : ENTRY");
  wds_modem_assisted_ka_stop_req_msg_v01 qmi_req;
  wds_modem_assisted_ka_stop_resp_msg_v01 resp;
  memset( &qmi_req, 0x0, sizeof(wds_modem_assisted_ka_stop_req_msg_v01) );
  memset( &resp, 0x0, sizeof(wds_modem_assisted_ka_stop_resp_msg_v01) );
  qmi_req.keep_alive_handle = handle;

  ResponseError_t rc = ResponseError_t::INVALID_ARGUMENT;

  auto retVal = wds_endpoint->sendRawSync(QMI_WDS_MODEM_ASSISTED_KA_STOP_REQ_V01,
                           (void *)&qmi_req, sizeof(qmi_req),
                           (void *)&resp, sizeof(resp),
                           QCRIL_DATA_QMI_TIMEOUT);

  if ( retVal == QMI_NO_ERR && resp.resp.result == QMI_RESULT_SUCCESS_V01)
  {
    rc = ResponseError_t::NO_ERROR;
  } else {
    Log::getInstance().d("sendKeepAliveStopRequestMessage():send stop failed");
  }

  if(bSendUnsol)
  {
    Log::getInstance().d("Sending KeepAlive Unsol Messages");
    sendKeepAliveUnsol(handle, KeepaliveStatusCode::INACTIVE);
  }
  return rc;
}

/*===========================================================================

  FUNCTION:  sendKeepAliveUnsol

===========================================================================*/
void KeepAliveHandler::sendKeepAliveUnsol(uint32_t handle, KeepaliveStatusCode status_code)
{
  Log::getInstance().d("Sending keepalive unsol for handle = "+ std::to_string(handle) +", status = " + std::to_string(status_code));
  auto radioMsg = std::make_shared<rildata::RadioKeepAliveStatusIndMessage>(status_code, handle);
  radioMsg->broadcast();
}

/*===========================================================================

  FUNCTION:  getKeepAliveElementByHandle

===========================================================================*/
/*!
    @brief
    Get ptr to an element if it exists in local cache

    @return
    qcril_data_ma_ka_list_elem_t* or NULL
*/
/*=========================================================================*/
ma_ka_list_elem_t * KeepAliveHandler::getKeepAliveElementByHandle(uint32_t handle)
{
  Log::getInstance().d("getKeepAliveElementByHandle() : ENTRY");
  ma_ka_list_elem_t *cur = ma_ka_list_head;
  while (cur != NULL) {
    if (cur->handle == handle)
    {
        return cur;
    }
    cur = cur->next;
  }
  Log::getInstance().d("Failed to get the element");
  return NULL;
}

/*===========================================================================

  FUNCTION:  createKeepAliveElement

===========================================================================*/
/*!
    @brief
    Give token and handle, create a new element

    @return
    uint64 mask
*/
/*=========================================================================*/
ma_ka_list_elem_t* KeepAliveHandler::createKeepAliveElement(uint32_t handle)
{
  Log::getInstance().d("createKeepAliveElement : ENTRY");
  ma_ka_list_elem_t* elem = NULL;
  elem = (ma_ka_list_elem_t*) calloc(sizeof(ma_ka_list_elem_t), 1);
  if (elem != NULL) {
      elem->next = NULL;
      elem->status = KeepaliveStatus_t::NONE;
      elem->handle = handle;
  } else {
    Log::getInstance().d("KeepAliveHandler: failed to allocate!");
  }
  return elem;
}

/*===========================================================================

  FUNCTION:  insertKeepAliveElement

===========================================================================*/
/*!
    @brief
    insert given elem at head

    @return
    void
*/
/*=========================================================================*/
void KeepAliveHandler::insertKeepAliveElement(ma_ka_list_elem_t* elem)
{
  if (elem == NULL) {
    Log::getInstance().d("insertKeepAliveElement: cannot insert null element");
    return;
  }
  elem->next = ma_ka_list_head;
  ma_ka_list_head = elem;
  Log::getInstance().d("insertKeepAliveElement: inserted handle " + std::to_string(elem->handle));
  return;
}


/*===========================================================================

  FUNCTION:  deleteKeepAliveElementByHandle

===========================================================================*/
/*!
    @brief
    Given token, remove from cache if found

    @return
    void
*/
/*=========================================================================*/
void KeepAliveHandler::deleteKeepAliveElementByHandle(uint32_t handle)
{
   Log::getInstance().d("deleteKeepAliveElementByHandle() : ENTRY");
  // Handle empty list
  if (ma_ka_list_head == NULL) {
    Log::getInstance().d("deleteKeepAliveElementByHandle: List is empty, cannot delete " + std::to_string(handle));
    return;
  }

  // If only one element in list and it's the one
  // we want then purge list
  if ((ma_ka_list_head->next == NULL) &&
      (ma_ka_list_head->handle == handle)) {
    free(ma_ka_list_head);
    ma_ka_list_head = NULL;
    Log::getInstance().d("Removed "+ std::to_string(handle) +", List is now empty");
  }
  // Deleting first element of a list with two or more
  // elements
  else if ((ma_ka_list_head->next != NULL) &&
      (ma_ka_list_head->handle == handle)) {
    ma_ka_list_elem_t * tmp = ma_ka_list_head;
    ma_ka_list_head = ma_ka_list_head->next;
    free(tmp);
  }
  else
  {
    // Deleting from the middle of a list or the end of the list
    ma_ka_list_elem_t *e = ma_ka_list_head;
    ma_ka_list_elem_t *next = e->next;
    while (next != NULL) {
      if (next->handle == handle)
      {
        ma_ka_list_elem_t *tmp = next;
        e->next = next->next;
        Log::getInstance().d("Cleaned up handle "+std::to_string(handle));
        free(tmp);
        break;
      } else
      {
        e = next;
        next = next->next;
      }
    }
  }
}

/*===========================================================================

  FUNCTION:  deleteKeepAliveElementByCid

===========================================================================*/
/*!
    @brief
    Given cid, remove from cache if found

    @return
    void
*/
/*=========================================================================*/
void KeepAliveHandler::deleteKeepAliveElementByCid(int cid)
{
  if (cid < 0) {
    Log::getInstance().d("Call ID cannot be negative");
    return;
  }

  // Handle empty list
  if (ma_ka_list_head == NULL) {
    Log::getInstance().d("List is empty");
    return;
  }

  // If only one element in list and it's the one
  // we want then purge list
  if ((ma_ka_list_head->next == NULL) &&
      (ma_ka_list_head->cid == cid)) {

    Log::getInstance().d("sending sendKeepAliveStopRequestMessage");
    sendKeepAliveStopRequestMessage(ma_ka_list_head->handle, true);

    Log::getInstance().d("Removing "+ std::to_string(ma_ka_list_head->handle) +", List is now empty");
    free(ma_ka_list_head);
    ma_ka_list_head = NULL;

  }

  ma_ka_list_elem_t * curr = ma_ka_list_head;
  ma_ka_list_elem_t * prev = NULL;

  while (curr != NULL) {
    ma_ka_list_elem_t *next = curr->next;

    // Deleting first element of list
    if ((curr == ma_ka_list_head) &&
        (curr->cid == cid)) {
      // update head to next element
      ma_ka_list_head = ma_ka_list_head->next;

      sendKeepAliveStopRequestMessage(curr->handle, true);

      // prev is not affected while we are deleting the first node
      Log::getInstance().d("Deleting first element, handle = "+ std::to_string(curr->handle) +" for cid = " + std::to_string(curr->cid));
      free(curr);
    // Deleting from middle or end of list
    } else if ((curr != ma_ka_list_head) &&
      (curr->cid == cid)) {

      sendKeepAliveStopRequestMessage(curr->handle, true);

      Log::getInstance().d("Deleting from middle, handle = "+ std::to_string(curr->handle) +"for cid = "+ std::to_string(curr->cid));
      // link previous and next
      if(prev != nullptr)
        prev->next = next;

      // no update to prev here since we delete curr
      free(curr);

    } else if (curr->cid != cid) {
      // Advance prev since curr is not deleted
      prev = curr;
      Log::getInstance().d("Mo match for handle = "+ std::to_string(curr->handle) +" for cid = "+ std::to_string(curr->cid));
    }

    // IMPORTANT
    // ---------
    // we always ensure that curr is advanced regardless of
    // whether we deleted a node or not
    curr = next;
  }
}

/*===========================================================================

  FUNCTION:  DetectKeepAliveDuplicateReq

===========================================================================*/
/*!
    @brief
    Check if an incoming request is a duplicate

    @return
    boolean
*/
/*=========================================================================*/
boolean KeepAliveHandler::DetectKeepAliveDuplicateReq(const KeepaliveRequest_t* req)
{
  /*
  if the src addr/port and dest addr/port + the NAT type matches
  then we treat it as a duplicate request.
  */
  Log::getInstance().d("Checking for duplicate request");
  ma_ka_list_elem_t *cur = ma_ka_list_head;

  while (cur != NULL) {
    if (matchAddress(req->destinationAddress, cur->destinationAddress) &&
      matchPort(req->destinationPort, cur->destinationPort) &&
      matchAddress(req->sourceAddress, cur->sourceAddress) &&
      matchPort(req->sourcePort, cur->sourcePort) &&
      matchKeepAliveType(req->type, cur->type) &&
      req->cid == cur->cid)
    {
        Log::getInstance().d("matched for handle "+ std::to_string(cur->handle));
        return TRUE;
    }
    cur = cur->next;
  }
  return FALSE;
}

/*===========================================================================

  FUNCTION:  matchAddress

===========================================================================*/
/*!
    @brief
    Helper function to match addresses

    @return
    void
*/
/*=========================================================================*/
boolean KeepAliveHandler::matchAddress(const vector<uint8_t>& a, const vector<uint8_t>& b)
{
  Log::getInstance().d("matchAddress() : ENTRY");
  return (a==b)? TRUE:FALSE;
}

/*===========================================================================

  FUNCTION:  matchPort

===========================================================================*/
/*!
    @brief
    Helper function to match ports

    @return
    void
*/
/*=========================================================================*/
boolean KeepAliveHandler::matchPort(int a, int b)
{
  Log::getInstance().d("matchPort() : ENTRY");
  return (a == b)? TRUE:FALSE;
}

/*===========================================================================

  FUNCTION:  matchKeepAliveType

===========================================================================*/
/*!
    @brief
    Helper function to match types

    @return
    void
*/
/*=========================================================================*/
boolean KeepAliveHandler::matchKeepAliveType(KeepaliveType_t a, KeepaliveType_t b)
{
  Log::getInstance().d("matchKeepAliveType() : ENTRY");
  return (a == b)? TRUE:FALSE;
}





