/*===========================================================================

  Copyright (c) 2017 - 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include "framework/Log.h"
#include "modules/uim/qcril_uim_srvc.h"
#include "modules/uim/UimChangeSubscriptionReqMsg.h"
#include "modules/uim/UimProvisionStatusIndMsg.h"
#include "modules/uim/UimCardStatusChangeIndMsg.h"
#include "modules/uim/UimCardPowerReqMsg.h"
#include "modules/uim/UimCardAppStatusIndMsg.h"
#include "modules/uim/UimCardStateChangeInd.h"
#include "modules/uim/UimSimStatusChangedInd.h"
#include "modules/uim/UimVoltageStatusInd.h"
#include "modules/uim/UimSimInternalResetInd.h"
#include "UimTimerExpiryMsg.h"
#include "UimTimerRequestMsg.h"
#include "qcril_uim_sap.h"
#include "qcril_uim.h"
#include "qcril_uim_util.h"
#include "qcril_uim_card.h"
#include "qcril_uim_security.h"
#include "qcril_uim_refresh.h"
#include "qcril_uim_restart.h"
#include "qcril_uim_file.h"
#include <string.h>
#include <pthread.h>

#define TAG "QCRIL_UIM_CARD"
#define QCRIL_UIM_CARD_STATE_CHECK_TIME  1500 /* 1.5 secs */

/*===========================================================================

                               GLOBAL VARIABLES

===========================================================================*/
static std::shared_ptr<UimTimerRequestMsg> qcril_uim_card_state_check_timer_msg_ptr = NULL;

static qtimutex::QtiSharedMutex qcril_uim_card_status_mutex;

/*===========================================================================

                               INTERNAL FUNCTIONS

===========================================================================*/

/*===========================================================================

  FUNCTION:  qcril_uim_send_get_label

===========================================================================*/
/*!
    @brief
    Issues the QMI command for fetching label for an App.

    @return
    result_code of the QMI call.
*/
/*=========================================================================*/
static int qcril_uim_send_get_label
(
  uint8                                     slot_index,
  qmi_uim_rsp_data_type                  *  qmi_rsp_data_ptr,
  const char                             *  qmi_aid_ptr,
  unsigned char                             qmi_aid_len
)
{
  int result_code                         = -1;
  qmi_uim_get_label_params_type           get_label_params;

  memset(&get_label_params, 0, sizeof(qmi_uim_get_label_params_type));

  if (qmi_aid_len <= 0)
  {
    QCRIL_LOG_DEBUG( "Application's aid len <= 0 %d \n", (int)qmi_aid_len);
    return result_code;
  }

  switch (slot_index)
  {
    case 0:
      get_label_params.app_info.slot = QMI_UIM_SLOT_1;
      break;
    case 1:
      get_label_params.app_info.slot = QMI_UIM_SLOT_2;
      break;
    case 2:
      get_label_params.app_info.slot = QMI_UIM_SLOT_3;
      break;
    default:
      QCRIL_LOG_ERROR( "Invalid slot for qcril_uim_send_get_label: %d,\n",
                       slot_index);
      return result_code;
  }

  get_label_params.app_info.aid.data_len = qmi_aid_len;
  get_label_params.app_info.aid.data_ptr = (unsigned char *)qmi_aid_ptr;

  result_code = qcril_uim_send_qmi_sync_msg(QCRIL_UIM_REQUEST_GET_LABEL,
                                            &get_label_params,
                                            qmi_rsp_data_ptr);
  if(result_code < 0)
  {
    QCRIL_LOG_ERROR( "Error for qcril_qmi_uim_get_label, result_code: %d, qmi_err_code: 0x%X\n",
                     result_code, qmi_rsp_data_ptr->qmi_err_code);
  }
  return result_code;
} /* qcril_uim_send_get_label */


/*=========================================================================

  FUNCTION:  qcril_uim_get_prov_session_state

===========================================================================*/
/*!
    @brief
    returns the state of passed session type from QCRIL uim globals.

    @return
    result_code
*/
/*=========================================================================*/
static int qcril_uim_get_prov_session_state
(
  qmi_uim_session_type                 session_type,
  qcril_uim_prov_session_state_type  * session_state_ptr /*!< Output parameter */
)
{
  int result_code = 0;

  /* Sanity check */
  if (session_state_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    return -1;
  }

  switch(session_type)
  {
    case QMI_UIM_SESSION_TYPE_PRI_GW_PROV:
      *session_state_ptr = qcril_uim.prov_session_info.session_state_gw_indexes[0];
      break;
    case QMI_UIM_SESSION_TYPE_PRI_1X_PROV:
      *session_state_ptr = qcril_uim.prov_session_info.session_state_1x_indexes[0];
      break;
    case QMI_UIM_SESSION_TYPE_SEC_GW_PROV:
      *session_state_ptr = qcril_uim.prov_session_info.session_state_gw_indexes[1];
      break;
    case QMI_UIM_SESSION_TYPE_SEC_1X_PROV:
      *session_state_ptr = qcril_uim.prov_session_info.session_state_1x_indexes[1];
      break;
    case QMI_UIM_SESSION_TYPE_TER_GW_PROV:
      *session_state_ptr = qcril_uim.prov_session_info.session_state_gw_indexes[2];
      break;
    case QMI_UIM_SESSION_TYPE_TER_1X_PROV:
      *session_state_ptr = qcril_uim.prov_session_info.session_state_1x_indexes[2];
      break;
    default:
      QCRIL_LOG_ERROR( "%s\n", "Unsupported session type!" );
      result_code = -1;
      break;
  }
  return result_code;
} /* qcril_uim_get_prov_session_state */


/*=========================================================================

  FUNCTION:  qcril_uim_update_prov_session_type

===========================================================================*/
/*!
    @brief
    Updates the global provisioning session status type based on the passed
    session type.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_update_prov_session_type
(
  qmi_uim_session_type                  session_type,
  qcril_uim_prov_session_state_type     session_state
)
{
  QCRIL_LOG_DEBUG("New session state, session_type:%d, session_state:%d \n",
                  (int)session_type, (int)session_state);

  switch(session_type)
  {
    case QMI_UIM_SESSION_TYPE_PRI_GW_PROV:
      qcril_uim.prov_session_info.session_state_gw_indexes[0] = session_state;
      break;
    case QMI_UIM_SESSION_TYPE_PRI_1X_PROV:
      qcril_uim.prov_session_info.session_state_1x_indexes[0] = session_state;
      break;
    case QMI_UIM_SESSION_TYPE_SEC_GW_PROV:
      qcril_uim.prov_session_info.session_state_gw_indexes[1] = session_state;
      break;
    case QMI_UIM_SESSION_TYPE_SEC_1X_PROV:
      qcril_uim.prov_session_info.session_state_1x_indexes[1] = session_state;
      break;
    case QMI_UIM_SESSION_TYPE_TER_GW_PROV:
      qcril_uim.prov_session_info.session_state_gw_indexes[2] = session_state;
      break;
    case QMI_UIM_SESSION_TYPE_TER_1X_PROV:
      qcril_uim.prov_session_info.session_state_1x_indexes[2] = session_state;
      break;
    default:
      QCRIL_LOG_ERROR( "%s\n", "Unsupported session type!" );
      break;
  }
} /* qcril_uim_update_prov_session_type */


/*===========================================================================

  FUNCTION:  qcril_uim_find_prov_session_type

===========================================================================*/
/*!
    @brief
    Updates the passed provisioning session parameter with the session type
    and also the session state pointer with the right state by checking the
    app type and previous state respectively.

    @return
    RIL_UIM_Errno error code
*/
/*=========================================================================*/
static RIL_UIM_Errno qcril_uim_find_prov_session_type
(
  qmi_uim_change_prov_session_params_type     * session_params_ptr,
  qmi_uim_app_type                              app_type,
  RIL_UIM_SubscriptionType                      RIL_sub_type,
  qcril_uim_subs_mode_pref                      subs_mode_pref,
  RIL_UIM_UiccSubActStatus                      act_status
)
{
  qcril_uim_prov_session_state_type  check_prov_state = QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED;
  qcril_uim_prov_session_state_type  next_prov_state  = QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED;

  if (session_params_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return RIL_UIM_E_INTERNAL_ERR;
  }

  QCRIL_LOG_INFO( "app_type: %d, RIL_sub_type: %d, subs_mode_pref: %d\n",
                  app_type,
                  RIL_sub_type,
                  subs_mode_pref);

  QCRIL_LOG_DEBUG("Original session states, GW_PRI:%d, GW_SEC:%d, GW_TER:%d, 1X_PRI:%d, 1X_SEC:%d, 1X_TER:%d\n",
                  qcril_uim.prov_session_info.session_state_gw_indexes[0],
                  qcril_uim.prov_session_info.session_state_gw_indexes[1],
                  qcril_uim.prov_session_info.session_state_gw_indexes[2],
                  qcril_uim.prov_session_info.session_state_1x_indexes[0],
                  qcril_uim.prov_session_info.session_state_1x_indexes[1],
                  qcril_uim.prov_session_info.session_state_1x_indexes[2]);

  /* Check app type */
  if ((app_type != QMI_UIM_APP_SIM) &&
      (app_type != QMI_UIM_APP_USIM) &&
      (app_type != QMI_UIM_APP_RUIM) &&
      (app_type != QMI_UIM_APP_CSIM))
  {
    QCRIL_LOG_ERROR("Invalid app_type: 0x%x", app_type);
    return RIL_UIM_E_INTERNAL_ERR;
  }

  if (act_status == UIM_UICC_SUBSCRIPTION_ACTIVATE)
  {
    check_prov_state = QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED;
    next_prov_state  = QCRIL_UIM_PROV_SESSION_ACTIVATION_IN_PROGRESS;
  }
  else if(act_status == UIM_UICC_SUBSCRIPTION_DEACTIVATE)
  {
    check_prov_state = QCRIL_UIM_PROV_SESSION_ACTIVATED;
    next_prov_state  = QCRIL_UIM_PROV_SESSION_DEACTIVATION_IN_PROGESS;
  }

  /* Check conditions for respective session types */
  if (((app_type == QMI_UIM_APP_SIM) || (app_type == QMI_UIM_APP_USIM)) &&
      (subs_mode_pref == QCRIL_SUBS_MODE_GW))
  {
    if ((RIL_sub_type == UIM_SUBSCRIPTION_1) &&
        (qcril_uim.prov_session_info.session_state_gw_indexes[0] == check_prov_state))
    {
      session_params_ptr->session_type = QMI_UIM_SESSION_TYPE_PRI_GW_PROV;
      qcril_uim_update_prov_session_type(session_params_ptr->session_type,
                                         next_prov_state);
      return RIL_UIM_E_SUCCESS;
    }
    else if ((RIL_sub_type == UIM_SUBSCRIPTION_2) &&
             (qcril_uim.prov_session_info.session_state_gw_indexes[1] == check_prov_state))
    {
      session_params_ptr->session_type = QMI_UIM_SESSION_TYPE_SEC_GW_PROV;
      qcril_uim_update_prov_session_type(session_params_ptr->session_type,
                                         next_prov_state);
      return RIL_UIM_E_SUCCESS;
    }
    else if ((RIL_sub_type == UIM_SUBSCRIPTION_3) &&
             (qcril_uim.prov_session_info.session_state_gw_indexes[2] == check_prov_state))
    {
      session_params_ptr->session_type = QMI_UIM_SESSION_TYPE_TER_GW_PROV;
      qcril_uim_update_prov_session_type(session_params_ptr->session_type,
                                         next_prov_state);
      return RIL_UIM_E_SUCCESS;
    }
    else
    {
      QCRIL_LOG_ERROR("Cannot Activate RIL_sub_type: %d\n", RIL_sub_type);
      return RIL_UIM_E_SUBSCRIPTION_NOT_SUPPORTED;
    }
  }
  else if (((app_type == QMI_UIM_APP_RUIM) || (app_type == QMI_UIM_APP_CSIM)) &&
           (subs_mode_pref == QCRIL_SUBS_MODE_1X))
  {
    if ((RIL_sub_type == UIM_SUBSCRIPTION_1) &&
        (qcril_uim.prov_session_info.session_state_1x_indexes[0] == check_prov_state))
    {
      session_params_ptr->session_type = QMI_UIM_SESSION_TYPE_PRI_1X_PROV;
      qcril_uim_update_prov_session_type(session_params_ptr->session_type,
                                         next_prov_state);
      return RIL_UIM_E_SUCCESS;
    }
    else if ((RIL_sub_type == UIM_SUBSCRIPTION_2) &&
             (qcril_uim.prov_session_info.session_state_1x_indexes[1] == check_prov_state))
    {
      session_params_ptr->session_type = QMI_UIM_SESSION_TYPE_SEC_1X_PROV;
      qcril_uim_update_prov_session_type(session_params_ptr->session_type,
                                         next_prov_state);
      return RIL_UIM_E_SUCCESS;
    }
    else if ((RIL_sub_type == UIM_SUBSCRIPTION_3) &&
             (qcril_uim.prov_session_info.session_state_1x_indexes[2] == check_prov_state))
    {
      session_params_ptr->session_type = QMI_UIM_SESSION_TYPE_TER_1X_PROV;
      qcril_uim_update_prov_session_type(session_params_ptr->session_type,
                                         next_prov_state);
      return RIL_UIM_E_SUCCESS;
    }
    else
    {
      QCRIL_LOG_ERROR("Cannot Activate RIL_sub_type: %d\n", RIL_sub_type);
      return RIL_UIM_E_SUBSCRIPTION_NOT_SUPPORTED;
    }
  }
  /* Only in case of mismatch of app type & mode pref */
  QCRIL_LOG_ERROR("Unsupported case, app_type: %d, subs_mode_pref:%d \n",
                  app_type, subs_mode_pref);
  return RIL_UIM_E_SUBSCRIPTION_NOT_SUPPORTED;
} /* qcril_uim_find_prov_session_type */


/*===========================================================================

  FUNCTION:  qcril_uim_ignore_change_subscription

===========================================================================*/
/*!
    @brief
    Checks if change prov session request can be ignored. It can be ignored
    only on scenarios where the request was for activation & the prov app is
    already activated and vice-versa.

    @return
    RIL_UIM_Errno error code
*/
/*=========================================================================*/
static RIL_UIM_Errno qcril_uim_ignore_change_subscription
(
  const qcril_uim_uicc_subs_info_type         * RIL_uicc_subs_info_ptr,
  qmi_uim_change_prov_session_params_type     * session_params_ptr
)
{
  int RIL_slot          = 0;
  int RIL_app_index     = 0;
  uint16 prov_app_index = QCRIL_UIM_INVALID_SLOT_APP_INDEX_VALUE;

  /* Sanity checks */
  if ((RIL_uicc_subs_info_ptr == NULL) || (session_params_ptr == NULL))
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return RIL_UIM_E_INTERNAL_ERR;
  }

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  RIL_slot = RIL_uicc_subs_info_ptr->uicc_subs_info.slot;
  RIL_app_index = RIL_uicc_subs_info_ptr->uicc_subs_info.app_index;

  /* Check slot number */
  if (RIL_slot >= QMI_UIM_MAX_CARD_COUNT ||
      RIL_slot < 0)
  {
    QCRIL_LOG_ERROR("Invalid slot value: 0x%x", RIL_slot);
    return RIL_UIM_E_INTERNAL_ERR;
  }

  /* Check app index */
  if ((RIL_app_index < 0) || (RIL_app_index >= QMI_UIM_MAX_APP_PER_CARD_COUNT))
  {
    QCRIL_LOG_ERROR("Invalid app index: 0x%x", RIL_app_index);
    return RIL_UIM_E_INTERNAL_ERR;
  }

  /* Verify card status */
  if (qcril_uim.card_status.card[RIL_slot].card_state != QMI_UIM_CARD_STATE_PRESENT)
  {
    QCRIL_LOG_ERROR("Card is not present");
    return RIL_UIM_E_INTERNAL_ERR;
  }

  /* Find prov app index */
  prov_app_index = ((RIL_slot & 0xFF) << 8) | (RIL_app_index & 0xFF);

  /* Now check if we need to proceed with incoming request */
  if ((RIL_uicc_subs_info_ptr->subs_mode_pref == QCRIL_SUBS_MODE_GW) &&
      (RIL_uicc_subs_info_ptr->uicc_subs_info.sub_type == UIM_SUBSCRIPTION_1))
  {
    if (((RIL_uicc_subs_info_ptr->uicc_subs_info.act_status == UIM_UICC_SUBSCRIPTION_ACTIVATE) &&
         (qcril_uim.prov_session_info.session_state_gw_indexes[0] == QCRIL_UIM_PROV_SESSION_ACTIVATED) &&
         (prov_app_index == qcril_uim.card_status.index_gw_pri_prov)) ||
        ((RIL_uicc_subs_info_ptr->uicc_subs_info.act_status == UIM_UICC_SUBSCRIPTION_DEACTIVATE) &&
        (qcril_uim.prov_session_info.session_state_gw_indexes[0] == QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED)))
    {
      session_params_ptr->session_type = QMI_UIM_SESSION_TYPE_PRI_GW_PROV;
      return RIL_UIM_E_SUCCESS;
    }
  }
  else if ((RIL_uicc_subs_info_ptr->subs_mode_pref == QCRIL_SUBS_MODE_1X) &&
           (RIL_uicc_subs_info_ptr->uicc_subs_info.sub_type == UIM_SUBSCRIPTION_1))
  {
    if (((RIL_uicc_subs_info_ptr->uicc_subs_info.act_status == UIM_UICC_SUBSCRIPTION_ACTIVATE) &&
         (qcril_uim.prov_session_info.session_state_1x_indexes[0] == QCRIL_UIM_PROV_SESSION_ACTIVATED) &&
         (prov_app_index == qcril_uim.card_status.index_1x_pri_prov)) ||
        ((RIL_uicc_subs_info_ptr->uicc_subs_info.act_status == UIM_UICC_SUBSCRIPTION_DEACTIVATE) &&
        (qcril_uim.prov_session_info.session_state_1x_indexes[0] == QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED)))
    {
      session_params_ptr->session_type = QMI_UIM_SESSION_TYPE_PRI_1X_PROV;
      return RIL_UIM_E_SUCCESS;
    }
  }
  else if ((RIL_uicc_subs_info_ptr->subs_mode_pref == QCRIL_SUBS_MODE_GW) &&
           (RIL_uicc_subs_info_ptr->uicc_subs_info.sub_type == UIM_SUBSCRIPTION_2))
  {
    if (((RIL_uicc_subs_info_ptr->uicc_subs_info.act_status == UIM_UICC_SUBSCRIPTION_ACTIVATE) &&
         (qcril_uim.prov_session_info.session_state_gw_indexes[1] == QCRIL_UIM_PROV_SESSION_ACTIVATED) &&
         (prov_app_index == qcril_uim.card_status.index_gw_sec_prov)) ||
        ((RIL_uicc_subs_info_ptr->uicc_subs_info.act_status == UIM_UICC_SUBSCRIPTION_DEACTIVATE) &&
        (qcril_uim.prov_session_info.session_state_gw_indexes[1] == QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED)))
    {
      session_params_ptr->session_type = QMI_UIM_SESSION_TYPE_SEC_GW_PROV;
      return RIL_UIM_E_SUCCESS;
    }
  }
  else if ((RIL_uicc_subs_info_ptr->subs_mode_pref == QCRIL_SUBS_MODE_1X) &&
           (RIL_uicc_subs_info_ptr->uicc_subs_info.sub_type == UIM_SUBSCRIPTION_2))
  {
    if (((RIL_uicc_subs_info_ptr->uicc_subs_info.act_status == UIM_UICC_SUBSCRIPTION_ACTIVATE) &&
         (qcril_uim.prov_session_info.session_state_1x_indexes[1] == QCRIL_UIM_PROV_SESSION_ACTIVATED) &&
         (prov_app_index == qcril_uim.card_status.index_1x_sec_prov)) ||
        ((RIL_uicc_subs_info_ptr->uicc_subs_info.act_status == UIM_UICC_SUBSCRIPTION_DEACTIVATE) &&
        (qcril_uim.prov_session_info.session_state_1x_indexes[1] == QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED)))
    {
      session_params_ptr->session_type = QMI_UIM_SESSION_TYPE_SEC_1X_PROV;
      return RIL_UIM_E_SUCCESS;
    }
  }
  else if ((RIL_uicc_subs_info_ptr->subs_mode_pref == QCRIL_SUBS_MODE_GW) &&
           (RIL_uicc_subs_info_ptr->uicc_subs_info.sub_type == UIM_SUBSCRIPTION_3))
  {
    if (((RIL_uicc_subs_info_ptr->uicc_subs_info.act_status == UIM_UICC_SUBSCRIPTION_ACTIVATE) &&
         (qcril_uim.prov_session_info.session_state_gw_indexes[2] == QCRIL_UIM_PROV_SESSION_ACTIVATED) &&
         (prov_app_index == qcril_uim.card_status.index_gw_ter_prov)) ||
        ((RIL_uicc_subs_info_ptr->uicc_subs_info.act_status == UIM_UICC_SUBSCRIPTION_DEACTIVATE) &&
        (qcril_uim.prov_session_info.session_state_gw_indexes[2] == QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED)))
    {
      session_params_ptr->session_type = QMI_UIM_SESSION_TYPE_TER_GW_PROV;
      return RIL_UIM_E_SUCCESS;
    }
  }
  else if ((RIL_uicc_subs_info_ptr->subs_mode_pref == QCRIL_SUBS_MODE_1X) &&
           (RIL_uicc_subs_info_ptr->uicc_subs_info.sub_type == UIM_SUBSCRIPTION_3))
  {
    if (((RIL_uicc_subs_info_ptr->uicc_subs_info.act_status == UIM_UICC_SUBSCRIPTION_ACTIVATE) &&
         (qcril_uim.prov_session_info.session_state_1x_indexes[2] == QCRIL_UIM_PROV_SESSION_ACTIVATED) &&
         (prov_app_index == qcril_uim.card_status.index_1x_ter_prov)) ||
        ((RIL_uicc_subs_info_ptr->uicc_subs_info.act_status == UIM_UICC_SUBSCRIPTION_DEACTIVATE) &&
        (qcril_uim.prov_session_info.session_state_1x_indexes[2] == QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED)))
    {
      session_params_ptr->session_type = QMI_UIM_SESSION_TYPE_TER_1X_PROV;
      return RIL_UIM_E_SUCCESS;
    }
  }

  QCRIL_LOG_DEBUG("Couldnt match session info: GW_PRI:%d, GW_SEC:%d, GW_TER:%d, 1X_PRI:%d, 1X_SEC:%d, 1X_TER:%d\n",
                  qcril_uim.prov_session_info.session_state_gw_indexes[0],
                  qcril_uim.prov_session_info.session_state_gw_indexes[1],
                  qcril_uim.prov_session_info.session_state_gw_indexes[2],
                  qcril_uim.prov_session_info.session_state_1x_indexes[0],
                  qcril_uim.prov_session_info.session_state_1x_indexes[1],
                  qcril_uim.prov_session_info.session_state_1x_indexes[2]);

  return RIL_UIM_E_INTERNAL_ERR;
} /* qcril_uim_ignore_change_subscription */


/*===========================================================================

  FUNCTION:  qcril_uim_fill_change_session_params

===========================================================================*/
/*!
    @brief
    Updates the passed provisioning session parameters by checking the input
    QCRIL request with our global data. Main paramters to be looked for are
    session_type and aid since aid is not passed by the RIL interface.

    @return
    RIL_UIM_Errno error code
*/
/*=========================================================================*/
static RIL_UIM_Errno qcril_uim_fill_change_session_params
(
  const qcril_uim_uicc_subs_info_type         * RIL_uicc_subs_info_ptr,
  qmi_uim_change_prov_session_params_type     * session_params_ptr
)
{
  int              RIL_slot      = 0;
  int              RIL_app_index = 0;
  qmi_uim_app_type app_type      = QMI_UIM_APP_UNKNOWN;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  /* Sanity checks */
  if ((RIL_uicc_subs_info_ptr == NULL) || (session_params_ptr == NULL))
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return RIL_UIM_E_INTERNAL_ERR;
  }

  /* Verify request type */
  if (RIL_uicc_subs_info_ptr->uicc_subs_info.act_status != UIM_UICC_SUBSCRIPTION_ACTIVATE &&
      RIL_uicc_subs_info_ptr->uicc_subs_info.act_status != UIM_UICC_SUBSCRIPTION_DEACTIVATE)
  {
    QCRIL_LOG_ERROR("Invalid Activation status request : 0x%x",
                                RIL_uicc_subs_info_ptr->uicc_subs_info.act_status);
    return RIL_UIM_E_INTERNAL_ERR;
  }

  RIL_slot = RIL_uicc_subs_info_ptr->uicc_subs_info.slot;
  RIL_app_index = RIL_uicc_subs_info_ptr->uicc_subs_info.app_index;

  /* Check slot number */
  if (RIL_slot >= QMI_UIM_MAX_CARD_COUNT ||
      RIL_slot < 0)
  {
    QCRIL_LOG_ERROR("Invalid slot value: 0x%x", RIL_slot);
    return RIL_UIM_E_INTERNAL_ERR;
  }

  /* Check app index */
  if ((RIL_app_index < 0) || (RIL_app_index >= QMI_UIM_MAX_APP_PER_CARD_COUNT))
  {
    QCRIL_LOG_ERROR("Invalid app index: 0x%x", RIL_app_index);
    return RIL_UIM_E_INTERNAL_ERR;
  }

  /* Verify card status */
  if (qcril_uim.card_status.card[RIL_slot].card_state != QMI_UIM_CARD_STATE_PRESENT)
  {
    QCRIL_LOG_ERROR("Card is not present");
    return RIL_UIM_E_INTERNAL_ERR;
  }

  /* During an activation request, we expect that it is currently not activated
     on any application, hence we cannot check card_status.index_gw_pri_prov or
     Deactivation received before card status indication with app activation info */
  app_type = qcril_uim.card_status.card[RIL_slot].application[RIL_app_index].app_type;

  if (RIL_uicc_subs_info_ptr->uicc_subs_info.act_status == UIM_UICC_SUBSCRIPTION_ACTIVATE)
  {
    uint8 aid_len = qcril_uim.card_status.card[RIL_slot].application[RIL_app_index].aid_len;

    /* For activation request, we need to send the aid & slot also */
    if (RIL_slot == 0)
    {
      session_params_ptr->app_info.slot = QMI_UIM_SLOT_1;
    }
    else if (RIL_slot == 1)
    {
      session_params_ptr->app_info.slot = QMI_UIM_SLOT_2;
    }
    else
    {
      session_params_ptr->app_info.slot = QMI_UIM_SLOT_3;
    }

    if ((aid_len > 0) && (aid_len <= QMI_UIM_MAX_AID_LEN))
    {
      /* We pass AID only if requested to accomodate SIM/RUIM app_type */
      session_params_ptr->app_info.aid.data_ptr =
          (unsigned char*)qcril_uim.card_status.card[RIL_slot].application[RIL_app_index].aid_value;
      session_params_ptr->app_info.aid.data_len = aid_len;
    }
  }

  return qcril_uim_find_prov_session_type(session_params_ptr, app_type,
                                          RIL_uicc_subs_info_ptr->uicc_subs_info.sub_type,
                                          RIL_uicc_subs_info_ptr->subs_mode_pref,
                                          RIL_uicc_subs_info_ptr->uicc_subs_info.act_status);
} /* qcril_uim_fill_change_session_params */


/*===========================================================================

  FUNCTION:  qcril_uim_update_prov_app_index

===========================================================================*/
/*!
    @brief
    Checks if the passed provisioning app index belongs to the input slot
    requested for. If yes, respective index variables - GSM/UMTS or CDMA of
    RIL_CardStatus_v6 are updated based on the type of the provisioning app.

    @return
    None.

*/
/*=========================================================================*/
static void qcril_uim_update_prov_app_index
(
  unsigned short        input_index,
  uint8                 input_slot,
  std::shared_ptr<RIL_UIM_CardStatus> ril_card_status_ptr,
  qmi_uim_session_type  session_type
)
{
  uint8 prov_app_index = QCRIL_UIM_INVALID_APP_INDEX_VALUE;
  uint8 prov_app_slot  = QCRIL_UIM_INVALID_SLOT_INDEX_VALUE;

  if (ril_card_status_ptr == NULL)
  {
    QCRIL_LOG_ERROR("NULL ril_card_status_ptr, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  prov_app_index = input_index & 0xFF;
  prov_app_slot  = (input_index >> 8) & 0xFF;

  /* Check if the passed prov app's slot matches with the card index */
  if ((input_slot == prov_app_slot) &&
      (prov_app_index < RIL_CARD_MAX_APPS))
  {
    switch(session_type)
    {
      case QMI_UIM_SESSION_TYPE_PRI_GW_PROV:
      case QMI_UIM_SESSION_TYPE_SEC_GW_PROV:
      case QMI_UIM_SESSION_TYPE_TER_GW_PROV:
        ril_card_status_ptr->gsm_umts_subscription_app_index = prov_app_index;
        break;

      case QMI_UIM_SESSION_TYPE_PRI_1X_PROV:
      case QMI_UIM_SESSION_TYPE_SEC_1X_PROV:
      case QMI_UIM_SESSION_TYPE_TER_1X_PROV:
        ril_card_status_ptr->cdma_subscription_app_index = prov_app_index;
        break;

      default:
        QCRIL_LOG_DEBUG( "Unsupported prov_app_type - %d \n", session_type);
        break;
    }
  }
} /* qcril_uim_update_prov_app_index */


/*===========================================================================

  FUNCTION:  qcril_uim_convert_card_state

===========================================================================*/
/*!
    @brief
    Converts a qmi_uim_card_state_type to RIL_CardState. Mapped RIL_CardState
    is updated in the passed pointer.

    @return
    None.
*/
/*=========================================================================*/
static void qcril_uim_convert_card_state
(
  RIL_UIM_CardState         * ril_card_state_ptr,
  qmi_uim_card_state_type     qmi_card_state,
  qmi_uim_card_error_type     qmi_card_error
)
{
  if (ril_card_state_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  switch(qmi_card_state)
  {
    case QMI_UIM_CARD_STATE_ABSENT:
    case QMI_UIM_CARD_STATE_UNKNOWN:
      *ril_card_state_ptr = RIL_UIM_CARDSTATE_ABSENT;
      break;

    case QMI_UIM_CARD_STATE_PRESENT:
      *ril_card_state_ptr = RIL_UIM_CARDSTATE_PRESENT;
      break;

    case QMI_UIM_CARD_STATE_ERROR:
      if ((qmi_card_error == QMI_UIM_CARD_ERROR_NO_ATR_RECEIVED) ||
          (qmi_card_error == QMI_UIM_CARD_ERROR_UNKNOWN_REMOVED) ||
          (qmi_card_error == QMI_UIM_CARD_ERROR_POWER_DOWN))
      {
        *ril_card_state_ptr = RIL_UIM_CARDSTATE_ABSENT;
      }
      else
      {
        *ril_card_state_ptr = RIL_UIM_CARDSTATE_ERROR;
      }
      break;

    default:
      *ril_card_state_ptr = RIL_UIM_CARDSTATE_ERROR;
      break;
  }
} /* qcril_uim_convert_card_state */


/*===========================================================================

  FUNCTION:  qcril_uim_convert_pin_state

===========================================================================*/
/*!
    @brief
    Converts a qmi_uim_pin_status_type to RIL_PinState. Mapped RIL_PinState
    is updated in the passed pointer.

    @return
    None.
*/
/*=========================================================================*/
static void qcril_uim_convert_pin_state
(
  RIL_UIM_PinState          * ril_pin_state_ptr,
  qmi_uim_pin_status_type     qmi_pin_state
)
{
  if (ril_pin_state_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  switch(qmi_pin_state)
  {
    case QMI_UIM_PIN_STATE_ENABLED_NOT_VERIFIED:
      *ril_pin_state_ptr = RIL_UIM_PINSTATE_ENABLED_NOT_VERIFIED;
      break;

    case QMI_UIM_PIN_STATE_ENABLED_VERIFIED:
      *ril_pin_state_ptr = RIL_UIM_PINSTATE_ENABLED_VERIFIED;
      break;

    case QMI_UIM_PIN_STATE_DISABLED:
      *ril_pin_state_ptr = RIL_UIM_PINSTATE_DISABLED;
      break;

    case QMI_UIM_PIN_STATE_BLOCKED:
      *ril_pin_state_ptr = RIL_UIM_PINSTATE_ENABLED_BLOCKED;
      break;

    case QMI_UIM_PIN_STATE_PERM_BLOCKED:
      *ril_pin_state_ptr = RIL_UIM_PINSTATE_ENABLED_PERM_BLOCKED;
      break;

    case QMI_UIM_PIN_STATE_UNKNOWN:
    default:
      *ril_pin_state_ptr = RIL_UIM_PINSTATE_UNKNOWN;
      break;
  }
} /* qcril_uim_convert_pin_state */


/*===========================================================================

  FUNCTION:  qcril_uim_convert_app_type

===========================================================================*/
/*!
    @brief
    Converts a qmi_uim_app_type to RIL_AppType. Mapped RIL_AppType
    is updated in the passed pointer.

    @return
    None.
*/
/*=========================================================================*/
static void qcril_uim_convert_app_type
(
  RIL_UIM_AppType           * ril_app_type_ptr,
  qmi_uim_app_type            qmi_app_type
)
{
  if (ril_app_type_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  switch(qmi_app_type)
  {
    case QMI_UIM_APP_SIM:
      *ril_app_type_ptr = RIL_UIM_APPTYPE_SIM;
      break;

    case QMI_UIM_APP_USIM:
      *ril_app_type_ptr = RIL_UIM_APPTYPE_USIM;
      break;

    case QMI_UIM_APP_RUIM:
      *ril_app_type_ptr = RIL_UIM_APPTYPE_RUIM;
      break;

    case QMI_UIM_APP_CSIM:
      *ril_app_type_ptr = RIL_UIM_APPTYPE_CSIM;
      break;

    case QMI_UIM_APP_ISIM:
      *ril_app_type_ptr = RIL_UIM_APPTYPE_ISIM;
      break;

    case QMI_UIM_APP_UNKNOWN:
    default:
      *ril_app_type_ptr = RIL_UIM_APPTYPE_UNKNOWN;
      break;
  }
} /* qcril_uim_convert_app_type */


/*===========================================================================

  FUNCTION:  qcril_uim_convert_app_state_to_ril

===========================================================================*/
/*!
    @brief
    Converts a qmi_uim_app_state_type to RIL_AppState. Mapped RIL_AppState
    is updated in the passed pointer.

    @return
    None.
*/
/*=========================================================================*/
static void qcril_uim_convert_app_state_to_ril
(
  RIL_UIM_AppState          * ril_app_state_ptr,
  qmi_uim_app_state_type      qmi_app_state
)
{
  if (ril_app_state_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  switch(qmi_app_state)
  {

    case QMI_UIM_APP_STATE_DETECTED:
      *ril_app_state_ptr = RIL_UIM_APPSTATE_DETECTED;
      break;

    case QMI_UIM_APP_STATE_PIN_REQUIRED:
      *ril_app_state_ptr = RIL_UIM_APPSTATE_PIN;
      break;

    case QMI_UIM_APP_STATE_PUK1_REQUIRED:
    case QMI_UIM_APP_STATE_BLOCKED:
      *ril_app_state_ptr = RIL_UIM_APPSTATE_PUK;
      break;

    case QMI_UIM_APP_STATE_PERSO:
      *ril_app_state_ptr = RIL_UIM_APPSTATE_SUBSCRIPTION_PERSO;
      break;

    case QMI_UIM_APP_STATE_ILLEGAL:
#ifdef RIL_UIM_APPSTATE_ILLEGAL
      *ril_app_state_ptr = RIL_UIM_APPSTATE_ILLEGAL;
#else
      *ril_app_state_ptr = RIL_UIM_APPSTATE_READY;
#endif /* RIL_APPSTATE_ILLEGAL */
      break;

    case QMI_UIM_APP_STATE_READY:
      *ril_app_state_ptr = RIL_UIM_APPSTATE_READY;
      break;

    case QMI_UIM_APP_STATE_UNKNOWN:
    default:
      *ril_app_state_ptr = RIL_UIM_APPSTATE_UNKNOWN;
      break;
  }
} /* qcril_uim_convert_app_state_to_ril */


/*===========================================================================

  FUNCTION:  qcril_uim_convert_app_state_to_qcril

===========================================================================*/
/*!
    @brief
    Converts a qmi_uim_app_state_type to qcril_sim_state_e_type. Mapped
    qcril_sim_state_e_type is updated in the passed pointer.

    @return
    None.
*/
/*=========================================================================*/
static void qcril_uim_convert_app_state_to_qcril
(
  qcril_sim_state_e_type    * qcril_sim_state_ptr,
  qmi_uim_app_state_type      qmi_app_state
)
{
  if (qcril_sim_state_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  switch(qmi_app_state)
  {
    case QMI_UIM_APP_STATE_PIN_REQUIRED:
      *qcril_sim_state_ptr = QCRIL_SIM_STATE_PIN;
      break;

    case QMI_UIM_APP_STATE_PUK1_REQUIRED:
      *qcril_sim_state_ptr = QCRIL_SIM_STATE_PUK;
      break;

    case QMI_UIM_APP_STATE_PERSO:
      *qcril_sim_state_ptr = QCRIL_SIM_STATE_NETWORK_PERSONALIZATION;
      break;

    case QMI_UIM_APP_STATE_BLOCKED:
      *qcril_sim_state_ptr = QCRIL_SIM_STATE_ABSENT;
      break;

    case QMI_UIM_APP_STATE_ILLEGAL:
      *qcril_sim_state_ptr = QCRIL_SIM_STATE_ILLEGAL;
      break;

    case QMI_UIM_APP_STATE_READY:
      *qcril_sim_state_ptr = QCRIL_SIM_STATE_READY;
      break;

    case QMI_UIM_APP_STATE_UNKNOWN:
    case QMI_UIM_APP_STATE_DETECTED:
    default:
      *qcril_sim_state_ptr = QCRIL_SIM_STATE_NOT_READY;
      break;
  }
} /* qcril_uim_convert_app_state_to_qcril */


/*===========================================================================

  FUNCTION:  qcril_uim_convert_perso_state_to_qcril

===========================================================================*/
/*!
    @brief
    Converts a qmi_uim_app_state_type to qcril_sim_state_e_type. Mapped
    qcril_sim_state_e_type is updated in the passed pointer.

    @return
    None.
*/
/*=========================================================================*/
static void qcril_uim_convert_perso_state_to_qcril
(
  qcril_sim_state_e_type    * qcril_sim_state_ptr,
  qmi_uim_perso_state_type    qmi_perso_state
)
{
  if (qcril_sim_state_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  switch(qmi_perso_state)
  {
    case QMI_UIM_PERSO_STATE_IN_PROGRESS:
      /* No change to sim state */
      break;

    case QMI_UIM_PERSO_STATE_READY:
      *qcril_sim_state_ptr = QCRIL_SIM_STATE_READY;
      break;

    /* Todo - check if there is any other code for other feature ids */
    case QMI_UIM_PERSO_STATE_CODE_REQUIRED:
      *qcril_sim_state_ptr = QCRIL_SIM_STATE_NETWORK_PERSONALIZATION;
      break;

    case QMI_UIM_PERSO_STATE_PUK_REQUIRED:
    case QMI_UIM_PERSO_STATE_PERM_BLOCKED:
      *qcril_sim_state_ptr = QCRIL_SIM_STATE_ABSENT;
      break;

    case QMI_UIM_PERSO_STATE_UNKNOWN:
    default:
      *qcril_sim_state_ptr = QCRIL_SIM_STATE_NOT_READY;
      break;
  }
} /* qcril_uim_convert_perso_state_to_qcril */


/*===========================================================================

  FUNCTION:  qcril_uim_convert_perso_state_code_required

===========================================================================*/
/*!
    @brief
    Converts qmi_uim_perso_feature_id_type to RIL_UIM_PersoSubstate for the
    cases where the codes for personalization codes are needed.

    @return
    Mapped RIL_UIM_PersoSubstate.
*/
/*=========================================================================*/
static RIL_UIM_PersoSubstate qcril_uim_convert_perso_state_code_required
(
  qmi_uim_perso_feature_status_type   qmi_perso_feature_status
)
{
  int perso_substate = RIL_UIM_PERSOSUBSTATE_UNKNOWN;

  switch(qmi_perso_feature_status)
  {
    case QMI_UIM_PERSO_FEATURE_STATUS_GW_NW:
      perso_substate = RIL_UIM_PERSOSUBSTATE_SIM_NETWORK;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_GW_NS:
      perso_substate = RIL_UIM_PERSOSUBSTATE_SIM_NETWORK_SUBSET;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_GW_SP:
      perso_substate = RIL_UIM_PERSOSUBSTATE_SIM_SERVICE_PROVIDER;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_GW_CP:
      perso_substate = RIL_UIM_PERSOSUBSTATE_SIM_CORPORATE;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_GW_SPN:
      perso_substate = RIL_UIM_PERSOSUBSTATE_SIM_SPN;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_GW_SP_EHPLMN:
      perso_substate = RIL_UIM_PERSOSUBSTATE_SIM_SP_EHPLMN;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_GW_ICCID:
      perso_substate = RIL_UIM_PERSOSUBSTATE_SIM_ICCID;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_GW_IMPI:
      perso_substate = RIL_UIM_PERSOSUBSTATE_SIM_IMPI;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_GW_SIM:
      perso_substate = RIL_UIM_PERSOSUBSTATE_SIM_SIM;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_GW_NS_SP:
      perso_substate = RIL_UIM_PERSOSUBSTATE_SIM_NS_SP;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_1X_NW1:
      perso_substate = RIL_UIM_PERSOSUBSTATE_RUIM_NETWORK1;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_1X_NW2:
      perso_substate = RIL_UIM_PERSOSUBSTATE_RUIM_NETWORK2;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_1X_HRPD:
      perso_substate = RIL_UIM_PERSOSUBSTATE_RUIM_HRPD;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_1X_SP:
      perso_substate = RIL_UIM_PERSOSUBSTATE_RUIM_SERVICE_PROVIDER;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_1X_CP:
      perso_substate = RIL_UIM_PERSOSUBSTATE_RUIM_CORPORATE;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_1X_RUIM:
      perso_substate = RIL_UIM_PERSOSUBSTATE_RUIM_RUIM;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_UNKNOWN:
    default:
      perso_substate = RIL_UIM_PERSOSUBSTATE_UNKNOWN;
      break;
  }

  return (RIL_UIM_PersoSubstate)perso_substate;
} /* qcril_uim_convert_perso_state_code_required */


/*===========================================================================

  FUNCTION:  qcril_uim_convert_perso_state_puk_required

===========================================================================*/
/*!
    @brief
    Converts qmi_uim_perso_feature_id_type to RIL_UIM_PERSOSUBSTATE for the
    cases where PUK for personalization codes are needed.

    @return
    Mapped RIL_UIM_PersoSubstate.
*/
/*=========================================================================*/
static RIL_UIM_PersoSubstate qcril_uim_convert_perso_state_puk_required
(
  qmi_uim_perso_feature_status_type   qmi_perso_feature_status
)
{
  int perso_substate = RIL_UIM_PERSOSUBSTATE_UNKNOWN;

  switch(qmi_perso_feature_status)
  {
    case QMI_UIM_PERSO_FEATURE_STATUS_GW_NW:
      perso_substate = RIL_UIM_PERSOSUBSTATE_SIM_NETWORK_PUK;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_GW_NS:
      perso_substate = RIL_UIM_PERSOSUBSTATE_SIM_NETWORK_SUBSET_PUK;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_GW_SP:
      perso_substate = RIL_UIM_PERSOSUBSTATE_SIM_SERVICE_PROVIDER_PUK;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_GW_CP:
      perso_substate = RIL_UIM_PERSOSUBSTATE_SIM_CORPORATE_PUK;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_GW_SPN:
      perso_substate = RIL_UIM_PERSOSUBSTATE_SIM_SPN_PUK;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_GW_SP_EHPLMN:
      perso_substate = RIL_UIM_PERSOSUBSTATE_SIM_SP_EHPLMN_PUK;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_GW_ICCID:
      perso_substate = RIL_UIM_PERSOSUBSTATE_SIM_ICCID_PUK;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_GW_IMPI:
      perso_substate = RIL_UIM_PERSOSUBSTATE_SIM_IMPI_PUK;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_GW_SIM:
      perso_substate = RIL_UIM_PERSOSUBSTATE_SIM_SIM_PUK;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_GW_NS_SP:
      perso_substate = RIL_UIM_PERSOSUBSTATE_SIM_NS_SP_PUK;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_1X_NW1:
      perso_substate = RIL_UIM_PERSOSUBSTATE_RUIM_NETWORK1_PUK;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_1X_NW2:
      perso_substate = RIL_UIM_PERSOSUBSTATE_RUIM_NETWORK2_PUK;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_1X_HRPD:
      perso_substate = RIL_UIM_PERSOSUBSTATE_RUIM_HRPD_PUK;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_1X_SP:
      perso_substate = RIL_UIM_PERSOSUBSTATE_RUIM_SERVICE_PROVIDER_PUK;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_1X_CP:
      perso_substate = RIL_UIM_PERSOSUBSTATE_RUIM_CORPORATE_PUK;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_1X_RUIM:
      perso_substate = RIL_UIM_PERSOSUBSTATE_RUIM_RUIM_PUK;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_UNKNOWN:
    default:
      perso_substate = RIL_UIM_PERSOSUBSTATE_UNKNOWN;
      break;
  }

  return (RIL_UIM_PersoSubstate)perso_substate;
} /* qcril_uim_convert_perso_state_puk_required */


/*===========================================================================

  FUNCTION:  qcril_uim_convert_perso_feature_status_to_perso_feature_id

===========================================================================*/
/*!
    @brief
    Converts personalization feature status to personalization feature ID

    @return
    None
*/
/*=========================================================================*/
static boolean qcril_uim_convert_perso_feature_status_to_perso_feature_id
(
  qmi_uim_perso_feature_status_type   perso_feature_status,
  qmi_uim_perso_feature_id_type      *perso_feature_id_ptr
)
{
  boolean status = TRUE;

  if(perso_feature_id_ptr == NULL)
  {
    return FALSE;
  }

  switch(perso_feature_status)
  {
    case QMI_UIM_PERSO_FEATURE_STATUS_GW_NW:
      *perso_feature_id_ptr = QMI_UIM_PERSO_FEATURE_GW_NW;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_GW_NS:
      *perso_feature_id_ptr = QMI_UIM_PERSO_FEATURE_GW_NS;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_GW_SP:
      *perso_feature_id_ptr = QMI_UIM_PERSO_FEATURE_GW_SP;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_GW_CP:
      *perso_feature_id_ptr = QMI_UIM_PERSO_FEATURE_GW_CP;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_GW_SIM:
      *perso_feature_id_ptr = QMI_UIM_PERSO_FEATURE_GW_SIM;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_1X_NW1:
      *perso_feature_id_ptr = QMI_UIM_PERSO_FEATURE_1X_NW1;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_1X_NW2:
      *perso_feature_id_ptr = QMI_UIM_PERSO_FEATURE_1X_NW2;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_1X_HRPD:
      *perso_feature_id_ptr = QMI_UIM_PERSO_FEATURE_1X_HRPD;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_1X_SP:
      *perso_feature_id_ptr = QMI_UIM_PERSO_FEATURE_1X_SP;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_1X_CP:
      *perso_feature_id_ptr = QMI_UIM_PERSO_FEATURE_1X_CP;
      break;
     case QMI_UIM_PERSO_FEATURE_STATUS_1X_RUIM:
      *perso_feature_id_ptr = QMI_UIM_PERSO_FEATURE_1X_RUIM;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_GW_SPN:
      *perso_feature_id_ptr = QMI_UIM_PERSO_FEATURE_GW_SPN;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_GW_SP_EHPLMN:
      *perso_feature_id_ptr = QMI_UIM_PERSO_FEATURE_GW_SP_EHPLMN;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_GW_ICCID:
      *perso_feature_id_ptr = QMI_UIM_PERSO_FEATURE_GW_ICCID;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_GW_IMPI:
      *perso_feature_id_ptr = QMI_UIM_PERSO_FEATURE_GW_IMPI;
      break;
    case QMI_UIM_PERSO_FEATURE_STATUS_GW_NS_SP:
      *perso_feature_id_ptr = QMI_UIM_PERSO_FEATURE_GW_NS_SP;
      break;
    default:
      status = FALSE;
      break;
  }

  return status;
} /* qcril_uim_convert_perso_feature_status_to_perso_feature_id */


/*===========================================================================

  FUNCTION:  qcril_uim_convert_perso_state_all

===========================================================================*/
/*!
    @brief
    Converts a qmi_uim_app_state_type to RIL_UIM_PERSOSUBSTATE. Mapped
    RIL_UIM_PERSOSUBSTATE is updated in the passed pointer.

    @return
    None.
*/
/*=========================================================================*/
static void qcril_uim_convert_perso_state_all
(
  RIL_UIM_PersoSubstate             * ril_perso_substate_ptr,
  qmi_uim_perso_state_type            qmi_perso_state,
  qmi_uim_perso_feature_status_type   qmi_perso_feature_status
)
{
  uint8                                  i                 = 0;
  int                                    ret    = 0;
  qmi_uim_get_configuration_params_type  get_config_params;
  qmi_uim_rsp_data_type                  rsp_data;

  memset(&get_config_params, 0x00, sizeof(get_config_params));
  memset(&rsp_data, 0x00, sizeof(rsp_data));

  if (ril_perso_substate_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  switch(qmi_perso_state)
  {
    case QMI_UIM_PERSO_STATE_IN_PROGRESS:
      *ril_perso_substate_ptr = RIL_UIM_PERSOSUBSTATE_IN_PROGRESS;
      break;

    case QMI_UIM_PERSO_STATE_READY:
      *ril_perso_substate_ptr = RIL_UIM_PERSOSUBSTATE_READY;
      break;

    case QMI_UIM_PERSO_STATE_CODE_REQUIRED:
    case QMI_UIM_PERSO_STATE_PUK_REQUIRED:
    case QMI_UIM_PERSO_STATE_PERM_BLOCKED:
      get_config_params.perso_status = QMI_UIM_TRUE;
      ret = qcril_uim_send_qmi_sync_msg(QCRIL_UIM_REQUEST_GET_CONFIGURATION,
                                        &get_config_params,
                                        &rsp_data);
      if (ret == 0)
      {
        boolean                       perso_convert_status = FALSE;
        qmi_uim_perso_feature_id_type perso_feature_id     = QMI_UIM_PERSO_FEATURE_GW_NW;

        perso_convert_status =
          qcril_uim_convert_perso_feature_status_to_perso_feature_id(
                                 qmi_perso_feature_status,
                                 &perso_feature_id);

        if(perso_convert_status)
        {
          /* If control key is not present then send perso status as SIM_RSU.
             This makes sure to not show the SIMLOCK pop up on UI asking control key */
          for(i = 0; i < QMI_UIM_MAX_PERSO_FEATURES; i++)
          {
            if(rsp_data.rsp_data.get_configuration_rsp.perso_status[i].feature == perso_feature_id)
            {
              if (!rsp_data.rsp_data.get_configuration_rsp.perso_status[i].ck_present)
              {
                *ril_perso_substate_ptr = (RIL_UIM_PersoSubstate)RIL_UIM_PERSOSUBSTATE_SIM_RSU_NO_CK;
                return;
              }
              break;
            }
          }
        }
      }

      if(qmi_perso_state == QMI_UIM_PERSO_STATE_CODE_REQUIRED)
      {
        *ril_perso_substate_ptr =
          qcril_uim_convert_perso_state_code_required(qmi_perso_feature_status);
      }
      else
      {
        *ril_perso_substate_ptr =
          qcril_uim_convert_perso_state_puk_required(qmi_perso_feature_status);
      }
      break;

    case QMI_UIM_PERSO_STATE_UNKNOWN:
    default:
      *ril_perso_substate_ptr = RIL_UIM_PERSOSUBSTATE_UNKNOWN;
      break;
  }
} /* qcril_uim_convert_perso_state_all */


/*===========================================================================

  FUNCTION:  qcril_uim_add_aid_info

===========================================================================*/
/*!
    @brief
    Updates the passed RIL AID pointer with AID data if it has been received
    from QMI, does nothing otherwise.

    @return
    None.
*/
/*=========================================================================*/
static void qcril_uim_add_aid_info
(
  char             ** ril_aid_ptr,
  const char       *  qmi_aid_ptr,
  unsigned char       qmi_aid_len
)
{
  if ((ril_aid_ptr == NULL) || (qmi_aid_ptr == NULL))
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  /* Convert the AID in hex to a null terminated ASCII string
     The function qcril_uim_alloc_bin_to_hexstring will perform the allocation & data conversion */
  *ril_aid_ptr = qcril_uim_alloc_bin_to_hexstring((const uint8*)qmi_aid_ptr,
                                                  qmi_aid_len);

  if (*ril_aid_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "Unable to allocate aid_ptr\n");
  }
} /* qcril_uim_add_aid_info */


/*===========================================================================

  FUNCTION:  qcril_uim_add_label_info

===========================================================================*/
/*!
    @brief
    Updates the passed RIL Label pointer with label data if it has been
    received from QMI, does nothing otherwise.

    @return
    None.
*/
/*=========================================================================*/
static void qcril_uim_add_label_info
(
  string                             & ril_label,
  qmi_uim_get_label_rsp_type         * qmi_label_rsp_ptr
)
{
  char * ril_label_ptr = NULL;

  /* Sanity check */
  if ((qmi_label_rsp_ptr == NULL) ||
      (qmi_label_rsp_ptr->label_len > QMI_UIM_MAX_LABEL_LEN))
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  /* Print label */
  QCRIL_LOG_DEBUG("Raw label value:\n");
  for (int i = 0; i < qmi_label_rsp_ptr->label_len ; i++)
  {
    QCRIL_LOG_DEBUG(" 0X%x", qmi_label_rsp_ptr->label_value[i]);
  }

  /* Convert the Label in hex to a null terminated ASCII string
     The function qcril_uim_alloc_bin_to_hexstring will perform the allocation & data conversion */
  ril_label_ptr = qcril_uim_alloc_bin_to_hexstring((const uint8*)qmi_label_rsp_ptr->label_value,
                                                     qmi_label_rsp_ptr->label_len);

  if (ril_label_ptr != NULL)
  {
    ril_label = ril_label_ptr;
    qcril_free(ril_label_ptr);
  }
} /* qcril_uim_add_label_info */


/*===========================================================================

  FUNCTION:  qcril_uim_convert_pin1_replaced_state

===========================================================================*/
/*!
    @brief
    Updates the passed pointer to indicate if PIN1 is replaced with UPIN.

    @return
    None.
*/
/*=========================================================================*/
static void qcril_uim_convert_pin1_replaced_state
(
  int              * ril_pin1_replaced_ptr,
  unsigned char      qmi_univ_pin
)
{
  if (ril_pin1_replaced_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  if(qmi_univ_pin == 0)
  {
    *ril_pin1_replaced_ptr = 0;
  }
  else if(qmi_univ_pin == 1)
  {
    *ril_pin1_replaced_ptr = 1;
  }
} /* qcril_uim_convert_pin1_replaced_state */


/*===========================================================================

  FUNCTION:  qcril_uim_construct_card_status

===========================================================================*/
/*!
    @brief
    Constructs the RIL card status from our internal QMI card status type.
    Also, an appropriate card state name is filled.

    @return
    None.
*/
/*=========================================================================*/
static void qcril_uim_construct_card_status
(
  std::shared_ptr<RIL_UIM_CardStatus>    ril_card_status_ptr,
  const qmi_uim_card_status_type       * card_status_ptr,
  uint8                                  slot
)
{
  int                   app_index     = 0;
  int                   max_apps      = 0;
  uint8                 isim_index    = QCRIL_UIM_INVALID_APP_INDEX_VALUE;
  qmi_uim_rsp_data_type get_label_rsp = {};

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if ((ril_card_status_ptr == NULL) || (card_status_ptr == NULL))
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  if(slot >= QMI_UIM_MAX_CARD_COUNT)
  {
    QCRIL_LOG_ERROR("Invalid slot index 0x%x", slot);
    return;
  }

  /* Update physical slot id */
  ril_card_status_ptr->physical_slot_id = card_status_ptr->card[slot].physical_slot_id;

  /* Update Card state */
  qcril_uim_convert_card_state(&ril_card_status_ptr->card_state,
                               card_status_ptr->card[slot].card_state,
                               card_status_ptr->card[slot].card_error);

  /* Update ICCID and ATR if card state is card present */
  if (ril_card_status_ptr->card_state == RIL_UIM_CARDSTATE_PRESENT)
  {
    char *atr   = NULL;
    char *iccid = NULL;
    char *eid   = NULL;

    iccid = qcril_uim_util_iccid_to_string(
           card_status_ptr->card[slot].iccid,
           card_status_ptr->card[slot].iccid_len);

    if (iccid != NULL)
    {
      ril_card_status_ptr->iccid = iccid;
      qcril_free(iccid);
    }

    atr = qcril_uim_alloc_bin_to_hexstring(
           card_status_ptr->card[slot].atr,
           card_status_ptr->card[slot].atr_len);

    if (atr != NULL)
    {
      ril_card_status_ptr->atr = atr;
      qcril_free(atr);
    }

    eid = qcril_uim_alloc_bin_to_hexstring(
       card_status_ptr->card[slot].eid,
       card_status_ptr->card[slot].eid_len);

    if (eid != NULL)
    {
      ril_card_status_ptr->eid = eid;
      qcril_free(eid);
    }
  }

  /* Update UPIN state */
  qcril_uim_convert_pin_state(&ril_card_status_ptr->universal_pin_state,
                              card_status_ptr->card[slot].upin_state);

  /* Init App indexes */
  ril_card_status_ptr->gsm_umts_subscription_app_index = -1;
  ril_card_status_ptr->cdma_subscription_app_index     = -1;
  ril_card_status_ptr->ims_subscription_app_index      = -1;

  /* Update App indexes */
  qcril_uim_update_prov_app_index(card_status_ptr->index_gw_pri_prov, slot,
                                  ril_card_status_ptr, QMI_UIM_SESSION_TYPE_PRI_GW_PROV);
  qcril_uim_update_prov_app_index(card_status_ptr->index_gw_sec_prov, slot,
                                  ril_card_status_ptr, QMI_UIM_SESSION_TYPE_SEC_GW_PROV);
  qcril_uim_update_prov_app_index(card_status_ptr->index_gw_ter_prov, slot,
                                  ril_card_status_ptr, QMI_UIM_SESSION_TYPE_TER_GW_PROV);
  qcril_uim_update_prov_app_index(card_status_ptr->index_1x_pri_prov, slot,
                                  ril_card_status_ptr, QMI_UIM_SESSION_TYPE_PRI_1X_PROV);
  qcril_uim_update_prov_app_index(card_status_ptr->index_1x_sec_prov, slot,
                                  ril_card_status_ptr, QMI_UIM_SESSION_TYPE_SEC_1X_PROV);
  qcril_uim_update_prov_app_index(card_status_ptr->index_1x_ter_prov, slot,
                                  ril_card_status_ptr, QMI_UIM_SESSION_TYPE_TER_1X_PROV);

  /* Update ISIM index if applicable */
  if ((qcril_uim_extract_isim_index(&isim_index, slot) == RIL_UIM_E_SUCCESS) &&
      (isim_index < RIL_CARD_MAX_APPS))
  {
    ril_card_status_ptr->ims_subscription_app_index = isim_index;
  }

  max_apps = (card_status_ptr->card[slot].num_app <= RIL_CARD_MAX_APPS)
              ? card_status_ptr->card[slot].num_app : RIL_CARD_MAX_APPS;

  ril_card_status_ptr->num_applications = max_apps;

  for (app_index = 0; app_index < max_apps; app_index++)
  {
    qcril_uim_convert_app_type(&ril_card_status_ptr->applications[app_index].app_type,
                               card_status_ptr->card[slot].application[app_index].app_type);
    qcril_uim_convert_app_state_to_ril(&ril_card_status_ptr->applications[app_index].app_state,
                                       card_status_ptr->card[slot].application[app_index].app_state);

    /* Move the ISIM app state to Ready if the following conditions satisfy:
       1) App state is DETECTED
       2) PIN1 state is either DISABLED or ENABLED & VERIFIED */
    if ((card_status_ptr->card[slot].application[app_index].app_type == QMI_UIM_APP_ISIM) &&
        (card_status_ptr->card[slot].application[app_index].app_state == QMI_UIM_APP_STATE_DETECTED) &&
        ((card_status_ptr->card[slot].application[app_index].pin1_state == QMI_UIM_PIN_STATE_DISABLED) ||
         (card_status_ptr->card[slot].application[app_index].pin1_state == QMI_UIM_PIN_STATE_ENABLED_VERIFIED)))
    {
      ril_card_status_ptr->applications[app_index].app_state = RIL_UIM_APPSTATE_READY;
    }
    qcril_uim_convert_perso_state_all(&ril_card_status_ptr->applications[app_index].perso_substate,
                                      card_status_ptr->card[slot].application[app_index].perso_state,
                                      card_status_ptr->card[slot].application[app_index].perso_feature);

    if (card_status_ptr->card[slot].application[app_index].aid_len <= QMI_UIM_MAX_AID_LEN)
    {
      char *aid_ptr = NULL;

      qcril_uim_add_aid_info(&aid_ptr,
                             card_status_ptr->card[slot].application[app_index].aid_value,
                             card_status_ptr->card[slot].application[app_index].aid_len);

      if (aid_ptr != NULL)
      {
        ril_card_status_ptr->applications[app_index].aid_ptr = aid_ptr;
        qcril_free(aid_ptr);
      }
    }
    /* Note - Sync QMI call for fetching app_label_ptr data, supposed to be fetched
     *        directly from QMI cache */
    if (qcril_uim_send_get_label(slot,
                                 &get_label_rsp,
                                 card_status_ptr->card[slot].application[app_index].aid_value,
                                 card_status_ptr->card[slot].application[app_index].aid_len) == 0)
    {
      qcril_uim_add_label_info(ril_card_status_ptr->applications[app_index].app_label_ptr,
                               &get_label_rsp.rsp_data.get_label_rsp);
    }
    qcril_uim_convert_pin1_replaced_state(&ril_card_status_ptr->applications[app_index].pin1_replaced,
                                          card_status_ptr->card[slot].application[app_index].univ_pin);
    qcril_uim_convert_pin_state(&ril_card_status_ptr->applications[app_index].pin1,
                                card_status_ptr->card[slot].application[app_index].pin1_state);
#ifdef FEATURE_QCRIL_UIM_QMI_UPIN
    /* Workaround to send state of UPIN in PIN1 variable if UPIN is enabled,
       until UI support for UPIN is available */
    if (card_status_ptr->card[slot].application[app_index].univ_pin == QCRIL_UIM_UPIN_STATE_REPLACES_PIN1)
    {
      /* App's PIN1 is replaced by UPIN */
      qcril_uim_convert_pin_state(&ril_card_status_ptr->applications[app_index].pin1,
                                  card_status_ptr->card[slot].upin_state);
    }
#endif /* FEATURE_QCRIL_UIM_QMI_UPIN */
    qcril_uim_convert_pin_state(&ril_card_status_ptr->applications[app_index].pin2,
                                card_status_ptr->card[slot].application[app_index].pin2_state);
  }
} /* qcril_uim_construct_card_status */


/*===========================================================================

  FUNCTION:  qcril_uim_update_cm_card_status

===========================================================================*/
/*!
    @brief
    Update QCRIL(CM) card status per UIM card state.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_update_cm_card_status
(
  int                          slot,
  qcril_uim_card_status_e_type new_card_status
)
{
  qcril_uim_card_info_type card_info;

  card_info.slot = slot;
  card_info.status = new_card_status;

  std::shared_ptr<UimCardStatusIndMsg> ind_msg_ptr =
       std::make_shared<UimCardStatusIndMsg>(card_info);
  if (ind_msg_ptr != nullptr)
  {
    ind_msg_ptr->broadcast();
  }
  else
  {
    QCRIL_LOG_ERROR( "Slot %d CM_CARD_STATUS_UPDATED Failed!\n", slot );
  }
} /* qcril_uim_update_cm_card_status() */


/*===========================================================================

  FUNCTION:  qcril_uim_card_state_timer_cb

===========================================================================*/
/*!
    @brief
    Timer callback for the threshold time that was specified. If this timer
    expires, it means that ATEL didnt call the get_sim_status after the unsol
    was sent. Since this is the max QCRIL UIM can wait, we need to fetch the
    latest card status from the modem & proceed as normal.

    @return
    None
*/
/*=========================================================================*/
static void qcril_uim_card_state_timer_cb
(
  void*  param_ptr
)
{
  int                    qmi_err_code = 0;
  qmi_uim_rsp_data_type  rsp_data;

  /*param_ptr doesn't contain any address but the TimerCancelReason, DON'T de-reference it */
  long reasonVal = reinterpret_cast<long>(param_ptr);
  UimTimerCancelReason reason =  (UimTimerCancelReason)reasonVal;
  param_ptr = NULL;

  /* Get card status to get the latest card info */
  QCRIL_LOG_INFO("card state check timer expired reason: %d",(int)reason);

  /* Clean up timer info before returning */
  qcril_uim_card_state_check_timer_msg_ptr = NULL;

  if(reason != TIMER_CANCEL_REASON_MODEM_SSR)
  {
      /* Get card status to get the latest status */
      memset(&rsp_data, 0x00, sizeof(rsp_data));
      qmi_err_code = qcril_uim_send_qmi_sync_msg(QCRIL_UIM_REQUEST_GET_CARD_STATUS,
                                                 nullptr,
                                                 &rsp_data);
      if (qmi_err_code != 0)
      {
          QCRIL_LOG_ERROR( "Get card status failed, qmi_err_code: 0x%x\n", qmi_err_code);
      }
      else
      {
          qmi_uim_card_init_callback(&rsp_data);
      }
  }
} /* qcril_uim_card_state_timer_cb */


/*===========================================================================

  FUNCTION:  qcril_uim_skip_card_indication

===========================================================================*/
/*!
    @brief
    This function checks the card state transition from/to error/absent and
    present. If the new state is:
    * If new status is Card Error or absent, we setup a timed callback
    * If the next transition is card present, we check if the timer is still
      running (below the threshold) Timer is running if ATEL has not read
      the status or it hasnt expired. If so, we skip processing that indication.
      The idea is that we want to make sure that ATEL get a chance to get the
      card error/absent status within the reasonable threshold that we set of
      500 msecs. When the timer expires or soon after the card error/absent is
      read by ATEL, we issue a qmi get card status & send the UNSOL to ATEL.
      The side effect of this approach is that potentially QCRIL could have stale
      data for a max time period of 500 msecs.

    @return
    TRUE - if indication needs to be skipped
    FALSE - if indication needs to be stored
*/
/*=========================================================================*/
static boolean qcril_uim_skip_card_indication
(
  const qmi_uim_card_status_type * old_status_ptr,
  const qmi_uim_card_status_type * new_status_ptr,
  uint8                            slot
)
{
  /* Sanity checks */
  if ((old_status_ptr == NULL) ||
      (new_status_ptr == NULL) ||
      (slot >= QMI_UIM_MAX_CARD_COUNT))
  {
    return FALSE;
  }

  /* If card state is same and time is running, we need to skip this indication.
     Upon timer expiration, new card status is retrieved */
  if (old_status_ptr->card[slot].card_state == new_status_ptr->card[slot].card_state)
  {
    if (qcril_uim_card_state_check_timer_msg_ptr != NULL)
    {
      return TRUE;
    }
    return FALSE;
  }

  /* We are interested only in transition from error/absent to present & vice versa.
     Nothing needed for any other transitions, e.g. - unknown to error/absent/present */
  if ((new_status_ptr->card[slot].card_state == QMI_UIM_CARD_STATE_ERROR ||
       new_status_ptr->card[slot].card_state == QMI_UIM_CARD_STATE_ABSENT) &&
      (old_status_ptr->card[slot].card_state == QMI_UIM_CARD_STATE_PRESENT))
  {
    /* For card present to error/absent transition, we need to setup timer to
       ensure that ATEL has a chance to retrieve card error status before the
       card present indication is received from modem */
    if (qcril_uim_card_state_check_timer_msg_ptr == NULL)
    {
      qcril_uim_card_state_check_timer_msg_ptr =
        std::make_shared<UimTimerRequestMsg>(QCRIL_UIM_CARD_STATE_CHECK_TIME);

      QCRIL_LOG_INFO( "New UimTimerRequestMsg created \n");

      if (qcril_uim_card_state_check_timer_msg_ptr != NULL)
      {
        GenericCallback<int> cb(
            ([](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<int> responseDataPtr) -> void
        {
          void* timerCancelReasonPtr = nullptr;
          if (responseDataPtr) {
            int reason = *responseDataPtr;
            timerCancelReasonPtr = reinterpret_cast<void*>(reason);

            QCRIL_LOG_INFO( "UimTimerRequestMsg Cb called reason:%d CancelReasonPtr:%p\n",
                            reason, timerCancelReasonPtr);
          }
          auto timer_msg = 
          std::make_shared<UimTimerExpiryMsg>(qcril_uim_card_state_timer_cb, timerCancelReasonPtr);

          (void)solicitedMsg;
          (void)status;
          (void)responseDataPtr;

          if (timer_msg != NULL)
          {
            timer_msg->dispatch();
          }
          else
          {
            qcril_uim_card_state_timer_cb(timerCancelReasonPtr);
          }
        }));
        qcril_uim_card_state_check_timer_msg_ptr->setCallback(&cb);
        qcril_uim_card_state_check_timer_msg_ptr->dispatch();
      }
    }
  }
  else if ((new_status_ptr->card[slot].card_state == QMI_UIM_CARD_STATE_PRESENT) &&
           (old_status_ptr->card[slot].card_state == QMI_UIM_CARD_STATE_ERROR ||
            old_status_ptr->card[slot].card_state == QMI_UIM_CARD_STATE_ABSENT))
  {
    /* For card error/absent to present transition, check if timer is running.
       If timer is running, it means there was no get sim status request from
       ATEL and this IND should be skipped for this transition */
    if (qcril_uim_card_state_check_timer_msg_ptr != NULL)
    {
      return TRUE;
    }
  }
  return FALSE;
} /* qcril_uim_skip_card_indication */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_has_card_status_changed

===========================================================================*/
/*!
    @brief
    Based on the indication received, this function checks what part of card
    status has changed and returns accordingly.

    @return
    TRUE - if changed
    FALSE - if not changed
*/
/*=========================================================================*/
static boolean qcril_uim_has_card_status_changed
(
  const qmi_uim_card_status_type * card_status_ptr,
  const qmi_uim_card_status_type * status_change_ind_ptr,
  uint8                            slot
)
{
  int i = 0;

  /* Sanity checks */
  if ((card_status_ptr == NULL) || (status_change_ind_ptr == NULL) ||
      (slot >= QMI_UIM_MAX_CARD_COUNT))
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return FALSE;
  }

  if (card_status_ptr->index_gw_pri_prov != status_change_ind_ptr->index_gw_pri_prov)
  {
    QCRIL_LOG_DEBUG("GW primary index changed: 0x%x -> 0x%x\n",
                    card_status_ptr->index_gw_pri_prov,
                    status_change_ind_ptr->index_gw_pri_prov);
    if ((slot == ((status_change_ind_ptr->index_gw_pri_prov >> 8) & 0xFF)) ||
        (slot == ((card_status_ptr->index_gw_pri_prov >> 8) & 0xFF)))
    {
      return TRUE;
    }
  }

  if (card_status_ptr->index_1x_pri_prov != status_change_ind_ptr->index_1x_pri_prov)
  {
    QCRIL_LOG_DEBUG("1X primary index changed: 0x%x -> 0x%x\n",
                    card_status_ptr->index_1x_pri_prov,
                    status_change_ind_ptr->index_1x_pri_prov);
    if ((slot == ((status_change_ind_ptr->index_1x_pri_prov >> 8) & 0xFF)) ||
        (slot == ((card_status_ptr->index_1x_pri_prov >> 8) & 0xFF)))
    {
      return TRUE;
    }
  }

  if (card_status_ptr->index_gw_sec_prov != status_change_ind_ptr->index_gw_sec_prov)
  {
    QCRIL_LOG_DEBUG("GW secondary index changed: 0x%x -> 0x%x\n",
                    card_status_ptr->index_gw_sec_prov,
                    status_change_ind_ptr->index_gw_sec_prov);
    if ((slot == ((status_change_ind_ptr->index_gw_sec_prov >> 8) & 0xFF)) ||
        (slot == ((card_status_ptr->index_gw_sec_prov >> 8) & 0xFF)))
    {
      return TRUE;
    }
  }

  if (card_status_ptr->index_1x_sec_prov != status_change_ind_ptr->index_1x_sec_prov)
  {
    QCRIL_LOG_DEBUG("1X secondary index changed: 0x%x -> 0x%x\n",
                    card_status_ptr->index_1x_sec_prov,
                    status_change_ind_ptr->index_1x_sec_prov);
    if ((slot == ((status_change_ind_ptr->index_1x_sec_prov >> 8) & 0xFF)) ||
        (slot == ((card_status_ptr->index_1x_sec_prov >> 8) & 0xFF)))
    {
      return TRUE;
    }
  }

  if (status_change_ind_ptr->card_status_type == QMI_UIM_EXTENDED_CARD_STATUS_TYPE &&
      card_status_ptr->index_gw_ter_prov != status_change_ind_ptr->index_gw_ter_prov)
  {
    QCRIL_LOG_DEBUG("GW tertiary index changed: 0x%x -> 0x%x\n",
                    card_status_ptr->index_gw_ter_prov,
                    status_change_ind_ptr->index_gw_ter_prov);
    if ((slot == ((status_change_ind_ptr->index_gw_ter_prov >> 8) & 0xFF)) ||
        (slot == ((card_status_ptr->index_gw_ter_prov >> 8) & 0xFF)))
    {
      return TRUE;
    }
  }

  if (status_change_ind_ptr->card_status_type == QMI_UIM_EXTENDED_CARD_STATUS_TYPE &&
      card_status_ptr->index_1x_ter_prov != status_change_ind_ptr->index_1x_ter_prov)
  {
    QCRIL_LOG_DEBUG("1X tertiary index changed: 0x%x -> 0x%x\n",
                    card_status_ptr->index_1x_ter_prov,
                    status_change_ind_ptr->index_1x_ter_prov);
    if ((slot == ((status_change_ind_ptr->index_1x_ter_prov >> 8) & 0xFF)) ||
        (slot == ((card_status_ptr->index_1x_ter_prov >> 8) & 0xFF)))
    {
      return TRUE;
    }
  }

  if (card_status_ptr->card[slot].card_state != status_change_ind_ptr->card[slot].card_state)
  {
    QCRIL_LOG_DEBUG("card state changed: 0x%x -> 0x%x\n",
                    card_status_ptr->card[slot].card_state,
                    status_change_ind_ptr->card[slot].card_state);
    return TRUE;
  }

  if ((card_status_ptr->card[slot].card_state == QMI_UIM_CARD_STATE_ERROR) &&
      (card_status_ptr->card[slot].card_error != status_change_ind_ptr->card[slot].card_error))
  {
    QCRIL_LOG_DEBUG("card error changed: 0x%x -> 0x%x\n",
                    card_status_ptr->card[slot].card_error,
                    status_change_ind_ptr->card[slot].card_error);
    return TRUE;
  }

  if (card_status_ptr->card[slot].upin_state != status_change_ind_ptr->card[slot].upin_state)
  {
    QCRIL_LOG_DEBUG("UPIN state changed: 0x%x -> 0x%x\n",
                    card_status_ptr->card[slot].upin_state,
                    status_change_ind_ptr->card[slot].upin_state);
    return TRUE;
  }

  if (card_status_ptr->card[slot].upin_num_retries != status_change_ind_ptr->card[slot].upin_num_retries)
  {
    QCRIL_LOG_DEBUG("UPIN retries changed: 0x%x -> 0x%x\n",
                    card_status_ptr->card[slot].upin_num_retries,
                    status_change_ind_ptr->card[slot].upin_num_retries);
    return TRUE;
  }

  if (card_status_ptr->card[slot].upuk_num_retries != status_change_ind_ptr->card[slot].upuk_num_retries)
  {
    QCRIL_LOG_DEBUG("UPUK retries changed: 0x%x -> 0x%x\n",
                    card_status_ptr->card[slot].upuk_num_retries,
                    status_change_ind_ptr->card[slot].upuk_num_retries);
    return TRUE;
  }

  if (card_status_ptr->card[slot].num_app != status_change_ind_ptr->card[slot].num_app)
  {
    QCRIL_LOG_DEBUG("Number of applications changed: 0x%x -> 0x%x\n",
                    card_status_ptr->card[slot].num_app,
                    status_change_ind_ptr->card[slot].num_app);
    return TRUE;
  }

  for (i = 0; i < status_change_ind_ptr->card[slot].num_app &&
              i < QMI_UIM_MAX_APP_PER_CARD_COUNT; i++)
  {
    if (card_status_ptr->card[slot].application[i].app_type !=
        status_change_ind_ptr->card[slot].application[i].app_type)
    {
      QCRIL_LOG_DEBUG("App type changed: 0x%x -> 0x%x\n",
                      card_status_ptr->card[slot].application[i].app_type,
                      status_change_ind_ptr->card[slot].application[i].app_type);
      return TRUE;
    }

    if (card_status_ptr->card[slot].application[i].app_state !=
        status_change_ind_ptr->card[slot].application[i].app_state)
    {
      QCRIL_LOG_DEBUG("App state changed: 0x%x -> 0x%x\n",
                      card_status_ptr->card[slot].application[i].app_state,
                      status_change_ind_ptr->card[slot].application[i].app_state);
      return TRUE;
    }

    if (card_status_ptr->card[slot].application[i].perso_state !=
        status_change_ind_ptr->card[slot].application[i].perso_state)
    {
      QCRIL_LOG_DEBUG("Perso state changed: 0x%x -> 0x%x\n",
                      card_status_ptr->card[slot].application[i].perso_state,
                      status_change_ind_ptr->card[slot].application[i].perso_state);
      return TRUE;
    }

    if (card_status_ptr->card[slot].application[i].perso_feature !=
        status_change_ind_ptr->card[slot].application[i].perso_feature)
    {
      QCRIL_LOG_DEBUG("Perso feature changed: 0x%x -> 0x%x\n",
                      card_status_ptr->card[slot].application[i].perso_feature,
                      status_change_ind_ptr->card[slot].application[i].perso_feature);
      return TRUE;
    }

    if (card_status_ptr->card[slot].application[i].perso_retries !=
        status_change_ind_ptr->card[slot].application[i].perso_retries)
    {
      QCRIL_LOG_DEBUG("Perso retries changed: 0x%x -> 0x%x\n",
                      card_status_ptr->card[slot].application[i].perso_retries,
                      status_change_ind_ptr->card[slot].application[i].perso_retries);
      return TRUE;
    }

    if (card_status_ptr->card[slot].application[i].perso_unblock_retries !=
        status_change_ind_ptr->card[slot].application[i].perso_unblock_retries)
    {
      QCRIL_LOG_DEBUG("Perso unblock retries changed: 0x%x -> 0x%x\n",
                      card_status_ptr->card[slot].application[i].perso_unblock_retries,
                      status_change_ind_ptr->card[slot].application[i].perso_unblock_retries);
      return TRUE;
    }

    if (card_status_ptr->card[slot].application[i].aid_len !=
        status_change_ind_ptr->card[slot].application[i].aid_len)
    {
      QCRIL_LOG_DEBUG("App AID len changed: 0x%x -> 0x%x\n",
                      card_status_ptr->card[slot].application[i].aid_len,
                      status_change_ind_ptr->card[slot].application[i].aid_len);
      return TRUE;
    }

    if (status_change_ind_ptr->card[slot].application[i].aid_len <= QMI_UIM_MAX_AID_LEN &&
        memcmp(card_status_ptr->card[slot].application[i].aid_value,
               status_change_ind_ptr->card[slot].application[i].aid_value,
               status_change_ind_ptr->card[slot].application[i].aid_len) != 0)
    {
      QCRIL_LOG_DEBUG("App AID value changed\n");
      return TRUE;
    }

    if (card_status_ptr->card[slot].application[i].univ_pin !=
        status_change_ind_ptr->card[slot].application[i].univ_pin)
    {
      QCRIL_LOG_DEBUG("Upin flag changed: 0x%x -> 0x%x\n",
                      card_status_ptr->card[slot].application[i].univ_pin,
                      status_change_ind_ptr->card[slot].application[i].univ_pin);
      return TRUE;
    }

    if (card_status_ptr->card[slot].application[i].pin1_state !=
        status_change_ind_ptr->card[slot].application[i].pin1_state)
    {
      QCRIL_LOG_DEBUG("PIN1 changed: 0x%x -> 0x%x\n",
                      card_status_ptr->card[slot].application[i].pin1_state,
                      status_change_ind_ptr->card[slot].application[i].pin1_state);
      return TRUE;
    }

    if (card_status_ptr->card[slot].application[i].pin1_num_retries !=
        status_change_ind_ptr->card[slot].application[i].pin1_num_retries)
    {
      QCRIL_LOG_DEBUG("PIN1 retries changed: 0x%x -> 0x%x\n",
                      card_status_ptr->card[slot].application[i].pin1_num_retries,
                      status_change_ind_ptr->card[slot].application[i].pin1_num_retries);
      return TRUE;
    }

    if (card_status_ptr->card[slot].application[i].puk1_num_retries !=
        status_change_ind_ptr->card[slot].application[i].puk1_num_retries)
    {
      QCRIL_LOG_DEBUG("PUK1 retries changed: 0x%x -> 0x%x\n",
                      card_status_ptr->card[slot].application[i].puk1_num_retries,
                      status_change_ind_ptr->card[slot].application[i].puk1_num_retries);
      return TRUE;
    }

    if (card_status_ptr->card[slot].application[i].pin2_state !=
        status_change_ind_ptr->card[slot].application[i].pin2_state)
    {
      QCRIL_LOG_DEBUG("PIN2 changed: 0x%x -> 0x%x\n",
                      card_status_ptr->card[slot].application[i].pin2_state,
                      status_change_ind_ptr->card[slot].application[i].pin2_state);
      return TRUE;
    }

    if (card_status_ptr->card[slot].application[i].pin2_num_retries !=
        status_change_ind_ptr->card[slot].application[i].pin2_num_retries)
    {
      QCRIL_LOG_DEBUG("PIN2 retries changed: 0x%x -> 0x%x\n",
                      card_status_ptr->card[slot].application[i].pin2_num_retries,
                      status_change_ind_ptr->card[slot].application[i].pin2_num_retries);
      return TRUE;
    }

    if (card_status_ptr->card[slot].application[i].puk2_num_retries !=
        status_change_ind_ptr->card[slot].application[i].puk2_num_retries)
    {
      QCRIL_LOG_DEBUG("PUK2 retries changed: 0x%x -> 0x%x\n",
                      card_status_ptr->card[slot].application[i].puk2_num_retries,
                      status_change_ind_ptr->card[slot].application[i].puk2_num_retries);
      return TRUE;
    }
  }

  QCRIL_LOG_DEBUG("No card change\n");

  return FALSE;
} /* qcril_uim_has_card_status_changed */


/*===========================================================================

  FUNCTION:  qcril_uim_handle_card_state_change

===========================================================================*/
/*!
    @brief


    @return
    None.
*/
/*=========================================================================*/
static void qcril_uim_handle_card_state_change
(
  uint8                            slot,
  const qmi_uim_card_status_type * old_status_ptr,
  const qmi_uim_card_status_type * new_status_ptr
)
{
  qcril_sim_state_e_type          new_sim_state     = QCRIL_SIM_STATE_CARD_ERROR;
  qcril_uim_card_status_e_type    qcril_card_state  = QCRIL_CARD_STATUS_UP;
  uint8                           instance_slot     = qmi_ril_get_sim_slot();

  if ((old_status_ptr == NULL) ||
      (new_status_ptr == NULL))
  {
    QCRIL_LOG_ERROR("%s","Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  QCRIL_LOG_DEBUG("Card status: 0x%x -> 0x%x\n",
                  old_status_ptr->card[slot].card_state,
                  new_status_ptr->card[slot].card_state);

  /* clean up cached LPA profiles */
  if (instance_slot == slot)
  {
    qcril_uim_lpa_cleanup_cached_iccids();
  }

  qcril_uim_sap_process_card_status(new_status_ptr->card[slot].card_state,
                                    new_status_ptr->card[slot].card_error,
                                    slot);

  if (new_status_ptr->card[slot].card_state == QMI_UIM_CARD_STATE_PRESENT)
  {
    uint8 iccid_len = QCRIL_UIM_ICCID_LEN;
    uint8 iccid_data[QCRIL_UIM_ICCID_LEN];

    memset(iccid_data, 0x00, sizeof(iccid_data));
    if (instance_slot == slot)
    {
      clock_gettime(CLOCK_MONOTONIC, &qcril_uim.card_inserted_time);
    }

    QCRIL_LOG_INFO("qcril_uim.card_inserted_time tv_sec:0x%x, tv_nsec:0x%x",
                        qcril_uim.card_inserted_time.tv_sec,
                        qcril_uim.card_inserted_time.tv_nsec);

    qcril_uim_update_cm_card_status(slot,
                                    qcril_card_state);

    if(qmi_ril_get_process_instance_id() == slot &&
       slot < QMI_UIM_MAX_CARD_COUNT)
    {
      qcril_uim.card_status.card[slot].iccid_len = 0;

      if((RIL_UIM_E_SUCCESS == qcril_uim_read_iccid(slot,
                                                    iccid_data,
                                                    &iccid_len)) &&
         (iccid_len > 0) &&
         (iccid_len <= QCRIL_UIM_ICCID_LEN))
      {
        /* Copy ICCID into card cache */
        memcpy(qcril_uim.card_status.card[slot].iccid,
               iccid_data,
               iccid_len);
        qcril_uim.card_status.card[slot].iccid_len = iccid_len;
      }
    }
  }

  if (new_status_ptr->card[slot].card_state == QMI_UIM_CARD_STATE_ERROR ||
      new_status_ptr->card[slot].card_state == QMI_UIM_CARD_STATE_ABSENT)
  {
    /* Based on card_error, update qcril_card_state */
    if (new_status_ptr->card[slot].card_state == QMI_UIM_CARD_STATE_ERROR &&
        new_status_ptr->card[slot].card_error == QMI_UIM_CARD_ERROR_POWER_DOWN)
    {
      qcril_card_state = QCRIL_CARD_STATUS_DOWN;
    }
    else if (new_status_ptr->card[slot].card_state == QMI_UIM_CARD_STATE_ABSENT ||
             (new_status_ptr->card[slot].card_state == QMI_UIM_CARD_STATE_ERROR &&
              ((new_status_ptr->card[slot].card_error == QMI_UIM_CARD_ERROR_NO_ATR_RECEIVED) ||
               (new_status_ptr->card[slot].card_error == QMI_UIM_CARD_ERROR_UNKNOWN_REMOVED) ||
               (new_status_ptr->card[slot].card_error == QMI_UIM_CARD_ERROR_POWER_DOWN)) ))
    {
      qcril_card_state = QCRIL_CARD_STATUS_ABSENT;
    }
    else
    {
      qcril_card_state = QCRIL_CARD_STATUS_NOT_ACCESSIBLE;
    }

    qcril_uim_update_cm_card_status(slot,
                                    qcril_card_state);

    if (instance_slot == slot)
    {
      /* Cleanup refresh data */
      qcril_uim_cleanup_refresh_info();

      /* clear ARA-M delay cache info */
      qcril_uim_cleanup_ara_m_delay_cache_info();

      /* Clean up Long APDU info, if any */
      qcril_uim_cleanup_long_apdu_info();

      /* Clean up select response info, if any */
      qcril_uim_cleanup_select_response_info();

      /* Update the flag for silent PIN verification so that PIN1 can be silently
         verified after SAP is disconnected. */
      if (new_status_ptr->card[slot].card_error == QMI_UIM_CARD_ERROR_SAP_CONNECTED &&
          qmi_ril_is_feature_supported(QMI_RIL_FEATURE_SAP_SILENT_PIN_VERIFY))
      {
        qcril_uim.silent_pin_verify_reqd = TRUE;
      }

      /* Reset ICCID data and length */
      qcril_uim.card_status.card[slot].iccid_len = 0;
      memset(&qcril_uim.card_status.card[slot].iccid, 0x00,
             sizeof(qcril_uim.card_status.card[slot].iccid));
    }

    /* Check to see if card_state is "Absent" */
    if (new_status_ptr->card[slot].card_state == QMI_UIM_CARD_STATE_ABSENT ||
        (new_status_ptr->card[slot].card_state == QMI_UIM_CARD_STATE_ERROR &&
         ((new_status_ptr->card[slot].card_error == QMI_UIM_CARD_ERROR_NO_ATR_RECEIVED) ||
          (new_status_ptr->card[slot].card_error == QMI_UIM_CARD_ERROR_UNKNOWN_REMOVED) ||
          (new_status_ptr->card[slot].card_error == QMI_UIM_CARD_ERROR_POWER_DOWN)) ))
    {
      QCRIL_LOG_INFO("Setting QMI_UIM_CARD_STATE_ABSENT, card_error: %d, Slot: %d \n",
                     new_status_ptr->card[slot].card_error, slot);
      new_sim_state = QCRIL_SIM_STATE_ABSENT;

      /* Clear encrypted PIN1 data in cases of card error except when the card
         was powered down. QCRIL is supposed to retain the encrypted PIN1 data
         to silently verify PIN1 after airplane mode ON-OFF. */
      if (new_status_ptr->card[slot].card_error != QMI_UIM_CARD_ERROR_POWER_DOWN)
      {
        qcril_uim_clear_encrypted_pin_after_card_reset((uint8)slot);
      }

      /* Upon card power down, update the flag for silent PIN verification so
         that PIN1 can be silently verified after the card is powered up. */
      if (instance_slot == slot &&
          new_status_ptr->card[slot].card_error == QMI_UIM_CARD_ERROR_POWER_DOWN &&
          qmi_ril_is_feature_supported(QMI_RIL_FEATURE_SAP_SILENT_PIN_VERIFY))
      {
        qcril_uim.silent_pin_verify_reqd = TRUE;
      }
    }

    /* Check which app type's state to update */
    if ((slot == ((old_status_ptr->index_gw_pri_prov >> 8) & 0xFF)) &&
        ((old_status_ptr->index_gw_pri_prov & 0xFF) < QMI_UIM_MAX_APP_PER_CARD_COUNT))
    {
      /* Means there was a GW session before the card error in this slot */
      if (old_status_ptr->num_slots > 1)
      {
        /* Clear the prov related global data on card error */
        qcril_uim_update_prov_session_type(QMI_UIM_SESSION_TYPE_PRI_GW_PROV,
                                           QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED);
      }
    }
    if ((slot == ((old_status_ptr->index_1x_pri_prov >> 8) & 0xFF)) &&
        ((old_status_ptr->index_1x_pri_prov & 0xFF) < QMI_UIM_MAX_APP_PER_CARD_COUNT))
    {
      if (old_status_ptr->num_slots > 1)
      {
        /* Clear the prov related global data on card error */
        qcril_uim_update_prov_session_type(QMI_UIM_SESSION_TYPE_PRI_1X_PROV,
                                           QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED);
      }
    }
    if ((slot == ((old_status_ptr->index_gw_sec_prov >> 8) & 0xFF)) &&
        ((old_status_ptr->index_gw_sec_prov & 0xFF) < QMI_UIM_MAX_APP_PER_CARD_COUNT))
    {
      if (old_status_ptr->num_slots > 1)
      {
        /* Clear the prov related global data on card error */
        qcril_uim_update_prov_session_type(QMI_UIM_SESSION_TYPE_SEC_GW_PROV,
                                           QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED);
      }
    }
    if ((slot == ((old_status_ptr->index_1x_sec_prov >> 8) & 0xFF)) &&
        ((old_status_ptr->index_1x_sec_prov & 0xFF) < QMI_UIM_MAX_APP_PER_CARD_COUNT))
    {
      if (old_status_ptr->num_slots > 1)
      {
        /* Clear the prov related global data on card error */
        qcril_uim_update_prov_session_type(QMI_UIM_SESSION_TYPE_SEC_1X_PROV,
                                           QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED);
      }
    }
    if ((slot == ((old_status_ptr->index_gw_ter_prov >> 8) & 0xFF)) &&
        ((old_status_ptr->index_gw_ter_prov & 0xFF) < QMI_UIM_MAX_APP_PER_CARD_COUNT))
    {
      if (old_status_ptr->num_slots > 2)
      {
        /* Clear the prov related global data on card error */
        qcril_uim_update_prov_session_type(QMI_UIM_SESSION_TYPE_TER_GW_PROV,
                                           QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED);
      }
    }
    if ((slot == ((old_status_ptr->index_1x_ter_prov >> 8) & 0xFF)) &&
        ((old_status_ptr->index_1x_ter_prov & 0xFF) < QMI_UIM_MAX_APP_PER_CARD_COUNT))
    {
      if (old_status_ptr->num_slots > 2)
      {
        /* Clear the prov related global data on card error */
        qcril_uim_update_prov_session_type(QMI_UIM_SESSION_TYPE_TER_1X_PROV,
                                           QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED);
      }
    }
  }
}/* qcril_uim_handle_card_state_change */


/*===========================================================================

  FUNCTION:  qcril_uim_is_prov_app

===========================================================================*/
/*!
    @brief


    @return
    None.
*/
/*=========================================================================*/
static boolean qcril_uim_is_prov_app
(
  unsigned short      index_prov_app,
  int                 i,
  int                 j
)
{
  if((i == ((index_prov_app >> 8) & 0xFF)) &&
     (j == (index_prov_app & 0xFF)))
  {
    return TRUE;
  }
  return FALSE;
} /* qcril_uim_is_prov_app */


/*===========================================================================

  FUNCTION:  qcril_uim_handle_app_state_change

===========================================================================*/
/*!
    @brief


    @return
    None.
*/
/*=========================================================================*/
static void qcril_uim_handle_app_state_change
(
  const qmi_uim_card_status_type      * new_status_ptr,
  int                                   card_index,
  int                                   app_index,
  uint8                                 slot
)
{
  boolean                                  app_status_changed = FALSE;
  qcril_sim_state_e_type                   new_sim_state      = QCRIL_SIM_STATE_ABSENT;
  qmi_uim_session_type                     session_type       = QMI_UIM_SESSION_TYPE_PRI_GW_PROV;
  RIL_UIM_AppStatus                        card_app_info      = {};
  std::shared_ptr<UimCardAppStatusIndMsg>  app_info_ptr       = nullptr;

  /* Sanity checks */
  if (new_status_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  qcril_uim_convert_app_state_to_qcril(&new_sim_state,
                                       new_status_ptr->card[card_index].application[app_index].app_state);

  /* This covers case where PIN1 is replaced by UPIN also */
  if(qcril_uim_is_prov_app(new_status_ptr->index_gw_pri_prov, card_index, app_index) &&
     (slot == ((new_status_ptr->index_gw_pri_prov >> 8) & 0xFF)))
  {
    app_status_changed = TRUE;
    session_type = QMI_UIM_SESSION_TYPE_PRI_GW_PROV;
    if (qcril_uim.prov_session_info.session_state_gw_indexes[0] != QCRIL_UIM_PROV_SESSION_ACTIVATED)
    {
      qcril_uim_update_prov_session_type(session_type, QCRIL_UIM_PROV_SESSION_ACTIVATED);
    }
  }
  else if(qcril_uim_is_prov_app(new_status_ptr->index_1x_pri_prov, card_index, app_index) &&
          (slot == ((new_status_ptr->index_1x_pri_prov >> 8) & 0xFF)))
  {
    app_status_changed = TRUE;
    session_type = QMI_UIM_SESSION_TYPE_PRI_1X_PROV;
    if (qcril_uim.prov_session_info.session_state_1x_indexes[0] != QCRIL_UIM_PROV_SESSION_ACTIVATED)
    {
      qcril_uim_update_prov_session_type(session_type, QCRIL_UIM_PROV_SESSION_ACTIVATED);
    }
  }
  else if(qcril_uim_is_prov_app(new_status_ptr->index_gw_sec_prov, card_index, app_index) &&
          (slot == ((new_status_ptr->index_gw_sec_prov >> 8) & 0xFF)))
  {
    app_status_changed = TRUE;
    session_type = QMI_UIM_SESSION_TYPE_SEC_GW_PROV;
    if (qcril_uim.prov_session_info.session_state_gw_indexes[1] != QCRIL_UIM_PROV_SESSION_ACTIVATED)
    {
      qcril_uim_update_prov_session_type(session_type, QCRIL_UIM_PROV_SESSION_ACTIVATED);
    }
  }
  else if(qcril_uim_is_prov_app(new_status_ptr->index_1x_sec_prov, card_index, app_index) &&
          (slot == ((new_status_ptr->index_1x_sec_prov >> 8) & 0xFF)))
  {
    app_status_changed = TRUE;
    session_type = QMI_UIM_SESSION_TYPE_SEC_1X_PROV;
    if (qcril_uim.prov_session_info.session_state_1x_indexes[1] != QCRIL_UIM_PROV_SESSION_ACTIVATED)
    {
      qcril_uim_update_prov_session_type(session_type, QCRIL_UIM_PROV_SESSION_ACTIVATED);
    }
  }
  else if(qcril_uim_is_prov_app(new_status_ptr->index_gw_ter_prov, card_index, app_index) &&
          (slot == ((new_status_ptr->index_gw_ter_prov >> 8) & 0xFF)))
  {
    app_status_changed = TRUE;
    session_type = QMI_UIM_SESSION_TYPE_TER_GW_PROV;
    if (qcril_uim.prov_session_info.session_state_gw_indexes[2] != QCRIL_UIM_PROV_SESSION_ACTIVATED)
    {
      qcril_uim_update_prov_session_type(session_type, QCRIL_UIM_PROV_SESSION_ACTIVATED);
    }
  }
  else if(qcril_uim_is_prov_app(new_status_ptr->index_1x_ter_prov, card_index, app_index) &&
          (slot == ((new_status_ptr->index_1x_ter_prov >> 8) & 0xFF)))
  {
    app_status_changed = TRUE;
    session_type = QMI_UIM_SESSION_TYPE_TER_1X_PROV;
    if (qcril_uim.prov_session_info.session_state_1x_indexes[2] != QCRIL_UIM_PROV_SESSION_ACTIVATED)
    {
      qcril_uim_update_prov_session_type(session_type, QCRIL_UIM_PROV_SESSION_ACTIVATED);
    }
  }

  /* Send QCRIL_EVT_CM_CARD_APP_STATUS_CHANGE */
  qcril_uim_convert_app_type(&card_app_info.app_type,
                             new_status_ptr->card[card_index].application[app_index].app_type);
  qcril_uim_convert_app_state_to_ril(&card_app_info.app_state,
                                     new_status_ptr->card[card_index].application[app_index].app_state);
  if (new_status_ptr->card[card_index].application[app_index].aid_len > 0)
  {
    char *aid_ptr = NULL;

    qcril_uim_add_aid_info(&aid_ptr,
                           new_status_ptr->card[card_index].application[app_index].aid_value,
                           new_status_ptr->card[card_index].application[app_index].aid_len);

    if (aid_ptr != NULL)
    {
      card_app_info.aid_ptr = aid_ptr;
      qcril_free(aid_ptr);
    }
  }
  app_info_ptr = std::make_shared<UimCardAppStatusIndMsg>(card_app_info);
  if (app_info_ptr != nullptr)
  {
    app_info_ptr->broadcast();
  }
} /* qcril_uim_handle_app_state_change */


/*===========================================================================

  FUNCTION:  qcril_uim_handle_perso_state_change

===========================================================================*/
/*!
    @brief


    @return
    None.
*/
/*=========================================================================*/
static void qcril_uim_handle_perso_state_change
(
  const qmi_uim_card_status_type   * new_status_ptr,
  int                                card_index,
  int                                app_index
)
{
  qcril_sim_state_e_type      new_sim_state         = QCRIL_SIM_STATE_NOT_READY;
  RIL_UIM_PersoSubstate       new_perso_substate    = RIL_UIM_PERSOSUBSTATE_UNKNOWN;

  /* Sanity checks */
  if (new_status_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    return;
  }

  /* Get the QCRIL sim state */
  qcril_uim_convert_perso_state_to_qcril(&new_sim_state,
                                         new_status_ptr->card[card_index].application[app_index].perso_state);

  /* Get the RIL perso substate */
  qcril_uim_convert_perso_state_all(&new_perso_substate,
                                    new_status_ptr->card[card_index].application[app_index].perso_state,
                                    new_status_ptr->card[card_index].application[app_index].perso_feature);
} /* qcril_uim_handle_perso_state_change */


/*===========================================================================

  FUNCTION:  qcril_uim_handle_prov_index_state_change

===========================================================================*/
/*!
    @brief


    @return
    None.
*/
/*=========================================================================*/
static void qcril_uim_handle_prov_index_state_change
(
  const qmi_uim_card_status_type   * old_status_ptr,
  const qmi_uim_card_status_type   * new_status_ptr
)
{
  /* Sanity checks */
  if ((old_status_ptr == NULL) || (new_status_ptr == NULL))
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  if (old_status_ptr->num_slots <= 1)
  {
    /* No change needed */
    return;
  }

  /* Clear the respective prov app's global session state */
  if ((old_status_ptr->index_gw_pri_prov != new_status_ptr->index_gw_pri_prov) &&
      (new_status_ptr->index_gw_pri_prov == QCRIL_UIM_INVALID_SLOT_APP_INDEX_VALUE) &&
      (qcril_uim.prov_session_info.session_state_gw_indexes[0] != QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED))
  {
    /* Clear the prov related global data on card error */
    qcril_uim_update_prov_session_type(QMI_UIM_SESSION_TYPE_PRI_GW_PROV,
                                       QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED);
  }

  if ((old_status_ptr->index_1x_pri_prov != new_status_ptr->index_1x_pri_prov) &&
      (new_status_ptr->index_1x_pri_prov == QCRIL_UIM_INVALID_SLOT_APP_INDEX_VALUE) &&
      (qcril_uim.prov_session_info.session_state_1x_indexes[0] != QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED))
  {
    /* Clear the prov related global data on card error */
    qcril_uim_update_prov_session_type(QMI_UIM_SESSION_TYPE_PRI_1X_PROV,
                                       QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED);
  }

  if ((old_status_ptr->index_gw_sec_prov != new_status_ptr->index_gw_sec_prov) &&
      (new_status_ptr->index_gw_sec_prov == QCRIL_UIM_INVALID_SLOT_APP_INDEX_VALUE) &&
      (qcril_uim.prov_session_info.session_state_gw_indexes[1] != QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED))
  {
    /* Clear the prov related global data on card error */
    qcril_uim_update_prov_session_type(QMI_UIM_SESSION_TYPE_SEC_GW_PROV,
                                       QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED);
  }

  if ((old_status_ptr->index_1x_sec_prov != new_status_ptr->index_1x_sec_prov) &&
      (new_status_ptr->index_1x_sec_prov == QCRIL_UIM_INVALID_SLOT_APP_INDEX_VALUE) &&
      (qcril_uim.prov_session_info.session_state_1x_indexes[1] != QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED))
  {
    /* Clear the prov related global data on card error */
    qcril_uim_update_prov_session_type(QMI_UIM_SESSION_TYPE_SEC_1X_PROV,
                                       QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED);
  }

  if ((old_status_ptr->index_gw_ter_prov != new_status_ptr->index_gw_ter_prov) &&
      (new_status_ptr->index_gw_ter_prov == QCRIL_UIM_INVALID_SLOT_APP_INDEX_VALUE) &&
      (qcril_uim.prov_session_info.session_state_gw_indexes[2] != QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED))
  {
    /* Clear the prov related global data on card error */
    qcril_uim_update_prov_session_type(QMI_UIM_SESSION_TYPE_TER_GW_PROV,
                                       QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED);
  }

  if ((old_status_ptr->index_1x_ter_prov != new_status_ptr->index_1x_ter_prov) &&
      (new_status_ptr->index_1x_ter_prov == QCRIL_UIM_INVALID_SLOT_APP_INDEX_VALUE) &&
      (qcril_uim.prov_session_info.session_state_1x_indexes[2] != QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED))
  {
    /* Clear the prov related global data on card error */
    qcril_uim_update_prov_session_type(QMI_UIM_SESSION_TYPE_TER_1X_PROV,
                                       QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED);
  }
} /* qcril_uim_handle_prov_index_state_change */


/*===========================================================================

  FUNCTION:  qcril_uim_send_card_status_events

===========================================================================*/
/*!
    @brief
    Based on the indication received, this function checks only specific
    fields for the  card status change, updates the new card status to global,
    and sends card events to necessary internal QCRIL clients.

    @return
    None.
*/
/*=========================================================================*/
static void qcril_uim_send_card_status_events
(
  const qmi_uim_card_status_type * old_status_ptr,
  const qmi_uim_card_status_type * new_status_ptr,
  const qmi_uim_bool_type        * card_status_invalid_ptr,
  uint8                            slot
)
{
  uint8                   i                   = 0;
  uint8                   j                   = 0;

  /* Sanity checks */
  if ((old_status_ptr == NULL) || (new_status_ptr == NULL) ||
      (card_status_invalid_ptr == NULL))
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  /* This check covers a corner case where modem might deactivate only the provisioning
     session indices. In such cases, we need to update the session state for DSDS */
  qcril_uim_handle_prov_index_state_change(old_status_ptr,
                                           new_status_ptr);

  for (i = 0; i < new_status_ptr->num_slots && i < QMI_UIM_MAX_CARD_COUNT; i++)
  {
    if (card_status_invalid_ptr[i] == QMI_UIM_TRUE)
    {
      continue;
    }

    if ((old_status_ptr->card[i].card_state != new_status_ptr->card[i].card_state) ||
        ((new_status_ptr->card[i].card_state == QMI_UIM_CARD_STATE_ERROR) &&
         (old_status_ptr->card[i].card_error != new_status_ptr->card[i].card_error)))
    {
      qcril_uim_handle_card_state_change(i,
                                         old_status_ptr,
                                         new_status_ptr);
    }

    for (j = 0; j < new_status_ptr->card[i].num_app &&
                j < QMI_UIM_MAX_APP_PER_CARD_COUNT; j++)
    {
      if (old_status_ptr->card[i].application[j].app_state !=
          new_status_ptr->card[i].application[j].app_state)
      {
        qcril_uim_handle_app_state_change(new_status_ptr,
                                          i,
                                          j,
                                          slot);
      }

      if (new_status_ptr->card[i].application[j].app_state == QMI_UIM_APP_STATE_PERSO)
      {
        if ((old_status_ptr->card[i].application[j].perso_state !=
             new_status_ptr->card[i].application[j].perso_state) ||
            (old_status_ptr->card[i].application[j].perso_feature !=
             new_status_ptr->card[i].application[j].perso_feature))
        {
          qcril_uim_handle_perso_state_change(new_status_ptr, i, j);
        }
      }
    }
  }
} /* qcril_uim_send_card_status_events */


/*===========================================================================

  FUNCTION:  qcril_uim_copy_card_status

===========================================================================*/
/*!
    @brief
    Copies the new card status to the global.

    @return
    None.
*/
/*=========================================================================*/
static void qcril_uim_copy_card_status
(
  qmi_uim_card_status_type          * out_ptr,
  const qmi_uim_card_status_type    * in_ptr,
  uint16                              size
)
{
  if ((out_ptr == NULL) || (in_ptr == NULL))
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  if (in_ptr->card_status_type == QMI_UIM_EXTENDED_CARD_STATUS_TYPE)
  {
    memcpy(out_ptr, in_ptr, size);
  }
  else
  {
    /* In case of legacy card status indication, copy only primary and
       secondary provisioning session indices and info for up to 2 cards. */
    uint8 i = 0;
    uint8 j = 0;

    out_ptr->index_gw_pri_prov = in_ptr->index_gw_pri_prov;
    out_ptr->index_1x_pri_prov = in_ptr->index_1x_pri_prov;
    out_ptr->index_gw_sec_prov = in_ptr->index_gw_sec_prov;
    out_ptr->index_1x_sec_prov = in_ptr->index_1x_sec_prov;
    /* Ensure that the new num_slots doesn't overwrite a value previously
       updated by extended card status indication.  */
    if (out_ptr->num_slots <= QMI_UIM_LEGACY_MAX_CARD_COUNT ||
        in_ptr->num_slots != QMI_UIM_LEGACY_MAX_CARD_COUNT)
    {
      out_ptr->num_slots = in_ptr->num_slots;
    }
    for (i = 0; i < in_ptr->num_slots && i < QMI_UIM_MAX_CARD_COUNT; i++)
    {
      out_ptr->card[i].physical_slot_id = in_ptr->card[i].physical_slot_id;
      out_ptr->card[i].iccid_len = in_ptr->card[i].iccid_len;
      memcpy(out_ptr->card[i].iccid, in_ptr->card[i].iccid, out_ptr->card[i].iccid_len);
      out_ptr->card[i].atr_len = in_ptr->card[i].atr_len;
      memcpy(out_ptr->card[i].atr, in_ptr->card[i].atr, out_ptr->card[i].atr_len);
      out_ptr->card[i].eid_len = in_ptr->card[i].eid_len;
      memcpy(out_ptr->card[i].eid, in_ptr->card[i].eid, out_ptr->card[i].eid_len);
      out_ptr->card[i].card_state = in_ptr->card[i].card_state;
      out_ptr->card[i].upin_state = in_ptr->card[i].upin_state;
      out_ptr->card[i].upin_num_retries = in_ptr->card[i].upin_num_retries;
      out_ptr->card[i].upuk_num_retries = in_ptr->card[i].upuk_num_retries;
      out_ptr->card[i].card_error = in_ptr->card[i].card_error;
      /* Ensure that the new num_app doesn't overwrite a value previously
         updated by extended card status indication. */
      if (out_ptr->card[i].num_app <= QMI_UIM_LEGACY_MAX_APP_PER_CARD_COUNT ||
          in_ptr->card[i].num_app != QMI_UIM_LEGACY_MAX_APP_PER_CARD_COUNT)
      {
        out_ptr->card[i].num_app = in_ptr->card[i].num_app;
      }
      for (j = 0; (j < in_ptr->card[i].num_app) &&
                  (j < QMI_UIM_MAX_APP_PER_CARD_COUNT); j++)
      {
        out_ptr->card[i].application[j].app_type =
          in_ptr->card[i].application[j].app_type;
        out_ptr->card[i].application[j].app_state =
          in_ptr->card[i].application[j].app_state;
        out_ptr->card[i].application[j].perso_state =
          in_ptr->card[i].application[j].perso_state;
        out_ptr->card[i].application[j].perso_feature =
          in_ptr->card[i].application[j].perso_feature;
        out_ptr->card[i].application[j].perso_retries =
          in_ptr->card[i].application[j].perso_retries;
        out_ptr->card[i].application[j].perso_unblock_retries =
          in_ptr->card[i].application[j].perso_unblock_retries;
        out_ptr->card[i].application[j].aid_len = MIN(QMI_UIM_MAX_AID_LEN,
                                    in_ptr->card[i].application[j].aid_len);
        memcpy(out_ptr->card[i].application[j].aid_value,
               in_ptr->card[i].application[j].aid_value,
               out_ptr->card[i].application[j].aid_len);
        out_ptr->card[i].application[j].univ_pin =
          in_ptr->card[i].application[j].univ_pin;
        out_ptr->card[i].application[j].pin1_state =
          in_ptr->card[i].application[j].pin1_state;
        out_ptr->card[i].application[j].pin1_num_retries =
          in_ptr->card[i].application[j].pin1_num_retries;
        out_ptr->card[i].application[j].puk1_num_retries =
          in_ptr->card[i].application[j].puk1_num_retries;
        out_ptr->card[i].application[j].pin2_state =
          in_ptr->card[i].application[j].pin2_state;
        out_ptr->card[i].application[j].pin2_num_retries =
          in_ptr->card[i].application[j].pin2_num_retries;
        out_ptr->card[i].application[j].puk2_num_retries =
          in_ptr->card[i].application[j].puk2_num_retries;
      }
    }
  }
} /* qcril_uim_copy_card_status */


/*===========================================================================

  FUNCTION:  qcril_uim_init_card_status

===========================================================================*/
/*!
    @brief
    Initializes the global card status structure.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_init_card_status
(
  qmi_uim_card_status_type  * card_status_ptr
)
{
  int      i                                = 0;
  int      j                                = 0;
  uint8_t  phy_slot[QMI_UIM_MAX_CARD_COUNT] = {0};

  if (card_status_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  /* Cache physical slot data */
  for (i = 0; i < QMI_UIM_MAX_CARD_COUNT; i++)
  {
    phy_slot[i] = card_status_ptr->card[i].physical_slot_id;
  }

  memset(card_status_ptr, 0, sizeof(qmi_uim_card_status_type));

  /* Initialize non-zero init params */
  card_status_ptr->index_gw_pri_prov = QCRIL_UIM_INVALID_SLOT_APP_INDEX_VALUE;
  card_status_ptr->index_1x_pri_prov = QCRIL_UIM_INVALID_SLOT_APP_INDEX_VALUE;
  card_status_ptr->index_gw_sec_prov = QCRIL_UIM_INVALID_SLOT_APP_INDEX_VALUE;
  card_status_ptr->index_1x_sec_prov = QCRIL_UIM_INVALID_SLOT_APP_INDEX_VALUE;
  card_status_ptr->index_gw_ter_prov = QCRIL_UIM_INVALID_SLOT_APP_INDEX_VALUE;
  card_status_ptr->index_1x_ter_prov = QCRIL_UIM_INVALID_SLOT_APP_INDEX_VALUE;

  for (i = 0; i < QMI_UIM_MAX_CARD_COUNT; i++)
  {
    card_status_ptr->card[i].physical_slot_id = phy_slot[i];
    card_status_ptr->card[i].card_state = QMI_UIM_CARD_STATE_UNKNOWN;
    card_status_ptr->card[i].upin_state = QMI_UIM_PIN_STATE_UNKNOWN;
    card_status_ptr->card[i].card_error = QMI_UIM_CARD_ERROR_UNKNOWN;
    for (j=0; j<QMI_UIM_MAX_APP_PER_CARD_COUNT; j++)
    {
      card_status_ptr->card[i].application[j].app_type = QMI_UIM_APP_UNKNOWN;
      card_status_ptr->card[i].application[j].app_state = QMI_UIM_APP_STATE_UNKNOWN;
      card_status_ptr->card[i].application[j].perso_state = QMI_UIM_PERSO_STATE_UNKNOWN;
      card_status_ptr->card[i].application[j].perso_feature = QMI_UIM_PERSO_FEATURE_STATUS_UNKNOWN;
      card_status_ptr->card[i].application[j].pin1_state = QMI_UIM_PIN_STATE_UNKNOWN;
      card_status_ptr->card[i].application[j].pin2_state = QMI_UIM_PIN_STATE_UNKNOWN;
    }
  }
} /* qcril_uim_init_card_status */


/*===========================================================================

  FUNCTION:  qcril_uim_print_card_status

===========================================================================*/
/*!
    @brief
    Prints relevant card status information from the global structure.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_print_card_status
(
  const qmi_uim_card_status_type  * card_status_ptr
)
{
  int i = 0;
  int j = 0;
  int k = 0;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (card_status_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  /* Todo Add verbose strings for these types */
  QCRIL_LOG_DEBUG("Index of GW primary prov: 0X%x \n", card_status_ptr->index_gw_pri_prov);
  QCRIL_LOG_DEBUG("Index of 1X primary prov: 0X%x \n", card_status_ptr->index_1x_pri_prov);
  QCRIL_LOG_DEBUG("Index of GW secondary prov: 0X%x \n", card_status_ptr->index_gw_sec_prov);
  QCRIL_LOG_DEBUG("Index of 1X secondary prov: 0X%x \n", card_status_ptr->index_1x_sec_prov);
  QCRIL_LOG_DEBUG("Index of GW tertiary prov: 0X%x \n", card_status_ptr->index_gw_ter_prov);
  QCRIL_LOG_DEBUG("Index of 1X tertiary prov: 0X%x \n", card_status_ptr->index_1x_ter_prov);

  QCRIL_LOG_DEBUG("Number of slots: %d \n", card_status_ptr->num_slots);

  for (i = 0; i < card_status_ptr->num_slots && i < QMI_UIM_MAX_CARD_COUNT; i++)
  {
    QCRIL_LOG_DEBUG("card[%d].card_state: %d \n", i, card_status_ptr->card[i].card_state);
    QCRIL_LOG_DEBUG("card[%d].upin_state: %d \n", i, card_status_ptr->card[i].upin_state);
    QCRIL_LOG_DEBUG("card[%d].upin_num_retries: %d \n", i, card_status_ptr->card[i].upin_num_retries);
    QCRIL_LOG_DEBUG("card[%d].upuk_num_retries: %d \n", i, card_status_ptr->card[i].upuk_num_retries);
    QCRIL_LOG_DEBUG("card[%d].card_error: %d \n", i, card_status_ptr->card[i].card_error);

    QCRIL_LOG_DEBUG("Number of apps: %d \n", card_status_ptr->card[i].num_app);

    for (j = 0; j < card_status_ptr->card[i].num_app &&
                j < QMI_UIM_MAX_APP_PER_CARD_COUNT; j++)
    {
      QCRIL_LOG_DEBUG("card[%d].application[%d].app_type: %d \n", i, j,
                      card_status_ptr->card[i].application[j].app_type);
      QCRIL_LOG_DEBUG("card[%d].application[%d].app_state: %d \n", i, j,
                      card_status_ptr->card[i].application[j].app_state);
      QCRIL_LOG_DEBUG("card[%d].application[%d].perso_state: %d \n", i, j,
                      card_status_ptr->card[i].application[j].perso_state);
      QCRIL_LOG_DEBUG("card[%d].application[%d].perso_feature: %d \n", i, j,
                      card_status_ptr->card[i].application[j].perso_feature);
      QCRIL_LOG_DEBUG("card[%d].application[%d].perso_retries: %d \n", i, j,
                      card_status_ptr->card[i].application[j].perso_retries);
      QCRIL_LOG_DEBUG("card[%d].application[%d].perso_unblock_retries: %d \n", i, j,
                      card_status_ptr->card[i].application[j].perso_unblock_retries);
      QCRIL_LOG_DEBUG("card[%d].application[%d].aid_len: %d \n", i, j,
                      card_status_ptr->card[i].application[j].aid_len);
      QCRIL_LOG_DEBUG("card[%d].application[%d].aid_value: ", i);
      for (k = 0; k < QMI_UIM_MAX_AID_LEN &&
                  k < card_status_ptr->card[i].application[j].aid_len ; k++)
      {
        QCRIL_LOG_DEBUG(" 0X%x", card_status_ptr->card[i].application[j].aid_value[k]);
      }
      QCRIL_LOG_DEBUG("\ncard[%d].application[%d].univ_pin: %d \n", i, j,
                      card_status_ptr->card[i].application[j].univ_pin);
      QCRIL_LOG_DEBUG("card[%d].application[%d].pin1_state: %d \n", i, j,
                      card_status_ptr->card[i].application[j].pin1_state);
      QCRIL_LOG_DEBUG("card[%d].application[%d].pin1_num_retries: %d \n", i, j,
                      card_status_ptr->card[i].application[j].pin1_num_retries);
      QCRIL_LOG_DEBUG("card[%d].application[%d].puk1_num_retries: %d \n", i, j,
                      card_status_ptr->card[i].application[j].puk1_num_retries);
      QCRIL_LOG_DEBUG("card[%d].application[%d].pin2_state: %d \n", i, j,
                      card_status_ptr->card[i].application[j].pin2_state);
      QCRIL_LOG_DEBUG("card[%d].application[%d].pin2_num_retries: %d \n", i, j,
                      card_status_ptr->card[i].application[j].pin2_num_retries);
      QCRIL_LOG_DEBUG("card[%d].application[%d].puk2_num_retries: %d \n", i, j,
                      card_status_ptr->card[i].application[j].puk2_num_retries);
    }
  }
} /* qcril_uim_print_card_status */


/*=========================================================================

  FUNCTION:  qcril_uim_voltage_supply_resp

===========================================================================*/
/*!
    @brief
    Processes the response for voltage supply command.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_voltage_supply_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<UimVoltageStatusRequestMsg> req_ptr
)
{
  std::shared_ptr<RIL_UIM_Errno>    rsp_ptr = NULL;
  RIL_UIM_Errno                     ril_err = RIL_UIM_E_SUCCESS;

  /* Sanity checks */
  if (params_ptr == NULL || req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  QCRIL_LOG_DEBUG( "qcril_uim_voltage_supply_resp: qmi_err_code=%d \n",
                    params_ptr->qmi_rsp_data.qmi_err_code );

  ril_err = qcril_uim_convert_err_value(params_ptr->qmi_rsp_data.qmi_err_code);

  rsp_ptr = std::make_shared<RIL_UIM_Errno>(ril_err);

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_voltage_supply_resp */


/*=========================================================================

  FUNCTION:  qcril_uim_get_atr_resp

===========================================================================*/
/*!
    @brief
    Processes the response for get ATR command.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_get_atr_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<UimGetAtrRequestMsg> req_ptr
)
{
  RIL_UIM_Errno                     ril_err = RIL_UIM_E_SUCCESS;
  char                            * atr_ptr = NULL;
  std::shared_ptr<UimAtrRspParam>   rsp_ptr = NULL;

  /* Sanity checks */
  if (params_ptr == NULL || req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  ril_err = qcril_uim_convert_err_value(params_ptr->qmi_rsp_data.qmi_err_code);

  QCRIL_LOG_DEBUG( "qcril_uim_get_atr_resp: qmi_err_code=%d \n",
                    params_ptr->qmi_rsp_data.qmi_err_code );

  if (ril_err == RIL_UIM_E_SUCCESS)
  {
    /* Convert the raw ATR bytes to a null terminated ASCII string
     The function qcril_uim_alloc_bin_to_hexstring will perform the allocation & data conversion */
     atr_ptr = qcril_uim_alloc_bin_to_hexstring(
                 (const uint8*)params_ptr->qmi_rsp_data.rsp_data.get_atr_rsp.atr_response.data_ptr,
                 (uint16)params_ptr->qmi_rsp_data.rsp_data.get_atr_rsp.atr_response.data_len);

     /* Update the length */
     if (atr_ptr == NULL)
     {
       ril_err = RIL_UIM_E_NO_MEMORY;
     }
  }

  QCRIL_LOG_DEBUG( "ATR Response: %s\n", atr_ptr != NULL ? atr_ptr : "NULL");

  rsp_ptr = std::make_shared<UimAtrRspParam>();

  if (rsp_ptr != NULL)
  {
    rsp_ptr->err = ril_err;
    if (atr_ptr != NULL)
    {
      rsp_ptr->atr = atr_ptr;
      qcril_free(atr_ptr);
      atr_ptr = NULL;
    }
  }

  /* Send the response */
  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_get_atr_resp */


/*=========================================================================

  FUNCTION:  qcril_uim_power_resp

===========================================================================*/
/*!
    @brief
    Processes the response for power up/down command.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_power_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  std::shared_ptr<UimCardPowerReqMsg>   msg_ptr
)
{
  std::shared_ptr<RIL_UIM_Errno>   ril_err = std::make_shared<RIL_UIM_Errno>(RIL_UIM_E_SUCCESS);
  /* Sanity checks */
  if (msg_ptr == NULL || ril_err == NULL || params_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  *ril_err = qcril_uim_convert_err_value(params_ptr->qmi_rsp_data.qmi_err_code);

  QCRIL_LOG_DEBUG( "qcril_uim_power_resp: qmi_err_code=%d \n",
                    params_ptr->qmi_rsp_data.qmi_err_code);

  msg_ptr->sendResponse(msg_ptr, Message::Callback::Status::SUCCESS, ril_err);

} /* qcril_uim_power_resp */


/*=========================================================================

  FUNCTION:  qcril_uim_change_prov_session_callback

===========================================================================*/
/*!
    @brief
    Callback for the change provisioning session. It sends an event to
    QCRIL(CM) regarding the status of the activate/deactivate command along
    with the session type.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_change_prov_session_callback
(
  qmi_uim_rsp_data_type                              * rsp_data,
  const std::shared_ptr<UimChangeSubscriptionReqMsg>   req_ptr
)
{
  qcril_provision_info_type                       provision_info  = {};
  qcril_uim_prov_session_state_type               session_state   = {};
  std::shared_ptr<UimProvisionStatusIndMsg>       prov_status_ptr = nullptr;
  const qcril_uim_uicc_subs_info_type           * sub_info_ptr    = NULL;
  uim_session_type                                session_type    = UIM_SESSION_TYPE_PRI_GW_PROV;
  uim_provision_status_type                       evt             = UIM_DEACTIVATE_PROVISION_STATUS;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  /* Sanity checks */
  if (rsp_data == NULL || req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  sub_info_ptr = req_ptr->get_sub_info();

  if (sub_info_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    return;
  }

  /* Verify certain parameters */
  if (rsp_data->rsp_id != QMI_UIM_SRVC_CHANGE_PROV_SESSION_RSP_MSG)
  {
    QCRIL_LOG_ERROR("Invalid values, rsp_id: 0x%x, ", rsp_data->rsp_id);
    QCRIL_ASSERT(0);
    return;
  }

  if ((sub_info_ptr->subs_mode_pref == QCRIL_SUBS_MODE_GW) &&
      (sub_info_ptr->uicc_subs_info.sub_type == UIM_SUBSCRIPTION_1))
  {
    session_type = UIM_SESSION_TYPE_PRI_GW_PROV;
  }
  else if ((sub_info_ptr->subs_mode_pref == QCRIL_SUBS_MODE_1X) &&
           (sub_info_ptr->uicc_subs_info.sub_type == UIM_SUBSCRIPTION_1))
  {
    session_type = UIM_SESSION_TYPE_PRI_1X_PROV;
  }
  else if ((sub_info_ptr->subs_mode_pref == QCRIL_SUBS_MODE_GW) &&
           (sub_info_ptr->uicc_subs_info.sub_type == UIM_SUBSCRIPTION_2))
  {
    session_type = UIM_SESSION_TYPE_SEC_GW_PROV;
  }
  else if ((sub_info_ptr->subs_mode_pref == QCRIL_SUBS_MODE_1X) &&
           (sub_info_ptr->uicc_subs_info.sub_type == UIM_SUBSCRIPTION_2))
  {
    session_type = UIM_SESSION_TYPE_SEC_1X_PROV;
  }
  else if ((sub_info_ptr->subs_mode_pref == QCRIL_SUBS_MODE_GW) &&
           (sub_info_ptr->uicc_subs_info.sub_type == UIM_SUBSCRIPTION_3))
  {
    session_type = UIM_SESSION_TYPE_TER_GW_PROV;
  }
  else if ((sub_info_ptr->subs_mode_pref == QCRIL_SUBS_MODE_1X) &&
           (sub_info_ptr->uicc_subs_info.sub_type == UIM_SUBSCRIPTION_3))
  {
    session_type = UIM_SESSION_TYPE_TER_1X_PROV;
  }

  QCRIL_LOG_DEBUG( "qcril_uim_change_prov_session_callback: event=%d, qmi_err_code=%d \n",
                   sub_info_ptr->uicc_subs_info.act_status,
                   rsp_data->qmi_err_code);

  /* Update info being sent to QCRIL_CM */
  provision_info.status = (rsp_data->qmi_err_code == 0) ? QCRIL_PROVISION_STATUS_SUCCESS : QCRIL_PROVISION_STATUS_FAILURE;
  provision_info.err_code = (uint32_t)qcril_uim_convert_err_value(rsp_data->qmi_err_code);
  provision_info.session_type = session_type;

  /* Change the global session type appropriately */
  if (sub_info_ptr->uicc_subs_info.act_status == UIM_UICC_SUBSCRIPTION_ACTIVATE)
  {
    evt = UIM_ACTIVATE_PROVISION_STATUS;

    if (provision_info.status == QCRIL_PROVISION_STATUS_SUCCESS)
    {
      qcril_uim_update_prov_session_type((qmi_uim_session_type)session_type,
                                         QCRIL_UIM_PROV_SESSION_ACTIVATED);
    }
    else if (provision_info.status == QCRIL_PROVISION_STATUS_FAILURE)
    {
      qcril_uim_update_prov_session_type((qmi_uim_session_type)session_type,
                                         QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED);
    }
    else
    {
      QCRIL_LOG_ERROR( "Invalid provision_info status mapping: %d\n",
                       provision_info.status );
    }
  }
  else if (sub_info_ptr->uicc_subs_info.act_status == UIM_UICC_SUBSCRIPTION_DEACTIVATE)
  {
    evt = UIM_DEACTIVATE_PROVISION_STATUS;

    if (provision_info.status == QCRIL_PROVISION_STATUS_SUCCESS)
    {
      qcril_uim_update_prov_session_type((qmi_uim_session_type)session_type,
                                         QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED);
    }
    else if (provision_info.status == QCRIL_PROVISION_STATUS_FAILURE)
    {
      if((qcril_uim_get_prov_session_state((qmi_uim_session_type)session_type,
                                           &session_state) == 0) &&
         (session_state == QCRIL_UIM_PROV_SESSION_DEACTIVATION_IN_PROGESS))
      {
        qcril_uim_update_prov_session_type((qmi_uim_session_type)session_type,
                                           QCRIL_UIM_PROV_SESSION_ACTIVATED);
      }
    }
    else
    {
      QCRIL_LOG_ERROR( "Invalid provision_info status mapping: %d\n",
                       provision_info.status);
    }
  }

  /* Send response to QCRIL_CM */
  prov_status_ptr = std::make_shared<UimProvisionStatusIndMsg>(evt,
                                                               provision_info);
  if (prov_status_ptr != nullptr)
  {
    prov_status_ptr->broadcast();
  }
  else
  {
    QCRIL_LOG_ERROR("Processing subcription evt failed!\n" );
  }
} /* qcril_uim_change_prov_session_callback */


/*===========================================================================

  FUNCTION:  qcril_uim_process_status_change_ind

===========================================================================*/
/*!
    @brief
    Main function for processing QMI card status changed indication. Based
    on the indication received, if needed, it updates the global card status,
    and sends card events internal to QCRIL (CM & PBM).

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_process_status_change_ind
(
  const qcril_uim_indication_params_type  * ind_param_ptr
)
{
  qmi_uim_card_status_type                  * status_change_ind_ptr = NULL;
  RIL_UIM_Errno                               verify_status         = RIL_UIM_E_INTERNAL_ERR;
  qmi_uim_bool_type                         * card_status_invalid   = NULL;
  uint8                                       slot                  = 0;
  boolean                                     allow_indication      = FALSE;
  uint8                                       i                     = 0;
  boolean                                     status_changed        = FALSE;
  std::shared_ptr<UimCardStatusChangeIndMsg>  card_st_ch_ind_ptr    = nullptr;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (ind_param_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid inputs, cannot proceed");
    return;
  }

  status_change_ind_ptr = (qmi_uim_card_status_type*)&ind_param_ptr->ind_data.status_change_ind;
  if (status_change_ind_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, NULL status_change_ind_ptr");
    return;
  }

  card_status_invalid = (qmi_uim_bool_type *)ind_param_ptr->ind_data.status_change_ind.card_status_validity.card_status_invalid;

  slot = qmi_ril_get_process_instance_id();
  if (slot >= QMI_UIM_MAX_CARD_COUNT)
  {
    QCRIL_LOG_ERROR("Invalid slot 0x%x", slot);
    return;
  }

  for (uint32_t i = 0; i < status_change_ind_ptr->num_slots && i < QMI_UIM_MAX_CARD_COUNT; i++)
  {
    qmi_uim_rsp_data_type          rsp_data      = {};
    int                            qmi_err_code  = 0;

    status_change_ind_ptr->card[i].physical_slot_id = qcril_uim.card_status.card[i].physical_slot_id;

    if (status_change_ind_ptr->card[i].card_state == QMI_UIM_CARD_STATE_PRESENT)
    {
      if (status_change_ind_ptr->card[i].card_state != qcril_uim.card_status.card[i].card_state)
      {
        if(status_change_ind_ptr->card[i].atr_len == 0)
        {
          qmi_uim_get_atr_params_type       get_atr_params = {};

          get_atr_params.slot = (qmi_uim_slot_type)(i + QMI_UIM_SLOT_1);

          /* get atr */
          qmi_err_code = qcril_uim_send_qmi_sync_msg(QCRIL_UIM_REQUEST_GET_ATR,
                                                     &get_atr_params,
                                                     &rsp_data);
          if (qmi_err_code == 0 && rsp_data.qmi_err_code == 0)
          {
            status_change_ind_ptr->card[i].atr_len =
                MIN(rsp_data.rsp_data.get_atr_rsp.atr_response.data_len,
                    QMI_UIM_MAX_ATR_LEN);
            memcpy(&status_change_ind_ptr->card[i].atr[0],
                   rsp_data.rsp_data.get_atr_rsp.atr_response.data_ptr,
                   status_change_ind_ptr->card[i].atr_len);
          }
        }

        if (status_change_ind_ptr->card[i].iccid_len == 0)
        {
          /* get ICCID */
          status_change_ind_ptr->card[i].iccid_len = QMI_UIM_MAX_ICCID_LEN;
          (void)qcril_uim_read_iccid(i,
                                     &status_change_ind_ptr->card[i].iccid[0],
                                     (uint8_t *)&status_change_ind_ptr->card[i].iccid_len);
        }
      }
      else
      {
        if (status_change_ind_ptr->card[i].atr_len == 0)
        {
          status_change_ind_ptr->card[i].atr_len = qcril_uim.card_status.card[i].atr_len;
          memcpy(status_change_ind_ptr->card[i].atr,
                 qcril_uim.card_status.card[i].atr,
                 status_change_ind_ptr->card[i].atr_len);
        }

        if(status_change_ind_ptr->card[i].iccid_len == 0)
        {
          status_change_ind_ptr->card[i].iccid_len = qcril_uim.card_status.card[i].iccid_len;
          memcpy(status_change_ind_ptr->card[i].iccid,
                 qcril_uim.card_status.card[i].iccid,
                 status_change_ind_ptr->card[i].iccid_len);
        }

      }
    }
  }

  /* Below event is not supported in RIL UT */
  /* Card state change notification is sent with oem hook message with slot id and
     card state. The oem hook message will be sent only when there is card state
     change not for the APP state change. One unsol message for each slot, same
     indication will come on other slots also, check only slot 0 */
  if (slot == 0)
  {
    for (i = 0; i < status_change_ind_ptr->num_slots && i < QMI_UIM_MAX_CARD_COUNT; i++)
    {
      /* Determine change in card status only if it is not invalid */
      if ((card_status_invalid == NULL || !card_status_invalid[i]) &&
          qcril_uim.card_status.card[i].card_state != status_change_ind_ptr->card[i].card_state &&
          status_change_ind_ptr->card[i].card_state != QMI_UIM_CARD_STATE_UNKNOWN)
      {
        auto card_state = std::make_shared<UimCardStateChangeInd>(i, (RIL_UIM_CardState)status_change_ind_ptr->card[i].card_state);

        QCRIL_LOG_INFO("OEM HOOK UNSOL_CARD_STATE_CHANGE_IND");

        if (card_state != NULL)
        {
          card_state->broadcast();
        }
      }
    }
  }

  /* Determine change in card status only if it is not invalid */
  if (card_status_invalid == NULL || !card_status_invalid[slot])
  {
    boolean skip_copy = FALSE;

    status_changed = qcril_uim_has_card_status_changed(&qcril_uim.card_status,
                                                       status_change_ind_ptr,
                                                       slot);

    qcril_uim_card_status_mutex.lock();

    if (status_changed)
    {

      /* Compute the cardAppState for UIM remote client*/
      qcril_uim_determine_card_app_init_status_and_send_indication(slot, status_change_ind_ptr);

      /* Check the card state transition to see if we need to skip the card
         status copy & posting of internal events & UNSOL to ATEL */
      skip_copy = qcril_uim_skip_card_indication(&qcril_uim.card_status,
                                                 status_change_ind_ptr,
                                                 slot);
      if (skip_copy)
      {
        status_changed = FALSE;
        QCRIL_LOG_INFO("Skipping UNSOL for slot_id 0x%x", slot);
      }
      else
      {
        qcril_uim_send_card_status_events(&qcril_uim.card_status,
                                          status_change_ind_ptr,
                                          card_status_invalid,
                                          slot);
      }
    }

    /* Update global card status */
    if (!skip_copy)
    {
      qcril_uim_copy_card_status(&qcril_uim.card_status,
                                 status_change_ind_ptr,
                                 sizeof(qmi_uim_card_status_type));
    }

    qcril_uim_card_status_mutex.unlock();
  }
  else
  {
    QCRIL_LOG_ERROR("card status invalid for slot 0x%x", slot);
  }

  if (status_changed)
  {
    uint8 session_slot  = 0;
    uint8 session_index = 0;

    if(qcril_uim.silent_pin_verify_reqd &&
       qcril_uim.card_status.card[slot].card_state == QMI_UIM_CARD_STATE_PRESENT)
    {
      verify_status = qcril_uim_try_pin1_verification(slot);

      /* For Multi-SIM manual app selection the client must be notified when the card
         is present to manually select the application. This notification should only
         be sent once, when all provisioning applications on the card are still in
         detected state and the provisioning indices are not set. */
      if (status_change_ind_ptr->num_slots > 1)
      {
        for (i = 0; i < QMI_UIM_MAX_APP_PER_CARD_COUNT; i++)
        {
          /* If any provisioning application on the card has made it past app state detected,
             then the indication to notify the client to manually select has already been sent. */
          if (qcril_uim.card_status.card[slot].application[i].app_state > QMI_UIM_APP_STATE_DETECTED &&
              qcril_uim_is_prov_app_type(qcril_uim.card_status.card[slot].application[i].app_type))
          {
            allow_indication = FALSE;
            break;
          }

          if (qcril_uim.card_status.card[slot].application[i].app_state == QMI_UIM_APP_STATE_DETECTED &&
              qcril_uim_is_prov_app_type(qcril_uim.card_status.card[slot].application[i].app_type) &&
              !qcril_uim_is_prov_app_activated(slot,
                                               qcril_uim.card_status.card[slot].application[i].app_type,
                                               qcril_uim.card_status.card[slot].application[i].aid_len,
                                               qcril_uim.card_status.card[slot].application[i].aid_value))
          {
            allow_indication = TRUE;
          }
        }
      }
    }
    qcril_uim_check_silent_pin_verify_in_progress(slot);

    /* Register for refresh if the slot derived from RIL instance and the
       slot derived from session index matches */
    if((qcril_uim_extract_index(&session_index,
                                &session_slot,
                                QMI_UIM_SESSION_TYPE_PRI_GW_PROV) == RIL_UIM_E_SUCCESS) &&
       (slot == session_slot))
    {
      qcril_uim_refresh_register(QMI_UIM_SESSION_TYPE_PRI_GW_PROV);
    }

    if((qcril_uim_extract_index(&session_index,
                                &session_slot,
                                QMI_UIM_SESSION_TYPE_PRI_1X_PROV) == RIL_UIM_E_SUCCESS) &&
       (slot == session_slot))
    {
      qcril_uim_refresh_register(QMI_UIM_SESSION_TYPE_PRI_1X_PROV);
    }
    if(slot == 0)
    {
      qcril_uim_refresh_register_isim(QMI_UIM_SLOT_1);
    }

#ifndef QMI_RIL_UTF
    qcril_uim_refresh_register_card_session(slot);
#endif /* QMI_RIL_UTF */

    if (status_change_ind_ptr->num_slots > 1)
    {
      /* Register for refresh if the slot derived from RIL instance and the
         slot derived from session index matches */
      if((qcril_uim_extract_index(&session_index,
                                  &session_slot,
                                  QMI_UIM_SESSION_TYPE_SEC_GW_PROV) == RIL_UIM_E_SUCCESS) &&
         (slot == session_slot))
      {
        qcril_uim_refresh_register(QMI_UIM_SESSION_TYPE_SEC_GW_PROV);
      }

      if((qcril_uim_extract_index(&session_index,
                                  &session_slot,
                                  QMI_UIM_SESSION_TYPE_SEC_1X_PROV) == RIL_UIM_E_SUCCESS) &&
         (slot == session_slot))
      {
        qcril_uim_refresh_register(QMI_UIM_SESSION_TYPE_SEC_1X_PROV);
      }
      if(slot == 1)
      {
        qcril_uim_refresh_register_isim(QMI_UIM_SLOT_2);
      }
      if (status_change_ind_ptr->num_slots > 2)
      {
        /* Register for refresh if the slot derived from RIL instance and the
           slot derived from session index matches */
        if((qcril_uim_extract_index(&session_index,
                                    &session_slot,
                                    QMI_UIM_SESSION_TYPE_TER_GW_PROV) == RIL_UIM_E_SUCCESS) &&
           (slot == session_slot))
        {
          qcril_uim_refresh_register(QMI_UIM_SESSION_TYPE_TER_GW_PROV);
        }

        if((qcril_uim_extract_index(&session_index,
                                    &session_slot,
                                    QMI_UIM_SESSION_TYPE_TER_1X_PROV) == RIL_UIM_E_SUCCESS) &&
           (slot == session_slot))
        {
          qcril_uim_refresh_register(QMI_UIM_SESSION_TYPE_TER_1X_PROV);
        }
        if(slot == 2)
        {
          qcril_uim_refresh_register_isim(QMI_UIM_SLOT_3);
        }
      }
    }

    if ((verify_status != RIL_UIM_E_SUCCESS &&
         qcril_uim.pin1_info[0].silent_verify_in_progress == FALSE &&
         qcril_uim.pin1_info[1].silent_verify_in_progress == FALSE) ||
         allow_indication )
    {
      auto sim_state = std::make_shared<UimSimStatusChangedInd>();

      QCRIL_LOG_INFO("UNSOL_SIM_STATUS_CHANGED");

      if (sim_state != NULL)
      {
        sim_state->broadcast();
      }
    }
  }

  qcril_uim_print_card_status(&qcril_uim.card_status);

  /* send card status change indication internally event to QCRIL */
  card_st_ch_ind_ptr = std::make_shared<UimCardStatusChangeIndMsg>();
  if (card_st_ch_ind_ptr != nullptr)
  {
    card_st_ch_ind_ptr->broadcast();
  }
} /* qcril_uim_process_status_change_ind */


/*=========================================================================

  FUNCTION:  qcril_uim_process_change_subscription

===========================================================================*/
/*!
    @brief
    Processes the subscription activation/deactivation requests from QCRIL.
    called as a result of QCRIL_EVT_INTERNAL_MMGSDI_ACTIVATE_SUB or
    QCRIL_EVT_INTERNAL_MMGSDI_DEACTIVATE_SUB.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_process_change_subscription
(
  std::shared_ptr<UimChangeSubscriptionReqMsg> msg_ptr
)
{
  const qcril_uim_uicc_subs_info_type     * ril_uicc_subs_info_ptr = NULL;
  qmi_uim_change_prov_session_params_type   change_session_params  = {};
  qcril_provision_info_type                 provision_info         = {};
  qcril_uim_prov_session_info_type          orig_session_info      = {};
  uim_provision_status_type                 prov_evt = UIM_DEACTIVATE_PROVISION_STATUS;
  std::shared_ptr<UimProvisionStatusIndMsg> prov_status_fail_ptr   = nullptr;

  /* Sanity checks */
  if (msg_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  if ( qcril_uim.card_status.num_slots <= 1)
  {
    QCRIL_LOG_ERROR("Multi SIM not enabled, cannot proceed");
    return;
  }

  /* Cast input info */
  ril_uicc_subs_info_ptr = (const qcril_uim_uicc_subs_info_type *)msg_ptr->get_sub_info();

  QCRIL_LOG_INFO( "qcril_uim_process_change_subscription:slot: 0x%x, app_index: 0x%x, sub_type: 0x%x, act_status: 0x%x\n",
                  ril_uicc_subs_info_ptr->uicc_subs_info.slot,
                  ril_uicc_subs_info_ptr->uicc_subs_info.app_index,
                  (int)ril_uicc_subs_info_ptr->uicc_subs_info.sub_type,
                  (int)ril_uicc_subs_info_ptr->uicc_subs_info.act_status );

  /* Fill in the backup data */
  provision_info.status  = QCRIL_PROVISION_STATUS_FAILURE;
  if (ril_uicc_subs_info_ptr->uicc_subs_info.act_status == UIM_UICC_SUBSCRIPTION_ACTIVATE)
  {
    prov_evt = UIM_ACTIVATE_PROVISION_STATUS;
  }

  memcpy(&orig_session_info, &qcril_uim.prov_session_info,
         sizeof(qcril_uim_prov_session_info_type));

  /* Ignore requests if already activated/deactivated and send SUCCESS */
  if (qcril_uim_ignore_change_subscription(ril_uicc_subs_info_ptr,
                                           &change_session_params) == RIL_UIM_E_SUCCESS)
  {
    provision_info.err_code      = RIL_UIM_E_SUCCESS;
    provision_info.status        = QCRIL_PROVISION_STATUS_SUCCESS;
    provision_info.session_type  = (uim_session_type)change_session_params.session_type;
    goto send_resp;
  }

  /* Fill the aid, session type, activation type */
  provision_info.err_code = (uint32_t)qcril_uim_fill_change_session_params(ril_uicc_subs_info_ptr,
                                                                 &change_session_params);
  if (provision_info.err_code != RIL_UIM_E_SUCCESS)
  {
    goto send_resp;
  }

  /* Activation type */
  switch (ril_uicc_subs_info_ptr->uicc_subs_info.act_status)
  {
    case UIM_UICC_SUBSCRIPTION_ACTIVATE:
      change_session_params.activation_type = QMI_UIM_SESSION_ACTIVATE;
      break;
    case UIM_UICC_SUBSCRIPTION_DEACTIVATE:
      change_session_params.activation_type = QMI_UIM_SESSION_DEACTIVATE;
      break;
    default:
      /* Invalid activation type */
      QCRIL_LOG_ERROR( " Invalid activation type in RIL Request ID 0x%x\n",
                       ril_uicc_subs_info_ptr->uicc_subs_info.act_status);
      goto send_resp;
  }

  /* Provisioning session in progress, update QCRIL(CM) on the session type */
  if ( prov_evt == UIM_ACTIVATE_PROVISION_STATUS )
  {
    std::shared_ptr<UimProvisionStatusIndMsg> prov_status_ptr = nullptr;

    provision_info.status  = QCRIL_PROVISION_STATUS_IN_PROGRESS;
    provision_info.session_type = (uim_session_type)change_session_params.session_type;

    prov_status_ptr = std::make_shared<UimProvisionStatusIndMsg>(prov_evt, provision_info);

    if (prov_status_ptr != nullptr)
    {
      prov_status_ptr->broadcast();
    }
    else
    {
      QCRIL_LOG_ERROR( "Sending subcription evt failed!\n" );
    }
  }

  if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_CHANGE_PROV_SESSION,
                                   &change_session_params,
                                   msg_ptr) >= 0)
  {
    return;
  }

send_resp:
  /* revert our global provisioning app session state */
  memcpy(&qcril_uim.prov_session_info, &orig_session_info,
         sizeof(qcril_uim_prov_session_info_type));

  prov_status_fail_ptr = std::make_shared<UimProvisionStatusIndMsg>(prov_evt, provision_info);

  if (prov_status_fail_ptr != nullptr)
  {
    prov_status_fail_ptr->broadcast();
  }
  else
  {
    QCRIL_LOG_ERROR( "%s\n", "change provisioning session failed!" );
  }
} /* qcril_uim_process_change_subscription */


/*=========================================================================

  FUNCTION:  qcril_uim_process_get_atr

===========================================================================*/
/*!
    @brief
     Handles RIL_REQUEST_SIM_GET_ATR and QCRIL_EVT_HOOK_SLOT_GET_ATR_REQ
     request from QCRIL.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_get_atr
(
  const std::shared_ptr<UimGetAtrRequestMsg> req_ptr
)
{
  qmi_uim_get_atr_params_type       get_atr_params = {};
  RIL_UIM_Errno                     err            = RIL_UIM_E_INTERNAL_ERR;
  std::shared_ptr<UimAtrRspParam>   rsp_ptr        = nullptr;

  /* Sanity check */
  if (req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    return;
  }

  QCRIL_LOG_INFO("qcril_uim_process_get_atr\n");

  memset(&get_atr_params, 0, sizeof(qmi_uim_get_atr_params_type));

  switch (req_ptr->get_slot())
  {
    case 0:
      get_atr_params.slot = QMI_UIM_SLOT_1;
      break;

    case 1:
      get_atr_params.slot = QMI_UIM_SLOT_2;
      break;

    case 2:
      get_atr_params.slot = QMI_UIM_SLOT_3;
      break;

    default:
      QCRIL_LOG_ERROR( "Invalid slot %d\n", req_ptr->get_slot());
      err = RIL_UIM_E_INVALID_ARGUMENTS;
      goto get_atr_error;
  }

  if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_GET_ATR,
                                   &get_atr_params,
                                   req_ptr) >= 0)
  {
    return;
  }

  err = RIL_UIM_E_NO_MEMORY;

get_atr_error:
  rsp_ptr = std::make_shared<UimAtrRspParam>();

  if (rsp_ptr != NULL)
  {
    rsp_ptr->err = err;
  }

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_request_get_atr */

/*===========================================================================

                               QCRIL INTERFACE FUNCTIONS

===========================================================================*/

/*===========================================================================

  FUNCTION:  qcril_uim_request_get_sim_status

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_GET_SIM_STATUS request from the framework

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_get_sim_status
(
  const std::shared_ptr<UimGetCardStatusRequestMsg> msg_ptr
)
{
  std::shared_ptr<RIL_UIM_CardStatus>   ril_card_status_ptr = NULL;
  uint8_t                               slot                = QCRIL_UIM_INVALID_SLOT_INDEX_VALUE;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  /* Sanity check */
  if (msg_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  ril_card_status_ptr = std::make_shared<RIL_UIM_CardStatus>();

  if (ril_card_status_ptr == NULL)
  {
    msg_ptr->sendResponse(msg_ptr, Message::Callback::Status::SUCCESS, NULL);
    return;
  }

  slot = msg_ptr->get_slot();

  if (slot >= QMI_UIM_MAX_CARD_COUNT)
  {
    QCRIL_LOG_ERROR("Invalid value, slot: 0x%x", slot);
    ril_card_status_ptr->err = RIL_UIM_E_INVALID_ARGUMENTS;
    msg_ptr->sendResponse(msg_ptr, Message::Callback::Status::SUCCESS, ril_card_status_ptr);
    return;
  }

  if (qcril_uim.card_status.card[slot].card_state == QMI_UIM_CARD_STATE_UNKNOWN)
  {
    /* For an un-init card state, send an error */
    QCRIL_LOG_ERROR( "Card status is unknown for slot: 0x%x,\n", slot);
    ril_card_status_ptr->err = RIL_UIM_E_INVALID_SIM_STATE;
  }
  else if (qcril_uim.pin1_info[0].silent_verify_in_progress ||
           qcril_uim.pin1_info[1].silent_verify_in_progress)
  {
    /* Send an error if PIN1 state is not yet updated after silent PIN1
       verification */
    QCRIL_LOG_ERROR("PIN1 state not updated after silent verification");
    ril_card_status_ptr->err = RIL_UIM_E_INVALID_SIM_STATE;
  }
  /* If silent pin verification is required and pin state is
     ENABLED_NOT_VERIFIED then do not send card status */
  else if(qcril_uim.silent_pin_verify_reqd &&
          qcril_uim_is_silent_pin_verification_needed(slot))
  {
    QCRIL_LOG_ERROR("Silent PIN verification is in progress");
    ril_card_status_ptr->err = RIL_UIM_E_INVALID_SIM_STATE;
  }
  else
  {
    /* Construct RIL_UIM_CardStatus and log string */
    qcril_uim_construct_card_status(ril_card_status_ptr,
                                    &qcril_uim.card_status,
                                    slot);

    ril_card_status_ptr->err = RIL_UIM_E_SUCCESS;
  }

  /* Send the response */
  msg_ptr->sendResponse(msg_ptr, Message::Callback::Status::SUCCESS, ril_card_status_ptr);

  /* We need to clear the timer if it is running because we block the incoming new
     card status INDs until ATEL successfully reads the card status. Also QCRIL
     needs to fetch latest card status from modem that we might have skipped */
  if ((ril_card_status_ptr->err == RIL_UIM_E_SUCCESS) && (qcril_uim_card_state_check_timer_msg_ptr != NULL))
  {

    QCRIL_LOG_INFO( "GET_SIM_STATUS_REQUEST received - Cancelling Card status check timer\n");
    qcril_uim_cancel_card_state_check_timer(TIMER_CANCEL_REASON_NORMAL);

    /* No need to call latest GET_CARD_STATUS from Modem now, Framework, 
       shall call the Cb function immediately where GET_CARD_STATUS will
       be fetched from Modem and STATUS_CHANGE_IND will be notified to clients */
  }
} /* qcril_uim_request_get_sim_status */

/*===========================================================================

  FUNCTION:  qcril_uim_direct_get_card_status

===========================================================================*/
/*!
    @brief
    Returns card status from UIM RIL cache

    @return
    None
*/
/*=========================================================================*/
RIL_UIM_Errno qcril_uim_direct_get_card_status
(
  uint8                               slot,
  std::shared_ptr<RIL_UIM_CardStatus> ril_card_status_ptr
)
{
  RIL_UIM_Errno                  ret;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (ril_card_status_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, NULL ril_card_statusd");
    QCRIL_ASSERT(0);
    return RIL_UIM_E_INTERNAL_ERR;
  }

  if (slot >= QMI_UIM_MAX_CARD_COUNT)
  {
    QCRIL_LOG_ERROR("Invalid value, slot: 0x%x", slot);
    QCRIL_ASSERT(0);
    return RIL_UIM_E_INTERNAL_ERR;
  }

  qcril_uim_card_status_mutex.lock();

  if (qcril_uim.card_status.card[slot].card_state == QMI_UIM_CARD_STATE_UNKNOWN)
  {
    /* For an un-init card state, send an error */
    QCRIL_LOG_ERROR( "Card status is unknown for slot: 0x%x,\n", slot);
    ret = RIL_UIM_E_INVALID_SIM_STATE;
  }
  else
  {
    /* Construct RIL_CardStatus_v6 and log string */
    qcril_uim_construct_card_status(ril_card_status_ptr,
                                    &qcril_uim.card_status,
                                    slot);
    ret = RIL_UIM_E_SUCCESS;
  }
  qcril_uim_card_status_mutex.unlock();

  return ret;
}  /* qcril_uim_direct_get_card_status */


/*===========================================================================

  FUNCTION:  qcril_uim_process_recovery_ind

===========================================================================*/
/*!
    @brief
    Main function for processing QMI recovery complete indication.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_process_recovery_ind
(
  const qcril_uim_indication_params_type  * ind_param_ptr
)
{
  qmi_uim_slot_type                        slot    = QMI_UIM_SLOT_1;
  std::shared_ptr<UimSimInternalResetInd>  ind_ptr = NULL;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (ind_param_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid inputs, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  switch ((uint8_t)qmi_ril_get_process_instance_id())
  {
    case 0:
      slot = QMI_UIM_SLOT_1;
      break;

    case 1:
      slot = QMI_UIM_SLOT_2;
      break;

    case 2:
      slot = QMI_UIM_SLOT_3;
      break;

    default:
      QCRIL_LOG_ERROR("Invalid instance 0x%x", qmi_ril_get_process_instance_id());
      return;
  }

  if (slot != ind_param_ptr->ind_data.recovery_ind.slot)
  {
    return;
  }

  ind_ptr = std::make_shared<UimSimInternalResetInd>();

  if (ind_ptr)
  {
    ind_ptr->broadcast();
  }
} /* qcril_uim_process_recovery_ind */


/*===========================================================================

  FUNCTION:  qcril_uim_process_supply_voltage_ind

===========================================================================*/
/*!
    @brief
    Main function for processing QMI supply voltage indication.
    Note that the data type for QCRIL_EVT_HOOK_UNSOL_UICC_VOLTAGE_STATUS is
    RIL_UiccVoltageStatus

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_process_supply_voltage_ind
(
  const qcril_uim_indication_params_type  * ind_param_ptr
)
{
  qmi_uim_slot_type                     slot           = QMI_UIM_SLOT_1;
  RIL_UIM_UiccVoltageStatus             voltage_status = RIL_UIM_UICC_VOLTAGE_START_ACTIVATION;
  std::shared_ptr<UimVoltageStatusInd>  ind_ptr        = NULL;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (ind_param_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid inputs, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  QCRIL_LOG_INFO( "slot: 0x%x, vcc_command: 0x%x\n",
                  ind_param_ptr->ind_data.supply_voltage_ind.slot,
                  ind_param_ptr->ind_data.supply_voltage_ind.vcc_command);

  slot = (qmi_uim_slot_type)((uint8)QMI_UIM_SLOT_1 + qmi_ril_get_process_instance_id());

  /* We send an ACK from QCRIL direclty only for this condition: different slot
     than RILD instance & the vcc status is deactivate. Otherwise we send the UNSOL
     and expect the client to respond with an ACK */
  if ((slot != ind_param_ptr->ind_data.supply_voltage_ind.slot) &&
      (ind_param_ptr->ind_data.supply_voltage_ind.vcc_command == QMI_UIM_VCC_START_DEACTIVATION))
  {
    int                                ret = -1;
    qmi_uim_rsp_data_type              rsp_data;
    qmi_uim_supply_voltage_params_type vcc_params;

    memset(&rsp_data, 0x00, sizeof(qmi_uim_rsp_data_type));
    memset(&vcc_params, 0x00, sizeof(qmi_uim_supply_voltage_params_type));

    vcc_params.slot = ind_param_ptr->ind_data.supply_voltage_ind.slot;

    ret = qcril_uim_send_qmi_sync_msg(QCRIL_UIM_REQUEST_SUPPLY_VOLTAGE,
                                      &vcc_params,
                                      &rsp_data);
    if (ret < 0)
    {
      QCRIL_LOG_ERROR("Error for qcril_qmi_uim_supply_voltage, ret: 0x%x", ret);
    }
    return;
  }

  /* Map the status */
  switch (ind_param_ptr->ind_data.supply_voltage_ind.vcc_command)
  {
    case QMI_UIM_VCC_ACTIVATED:
      voltage_status = RIL_UIM_UICC_VOLTAGE_ACTIVATED;
      break;

    case QMI_UIM_VCC_START_DEACTIVATION:
      voltage_status = RIL_UIM_UICC_VOLTAGE_START_DEACTIVATION;
      break;

    default:
      QCRIL_LOG_ERROR("Unsupported vcc status 0x%x",
                      ind_param_ptr->ind_data.supply_voltage_ind.vcc_command);
      return;
  }

  /* Send OEMHook unsolicited response to framework */
  ind_ptr = std::make_shared<UimVoltageStatusInd>(voltage_status);

  if (ind_ptr != NULL)
  {
    ind_ptr->broadcast();
  }
} /* qcril_uim_process_supply_voltage_ind */


/*=========================================================================

  FUNCTION:  qcril_uim_request_voltage_status

===========================================================================*/
/*!
    @brief
     Handles QCRIL_EVT_HOOK_UICC_VOLTAGE_STATUS_REQ request from QCRIL.
     Note that both the request & response have no data payload.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_voltage_status
(
  const std::shared_ptr<UimVoltageStatusRequestMsg> req_ptr
)
{
  qmi_uim_supply_voltage_params_type   voltage_params  = {};
  std::shared_ptr<RIL_UIM_Errno>       rsp_ptr         = NULL;

  /* Sanity check */
  if (req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    return;
  }

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  voltage_params.slot = (qmi_uim_slot_type)(req_ptr->get_slot() + (uint8_t)QMI_UIM_SLOT_1);

  if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_SUPPLY_VOLTAGE,
                                   &voltage_params,
                                   req_ptr) >= 0)
  {
    return;
  }

  rsp_ptr = std::make_shared<RIL_UIM_Errno>(RIL_UIM_E_MODEM_ERR);
  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_request_voltage_status */


/*=========================================================================

  FUNCTION:  qcril_uim_request_uicc_power

===========================================================================*/
void qcril_uim_request_uicc_power
(
  const std::shared_ptr<UimCardPowerReqMsg> msg_ptr
)
{
  qmi_uim_slot_type                 slot_id         = QMI_UIM_SLOT_1;
  RIL_UIM_CardPowerState            power_req       = RIL_UIM_CARD_POWER_DOWN;
  std::shared_ptr<RIL_UIM_Errno>    err             = std::make_shared<RIL_UIM_Errno>(RIL_UIM_E_INTERNAL_ERR);
  qcril_uim_card_status_e_type      new_card_status = QCRIL_CARD_STATUS_POWERDOWN_FAILED;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (msg_ptr == NULL || err == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot process request");
    return;
  }

  power_req = msg_ptr->get_card_power_req();

  switch(msg_ptr->get_slot())
  {
    case 0:
      slot_id = QMI_UIM_SLOT_1;
      break;
    case 1:
      slot_id = QMI_UIM_SLOT_2;
      break;
    case 2:
      slot_id = QMI_UIM_SLOT_3;
      break;
    default:
      QCRIL_LOG_ERROR( " Invalid slot_id in input: 0x%x\n", msg_ptr->get_slot());
      *err = RIL_UIM_E_INVALID_ARGUMENTS;
      goto send_power_req_error;
  }

  if (power_req == RIL_UIM_CARD_POWER_DOWN)
  {
    qmi_uim_power_down_params_type power_down_params = {};

    power_down_params.slot = slot_id;

    if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_POWER_DOWN,
                                     &power_down_params,
                                     msg_ptr) >= 0)
    {
      return;
    }
    *err = RIL_UIM_E_MODEM_ERR;
  }
  else if (power_req == RIL_UIM_CARD_POWER_UP ||
           power_req == RIL_UIM_CARD_POWER_UP_PASS_THROUGH)
  {
    qmi_uim_power_up_params_type power_up_params;

    power_up_params.mode = QMI_UIM_CARD_MODE_TELECOM_CARD;
    power_up_params.slot = slot_id;

    if (power_req == RIL_UIM_CARD_POWER_UP_PASS_THROUGH)
    {
      power_up_params.mode = QMI_UIM_CARD_MODE_NON_TELECOM_CARD;
    }

    if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_POWER_UP,
                                     &power_up_params,
                                     msg_ptr) >= 0)
    {
      return;
    }
    *err = RIL_UIM_E_MODEM_ERR;
  }
  else
  {
    *err = RIL_UIM_E_INVALID_ARGUMENTS;
  }

send_power_req_error:
  /* Send indication to NAS about power request failure */
  if (power_req == RIL_UIM_CARD_POWER_DOWN || power_req == RIL_UIM_CARD_POWER_UP)
  {
    if(power_req == RIL_UIM_CARD_POWER_UP)
    {
      new_card_status = QCRIL_CARD_STATUS_POWERUP_FAILED;
    }

    qcril_uim_update_cm_card_status(msg_ptr->get_slot(), new_card_status);
  }
  msg_ptr->sendResponse(msg_ptr, Message::Callback::Status::SUCCESS, err);
} /* qcril_uim_request_uicc_power */


/*=========================================================================

  FUNCTION:  qcril_uim_cancel_card_state_check_timer

===========================================================================*/
/*!
    @brief
    Cancels the card state check timer

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_cancel_card_state_check_timer
(
  UimTimerCancelReason reason
)
{
  if (qcril_uim_card_state_check_timer_msg_ptr != NULL)
  {
    QCRIL_LOG_INFO("Cancelling card state check timer reason: %d", (int)reason);
    std::shared_ptr<int> reasonVal = std::make_shared<int>((int)reason);

    /* sendResponse will ensure proper disposition of Timer message and stopping of
       running timer with TimeKeeper by QCRIL framework dispatcher */
    qcril_uim_card_state_check_timer_msg_ptr->sendResponse(qcril_uim_card_state_check_timer_msg_ptr,
                                                           Message::Callback::Status::SUCCESS,
                                                           reasonVal);
  }
} /* qcril_uim_cancel_card_state_check_timer */


/*=========================================================================

  FUNCTION:  qcril_uim_to_uim_rmt_card_app_init_state

===========================================================================*/
/*!
    @brief
    Utility function to derive cardApp state for UIM remote test
    client usage purpose

    @return
    UimRmtCardAppInitStateType
*/
/*=========================================================================*/
UimRmtCardAppInitStateType qcril_uim_to_uim_rmt_card_app_init_state(
                                 qmi_uim_app_state_type    app_state,
                                 qmi_uim_perso_state_type  perso_state)
{
  UimRmtCardAppInitStateType cardAppState = UIM_RMT_APP_STATE_UNKNOWN;

  switch(app_state)
  {
    case QMI_UIM_APP_STATE_UNKNOWN:
      cardAppState = UIM_RMT_APP_STATE_UNKNOWN;
      break;

    case QMI_UIM_APP_STATE_DETECTED:
      cardAppState = UIM_RMT_APP_STATE_DETECTED;
      break;

    case QMI_UIM_APP_STATE_PIN_REQUIRED:
    case QMI_UIM_APP_STATE_PUK1_REQUIRED:
      cardAppState = UIM_RMT_APP_STATE_WAITING_ON_USER;
      break;

    case QMI_UIM_APP_STATE_PERSO:
      switch(perso_state)
      {
        case QMI_UIM_PERSO_STATE_CODE_REQUIRED:
        case QMI_UIM_PERSO_STATE_PUK_REQUIRED:
          cardAppState = UIM_RMT_APP_STATE_WAITING_ON_USER;
          break;

        case QMI_UIM_PERSO_STATE_PERM_BLOCKED:
          cardAppState = UIM_RMT_APP_STATE_HALTED;
          break;

        case QMI_UIM_PERSO_STATE_UNKNOWN:
        case QMI_UIM_PERSO_STATE_IN_PROGRESS:
        case QMI_UIM_PERSO_STATE_READY:
        default:
          cardAppState = UIM_RMT_APP_STATE_DETECTED;
          break;
      }
      break;

    case QMI_UIM_APP_STATE_BLOCKED:
    case QMI_UIM_APP_STATE_ILLEGAL:
      cardAppState = UIM_RMT_APP_STATE_HALTED;
      break;

    case QMI_UIM_APP_STATE_READY:
      cardAppState = UIM_RMT_APP_STATE_READY;
      break;

    default:
      cardAppState = UIM_RMT_APP_STATE_UNKNOWN;
      break;
  }

  QCRIL_LOG_INFO("In - app_state: %d perso_state: %d Out - cardAppState: %d",
                        (int)app_state, (int)perso_state ,(int)cardAppState);

  return cardAppState;
}
/*===========================================================================*/
/*!
    @brief
    Based on the indication received, this function checks only specific
    fields for the SIM, USIM, RUIM or CSIM Apps, derives a CARD Initialization
    status for UIM Remote internal test clients.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_determine_card_app_init_status_and_send_indication
( uint8                            slot,
  const qmi_uim_card_status_type * new_status_ptr
)
{
  uint8 i = 0;
  uint8 j = 0;
  bool simUsimAppFound  = false;
  bool ruimCsimAppFound = false;
  UimRmtCardAppInitStatusIndData cardAppInitIndData = {};
  UimRmtCardAppInitStateType cardAppState = UIM_RMT_APP_STATE_UNKNOWN;
  uint8                 instance_slot     = qmi_ril_get_sim_slot();

  /* Sanity checks */
  if(new_status_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  cardAppInitIndData.numOfActiveSlots = new_status_ptr->num_slots;

  for (i = 0; i < new_status_ptr->num_slots && i < QMI_UIM_MAX_CARD_COUNT; i++)
  {
    if(slot != instance_slot)
    {
      QCRIL_LOG_INFO("i = %d Slots are not matching Ind slot: %d RIL Instance slot: %d",
                                                                i, slot, instance_slot);
      continue;
    }

    QCRIL_LOG_INFO(" num_slots %d  i: %d  card_state: %d",
              new_status_ptr->num_slots, i , new_status_ptr->card[i].card_state);

    if(new_status_ptr->card[i].card_state == QMI_UIM_CARD_STATE_PRESENT)
    {
      for (j = 0; (j < new_status_ptr->card[i].num_app && j < QMI_UIM_MAX_APP_PER_CARD_COUNT)
                   &&(cardAppInitIndData.numOfApps <= MAX_UIM_RMT_CLIENT_APPS); j++)
      {
        if ((new_status_ptr->card[i].application[j].app_type == QMI_UIM_APP_SIM ||
             new_status_ptr->card[i].application[j].app_type == QMI_UIM_APP_USIM)
            && !simUsimAppFound )
        {
          simUsimAppFound = true;

          cardAppState = qcril_uim_to_uim_rmt_card_app_init_state(
                            new_status_ptr->card[i].application[j].app_state,
                            new_status_ptr->card[i].application[j].perso_state
                            );

          cardAppInitIndData.appInfo[cardAppInitIndData.numOfApps].appType  =
          (UimRmtCardAppType) new_status_ptr->card[i].application[j].app_type;

          cardAppInitIndData.appInfo[cardAppInitIndData.numOfApps].appState = cardAppState;

          QCRIL_LOG_INFO(" numOfApps %d  appType: %d  appState: %d",
              cardAppInitIndData.numOfApps,
              cardAppInitIndData.appInfo[cardAppInitIndData.numOfApps].appType,
              cardAppInitIndData.appInfo[cardAppInitIndData.numOfApps].appState );

          /*Increment the numOfApps after updating appType and appState at app index */
          cardAppInitIndData.numOfApps++;
        }
        else if((new_status_ptr->card[i].application[j].app_type == QMI_UIM_APP_RUIM ||
                 new_status_ptr->card[i].application[j].app_type == QMI_UIM_APP_CSIM)
                 && !ruimCsimAppFound )
        {
          ruimCsimAppFound = true;
          cardAppState = qcril_uim_to_uim_rmt_card_app_init_state(
                            new_status_ptr->card[i].application[j].app_state,
                            new_status_ptr->card[i].application[j].perso_state
                            );

          cardAppInitIndData.appInfo[cardAppInitIndData.numOfApps].appType  =
          (UimRmtCardAppType) new_status_ptr->card[i].application[j].app_type;

          cardAppInitIndData.appInfo[cardAppInitIndData.numOfApps].appState = cardAppState;

          QCRIL_LOG_INFO(" numOfApps %d  appType: %d  appState: %d",
              cardAppInitIndData.numOfApps,
              cardAppInitIndData.appInfo[cardAppInitIndData.numOfApps].appType,
              cardAppInitIndData.appInfo[cardAppInitIndData.numOfApps].appState );
          /*Increment the numOfApps after updating appType and appState at app index */
          cardAppInitIndData.numOfApps++;
        }
        else
        {
          continue;
        }
      }
    }
  }


  std::shared_ptr<UimRmtCardAppInitStatusIndMsg> indMsgPtr =
       std::make_shared<UimRmtCardAppInitStatusIndMsg>(cardAppInitIndData);
  if (indMsgPtr != nullptr)
  {
    QCRIL_LOG_INFO( " uim_remote_card_app_init_status_indication Sent!\n");
    indMsgPtr->broadcast();
  }
  else
  {
    QCRIL_LOG_ERROR( "uim_remote_card_app_init_status_indication Failed!\n");
  }
} /* qcril_uim_determine_card_app_init_status_and_send_indication */
