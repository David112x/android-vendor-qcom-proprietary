/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include <android/hardware/secure_element/1.0/ISecureElement.h>
#include <cutils/properties.h>
#include <hidl/HidlTransportSupport.h>
#include <utils/Mutex.h>

#include <vector>
#include <queue>

extern "C"
{
  #include "qcril_uim.h"
  #include "qcril_uim_qcci.h"
}

using namespace android::hardware::secure_element;
using namespace android::hardware::secure_element::V1_0;
using ::android::hardware::Return;
using ::android::hardware::hidl_vec;
using ::android::hardware::Void;
using ::android::hardware::hidl_death_recipient;
using ::android::Mutex;
using android::sp;
using android::hardware::Status;

#define QCRIL_UIM_INS_BYTE_SELECT                   0xA4
#define QCRIL_UIM_P1_VALUE_SELECT_BY_DF_NAME        0x04
#define QCRIL_UIM_P2_MASK_SELECT_NEXT               0x02

/* Delay timer value. We use this to delay sending card inserted event to
   Secure Element client */
#define QCRIL_UIM_SECURE_ELEMENT_TIMER              0x03

#define UICC_SE_HAL_PROP "persist.vendor.radio.uicc_se_enabled"


/*===========================================================================

                         DATA TYPE DECLARATIONS

===========================================================================*/
enum SecureElementEvent
{
  SE_INIT            = 0,
  CARD_PRESENT       = 1,
  CARD_ABSENT        = 2,
  TIMER_EXPIRED      = 3,
  CARD_RECOVERY_DONE = 4
};


/*===========================================================================

                         CLASS DEFINITIONS

===========================================================================*/

struct SecureElementImpl : public ISecureElement, public hidl_death_recipient
{
  sp<ISecureElementHalCallback> mClientCb;
  Mutex mCallbackLock;
  Mutex ch_id_list_lock;
  std::vector<uint8_t> ch_id_list;

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


static pthread_mutexattr_t            mattr;
static pthread_cond_t                 cond                     = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t                mutx                     = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutexattr_t            mattr_se_queue;
static pthread_cond_t                 cond_se_queue            = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t                mutx_se_queue            = PTHREAD_MUTEX_INITIALIZER;
static qcril_uim_long_apdu_info_type  long_apdu_info           = {};
static boolean                        is_send_apdu_in_progress = FALSE;
static sp<SecureElementImpl>          secureElementService     = NULL;
static bool                           card_accessible          = false;
static std::queue<SecureElementEvent> seQueue;
static struct
{
  uint32          timer_id;
  struct timeval  secure_element_status_timer;
} qcril_uim_se_delay_timer = {0, {QCRIL_UIM_SECURE_ELEMENT_TIMER, 0}};


/*===========================================================================

                             MACROS

===========================================================================*/
#define SE_MUTEX_LOCK(mutex) QCRIL_LOG_ADDITIONAL("Before acquiring SE LOCK %s", #mutex); \
        pthread_mutex_lock(&mutex); \
        QCRIL_LOG_ADDITIONAL("After acquiring SE LOCK %s", #mutex);

#define SE_MUTEX_UNLOCK(mutex) QCRIL_LOG_ADDITIONAL("Before releasing SE LOCK %s", #mutex);\
        pthread_mutex_unlock(&mutex);\
        QCRIL_LOG_ADDITIONAL("After releasing SE LOCK %s", #mutex);

#define SE_QUEUE_PUSH_AND_SIGNAL(event)\
                            SE_MUTEX_LOCK(mutx_se_queue);\
                            seQueue.push(event);\
                            pthread_cond_signal(&cond_se_queue);\
                            SE_MUTEX_UNLOCK(mutx_se_queue);


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

    (void) qcril_qmi_uim_logical_channel(qcril_uim.qmi_handle,
                                         &close_ch,
                                         NULL,
                                         NULL,
                                         &rsp_data);
  }
  ch_id_list.clear();
} /* secureElementClearChannelIdList */


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
    case QMI_ERR_INSUFFICIENT_RESOURCES_V01:
      return SecureElementStatus::CHANNEL_NOT_AVAILABLE;

    case QMI_ERR_INCOMPATIBLE_STATE_V01:
    case QMI_ERR_SIM_FILE_NOT_FOUND_V01:
      return SecureElementStatus::NO_SUCH_ELEMENT_ERROR;

    case QMI_ERR_NONE_V01:
      return SecureElementStatus::SUCCESS;

    default:
      return SecureElementStatus::FAILED;
  }
} /* SecureElementImpl::convertQmiErrSecureElementStatus */


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

  qmi_err_code = qcril_qmi_uim_open_logical_channel(qcril_uim.qmi_handle,
                                                    &open_ch,
                                                    NULL,
                                                    NULL,
                                                    &rsp_data);

  delete[] open_ch.aid.data_ptr;

  if (qmi_err_code != QMI_ERR_NONE_V01)
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
  sp<ISecureElementHalCallback> callback = nullptr;

  QCRIL_LOG_INFO("SecureElementImpl::init");
  {
    Mutex::Autolock lock(mCallbackLock);
    if (mClientCb != nullptr)
    {
      mClientCb->unlinkToDeath(this);
    }
    mClientCb = clientCallback;
    callback = mClientCb;
    if (mClientCb != nullptr)
    {
      mClientCb->linkToDeath(this, 0);
    }
  }

  SE_QUEUE_PUSH_AND_SIGNAL(SE_INIT);

  return Void();
} /* SecureElementImpl::init */


/*===========================================================================

FUNCTION:  SecureElementImpl::isCardPresent

===========================================================================*/
Return<bool> SecureElementImpl::isCardPresent
(
  void
)
{
  return card_accessible;
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
  qmi_err_code = qcril_qmi_uim_get_atr(qcril_uim.qmi_handle,
                                       &get_atr_params,
                                       NULL,
                                       NULL,
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

    qmi_err_code = qcril_qmi_uim_reselect(qcril_uim.qmi_handle,
                                          &reselect_params,
                                          NULL,
                                          NULL,
                                          &rsp_data);

    if (rsp_data.qmi_err_code != QMI_ERR_INVALID_QMI_CMD_V01)
    {
      select_rsp =
        new uint8_t[rsp_data.rsp_data.reselect_rsp.select_response.data_len + 2];

      not_select_next = false;

      if (select_rsp != NULL)
      {
        if (rsp_data.qmi_err_code == QMI_ERR_NOT_SUPPORTED_V01)
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
    qmi_err_code = qcril_qmi_uim_send_apdu(qcril_uim.qmi_handle,
                                           &apdu_params,
                                           NULL,
                                           NULL,
                                           &rsp_data);

    delete[] apdu_params.apdu.data_ptr;

    /* Special case for long APDUs - we send the response of the long APDU
       stream after we get all the chunks in the corresponding SEND_APDU_INDs */
    if (rsp_data.qmi_err_code == QMI_ERR_INSUFFICIENT_RESOURCES_V01)
    {
      QCRIL_LOG_DEBUG("transmit: total_len=0x%x bytes, token=0x%x",
                       rsp_data.rsp_data.send_apdu_rsp.total_len,
                       rsp_data.rsp_data.send_apdu_rsp.token);

      SE_MUTEX_LOCK(mutx);

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
        pthread_cond_wait(&cond, &mutx);

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

      SE_MUTEX_UNLOCK(mutx);

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
    Mutex::Autolock lock(ch_id_list_lock);

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

  qmi_err_code = qcril_qmi_uim_logical_channel(qcril_uim.qmi_handle,
                                               &close_ch,
                                               NULL,
                                               NULL,
                                               &rsp_data);

  QCRIL_LOG_INFO("SecureElementImpl::closeChannel status : %d", qmi_err_code);

  if (qmi_err_code != QMI_ERR_NONE_V01 ||
      rsp_data.qmi_err_code != QMI_ERR_NONE_V01)
  {
    return SecureElementStatus::FAILED;
  }
  else
  {
    Mutex::Autolock lock(ch_id_list_lock);

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
    Mutex::Autolock lock(mCallbackLock);
    if (mClientCb != nullptr)
    {
      mClientCb = NULL;
    }
  }

  {
    Mutex::Autolock lock(ch_id_list_lock);
    /* Clear cached channel ids if client dies */
    if (!ch_id_list.empty())
    {
      secureElementClearChannelIdList(ch_id_list);
    }
  }

  /* Long APDU transaction might be in progress send signal to send response */
  SE_MUTEX_LOCK(mutx);
  if (long_apdu_info.in_use == TRUE)
  {
    cleanUpLongApduInfo();
    pthread_cond_signal(&cond);
  }
  SE_MUTEX_UNLOCK(mutx);
} /* SecureElementImpl::clearCallbacks() */


/*===========================================================================

FUNCTION:  SecureElementClearTimerInfo

===========================================================================*/
void SecureElementClearTimerInfo
(
  void
)
{
  QCRIL_LOG_INFO("SecureElementClearTimerInfo");

  if(qcril_uim_se_delay_timer.timer_id != 0)
  {
    qcril_cancel_timed_callback((void *)(long)qcril_uim_se_delay_timer.timer_id);
    qcril_uim_se_delay_timer.timer_id = 0;
  }
}


/*===========================================================================

FUNCTION:  SecureElementTimedCb

===========================================================================*/
void SecureElementTimedCb
(
  void *param_ptr
)
{
  (void) param_ptr;

  QCRIL_LOG_INFO("SecureElementTimedCb");

  SE_QUEUE_PUSH_AND_SIGNAL(TIMER_EXPIRED);

  return;
}


/*===========================================================================

FUNCTION:  SecureElementThreadHandler

===========================================================================*/

void *SecureElementThreadHandler
(
  void *param_ptr
)
{
  (void)param_ptr;

  qcril_instance_id_e_type      ril_instance_id        = qmi_ril_get_process_instance_id();
  android::status_t             ret_RegistrationStatus = android::UNKNOWN_ERROR;
  SecureElementEvent            event                  = SE_INIT;
  sp<ISecureElementHalCallback> callback               = nullptr;
  Return<void>                  ret;
  pthread_t                     tid;

  QCRIL_LOG_INFO("In SecureElementThreadHandler");

  tid = pthread_self();
  qmi_ril_set_thread_name(tid, "SecureElementThread");

  secureElementService = new SecureElementImpl{};

  if (secureElementService != NULL)
  {
    switch(ril_instance_id)
    {
      case 0:
        ret_RegistrationStatus = secureElementService->registerAsService("SIM1");
        break;
      case 1:
        ret_RegistrationStatus = secureElementService->registerAsService("SIM2");
        break;
      default:
        break;
    }
    QCRIL_LOG_INFO("secure_element_register_service status=%d", ret_RegistrationStatus);
  }

  if (ret_RegistrationStatus == android::OK)
  {
    /* Infinite loop in which we process the events in the queue if queue is non empty or else
     we will wait for any update of event on the queue*/
    while(1)
    {
      SE_MUTEX_LOCK(mutx_se_queue);

      if (seQueue.empty())
      {
        /* Here mutx is released and we wait on the condition.
           Once we are unblocked, mutex will be again acquired.*/
        QCRIL_LOG_ADDITIONAL("Released SE LOCK mutx_se_queue");
        pthread_cond_wait(&cond_se_queue, &mutx_se_queue);
        QCRIL_LOG_ADDITIONAL("Acquired SE LOCK mutx_se_queue");
      }

      QCRIL_LOG_INFO("Processing event in SecureElement queue");

      callback = secureElementService->mClientCb;
      event    = seQueue.front();
      seQueue.pop();
      SE_MUTEX_UNLOCK(mutx_se_queue);

      switch (event)
      {
        case SE_INIT:
          QCRIL_LOG_INFO("SecureElement init event");

          if(callback != nullptr && qcril_uim_se_delay_timer.timer_id == 0 && card_accessible)
          {
            ret = callback->onStateChange(true);
            if (!ret.isOk())
            {
               QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
            }
          }
          break;

        case TIMER_EXPIRED:
          QCRIL_LOG_INFO("SecureElement TIMER expired");

          SecureElementClearTimerInfo();
          card_accessible = true;

          /* Since the timer is expired, we need to send onStateChange
             indication to Secure Element client */
          if(callback != nullptr)
          {
            ret = callback->onStateChange(true);

            if (!ret.isOk())
            {
               QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
            }
          }
          break;

        case CARD_ABSENT:
          QCRIL_LOG_INFO("SecureElement handling card absent");

          card_accessible = false;
          if(qcril_uim_se_delay_timer.timer_id != 0)
          {
            SecureElementClearTimerInfo();
          }
          else if(callback != nullptr)
          {
           ret = callback->onStateChange(false);
           if (!ret.isOk())
           {
              QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
           }
          }
          break;

        case CARD_RECOVERY_DONE:
          QCRIL_LOG_INFO("SecureElement handling Recovery");

          if(qcril_uim_se_delay_timer.timer_id == 0)
          {
            if(callback != nullptr)
            {
              ret = callback->onStateChange(false);
              if (!ret.isOk())
              {
                 QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
                 break;
              }
              ret = callback->onStateChange(true);
              if (!ret.isOk())
              {
                QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
              }
            }
          }
          else
          {
            SecureElementClearTimerInfo();
            qcril_setup_timed_callback(ril_instance_id,
                                       QCRIL_DEFAULT_MODEM_ID,
                                       SecureElementTimedCb,
                                       &qcril_uim_se_delay_timer.secure_element_status_timer,
                                       &qcril_uim_se_delay_timer.timer_id);
          }
          break;

        case CARD_PRESENT:
          QCRIL_LOG_INFO("SecureElement handling card present");

          qcril_setup_timed_callback(ril_instance_id,
                                     QCRIL_DEFAULT_MODEM_ID,
                                     SecureElementTimedCb,
                                     &qcril_uim_se_delay_timer.secure_element_status_timer,
                                     &qcril_uim_se_delay_timer.timer_id);
          break;

        default:
          QCRIL_LOG_INFO("Unhandled event in SecureElement %d", (int)event);
          break;
      }
    }
  }

  qmi_ril_clear_thread_name(tid);

  return NULL;
}


/*===========================================================================

FUNCTION:  SecureElementInit

===========================================================================*/
extern "C" void SecureElementInit
(
  void
)
{
  pthread_attr_t  attr;
  pthread_t       thread_pid;
  int             res = 0;

  if (!property_get_bool(UICC_SE_HAL_PROP, true))
  {
    return;
  }

  /* Initialize the mutex and condition variables */
  (void)pthread_mutexattr_init(&mattr);
  (void)pthread_mutexattr_settype(&mattr,PTHREAD_MUTEX_RECURSIVE_NP);
  (void)pthread_mutex_init(&mutx, &mattr);
  (void)pthread_cond_init(&cond, NULL);

  (void)pthread_mutexattr_init(&mattr_se_queue);
  (void)pthread_mutexattr_settype(&mattr_se_queue,PTHREAD_MUTEX_RECURSIVE_NP);
  (void)pthread_mutex_init(&mutx_se_queue, &mattr_se_queue);
  (void)pthread_cond_init(&cond_se_queue, NULL);

  res = pthread_attr_init(&attr);
  if(res == 0)
  {

    res = pthread_create(&thread_pid, &attr,
                         SecureElementThreadHandler,
                         NULL);

    QCRIL_LOG_INFO("SE thread created with tid as %ld",(long int)thread_pid);
  }

  return;
} /* SecureElementRegisterService */


/*===========================================================================

FUNCTION:  SecureElementhandleCardStateChange

===========================================================================*/
extern "C" void SecureElementhandleCardStateChange
(
  qmi_uim_card_state_type card_state
)
{
  sp<ISecureElementHalCallback> callback =
  (secureElementService == nullptr) ? nullptr : secureElementService->mClientCb;

  QCRIL_LOG_INFO("SecureElementhandleCardStateChange card_state :%d", card_state);

  if (secureElementService != nullptr)
  {
    if (card_state == QMI_UIM_CARD_STATE_PRESENT)
    {
      SE_QUEUE_PUSH_AND_SIGNAL(CARD_PRESENT);
    }
    else
    {
      SE_QUEUE_PUSH_AND_SIGNAL(CARD_ABSENT);

      {
        Mutex::Autolock lock(secureElementService->ch_id_list_lock);
        if (!secureElementService->ch_id_list.empty())
        {
          secureElementService->ch_id_list.clear();
        }
      }

      /* Long APDU transaction might be in progress send signal to send response */
      SE_MUTEX_LOCK(mutx);
      if (long_apdu_info.in_use == TRUE)
      {
        cleanUpLongApduInfo();
        pthread_cond_signal(&cond);
      }
      SE_MUTEX_UNLOCK(mutx);
    }
  }

} /* SecureElementhandleCardStateChange */


/*===========================================================================

FUNCTION:  SecureElementhandleRecovery

===========================================================================*/
extern "C" void SecureElementhandleRecovery()
{
  sp<ISecureElementHalCallback> callback =
  (secureElementService == nullptr) ? nullptr : secureElementService->mClientCb;

  QCRIL_LOG_INFO("SecureElementhandleRecovery");

  /* Long APDU transaction might be in progress send signal to send response */
  SE_MUTEX_LOCK(mutx);
  if (long_apdu_info.in_use == TRUE)
  {
    cleanUpLongApduInfo();
    pthread_cond_signal(&cond);
  }
  SE_MUTEX_UNLOCK(mutx);

  if (secureElementService != nullptr)
  {
    SE_QUEUE_PUSH_AND_SIGNAL(CARD_RECOVERY_DONE);

    {
      Mutex::Autolock lock(secureElementService->ch_id_list_lock);
      if (!secureElementService->ch_id_list.empty())
      {
        secureElementService->ch_id_list.clear();
      }
    }
  }
} /* SecureElementhandleRecovery */


/*===========================================================================

FUNCTION:  SecureElementProcessApduInd

===========================================================================*/
extern "C" void SecureElementProcessApduInd
(
  const qcril_uim_indication_params_type  * param_ptr
)
{
  qmi_uim_send_apdu_ind_type       * apdu_ind_ptr  = nullptr;
  uint16_t                           remaining_len = 0;
  uint16_t                           stored_len    = 0;

  if (param_ptr == nullptr)
  {
    return;
  }

  if (is_send_apdu_in_progress)
  {
    SE_MUTEX_LOCK(mutx);

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
        pthread_cond_signal(&cond);
        SE_MUTEX_UNLOCK(mutx);
        return;
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
        pthread_cond_signal(&cond);
        SE_MUTEX_UNLOCK(mutx);
        return;
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
      pthread_cond_signal(&cond);
      SE_MUTEX_UNLOCK(mutx);
      return;
    }

    /* Save the data & update the data length */
    memcpy(long_apdu_info.apdu_ptr + apdu_ind_ptr->offset,
           apdu_ind_ptr->apdu.data_ptr,
           apdu_ind_ptr->apdu.data_len);
    long_apdu_info.rx_len += apdu_ind_ptr->apdu.data_len;

    /* If it is the last one, send the response back & clean up global buffer */
    if (long_apdu_info.total_apdu_len == long_apdu_info.rx_len)
    {
      pthread_cond_signal(&cond);
    }
    SE_MUTEX_UNLOCK(mutx);
  }
} /* SecureElementProcessApduInd */
