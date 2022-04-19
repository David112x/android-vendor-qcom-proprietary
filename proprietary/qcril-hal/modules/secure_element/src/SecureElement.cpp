/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#ifndef QMI_RIL_UTF
#include <framework/ModuleLooper.h>
#include <framework/Log.h>
#include <vector>
#include <mutex>
#include <unistd.h>

#include <android/hardware/secure_element/1.0/ISecureElement.h>
#include <cutils/properties.h>
#include <hidl/HidlTransportSupport.h>
#include <inttypes.h>

#include "modules/qmi/ModemEndPointFactory.h"
#include "modules/uim/UimModemEndPoint.h"
#include "modules/uim/qcril_uim_srvc.h"
#include "modules/uim/UimQmiUimRequestMsg.h"
#include "modules/secure_element/SecureElementOnStateChangeIndMsg.h"

#include "SecureElement.h"

#define TAG "SecureElement_Module"
#define QCRIL_UIM_INS_BYTE_SELECT                   0xA4
#define QCRIL_UIM_P1_VALUE_SELECT_BY_DF_NAME        0x04
#define QCRIL_UIM_P2_MASK_SELECT_NEXT               0x02

#define UICC_SE_HAL_PROP "persist.vendor.radio.uicc_se_enabled"

using namespace android::hardware::secure_element;
using namespace android::hardware::secure_element::V1_0;
using ::android::hardware::Return;
using ::android::hardware::hidl_vec;
using ::android::hardware::Void;
using ::android::hardware::hidl_death_recipient;
using android::sp;
using android::hardware::Status;

typedef struct
{
  boolean                             in_use;
  uint16                              total_apdu_len;
  uint32                              token;
  uint16                              rx_len;
  uint8                             * apdu_ptr;
} secure_element_long_apdu_info_type;

typedef struct
{
  qmi_uim_indication_id_type       ind_id;
  qmi_uim_indication_data_type     ind_data;
  uint8                            payload;
} secure_element_indication_params_type;

static std::condition_variable_any         cond;
static qtimutex::QtiRecursiveMutex         mutx;
static secure_element_long_apdu_info_type  long_apdu_info = {};
static boolean                             is_send_apdu_in_progress = FALSE;

struct SecureElementImpl : public ISecureElement, public hidl_death_recipient
{
  sp<ISecureElementHalCallback> mClientCb;
  qtimutex::QtiSharedMutex mCallbackLock;
  qtimutex::QtiSharedMutex ch_id_list_lock;
  std::vector<uint8_t> ch_id_list;

  sp<ISecureElementHalCallback> getISecureElementHalCallback();
  void clearCallbacks();

  void serviceDied( uint64_t, const ::android::wp<::android::hidl::base::V1_0::IBase> &);

  Return<void> init(const sp<ISecureElementHalCallback>& clientCallback);
  Return<void> getAtr(getAtr_cb _hidl_cb);
  Return<bool> isCardPresent();
  Return<void> transmit(const hidl_vec<uint8_t>& data, transmit_cb _hidl_cb);
  Return<void> openLogicalChannel(const hidl_vec<uint8_t>& aid, uint8_t p2,
                                             openLogicalChannel_cb _hidl_cb);
  Return<void> openBasicChannel(const hidl_vec<uint8_t>& aid, uint8_t p2,
                                               openBasicChannel_cb _hidl_cb);
  Return<SecureElementStatus> closeChannel(uint8_t channelNumber);

  SecureElementStatus openChannelRequest(const hidl_vec<uint8_t>& aid, uint8_t p2,
                                        qmi_uim_logical_channel_rsp_type *rsp_data_ptr);
  SecureElementStatus convertQmiErrSecureElementStatus(int qmi_err_code,
                                  qmi_uim_logical_channel_rsp_type logical_channel_rsp);
};


static load_module<SecureElementModule> secure_element_module;
static sp<SecureElementImpl> secureElementService = NULL;

/*=========================================================================

  FUNCTION:  secure_element_send_qmi_sync_msg

===========================================================================*/
int secure_element_send_qmi_sync_msg
(
  uint32                        msg_id,
  const void                  * params,
  qmi_uim_rsp_data_type       * rsp_data_ptr
)
{
  int  ret         = -1;
  auto req_msg_ptr = std::make_shared<UimQmiUimRequestMsg>(msg_id,
                                                           params,
                                                           (void *)rsp_data_ptr,
                                                           nullptr);

  if (req_msg_ptr != nullptr)
  {
    std::shared_ptr<int>  respPtr        = nullptr;

    if ((req_msg_ptr->dispatchSync(respPtr) == Message::Callback::Status::SUCCESS) &&
        (respPtr != nullptr))
    {
      ret = *respPtr;
    }
  }

  return ret;
} /* secure_element_send_qmi_sync_msg */


/*===========================================================================

FUNCTION:  secureElementClearChannelIdList

===========================================================================*/
static void secureElementClearChannelIdList(std::vector<uint8_t> &ch_id_list)
{
  for (auto ch_id:ch_id_list)
  {
    /* Send close channel, modem might re-open if there is no
       transmit APDU on that channel in case of recovery */
    qmi_uim_logical_channel_params_type  close_ch       = {};
    qmi_uim_rsp_data_type                rsp_data       = {};

    if (ch_id == 0)
    {
      continue;
    }

    close_ch.slot = (qmi_uim_slot_type)(qmi_ril_get_process_instance_id() + QMI_UIM_SLOT_1);
    close_ch.operation_type = QMI_UIM_LOGICAL_CHANNEL_CLOSE;
    close_ch.channel_data.close_channel_info.channel_id = ch_id;

    (void) secure_element_send_qmi_sync_msg(QCRIL_UIM_REQUEST_LOGICAL_CHANNEL,
                                            &close_ch,
                                            &rsp_data);
  }
  ch_id_list.clear();
} /* secureElementClearChannelIdList */


/*===========================================================================

FUNCTION:  handleSecureElementModuleReady

===========================================================================*/
static void handleSecureElementModuleReady
(
  void
)
{
  int                                      qmi_err_code     = QMI_UIM_SERVICE_ERR_NONE;
  qmi_uim_rsp_data_type                    rsp_data         = {};
  sp<ISecureElementHalCallback>            callback         =
      (secureElementService == nullptr) ? nullptr
                                        : secureElementService->getISecureElementHalCallback();

  QCRIL_LOG_INFO("=DBG= handleSecureElementModuleReady called");

  qmi_err_code = secure_element_send_qmi_sync_msg(QCRIL_UIM_REQUEST_GET_CARD_STATUS,
                                                  nullptr,
                                                  &rsp_data);

  if (qmi_err_code == QMI_UIM_SERVICE_ERR_NONE &&
      rsp_data.qmi_err_code == QMI_UIM_SERVICE_ERR_NONE)
  {
    uint8_t   slot = (uint8_t) qmi_ril_get_process_instance_id();

    if (slot < QMI_UIM_MAX_CARD_COUNT &&
        rsp_data.rsp_data.get_card_status_rsp.card_status_validity.card_status_invalid[slot] ==
                                                                                QMI_UIM_FALSE &&
        rsp_data.rsp_data.get_card_status_rsp.card_status.card[slot].card_state ==
                                                                     QMI_UIM_CARD_STATE_PRESENT)
    {
      secure_element_module.get_module().card_accessible = true;

      /* Delay the card insert time for 3 secs to delay non prov app activation requests */
      sleep(3);

      if (callback != nullptr)
      {
        Return<void> ret = callback->onStateChange(true);

        QCRIL_LOG_INFO("=DBG= onStateChange informed to clients");

        if (!ret.isOk())
        {
           QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
        }
      }
    }
  }
} /* handleSecureElementModuleReady */


/*===========================================================================

FUNCTION:  cleanUpLongApduInfo

===========================================================================*/
static void cleanUpLongApduInfo
(
  void
)
{
  is_send_apdu_in_progress = FALSE;
  if (long_apdu_info.apdu_ptr != NULL)
  {
    qcril_free(long_apdu_info.apdu_ptr);
    long_apdu_info.apdu_ptr = NULL;
  }
  memset(&long_apdu_info, 0x00, sizeof(long_apdu_info));
} /* cleanUpLongApduInfo */


/*===========================================================================

FUNCTION:  SecureElementImpl::convertQmiErrSecureElementStatus

===========================================================================*/
SecureElementStatus SecureElementImpl::convertQmiErrSecureElementStatus
(
  int                              qmi_err_code,
  qmi_uim_logical_channel_rsp_type logical_channel_rsp
)
{
  if (logical_channel_rsp.sw1_sw2_valid == QMI_UIM_FALSE)
  {
    return SecureElementStatus::IOERROR;
  }

  switch (qmi_err_code)
  {
    case QMI_UIM_SERVICE_ERR_INSUFFICIENT_RESOURCES:
      return SecureElementStatus::CHANNEL_NOT_AVAILABLE;

    case QMI_UIM_SERVICE_ERR_INCOMPATIBLE_STATE:
    case QMI_UIM_SERVICE_ERR_SIM_FILE_NOT_FOUND:
      return SecureElementStatus::NO_SUCH_ELEMENT_ERROR;

    case QMI_UIM_SERVICE_ERR_NONE:
      return SecureElementStatus::SUCCESS;

    default:
      return SecureElementStatus::FAILED;
  }
} /* SecureElementImpl::convertQmiErrSecureElementStatus */


/*===========================================================================

FUNCTION:  SecureElementImpl::openChannelRequest

===========================================================================*/
SecureElementStatus SecureElementImpl::openChannelRequest
(
  const hidl_vec<uint8_t>                   & aid,
  uint8_t                                     p2,
  qmi_uim_logical_channel_rsp_type          * rsp_data_ptr
)
{
  qmi_uim_open_logical_channel_params_type  open_ch = {};
  qmi_uim_rsp_data_type                     rsp_data       = {};
  int                                       qmi_err_code   = 0;
  SecureElementStatus                       status         = {};

  QCRIL_LOG_INFO("SecureElementImpl::openChannelRequest");

  if (rsp_data_ptr == NULL)
  {
    return SecureElementStatus::FAILED;
  }

  open_ch.slot = (qmi_uim_slot_type)(qmi_ril_get_process_instance_id() + QMI_UIM_SLOT_1);

  QCRIL_LOG_INFO("%d", aid.size());

  if (aid.size() > 0)
  {
    open_ch.aid_present = QMI_UIM_TRUE;
    open_ch.aid.data_ptr = new uint8_t[aid.size()];

    if (open_ch.aid.data_ptr == NULL)
    {
      return SecureElementStatus::FAILED;
    }

    memcpy(open_ch.aid.data_ptr, aid.data(), aid.size());
    open_ch.aid.data_len = aid.size();
  }

  open_ch.file_control_information.is_valid = QMI_UIM_TRUE;

  switch (p2)
  {
    case 0x00:
      open_ch.file_control_information.fci_value = QMI_UIM_FCI_VALUE_FCI;
      break;
    case 0x04:
      open_ch.file_control_information.fci_value = QMI_UIM_FCI_VALUE_FCP;
      break;
    case 0x08:
      open_ch.file_control_information.fci_value = QMI_UIM_FCI_VALUE_FMD;
      break;
    case 0x0C:
      open_ch.file_control_information.fci_value = QMI_UIM_FCI_VALUE_NO_DATA;
      break;
    default:
      open_ch.file_control_information.is_valid = QMI_UIM_FALSE;
      QCRIL_LOG_ERROR("Invalid P2 value: 0x%x", p2);
      return SecureElementStatus::UNSUPPORTED_OPERATION;
  }

  qmi_err_code = secure_element_send_qmi_sync_msg(QCRIL_UIM_REQUEST_OPEN_LOGICAL_CHANNEL,
                                                  &open_ch,
                                                  &rsp_data);

  delete[] open_ch.aid.data_ptr;

  if (qmi_err_code != QMI_UIM_SERVICE_ERR_NONE)
  {
    QCRIL_LOG_INFO("openChannelRequest: Error %d", rsp_data.qmi_err_code);

    status = SecureElementStatus::FAILED;
  }
  else
  {
    status = convertQmiErrSecureElementStatus(rsp_data.qmi_err_code, rsp_data.rsp_data.logical_channel_rsp);
  }

  if (status != SecureElementStatus::SUCCESS)
  {
    if (rsp_data.rsp_data.logical_channel_rsp.select_response.data_ptr != NULL)
    {
      qcril_free(rsp_data.rsp_data.logical_channel_rsp.select_response.data_ptr);
    }
    return status;
  }

  memcpy(rsp_data_ptr,
         &rsp_data.rsp_data.logical_channel_rsp,
         sizeof(qmi_uim_logical_channel_rsp_type));

  rsp_data_ptr->select_response.data_ptr =
            new uint8_t[rsp_data.rsp_data.logical_channel_rsp.select_response.data_len + 2];

  rsp_data_ptr->select_response.data_len =
                         rsp_data.rsp_data.logical_channel_rsp.select_response.data_len + 2;

  if (rsp_data_ptr->select_response.data_ptr != NULL)
  {
    if (rsp_data.rsp_data.logical_channel_rsp.select_response.data_ptr != NULL)
    {
      memcpy(rsp_data_ptr->select_response.data_ptr,
             rsp_data.rsp_data.logical_channel_rsp.select_response.data_ptr,
             rsp_data.rsp_data.logical_channel_rsp.select_response.data_len);
      qcril_free(rsp_data.rsp_data.logical_channel_rsp.select_response.data_ptr);
    }
    rsp_data_ptr->select_response.data_ptr[rsp_data.rsp_data.logical_channel_rsp.select_response.data_len] =
                                                                  rsp_data.rsp_data.logical_channel_rsp.sw1;
    rsp_data_ptr->select_response.data_ptr[rsp_data.rsp_data.logical_channel_rsp.select_response.data_len + 1] =
                                                                  rsp_data.rsp_data.logical_channel_rsp.sw2;
  }
  else
  {
    rsp_data_ptr->select_response.data_len = 0;
  }

  return status;
} /* SecureElementImpl::openChannelRequest */


/*===========================================================================

FUNCTION:  SecureElementImpl::init

===========================================================================*/
Return<void> SecureElementImpl::init
(
  const sp<ISecureElementHalCallback>& clientCallback
)
{
  QCRIL_LOG_INFO("SecureElementImpl::init");

  {
    std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
    if (mClientCb != nullptr)
    {
      mClientCb->unlinkToDeath(this);
    }
    mClientCb = clientCallback;

    if (mClientCb != nullptr)
    {
      mClientCb->linkToDeath(this, 0);
    }

    secure_element_module.get_module().mServiceUp = true;

    if (secure_element_module.get_module().mQmiUimUp)
    {
      secure_element_module.get_module().mReady = true;
    }
  }

  if (secure_element_module.get_module().mReady == true)
  {
    std::shared_ptr<SecureElementOnStateChangeIndMsg>  seOnStateChangePtr = nullptr;

    /* Send ONStateChange Indication to SE clients */
    seOnStateChangePtr = std::make_shared<SecureElementOnStateChangeIndMsg>();

    if (seOnStateChangePtr)
    {
      seOnStateChangePtr->broadcast();
    }
  }

  return Void();
} /* SecureElementImpl::init */

sp<ISecureElementHalCallback> SecureElementImpl::getISecureElementHalCallback() {
  std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mClientCb;
}

/*===========================================================================

FUNCTION:  SecureElementImpl::isCardPresent

===========================================================================*/
Return<bool> SecureElementImpl::isCardPresent
(
  void
)
{
  return secure_element_module.get_module().card_accessible;
} /* SecureElementImpl::isCardPresent */


/*===========================================================================

FUNCTION:  SecureElementImpl::getAtr

===========================================================================*/
Return<void> SecureElementImpl::getAtr
(
  getAtr_cb _hidl_cb
)
{
  qmi_uim_get_atr_params_type       get_atr_params = {};
  qmi_uim_rsp_data_type             rsp_data       = {};
  int                               qmi_err_code   = 0;
  hidl_vec<uint8_t>                 atr;

  QCRIL_LOG_INFO("SecureElementImpl::getAtr");

  if (_hidl_cb == NULL)
  {
    return Void();
  }

  get_atr_params.slot = (qmi_uim_slot_type)(qmi_ril_get_process_instance_id() + QMI_UIM_SLOT_1);

  /* get atr */
  qmi_err_code = secure_element_send_qmi_sync_msg(QCRIL_UIM_REQUEST_GET_ATR,
                                                  &get_atr_params,
                                                  &rsp_data);

  atr.setToExternal(rsp_data.rsp_data.get_atr_rsp.atr_response.data_ptr,
                    rsp_data.rsp_data.get_atr_rsp.atr_response.data_len);

  _hidl_cb(atr);

  if (rsp_data.rsp_data.get_atr_rsp.atr_response.data_ptr != NULL)
  {
    qcril_free(rsp_data.rsp_data.get_atr_rsp.atr_response.data_ptr);
    rsp_data.rsp_data.get_atr_rsp.atr_response.data_ptr = NULL;
  }

  return Void();
} /* SecureElementImpl::getAtr */


/*===========================================================================

FUNCTION:  SecureElementImpl::transmit

===========================================================================*/
Return<void> SecureElementImpl::transmit
(
  const hidl_vec<uint8_t>& data,
  transmit_cb              _hidl_cb
)
{
  hidl_vec<uint8_t>               rsp             = {};
  qmi_uim_rsp_data_type           rsp_data       = {};
  int                             qmi_err_code   = 0;
  bool                            not_select_next = true;
  uint8_t                       * select_rsp      = NULL;

  QCRIL_LOG_INFO("SecureElementImpl::transmit");

  if (_hidl_cb == NULL)
  {
    return Void();
  }

  if (data.size() >= 4 &&
      (data.data()[1]         == QCRIL_UIM_INS_BYTE_SELECT)            &&
      (data.data()[2]          == QCRIL_UIM_P1_VALUE_SELECT_BY_DF_NAME) &&
      ((data.data()[3] & 0x03) == QCRIL_UIM_P2_MASK_SELECT_NEXT))
  {
    qmi_uim_reselect_params_type          reselect_params{};

    reselect_params.slot =
      (qmi_uim_slot_type)(qmi_ril_get_process_instance_id() + QMI_UIM_SLOT_1);
    reselect_params.channel_id = data.data()[0];
    reselect_params.select_mode = QMI_UIM_SELECT_MODE_NEXT;

    qmi_err_code = secure_element_send_qmi_sync_msg(QCRIL_UIM_REQUEST_RESELECT,
                                                    &reselect_params,
                                                    &rsp_data);

    if (rsp_data.qmi_err_code != QMI_UIM_SERVICE_ERR_INVALID_QMI_CMD)
    {
      select_rsp =
        new uint8_t[rsp_data.rsp_data.reselect_rsp.select_response.data_len + 2];

      not_select_next = false;

      if (select_rsp != NULL)
      {
        if (rsp_data.qmi_err_code == QMI_UIM_SERVICE_ERR_NOT_SUPPORTED)
        {
          select_rsp[0] = 0X6F;
          select_rsp[1] = 0x00;
          rsp.setToExternal(select_rsp, 2);
        }
        else
        {
          if (rsp_data.rsp_data.reselect_rsp.sw1_sw2_valid == QMI_UIM_FALSE ||
              (rsp_data.rsp_data.reselect_rsp.sw1 == 0 &&
               rsp_data.rsp_data.reselect_rsp.sw2 == 0))
          {
            rsp.setToExternal(NULL, 0);
          }
          else
          {
            if (rsp_data.rsp_data.reselect_rsp.select_response.data_ptr != NULL)
            {
              memcpy(select_rsp,
                     rsp_data.rsp_data.reselect_rsp.select_response.data_ptr,
                     rsp_data.rsp_data.reselect_rsp.select_response.data_len);
              qcril_free(rsp_data.rsp_data.reselect_rsp.select_response.data_ptr);
            }
            select_rsp[rsp_data.rsp_data.reselect_rsp.select_response.data_len] =
              rsp_data.rsp_data.reselect_rsp.sw1;
            select_rsp[rsp_data.rsp_data.reselect_rsp.select_response.data_len + 1] =
              rsp_data.rsp_data.reselect_rsp.sw2;
            rsp.setToExternal(select_rsp,
                              rsp_data.rsp_data.reselect_rsp.select_response.data_len + 2);
          }
        }
      }
    }
  }

  if (not_select_next)
  {
    qmi_uim_send_apdu_params_type   apdu_params{};

    apdu_params.slot = (qmi_uim_slot_type)(qmi_ril_get_process_instance_id() + QMI_UIM_SLOT_1);
    apdu_params.apdu.data_ptr = new uint8_t[data.size()];

    if (apdu_params.apdu.data_ptr == NULL)
    {
      _hidl_cb(rsp);
      return Void();
    }

    memcpy(apdu_params.apdu.data_ptr, data.data(), data.size());
    apdu_params.apdu.data_len = data.size();

    is_send_apdu_in_progress = TRUE;
    qmi_err_code = secure_element_send_qmi_sync_msg(QCRIL_UIM_REQUEST_SEND_APDU,
                                                    &apdu_params,
                                                    &rsp_data);

    delete[] apdu_params.apdu.data_ptr;

    /* Special case for long APDUs - we send the response of the long APDU
       stream after we get all the chunks in the corresponding SEND_APDU_INDs */
    if (rsp_data.qmi_err_code == QMI_UIM_SERVICE_ERR_INSUFFICIENT_RESOURCES)
    {
      QCRIL_LOG_DEBUG("transmit: total_len=0x%x bytes, token=0x%x",
                       rsp_data.rsp_data.send_apdu_rsp.total_len,
                       rsp_data.rsp_data.send_apdu_rsp.token);

      mutx.lock();

      /* Store only if the Long APDU response TLV is valid. We also need to check
         and handle cases where the INDs might have come earlier than this response.
         Note that original_request_ptr will be freed when we get SEND_APDU_INDs */
      if (long_apdu_info.in_use == TRUE)
      {
        /* If Indication already came, we need to check incoming info */
        if ((long_apdu_info.token == rsp_data.rsp_data.send_apdu_rsp.token) &&
            (long_apdu_info.total_apdu_len == rsp_data.rsp_data.send_apdu_rsp.total_len))
        {
          /* If Indication already came & incoming info matches,
             nothing else to do wait for indication to complete*/
          goto apdu_ind_wait;
        }
        else
        {
          /* Error condition - mismatch in data, send error if there was any previous
             request & store the current response's token */
          QCRIL_LOG_ERROR("Mismatch with global data, token: 0x%x, total_apdu_len: 0x%x",
                          long_apdu_info.token,
                          long_apdu_info.total_apdu_len);
          /* Cleanup and proceed to store current resp info */
          cleanUpLongApduInfo();
        }
      }

      /* Store response info. We return after successfully storing since
         we expect subsequent INDs */
      if (rsp_data.rsp_data.send_apdu_rsp.total_len > 0)
      {
        QCRIL_LOG_INFO("Storing long_apdu_info");
        long_apdu_info.in_use = TRUE;
        long_apdu_info.token = rsp_data.rsp_data.send_apdu_rsp.token;
        long_apdu_info.total_apdu_len = rsp_data.rsp_data.send_apdu_rsp.total_len;
      }

apdu_ind_wait:
      /* Wait only when we have still some response data to be received */
      if (long_apdu_info.rx_len < long_apdu_info.total_apdu_len)
      {
        cond.wait(mutx);

        if (long_apdu_info.rx_len != long_apdu_info.total_apdu_len)
        {
          rsp.setToExternal(NULL, 0);
        }
        else
        {
          rsp.setToExternal(long_apdu_info.apdu_ptr,
                            long_apdu_info.total_apdu_len);
        }
      }
      else
      {
        rsp.setToExternal(long_apdu_info.apdu_ptr,
                          long_apdu_info.total_apdu_len);
      }
      cleanUpLongApduInfo();

      mutx.unlock();

      is_send_apdu_in_progress = FALSE;
      _hidl_cb(rsp);

      return Void();
    }

    rsp.setToExternal(rsp_data.rsp_data.send_apdu_rsp.apdu_response.data_ptr,
                      rsp_data.rsp_data.send_apdu_rsp.apdu_response.data_len);
  }

  is_send_apdu_in_progress = FALSE;
  _hidl_cb(rsp);

  if (not_select_next)
  {
    if (rsp_data.rsp_data.send_apdu_rsp.apdu_response.data_ptr != NULL)
    {
      qcril_free(rsp_data.rsp_data.send_apdu_rsp.apdu_response.data_ptr);
    }
  }
  if (select_rsp != NULL)
  {
    delete[] select_rsp;
  }

  return Void();
} /* SecureElementImpl::transmit */


/*===========================================================================

FUNCTION:  SecureElementImpl::openLogicalChannel

===========================================================================*/
Return<void> SecureElementImpl::openLogicalChannel
(
  const hidl_vec<uint8_t>& aid,
  uint8_t                  p2,
  openLogicalChannel_cb    _hidl_cb
)
{
  LogicalChannelResponse                    rsp            = {};
  qmi_uim_logical_channel_rsp_type          rsp_data       = {};
  SecureElementStatus                       status         = {};

  QCRIL_LOG_INFO("SecureElementImpl::openLogicalChannel p2: %d", p2);

  QCRIL_LOG_INFO("%d", aid.size());

  if (_hidl_cb == NULL)
  {
    return Void();
  }

  status = openChannelRequest(aid, p2, &rsp_data);

  QCRIL_LOG_INFO("SecureElementImpl::openLogicalChannel status : %d, channel_id : %d",
                   status, rsp_data.channel_id);

  if (status == SecureElementStatus::SUCCESS)
  {
    std::lock_guard<qtimutex::QtiSharedMutex> lock(ch_id_list_lock);

    rsp.channelNumber = rsp_data.channel_id;
    rsp.selectResponse.setToExternal(rsp_data.select_response.data_ptr, rsp_data.select_response.data_len);

    /* cache the channeld ID, client may die and service need to close the channels */
    ch_id_list.push_back(rsp_data.channel_id);
  }

  _hidl_cb(rsp, status);

  if (rsp_data.select_response.data_ptr != NULL)
  {
    delete[] rsp_data.select_response.data_ptr;
  }

  return Void();
} /* SecureElementImpl::openLogicalChannel */


/*===========================================================================

FUNCTION:  SecureElementImpl::openBasicChannel

===========================================================================*/
Return<void> SecureElementImpl::openBasicChannel
(
  const hidl_vec<uint8_t>& aid,
  uint8_t                  p2,
  openBasicChannel_cb      _hidl_cb
)
{
  hidl_vec<uint8_t>                         rsp            = {};

  (void) aid;

  QCRIL_LOG_INFO("SecureElementImpl::openBasicChannel : %d", p2);

  if (_hidl_cb == NULL)
  {
    return Void();
  }

 /* Basic channel is channel 0 for telecom app, accessing not support on UICC */
  _hidl_cb(rsp, SecureElementStatus::CHANNEL_NOT_AVAILABLE);
  return Void();
} /* SecureElementImpl::openBasicChannel */


/*===========================================================================

FUNCTION:  SecureElementImpl::closeChannel

===========================================================================*/
Return<SecureElementStatus> SecureElementImpl::closeChannel
(
  uint8_t channelNumber
)
{
  qmi_uim_rsp_data_type                rsp_data       = {};
  int                                  qmi_err_code   = 0;
  qmi_uim_logical_channel_params_type  close_ch       = {};

  QCRIL_LOG_INFO("SecureElementImpl::closeChannel");

  if (channelNumber == 0)
  {
    return SecureElementStatus::FAILED;
  }

  close_ch.slot = (qmi_uim_slot_type)(qmi_ril_get_process_instance_id() + QMI_UIM_SLOT_1);
  close_ch.operation_type = QMI_UIM_LOGICAL_CHANNEL_CLOSE;
  close_ch.channel_data.close_channel_info.channel_id = channelNumber;

  qmi_err_code = secure_element_send_qmi_sync_msg(QCRIL_UIM_REQUEST_LOGICAL_CHANNEL,
                                                  &close_ch,
                                                  &rsp_data);

  QCRIL_LOG_INFO("SecureElementImpl::closeChannel status : %d", qmi_err_code);

  if (qmi_err_code != QMI_UIM_SERVICE_ERR_NONE &&
      rsp_data.qmi_err_code == QMI_UIM_SERVICE_ERR_NONE)
  {
    return SecureElementStatus::FAILED;
  }
  else
  {
    std::lock_guard<qtimutex::QtiSharedMutex> lock(ch_id_list_lock);

    /* Remove channel id from the cached list */
    if (!ch_id_list.empty())
    {
      std::vector<uint8_t>::iterator it = ch_id_list.begin();
      while (it != ch_id_list.end())
      {
        if (*it == channelNumber)
        {
          ch_id_list.erase(it);
          break;
        }
        it++;
      }
    }

    return SecureElementStatus::SUCCESS;
  }
} /* SecureElementImpl::closeChannel */


/*===========================================================================

FUNCTION:  SecureElementImpl::serviceDied

===========================================================================*/
void SecureElementImpl::serviceDied
(
 uint64_t,
 const ::android::wp<::android::hidl::base::V1_0::IBase> &
)
{
  QCRIL_LOG_INFO("SecureElementImpl::serviceDied: Client died. Cleaning up callbacks");
  clearCallbacks();
} /* SecureElementImpl::serviceDied */


/*===========================================================================

FUNCTION:  SecureElementImpl::clearCallbacks

===========================================================================*/
void SecureElementImpl::clearCallbacks()
{
  QCRIL_LOG_INFO("SecureElementImpl::clearCallbacks");

  {
    std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
    if (mClientCb != nullptr)
    {
      mClientCb = NULL;
    }

    secure_element_module.get_module().mServiceUp = false;
    secure_element_module.get_module().mReady = false;
  }

  {
    std::lock_guard<qtimutex::QtiSharedMutex> lock(ch_id_list_lock);
    /* Clear cached channel ids if client dies */
    if (!ch_id_list.empty())
    {
      secureElementClearChannelIdList(ch_id_list);
    }
  }

  /* Long APDU transaction might be in progress send signal to send response */
  mutx.lock();
  if (long_apdu_info.in_use == TRUE)
  {
    cleanUpLongApduInfo();
    cond.notify_one();
  }
  mutx.unlock();
} /* SecureElementImpl::clearCallbacks() */


/*===========================================================================

FUNCTION:  SecureElementModule::SecureElementModule

===========================================================================*/
SecureElementModule::SecureElementModule()
{
  mName  = "SecureElementModule";
  mQmiUimUp = false;
  mServiceUp = false;
  mReady = false;

  if (property_get_bool(UICC_SE_HAL_PROP, true))
  {
    QCRIL_LOG_INFO("PROP enabled");

    mLooper = std::unique_ptr<ModuleLooper>(new ModuleLooper);

    using std::placeholders::_1;
    mMessageHandler = {
        HANDLER(QcrilInitMessage, SecureElementModule::handleQcrilInitMessage),
        HANDLER(UimQmiUimIndicationMsg, SecureElementModule::handleQmiUimIndiaction),
        HANDLER(SecureElementOnStateChangeIndMsg, SecureElementModule::handleSecureElementOnStateChangeInd)
    };
  }
} /* SecureElementModule::SecureElementModule */


/*===========================================================================

FUNCTION:  SecureElementModule::~SecureElementModule

===========================================================================*/
SecureElementModule::~SecureElementModule()
{
  mQmiUimUp = false;
  mServiceUp = false;
  mReady = false;
} /* SecureElementModule::~SecureElementModule */


/*===========================================================================

FUNCTION:  SecureElementModule::init

===========================================================================*/
void SecureElementModule::init()
{
  Module::init();

  // Initializations complete.
  mReady = false;
  mQmiUimUp = false;
  mServiceUp = false;

  ModemEndPointFactory<UimModemEndPoint>::getInstance().buildEndPoint();
} /* SecureElementModule::init */


/*===========================================================================

FUNCTION:  SecureElementModule::handleQcrilInitMessage

===========================================================================*/
void SecureElementModule::handleQcrilInitMessage
(
  std::shared_ptr<QcrilInitMessage> msg
)
{
  std::shared_ptr<string>    sharedResponse;
  android::status_t          ret = android::OK;

  (void)msg;

  ModemEndPointFactory<UimModemEndPoint>::getInstance().buildEndPoint()->requestSetupSync(
                                  sharedResponse);

  secureElementService = new SecureElementImpl{};

  if (secureElementService != NULL)
  {
    switch(qmi_ril_get_process_instance_id())
    {
      case 0:
        ret = secureElementService->registerAsService("SIM1");
        break;
      case 1:
        ret = secureElementService->registerAsService("SIM2");
        break;
      default:
        break;
    }
  }
  QCRIL_LOG_INFO("secure_element_register_service status=%d", ret);
} /* SecureElementModule::handleQcrilInitMessage */


/*===========================================================================

FUNCTION:  SecureElementModule::handleQmiUimIndiaction

===========================================================================*/
void SecureElementModule::handleQmiUimIndiaction(
          std::shared_ptr<UimQmiUimIndicationMsg> uim_qmi_ind_msg_ptr)
{
  sp<ISecureElementHalCallback> callback =
      (secureElementService == nullptr) ? nullptr
                                        : secureElementService->getISecureElementHalCallback();
  Return<void>                  ret;

  if (uim_qmi_ind_msg_ptr != NULL)
  {
    switch(uim_qmi_ind_msg_ptr->get_ind_id())
    {
      case QMI_UIM_SRVC_UP_IND_MSG:
        mQmiUimUp = true;

        if (mServiceUp)
        {
          mReady = true;
          handleSecureElementModuleReady();
        }
        break;

      case QMI_UIM_SRVC_DOWN_IND_MSG:
        mReady = false;
        mQmiUimUp = false;
        card_accessible = false;

        if (callback != nullptr)
        {
          ret = callback->onStateChange(false);
          if (!ret.isOk())
          {
            QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
          }
        }

        if (secureElementService != nullptr)
        {
          std::lock_guard<qtimutex::QtiSharedMutex> lock(secureElementService->ch_id_list_lock);
          if (!secureElementService->ch_id_list.empty())
          {
            secureElementService->ch_id_list.clear();
          }
        }

        /* Long APDU transaction might be in progress send signal to send response */
        mutx.lock();
        if (long_apdu_info.in_use == TRUE)
        {
          cleanUpLongApduInfo();
          cond.notify_one();
        }
        mutx.unlock();
        break;

      case QMI_UIM_SRVC_STATUS_CHANGE_IND_MSG:
        {
          secure_element_indication_params_type * param_ptr =
            (secure_element_indication_params_type *)uim_qmi_ind_msg_ptr->get_message();
          uint8_t                            slot = (uint8_t) qmi_ril_get_process_instance_id();

          if (slot < QMI_UIM_MAX_CARD_COUNT &&
              param_ptr != nullptr &&
              param_ptr->ind_data.status_change_ind.card_status_validity.card_status_invalid[slot]
                                                                               == QMI_UIM_FALSE &&
              param_ptr->ind_data.status_change_ind.card_status.card[slot].card_state
                                                                    == QMI_UIM_CARD_STATE_PRESENT)
          {
            if (callback != nullptr && !card_accessible)
            {
              /* Delay the card insert time for 3 secs to delay non prov app activation requests */
              sleep(3);
              ret = callback->onStateChange(true);

              if (!ret.isOk())
              {
                QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
              }
            }
            card_accessible = true;
          }
          else
          {
            if (callback != nullptr && card_accessible)
            {
              ret = callback->onStateChange(false);

              if (!ret.isOk())
              {
                QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
              }

              if (secureElementService != nullptr)
              {
                std::lock_guard<qtimutex::QtiSharedMutex> lock(secureElementService->ch_id_list_lock);
                if (!secureElementService->ch_id_list.empty())
                {
                  secureElementService->ch_id_list.clear();
                }
              }
            }
            card_accessible = false;

            /* Long APDU transaction might be in progress send signal to send response */
            mutx.lock();
            if (long_apdu_info.in_use == TRUE)
            {
              cleanUpLongApduInfo();
              cond.notify_one();
            }
            mutx.unlock();
          }
        }
        break;

      case QMI_UIM_SRVC_RECOVERY_IND_MSG:
        /* Long APDU transaction might be in progress send signal to send response */
        mutx.lock();
        if (long_apdu_info.in_use == TRUE)
        {
          cleanUpLongApduInfo();
          cond.notify_one();
        }
        mutx.unlock();

        if (secureElementService != nullptr && callback != nullptr)
        {
          /* Clear cached channel ids and send state change to false and true for recovery
             So that app can clear its data and can connect again */
          ret = callback->onStateChange(false);

          if (!ret.isOk())
          {
            QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
          }

          {
            std::lock_guard<qtimutex::QtiSharedMutex> lock(secureElementService->ch_id_list_lock);
            if (!secureElementService->ch_id_list.empty())
            {
              secureElementClearChannelIdList(secureElementService->ch_id_list);
            }
          }

          /* Delay the card insert time for 3 secs to delay non prov app activation requests */
          sleep(3);

          ret = callback->onStateChange(true);

          if (!ret.isOk())
          {
            QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
          }
        }
        break;

      case QMI_UIM_SRVC_REFRESH_IND_MSG:
        if (secureElementService != nullptr && callback != nullptr)
        {
          secure_element_indication_params_type * param_ptr =
            (secure_element_indication_params_type *)uim_qmi_ind_msg_ptr->get_message();

          /* Clear cached channel ids and send state change to false for refresh reset
             so that app can cleanup its data */
          if (param_ptr != nullptr &&
              param_ptr->ind_data.refresh_ind.refresh_event.refresh_stage
                                                             == QMI_UIM_REFRESH_STAGE_START &&
              param_ptr->ind_data.refresh_ind.refresh_event.refresh_mode
                                                             == QMI_UIM_REFRESH_MODE_RESET)
          {
            if (card_accessible)
            {
              ret = callback->onStateChange(false);

              if (!ret.isOk())
              {
                QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
              }

              {
                std::lock_guard<qtimutex::QtiSharedMutex> lock(secureElementService->ch_id_list_lock);
                if (!secureElementService->ch_id_list.empty())
                {
                  secureElementService->ch_id_list.clear();
                }
              }
            }
            card_accessible = false;

            /* Long APDU transaction might be in progress send signal to send response */
            mutx.lock();
            if (long_apdu_info.in_use == TRUE)
            {
              cleanUpLongApduInfo();
              cond.notify_one();
            }
            mutx.unlock();
          }
        }
        break;

      case QMI_UIM_SRVC_SEND_APDU_IND_MSG:
        if (is_send_apdu_in_progress)
        {
          secure_element_indication_params_type * param_ptr     =
            (secure_element_indication_params_type *)uim_qmi_ind_msg_ptr->get_message();
          qmi_uim_send_apdu_ind_type       * apdu_ind_ptr  = nullptr;
          uint16_t                           remaining_len = 0;
          uint16_t                           stored_len    = 0;

          if (param_ptr == nullptr)
          {
            break;
          }

          mutx.lock();

          apdu_ind_ptr = (qmi_uim_send_apdu_ind_type*)&param_ptr->ind_data.send_apdu_ind;
          QCRIL_LOG_INFO("Send APDU Indication - token: 0x%x, total_len: 0x%x, offset: 0x%x, data_len: 0x%x",
                         apdu_ind_ptr->token,
                         apdu_ind_ptr->total_len,
                         apdu_ind_ptr->offset,
                         apdu_ind_ptr->apdu.data_len);

          if (long_apdu_info.in_use == TRUE)
          {
            /* If Response already came, we need to check incoming info */
            if ((long_apdu_info.token          != apdu_ind_ptr->token) ||
                (long_apdu_info.total_apdu_len != apdu_ind_ptr->total_len))
            {
              /* Error condition - mismatch in data, discrd the response */
              QCRIL_LOG_ERROR("Mismatch with global data, token: 0x%x, total_apdu_len: 0x%x",
                            long_apdu_info.token,
                            long_apdu_info.total_apdu_len);
              cleanUpLongApduInfo();
              cond.notify_one();
              mutx.unlock();
              break;
            }
          }
          else
          {
            /* Response hasn't come yet, we can still store IND info */
            QCRIL_LOG_INFO("long_apdu_info.in_use is FALSE, storing info");
            long_apdu_info.in_use         = TRUE;
            long_apdu_info.token          = apdu_ind_ptr->token;
            long_apdu_info.total_apdu_len = apdu_ind_ptr->total_len;
          }

          /* If this is the first chunk, allocate the buffer. This buffer will
             only be freed at the end of the receiving all the INDs */
          if (long_apdu_info.apdu_ptr == NULL)
          {
            long_apdu_info.rx_len = 0;
            long_apdu_info.apdu_ptr = (uint8 *)qcril_malloc(apdu_ind_ptr->total_len);
            if (long_apdu_info.apdu_ptr == NULL)
            {
              QCRIL_LOG_ERROR("%s", "Couldnt allocate apdu_ptr pointer !");
              long_apdu_info.in_use = FALSE;
              cond.notify_one();
              mutx.unlock();
              break;
            }
          }

          /* Find out the remaining APDU buffer length */
          stored_len    = long_apdu_info.rx_len;
          remaining_len = long_apdu_info.total_apdu_len - stored_len;

          /* If this chunk cannot fit in our global buffer, discard the IND */
          if ((apdu_ind_ptr->apdu.data_len > remaining_len) ||
              (apdu_ind_ptr->offset  >= long_apdu_info.total_apdu_len) ||
              ((apdu_ind_ptr->offset + apdu_ind_ptr->apdu.data_len) > long_apdu_info.total_apdu_len))
          {
            QCRIL_LOG_ERROR("Mismatch with global data, total_apdu_len: 0x%x stored_len: 0x%x, remaining_len: 0x%x",
                            long_apdu_info.total_apdu_len,
                            stored_len,
                            remaining_len);
            long_apdu_info.in_use = FALSE;
            cond.notify_one();
            mutx.unlock();
            break;
          }

          /* Save the data & update the data length */
          memcpy(long_apdu_info.apdu_ptr + apdu_ind_ptr->offset,
                 apdu_ind_ptr->apdu.data_ptr,
                 apdu_ind_ptr->apdu.data_len);
          long_apdu_info.rx_len += apdu_ind_ptr->apdu.data_len;

          /* If it is the last one, send the response back & clean up global buffer */
          if (long_apdu_info.total_apdu_len == long_apdu_info.rx_len)
          {
            cond.notify_one();
          }
          mutx.unlock();
        }
        break;
    }
  }
} /* SecureElementModule::handleQmiUimIndiaction */


/*===========================================================================

FUNCTION:  SecureElementModule::handleSecureElementOnStateChangeInd

===========================================================================*/
void SecureElementModule::handleSecureElementOnStateChangeInd(
          std::shared_ptr<SecureElementOnStateChangeIndMsg> msg)
{
  if(msg == NULL)
  {
    QCRIL_LOG_ERROR("SecureElementOnStateChangeIndMsg is NULL");
  }

  /* Msg should never be NULL, call handleSecureElementModuleReady anyways */
  handleSecureElementModuleReady();

} /* SecureElementModule::handleSecureElementOnStateChangeInd */

#endif
