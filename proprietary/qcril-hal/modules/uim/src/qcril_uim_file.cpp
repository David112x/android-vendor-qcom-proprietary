/*===========================================================================

  Copyright (c) 2017 - 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include "framework/Log.h"
#include "modules/uim/UimGetMccMncRequestMsg.h"
#include "UimTimerExpiryMsg.h"
#include "UimTimerRequestMsg.h"
#include "qcril_uim.h"
#include "qcril_uim_util.h"
#include "qcril_uim_file.h"
#include "util_timer.h"
#include <string.h>

extern bool qcril_uim_log_adb_on;

/*===========================================================================

                           DEFINES

===========================================================================*/
#define TAG "QCRIL_UIM_FILE"

/* SIM IO commands */
#define SIM_CMD_READ_BINARY                         176
#define SIM_CMD_READ_RECORD                         178
#define SIM_CMD_GET_RESPONSE                        192
#define SIM_CMD_UPDATE_BINARY                       214
#define SIM_CMD_RETRIEVE_DATA                       203
#define SIM_CMD_SET_DATA                            219
#define SIM_CMD_UPDATE_RECORD                       220
#define SIM_CMD_STATUS                              242

#define QCRIL_UIM_IMSI_PATH_SIZE                    4
#define QCRIL_UIM_FILEID_EF_IMSI                    0x6F07
#define QCRIL_UIM_FILEID_EF_IMSI_M                  0x6F22
#define QCRIL_UIM_FILEID_EF_AD                      0x6FAD
#define QCRIL_UIM_IMSI_M_RAW_SIZE                   10
#define QCRIL_UIM_IMSI_M_PARSED_SIZE                16

#define QCRIL_UIM_APDU_MIN_P3                       0
#define QCRIL_UIM_APDU_MIN_SIZE                     4
#define QCRIL_UIM_APDU_MIN_SIZE_PLUS_P3             5
#define QCRIL_UIM_LONG_APDU_MIN_SIZE_PLUS_P3        7
#define QCRIL_UIM_APDU_MAX_SHORT_APDU_SIZE          256
#define QCRIL_UIM_APDU_MAX_LONG_APDU_SIZE           65536
#define QCRIL_UIM_INS_BYTE_GET_RESPONSE             0xC0
#define QCRIL_UIM_CHANNEL_ID_MAX_VAL                19
#define QCRIL_UIM_INVALID_INS_BYTE_MASK             0x60
#define QCRIL_UIM_SW1_INS_CODE_NOT_SUPPORTED        0x6D
#define QCRIL_UIM_SW2_NORMAL_END                    0x00
#define QCRIL_UIM_INS_BYTE_SELECT                   0xA4
#define QCRIL_UIM_P1_VALUE_SELECT_BY_DF_NAME        0x04
#define QCRIL_UIM_P2_MASK_SELECT_NEXT               0x02

/* Get file attributes response defines */
/* Default value: not invalidated, readable and updatable when invalidated */
#define QCRIL_UIM_GET_RESPONSE_MIN_SIZE             14
#define QCRIL_UIM_GET_RESPONSE_FILE_STATUS          0X05
#define QCRIL_UIM_TAG_FCP_TEMPLATE                  0x62
#define QCRIL_UIM_TAG_LIFE_CYCLE_STATUS             0x8A

#define QCRIL_UIM_AUTH_GSM_CONTEXT                  0x80
#define QCRIL_UIM_AUTH_3G_CONTEXT                   0x81
#define QCRIL_UIM_AUTH_VGCS_VBS_SECURITY_CONTEXT    0x82

#define QCRIL_UIM_AUTH_IMS_AKA                      0x81
#define QCRIL_UIM_AUTH_HTTP_DIGEST_SECURITY_CONTEXT 0x82

#define QCRIL_UIM_SW1_WRONG_PARAMS                  0x6A
#define QCRIL_UIM_SW2_BAD_PARAMS_P1_P2              0x86

#define QCRIL_UIM_SW1_TECH_PROBLEM                  0X6F
#define QCRIL_UIM_SW2_TECH_PROBLEM                  0x00

#define QCRIL_UIM_ARA_M_APP_OPEN_WAIT_TIME          3

#define QCRIL_UIM_INVALID_OPEN_CH_P2                -1

#define QCRIL_UIM_FILEID_EF_GID1                    0x6F3E
#define QCRIL_UIM_FILEID_EF_GID2                    0x6F3F


/*===========================================================================
  structures
===========================================================================*/
/*===========================================================================
   STRUCT:      QCRIL_UIM_CARD_ARA_M_DELAY_TIMER_TYPE

   DESCRIPTION:
     Stores the info needed during the ARA-M app open delay
===========================================================================*/
struct
{
  std::shared_ptr<UimTimerRequestMsg> timer_msg_ptr;
  std::shared_ptr<UimSIMOpenChannelRequestMsg> req_ptr;
} qcril_uim_ara_m_delay_timer = {NULL, NULL};


/*===========================================================================
                               FUNCTION DECLARATIONS
===========================================================================*/
static void qcril_uim_resend_ara_m_cmd_timer_cb
(
  void *param_ptr
);

/*===========================================================================

                               INTERNAL FUNCTIONS

===========================================================================*/

/*=========================================================================

  FUNCTION:  qcril_uim_get_file_status_byte

===========================================================================*/
/*!
    @brief
    This function parses the raw response data for get file attibutes to
    check for the Life cycle status integer. This byte is mapped to the
    file status byte coding defined in 51.011, Section 9.2.1

    @return
    Mapped File status byte
*/
/*=========================================================================*/
static uint8 qcril_uim_get_file_status_byte
(
  uint16                         data_len,
  const uint8                  * data_ptr
)
{
  uint16 data_index = 0;
  uint8  file_status_byte = QCRIL_UIM_GET_RESPONSE_FILE_STATUS;

  if ((data_ptr == NULL) || (data_len == 0))
  {
    QCRIL_LOG_ERROR("Invalid input, returning default value 0x05");
    return file_status_byte;
  }

  /* Parse through the raw response to find the TLV */
  if (data_ptr[data_index] != QCRIL_UIM_TAG_FCP_TEMPLATE)
  {
    QCRIL_LOG_ERROR("Invalid FCP template tag: 0x%X", data_ptr[data_index]);
    return file_status_byte;
  }

  /* Byte 2 & 3 has the total length of FCP Template
     We need to udpate the data_index appropriately  */
  if (data_len >= 2 && data_ptr[1] <= 0x7F)
  {
    if (data_len < (data_ptr[1] + 2))
    {
      return file_status_byte;
    }
    data_len    = data_ptr[1] + 2;
    data_index += 2;
  }
  else if (data_len >= 3 && data_ptr[1] == 0x81)
  {
    if (data_len < (data_ptr[2] + 3))
    {
      return file_status_byte;
    }
    data_len   = data_ptr[2] + 3;
    data_index += 3;
  }
  else
  {
    return file_status_byte;
  }

  /* Parse through the raw response to find the TLV */
  while ((data_index+1) < data_len)
  {
    uint8 tag_len   = 0;
    uint8 read_byte = 0;

    switch (data_ptr[data_index])
    {
      case QCRIL_UIM_TAG_LIFE_CYCLE_STATUS:
        /* Sanity check the length */
        tag_len = data_ptr[data_index+1];
        if (((data_index + 2 + tag_len) > data_len) ||
             (tag_len != 1))
        {
          /* Incorrect data length, cannot parse further */
          QCRIL_LOG_ERROR("Incorrect tag length, cannot parse further: 0x%X \n",
                          tag_len);
          return file_status_byte;
        }
        read_byte = data_ptr[data_index+2];
        QCRIL_LOG_INFO( "Life cycle status integer byte: 0x%X", read_byte);
        /* Check for bit b1 and these condition:
           1. b3 should be set for operational state
           2. b4 - b8 should be 0s */
        if ((read_byte <= 0x07) && (read_byte >= 0x04))
        {
          if (read_byte & 0x01)
          {
            /* Not Invalidated, readable/updatable when invalidated */
            file_status_byte = 0x05;
          }
          else
          {
            /* Invalidated, not readable/updatable when invalidated */
            file_status_byte = 0x00;
          }
        }
        /* Nothing else to do, return */
        return file_status_byte;

      default:
        /* Tag that we are not interested in, move to next one if avaliable */
        tag_len = data_ptr[data_index+1];
        data_index += tag_len + 2;
        break;
    }
  }

  return file_status_byte;
} /* qcril_uim_get_file_status_byte */


/*=========================================================================

  FUNCTION:  qcril_uim_compose_apdu_data

===========================================================================*/
/*!
    @brief
    Function to compose raw APDU command. Composed data pointer and length
    are updated based on the request.

    @return
    TRUE if successful, FALSE otherwise.
*/
/*=========================================================================*/
static boolean qcril_uim_compose_apdu_data
(
  qmi_uim_data_type       * apdu_data_ptr,
  int                       cla,
  int                       ins,
  int                       p1,
  int                       p2,
  int                       p3,
  const char              * data_ptr
)
{
  qmi_uim_data_type     binary_apdu_data;
  uint16                total_apdu_len = 0;

  if ((apdu_data_ptr == NULL) ||
      (apdu_data_ptr->data_ptr == NULL) ||
      (apdu_data_ptr->data_len == 0))
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return FALSE;
  }

  memset(apdu_data_ptr->data_ptr, 0, apdu_data_ptr->data_len);
  memset(&binary_apdu_data, 0, sizeof(qmi_uim_data_type));

  total_apdu_len = apdu_data_ptr->data_len;

  /* Update mandatory parameters - CLA, INS, P1 & P2 */
  if (total_apdu_len >= QCRIL_UIM_APDU_MIN_SIZE)
  {
    apdu_data_ptr->data_ptr[0] = (uint8)(cla & 0xFF);
    apdu_data_ptr->data_ptr[1] = (uint8)(ins & 0xFF);
    apdu_data_ptr->data_ptr[2] = (uint8)(p1 & 0xFF);
    apdu_data_ptr->data_ptr[3] = (uint8)(p2 & 0xFF);
    apdu_data_ptr->data_len = QCRIL_UIM_APDU_MIN_SIZE;
  }

  /* Update P3 parameter if valid */
  if (p3 > QCRIL_UIM_APDU_MIN_P3 &&
      p3 < QCRIL_UIM_APDU_MAX_SHORT_APDU_SIZE)
  {
    apdu_data_ptr->data_ptr[4] = (uint8)(p3 & 0xFF);
    apdu_data_ptr->data_len = QCRIL_UIM_APDU_MIN_SIZE_PLUS_P3;
  }
  else if (p3 >= QCRIL_UIM_APDU_MAX_SHORT_APDU_SIZE)
  {
    apdu_data_ptr->data_ptr[4] = (uint8)(0x00);
    apdu_data_ptr->data_ptr[5] = (uint8)((p3 >> 8) & 0xFF);
    apdu_data_ptr->data_ptr[6] = (uint8)(p3 & 0xFF);
    apdu_data_ptr->data_len = QCRIL_UIM_LONG_APDU_MIN_SIZE_PLUS_P3;
  }

  /* Update data parameter if valid */
  if((p3 >= QCRIL_UIM_APDU_MAX_SHORT_APDU_SIZE) &&
      (total_apdu_len == QCRIL_UIM_LONG_APDU_MIN_SIZE_PLUS_P3))
  {
    QCRIL_LOG_INFO("Case - 2 Extended APDU p3: 0x%x",
                 p3);
  }
  else if (total_apdu_len > QCRIL_UIM_APDU_MIN_SIZE_PLUS_P3)
  {
    if ((data_ptr == NULL) || (strlen(data_ptr) == 0))
    {
      QCRIL_LOG_ERROR("Mismatch in total_apdu_len & input APDU data!");
      return FALSE;
    }

    binary_apdu_data.data_ptr = qcril_uim_alloc_hexstring_to_bin(data_ptr,
                                                                 &binary_apdu_data.data_len);
    if (binary_apdu_data.data_ptr == NULL)
    {
      QCRIL_LOG_ERROR("Unable to convert input APDU data!");
      return FALSE;
    }

    /* Update data parameter if valid */
    if (binary_apdu_data.data_len <= (total_apdu_len - QCRIL_UIM_APDU_MIN_SIZE_PLUS_P3))
    {
      if (total_apdu_len >= QCRIL_UIM_APDU_MIN_SIZE_PLUS_P3 &&
          total_apdu_len <= QCRIL_UIM_APDU_MIN_SIZE_PLUS_P3 + QCRIL_UIM_APDU_MAX_SHORT_APDU_SIZE)
      {
        memcpy(&apdu_data_ptr->data_ptr[5], binary_apdu_data.data_ptr, binary_apdu_data.data_len);
        apdu_data_ptr->data_len = QCRIL_UIM_APDU_MIN_SIZE_PLUS_P3 + binary_apdu_data.data_len;
      }
      else if (total_apdu_len > QCRIL_UIM_APDU_MIN_SIZE_PLUS_P3 + QCRIL_UIM_APDU_MAX_SHORT_APDU_SIZE)
      {
        memcpy(&apdu_data_ptr->data_ptr[7], binary_apdu_data.data_ptr, binary_apdu_data.data_len);
        apdu_data_ptr->data_len = QCRIL_UIM_LONG_APDU_MIN_SIZE_PLUS_P3 + binary_apdu_data.data_len;
      }
    }

    /* Free temp buffer */
    qcril_free(binary_apdu_data.data_ptr);
    binary_apdu_data.data_ptr = NULL;
  }

  return TRUE;
} /* qcril_uim_compose_apdu_data */


/*=========================================================================

  FUNCTION:  qcril_uim_remove_select_response_info_entry

===========================================================================*/
/*!
    @brief
    Function that clears the select response data corresponding to the client
    id.

    @return
    void
*/
/*=========================================================================*/
static void qcril_uim_remove_select_response_info_entry
(
  int channel_id
)
{
  uint8  i                 = 0;

  if ((channel_id == 0) ||
      (channel_id > QCRIL_UIM_CHANNEL_ID_MAX_VAL))
  {
    QCRIL_LOG_ERROR("Invalid channel_id: 0x%x", channel_id);
    return;
  }

  QCRIL_LOG_INFO("Clearing select response info entry channel_id: 0x%x",
                 channel_id);

  for (i = 0; i < QCRIL_UIM_MAX_SELECT_RESP_COUNT; i++)
  {
    if (qcril_uim.select_response_info[i].in_use == TRUE &&
        qcril_uim.select_response_info[i].channel_id == channel_id)
    {
      if (qcril_uim.select_response_info[i].select_resp_ptr != NULL)
      {
        qcril_free(qcril_uim.select_response_info[i].select_resp_ptr);
        qcril_uim.select_response_info[i].select_resp_ptr = NULL;
      }

      memset(&qcril_uim.select_response_info[i],
             0x00,
             sizeof(qcril_uim_select_response_info_type));
    }
  }
} /* qcril_uim_remove_select_response_info_entry */


/*=========================================================================

  FUNCTION:  qcril_uim_store_select_response_info

===========================================================================*/
/*!
    @brief
    Function that temporarily caches the select response data if available in
    a global variable. It is cleaned upon the next APDU request by the client
    in any circumstance.

    @return
    RIL_UIM_E_SUCCESS if successful, RIL_E_GENERIC_FAILURE otherwise.
*/
/*=========================================================================*/
static RIL_UIM_Errno qcril_uim_store_select_response_info
(
  const qmi_uim_logical_channel_rsp_type   * logical_ch_rsp_ptr
)
{
  uint8  select_resp_index = QCRIL_UIM_MAX_SELECT_RESP_COUNT;

  if ((logical_ch_rsp_ptr->channel_id == 0) ||
      (logical_ch_rsp_ptr->channel_id > QCRIL_UIM_CHANNEL_ID_MAX_VAL))
  {
    QCRIL_LOG_ERROR("Invalid channel_id: 0x%x", logical_ch_rsp_ptr->channel_id);
    return RIL_UIM_E_INTERNAL_ERR;
  }

  for (uint8 i = 0; i < QCRIL_UIM_MAX_SELECT_RESP_COUNT; i++)
  {
    if (qcril_uim.select_response_info[i].in_use == FALSE)
    {
      select_resp_index = i;
      break;
    }
  }

  if (select_resp_index == QCRIL_UIM_MAX_SELECT_RESP_COUNT)
  {
    QCRIL_LOG_ERROR("Couldnt get select resp array index !");
    return RIL_UIM_E_NO_RESOURCES;
  }

  /* Allocate & save the data */
  QCRIL_LOG_INFO("Storing logical_channel_rsp for select_resp_index 0x%x, select_resp_len: 0x%x",
                 select_resp_index, logical_ch_rsp_ptr->select_response.data_len);

  memset(&qcril_uim.select_response_info[select_resp_index],
         0,
         sizeof(qcril_uim_select_response_info_type));

  /* In some cases, we may have no actual select response,
     so store only the channel & SW info */
  if ((logical_ch_rsp_ptr->select_response.data_len > 0) &&
      (logical_ch_rsp_ptr->select_response.data_ptr != NULL))
  {
    qcril_uim.select_response_info[select_resp_index].select_resp_ptr =
      (uint8 *)qcril_malloc(logical_ch_rsp_ptr->select_response.data_len);
    if (qcril_uim.select_response_info[select_resp_index].select_resp_ptr == NULL)
    {
      QCRIL_LOG_ERROR("Couldnt allocate select resp array index !");
      return RIL_UIM_E_NO_MEMORY;
    }
    qcril_uim.select_response_info[select_resp_index].select_resp_len =
      logical_ch_rsp_ptr->select_response.data_len;
    memcpy(qcril_uim.select_response_info[select_resp_index].select_resp_ptr,
           logical_ch_rsp_ptr->select_response.data_ptr,
           logical_ch_rsp_ptr->select_response.data_len);
  }

  qcril_uim.select_response_info[select_resp_index].in_use     = TRUE;
  qcril_uim.select_response_info[select_resp_index].sw1        = logical_ch_rsp_ptr->sw1;
  qcril_uim.select_response_info[select_resp_index].sw2        = logical_ch_rsp_ptr->sw2;
  qcril_uim.select_response_info[select_resp_index].channel_id = logical_ch_rsp_ptr->channel_id;

  return RIL_UIM_E_SUCCESS;
} /* qcril_uim_store_select_response_info */


/*=========================================================================

  FUNCTION:  qcril_uim_update_get_response_apdu

===========================================================================*/
/*!
    @brief
    Function to compose a get response APDU command based on what is cached
    in the global select response info data.

    @return
    RIL_UIM_E_SUCCESS if successful, RIL_E_GENERIC_FAILURE otherwise.
*/
/*=========================================================================*/
static RIL_UIM_Errno qcril_uim_update_get_response_apdu
(
  uint8                        select_resp_index,
  RIL_UIM_SIM_IO_Response    * ril_response_ptr
)
{
  if (ril_response_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    return RIL_UIM_E_INTERNAL_ERR;
  }

  if (select_resp_index >= QCRIL_UIM_MAX_SELECT_RESP_COUNT)
  {
    QCRIL_LOG_ERROR("Invalid input, select_resp_index: 0x%x", select_resp_index);
    return RIL_UIM_E_INTERNAL_ERR;
  }

  QCRIL_LOG_INFO("Updating get_response from select_resp_index 0x%x, select_resp_len: 0x%x",
                 select_resp_index,
                 qcril_uim.select_response_info[select_resp_index].select_resp_len);

  /* Fill RIL_SIM_IO_Response with necesary info */
  ril_response_ptr->sw1 = qcril_uim.select_response_info[select_resp_index].sw1;
  ril_response_ptr->sw2 = qcril_uim.select_response_info[select_resp_index].sw2;

  if ((qcril_uim.select_response_info[select_resp_index].select_resp_ptr != NULL) &&
      (qcril_uim.select_response_info[select_resp_index].select_resp_len != 0))
  {
    char *simResponse = NULL;

    simResponse = qcril_uim_alloc_bin_to_hexstring(
      qcril_uim.select_response_info[select_resp_index].select_resp_ptr,
      qcril_uim.select_response_info[select_resp_index].select_resp_len);

    if (simResponse != NULL)
    {
      ril_response_ptr->simResponse = simResponse;
      qcril_free(simResponse);
      simResponse = NULL;
    }
  }

  return RIL_UIM_E_SUCCESS;
} /* qcril_uim_update_get_response_apdu */


/*=========================================================================

  FUNCTION:  qcril_uim_read_binary

===========================================================================*/
/*!
    @brief
    Function to read transparent

    @return
    None
*/
/*=========================================================================*/
static void qcril_uim_read_binary
(
  const std::shared_ptr<UimSIMIORequestMsg> req_ptr
)
{
  int                                   res         = -1;
  RIL_UIM_Errno                         err         = RIL_UIM_E_SUCCESS;
  qmi_uim_read_transparent_params_type  read_params = {};
  uint8                                 aid[QCRIL_UIM_MAX_AID_SIZE]   = {};
  uint8                                 path[QCRIL_UIM_MAX_PATH_SIZE] = {};
  uint16                                first_level_df_path           = 0;
  uint8                                 slot                          = QCRIL_UIM_INVALID_SLOT_INDEX_VALUE;
  qcril_uim_callback_params_type        callback_params               = {};
  const RIL_UIM_SIM_IO                * io_ptr                        = NULL;
  std::shared_ptr<UimReadTransparentReqMsg>  req_msg_ptr              = NULL;
  std::shared_ptr<RIL_UIM_SIM_IO_Response>   rsp_ptr                  = NULL;

  if(req_ptr == NULL ||
     req_ptr->get_sim_io_data() == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  io_ptr = req_ptr->get_sim_io_data();

  /* Fetch slot info */
  slot = req_ptr->get_slot();
  if(slot >= QMI_UIM_MAX_CARD_COUNT)
  {
    QCRIL_LOG_ERROR("Invalid slot %d", slot);
    QCRIL_ASSERT(0);
    return;
  }

  /* Sanity check */
  if ((io_ptr->p1 < 0) || (io_ptr->p1 > 0xFF) ||
      (io_ptr->p2 < 0) || (io_ptr->p2 > 0xFF) ||
      (io_ptr->p3 < 0))
  {
    QCRIL_LOG_ERROR( "Unsupported case, P1: 0x%X, P2: 0x%X, P3: 0x%X \n",
                     io_ptr->p1, io_ptr->p2, io_ptr->p3);
    err = RIL_UIM_E_INVALID_ARGUMENTS;
    goto send_read_binary_error;
  }

  /* File id */
  err = qcril_uim_extract_file_id(io_ptr,
                                  &read_params.file_id,
                                  path,
                                  sizeof(path));
  if (err != RIL_UIM_E_SUCCESS)
  {
    goto send_read_binary_error;
  }

  /* Extract the first level DF */
  if ((read_params.file_id.path.data_len >= 4) && (read_params.file_id.path.data_ptr))
  {
    /* Interested only in 3rd & 4th bytes */
    first_level_df_path = (*(read_params.file_id.path.data_ptr + 2) << 8) |
                          (*(read_params.file_id.path.data_ptr + 3));
  }

  /* Session information */
  err = qcril_uim_extract_session_type(slot,
                                       io_ptr->aidPtr.c_str(),
                                       first_level_df_path,
                                       &read_params.session_info,
                                       aid,
                                       sizeof(aid));
  if (err != RIL_UIM_E_SUCCESS)
  {
    goto send_read_binary_error;
  }

  QCRIL_LOG_INFO( "Session type found: %d", read_params.session_info.session_type);

  /* Read parameters: length and offset */
  read_params.offset = (io_ptr->p1) << 8 | (io_ptr->p2);
  read_params.length = io_ptr->p3;

  req_msg_ptr = std::make_shared<UimReadTransparentReqMsg>(req_ptr,
                                                           read_params);

  if (req_msg_ptr == NULL)
  {
    err = RIL_UIM_E_NO_MEMORY;
    goto send_read_binary_error;
  }

  if (!io_ptr->pin2.empty())
  {
    qmi_uim_verify_pin_params_type         verify_pin_params    = {};
    std::shared_ptr<UimVerifyPIN2Msg>      msg_ptr              = NULL;

    /* Session information: same as for read, except card sessions */
    verify_pin_params.session_info = read_params.session_info;

    /* We try to fake a GSM/CDMA first level DF path only for PIN2 verification */
    if ((read_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
        (read_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
        (read_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3))
    {
      err = qcril_uim_extract_session_type(slot,
                                           io_ptr->aidPtr.c_str(),
                                           QCRIL_UIM_FILEID_DF_CDMA,
                                           &verify_pin_params.session_info,
                                           NULL,
                                           0);
      if ((err != RIL_UIM_E_SUCCESS) ||
          (verify_pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
          (verify_pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
          (verify_pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3))
      {
        err = qcril_uim_extract_session_type(slot,
                                             io_ptr->aidPtr.c_str(),
                                             QCRIL_UIM_FILEID_DF_GSM,
                                             &verify_pin_params.session_info,
                                             NULL,
                                             0);
        if ((err != RIL_UIM_E_SUCCESS) ||
            (verify_pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
            (verify_pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
            (verify_pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3))
        {
          err = RIL_UIM_E_REQUEST_NOT_SUPPORTED;
          goto send_read_binary_error;
        }
      }
      QCRIL_LOG_INFO( "Provisioning session type found: %d",
                      verify_pin_params.session_info.session_type);
    }

    /* PIN id: PIN2 */
    verify_pin_params.pin_id = QMI_UIM_PIN_ID_PIN2;

    /* PIN value */
    verify_pin_params.pin_data.data_len = io_ptr->pin2.length();
    verify_pin_params.pin_data.data_ptr = (uint8*)io_ptr->pin2.c_str();

    msg_ptr = std::make_shared<UimVerifyPIN2Msg>(req_msg_ptr,
                                                 verify_pin_params.session_info);

    if (msg_ptr == NULL)
    {
      err = RIL_UIM_E_NO_MEMORY;
      goto send_read_binary_error;
    }

    res = qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_VERIFY_PIN2,
                                       (void *)&verify_pin_params,
                                       msg_ptr);
    if (res < 0)
    {
      QCRIL_LOG_ERROR("error in qmi_uim_verify_pin!");
      err = qcril_uim_convert_err_value(res);
      goto send_read_binary_error;
    }

    return;
  }

  /* If read request is for EF-ICCID and session type is card session, send
     response from cached ICCID file */
  if (((read_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
       (read_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
       (read_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3)) &&
      (read_params.file_id.file_id == QCRIL_UIM_EF_ICCID))
  {
    callback_params.qmi_rsp_data.qmi_err_code = QMI_UIM_SERVICE_ERR_INTERNAL;

    if (qcril_uim.card_status.card[slot].card_state == QMI_UIM_CARD_STATE_PRESENT &&
        qcril_uim.card_status.card[slot].iccid_len > 0 &&
        read_params.length + read_params.offset <= qcril_uim.card_status.card[slot].iccid_len)
    {
      unsigned short read_length = 0;

      if(read_params.length == 0)
      {
        read_length = qcril_uim.card_status.card[slot].iccid_len - read_params.offset;
      }
      else
      {
        read_length = read_params.length;
      }
      callback_params.qmi_rsp_data.rsp_data.read_transparent_rsp.content.data_ptr =
        (unsigned char *)qcril_malloc(read_length);

      if(callback_params.qmi_rsp_data.rsp_data.read_transparent_rsp.content.data_ptr != NULL)
      {
        memcpy(callback_params.qmi_rsp_data.rsp_data.read_transparent_rsp.content.data_ptr,
               &qcril_uim.card_status.card[slot].iccid[read_params.offset],
               read_length);
        callback_params.qmi_rsp_data.rsp_data.read_transparent_rsp.content.data_len =
          read_length;
        callback_params.qmi_rsp_data.qmi_err_code = RIL_UIM_E_SUCCESS;
      }
    }

    if(callback_params.qmi_rsp_data.qmi_err_code == RIL_UIM_E_SUCCESS)
    {
      qcril_uim_read_binary_resp(&callback_params, req_msg_ptr);

      /* Client needs to free the memory for raw data */
      if(callback_params.qmi_rsp_data.rsp_data.read_transparent_rsp.content.data_ptr)
      {
        qcril_free(callback_params.qmi_rsp_data.rsp_data.read_transparent_rsp.content.data_ptr);
      }

      return;
    }
  }

  if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_READ_TRANSPARENT,
                                   (void *)&read_params,
                                   req_msg_ptr) >= 0)
  {
    return;
  }
  err = RIL_UIM_E_MODEM_ERR;

send_read_binary_error:
  rsp_ptr = std::make_shared<RIL_UIM_SIM_IO_Response>();

  if (rsp_ptr != NULL)
  {
    rsp_ptr->slot_id = slot;
    rsp_ptr->err = err;
  }

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_read_binary */


/*=========================================================================

  FUNCTION:  qcril_uim_read_record

===========================================================================*/
/*!
    @brief
    Function to read record

    @return
    None
*/
/*=========================================================================*/
static void qcril_uim_read_record
(
  const std::shared_ptr<UimSIMIORequestMsg> req_ptr
)
{
  int                               res                           = -1;
  RIL_UIM_Errno                     err                           = RIL_UIM_E_SUCCESS;
  qmi_uim_read_record_params_type   read_params                   = {};
  uint8                             aid[QCRIL_UIM_MAX_AID_SIZE]   = {};
  uint8                             path[QCRIL_UIM_MAX_PATH_SIZE] = {};
  uint16                            first_level_df_path           = 0;
  uint8                             slot =  QCRIL_UIM_INVALID_SLOT_INDEX_VALUE;
  const RIL_UIM_SIM_IO            * io_ptr                        = NULL;
  std::shared_ptr<UimReadRecordReqMsg>       req_msg_ptr          = NULL;
  std::shared_ptr<RIL_UIM_SIM_IO_Response>   rsp_ptr              = NULL;

  if(req_ptr == NULL ||
     req_ptr->get_sim_io_data() == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  io_ptr = req_ptr->get_sim_io_data();
  /* Fetch slot info */
  slot = req_ptr->get_slot();
  if(slot >= QMI_UIM_MAX_CARD_COUNT)
  {
    QCRIL_LOG_ERROR("Invalid slot %d", slot);
    QCRIL_ASSERT(0);
    return;
  }

  /* Sanity check */
  if ((io_ptr->p1 < 0) || (io_ptr->p1 > 0xFF) ||
      (io_ptr->p2 < 0) || (io_ptr->p2 > 0xFF) ||
      (io_ptr->p3 < 0) || (io_ptr->p3 > 0xFF))
  {
    QCRIL_LOG_ERROR( "Unsupported case, P1: 0x%X, P2: 0x%X, P3: 0x%X \n",
                     io_ptr->p1, io_ptr->p2, io_ptr->p3);
    err = RIL_UIM_E_INVALID_ARGUMENTS;
    goto send_read_record_error;
  }

  /* File id */
  err = qcril_uim_extract_file_id(io_ptr,
                                  &read_params.file_id,
                                  path,
                                  sizeof(path));
  if (err != RIL_UIM_E_SUCCESS)
  {
    goto send_read_record_error;
  }

  /* Extract the first level DF */
  if ((read_params.file_id.path.data_len >= 4) && (read_params.file_id.path.data_ptr))
  {
    /* Interested only in 3rd & 4th bytes */
    first_level_df_path = (*(read_params.file_id.path.data_ptr + 2) << 8) |
                          (*(read_params.file_id.path.data_ptr + 3));
  }

  /* Session information */
  err = qcril_uim_extract_session_type(slot,
                                       io_ptr->aidPtr.c_str(),
                                       first_level_df_path,
                                       &read_params.session_info,
                                       aid,
                                       sizeof(aid));
  if (err != RIL_UIM_E_SUCCESS)
  {
    goto send_read_record_error;
  }

  QCRIL_LOG_INFO( "Session type found: %d", read_params.session_info.session_type);

  /* Read parameters: length and record */
  read_params.record = io_ptr->p1;
  read_params.length = io_ptr->p3;

  /* p2 can take values 2, 3, 4, per 3GPP TS 51.011, however QCRIL
   * does not support next record (2) or previous record (3) reads */
  if (io_ptr->p2 != 4)
  {
    QCRIL_LOG_ERROR( "%s: unsupported case P2 = %d\n", __FUNCTION__, io_ptr->p2);
    err = RIL_UIM_E_REQUEST_NOT_SUPPORTED;
    goto send_read_record_error;
  }

  req_msg_ptr = std::make_shared<UimReadRecordReqMsg>(req_ptr, read_params);

  if (req_msg_ptr == NULL)
  {
    err = RIL_UIM_E_NO_MEMORY;
    goto send_read_record_error;
  }

  if (!io_ptr->pin2.empty())
  {
    qmi_uim_verify_pin_params_type         verify_pin_params = {};
    std::shared_ptr<UimVerifyPIN2Msg>      msg_ptr           = NULL;

    /* Session information: same as for read, except card sessions */
    verify_pin_params.session_info = read_params.session_info;

    /* We try to fake a GSM/CDMA first level DF path only for PIN2 verification */
    if ((read_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
        (read_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
        (read_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3))
    {
      err = qcril_uim_extract_session_type(slot,
                                           io_ptr->aidPtr.c_str(),
                                           QCRIL_UIM_FILEID_DF_CDMA,
                                           &verify_pin_params.session_info,
                                           NULL,
                                           0);
      if ((err != RIL_UIM_E_SUCCESS) ||
          (verify_pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
          (verify_pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
          (verify_pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3))
      {
        err = qcril_uim_extract_session_type(slot,
                                             io_ptr->aidPtr.c_str(),
                                             QCRIL_UIM_FILEID_DF_GSM,
                                             &verify_pin_params.session_info,
                                             NULL,
                                             0);
        if ((err != RIL_UIM_E_SUCCESS) ||
            (verify_pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
            (verify_pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
            (verify_pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3))
        {
          err = RIL_UIM_E_REQUEST_NOT_SUPPORTED;
          goto send_read_record_error;
        }
      }
      QCRIL_LOG_INFO( "Provisioning session type found: %d",
                      verify_pin_params.session_info.session_type);
    }

    /* PIN id: PIN2 */
    verify_pin_params.pin_id = QMI_UIM_PIN_ID_PIN2;

    /* PIN value */
    verify_pin_params.pin_data.data_len = io_ptr->pin2.length();
    verify_pin_params.pin_data.data_ptr = (uint8*)io_ptr->pin2.c_str();

    msg_ptr = std::make_shared<UimVerifyPIN2Msg>(req_msg_ptr,
                                                 verify_pin_params.session_info);

    if (msg_ptr == NULL)
    {
      err = RIL_UIM_E_NO_MEMORY;
      goto send_read_record_error;
    }

    res = qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_VERIFY_PIN2,
                                       (void *)&verify_pin_params,
                                       msg_ptr);
    if (res < 0)
    {
      QCRIL_LOG_ERROR("error in qmi_uim_verify_pin!");
      err = qcril_uim_convert_err_value(res);
      goto send_read_record_error;
    }

    return;
  }

  /* No pin2 was provided: proceed with read transparent */
  if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_READ_RECORD,
                                   (void *)&read_params,
                                   req_msg_ptr) >= 0)
  {
    return;
  }
  err = RIL_UIM_E_MODEM_ERR;

send_read_record_error:
  rsp_ptr = std::make_shared<RIL_UIM_SIM_IO_Response>();

  if (rsp_ptr != NULL)
  {
    rsp_ptr->slot_id = slot;
    rsp_ptr->err = err;
  }

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_read_record */


/*=========================================================================

  FUNCTION:  qcril_uim_update_binary

===========================================================================*/
/*!
    @brief
    Function to write transparent

    @return
    None
*/
/*=========================================================================*/
static void qcril_uim_update_binary
(
  const std::shared_ptr<UimSIMIORequestMsg> req_ptr
)
{
  int                                    res                            = -1;
  RIL_UIM_Errno                          err                            = RIL_UIM_E_SUCCESS;
  qmi_uim_write_transparent_params_type  write_params                   = {};
  uint8                                  aid[QCRIL_UIM_MAX_AID_SIZE]    = {};
  uint8                                  path[QCRIL_UIM_MAX_PATH_SIZE]  = {};
  uint16                                 first_level_df_path            = 0;
  uint8                                  slot = QCRIL_UIM_INVALID_SLOT_INDEX_VALUE;
  const RIL_UIM_SIM_IO                 * io_ptr                         = NULL;
  std::shared_ptr<UimUpdateBinaryReqMsg>     req_msg_ptr                = NULL;
  std::shared_ptr<RIL_UIM_SIM_IO_Response>   rsp_ptr                    = NULL;

  if(req_ptr == NULL ||
     req_ptr->get_sim_io_data() == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  io_ptr = req_ptr->get_sim_io_data();
  /* Fetch slot info */
  slot = req_ptr->get_slot();
  if(slot >= QMI_UIM_MAX_CARD_COUNT)
  {
    QCRIL_LOG_ERROR("Invalid slot %d", slot);
    QCRIL_ASSERT(0);
    return;
  }

  /* Sanity check */
  if ((io_ptr->p1 < 0) || (io_ptr->p1 > 0xFF) ||
      (io_ptr->p2 < 0) || (io_ptr->p2 > 0xFF) ||
      (io_ptr->p3 < 0))
  {
    QCRIL_LOG_ERROR( "Unsupported case, P1: 0x%X, P2: 0x%X, P3: 0x%X \n",
                     io_ptr->p1, io_ptr->p2, io_ptr->p3);
    err = RIL_UIM_E_INVALID_ARGUMENTS;
    goto send_update_binary_error;
  }

  /* File id */
  err = qcril_uim_extract_file_id(io_ptr,
                                  &write_params.file_id,
                                  path,
                                  sizeof(path));
  if (err != RIL_UIM_E_SUCCESS)
  {
    goto send_update_binary_error;
  }

  /* Extract the first level DF */
  if ((write_params.file_id.path.data_len >= 4) && (write_params.file_id.path.data_ptr))
  {
    /* Interested only in 3rd & 4th bytes */
    first_level_df_path = (*(write_params.file_id.path.data_ptr + 2) << 8) |
                          (*(write_params.file_id.path.data_ptr + 3));
  }

  /* Session information */
  err = qcril_uim_extract_session_type(slot,
                                       io_ptr->aidPtr.c_str(),
                                       first_level_df_path,
                                       &write_params.session_info,
                                       aid,
                                       sizeof(aid));
  if (err != RIL_UIM_E_SUCCESS)
  {
    goto send_update_binary_error;
  }

  QCRIL_LOG_INFO( "Session type found: %d", write_params.session_info.session_type);

  /* Data */
  write_params.data.data_ptr = qcril_uim_alloc_hexstring_to_bin(io_ptr->data.c_str(),
                                                                &write_params.data.data_len);
  if (write_params.data.data_ptr == NULL)
  {
    err = RIL_UIM_E_NO_MEMORY;
    goto send_update_binary_error;
  }
  if (write_params.data.data_len != io_ptr->p3)
  {
    err = RIL_UIM_E_INVALID_ARGUMENTS;
    goto send_update_binary_error;
  }

  /* Write parameters: length and offset */
  write_params.offset = (io_ptr->p1) << 8 | (io_ptr->p2);

  req_msg_ptr = std::make_shared<UimUpdateBinaryReqMsg>(req_ptr, write_params);

  if (req_msg_ptr == NULL)
  {
    err = RIL_UIM_E_NO_MEMORY;
    goto send_update_binary_error;
  }

  if (!io_ptr->pin2.empty())
  {
    qmi_uim_verify_pin_params_type         verify_pin_params = {};
    std::shared_ptr<UimVerifyPIN2Msg>      msg_ptr           = NULL;

    /* Session information: same as for write, except card sessions */
    verify_pin_params.session_info = write_params.session_info;

    /* We try to fake a GSM/CDMA first level DF path only for PIN2 verification */
    if ((write_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
        (write_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
        (write_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3))
    {
      err = qcril_uim_extract_session_type(slot,
                                           io_ptr->aidPtr.c_str(),
                                           QCRIL_UIM_FILEID_DF_CDMA,
                                           &verify_pin_params.session_info,
                                           NULL,
                                           0);
      if ((err != RIL_UIM_E_SUCCESS) ||
          (verify_pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
          (verify_pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
          (verify_pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3))
      {
        err = qcril_uim_extract_session_type(slot,
                                             io_ptr->aidPtr.c_str(),
                                             QCRIL_UIM_FILEID_DF_GSM,
                                             &verify_pin_params.session_info,
                                             NULL,
                                             0);
        if ((err != RIL_UIM_E_SUCCESS) ||
            (verify_pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
            (verify_pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
            (verify_pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3))
        {
          err = RIL_UIM_E_REQUEST_NOT_SUPPORTED;
          goto send_update_binary_error;
        }
      }
      QCRIL_LOG_INFO( "Provisioning session type found: %d",
                      verify_pin_params.session_info.session_type);
      /* Use this session type until the modem resolves card session issue */
      write_params.session_info = verify_pin_params.session_info;
    }

    /* PIN id: PIN2 */
    verify_pin_params.pin_id = QMI_UIM_PIN_ID_PIN2;

    /* PIN value */
    verify_pin_params.pin_data.data_len = io_ptr->pin2.length();
    verify_pin_params.pin_data.data_ptr = (uint8*)io_ptr->pin2.c_str();

    msg_ptr = std::make_shared<UimVerifyPIN2Msg>(req_msg_ptr,
                                                 verify_pin_params.session_info);

    if (msg_ptr == NULL)
    {
      err = RIL_UIM_E_NO_MEMORY;
      goto send_update_binary_error;
    }

    res = qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_VERIFY_PIN2,
                                       (void *)&verify_pin_params,
                                       msg_ptr);

    if (res < 0)
    {
      err = qcril_uim_convert_err_value(res);
      goto send_update_binary_error;
    }

    return;
  }

  /* No pin2 was provided: proceed with write transparent */
  res = qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_WRITE_TRANSPARENT,
                                     (void *)&write_params,
                                     req_msg_ptr);

  if (res >= 0)
  {
    qcril_free(write_params.data.data_ptr);
    write_params.data.data_ptr = NULL;
    return;
  }
  err = RIL_UIM_E_MODEM_ERR;

send_update_binary_error:
  rsp_ptr = std::make_shared<RIL_UIM_SIM_IO_Response>();

  if (rsp_ptr != NULL)
  {
    rsp_ptr->slot_id = slot;
    rsp_ptr->err = err;
  }

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);

  if (write_params.data.data_ptr != NULL)
  {
    qcril_free(write_params.data.data_ptr);
    write_params.data.data_ptr = NULL;
  }
} /* qcril_uim_update_binary */


/*=========================================================================

  FUNCTION:  qcril_uim_update_record

===========================================================================*/
/*!
    @brief
    Function to write record

    @return
    None
*/
/*=========================================================================*/
static void qcril_uim_update_record
(
  const std::shared_ptr<UimSIMIORequestMsg> req_ptr
)
{
  int                               res                                 = -1;
  RIL_UIM_Errno                     err                                 = RIL_UIM_E_SUCCESS;
  qmi_uim_write_record_params_type  write_params                        = {};
  uint8                             aid[QCRIL_UIM_MAX_AID_SIZE]         = {};
  uint8                             path[QCRIL_UIM_MAX_PATH_SIZE]       = {};
  uint16                            first_level_df_path                 = 0;
  uint8                             slot = QCRIL_UIM_INVALID_SLOT_INDEX_VALUE;
  const RIL_UIM_SIM_IO            * io_ptr                              = NULL;
  std::shared_ptr<UimUpdateRecordReqMsg>     req_msg_ptr                = NULL;
  std::shared_ptr<RIL_UIM_SIM_IO_Response>   rsp_ptr                    = NULL;

  if(req_ptr == NULL ||
     req_ptr->get_sim_io_data() == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  io_ptr = req_ptr->get_sim_io_data();
  /* Fetch slot info */
  slot = req_ptr->get_slot();
  if(slot >= QMI_UIM_MAX_CARD_COUNT)
  {
    QCRIL_LOG_ERROR("Invalid slot %d", slot);
    QCRIL_ASSERT(0);
    return;
  }

  /* Sanity check */
  if ((io_ptr->p1 < 0) || (io_ptr->p1 > 0xFF) ||
      (io_ptr->p2 < 0) || (io_ptr->p2 > 0xFF) ||
      (io_ptr->p3 < 0) || (io_ptr->p3 > 0xFF))
  {
    QCRIL_LOG_ERROR( "Unsupported case, P1: 0x%X, P2: 0x%X, P3: 0x%X \n",
                     io_ptr->p1, io_ptr->p2, io_ptr->p3);
    err = RIL_UIM_E_INVALID_ARGUMENTS;
    goto send_update_record_error;
  }

  /* File id */
  err = qcril_uim_extract_file_id(io_ptr,
                                  &write_params.file_id,
                                  path,
                                  sizeof(path));
  if (err != RIL_UIM_E_SUCCESS)
  {
    goto send_update_record_error;
  }

  /* Extract the first level DF */
  if ((write_params.file_id.path.data_len >= 4) && (write_params.file_id.path.data_ptr))
  {
    /* Interested only in 3rd & 4th bytes */
    first_level_df_path = (*(write_params.file_id.path.data_ptr + 2) << 8) |
                          (*(write_params.file_id.path.data_ptr + 3));
  }

  /* Session information */
  err = qcril_uim_extract_session_type(slot,
                                       io_ptr->aidPtr.c_str(),
                                       first_level_df_path,
                                       &write_params.session_info,
                                       aid,
                                       sizeof(aid));
  if (err != RIL_UIM_E_SUCCESS)
  {
    goto send_update_record_error;
  }

  QCRIL_LOG_INFO( "Session type found: %d", write_params.session_info.session_type);

  /* Data */
  write_params.data.data_ptr = qcril_uim_alloc_hexstring_to_bin(io_ptr->data.c_str(),
                                                                &write_params.data.data_len);
  if (write_params.data.data_ptr == NULL)
  {
    err = RIL_UIM_E_NO_MEMORY;
    goto send_update_record_error;;
  }
  if (write_params.data.data_len != io_ptr->p3)
  {
    goto send_update_record_error;
  }

  /* Write parameters: length and offset */
  write_params.record = io_ptr->p1;

  /* p2 can take values 2, 3, 4, per 3GPP TS 51.011, however QCRIL
   * does not support next record (2) updates */
  if ((io_ptr->p2 != 4) && (io_ptr->p2 != 3))
  {
    QCRIL_LOG_ERROR( "%s: unsupported case P2 = %d\n", __FUNCTION__, io_ptr->p2);
    err = RIL_UIM_E_REQUEST_NOT_SUPPORTED;
    goto send_update_record_error;
  }

  req_msg_ptr = std::make_shared<UimUpdateRecordReqMsg>(req_ptr, write_params);

  if (req_msg_ptr == NULL)
  {
    err = RIL_UIM_E_NO_MEMORY;
    goto send_update_record_error;
  }

  if (!io_ptr->pin2.empty())
  {
    qmi_uim_verify_pin_params_type         verify_pin_params = {};
    std::shared_ptr<UimVerifyPIN2Msg>      msg_ptr           = NULL;

    /* Session information: same as for write, except card sessions */
    verify_pin_params.session_info = write_params.session_info;

    /* We try to fake a GSM/CDMA first level DF path */
    if ((write_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
        (write_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
        (write_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3))
    {
      err = qcril_uim_extract_session_type(slot,
                                           io_ptr->aidPtr.c_str(),
                                           QCRIL_UIM_FILEID_DF_CDMA,
                                           &verify_pin_params.session_info,
                                           NULL,
                                           0);
      if ((err != RIL_UIM_E_SUCCESS) ||
          (verify_pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
          (verify_pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
          (verify_pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3))
      {
        err = qcril_uim_extract_session_type(slot,
                                             io_ptr->aidPtr.c_str(),
                                             QCRIL_UIM_FILEID_DF_GSM,
                                             &verify_pin_params.session_info,
                                             NULL,
                                             0);
        if ((err != RIL_UIM_E_SUCCESS) ||
            (verify_pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
            (verify_pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
            (verify_pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3))
        {
          err = RIL_UIM_E_REQUEST_NOT_SUPPORTED;
          goto send_update_record_error;
        }
      }
      QCRIL_LOG_INFO( "Provisioning session type found: %d",
                      verify_pin_params.session_info.session_type);
      /* Use this session type until the modem resolves card session issue */
      write_params.session_info = verify_pin_params.session_info;
    }

    /* PIN id: PIN2 */
    verify_pin_params.pin_id = QMI_UIM_PIN_ID_PIN2;

    /* PIN value */
    verify_pin_params.pin_data.data_len = (uint16)(io_ptr->pin2.length());
    verify_pin_params.pin_data.data_ptr = (uint8*)io_ptr->pin2.c_str();

    msg_ptr = std::make_shared<UimVerifyPIN2Msg>(req_msg_ptr,
                                                 verify_pin_params.session_info);

    if (msg_ptr == NULL)
    {
      err = RIL_UIM_E_NO_MEMORY;
      goto send_update_record_error;
    }

    res = qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_VERIFY_PIN2,
                                       (void *)&verify_pin_params,
                                       msg_ptr);

    if (res < 0)
    {
      err = qcril_uim_convert_err_value(res);
      goto send_update_record_error;
    }

    return;
  }

  /* No pin2 was provided: proceed with write record */
  res = qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_WRITE_RECORD,
                                     (void *)&write_params,
                                     req_msg_ptr);

  if (res >= 0)
  {
    qcril_free(write_params.data.data_ptr);
    write_params.data.data_ptr = NULL;
    return;
  }
  err = RIL_UIM_E_MODEM_ERR;

send_update_record_error:
  rsp_ptr = std::make_shared<RIL_UIM_SIM_IO_Response>();

  if (rsp_ptr != NULL)
  {
    rsp_ptr->slot_id = slot;
    rsp_ptr->err = err;
  }

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);

  if (write_params.data.data_ptr != NULL)
  {
    qcril_free(write_params.data.data_ptr);
    write_params.data.data_ptr = NULL;
  }
} /* qcril_uim_update_record */


/*=========================================================================

  FUNCTION:  qcril_uim_get_response

===========================================================================*/
/*!
    @brief
    Function to get the file attributes

    @return
    None
*/
/*=========================================================================*/
static void qcril_uim_get_response
(
  const std::shared_ptr<UimSIMIORequestMsg> req_ptr
)
{
  RIL_UIM_Errno                            err                            = RIL_UIM_E_SUCCESS;
  qmi_uim_get_file_attributes_params_type  get_attr_params                = {};
  uint8                                    aid[QCRIL_UIM_MAX_AID_SIZE]    = {};
  uint8                                    path[QCRIL_UIM_MAX_PATH_SIZE]  = {};
  uint16                                   first_level_df_path            = 0;
  uint8                                    slot = QCRIL_UIM_INVALID_SLOT_INDEX_VALUE;
  const RIL_UIM_SIM_IO                   * io_ptr                         = NULL;
  std::shared_ptr<RIL_UIM_SIM_IO_Response> rsp_ptr                        = NULL;
  std::shared_ptr<UimGetAttributesReqMsg>  req_msg_ptr                    = NULL;

  if(req_ptr == NULL || req_ptr->get_sim_io_data() == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  QCRIL_LOG_INFO( "%s Enter \n", __FUNCTION__);

  io_ptr = req_ptr->get_sim_io_data();
  /* Fetch slot info */
  slot = req_ptr->get_slot();
  if(slot >= QMI_UIM_MAX_CARD_COUNT)
  {
    QCRIL_LOG_ERROR("Invalid slot %d", slot);
    QCRIL_ASSERT(0);
    return;
  }

  /* File id */
  err = qcril_uim_extract_file_id(io_ptr,
                                  &get_attr_params.file_id,
                                  path,
                                  sizeof(path));
  if (err != RIL_UIM_E_SUCCESS)
  {
    goto send_get_response_error;
  }

  /* Extract the first level DF */
  if ((get_attr_params.file_id.path.data_len >= 4) && (get_attr_params.file_id.path.data_ptr))
  {
    /* Interested only in 3rd & 4th bytes */
    first_level_df_path = (*(get_attr_params.file_id.path.data_ptr + 2) << 8) |
                          (*(get_attr_params.file_id.path.data_ptr + 3));
  }

  /* Session information */
  err = qcril_uim_extract_session_type(slot,
                                       io_ptr->aidPtr.c_str(),
                                       first_level_df_path,
                                       &get_attr_params.session_info,
                                       aid,
                                       sizeof(aid));
  if (err != RIL_UIM_E_SUCCESS)
  {
    goto send_get_response_error;
  }

  QCRIL_LOG_INFO( "Session type found: %d", get_attr_params.session_info.session_type);

  /* In case of get_file_attributes, the FW cannot pass
     an optional PIN2 */

  req_msg_ptr = std::make_shared<UimGetAttributesReqMsg>(req_ptr, get_attr_params);

  if (req_msg_ptr == NULL)
  {
    err = RIL_UIM_E_NO_MEMORY;
    goto send_get_response_error;
  }

  if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_GET_RESPONSE,
                                   (void *)&get_attr_params,
                                   req_msg_ptr) >= 0)
  {
    return;
  }
  err = RIL_UIM_E_MODEM_ERR;

send_get_response_error:
  rsp_ptr = std::make_shared<RIL_UIM_SIM_IO_Response>();

  if (rsp_ptr != NULL)
  {
    rsp_ptr->slot_id = slot;
    rsp_ptr->err = err;
  }

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);

} /* qcril_uim_get_response */


/*===========================================================================

  FUNCTION:  qcril_uim_request_get_fdn_status

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_INTERNAL_MMGSDI_GET_FDN_STATUS request from QCRIL.
    This is due to handling of RIL_REQUEST_QUERY_FACILITY_LOCK with facility
    string "FD" from the framework.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_get_fdn_status
(
  const std::shared_ptr<UimGetFacilityLockRequestMsg> req_ptr
)
{
  RIL_UIM_Errno                             err       = RIL_UIM_E_SUCCESS;
  uint16_t                                  first_level_df_path   = 0;
  qmi_uim_get_service_status_params_type    service_status_params = {};
  std::shared_ptr<RIL_UIM_GetFacilityLockResponseMsg> rsp_ptr     = NULL;

  if(req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot process response");
    QCRIL_ASSERT(0);
    return;
  }

  QCRIL_LOG_INFO( "qcril_uim_request_get_fdn_status\n");

  first_level_df_path = (req_ptr->get_aid().length() == 0) ?
                         QCRIL_UIM_FILEID_DF_CDMA : QCRIL_UIM_FILEID_ADF_USIM_CSIM;

  /* Extract session type, we need prov session for getting FDN service status */
  err = qcril_uim_extract_session_type(qmi_ril_get_process_instance_id(),
                                       req_ptr->get_aid().c_str(),
                                       first_level_df_path,
                                       &service_status_params.session_info,
                                       NULL,
                                       0);
  if ((err != RIL_UIM_E_SUCCESS) ||
      (service_status_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
      (service_status_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
      (service_status_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3))
  {
    if (first_level_df_path == QCRIL_UIM_FILEID_DF_CDMA)
    {
      err = qcril_uim_extract_session_type(qmi_ril_get_process_instance_id(),
                                           req_ptr->get_aid().c_str(),
                                           QCRIL_UIM_FILEID_DF_GSM,
                                           &service_status_params.session_info,
                                           NULL,
                                           0);
    }
    if ((err != RIL_UIM_E_SUCCESS) ||
        (service_status_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
        (service_status_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
        (service_status_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3))
    {
      err = RIL_UIM_E_REQUEST_NOT_SUPPORTED;
      goto send_get_fdn_error;
    }
  }

  QCRIL_LOG_INFO( "Session type found: %d", service_status_params.session_info.session_type);

  /* Capabilities mask paramter */
  service_status_params.mask = QMI_UIM_CAPS_MASK_SERVICE_FDN;

  if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_GET_FDN,
                                   (void *)&service_status_params,
                                   req_ptr) >= 0)
  {
    return;
  }
  err = RIL_UIM_E_MODEM_ERR;

send_get_fdn_error:
  rsp_ptr = std::make_shared<RIL_UIM_GetFacilityLockResponseMsg>();

  if (rsp_ptr != NULL)
  {
    rsp_ptr->ril_err = err;
  }
  
  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_request_get_fdn_status */


/*===========================================================================

  FUNCTION:  qcril_uim_request_set_fdn_status

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_INTERNAL_MMGSDI_SET_FDN_STATUS request from QCRIL.
    This is due to handling of RIL_REQUEST_SET_FACILITY_LOCK with facility
    string "FD" from the framework.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_set_fdn_status
(
  const std::shared_ptr<UimSetFacilityLockRequestMsg> req_ptr
)
{
  int                                       res       = 0;
  RIL_UIM_Errno                             err       = RIL_UIM_E_SUCCESS;
  uint16                                    first_level_df_path   = 0;
  qmi_uim_set_service_status_params_type    service_status_params = {};
  std::shared_ptr<RIL_UIM_SIM_PIN_Response> rsp_ptr               = NULL;

  if(req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot process response");
    QCRIL_ASSERT(0);
    return;
  }

  QCRIL_LOG_INFO( "qcril_uim_request_set_fdn_status\n");

  /* Update the file path based on passed aid_ptr */
  first_level_df_path = (req_ptr->get_aid().length() == 0) ?
                         QCRIL_UIM_FILEID_DF_CDMA : QCRIL_UIM_FILEID_ADF_USIM_CSIM;

  /* Extract session type, we need prov session for setting FDN status */
  err = qcril_uim_extract_session_type(qmi_ril_get_process_instance_id(),
                                       req_ptr->get_aid().c_str(),
                                       first_level_df_path,
                                       &service_status_params.session_info,
                                       NULL,
                                       0);
  if ((err != RIL_UIM_E_SUCCESS) ||
      (service_status_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
      (service_status_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
      (service_status_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3))
  {
    if (first_level_df_path == QCRIL_UIM_FILEID_DF_CDMA)
    {
      err = qcril_uim_extract_session_type(qmi_ril_get_process_instance_id(),
                                           req_ptr->get_aid().c_str(),
                                           QCRIL_UIM_FILEID_DF_GSM,
                                           &service_status_params.session_info,
                                           NULL,
                                           0);
    }
    if ((err != RIL_UIM_E_SUCCESS) ||
        (service_status_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
        (service_status_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
        (service_status_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3))
    {
      err = RIL_UIM_E_REQUEST_NOT_SUPPORTED;
      goto send_set_fdn_error;
    }
  }

  QCRIL_LOG_INFO( "Session type found: %d", service_status_params.session_info.session_type);

  /* service status parameter */
  if (req_ptr->get_status())
  {
    service_status_params.fdn_status = QMI_UIM_SERVICE_ENABLE;
  }
  else
  {
    service_status_params.fdn_status = QMI_UIM_SERVICE_DISABLE;
  }

  /* If PIN2 was provided, verify PIN2 */
  if (req_ptr->get_password().length() != 0)
  {
    qmi_uim_verify_pin_params_type         verify_pin_params = {};
    std::shared_ptr<UimVerifyPIN2Msg>      msg_ptr           = NULL;

    /* Session information */
    verify_pin_params.session_info = service_status_params.session_info;

    /* PIN id: PIN2 */
    verify_pin_params.pin_id = QMI_UIM_PIN_ID_PIN2;

    /* PIN value */
    verify_pin_params.pin_data.data_len = (uint16)(req_ptr->get_password().length());
    verify_pin_params.pin_data.data_ptr = (uint8_t *)req_ptr->get_password().c_str();

    msg_ptr = std::make_shared<UimVerifyPIN2Msg>(req_ptr,
                                                 service_status_params.session_info);

    if (msg_ptr == NULL)
    {
      err = RIL_UIM_E_NO_MEMORY;
      goto send_set_fdn_error;
    }

    res = qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_VERIFY_PIN2,
                                       (void *)&verify_pin_params,
                                       msg_ptr);
    if (res < 0)
    {
      QCRIL_LOG_ERROR("error in qmi_uim_verify_pin!");
      err = qcril_uim_convert_err_value(res);
      goto send_set_fdn_error;
    }

    return;
  }

  /* No pin2 provided, so proceed with set service status */
  if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_SET_FDN,
                                   (void *)&service_status_params,
                                   req_ptr) >= 0)
  {
    return;
  }
  err = RIL_UIM_E_MODEM_ERR;

send_set_fdn_error:
  rsp_ptr = std::make_shared<RIL_UIM_SIM_PIN_Response>();

  if (rsp_ptr != NULL)
  {
    rsp_ptr->err = err;
  }
  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_request_set_fdn_status */


/*=========================================================================

  FUNCTION:  qcril_uim_get_status

===========================================================================*/
/*!
    @brief
    Function to send card status

    @return
    None
*/
/*=========================================================================*/
static void qcril_uim_get_status
(
  const std::shared_ptr<UimSIMIORequestMsg> req_ptr
)
{
  RIL_UIM_Errno                     err                         = RIL_UIM_E_SUCCESS;
  qmi_uim_status_cmd_params_type    status_params               = {};
  uint8                             aid[QCRIL_UIM_MAX_AID_SIZE] = {};
  uint8                             slot =  QCRIL_UIM_INVALID_SLOT_INDEX_VALUE;
  const RIL_UIM_SIM_IO            * io_ptr                      = NULL;
  std::shared_ptr<RIL_UIM_SIM_IO_Response> rsp_ptr              = NULL;
  std::shared_ptr<UimGetStatusReqMsg>      req_msg_ptr          = NULL;

  if(req_ptr == NULL || req_ptr->get_sim_io_data() == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  QCRIL_LOG_INFO( "%s Enter \n", __FUNCTION__);

  io_ptr = req_ptr->get_sim_io_data();

  if(req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  /* Fetch slot info */
  slot = req_ptr->get_slot();
  if(slot >= QMI_UIM_MAX_CARD_COUNT)
  {
    QCRIL_LOG_ERROR("Invalid slot %d", slot);
    QCRIL_ASSERT(0);
    return;
  }

  /* Extract command mode and response type */
  switch (io_ptr->p1)
  {
    case 0:
      status_params.mode = QMI_UIM_STATUS_CMD_MODE_NO_INDICATION;
      break;
    case 1:
      status_params.mode = QMI_UIM_STATUS_CMD_MODE_APP_INITIALIZED;
      break;
    case 2:
      status_params.mode = QMI_UIM_STATUS_CMD_MODE_WILL_TERMINATE_APP;
      break;
    default:
      QCRIL_LOG_ERROR("Unsupported case, P1: 0x%X\n",io_ptr->p1);
      err = RIL_UIM_E_REQUEST_NOT_SUPPORTED;
      goto send_status_error;
  }

  switch (io_ptr->p2)
  {
    case 0:
      status_params.resp_type = QMI_UIM_STATUS_CMD_FCP_RSP;
      break;
    case 1:
      status_params.resp_type = QMI_UIM_STATUS_CMD_AID_RSP;
      break;
    case 12:
      status_params.resp_type = QMI_UIM_STATUS_CMD_NO_DATA_RSP;
      break;
    default:
      QCRIL_LOG_ERROR("Unsupported case, P2: 0x%X\n",io_ptr->p2);
      err = RIL_UIM_E_REQUEST_NOT_SUPPORTED;
      goto send_status_error;
  }

  /* Extract session information */
  if (io_ptr->aidPtr.empty())
  {
    /* In case of NULL AID, use a GW session if it is active,
       else use a 1x session */
    err = qcril_uim_extract_session_type(slot,
                                         NULL,
                                         QCRIL_UIM_FILEID_DF_GSM,
                                         &status_params.session_info,
                                         NULL,
                                         0);
    if ((err != RIL_UIM_E_SUCCESS) ||
        (status_params.session_info.session_type != QMI_UIM_SESSION_TYPE_PRI_GW_PROV &&
         status_params.session_info.session_type != QMI_UIM_SESSION_TYPE_SEC_GW_PROV &&
         status_params.session_info.session_type != QMI_UIM_SESSION_TYPE_TER_GW_PROV))
    {
      err = qcril_uim_extract_session_type(slot,
                                           NULL,
                                           QCRIL_UIM_FILEID_DF_CDMA,
                                           &status_params.session_info,
                                           NULL,
                                           0);
    }
  }
  else
  {
    err = qcril_uim_extract_session_type(slot,
                                         io_ptr->aidPtr.c_str(),
                                         QCRIL_UIM_FILEID_ADF_USIM_CSIM,
                                         &status_params.session_info,
                                         aid,
                                         sizeof(aid));
  }
  if (err != RIL_UIM_E_SUCCESS)
  {
    goto send_status_error;
  }

  QCRIL_LOG_INFO( "Session type found: %d", status_params.session_info.session_type);

  req_msg_ptr = std::make_shared<UimGetStatusReqMsg>(req_ptr, status_params);
  if (req_msg_ptr == NULL)
  {
    err = RIL_UIM_E_NO_MEMORY;
    goto send_status_error;
  }

  if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_SEND_STATUS,
                                   (void *)&status_params,
                                   req_msg_ptr) >= 0)
  {
    return;
  }

  err = RIL_UIM_E_MODEM_ERR;

send_status_error:
  rsp_ptr = std::make_shared<RIL_UIM_SIM_IO_Response>();

  if (rsp_ptr != NULL)
  {
    rsp_ptr->slot_id = slot;
    rsp_ptr->err = err;
  }

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_get_status */


/*=========================================================================

  FUNCTION:  qcril_uim_read_binary_resp

===========================================================================*/
/*!
    @brief
    Process the response for read binary.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_read_binary_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<UimReadTransparentReqMsg> msg_ptr
)
{
  RIL_UIM_Errno ril_err  = RIL_UIM_E_SUCCESS;
  auto          rsp_ptr  = std::make_shared<RIL_UIM_SIM_IO_Response>();
  std::shared_ptr<UimSIMIORequestMsg> req_ptr = NULL;

  if(params_ptr == NULL || msg_ptr == NULL || msg_ptr->get_request_msg() == NULL)
  {
    QCRIL_LOG_ERROR("NULL params_ptr");
    QCRIL_ASSERT(0);
    return;
  }

  req_ptr = std::static_pointer_cast<UimSIMIORequestMsg>(msg_ptr->get_request_msg());

  ril_err = qcril_uim_convert_err_value(params_ptr->qmi_rsp_data.qmi_err_code);

  QCRIL_LOG_DEBUG( "qcril_uim_read_binary_resp: qmi_err_code=%d \n",
                    params_ptr->qmi_rsp_data.qmi_err_code);

  if (rsp_ptr != NULL)
  {
    char * simResponse = NULL;

    rsp_ptr->sw1 = params_ptr->qmi_rsp_data.rsp_data.read_transparent_rsp.sw1;
    rsp_ptr->sw2 = params_ptr->qmi_rsp_data.rsp_data.read_transparent_rsp.sw2;
    rsp_ptr->slot_id = req_ptr->get_slot();
    rsp_ptr->err = ril_err;

    simResponse = qcril_uim_alloc_bin_to_hexstring(
                              params_ptr->qmi_rsp_data.rsp_data.read_transparent_rsp.content.data_ptr,
                              params_ptr->qmi_rsp_data.rsp_data.read_transparent_rsp.content.data_len);

    if (simResponse != NULL)
    {
      rsp_ptr->simResponse = simResponse;
      qcril_free(simResponse);
    }

    QCRIL_LOG_DEBUG( "RIL_SIM_IO_Response: sw1=%d sw2=%d data=%s\n",
                      rsp_ptr->sw1, rsp_ptr->sw2, rsp_ptr->simResponse.c_str());

    /* Workaround until the modem build is ready */
    if (rsp_ptr->sw1 == 0)
    {
      if (ril_err == RIL_UIM_E_SUCCESS)
      {
        rsp_ptr->sw1 = 0x90;
        rsp_ptr->sw2 = 0x00;
      }
      else
      {
        rsp_ptr->sw1 = 0x94;
        rsp_ptr->sw2 = 0x04;
      }
    }
  }

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_read_binary_resp */


/*=========================================================================

  FUNCTION:  qcril_uim_read_record_resp

===========================================================================*/
/*!
    @brief
    Process the response for read record.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_read_record_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<UimReadRecordReqMsg> msg_ptr
)
{
  RIL_UIM_Errno ril_err  = RIL_UIM_E_SUCCESS;
  auto          rsp_ptr  = std::make_shared<RIL_UIM_SIM_IO_Response>();

  if(params_ptr == NULL || msg_ptr == NULL || msg_ptr->get_request_msg() == NULL)
  {
    QCRIL_LOG_ERROR("NULL params_ptr");
    QCRIL_ASSERT(0);
    return;
  }

  auto req_ptr = std::static_pointer_cast<UimSIMIORequestMsg>(msg_ptr->get_request_msg());

  ril_err = qcril_uim_convert_err_value(params_ptr->qmi_rsp_data.qmi_err_code);

  QCRIL_LOG_DEBUG( "qcril_uim_read_record_resp: qmi_err_code=%d \n",
                    params_ptr->qmi_rsp_data.qmi_err_code);

  if (rsp_ptr != NULL)
  {
    char * simResponse = NULL;

    rsp_ptr->sw1 = params_ptr->qmi_rsp_data.rsp_data.read_record_rsp.sw1;
    rsp_ptr->sw2 = params_ptr->qmi_rsp_data.rsp_data.read_record_rsp.sw2;
    rsp_ptr->slot_id = req_ptr->get_slot();
    rsp_ptr->err = ril_err;

    simResponse = qcril_uim_alloc_bin_to_hexstring(
                              params_ptr->qmi_rsp_data.rsp_data.read_record_rsp.content.data_ptr,
                              params_ptr->qmi_rsp_data.rsp_data.read_record_rsp.content.data_len);

    if (simResponse != NULL)
    {
      rsp_ptr->simResponse = simResponse;
      qcril_free(simResponse);
    }

    QCRIL_LOG_DEBUG( "RIL_SIM_IO_Response: sw1=%d sw2=%d data=%s\n",
                      rsp_ptr->sw1, rsp_ptr->sw2, rsp_ptr->simResponse.c_str());

    /* Workaround until the modem build is ready */
    if (rsp_ptr->sw1 == 0)
    {
      if (ril_err == RIL_UIM_E_SUCCESS)
      {
        rsp_ptr->sw1 = 0x90;
        rsp_ptr->sw2 = 0x00;
      }
      else
      {
        rsp_ptr->sw1 = 0x94;
        rsp_ptr->sw2 = 0x04;
      }
    }
  }

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_read_record_resp */


/*=========================================================================

  FUNCTION:  qcril_uim_update_binary_resp

===========================================================================*/
/*!
    @brief
    Process the response for write transparent.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_update_binary_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<UimUpdateBinaryReqMsg> msg_ptr
)
{
  RIL_UIM_Errno ril_err  = RIL_UIM_E_SUCCESS;
  auto          rsp_ptr  = std::make_shared<RIL_UIM_SIM_IO_Response>();

  if(params_ptr == NULL || msg_ptr == NULL || msg_ptr->get_request_msg() == NULL)
  {
    QCRIL_LOG_ERROR("NULL params_ptr");
    QCRIL_ASSERT(0);
    return;
  }

  auto req_ptr = std::static_pointer_cast<UimSIMIORequestMsg>(msg_ptr->get_request_msg());

  ril_err = qcril_uim_convert_err_value(params_ptr->qmi_rsp_data.qmi_err_code);

  QCRIL_LOG_DEBUG( "qcril_uim_update_binary_resp: qmi_err_code=%d \n",
                    params_ptr->qmi_rsp_data.qmi_err_code);

  if (rsp_ptr != NULL)
  {
    rsp_ptr->sw1 = params_ptr->qmi_rsp_data.rsp_data.write_transparent_rsp.sw1;
    rsp_ptr->sw2 = params_ptr->qmi_rsp_data.rsp_data.write_transparent_rsp.sw2;
    rsp_ptr->slot_id = req_ptr->get_slot();
    rsp_ptr->err = ril_err;

    QCRIL_LOG_DEBUG( "RIL_SIM_IO_Response: sw1=%d sw2=%d \n",
                      rsp_ptr->sw1, rsp_ptr->sw2);


    /* Workaround until the modem build is ready */
    if (rsp_ptr->sw1 == 0)
    {
      if (ril_err == RIL_UIM_E_SUCCESS)
      {
        rsp_ptr->sw1 = 0x90;
        rsp_ptr->sw2 = 0x00;
      }
      else
      {
        rsp_ptr->sw1 = 0x94;
        rsp_ptr->sw2 = 0x04;
      }
    }
  }

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_update_binary_resp */


/*=========================================================================

  FUNCTION:  qcril_uim_update_record_resp

===========================================================================*/
/*!
    @brief
    Process the response for write record.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_update_record_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<UimUpdateRecordReqMsg> msg_ptr
)
{
  RIL_UIM_Errno ril_err  = RIL_UIM_E_SUCCESS;
  auto          rsp_ptr  = std::make_shared<RIL_UIM_SIM_IO_Response>();

  if(params_ptr == NULL || msg_ptr == NULL || msg_ptr->get_request_msg() == NULL)
  {
    QCRIL_LOG_ERROR("NULL params_ptr");
    QCRIL_ASSERT(0);
    return;
  }

  auto req_ptr = std::static_pointer_cast<UimSIMIORequestMsg>(msg_ptr->get_request_msg());

  ril_err = qcril_uim_convert_err_value(params_ptr->qmi_rsp_data.qmi_err_code);

  QCRIL_LOG_DEBUG( "qcril_uim_update_record_resp: qmi_err_code=%d \n",
                    params_ptr->qmi_rsp_data.qmi_err_code);

  if (rsp_ptr != NULL)
  {
    rsp_ptr->sw1 = params_ptr->qmi_rsp_data.rsp_data.write_record_rsp.sw1;
    rsp_ptr->sw2 = params_ptr->qmi_rsp_data.rsp_data.write_record_rsp.sw2;
    rsp_ptr->slot_id = req_ptr->get_slot();
    rsp_ptr->err = ril_err;

    QCRIL_LOG_DEBUG( "RIL_SIM_IO_Response: sw1=%d sw2=%d \n",
                      rsp_ptr->sw1, rsp_ptr->sw2);


    /* Workaround until the modem build is ready */
    if (rsp_ptr->sw1 == 0)
    {
      if (ril_err == RIL_UIM_E_SUCCESS)
      {
        rsp_ptr->sw1 = 0x90;
        rsp_ptr->sw2 = 0x00;
      }
      else
      {
        rsp_ptr->sw1 = 0x94;
        rsp_ptr->sw2 = 0x04;
      }
    }
  }

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_update_record_resp */


/*=========================================================================

  FUNCTION:  qcril_uim_get_response_resp

===========================================================================*/
/*!
    @brief
    Process the response for get file attributes.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_get_response_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<UimGetAttributesReqMsg> msg_ptr
)
{
  RIL_UIM_Errno ril_err  = RIL_UIM_E_SUCCESS;
  auto          rsp_ptr  = std::make_shared<RIL_UIM_SIM_IO_Response>();

  if(params_ptr == NULL || msg_ptr == NULL || msg_ptr->get_request_msg() == NULL)
  {
    QCRIL_LOG_ERROR("NULL params_ptr");
    QCRIL_ASSERT(0);
    return;
  }

  auto req_ptr = std::static_pointer_cast<UimSIMIORequestMsg>(msg_ptr->get_request_msg());
  ril_err = qcril_uim_convert_err_value(params_ptr->qmi_rsp_data.qmi_err_code);

 
  QCRIL_LOG_DEBUG( "qcril_uim_get_response_resp: qmi_err_code=%d \n",
                    params_ptr->qmi_rsp_data.qmi_err_code);

  if (rsp_ptr != NULL)
  {
    rsp_ptr->sw1 = params_ptr->qmi_rsp_data.rsp_data.get_file_attributes_rsp.sw1;
    rsp_ptr->sw2 = params_ptr->qmi_rsp_data.rsp_data.get_file_attributes_rsp.sw2;
    rsp_ptr->slot_id = req_ptr->get_slot();
    rsp_ptr->err = ril_err;

    /* Compose file attributes in ICC format */
    if (params_ptr->qmi_rsp_data.qmi_err_code == 0)
    {
      char         *simResponse                                       = NULL;
      uint8         icc_attributes[QCRIL_UIM_GET_RESPONSE_MIN_SIZE+1] = {};

      /* If it is a 2G card, just use the raw data if available */
      if ((qcril_uim_find_app_in_slot(rsp_ptr->slot_id, QMI_UIM_APP_SIM) ||
           qcril_uim_find_app_in_slot(rsp_ptr->slot_id, QMI_UIM_APP_RUIM)) &&
          (params_ptr->qmi_rsp_data.rsp_data.get_file_attributes_rsp.raw_value.data_ptr != NULL) &&
          (params_ptr->qmi_rsp_data.rsp_data.get_file_attributes_rsp.raw_value.data_len >= QCRIL_UIM_GET_RESPONSE_MIN_SIZE))
      {
        /* Copy necessary data after sanity check */
        uint16 data_len = (params_ptr->qmi_rsp_data.rsp_data.get_file_attributes_rsp.raw_value.data_len <= sizeof(icc_attributes)) ?
                            params_ptr->qmi_rsp_data.rsp_data.get_file_attributes_rsp.raw_value.data_len : sizeof(icc_attributes);
        memcpy(icc_attributes,
               params_ptr->qmi_rsp_data.rsp_data.get_file_attributes_rsp.raw_value.data_ptr,
               data_len);
      }
      else
      {
        /* File size */
        icc_attributes[2] = (uint8)(params_ptr->qmi_rsp_data.rsp_data.get_file_attributes_rsp.file_size >> 8);
        icc_attributes[3] = (uint8)(params_ptr->qmi_rsp_data.rsp_data.get_file_attributes_rsp.file_size & 0xFF);
        /* File id */
        icc_attributes[4] = (uint8)(params_ptr->qmi_rsp_data.rsp_data.get_file_attributes_rsp.file_id >> 8);
        icc_attributes[5] = (uint8)(params_ptr->qmi_rsp_data.rsp_data.get_file_attributes_rsp.file_id & 0xFF);
        /* File type: EF */
        icc_attributes[6] = 0x04;
        switch (params_ptr->qmi_rsp_data.rsp_data.get_file_attributes_rsp.file_type)
        {
          case QMI_UIM_FILE_TYPE_LINEAR_FIXED:
            /* Linear fixed */
            icc_attributes[13] = 0x01;
            /* Record size */
            icc_attributes[14] =
              (uint8)(params_ptr->qmi_rsp_data.rsp_data.get_file_attributes_rsp.record_size);
            break;
          case QMI_UIM_FILE_TYPE_CYCLIC:
            /* Cyclic */
            icc_attributes[13] = 0x03;
            /* Increase allowed */
            icc_attributes[7] = 1;
            /* Record size */
            icc_attributes[14] =
              (uint8)(params_ptr->qmi_rsp_data.rsp_data.get_file_attributes_rsp.record_size);
            break;
          case QMI_UIM_FILE_TYPE_TRANSPARENT:
            /* Transparent */
            icc_attributes[13] = 0x00;
            break;
          default:
            /* non EF! */
            break;
        }
        /* Security attributes - need to be implemented */
        /* File status byte */
        icc_attributes[11] = qcril_uim_get_file_status_byte(
                               params_ptr->qmi_rsp_data.rsp_data.get_file_attributes_rsp.raw_value.data_len,
                               params_ptr->qmi_rsp_data.rsp_data.get_file_attributes_rsp.raw_value.data_ptr);

        /* Hardcoded to 2 because we only simulate byte 13 and 14 */
        icc_attributes[12] = 2;
      }

      simResponse = qcril_uim_alloc_bin_to_hexstring(
                                    icc_attributes,
                                    sizeof(icc_attributes));

      if (simResponse != NULL)
      {
        rsp_ptr->simResponse = simResponse;
        qcril_free(simResponse);
      }
    }

    QCRIL_LOG_DEBUG( "RIL_SIM_IO_Response: sw1=%d sw2=%d data=%s\n",
                      rsp_ptr->sw1, rsp_ptr->sw2,
                      rsp_ptr->simResponse.c_str());
  
    /* Workaround until the modem build is ready */
    if (rsp_ptr->sw1 == 0)
    {
      if (params_ptr->qmi_rsp_data.qmi_err_code == RIL_UIM_E_SUCCESS)
      {
        rsp_ptr->sw1 = 0x90;
        rsp_ptr->sw2 = 0x00;
      }
      else
      {
        rsp_ptr->sw1 = 0x94;
        rsp_ptr->sw2 = 0x04;
      }
    }
  }

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_get_response_resp */


/*=========================================================================

  FUNCTION:  qcril_uim_parse_gw_imsi

===========================================================================*/
/*!
    @brief
    Parses the data present in EF IMSI for SIM/USIM and packages the
    response in a null terminated ASCII string format.

    The EF structure is defined in 31.102 section 4.2.2:

    <TABLE>
      <TR>
        <TH> Byte(s) </TH>
        <TH> Description </TH>
      </TR>
      <TR>
        <TH> 1 </TH>
        <TD> IMSI length (bytes) </TD>
      </TR>
      <TR>
        <TH> 2 </TH>
        <TD> Bits 0-3: Unused  <BR> Bits 4-7: Digit 1   </TD>
      </TR>
      <TR>
        <TH> 3 to 9  </TH>
        <TD> Bits 0-3: Digit n <BR> Bits 4-7: Digit n+1 </TD>
      </TR>
    </TABLE>

    @return
    Pointer to the memory that contains the IMSI in ASCII string format
*/
/*=========================================================================*/
static char * qcril_uim_parse_gw_imsi
(
  const unsigned char * raw_imsi_ptr,
  unsigned short        raw_imsi_len,
  int                 * parsed_imsi_len_ptr
)
{
  int             src             = 0;
  int             dst             = 0;
  char          * parsed_imsi_ptr = NULL;

  /* Sanity check on input parameters */
  if ((raw_imsi_ptr == NULL) || (parsed_imsi_len_ptr == NULL))
  {
    QCRIL_LOG_ERROR( "Invalid input: raw_imsi_ptr 0x%x, parsed_imsi_len_ptr 0x%x\n",
                     raw_imsi_ptr, parsed_imsi_len_ptr);
    return NULL;
  }

  /* Check for the length of IMSI bytes in the first byte */
  *parsed_imsi_len_ptr = *raw_imsi_ptr;
  if (*parsed_imsi_len_ptr >= raw_imsi_len)
  {
    QCRIL_LOG_ERROR( "Invalid data length %d\n", *parsed_imsi_len_ptr);
    *parsed_imsi_len_ptr = 0;
    return NULL;
  }

  /* Allocate required amount of memory for IMSI in ASCII string format,
     note that it is freed by the caller */
  parsed_imsi_ptr = (char *)qcril_malloc((2 * (*parsed_imsi_len_ptr)));
  if (parsed_imsi_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "%s: memory allocation failed\n", __FUNCTION__);
    *parsed_imsi_len_ptr = 0;
    return NULL;
  }

  /* Compose IMSI */
  memset(parsed_imsi_ptr, 0, (2 * (*parsed_imsi_len_ptr)));
  for (src = 1, dst = 0;
      (src <= (*parsed_imsi_len_ptr)) && (dst < ((*parsed_imsi_len_ptr) * 2));
       src++)
  {
    uint8 imsi_nibble = 0;

    QCRIL_LOG_DEBUG( "IMSI[%d] src=%4x, dst=", src, raw_imsi_ptr[src]);

    /* Only process lower part of byte for second and subsequent bytes */
    if (src > 1)
    {
      imsi_nibble = raw_imsi_ptr[src] & 0x0F;
      if (imsi_nibble < 0x0a)
      {
        parsed_imsi_ptr[dst] = qcril_uim_bin_to_hexchar(imsi_nibble);
        QCRIL_LOG_DEBUG( "%c", parsed_imsi_ptr[dst]);
        dst++;
      }
    }
    /* Process upper part of byte for all bytes */
    imsi_nibble = raw_imsi_ptr[src] >> 4;
    if (imsi_nibble < 0x0a)
    {
      parsed_imsi_ptr[dst] = qcril_uim_bin_to_hexchar(imsi_nibble);
      QCRIL_LOG_DEBUG( "%c\n", parsed_imsi_ptr[dst]);
      dst++;
    }
  }

  /* Update parsed IMSI length in bytes - number of ASCII bytes copied plus NULL termination */
  *parsed_imsi_len_ptr = dst + 1;

  return parsed_imsi_ptr;
} /* qcril_uim_parse_gw_imsi */


/*=========================================================================

  FUNCTION:  qcril_uim_parse_1x_imsi

===========================================================================*/
/*!
    @brief
    Parses the data present in EF IMSI_M for RUIM/CSIM and packages the
    response in a null terminated ASCII string format.

    The EF structure is defined in C.S0065-0 section 5.2.2:

    <TABLE>
      <TR>
        <TH> Byte(s) </TH>
        <TH> Description </TH>
      </TR>
      <TR>
        <TH> 1 </TH>
        <TD> IMSI_M_CLASS (1 byte) </TD>
      </TR>
      <TR>
        <TH> 2 to 3 </TH>
        <TD> IMSI_M_S2 from IMSI_M_S (2 bytes) </TD>
      </TR>
      <TR>
        <TH> 4 to 6 </TH>
        <TD> IMSI_M_S1 from IMSI_M_S (3 bytes)  </TD>
      </TR>
      <TR>
        <TH> 7 </TH>
        <TD> IMSI_M_11_12 (1 byte)  </TD>
      </TR>
      <TR>
        <TH> 8 </TH>
        <TD> IMSI_M_PROGRAMMED/IMSI_M_ADDR_NUM (1 byte)  </TD>
      </TR>
      <TR>
        <TH> 9 to 10  </TH>
        <TD> MCC_M (2 bytes)  </TD>
      </TR>
    </TABLE>

    @return
    Pointer to the memory that contains the IMSI in ASCII string format
*/
/*=========================================================================*/
static char * qcril_uim_parse_1x_imsi
(
  const unsigned char * raw_imsi_ptr,
  unsigned short        raw_imsi_len,
  int                 * parsed_imsi_len_ptr
)
{
  uint8     i             = 0;
  uint16    mcc           = 0;
  uint8     mnc           = 0;
  uint32    min1          = 0;
  uint16    min2          = 0;
  uint16    second_three  = 0;
  uint8     thousands     = 0xFF;
  uint16    last_three    = 0;
  uint8     min_to_num[]  = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
  uint8     bcd_to_num[]  = { 0xFF, '1', '2', '3', '4', '5', '6', '7', '8',
                              '9', '0', 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

  char          * parsed_imsi_ptr = NULL;

  /* Sanity check on input parameters */
  if ((raw_imsi_ptr == NULL) || (parsed_imsi_len_ptr == NULL))
  {
    QCRIL_LOG_ERROR( "Invalid input: raw_imsi_ptr 0x%x, parsed_imsi_len_ptr 0x%x\n",
                     raw_imsi_ptr, parsed_imsi_len_ptr);
    return NULL;
  }

  /* Check for the minumim length of IMSI_M expected */
  if (raw_imsi_len < QCRIL_UIM_IMSI_M_RAW_SIZE)
  {
    QCRIL_LOG_ERROR( "Invalid data length %d\n", raw_imsi_len);
    return NULL;
  }

  /* Sanity check for IMSI_M_PROGRAMMED indicator */
  if ((raw_imsi_ptr[7] & 0x80) == 0)
  {
    QCRIL_LOG_ERROR("IMSI_M has not been programmed\n");
    return NULL;
  }

  /* Update parsed length - null terminated ASCII string length */
  *parsed_imsi_len_ptr = QCRIL_UIM_IMSI_M_PARSED_SIZE;

  /* Allocate the number of bytes */
  parsed_imsi_ptr = (char *)qcril_malloc(*parsed_imsi_len_ptr);
  if (parsed_imsi_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "Memory allocation failed for parsed_imsi_ptr! \n");
    *parsed_imsi_len_ptr = 0;
    return NULL;
  }

  memset(parsed_imsi_ptr, 0, *parsed_imsi_len_ptr);

  /* Parse certain parameters */
  mcc           = (((raw_imsi_ptr[9] & 0x03) << 8) + raw_imsi_ptr[8]);
  mnc           = raw_imsi_ptr[6] & 0x7F;
  min1          = ((raw_imsi_ptr[5] <<16) +(raw_imsi_ptr[4] <<8) + raw_imsi_ptr[3]);
  min2          = ((raw_imsi_ptr[2] <<8)  + raw_imsi_ptr[1]);
  /* left 10 bits */
  second_three  = (min1 & 0x00FFC000) >> 14;
  /* middle 4 bits */
  thousands     = (min1 & 0x00003C00) >> 10;
  /* right 10 bits */
  last_three    = (min1 & 0x000003FF);
  thousands     = bcd_to_num[thousands];

  QCRIL_LOG_DEBUG("mcc %d, mnc %d, min2 %d, thousands %d, second_three %d, last_three %d\n",
                  mcc, mnc, min2, thousands, second_three, last_three);

  if ((mcc > 999) || (mnc > 99) || (min2 > 999) || (thousands == 0xFF) ||
      (second_three > 999) || (last_three > 999))
  {
    QCRIL_LOG_ERROR("Invalid data while parsing IMSI_M \n");
    qcril_free(parsed_imsi_ptr);
    *parsed_imsi_len_ptr = 0;
    return NULL;
  }

  /* Construct ASCII IMSI_M, format:
     <3_digit_MCC><2_digit_11_12_digits><LS_10_digits_IMSI> */
  /* Update MCC - 3 digits */
  parsed_imsi_ptr[i++] = min_to_num[mcc/100];
  mcc %= 100;
  parsed_imsi_ptr[i++] = min_to_num[mcc/10];
  parsed_imsi_ptr[i++] = min_to_num[mcc%10];
  /* Update MNC - 2 digits */
  parsed_imsi_ptr[i++] = min_to_num[mnc/10];
  parsed_imsi_ptr[i++] = min_to_num[mnc%10];
  /* Update the first 3 digits of IMSI */
  parsed_imsi_ptr[i++] = min_to_num[min2/100];
  min2 %= 100;
  parsed_imsi_ptr[i++] = min_to_num[min2/10];
  parsed_imsi_ptr[i++] = min_to_num[min2%10];
  /* Update the last 7 digits of IMSI */
  parsed_imsi_ptr[i++] = min_to_num[second_three/100];
  second_three %= 100;
  parsed_imsi_ptr[i++] = min_to_num[second_three/10];
  parsed_imsi_ptr[i++] = min_to_num[second_three%10];
  parsed_imsi_ptr[i++] = thousands;
  parsed_imsi_ptr[i++] = min_to_num[last_three/100];
  last_three %= 100;
  parsed_imsi_ptr[i++] = min_to_num[last_three/10];
  parsed_imsi_ptr[i++] = min_to_num[last_three%10];

  QCRIL_LOG_DEBUG( "Parsed IMSI_M value:[%s]", parsed_imsi_ptr);

  return parsed_imsi_ptr;
} /* qcril_uim_parse_1x_imsi */


/*=========================================================================

  FUNCTION:  qcril_uim_get_imsi_resp

===========================================================================*/
/*!
    @brief
    Processes the response for RIL_REQUEST_GET_IMSI. IMSI/IMSI_M is stored
    as a transparent file on the SIM/USIM/CSIM. This information is contained
    within the data field returned in callback from QMI. The EF structure
    and coding is different between SIM/USIM and RUIM/CSIM and are defined
    in the respective Specifications.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_get_imsi_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<UimReadTransparentReqMsg> msg_ptr
)
{
  RIL_UIM_Errno    ril_err      = RIL_UIM_E_INTERNAL_ERR;
  char           * imsi_ptr     = NULL;
  int              imsi_length  = 0;
  int              is_gwl       = FALSE;
  auto             resp_ptr     = std::make_shared<RIL_UIM_IMSI_Response>();
  qmi_uim_read_transparent_params_type    *read_param_ptr = NULL;

  if((params_ptr == NULL) || (msg_ptr == NULL))
  {
    QCRIL_LOG_ERROR("NULL params_ptr");
    QCRIL_ASSERT(0);
    return;
  }

  auto req_ptr = std::static_pointer_cast<UimGetImsiRequestMsg>(msg_ptr->get_request_msg());

  if (req_ptr == NULL)
  {
    return;
  }

  ril_err = qcril_uim_convert_err_value(params_ptr->qmi_rsp_data.qmi_err_code);
  read_param_ptr = msg_ptr->get_params();

  QCRIL_LOG_DEBUG( "qcril_uim_get_imsi_resp: qmi_err_code=%d \n",
                    params_ptr->qmi_rsp_data.qmi_err_code);

  /* Based on the type of request, fetch appropriate IMSI */
  if (ril_err == RIL_UIM_E_SUCCESS && read_param_ptr != NULL)
  {
    if((read_param_ptr->session_info.session_type == QMI_UIM_SESSION_TYPE_PRI_GW_PROV) ||
       (read_param_ptr->session_info.session_type == QMI_UIM_SESSION_TYPE_SEC_GW_PROV) ||
       (read_param_ptr->session_info.session_type == QMI_UIM_SESSION_TYPE_TER_GW_PROV))
    {
      imsi_ptr = qcril_uim_parse_gw_imsi(
                  params_ptr->qmi_rsp_data.rsp_data.read_transparent_rsp.content.data_ptr,
                  params_ptr->qmi_rsp_data.rsp_data.read_transparent_rsp.content.data_len,
                  &imsi_length);
      is_gwl = TRUE;
    }
    else if((read_param_ptr->session_info.session_type == QMI_UIM_SESSION_TYPE_PRI_1X_PROV) ||
            (read_param_ptr->session_info.session_type == QMI_UIM_SESSION_TYPE_SEC_1X_PROV) ||
            (read_param_ptr->session_info.session_type == QMI_UIM_SESSION_TYPE_TER_1X_PROV))
    {
      imsi_ptr = qcril_uim_parse_1x_imsi(
                  params_ptr->qmi_rsp_data.rsp_data.read_transparent_rsp.content.data_ptr,
                  params_ptr->qmi_rsp_data.rsp_data.read_transparent_rsp.content.data_len,
                  &imsi_length);
      is_gwl = FALSE;
    }
  }

  if (imsi_ptr == NULL)
  {
    imsi_length = 0;
    ril_err = RIL_UIM_E_INTERNAL_ERR;
  }
  else
  {
    if (resp_ptr != NULL)
    {
      resp_ptr->IMSI = imsi_ptr;
    }

    qcril_free(imsi_ptr);
    imsi_ptr = NULL;
  }

  if (resp_ptr != NULL)
  {
    resp_ptr->err = ril_err;
  }
  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, resp_ptr);
} /* qcril_uim_get_imsi_resp */


/*=========================================================================

  FUNCTION:  qcril_uim_get_fdn_status_resp

===========================================================================*/
/*!
    @brief
    Process the response for get FDN status.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_get_fdn_status_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<UimGetFacilityLockRequestMsg> req_ptr
)
{
  RIL_UIM_Errno                     ril_err     = RIL_UIM_E_SUCCESS;
  int                               ret_value   = 0;
  std::shared_ptr<RIL_UIM_GetFacilityLockResponseMsg> rsp_ptr = NULL;

  if(params_ptr == NULL || req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("NULL params_ptr");
    QCRIL_ASSERT(0);
    return;
  }

  ril_err = qcril_uim_convert_err_value(params_ptr->qmi_rsp_data.qmi_err_code);

  QCRIL_LOG_DEBUG( "qcril_uim_get_fdn_status_resp: qmi_err_code=%d \n",
                    params_ptr->qmi_rsp_data.qmi_err_code);

  /* Map the response */
  if (params_ptr->qmi_rsp_data.qmi_err_code == 0)
  {
    switch (params_ptr->qmi_rsp_data.rsp_data.get_service_status_rsp.fdn_status)
    {
      case 0:
        ret_value = 2;
        QCRIL_LOG_INFO( "FDN service not available\n" );
        break;
      case 1:
        ret_value = 0;
        QCRIL_LOG_INFO( "FDN is available but disabled\n" );
        break;
      case 2:
        ret_value     = 1;
        QCRIL_LOG_INFO( "FDN is available and enabled\n" );
        break;
      default:
        ril_err = RIL_UIM_E_INTERNAL_ERR;
        QCRIL_LOG_ERROR( "FDN status unknown 0x%x\n",
                         params_ptr->qmi_rsp_data.rsp_data.get_service_status_rsp.fdn_status );
        break;
    }
  }

  rsp_ptr = std::make_shared<RIL_UIM_GetFacilityLockResponseMsg>();

  if (rsp_ptr != NULL)
  {
    rsp_ptr->ril_err = ril_err;
    rsp_ptr->lock_status = ret_value;
  }

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);

} /* qcril_uim_get_fdn_status_resp */


/*=========================================================================

  FUNCTION:  qcril_uim_set_fdn_status_resp

===========================================================================*/
/*!
    @brief
    Process the response for set FDN status.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_set_fdn_status_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<UimSetFacilityLockRequestMsg> req_ptr
)
{
  RIL_UIM_Errno  ril_err = RIL_UIM_E_SUCCESS;
  auto           rsp_ptr = std::make_shared<RIL_UIM_SIM_PIN_Response>();

  if(params_ptr == NULL || req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("NULL params_ptr");
    QCRIL_ASSERT(0);
    return;
  }

  ril_err = qcril_uim_convert_err_value(params_ptr->qmi_rsp_data.qmi_err_code);

  QCRIL_LOG_DEBUG( "qcril_uim_set_fdn_status_resp: qmi_err_code=%d \n",
                    params_ptr->qmi_rsp_data.qmi_err_code);

  if (rsp_ptr != NULL)
  {
    rsp_ptr->err = ril_err;
  }

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_set_fdn_status_resp */


/*=========================================================================

  FUNCTION:  qcril_uim_open_logical_channel_resp

===========================================================================*/
/*!
    @brief
    Process the response for open logical channel command.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_open_logical_channel_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<UimSIMOpenChannelRequestMsg> req_ptr
)
{
  RIL_UIM_Errno                                ril_err = RIL_UIM_E_SUCCESS;
  std::shared_ptr<RIL_UIM_OpenChannelResponse> rsp_ptr = std::make_shared<RIL_UIM_OpenChannelResponse>();
  uint8_t                                      slot    = qmi_ril_get_process_instance_id();

  if(params_ptr == NULL || req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("NULL params_ptr");
    QCRIL_ASSERT(0);
    return;
  }

  ril_err = qcril_uim_convert_err_value(params_ptr->qmi_rsp_data.qmi_err_code);

  QCRIL_LOG_DEBUG( "qcril_uim_open_logical_channel_resp: qmi_err_code=%d \n",
                    params_ptr->qmi_rsp_data.qmi_err_code);

  /* No need to call qcril_uim_remove_non_provisioning_session since we do not try
     to open one in the request, continue to generate response */

  if (params_ptr->qmi_rsp_data.qmi_err_code == 0)
  {
    uint16     open_ch_rsp_len = 0;
    uint8_t  * open_ch_rsp_ptr = NULL;

    QCRIL_LOG_DEBUG("qcril_uim_logical_channel_resp: channel_id=%d, SW1=0x%x, SW2=0x%x",
                    params_ptr->qmi_rsp_data.rsp_data.logical_channel_rsp.channel_id,
                    params_ptr->qmi_rsp_data.rsp_data.logical_channel_rsp.sw1,
                    params_ptr->qmi_rsp_data.rsp_data.logical_channel_rsp.sw2);

    /* Note that we are supporting both types of clients by this response:
       1. Clients who expects only channel id in the response. So, we have to store select
          response data if available so we can send it in the next GET RESPONSE APDU request by client
       2. Client who expects both channel id & select response as an int array -
          as per the L release support in ATEL */
    (void)qcril_uim_store_select_response_info(
            &params_ptr->qmi_rsp_data.rsp_data.logical_channel_rsp);

    /* Create the data in the form of an int array:
       0 to 'len-2' indexes are the select response bytes, one byte per integer array index,
       'len-1' index is SW1, 'len' index is SW2 (last 2 bytes) */
    if ((params_ptr->qmi_rsp_data.rsp_data.logical_channel_rsp.select_response.data_len >  0) &&
        (params_ptr->qmi_rsp_data.rsp_data.logical_channel_rsp.select_response.data_ptr != NULL))
    {
      open_ch_rsp_len = sizeof(uint8_t) *
        params_ptr->qmi_rsp_data.rsp_data.logical_channel_rsp.select_response.data_len;
    }

    /* Allocate the total response bytes. Note that even if there are no resp bytes,
       we still have SW1/SW2 bytes to return, hence the 2 bytes */
    open_ch_rsp_len += sizeof(uint8_t) * 2;
    open_ch_rsp_ptr = (uint8_t*)qcril_malloc(open_ch_rsp_len);
    if (open_ch_rsp_ptr == NULL)
    {
      QCRIL_LOG_ERROR( "Error allocating open_ch_rsp_ptr for 0x%x bytes", open_ch_rsp_len);
      open_ch_rsp_len = 0;
      ril_err = RIL_UIM_E_NO_MEMORY;
    }
    else
    {
      uint16 byte_idx = 0;

      /* Update the response with necessary info: Select resp bytes & SW1/SW2 bytes */
      if ((params_ptr->qmi_rsp_data.rsp_data.logical_channel_rsp.select_response.data_len >  0) &&
          (params_ptr->qmi_rsp_data.rsp_data.logical_channel_rsp.select_response.data_ptr != NULL))
      {
        memcpy(open_ch_rsp_ptr,
               params_ptr->qmi_rsp_data.rsp_data.logical_channel_rsp.select_response.data_ptr,
               params_ptr->qmi_rsp_data.rsp_data.logical_channel_rsp.select_response.data_len);

        byte_idx += params_ptr->qmi_rsp_data.rsp_data.logical_channel_rsp.select_response.data_len;
      }

      if (byte_idx < (open_ch_rsp_len - 1))
      {
        open_ch_rsp_ptr[byte_idx++] = params_ptr->qmi_rsp_data.rsp_data.logical_channel_rsp.sw1;
        open_ch_rsp_ptr[byte_idx]   = params_ptr->qmi_rsp_data.rsp_data.logical_channel_rsp.sw2;
      }
    }

    /* If select response data wasnt present or not allocated, pass only channel id */
    if (rsp_ptr != NULL)
    {
      rsp_ptr->err = ril_err;
      rsp_ptr->channel_id = params_ptr->qmi_rsp_data.rsp_data.logical_channel_rsp.channel_id;
      if (open_ch_rsp_ptr)
      {
        rsp_ptr->selectResponse.resize(open_ch_rsp_len);
        memcpy(&rsp_ptr->selectResponse[0], open_ch_rsp_ptr, open_ch_rsp_len);

        qcril_free(open_ch_rsp_ptr);
        open_ch_rsp_ptr = NULL;
      }
    }
    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
  }
  else
  {
    qcril_uim_fci_value_type   qcril_fci_value  = QCRIL_UIM_FCI_VALUE_FCP;

    if (req_ptr->get_p2() > QCRIL_UIM_INVALID_OPEN_CH_P2)
    {
      (void)qcril_uim_convert_p2_value(req_ptr->get_p2(), &qcril_fci_value);
    }
    else
    {
      /* When opening the channel, ask for appropriate template depending upon
         whether the it is an ICC card or a specific AID */
      if (qcril_uim_find_app_in_slot(slot, QMI_UIM_APP_SIM) ||
          qcril_uim_find_app_in_slot(slot, QMI_UIM_APP_RUIM))
      {
        qcril_fci_value = QCRIL_UIM_FCI_VALUE_FCP;
      }
      else
      {
        qcril_fci_value = qcril_uim_determine_select_template_from_aid(req_ptr->get_aid().c_str());
      }
    }

    /* In the cases that open channel fails due to incorrect P1/P2 (this generally
       means that the FCI value was incorrect), retry opening the channel but with
       an alternative FCI value. */
    if (params_ptr->qmi_rsp_data.rsp_data.logical_channel_rsp.sw1 == QCRIL_UIM_SW1_WRONG_PARAMS &&
        params_ptr->qmi_rsp_data.rsp_data.logical_channel_rsp.sw2 == QCRIL_UIM_SW2_BAD_PARAMS_P1_P2 &&
        qcril_fci_value == QCRIL_UIM_FCI_VALUE_FCI_FALLBACK_FCP)
    {
      qmi_uim_open_logical_channel_params_type open_logical_ch_params             = {};
      uint8                                    aid_buffer[QMI_UIM_MAX_AID_LEN]    = {};
      uint16                                   aid_size                           = 0;

      if (!req_ptr->get_aid().empty())
      {
        /* Convert AID string into binary */
        aid_size = qcril_uim_hexstring_to_bin(req_ptr->get_aid().c_str(),
                                              aid_buffer,
                                              QMI_UIM_MAX_AID_LEN);
        if (aid_size == 0 || aid_size > QMI_UIM_MAX_AID_LEN)
        {
          QCRIL_LOG_ERROR("Error converting AID string into binary");
        }
      }

      QCRIL_LOG_ERROR( "%s\n", "Entering fallback to retry SELECT with alternative FCI value");

      open_logical_ch_params.aid_present  = QMI_UIM_TRUE;
      open_logical_ch_params.aid.data_ptr = aid_buffer;
      open_logical_ch_params.aid.data_len = aid_size;
      open_logical_ch_params.slot         = (qmi_uim_slot_type)((uint8)QMI_UIM_SLOT_1 + qmi_ril_get_process_instance_id());
      open_logical_ch_params.file_control_information.is_valid  = QMI_UIM_TRUE;
      open_logical_ch_params.file_control_information.fci_value = QMI_UIM_FCI_VALUE_FCP;

      /* Proceed with open logical channel request */
      if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_OPEN_LOGICAL_CHANNEL,
                                       (void *)&open_logical_ch_params,
                                       req_ptr) >= 0)
      {
        return;
      }
    }

    /* On error, map to appropriate ril error code */
    switch (params_ptr->qmi_rsp_data.qmi_err_code)
    {
      case QMI_UIM_SERVICE_ERR_INSUFFICIENT_RESOURCES:
        ril_err = RIL_UIM_E_MISSING_RESOURCE;
        break;

      case QMI_UIM_SERVICE_ERR_INCOMPATIBLE_STATE:
      case QMI_UIM_SERVICE_ERR_SIM_FILE_NOT_FOUND:
        ril_err = RIL_UIM_E_NO_SUCH_ELEMENT;
        break;

      default:
        ril_err = qcril_uim_convert_err_value(params_ptr->qmi_rsp_data.qmi_err_code);
        break;
    }

    if (rsp_ptr != NULL)
    {
      rsp_ptr->err = ril_err;
      rsp_ptr->channel_id = params_ptr->qmi_rsp_data.rsp_data.logical_channel_rsp.channel_id;
      if (params_ptr->qmi_rsp_data.rsp_data.logical_channel_rsp.sw1_sw2_valid == QMI_UIM_TRUE)
      {
        uint8_t open_ch_rsp[2] = {0};

        open_ch_rsp[0] = params_ptr->qmi_rsp_data.rsp_data.logical_channel_rsp.sw1;
        open_ch_rsp[1] = params_ptr->qmi_rsp_data.rsp_data.logical_channel_rsp.sw2;

        rsp_ptr->selectResponse.resize(2);
        memcpy(&rsp_ptr->selectResponse[0], open_ch_rsp, 2);
      }
    }
    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
  }
} /* qcril_uim_open_logical_channel_resp */


/*=========================================================================

  FUNCTION:  qcril_uim_close_logical_channel_resp

===========================================================================*/
/*!
    @brief
    Process the response for close logical channel command.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_close_logical_channel_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<UimSIMCloseChannelRequestMsg> req_ptr
)
{
  RIL_UIM_Errno                  ril_err = RIL_UIM_E_SUCCESS;
  std::shared_ptr<RIL_UIM_Errno> rsp_ptr = NULL;

  if(params_ptr == NULL || req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("NULL params_ptr");
    QCRIL_ASSERT(0);
    return;
  }

  QCRIL_LOG_DEBUG( "qcril_uim_close_logical_channel_resp: qmi_err_code=%d \n",
                    params_ptr->qmi_rsp_data.qmi_err_code);

  ril_err = qcril_uim_convert_err_value(params_ptr->qmi_rsp_data.qmi_err_code);

  rsp_ptr = std::make_shared<RIL_UIM_Errno>(ril_err);

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_close_logical_channel_resp */


/*=========================================================================

  FUNCTION:  qcril_uim_reselect_resp

===========================================================================*/
/*!
    @brief
    Process the response for reselect. Note that this request comes via the
    send apdu API, hence the response uses that data type.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_reselect_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<UimReselectReqMsg> msg_ptr
)
{
  RIL_UIM_Errno                     ril_err         = RIL_UIM_E_SUCCESS;
  unsigned short                    select_resp_len = 0;
  RIL_UIM_SIM_IO_Response           ril_response    = {};
  qmi_uim_send_apdu_params_type   * send_apdu_params = NULL;
  std::shared_ptr<RIL_UIM_SIM_IO_Response> rsp_ptr  = NULL;
  std::shared_ptr<UimTransmitAPDURequestMsg> req_ptr = NULL;

  if(params_ptr == NULL || msg_ptr == NULL)
  {
    QCRIL_LOG_ERROR("NULL params_ptr");
    QCRIL_ASSERT(0);
    return;
  }

  send_apdu_params = msg_ptr->get_params();
  req_ptr = msg_ptr->get_request_msg();

  if (send_apdu_params == NULL ||
      req_ptr == NULL)
  {
    return;
  }

  ril_err = qcril_uim_convert_err_value(params_ptr->qmi_rsp_data.qmi_err_code);

  QCRIL_LOG_DEBUG( "qcril_uim_reselect_resp: qmi_err_code=%d \n",
                    params_ptr->qmi_rsp_data.qmi_err_code );

  /* If the error code was 'QMI_UIM_SERVICE_ERR_INVALID_QMI_CMD' it means that the reselect QMI API
     is not supported by modem, send reselect cmd via QM_UIM_SEND_APDU to be b/w compatible.
     For any other error code, send the error back to the client */
  if (params_ptr->qmi_rsp_data.qmi_err_code == QMI_UIM_SERVICE_ERR_INVALID_QMI_CMD)
  {
    QCRIL_LOG_ERROR( "%s\n", "Fallback to reselect via QMI_UIM_SEND_APDU");

    /* Proceed with send APDU request */
    if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_SEND_APDU,
                                     (void *)msg_ptr->get_params(),
                                     msg_ptr->get_request_msg()) >= 0)
    {
      if (send_apdu_params != NULL && send_apdu_params->apdu.data_ptr != NULL)
      {
        qcril_free(send_apdu_params->apdu.data_ptr);
        send_apdu_params->apdu.data_ptr = NULL;
      }
      return;
    }
  }

  memset(&ril_response, 0, sizeof(RIL_SIM_IO_Response));

  if (params_ptr->qmi_rsp_data.qmi_err_code == QMI_UIM_SERVICE_ERR_NOT_SUPPORTED)
  {
    /* Fake 6F 00 response for not supported reselect AID */
    ril_response.sw1 = QCRIL_UIM_SW1_TECH_PROBLEM;
    ril_response.sw2 = QCRIL_UIM_SW2_TECH_PROBLEM;
  }
  else
  {
    /* SW1 & SW2 are the last 2 bytes of the APDU response */
    ril_response.sw1 = params_ptr->qmi_rsp_data.rsp_data.reselect_rsp.sw1;
    ril_response.sw2 = params_ptr->qmi_rsp_data.rsp_data.reselect_rsp.sw2;
    if (params_ptr->qmi_rsp_data.rsp_data.reselect_rsp.select_response.data_len > 0)
    {
      char *simResponse = NULL;

      select_resp_len = params_ptr->qmi_rsp_data.rsp_data.reselect_rsp.select_response.data_len;
      ril_response.simResponse = qcril_uim_alloc_bin_to_hexstring(
                                    params_ptr->qmi_rsp_data.rsp_data.reselect_rsp.select_response.data_ptr,
                                    select_resp_len);
      if (simResponse != NULL)
      {
        ril_response.simResponse = simResponse;
        qcril_free(simResponse);
        simResponse = NULL;
      }
    }
    ril_err = RIL_UIM_E_SUCCESS;
  }

  QCRIL_LOG_DEBUG( "RIL_SIM_IO_Response: sw1=0x%X sw2=0x%X data=%s\n",
                    ril_response.sw1, ril_response.sw2,
                    ril_response.simResponse.c_str());


  ril_response.err = ril_err;
  ril_response.slot_id = req_ptr->get_slot();
  rsp_ptr = std::make_shared<RIL_UIM_SIM_IO_Response>(ril_response);

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);

  /* Free APDU data pointer if needed */
  if (send_apdu_params != NULL && send_apdu_params->apdu.data_ptr != NULL)
  {
    qcril_free(send_apdu_params->apdu.data_ptr);
    send_apdu_params->apdu.data_ptr = NULL;
  }
} /* qcril_uim_reselect_resp */


/*=========================================================================

  FUNCTION:  qcril_uim_send_apdu_resp

===========================================================================*/
/*!
    @brief
    Process the response for send apdu command.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_send_apdu_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<UimTransmitAPDURequestMsg> req_ptr
)
{
  RIL_UIM_Errno                            ril_err      = RIL_UIM_E_SUCCESS;
  unsigned short                           apdu_len     = 0;
  RIL_UIM_SIM_IO_Response                  ril_response = {};
  std::shared_ptr<RIL_UIM_SIM_IO_Response> rsp_ptr      = NULL;

  if(params_ptr == NULL)
  {
    QCRIL_LOG_ERROR("NULL params_ptr");
    QCRIL_ASSERT(0);
    return;
  }

  ril_err = qcril_uim_convert_err_value(params_ptr->qmi_rsp_data.qmi_err_code);

  QCRIL_LOG_DEBUG( "qcril_uim_send_apdu_resp: qmi_err_code=%d \n",
                    params_ptr->qmi_rsp_data.qmi_err_code );

  /* Special case for long APDUs - we send the response of the long APDU
     stream after we get all the chunks in the corresponding SEND_APDU_INDs */
  if (params_ptr->qmi_rsp_data.qmi_err_code == QMI_UIM_SERVICE_ERR_INSUFFICIENT_RESOURCES)
  {
    QCRIL_LOG_DEBUG( "qcril_uim_send_apdu_resp: total_len=0x%x bytes, token=0x%x",
                     params_ptr->qmi_rsp_data.rsp_data.send_apdu_rsp.total_len,
                     params_ptr->qmi_rsp_data.rsp_data.send_apdu_rsp.token );

    /* Store only if the Long APDU response TLV is valid. We also need to check
       and handle cases where the INDs might have come earlier than this response.
       Note that original_request_ptr will be freed when we get SEND_APDU_INDs */
    if (qcril_uim.long_apdu_info.in_use == TRUE)
    {
      /* If Indication already came, we need to check incoming info */
      if ((qcril_uim.long_apdu_info.token          ==
             params_ptr->qmi_rsp_data.rsp_data.send_apdu_rsp.token) &&
          (qcril_uim.long_apdu_info.total_apdu_len ==
             params_ptr->qmi_rsp_data.rsp_data.send_apdu_rsp.total_len))
      {
        /* If Indication already came & incoming info matches,
           nothing else to do */
        return;
      }

      /* Error condition - mismatch in data, send error if there was any previous
         request & store the current response's token */
      QCRIL_LOG_ERROR("Mismatch with global data, token: 0x%x, total_apdu_len: 0x%x",
                      qcril_uim.long_apdu_info.token,
                      qcril_uim.long_apdu_info.total_apdu_len);
      /* Send error for any pending response & proceed to store current resp info */
      qcril_uim_cleanup_long_apdu_info();
    }

    /* Store response info. We return after successfully storing since
       we expect subsequent INDs */
    if (params_ptr->qmi_rsp_data.rsp_data.send_apdu_rsp.total_len > 0)
    {
      QCRIL_LOG_INFO("Storing long_apdu_info");
      qcril_uim.long_apdu_info.in_use = TRUE;
      qcril_uim.long_apdu_info.token = params_ptr->qmi_rsp_data.rsp_data.send_apdu_rsp.token;
      qcril_uim.long_apdu_info.total_apdu_len = params_ptr->qmi_rsp_data.rsp_data.send_apdu_rsp.total_len;
      qcril_uim.long_apdu_info.req_ptr = req_ptr;
      return;
    }
  }

  if (ril_err == RIL_UIM_E_SUCCESS)
  {
    /* SW1 & SW2 are the last 2 bytes of the APDU response */
    if (params_ptr->qmi_rsp_data.rsp_data.send_apdu_rsp.apdu_response.data_len >= 2)
    {
      char *simResponse = NULL;
      apdu_len = params_ptr->qmi_rsp_data.rsp_data.send_apdu_rsp.apdu_response.data_len;
      ril_response.sw1 = params_ptr->qmi_rsp_data.rsp_data.send_apdu_rsp.apdu_response.data_ptr[apdu_len-2];
      ril_response.sw2 = params_ptr->qmi_rsp_data.rsp_data.send_apdu_rsp.apdu_response.data_ptr[apdu_len-1];

      simResponse = qcril_uim_alloc_bin_to_hexstring(
                                    params_ptr->qmi_rsp_data.rsp_data.send_apdu_rsp.apdu_response.data_ptr,
                                    apdu_len - 2);

      if (simResponse != NULL)
      {
        ril_response.simResponse = simResponse;
        qcril_free(simResponse);
        simResponse = NULL;
      }
    }
    else
    {
      ril_err = RIL_UIM_E_INTERNAL_ERR;
      QCRIL_LOG_ERROR( "Invalid apdu_response.data_len: 0x%x\n",
                       params_ptr->qmi_rsp_data.rsp_data.send_apdu_rsp.apdu_response.data_len );
    }
  }

  QCRIL_LOG_DEBUG( "RIL_SIM_IO_Response: sw1=0x%X sw2=0x%X data=%s\n",
                    ril_response.sw1, ril_response.sw2,
                    ril_response.simResponse.c_str());

  ril_response.err = ril_err;
  ril_response.slot_id = req_ptr->get_slot();

  rsp_ptr = std::make_shared<RIL_UIM_SIM_IO_Response>(ril_response);
  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_send_apdu_resp */


/*=========================================================================

  FUNCTION:  qcril_uim_set_apdu_behavior_resp

===========================================================================*/
/*!
    @brief
    Processes the response for set APDU behavior command.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_set_apdu_behavior_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<UimSetAPDUBehaviorRequestMsg> req_ptr
)
{
  RIL_UIM_Errno                   ril_err  = RIL_UIM_E_SUCCESS;
  std::shared_ptr<RIL_UIM_Errno>  rsp_ptr  = NULL;

  /* Sanity checks */
  if (params_ptr == NULL || req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  ril_err = qcril_uim_convert_err_value(params_ptr->qmi_rsp_data.qmi_err_code);

  QCRIL_LOG_DEBUG( "qcril_uim_set_apdu_behavior_resp: qmi_err_code=%d \n",
                    params_ptr->qmi_rsp_data.qmi_err_code );

  rsp_ptr = std::make_shared<RIL_UIM_Errno>(ril_err);

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_set_apdu_behavior_resp */


/*===========================================================================

  FUNCTION:  qcril_uim_process_send_apdu_ind

===========================================================================*/
/*!
    @brief
    Function for processing send APDU indication. Based on the data received
    in the send APDU response, this routine is responsible for concatenating
    all the chunks of the APDU indication & preparing & sending one long APDU
    to the client.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_process_send_apdu_ind
(
  const qcril_uim_indication_params_type  * ind_param_ptr
)
{
  uint16                            remaining_len = 0;
  uint16                            stored_len    = 0;
  qmi_uim_send_apdu_ind_type      * apdu_ind_ptr  = NULL;

  if(ind_param_ptr == NULL)
  {
    QCRIL_LOG_ERROR("NULL pointer");
    QCRIL_ASSERT(0);
    return;
  }

  apdu_ind_ptr = (qmi_uim_send_apdu_ind_type*)&ind_param_ptr->ind_data.send_apdu_ind;

  QCRIL_LOG_INFO("Send APDU Indication - token: 0x%x, total_len: 0x%x, offset: 0x%x, data_len: 0x%x",
                 apdu_ind_ptr->token,
                 apdu_ind_ptr->total_len,
                 apdu_ind_ptr->offset,
                 apdu_ind_ptr->apdu.data_len);

  if (qcril_uim.long_apdu_info.in_use == TRUE)
  {
    /* If Response already came, we need to check incoming info */
    if ((qcril_uim.long_apdu_info.token          != apdu_ind_ptr->token) ||
        (qcril_uim.long_apdu_info.total_apdu_len != apdu_ind_ptr->total_len))
    {
      /* Error condition - mismatch in data, discrd the response */
      QCRIL_LOG_ERROR("Mismatch with global data, token: 0x%x, total_apdu_len: 0x%x",
                    qcril_uim.long_apdu_info.token,
                    qcril_uim.long_apdu_info.total_apdu_len);
      qcril_uim_cleanup_long_apdu_info();
      return;
    }
  }
  else
  {
    /* Response hasn't come yet, we can still store IND info */
    QCRIL_LOG_INFO("long_apdu_info.in_use is FALSE, storing info");
    qcril_uim.long_apdu_info.in_use         = TRUE;
    qcril_uim.long_apdu_info.token          = apdu_ind_ptr->token;
    qcril_uim.long_apdu_info.total_apdu_len = apdu_ind_ptr->total_len;
  }

  /* If this is the first chunk, allocate the buffer. This buffer will
     only be freed at the end of the receiving all the INDs */
  if (qcril_uim.long_apdu_info.apdu_ptr == NULL)
  {
    qcril_uim.long_apdu_info.rx_len = 0;
    qcril_uim.long_apdu_info.apdu_ptr = (uint8 *)qcril_malloc(apdu_ind_ptr->total_len);
    if (qcril_uim.long_apdu_info.apdu_ptr == NULL)
    {
      QCRIL_LOG_ERROR("Couldnt allocate apdu_ptr pointer !");
      qcril_uim.long_apdu_info.in_use = FALSE;
      return;
    }
  }

  /* Find out the remaining APDU buffer length */
  stored_len    = qcril_uim.long_apdu_info.rx_len;
  remaining_len = qcril_uim.long_apdu_info.total_apdu_len - stored_len;

  /* If this chunk cannot fit in our global buffer, discard the IND */
  if ((apdu_ind_ptr->apdu.data_len > remaining_len) ||
      (apdu_ind_ptr->offset  >= qcril_uim.long_apdu_info.total_apdu_len) ||
      ((apdu_ind_ptr->offset + apdu_ind_ptr->apdu.data_len) > qcril_uim.long_apdu_info.total_apdu_len))
  {
    QCRIL_LOG_ERROR("Mismatch with global data, total_apdu_len: 0x%x stored_len: 0x%x, remaining_len: 0x%x",
                    qcril_uim.long_apdu_info.total_apdu_len,
                    stored_len,
                    remaining_len);
    qcril_uim.long_apdu_info.in_use = FALSE;
    return;
  }

  /* Save the data & update the data length */
  memcpy(qcril_uim.long_apdu_info.apdu_ptr + apdu_ind_ptr->offset,
         apdu_ind_ptr->apdu.data_ptr,
         apdu_ind_ptr->apdu.data_len);
  qcril_uim.long_apdu_info.rx_len += apdu_ind_ptr->apdu.data_len;

  /* If it is the last one, send the response back & clean up global buffer */
  if (qcril_uim.long_apdu_info.total_apdu_len == qcril_uim.long_apdu_info.rx_len)
  {
    unsigned short                              apdu_len     = qcril_uim.long_apdu_info.rx_len;
    RIL_UIM_SIM_IO_Response                     ril_response = {};
    char                                       *simResponse  = NULL;
    std::shared_ptr<RIL_UIM_SIM_IO_Response>    rsp_ptr      = NULL;
    std::shared_ptr<UimTransmitAPDURequestMsg>  req_ptr      = NULL;

    QCRIL_LOG_DEBUG("qcril_uim_process_send_apdu_ind: apdu_len=0x%x",
                     apdu_len);

    req_ptr = qcril_uim.long_apdu_info.req_ptr;

    if (req_ptr != NULL)
    {
      ril_response.sw1 = qcril_uim.long_apdu_info.apdu_ptr[apdu_len-2];
      ril_response.sw2 = qcril_uim.long_apdu_info.apdu_ptr[apdu_len-1];
      simResponse = qcril_uim_alloc_bin_to_hexstring(
                                   qcril_uim.long_apdu_info.apdu_ptr,
                                   apdu_len - 2);
      if(simResponse)
      {
        ril_response.simResponse = simResponse;
        qcril_free(simResponse);
        simResponse = NULL;
      }
      ril_response.err = RIL_UIM_E_SUCCESS;

      ril_response.slot_id = req_ptr->get_slot(); 

      QCRIL_LOG_DEBUG( "RIL_SIM_IO_Response: sw1=0x%X sw2=0x%X data=%s",
                       ril_response.sw1, ril_response.sw2,
                       ril_response.simResponse.c_str());

      rsp_ptr = std::make_shared<RIL_UIM_SIM_IO_Response>(ril_response);
      
      req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);

      qcril_uim.long_apdu_info.req_ptr = NULL;
    }

    /* Also clean up the global APDU data */
    qcril_uim_cleanup_long_apdu_info();
  }
} /* qcril_uim_process_send_apdu_ind */


/*===========================================================================

  FUNCTION:  qcril_uim_cleanup_long_apdu_info

===========================================================================*/
/*!
    @brief
    Frees if any memory is allocated in global APDU info structure. It also
    sends an error response in case the original request pointer is still
    pending.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_cleanup_long_apdu_info
(
  void
)
{
  /* Free any internal buffers we might have */
  if (qcril_uim.long_apdu_info.apdu_ptr != NULL)
  {
    qcril_free(qcril_uim.long_apdu_info.apdu_ptr);
    qcril_uim.long_apdu_info.apdu_ptr = NULL;
  }
  /* Send error response if request is still pending */
  if (qcril_uim.long_apdu_info.req_ptr != NULL)
  {
    std::shared_ptr<RIL_UIM_SIM_IO_Response>    rsp_ptr      = std::make_shared<RIL_UIM_SIM_IO_Response>();
    std::shared_ptr<UimTransmitAPDURequestMsg>  req_ptr      = qcril_uim.long_apdu_info.req_ptr;

    if (rsp_ptr != NULL)
    {
      rsp_ptr->err = RIL_UIM_E_INTERNAL_ERR;
      rsp_ptr->slot_id = req_ptr->get_slot();
    }
    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    /* Generate response */
  }

  memset(&qcril_uim.long_apdu_info,
         0,
         sizeof(qcril_uim_long_apdu_info_type));
} /* qcril_uim_cleanup_long_apdu_info */


/*===========================================================================

  FUNCTION:  qcril_uim_cleanup_select_response_info

===========================================================================*/
/*!
    @brief
    Cleans the global select response info structure & frees any memory
    allocated for the raw response data.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_cleanup_select_response_info
(
  void
)
{
  uint8  select_resp_index = 0;

  /* Loop thorough all the entries for the particular slot &
     free any cached info we might have */
  for (select_resp_index = 0; select_resp_index < QCRIL_UIM_MAX_SELECT_RESP_COUNT; select_resp_index++)
  {
    if (qcril_uim.select_response_info[select_resp_index].select_resp_ptr != NULL)
    {
      qcril_free(qcril_uim.select_response_info[select_resp_index].select_resp_ptr);
      qcril_uim.select_response_info[select_resp_index].select_resp_ptr = NULL;
    }
    memset(&qcril_uim.select_response_info[select_resp_index],
           0,
           sizeof(qcril_uim_select_response_info_type));
  }
} /* qcril_uim_cleanup_select_response_info */


/*=========================================================================

  FUNCTION:  qcril_uim_sim_authenticate_resp

===========================================================================*/
/*!
    @brief
    Process the response for SIM authenticate command.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_sim_authenticate_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<Message> req_ptr
)
{
  RIL_UIM_Errno   ril_err       = RIL_UIM_E_INTERNAL_ERR;
  char          * auth_resp_ptr = NULL;
  auto            rsp_ptr = std::make_shared<RIL_UIM_SIM_IO_Response>();

  if(params_ptr == NULL || req_ptr == NULL || rsp_ptr == NULL)
  {
    QCRIL_LOG_ERROR("NULL params_ptr");
    return;
  }

  ril_err = qcril_uim_convert_err_value(params_ptr->qmi_rsp_data.qmi_err_code);

  QCRIL_LOG_DEBUG( "qcril_sim_authenticate_resp: qmi_err_code=%d\n",
                    params_ptr->qmi_rsp_data.qmi_err_code);

  if (ril_err == RIL_UIM_E_SUCCESS)
  {
    auth_resp_ptr = qcril_uim_alloc_bin_to_base64string(
                      params_ptr->qmi_rsp_data.rsp_data.authenticate_rsp.auth_response.data_ptr,
                      params_ptr->qmi_rsp_data.rsp_data.authenticate_rsp.auth_response.data_len);

    if (auth_resp_ptr != NULL)
    {
      rsp_ptr->simResponse = auth_resp_ptr;
      qcril_free(auth_resp_ptr);
      auth_resp_ptr = NULL;
    }
    else
    {
      ril_err = RIL_UIM_E_NO_MEMORY;
    }
  }

  rsp_ptr->slot_id = qmi_ril_get_process_instance_id();
  rsp_ptr->err = ril_err;
  rsp_ptr->sw1 = params_ptr->qmi_rsp_data.rsp_data.authenticate_rsp.sw1;
  rsp_ptr->sw2 = params_ptr->qmi_rsp_data.rsp_data.authenticate_rsp.sw2;

  if (string(UimSIMAuthenticationRequestMsg::MESSAGE_NAME) == req_ptr->get_message_name())
  {
    std::static_pointer_cast<UimSIMAuthenticationRequestMsg>(req_ptr)->sendResponse
                              (req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
  }
  else if (string(UimISIMAuthenticationRequestMsg::MESSAGE_NAME) == req_ptr->get_message_name())
  {
    std::static_pointer_cast<UimISIMAuthenticationRequestMsg>(req_ptr)->sendResponse
                               (req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
  }
  else
  {
    QCRIL_LOG_ERROR("Unknow message :%s", req_ptr->get_message_name().c_str());
  }
} /* qcril_uim_sim_authenticate_resp */


/*=========================================================================

  FUNCTION:  qcril_uim_send_status_resp

===========================================================================*/
/*!
    @brief
    Process the response for STATUS command.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_send_status_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<UimGetStatusReqMsg> msg_ptr
)
{
  RIL_UIM_Errno ril_err  = RIL_UIM_E_SUCCESS;
  auto          rsp_ptr  = std::make_shared<RIL_UIM_SIM_IO_Response>();

  if(params_ptr == NULL || msg_ptr == NULL)
  {
    QCRIL_LOG_ERROR("NULL params_ptr");
    QCRIL_ASSERT(0);
    return;
  }

  auto req_ptr = std::static_pointer_cast<UimSIMIORequestMsg>(msg_ptr->get_request_msg());

  ril_err = qcril_uim_convert_err_value(params_ptr->qmi_rsp_data.qmi_err_code);

  QCRIL_LOG_DEBUG( "qcril_uim_send_status_resp: qmi_err_code=%d \n",
                    params_ptr->qmi_rsp_data.qmi_err_code);

  if (rsp_ptr != NULL)
  {
    rsp_ptr->err = ril_err;
    rsp_ptr->slot_id = req_ptr->get_slot();

    if (params_ptr->qmi_rsp_data.rsp_data.send_status_rsp.status_response.data_len > 0)
    {
      char * simResponse = NULL;

      simResponse = qcril_uim_alloc_bin_to_hexstring(
                                    params_ptr->qmi_rsp_data.rsp_data.send_status_rsp.status_response.data_ptr,
                                    params_ptr->qmi_rsp_data.rsp_data.send_status_rsp.status_response.data_len);

      if (simResponse != NULL)
      {
        rsp_ptr->simResponse = simResponse;
        qcril_free(simResponse);
        simResponse = NULL;
      }
    }

    if (ril_err == RIL_UIM_E_SUCCESS)
    {
      rsp_ptr->sw1 = 0x90;
      rsp_ptr->sw2 = 0x00;
    }

    QCRIL_LOG_DEBUG( "RIL_SIM_IO_Response: sw1=0x%X sw2=0x%X data=%s",
                     rsp_ptr->sw1, rsp_ptr->sw2,
                     rsp_ptr->simResponse.c_str());
  }

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_send_status_resp */


/*=========================================================================

  FUNCTION:  qcril_uim_get_gid_resp

===========================================================================*/
/*!
    @brief
    Processes the response for GID request

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_get_gid_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<UimReadTransparentReqMsg> msg_ptr
)
{
  RIL_UIM_Errno                         ril_err  = RIL_UIM_E_INTERNAL_ERR;
  std::shared_ptr<RIL_UIM_GIDResponse>  rsp_ptr  = std::make_shared<RIL_UIM_GIDResponse>();
  uint8_t                             * data_ptr = NULL;
  uint32_t                              data_len = 0;

  if(params_ptr == NULL || msg_ptr == NULL || rsp_ptr == NULL)
  {
    QCRIL_LOG_ERROR("NULL params_ptr");
    QCRIL_ASSERT(0);
    return;
  }

  auto req_ptr = msg_ptr->get_request_msg();

  if (req_ptr == NULL)
  {
    return;
  }

  data_ptr = params_ptr->qmi_rsp_data.rsp_data.read_transparent_rsp.content.data_ptr;
  data_len = params_ptr->qmi_rsp_data.rsp_data.read_transparent_rsp.content.data_len;
  ril_err = qcril_uim_convert_err_value(params_ptr->qmi_rsp_data.qmi_err_code);

  QCRIL_LOG_DEBUG( "qcril_uim_get_gid_resp: qmi_err_code=%d\n",
                    params_ptr->qmi_rsp_data.qmi_err_code);

  rsp_ptr->err = ril_err;
  rsp_ptr->GIDResponse.resize(data_len);
  if (data_ptr != NULL && data_len != 0)
  {
    memcpy(&rsp_ptr->GIDResponse[0], data_ptr, data_len);
  }
  
  std::static_pointer_cast<UimGetGIDRequestMsg>(req_ptr)->sendResponse
                     (req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_get_gid_resp */


/*=========================================================================

  FUNCTION:  qcril_uim_file_get_mcc_mnc_get_path_and_session_info

===========================================================================*/
/*!
    @brief
    Helper function to get Path and Session info based on AID

    @return
    RIL_UIM_Errno
*/
/*=========================================================================*/
static RIL_UIM_Errno qcril_uim_file_get_mcc_mnc_get_path_and_session_info
(
   const char                 *ril_aid_ptr,
   qmi_uim_app_type            app_type,
   qmi_uim_data_type          *file_path_ptr,
   qmi_uim_session_info_type  *session_info_ptr,
   uint32                      slot
)
{
  RIL_UIM_Errno  ret_err             = RIL_UIM_E_INTERNAL_ERR;
  uint16         first_level_df_path = 0;
  uint8          file_path[3][4]     = {
                                        {0x3F, 0x00, 0x7F, 0x20}, /* DF GSM        */
                                        {0x3F, 0x00, 0x7F, 0x25}, /* DF CDMA       */
                                        {0x3F, 0x00, 0x7F, 0xFF}  /* ADF USIM/CSIM */
                                       };

  /* Sanity check */
  if (NULL == file_path_ptr || NULL == session_info_ptr)
  {
    QCRIL_LOG_ERROR("NULL pointer, file_path_ptr=0x%x, session_info_ptr=0x%x",
                    file_path_ptr, session_info_ptr);
    return RIL_UIM_E_INTERNAL_ERR;
  }

  if(slot >= QMI_UIM_MAX_CARD_COUNT)
  {
    QCRIL_LOG_ERROR("Invalid slot value 0x%x", slot);
    QCRIL_ASSERT(0);
    return RIL_UIM_E_INTERNAL_ERR;
  }

  /* Update the file path based on passed aid pointer or app_type if aid is NULL or empty */
  QCRIL_UIM_FREE_IF_NOT_NULL(file_path_ptr->data_ptr);
  if ((ril_aid_ptr && strlen(ril_aid_ptr) > 0) ||
      (app_type == QMI_UIM_APP_USIM) ||
      (app_type == QMI_UIM_APP_CSIM))
  {
    file_path_ptr->data_ptr = (unsigned char *)qcril_malloc(QCRIL_UIM_IMSI_PATH_SIZE);

    if (file_path_ptr->data_ptr != NULL)
    {
      memcpy(file_path_ptr->data_ptr, file_path[2], QCRIL_UIM_IMSI_PATH_SIZE);
    }
    first_level_df_path = QCRIL_UIM_FILEID_ADF_USIM_CSIM;
  }
  else if (qcril_uim_find_app_in_slot(slot, app_type))
  {
    if (app_type == QMI_UIM_APP_SIM)
    {
      /* If SIM App is present we always get EF-AD or EF-IMSI from DF GSM */
      file_path_ptr->data_ptr = (unsigned char *)qcril_malloc(QCRIL_UIM_IMSI_PATH_SIZE);

      if (file_path_ptr->data_ptr != NULL)
      {
        memcpy(file_path_ptr->data_ptr, file_path[0], QCRIL_UIM_IMSI_PATH_SIZE);
      }
      first_level_df_path = QCRIL_UIM_FILEID_DF_GSM;
    }
    else if(app_type == QMI_UIM_APP_RUIM)
    {
      /* If RUIM App is present, we get EF-IMSI_M from DF CDMA*/
      file_path_ptr->data_ptr = (unsigned char *)qcril_malloc(QCRIL_UIM_IMSI_PATH_SIZE);

      if (file_path_ptr->data_ptr != NULL)
      {
        memcpy(file_path_ptr->data_ptr, file_path[1], QCRIL_UIM_IMSI_PATH_SIZE);
      }
      first_level_df_path   = QCRIL_UIM_FILEID_DF_CDMA;
    }
  }
  else
  {
    QCRIL_LOG_ERROR("Not supported App\n");
    return RIL_UIM_E_INTERNAL_ERR;
  }

  file_path_ptr->data_len = file_path_ptr->data_ptr ? QCRIL_UIM_IMSI_PATH_SIZE : 0;
  if (file_path_ptr->data_ptr)
  {
    /* Extract session information */
    ret_err = qcril_uim_extract_session_type(slot,
                                             ril_aid_ptr,
                                             first_level_df_path,
                                             session_info_ptr,
                                             NULL,
                                             0);
    QCRIL_LOG_INFO("extract_session_type status=0x%x, session_type=0x%x\n",
                   ret_err, session_info_ptr->session_type);

    /* Free allocated memory in case of error */
    if (ret_err != RIL_UIM_E_SUCCESS)
    {
      QCRIL_UIM_FREE_IF_NOT_NULL(file_path_ptr->data_ptr);
    }
  }
  else
  {
    ret_err = RIL_UIM_E_INVALID_ARGUMENTS;
  }

  return ret_err;
}/* qcril_uim_file_get_mcc_mnc_get_path_and_session_info */


/*=========================================================================

  FUNCTION:  qcril_uim_request_get_mcc_mnc

===========================================================================*/
/*!
    @brief
    Process the request to get MCC and MNC

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_get_mcc_mnc
(
  std::shared_ptr<UimGetMccMncRequestMsg> req_ptr
)
{
  RIL_UIM_Errno                                  ret_err         = RIL_UIM_E_SUCCESS;
  qmi_uim_read_transparent_params_type           read_params     = {};
  std::shared_ptr<qcril_uim_mcc_mnc_info_type>   mcc_mnc_info    = NULL;
  std::shared_ptr<UimReadTransparentReqMsg>      req_msg_ptr     = NULL;

  /* Sanity check */
  if(NULL == req_ptr)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot process response");
    QCRIL_ASSERT(0);
    return;
  }

  /* Get file path and session type info */
  ret_err = qcril_uim_file_get_mcc_mnc_get_path_and_session_info(
                                        req_ptr->get_aid().c_str(),
                                        (qmi_uim_app_type)req_ptr->get_app_type(),
                                        &read_params.file_id.path,
                                        &read_params.session_info,
                                        qmi_ril_get_sim_slot());
  if (ret_err != RIL_UIM_E_SUCCESS)
  {
    goto send_resp;
  }

  /* Check session type & update file_id */
  if((read_params.session_info.session_type == QMI_UIM_SESSION_TYPE_PRI_GW_PROV) ||
     (read_params.session_info.session_type == QMI_UIM_SESSION_TYPE_SEC_GW_PROV) ||
     (read_params.session_info.session_type == QMI_UIM_SESSION_TYPE_TER_GW_PROV))
  {
    read_params.file_id.file_id = QCRIL_UIM_FILEID_EF_AD;
  }
  else if ((read_params.session_info.session_type == QMI_UIM_SESSION_TYPE_PRI_1X_PROV) ||
           (read_params.session_info.session_type == QMI_UIM_SESSION_TYPE_SEC_1X_PROV) ||
           (read_params.session_info.session_type == QMI_UIM_SESSION_TYPE_TER_1X_PROV))
  {
    read_params.file_id.file_id = QCRIL_UIM_FILEID_EF_IMSI_M;
  }
  else
  {
    QCRIL_LOG_ERROR("Not proper session type 0x%x for UIM_GET_MCC_MNC",
                    read_params.session_info.session_type);
    ret_err = RIL_UIM_E_REQUEST_NOT_SUPPORTED;
    goto send_resp;
  }

  req_msg_ptr = std::make_shared<UimReadTransparentReqMsg>(req_ptr,
                                                           read_params);

  if (req_msg_ptr == NULL)
  {
    ret_err = RIL_UIM_E_NO_MEMORY;
    goto send_resp;
  }

  /* Proceed with read transparent  */
  if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_READ_TRANSPARENT,
                                   (void *)&read_params,
                                   req_msg_ptr) >= 0)
  {
    /* Free allocated memory for file path */
    QCRIL_UIM_FREE_IF_NOT_NULL(read_params.file_id.path.data_ptr);
    return;
  }
send_resp:
  QCRIL_LOG_DEBUG( "qcril_uim_get_mcc_mnc: ret_err=0x%x\n", ret_err);
  mcc_mnc_info = std::make_shared<qcril_uim_mcc_mnc_info_type>();
  if (mcc_mnc_info != NULL)
  {
    mcc_mnc_info->err_code = ret_err;
  }
  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, mcc_mnc_info);

  /* Free allocated memory for file path */
  QCRIL_UIM_FREE_IF_NOT_NULL(read_params.file_id.path.data_ptr);
}/* qcril_uim_request_get_mcc_mnc */


/*=========================================================================

  FUNCTION:  qcril_uim_get_mcc_mnc_resp_ad

===========================================================================*/
/*!
    @brief
    Process the response for reading EF-AD

    @return
    RIL_UIM_Errno
*/
/*=========================================================================*/
static RIL_UIM_Errno qcril_uim_get_mcc_mnc_resp_ad
(
   const qmi_uim_data_type                 *rsp_data_ptr,
   std::shared_ptr<UimGetMccMncRequestMsg>  req_ptr
)
{
  qmi_uim_read_transparent_params_type       read_params = {};
  RIL_UIM_Errno                              ret_err     = RIL_UIM_E_INTERNAL_ERR;

  /* Basic sanity check, should not happen, check anyway */
  if (NULL == req_ptr)
  {
    QCRIL_LOG_ERROR("NULL original_request_ptr");
    return ret_err;
  }

  /* Read EF IMSI */
  ret_err = qcril_uim_file_get_mcc_mnc_get_path_and_session_info(
                      req_ptr->get_aid().c_str(),
                      (qmi_uim_app_type)req_ptr->get_app_type(),
                      &read_params.file_id.path,
                      &read_params.session_info,
                      qmi_ril_get_sim_slot());

  if (RIL_UIM_E_SUCCESS == ret_err)
  {
    /* Check session type & update file_id */
    if((read_params.session_info.session_type == QMI_UIM_SESSION_TYPE_PRI_GW_PROV) ||
       (read_params.session_info.session_type == QMI_UIM_SESSION_TYPE_SEC_GW_PROV) ||
       (read_params.session_info.session_type == QMI_UIM_SESSION_TYPE_TER_GW_PROV))
    {
      int                                        res         = 0;
      std::shared_ptr<UimReadTransparentReqMsg>  req_msg_ptr = NULL;

      read_params.file_id.file_id = QCRIL_UIM_FILEID_EF_IMSI;

      req_msg_ptr = std::make_shared<UimReadTransparentReqMsg>(req_ptr, read_params);

      if (req_msg_ptr != NULL)
      {
        /* Proceed with read transparent  */

        /* Check length of EF AD */
        if (rsp_data_ptr)
        {
          QCRIL_LOG_DEBUG("data_ptr: 0x%x, data_len: 0x%x\n",
                          rsp_data_ptr->data_ptr,
                          rsp_data_ptr->data_len);
          if (rsp_data_ptr->data_len > 3  &&
              rsp_data_ptr->data_ptr      &&
              (2 == rsp_data_ptr->data_ptr[3] || 3 == rsp_data_ptr->data_ptr[3]))
          {
            req_msg_ptr->set_num_mnc_digits(rsp_data_ptr->data_ptr[3]);
          }
        }

        res = qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_READ_TRANSPARENT,
                                         (void *)&read_params,
                                         req_msg_ptr);
        if (res < 0)
        {
          QCRIL_LOG_ERROR("Error queueing READ_TRANSPARENT for IMSI");
          ret_err = RIL_UIM_E_MODEM_ERR;
        }
      }
      else
      {
        ret_err = RIL_UIM_E_NO_MEMORY;
      }
    }
    else
    {
      QCRIL_LOG_ERROR("Not proper session type 0x%x for UIM_GET_MCC_MNC",
                      read_params.session_info.session_type);
      ret_err = RIL_UIM_E_REQUEST_NOT_SUPPORTED;
    }
  }

  /* Free allocated memory for file path */
  QCRIL_UIM_FREE_IF_NOT_NULL(read_params.file_id.path.data_ptr);

  return ret_err;
}/* qcril_uim_get_mcc_mnc_resp_ad */


/*=========================================================================

  FUNCTION:  qcril_uim_get_mcc_mnc_resp_imsi

===========================================================================*/
/*!
    @brief
    Process the response for reading EF-IMSI

    @return
    RIL_UIM_Errno
*/
/*=========================================================================*/
static RIL_UIM_Errno qcril_uim_get_mcc_mnc_resp_imsi
(
   const qmi_uim_data_type                      *rsp_data_ptr,
   uint16_t                                      file_id,
   uint8_t                                       num_mnc_digits,
   std::shared_ptr<qcril_uim_mcc_mnc_info_type>  mcc_mnc_info_ptr
)
{
  uint16      mnc_3digit_mcc_list[] = {302, 310, 311, 312, 313, 314,
                                       315, 316, 334, 348, 405, 344,
                                       365, 722, 342, 350, 346, 732,
                                       366, 750, 352, 358, 354, 708,
                                       338, 356, 360, 374, 376, 1,
                                       2, 3, 4, 5, 6, 7, 8, 9, 10,
                                       11, 12};
  uint16      imsi_mcc              = 0;
  uint8       i                     = 0;
  uint8       imsi_num_mnc_digits   = 2;

  /* Basic sanity check, should not happen, check anyway */
  if (NULL == rsp_data_ptr ||
      NULL == mcc_mnc_info_ptr)
  {
    QCRIL_LOG_ERROR("NULL pointer, rsp_data_ptr=0x%x, mcc_mnc_info_ptr=0x%x",
                    rsp_data_ptr, mcc_mnc_info_ptr.get());
    return RIL_UIM_E_INTERNAL_ERR;
  }

  /* Make sure MCC and MNC are NULL terminated */
  memset(mcc_mnc_info_ptr->mcc, 0x00, sizeof(mcc_mnc_info_ptr->mcc));
  memset(mcc_mnc_info_ptr->mnc, 0x00, sizeof(mcc_mnc_info_ptr->mnc));

  if (file_id == QCRIL_UIM_FILEID_EF_IMSI)
  {
    QCRIL_LOG_DEBUG("rsp_data_ptr->data_ptr=0x%x, rsp_data_ptr->data_len=0x%x\n",
                    rsp_data_ptr->data_ptr, rsp_data_ptr->data_len);
    if (NULL == rsp_data_ptr->data_ptr ||
        rsp_data_ptr->data_len    != 9 ||
        rsp_data_ptr->data_ptr[0] < 4)
    {
      return RIL_UIM_E_INTERNAL_ERR;
    }

    /* Extract MCC */
    imsi_mcc  = (uint16)((rsp_data_ptr->data_ptr[1] >> 4) & 0x0F) * 100;
    imsi_mcc += (uint16)((rsp_data_ptr->data_ptr[2]) & 0x0F) * 10;
    imsi_mcc += (uint16)((rsp_data_ptr->data_ptr[2] >> 4) & 0x0F);

    mcc_mnc_info_ptr->mcc[0] = qcril_uim_bin_to_hexchar((rsp_data_ptr->data_ptr[1] >> 4) & 0x0F);
    mcc_mnc_info_ptr->mcc[1] = qcril_uim_bin_to_hexchar(rsp_data_ptr->data_ptr[2] & 0x0F);
    mcc_mnc_info_ptr->mcc[2] = qcril_uim_bin_to_hexchar((rsp_data_ptr->data_ptr[2] >> 4) & 0x0F);

    /* The number of digits of MNC depends on the 4th byte of EF-AD.
       hardcoded table is used in case EF-AD has only 3 bytes.*/
    if (num_mnc_digits < 2 ||
        num_mnc_digits > 3)
    {
      for (i = 0; i < sizeof(mnc_3digit_mcc_list) / sizeof(mnc_3digit_mcc_list[0]); i++)
      {
        if (imsi_mcc == mnc_3digit_mcc_list[i])
        {
          imsi_num_mnc_digits = 3;
          break;
        }
      }
    }
    else
    {
      imsi_num_mnc_digits = num_mnc_digits;
    }

    /* Extract MNC */
    mcc_mnc_info_ptr->mnc[0] = qcril_uim_bin_to_hexchar(rsp_data_ptr->data_ptr[3] & 0x0F);
    mcc_mnc_info_ptr->mnc[1] = qcril_uim_bin_to_hexchar((rsp_data_ptr->data_ptr[3] >> 4) & 0x0F);
    if (imsi_num_mnc_digits == 3)
    {
      mcc_mnc_info_ptr->mnc[2] = qcril_uim_bin_to_hexchar(rsp_data_ptr->data_ptr[4] & 0x0F);
    }
  }
  else if (file_id == QCRIL_UIM_FILEID_EF_IMSI_M)
  {
    uint16          mcc_in_bcd        = 0;
    uint8           mnc_in_bcd        = 0;
    const char      imsi_m_to_char[]  =
                      { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };

    if (NULL == rsp_data_ptr->data_ptr ||
        rsp_data_ptr->data_len != QCRIL_UIM_IMSI_M_RAW_SIZE)
    {
      return RIL_UIM_E_INTERNAL_ERR;
    }

    mcc_in_bcd = (uint16)(((rsp_data_ptr->data_ptr[9] & 0x03) << 8) +
                          rsp_data_ptr->data_ptr[8]);
    mnc_in_bcd = (rsp_data_ptr->data_ptr[6] & 0x7F);

    if (mcc_in_bcd > 999 || mnc_in_bcd > 99)
    {
      return RIL_UIM_E_INTERNAL_ERR;
    }

    /* MCC */
    mcc_mnc_info_ptr->mcc[0] = imsi_m_to_char[mcc_in_bcd / 100];
    mcc_in_bcd %= 100;
    mcc_mnc_info_ptr->mcc[1] = imsi_m_to_char[mcc_in_bcd / 10];
    mcc_mnc_info_ptr->mcc[2] = imsi_m_to_char[mcc_in_bcd % 10];

    /* MNC */
    mcc_mnc_info_ptr->mnc[0] = imsi_m_to_char[mnc_in_bcd / 10];
    mcc_mnc_info_ptr->mnc[1] = imsi_m_to_char[mnc_in_bcd % 10];
  }
  else
  {
    return RIL_UIM_E_INTERNAL_ERR;
  }

  QCRIL_LOG_DEBUG("mcc[%s], mnc[%s]\n",
                  mcc_mnc_info_ptr->mcc,
                  mcc_mnc_info_ptr->mnc);

  return RIL_UIM_E_SUCCESS;
}/* qcril_uim_get_mcc_mnc_resp_imsi */


/*=========================================================================

  FUNCTION:  qcril_uim_get_mcc_mnc_resp

===========================================================================*/
/*!
    @brief
    Processes the response for QCRIL_EVT_INTERNAL_UIM_GET_MCC_MNC

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_get_mcc_mnc_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<UimReadTransparentReqMsg> msg_ptr
)
{
  RIL_UIM_Errno                                 ret_err        = RIL_UIM_E_INTERNAL_ERR;
  const qmi_uim_data_type                      *rsp_data_ptr   = NULL;
  uint16_t                                      file_id        = 0;
  std::shared_ptr<qcril_uim_mcc_mnc_info_type>  resp_ptr       = std::make_shared<qcril_uim_mcc_mnc_info_type>();

  if((NULL == params_ptr) || (NULL == msg_ptr) || (NULL == resp_ptr))
  {
    QCRIL_LOG_ERROR("NULL params_ptr");
    QCRIL_ASSERT(0);
    return;
  }

  auto req_ptr = std::static_pointer_cast<UimGetMccMncRequestMsg>(msg_ptr->get_request_msg());
  ret_err = qcril_uim_convert_err_value(params_ptr->qmi_rsp_data.qmi_err_code);
  file_id = msg_ptr->get_file_id();

  if (req_ptr == NULL)
  {
    return;
  }

  QCRIL_LOG_DEBUG("qcril_uim_get_mcc_mnc_resp: qmi_err_code=%d, file_id=0x%x\n",
                  params_ptr->qmi_rsp_data.qmi_err_code,
                  file_id);

  /* Even if reading EF AD fails, still read EF IMSI */
  if (QCRIL_UIM_FILEID_EF_AD == file_id)
  {
    if (RIL_UIM_E_SUCCESS == ret_err)
    {
      rsp_data_ptr = &(params_ptr->qmi_rsp_data.rsp_data.read_transparent_rsp.content);
    }
    else
    {
      rsp_data_ptr = NULL;
    }

    ret_err = qcril_uim_get_mcc_mnc_resp_ad(rsp_data_ptr, req_ptr);
    if (RIL_UIM_E_SUCCESS == ret_err)
    {
      /* In the successful case, original request is used in READ_TRANSPARENT
         request for reading IMSI, no need to free */
      return;
    }
  }/* EF-AD */
  else if ((RIL_UIM_E_SUCCESS          == ret_err) &&
           (QCRIL_UIM_FILEID_EF_IMSI   == file_id ||
            QCRIL_UIM_FILEID_EF_IMSI_M == file_id))
  {
    rsp_data_ptr = &(params_ptr->qmi_rsp_data.rsp_data.read_transparent_rsp.content);
    ret_err = qcril_uim_get_mcc_mnc_resp_imsi(rsp_data_ptr,
                                              file_id,
                                              msg_ptr->get_num_mnc_digits(),
                                              resp_ptr);
  }/* EF-IMSI */
  else
  {
    ret_err = RIL_UIM_E_INTERNAL_ERR;
  }

  QCRIL_LOG_DEBUG("ret_err=0x%x\n", ret_err);

  /* Generate QCRIL_EVT_UIM_MCC_MNC_INFO */
  resp_ptr->err_code = ret_err;

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, resp_ptr);
}/* qcril_uim_get_mcc_mnc_resp */


/*=========================================================================

  FUNCTION:  qcril_uim_get_terminal_capability_resp

===========================================================================*/
/*!
    @brief
    Processes the response for get terminal capability command.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_get_terminal_capability_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<UimGetLPATermialCapabilitesRequestMsg> req_ptr
)
{
  RIL_UIM_Errno  ril_err = RIL_UIM_E_SUCCESS;
  auto           rsp_ptr = std::make_shared<RIL_UIM_TerminalCapabilityLPATLV>();

  /* Sanity checks */
  if (params_ptr == NULL || req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  ril_err = qcril_uim_convert_err_value(params_ptr->qmi_rsp_data.qmi_err_code);

  for (uint8_t i = 0; i < params_ptr->qmi_rsp_data.rsp_data.terminal_capability_rsp.num_tlvs && i < QMI_UIM_TERMINAL_CAPABILITY_TLVS_MAX; i++)
  {
    if (params_ptr->qmi_rsp_data.rsp_data.terminal_capability_rsp.tlvs[i].tag == QMI_UIM_TERMINAL_CAPABILITY_LPA_TLV_TAG &&
        params_ptr->qmi_rsp_data.rsp_data.terminal_capability_rsp.tlvs[i].value.data_len >= QMI_UIM_TERMINAL_CAPABILITY_LPA_TLV_LEN &&
        params_ptr->qmi_rsp_data.rsp_data.terminal_capability_rsp.tlvs[i].value.data_ptr)
    {
      /* LPA TLV should be one byte long per the spec. So, we try copying
         only first byte of value over. That means that if due to some
         wrong reason the value_len of this TLV, is reported as more than
         one byte by QMIUIM, we simply copy the first byte over, without
         flagging this as an error to QCRIL user. The reason is that QMIUIM
         is always expected to ensure LPA TLV's value is always one byte
         long */
      if (rsp_ptr != NULL)
      {
        rsp_ptr->value = params_ptr->qmi_rsp_data.rsp_data.terminal_capability_rsp.tlvs[i].value.data_ptr[0];
        rsp_ptr->tlv_present = true;
        rsp_ptr->err = ril_err;
      }
      break;
    }
  }

  QCRIL_LOG_DEBUG( "qcril_uim_get_terminal_capability_resp: qmi_err_code=%d \n",
                    params_ptr->qmi_rsp_data.qmi_err_code );

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_get_terminal_capability_resp */


/*===========================================================================*/
/*!
    @brief
    Processes the response for set terminal capability command.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_set_terminal_capability_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<UimSetLPATermialCapabilitesRequestMsg> req_ptr
)
{
  RIL_UIM_Errno                   ril_err = RIL_UIM_E_SUCCESS;
  std::shared_ptr<RIL_UIM_Errno>  rsp_ptr = NULL;

  /* Sanity checks */
  if (params_ptr == NULL || req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  ril_err = qcril_uim_convert_err_value(params_ptr->qmi_rsp_data.qmi_err_code);

  QCRIL_LOG_DEBUG( "qcril_uim_set_terminal_capability_resp: qmi_err_code=%d \n",
                    params_ptr->qmi_rsp_data.qmi_err_code );

  rsp_ptr = std::make_shared<RIL_UIM_Errno>(ril_err);

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_set_terminal_capability_resp */


/*===========================================================================

                               QCRIL INTERFACE FUNCTIONS

===========================================================================*/

/*=========================================================================

  FUNCTION:  qcril_uim_request_sim_io

===========================================================================*/
/*!
    @brief
    Handler for RIL_REQUEST_SIM_IO.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_sim_io
(
  const std::shared_ptr<UimSIMIORequestMsg> req_ptr
)
{
  const RIL_UIM_SIM_IO  *io_ptr  = NULL;
  auto                   rsp_ptr = std::make_shared<RIL_UIM_SIM_IO_Response>();

  if(req_ptr == NULL || req_ptr->get_sim_io_data() == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot process response");
    QCRIL_ASSERT(0);
    return;
  }

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  io_ptr = req_ptr->get_sim_io_data();

  QCRIL_LOG_INFO( "qcril_uim_request_sim_io(aid: %s)\n", io_ptr->aidPtr.c_str());

  QCRIL_LOG_INFO( "qcril_uim_request_sim_io(%d, 0x%X, %s, %d, %d, %d, %s, %s)\n",
                  io_ptr->command,
                  io_ptr->fileid,
                  io_ptr->path.c_str(),
                  io_ptr->p1,
                  io_ptr->p2,
                  io_ptr->p3,
                  io_ptr->data.c_str(),
                  io_ptr->pin2.c_str());

  switch (io_ptr->command)
  {
     case SIM_CMD_READ_BINARY:
      qcril_uim_read_binary(req_ptr);
      break;

    case SIM_CMD_READ_RECORD:
      qcril_uim_read_record(req_ptr);
      break;

    case SIM_CMD_GET_RESPONSE:
      qcril_uim_get_response(req_ptr);
      break;

    case SIM_CMD_UPDATE_BINARY:
      qcril_uim_update_binary(req_ptr);
      break;

    case SIM_CMD_UPDATE_RECORD:
      qcril_uim_update_record(req_ptr);
      break;

    case SIM_CMD_STATUS:
      qcril_uim_get_status(req_ptr);
      break;

    case SIM_CMD_SET_DATA:
    case SIM_CMD_RETRIEVE_DATA:
    default:
      QCRIL_LOG_ERROR( "ILLEGAL: qcril_uim_request_sim_io unknown cmd\n");
      if (rsp_ptr != NULL)
      {
        rsp_ptr->slot_id = req_ptr->get_slot();
        rsp_ptr->err = RIL_UIM_E_REQUEST_NOT_SUPPORTED;
      }
      req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
      break;
  }
} /* qcril_uim_request_sim_io() */


/*=========================================================================

  FUNCTION:  qcril_uim_get_imsi_path_for_aid

===========================================================================*/
/*!
    @brief
    Gets Imsi path and session type for given aid.

    @return
    RIL_UIM_Errno
*/
/*=========================================================================*/
static RIL_UIM_Errno qcril_uim_get_imsi_path_for_aid
(
  qmi_uim_read_transparent_params_type * read_params_ptr,
  const char                           * aid_ptr,
  uint8                                  slot
)
{
  uint8                                 path[3][4] =
                                        {
                                          {0x3F, 0x00, 0x7F, 0x20}, /* DF GSM        */
                                          {0x3F, 0x00, 0x7F, 0x25}, /* DF CDMA       */
                                          {0x3F, 0x00, 0x7F, 0xFF}  /* ADF USIM/CSIM */
                                        };
  uint16                                first_level_df_path = 0;
  RIL_UIM_Errno                         err                 = RIL_UIM_E_SUCCESS;

  if (read_params_ptr == NULL)
  {
    return RIL_UIM_E_INVALID_ARGUMENTS;
  }

  read_params_ptr->file_id.path.data_ptr = (unsigned char*)qcril_malloc(sizeof(uint8) * 4);

  if (read_params_ptr->file_id.path.data_ptr == NULL)
  {
    return RIL_UIM_E_NO_MEMORY;
  }

  /* Update the file path based on passed aid pointer */
  if ((aid_ptr == NULL) || (strlen(aid_ptr) == 0))
  {
    /* If SIM App is present we always fetch IMSI from DF GSM */
    if (qcril_uim_find_app_in_slot(slot, QMI_UIM_APP_SIM))
    {
      memcpy(read_params_ptr->file_id.path.data_ptr, path[0], 4);
      first_level_df_path = QCRIL_UIM_FILEID_DF_GSM;
    }
    /* if RUIM app is the only one present (& no SIM App), we fetch IMSI_M from DF CDMA */
    else if (qcril_uim_find_app_in_slot(slot, QMI_UIM_APP_RUIM))
    {
      memcpy(read_params_ptr->file_id.path.data_ptr, path[1], 4);
      first_level_df_path = QCRIL_UIM_FILEID_DF_CDMA;
    }
  }
  else
  {
    memcpy(read_params_ptr->file_id.path.data_ptr, path[2], 4);
    first_level_df_path = QCRIL_UIM_FILEID_ADF_USIM_CSIM;
  }

  /* Extract session information */
  err = qcril_uim_extract_session_type(slot,
                                       aid_ptr,
                                       first_level_df_path,
                                       &read_params_ptr->session_info,
                                       NULL,
                                       0);
  if (err != RIL_UIM_E_SUCCESS)
  {
    QCRIL_LOG_ERROR("error processing input params in RIL_REQUEST_GET_IMSI");
    QCRIL_UIM_FREE_IF_NOT_NULL(read_params_ptr->file_id.path.data_ptr);
    return err;
  }

  QCRIL_LOG_INFO( "session_type for RIL_REQUEST_GET_IMSI: %d\n",
                  read_params_ptr->session_info.session_type );

  /* Check session type & update file_id */
  if((read_params_ptr->session_info.session_type == QMI_UIM_SESSION_TYPE_PRI_GW_PROV) ||
     (read_params_ptr->session_info.session_type == QMI_UIM_SESSION_TYPE_SEC_GW_PROV) ||
     (read_params_ptr->session_info.session_type == QMI_UIM_SESSION_TYPE_TER_GW_PROV))
  {
    read_params_ptr->file_id.file_id = QCRIL_UIM_FILEID_EF_IMSI;
  }
  else if ((read_params_ptr->session_info.session_type == QMI_UIM_SESSION_TYPE_PRI_1X_PROV) ||
           (read_params_ptr->session_info.session_type == QMI_UIM_SESSION_TYPE_SEC_1X_PROV) ||
           (read_params_ptr->session_info.session_type == QMI_UIM_SESSION_TYPE_TER_1X_PROV))
  {
    read_params_ptr->file_id.file_id = QCRIL_UIM_FILEID_EF_IMSI_M;
  }
  else
  {
    QCRIL_LOG_ERROR("Not proper session type for RIL_REQUEST_GET_IMSI");
    QCRIL_UIM_FREE_IF_NOT_NULL(read_params_ptr->file_id.path.data_ptr);
    return RIL_UIM_E_INTERNAL_ERR;
  }

  /* Read parameters: length and offset */
  read_params_ptr->file_id.path.data_len = QCRIL_UIM_IMSI_PATH_SIZE;
  read_params_ptr->offset = 0;
  read_params_ptr->length = 0;

  return RIL_UIM_E_SUCCESS;
} /* qcril_uim_get_imsi_path_for_aid */


/*===========================================================================

  FUNCTION:  qcril_uim_request_get_imsi

===========================================================================*/
/*!
    @brief
    Handler for RIL_REQUEST_GET_IMSI.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_get_imsi
(
  const std::shared_ptr<UimGetImsiRequestMsg> req_ptr
)
{
  qmi_uim_read_transparent_params_type       read_params = {};
  RIL_UIM_Errno                              err         = RIL_UIM_E_SUCCESS;
  std::shared_ptr<UimReadTransparentReqMsg>  req_msg_ptr = NULL;
  std::shared_ptr<RIL_UIM_IMSI_Response>     resp_ptr    = NULL;

  if(req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot process response");
    QCRIL_ASSERT(0);
    return;
  }

  memset(&read_params, 0, sizeof(qmi_uim_read_transparent_params_type));

  QCRIL_LOG_INFO( "qcril_uim_request_get_imsi(%s)\n", req_ptr->get_aid().c_str() );

  err = qcril_uim_get_imsi_path_for_aid(&read_params, req_ptr->get_aid().c_str(), req_ptr->get_slot());
  if (err != RIL_UIM_E_SUCCESS)
  {
    goto send_imsi_error;
  }

  req_msg_ptr = std::make_shared<UimReadTransparentReqMsg>(req_ptr,
                                                           read_params);

  if (req_msg_ptr == NULL)
  {
    err = RIL_UIM_E_NO_MEMORY;
    goto send_imsi_error;
  }

  /* Proceed with read transparent  */
  if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_READ_TRANSPARENT,
                                   (void *)&read_params,
                                   req_msg_ptr) >= 0)
  {
    QCRIL_UIM_FREE_IF_NOT_NULL(read_params.file_id.path.data_ptr);
    return;
  }
  err = RIL_UIM_E_MODEM_ERR;

send_imsi_error:
  resp_ptr = std::make_shared<RIL_UIM_IMSI_Response>();

  if (resp_ptr != NULL)
  {
    resp_ptr->err = err;
  }

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, resp_ptr);

  QCRIL_UIM_FREE_IF_NOT_NULL(read_params.file_id.path.data_ptr)
} /* qcril_uim_request_get_imsi */


/*===========================================================================

  FUNCTION:  qcril_uim_request_isim_authenticate

===========================================================================*/
/*!
    @brief
    Handler for RIL_REQUEST_ISIM_AUTHENTICATION.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_isim_authenticate
(
  const std::shared_ptr<UimISIMAuthenticationRequestMsg> req_ptr
)
{
  uint8                                 slot                 = qmi_ril_get_process_instance_id();
  uint8                                 isim_index           = 0;
  qmi_uim_authenticate_params_type      auth_params          = {};
  RIL_UIM_Errno                         ret_err              = RIL_UIM_E_INTERNAL_ERR;
  std::shared_ptr<RIL_UIM_SIM_IO_Response> rsp_ptr           = NULL;

  if(req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot process response");
    QCRIL_ASSERT(0);
    return;
  }

  QCRIL_LOG_INFO( "qcril_uim_request_isim_authenticate()\n");


  /* Current RIL interface doesnt provide the ISIM AID, so this is what we do:
     Based on the RIL instance, we check if ISIM App is present in that slot
     and use a non-prov session type of that slot */
  switch(slot)
  {
    case 0:
      auth_params.session_info.session_type = QMI_UIM_SESSION_TYPE_NON_PROV_SLOT_1;
      break;

    case 1:
      auth_params.session_info.session_type = QMI_UIM_SESSION_TYPE_NON_PROV_SLOT_2;
      break;

    default:
      QCRIL_LOG_ERROR("Invalid qmi_uim slot id: 0x%X", slot);
      ret_err = RIL_UIM_E_INTERNAL_ERR;
      goto isim_auth_error;
  }

  if ((ret_err = qcril_uim_extract_isim_index(&isim_index, slot)) != RIL_UIM_E_SUCCESS)
  {
    QCRIL_LOG_ERROR( "ISIM app not found for slot: 0x%x", slot);
    goto isim_auth_error;
  }

  /* Update ISIM AID */
  auth_params.session_info.aid.data_len =
    qcril_uim.card_status.card[slot].application[isim_index].aid_len;
  auth_params.session_info.aid.data_ptr =
    (unsigned char *)qcril_uim.card_status.card[slot].application[isim_index].aid_value;

  /* Update auth parameters */
  auth_params.auth_context       = QMI_UIM_AUTH_CONTEXT_IMS_AKA_SECURITY;
  auth_params.auth_data.data_ptr = qcril_uim_alloc_base64string_to_bin(
                                     req_ptr->get_ISIM_data().c_str(),
                                     &auth_params.auth_data.data_len);

  if (auth_params.auth_data.data_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Unable to convert input ISIM auth data!");
    ret_err = RIL_UIM_E_NO_MEMORY;
    goto isim_auth_error;
  }

  /* Proceed with logical channel request */
  if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_AUTHENTICATE,
                                   (void *)&auth_params,
                                   req_ptr) >= 0)
  {
    /* For success case, free AID & data buffer & return */
    qcril_free(auth_params.auth_data.data_ptr);
    auth_params.auth_data.data_ptr = NULL;
    return;
  }
  ret_err = RIL_UIM_E_MODEM_ERR;

isim_auth_error:
  rsp_ptr = std::make_shared<RIL_UIM_SIM_IO_Response>();

  if (rsp_ptr != NULL)
  {
    rsp_ptr->err = ret_err;
  }

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
  /* Free data buffer that was allocated */
  if (auth_params.auth_data.data_ptr)
  {
    qcril_free(auth_params.auth_data.data_ptr);
    auth_params.auth_data.data_ptr = NULL;
  }
} /* qcril_uim_request_isim_authenticate */


/*===========================================================================

  FUNCTION:  qcril_uim_request_sim_authenticate

===========================================================================*/
/*!
    @brief
    Handler for RIL_REQUEST_SIM_AUTHENTICATION.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_sim_authenticate
(
  const std::shared_ptr<UimSIMAuthenticationRequestMsg> req_ptr
)
{
  uint8                                 slot                 = qmi_ril_get_process_instance_id();
  qmi_uim_authenticate_params_type      auth_params          = {};
  uint8                                 computed_aid[QMI_UIM_MAX_AID_LEN] = {};
  RIL_UIM_Errno                         ret_err              = RIL_UIM_E_INTERNAL_ERR;
  std::shared_ptr<RIL_UIM_SIM_IO_Response> rsp_ptr           = NULL;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if(req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot process response");
    QCRIL_ASSERT(0);
    return;
  }

  if (req_ptr->get_aid().length() != 0)
  {
    /* update session_type */
    if ((ret_err = qcril_uim_extract_session_type(slot,
                                       req_ptr->get_aid().c_str(),
                                       QCRIL_UIM_FILEID_ADF_USIM_CSIM,
                                       &auth_params.session_info,
                                       computed_aid,
                                       QMI_UIM_MAX_AID_LEN))
        != RIL_UIM_E_SUCCESS)
    {
      QCRIL_LOG_ERROR( "app not found for slot: 0x%x", slot);
      goto sim_auth_error;
    }

    /* Update auth parameters */
    if ((auth_params.session_info.session_type == QMI_UIM_SESSION_TYPE_NON_PROV_SLOT_1 ||
         auth_params.session_info.session_type == QMI_UIM_SESSION_TYPE_NON_PROV_SLOT_2 ||
         auth_params.session_info.session_type == QMI_UIM_SESSION_TYPE_NON_PROV_SLOT_3 ) &&
        (qcril_uim_check_aid_with_app_type((const qmi_uim_data_type *)&auth_params.session_info.aid,
                                           QMI_UIM_APP_ISIM)))
    {
      if (req_ptr->get_auth_context() == QCRIL_UIM_AUTH_IMS_AKA)
      {
        auth_params.auth_context = QMI_UIM_AUTH_CONTEXT_IMS_AKA_SECURITY;
      }
      else if (req_ptr->get_auth_context() == QCRIL_UIM_AUTH_HTTP_DIGEST_SECURITY_CONTEXT)
      {
        auth_params.auth_context = QMI_UIM_AUTH_CONTEXT_HTTP_DIGEST_SECURITY;
      }
      else
      {
        QCRIL_LOG_ERROR("security context not supported for ISIM: 0x%x", req_ptr->get_auth_context());
        ret_err = RIL_UIM_E_INVALID_ARGUMENTS;
        goto sim_auth_error;
      }
    }
    else if (auth_params.session_info.session_type == QMI_UIM_SESSION_TYPE_PRI_GW_PROV ||
             auth_params.session_info.session_type == QMI_UIM_SESSION_TYPE_SEC_GW_PROV ||
             auth_params.session_info.session_type == QMI_UIM_SESSION_TYPE_TER_GW_PROV ||
             (qcril_uim_check_aid_with_app_type((const qmi_uim_data_type *)&auth_params.session_info.aid,
                                                QMI_UIM_APP_USIM)))
    {
      if (req_ptr->get_auth_context() == QCRIL_UIM_AUTH_GSM_CONTEXT)
      {
        auth_params.auth_context = QMI_UIM_AUTH_CONTEXT_GSM_SECURITY;
      }
      else if (req_ptr->get_auth_context() == QCRIL_UIM_AUTH_3G_CONTEXT)
      {
        auth_params.auth_context = QMI_UIM_AUTH_CONTEXT_3G_SECURITY;
      }
      else if (req_ptr->get_auth_context() == QCRIL_UIM_AUTH_VGCS_VBS_SECURITY_CONTEXT)
      {
        auth_params.auth_context = QMI_UIM_AUTH_CONTEXT_VGCS_VBS_SECURITY;
      }
      else
      {
        QCRIL_LOG_ERROR("security context not supported for USIM: 0x%x", req_ptr->get_auth_context());
        ret_err = RIL_UIM_E_INVALID_ARGUMENTS;
        goto sim_auth_error;
      }
    }
    else /* for CSIM and invalid AID */
    {
      QCRIL_LOG_ERROR("authentication not supported for AID: %s at this point", req_ptr->get_aid().c_str());
      ret_err = RIL_UIM_E_INVALID_ARGUMENTS;
      goto sim_auth_error;
    }
  }
  else /* in_ptr->aid == NULL or AID size is 0 then go with 2g card */
  {
    if(qcril_uim_find_app_in_slot(slot, QMI_UIM_APP_SIM))
    {
      /* update session_type */
      ret_err = qcril_uim_extract_session_type(slot,
                                               NULL,
                                               QCRIL_UIM_FILEID_DF_GSM,
                                               &auth_params.session_info,
                                               NULL,
                                               0);
      if(ret_err !=  RIL_UIM_E_SUCCESS)
      {
        QCRIL_LOG_ERROR("Unable to map session with error: 0x%x",
                        ret_err);
        goto sim_auth_error;
      }

      if (auth_params.session_info.session_type != QMI_UIM_SESSION_TYPE_PRI_GW_PROV &&
          auth_params.session_info.session_type != QMI_UIM_SESSION_TYPE_SEC_GW_PROV &&
          auth_params.session_info.session_type != QMI_UIM_SESSION_TYPE_TER_GW_PROV)
      {
        QCRIL_LOG_ERROR("provisioning session not available for run GSM ALGOR: 0x%x",
                        auth_params.session_info.session_type);
        ret_err = RIL_UIM_E_INVALID_ARGUMENTS;
        goto sim_auth_error;
      }

      /* Check for QCRIL_UIM_AUTH_GSM_CONTEXT also, android telephony didn't have
         support for authContext 0 for 2G cards */
      if (req_ptr->get_auth_context() == 0 ||
          req_ptr->get_auth_context() == QCRIL_UIM_AUTH_GSM_CONTEXT)
      {
        auth_params.auth_context = QMI_UIM_AUTH_CONTEXT_RUN_GSM_ALGO;
      }
      else
      {
        QCRIL_LOG_ERROR("Invalid authContext for run GSM ALGOR: 0x%x",
                        req_ptr->get_auth_context());
        ret_err = RIL_UIM_E_INVALID_ARGUMENTS;
        goto sim_auth_error;
      }
    }
    else if (qcril_uim_find_app_in_slot(slot, QMI_UIM_APP_RUIM))
    {
      /* update session_type */
      ret_err = qcril_uim_extract_session_type(slot,
                                               NULL,
                                               QCRIL_UIM_FILEID_DF_CDMA,
                                               &auth_params.session_info,
                                               NULL,
                                               0);
      if(ret_err !=  RIL_UIM_E_SUCCESS)
      {
        QCRIL_LOG_ERROR("Unable to map session with error: 0x%x",
                        ret_err);
        goto sim_auth_error;
      }

      if (auth_params.session_info.session_type != QMI_UIM_SESSION_TYPE_PRI_1X_PROV &&
          auth_params.session_info.session_type != QMI_UIM_SESSION_TYPE_SEC_1X_PROV &&
          auth_params.session_info.session_type != QMI_UIM_SESSION_TYPE_TER_1X_PROV)
      {
        QCRIL_LOG_ERROR("provisioning session not available for run CAVE: 0x%x",
                        auth_params.session_info.session_type);
        ret_err = RIL_UIM_E_INVALID_ARGUMENTS;
        goto sim_auth_error;
      }

      if (req_ptr->get_auth_context() == 0)
      {
        auth_params.auth_context = QMI_UIM_AUTH_CONTEXT_RUN_CAVE_ALGO;
      }
      else
      {
        QCRIL_LOG_ERROR("Invalid authContext for run CAVE ALGO: 0x%x",
                        req_ptr->get_auth_context());
        ret_err = RIL_UIM_E_INTERNAL_ERR;
        goto sim_auth_error;
      }
    }
  }

  auth_params.auth_data.data_ptr = qcril_uim_alloc_base64string_to_bin(
                                     req_ptr->get_auth_data().c_str(),
                                     &auth_params.auth_data.data_len);

  if (auth_params.auth_data.data_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Unable to convert input SIM auth data!");
    ret_err = RIL_UIM_E_NO_MEMORY;
    goto sim_auth_error;
  }

  /* Proceed with logical channel request */
  if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_AUTHENTICATE,
                                   (void *)&auth_params,
                                   req_ptr) >= 0)
  {
    /* For success case, free AID & data buffer & return */

    qcril_free(auth_params.auth_data.data_ptr);
    auth_params.auth_data.data_ptr = NULL;
    return;
  }
  ret_err = RIL_UIM_E_MODEM_ERR;

sim_auth_error:
  rsp_ptr = std::make_shared<RIL_UIM_SIM_IO_Response>();

  if (rsp_ptr != NULL)
  {
    rsp_ptr->err = ret_err;
  }

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);

  /* Free data buffer that was allocated */
  if (auth_params.auth_data.data_ptr)
  {
    qcril_free(auth_params.auth_data.data_ptr);
    auth_params.auth_data.data_ptr = NULL;
  }
} /* qcril_uim_request_sim_authenticate */


/*===========================================================================

  FUNCTION:  qcril_uim_aid_log

===========================================================================*/
/*!
    @brief
    Responsible to print the aid in the adb log and QXDM logs

    @params
    aid_ptr       : input param
    print_aid_ptr : output param
    print_aid_len : input param, output param one extra byte for null
                    termination

    @return
    RIL_UIM_Errno
*/
/*=========================================================================*/
static RIL_UIM_Errno qcril_uim_aid_log
(
  const char *aid_ptr,
  char       *print_aid_ptr,
  uint8       print_aid_len
)
{
  if (aid_ptr == NULL ||
      print_aid_ptr == NULL ||
      print_aid_len == 0)
  {
    return RIL_UIM_E_INVALID_ARGUMENTS;
  }

  /* Print only when adb log is not on, AID should not get logged in adb logs */
  if (!qcril_uim_log_adb_on)
  {
    (void)strlcpy(print_aid_ptr,
                  aid_ptr,
                  print_aid_len);
  }
  else
  {
    uint8   aid_len    = 0;
    uint8   i          = 0;

    /* In case adb log, print only first two bytes and other bytest as * if aid bytes less than 5
       print first three and last 2 bytes and other as * if greater than 5 */
    aid_len = strlen(aid_ptr);
    if (aid_len > 4)
    {
      /* Copy first 2 bytes of aid_ptr */
      for (i = 0; i < 4 && i < (print_aid_len - 1); i++)
      {
        print_aid_ptr[i] = aid_ptr[i];
      }
      /* length greater than 5 bytes which is 10 charecters */
      if (aid_len > 10)
      {
        /* copy one more byte of aid_ptr */
        for (; i < 6 && i < (print_aid_len - 1); i++)
        {
          print_aid_ptr[i] = aid_ptr[i];
        }
        /* Remaining bytes as * except last two bytes */
        for (; i < (aid_len - 4) && i < (print_aid_len - 1); i++)
        {
          print_aid_ptr[i] = '*';
        }
        for (; i < aid_len && i < (print_aid_len - 1); i++)
        {
          print_aid_ptr[i] = aid_ptr[i];
        }
      } /* if (aid_len > 10) */
    } /* if (aid_len > 4) */
    /* Remaining bytes print as *: this is required in case aid_len <= 4
       (all bytes are printed with '*' or in case aid_len <= 10
       (only first 3 bytes are printed in clear) */
    for (; i < aid_len && i < (print_aid_len - 1); i++)
    {
      print_aid_ptr[i] = '*';
    }
  } /* else of (!qcril_uim_log_adb_on) */
  return RIL_UIM_E_SUCCESS;
} /* qcril_uim_aid_log */


/*===========================================================================

  FUNCTION:  qcril_uim_open_logical_ch_req

===========================================================================*/
/*!
    @brief
    Responsible to send QMI Open channel request to the modem. Note that
    we are using the QMI_UIM_OPEN_LOGICAL_CHANNEL command now since we have
    to support the opening of logical channel without specifying the AID TLV.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_open_logical_ch_req
(
  const std::shared_ptr<UimSIMOpenChannelRequestMsg> req_ptr
)
{
  qmi_uim_open_logical_channel_params_type open_logical_ch_params             = {};
  uint8                                    aid_buffer[QMI_UIM_MAX_AID_LEN]    = {};
  qcril_uim_fci_value_type                 qcril_fci_value                    = QCRIL_UIM_FCI_VALUE_FCP;
  uint8                                    ara_m_aid[]                        = {0xA0, 0x00, 0x00, 0x01, 0x51,
                                                                                 0x41, 0x43, 0x4C, 0x00};
  uint8                                    slot_id                            = qmi_ril_get_process_instance_id();
  uint16                                   aid_size                           = 0;
  RIL_UIM_Errno                            ret                                = RIL_UIM_E_INTERNAL_ERR;
  char                                     print_aid[QMI_UIM_MAX_AID_LEN + 1] = {'\0'};
  std::shared_ptr<RIL_UIM_OpenChannelResponse>  rsp_ptr                       = NULL;

  if (req_ptr == NULL)
  {
    return;
  }

  QCRIL_LOG_INFO( "qcril_uim_request_logical_channel(aid: %s)\n",
       (qcril_uim_aid_log(req_ptr->get_aid().c_str(), print_aid, sizeof(print_aid)) != RIL_UIM_E_SUCCESS) ? "NULL" : print_aid);

  open_logical_ch_params.slot = (qmi_uim_slot_type)((uint8_t)QMI_UIM_SLOT_1 + slot_id);

  /* If AID is provided, add the AID info to be sent to modem.
     If AID pointer is NULL or empty string, skip the AID TLV since we need to
     open the channel to MF with no select on any DF. */
  do
  {
    if (!req_ptr->get_aid().empty())
    {
      /* Convert AID string into binary */
      aid_size = qcril_uim_hexstring_to_bin(req_ptr->get_aid().c_str(),
                                            aid_buffer,
                                            QMI_UIM_MAX_AID_LEN);
      if (aid_size == 0 || aid_size > QMI_UIM_MAX_AID_LEN)
      {
        QCRIL_LOG_ERROR("Error converting AID string into binary");
        break;
      }

      /* If opening ARA-M application, we need to ensure that the provisioning application has
         been activated before accessing as to not delay provisioning of the USIM/CSIM applicaiton. */
      if ((slot_id < QMI_UIM_MAX_CARD_COUNT) &&
          (qcril_uim.card_status.card[slot_id].card_state == QMI_UIM_CARD_STATE_PRESENT) &&
          (qcril_uim_ara_m_delay_timer.timer_msg_ptr == NULL) &&
          (aid_size == sizeof(ara_m_aid)) &&
          (memcmp(aid_buffer, ara_m_aid, sizeof(ara_m_aid)) == 0))
      {
        struct timespec       current_time = {};
        struct timeval        init_time    = {};
        struct timeval        check_time   = {};

        /* Get the current time */
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        check_time.tv_sec  = current_time.tv_sec;
        check_time.tv_usec  = current_time.tv_nsec/1000;

        /* Give three seconds for provisioning app initalization following card insertion */
        init_time.tv_sec = qcril_uim.card_inserted_time.tv_sec + QCRIL_UIM_ARA_M_APP_OPEN_WAIT_TIME;
        init_time.tv_usec = qcril_uim.card_inserted_time.tv_nsec/1000;

        QCRIL_LOG_INFO("Init time tv_sec:0x%x, tv_usec:0x%x, current time tv_sec:0x%x, tv_usec:0x%x",
                             init_time.tv_sec, init_time.tv_usec, check_time.tv_sec, check_time.tv_usec);

        /* Card inserted time is zero or current time is lesser than init time then delay ARA-M app open */
        if((qcril_uim.card_inserted_time.tv_sec == 0 && qcril_uim.card_inserted_time.tv_nsec == 0) ||
           (0 > util_timer_compare_times(&check_time, &init_time)))
        {
          TimeKeeper::millisec delay_time = QCRIL_UIM_ARA_M_APP_OPEN_WAIT_TIME * 1000;

          QCRIL_LOG_INFO( "Delay the ARA-M APP open request");

          /* No Card insert event received yet delay ARA-M app open for three seconds, request might
             received just at the time of card reset */
          if(qcril_uim.card_inserted_time.tv_sec != 0 ||
             qcril_uim.card_inserted_time.tv_nsec != 0)
          {
            struct timeval        ara_m_delay_time   = {};

            timersub(&init_time, &check_time, &ara_m_delay_time);
            QCRIL_LOG_INFO("qcril_uim_ara_m_delay_timer tv_sec:0x%x, tv_usec:0x%x",
                                ara_m_delay_time.tv_sec, ara_m_delay_time.tv_usec);
            delay_time = (ara_m_delay_time.tv_sec * 1000) + (ara_m_delay_time.tv_usec/1000);
          }
          qcril_uim_ara_m_delay_timer.req_ptr = req_ptr;
          qcril_uim_ara_m_delay_timer.timer_msg_ptr = std::make_shared<UimTimerRequestMsg>(delay_time);

          if (qcril_uim_ara_m_delay_timer.timer_msg_ptr != NULL)
          {
            GenericCallback<int> cb(
                ([](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                                std::shared_ptr<int> responseDataPtr) -> void
            {
              auto timer_msg = std::make_shared<UimTimerExpiryMsg>(qcril_uim_resend_ara_m_cmd_timer_cb, nullptr);

              (void)solicitedMsg;
              (void)status;
              (void)responseDataPtr;

              if (timer_msg != NULL)
              {
                timer_msg->dispatch();
              }
              else
              {
                qcril_uim_resend_ara_m_cmd_timer_cb(NULL);
              }
            }));
            qcril_uim_ara_m_delay_timer.timer_msg_ptr->setCallback(&cb);
            qcril_uim_ara_m_delay_timer.timer_msg_ptr->dispatch();
            return;
          }
  
          /* Timer failed cleanup cache */
          qcril_uim_ara_m_delay_timer.req_ptr = NULL;
          qcril_uim_ara_m_delay_timer.timer_msg_ptr = NULL;
        }
      }

      /* Update AID info */
      open_logical_ch_params.aid_present = QMI_UIM_TRUE;
      open_logical_ch_params.aid.data_ptr = (unsigned char*)aid_buffer;
      open_logical_ch_params.aid.data_len = (unsigned short)aid_size;
    }

    open_logical_ch_params.file_control_information.is_valid = QMI_UIM_TRUE;

    if (req_ptr->get_p2() > QCRIL_UIM_INVALID_OPEN_CH_P2)
    {
      ret = qcril_uim_convert_p2_value(req_ptr->get_p2(), &qcril_fci_value);
      if(ret != RIL_UIM_E_SUCCESS)
      {
        break;
      }
    }
    else
    {
      /* When opening the channel, ask for appropriate template depending upon
         whether the it is an ICC card or a specific AID */
      if (qcril_uim_find_app_in_slot(slot_id, QMI_UIM_APP_SIM) ||
          qcril_uim_find_app_in_slot(slot_id, QMI_UIM_APP_RUIM))
      {
        qcril_fci_value = QCRIL_UIM_FCI_VALUE_FCP;
      }
      else
      {
        qcril_fci_value = qcril_uim_determine_select_template_from_aid(req_ptr->get_aid().c_str());
      }
    }

    open_logical_ch_params.file_control_information.fci_value =
                                   qcril_uim_convert_fci_value(qcril_fci_value);

    /* Proceed with logical channel request */
    if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_OPEN_LOGICAL_CHANNEL,
                                     (void *)&open_logical_ch_params,
                                     req_ptr) >= 0)
    {
      return;
    }

    ret = RIL_UIM_E_MODEM_ERR;
  } while(0);

  rsp_ptr = std::make_shared<RIL_UIM_OpenChannelResponse>();

  if (rsp_ptr != NULL)
  {
    rsp_ptr->err = ret;
  }
  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_open_logical_ch_req */


/*===========================================================================

  FUNCTION: qcril_uim_resend_ara_m_cmd_timer_cb

===========================================================================*/
/*!
    @brief
    Timer callback function responsible to re-send ARA-M Open channel cmd.

    @return
    None
*/
/*=========================================================================*/
static void qcril_uim_resend_ara_m_cmd_timer_cb
(
  void *param_ptr
)
{
  (void) param_ptr;

  if(qcril_uim_ara_m_delay_timer.timer_msg_ptr != NULL)
  {
    QCRIL_LOG_INFO("ARA M app open delay timer callback");

    qcril_uim_open_logical_ch_req(qcril_uim_ara_m_delay_timer.req_ptr);

    /* Done with timer request, cleanup cache */
    qcril_uim_ara_m_delay_timer.req_ptr = NULL;
    qcril_uim_ara_m_delay_timer.timer_msg_ptr = NULL;
  }
} /* qcril_uim_resend_ara_m_cmd_timer_cb */


/*===========================================================================

  FUNCTION:  qcril_uim_send_close_logical_ch_req

===========================================================================*/
/*!
    @brief
    Responsible to send QMI Close channel request to the modem.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_close_logical_ch_req
(
  const std::shared_ptr<UimSIMCloseChannelRequestMsg> req_ptr
)
{
  qmi_uim_logical_channel_params_type   logical_channel_params  = {};
  std::shared_ptr<RIL_UIM_Errno>        rsp_ptr                 = NULL;
  uint8_t                               slot_id                 = qmi_ril_get_process_instance_id();

  /* Return with error if input pointer was not provided */
  if (req_ptr == NULL)
  {
    QCRIL_LOG_ERROR( " Invalid input pointer \n");
    return;
  }

  QCRIL_LOG_INFO( "qcril_uim_request_logical_channel");

  /* Fill QMI parameter */
  memset(&logical_channel_params, 0, sizeof(qmi_uim_logical_channel_params_type));
  logical_channel_params.slot = (qmi_uim_slot_type)((uint8_t)QMI_UIM_SLOT_1 + slot_id);
  logical_channel_params.operation_type = QMI_UIM_LOGICAL_CHANNEL_CLOSE;
  logical_channel_params.channel_data.close_channel_info.channel_id = req_ptr->get_channel_id();
  logical_channel_params.channel_data.close_channel_info.terminate_app = QMI_UIM_FALSE;

  qcril_uim_remove_select_response_info_entry(
    logical_channel_params.channel_data.close_channel_info.channel_id);

  /* Proceed with logical channel request */
  if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_LOGICAL_CHANNEL,
                                   (void *)&logical_channel_params,
                                   req_ptr) >= 0)
  {
    return;
  }

  rsp_ptr = std::make_shared<RIL_UIM_Errno>(RIL_UIM_E_MODEM_ERR);

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_close_logical_ch_req */


/*===========================================================================

  FUNCTION:  qcril_uim_send_reselect_req

===========================================================================*/
/*!
    @brief
    Responsible to send QMI reselect request to the modem.

    @return
    TRUE if successful, FALSE otherwise
*/
/*=========================================================================*/
static boolean qcril_uim_send_reselect_req
(
  int                             channel_id,
  qmi_uim_slot_type               slot,
  qmi_uim_send_apdu_params_type & apdu_params,
  std::shared_ptr<UimTransmitAPDURequestMsg>  req_ptr
)
{
  qmi_uim_reselect_params_type  reselect_params = {};
  auto                          msg_ptr         = std::make_shared<UimReselectReqMsg>(req_ptr, apdu_params);

  if (msg_ptr == NULL)
  {
    return FALSE;
  }

  QCRIL_LOG_INFO( "qcril_uim_send_reselect_req (channel id: 0x%x)", channel_id);

  /* Fill QMI parameter */
  reselect_params.slot = slot;
  reselect_params.channel_id = channel_id;
  reselect_params.select_mode = QMI_UIM_SELECT_MODE_NEXT;

  /* Proceed with logical channel request */
  if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_RESELECT,
                                   (void *)&reselect_params,
                                   msg_ptr) >= 0)
  {
    return TRUE;
  }

  return FALSE;
} /* qcril_uim_send_reselect_req */


/*===========================================================================

  FUNCTION:  qcril_uim_request_send_apdu

===========================================================================*/
/*!
    @brief
    Handles these RIL requests:

    RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC
    RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_send_apdu
(
  const std::shared_ptr<UimTransmitAPDURequestMsg> req_ptr
)
{
  uint16                            in_apdu_length       = 0;
  qmi_uim_send_apdu_params_type     send_apdu_params     = {};
  int                               cla                  = 0;
  int                               ins                  = 0;
  int                               p1                   = 0;
  int                               p2                   = 0;
  int                               p3                   = 0;
  const char                      * data_ptr             = NULL;
  int                               channel_id           = 0;
  uint8                             i                    = 0;
  boolean                           send_select_rsp_data = FALSE;
  RIL_UIM_SIM_IO_Response           ril_response         = {};
  RIL_UIM_Errno                     ril_err              = RIL_UIM_E_INTERNAL_ERR;
  uint8                             select_resp_index    = QCRIL_UIM_MAX_SELECT_RESP_COUNT;
  qmi_uim_slot_type                 slot                 = QMI_UIM_SLOT_1;
  const RIL_UIM_SIM_APDU          * request_apdu_ptr     = NULL;
  std::shared_ptr<RIL_UIM_SIM_IO_Response> rsp_ptr       = NULL;

  if(req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot process response");
    QCRIL_ASSERT(0);
    return;
  }

  request_apdu_ptr = req_ptr->get_sim_apdu_data();

  if (request_apdu_ptr == NULL)
  {
    ril_err = RIL_UIM_E_INVALID_ARGUMENTS;
    goto send_apdu_response;
  }

  switch(req_ptr->get_slot())
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
      QCRIL_LOG_ERROR("Invalid slot_id in input: 0x%x\n", req_ptr->get_slot());
      ril_err = RIL_UIM_E_INVALID_ARGUMENTS;
      goto send_apdu_response;
  }

   /* Update parameters from input pointer */
  cla        = request_apdu_ptr->cla;
  ins        = request_apdu_ptr->instruction;
  channel_id = request_apdu_ptr->sessionid;
  p1         = request_apdu_ptr->p1;
  p2         = request_apdu_ptr->p2;
  p3         = request_apdu_ptr->p3;
  data_ptr   = request_apdu_ptr->data.c_str();

  /* Note - An ADPU is constructed from input APIs as follows:
     RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC or RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL: RIL_SIM_APDU
     Mapping for these input structs to CLA + INS + P1 + P2 + P3 + data:
     channel_id: fileid or sessionid, CLA: cla, INS: command or instruction,
     P1: p1, P2: p2, P3: p3, data: data
     Only RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL have the valid channel_id */
  /* Log only if adb logs not on, apdu logs not get printed in adb logs */
  if (!qcril_uim_log_adb_on)
  {
    QCRIL_LOG_INFO( "qcril_uim_request_send_apdu(0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, %s)\n",
                    cla,
                    ins,
                    channel_id,
                    p1,
                    p2,
                    p3,
                    data_ptr != NULL ? data_ptr : "NULL" );
  }
  else
  {
    /* In case of adb log print only cls ins and last two bytes of data */
    QCRIL_LOG_INFO( "qcril_uim_request_send_apdu(0x%X, 0x%X, **, **, **, **, ***%s)\n",
                    cla,
                    ins,
                    data_ptr == NULL ? "NULL" : ((strlen(data_ptr) <= 4) ? data_ptr : (data_ptr + strlen(data_ptr) - 4)));
  }

  /* Sanity check */
  if ((p1  < 0)    || (p1  > 0xFF) ||
      (p2  < 0)    || (p2  > 0xFF) ||
      (cla < 0)    || (cla > 0xFF) ||
      (ins < 0)    || (ins > 0xFF))
  {
    QCRIL_LOG_ERROR( "Unsupported case, P1: 0x%X, P2: 0x%X, P3: 0x%X, cla: 0x%X, ins: 0x%X \n",
                     p1, p2, p3, cla, ins);
    ril_err = RIL_UIM_E_INVALID_ARGUMENTS;
    goto send_apdu_response;
  }

  /* Limitation on instruction byte is applicable only in case of T=0 protocol,
     but QCRIL does not have that information. So, blocking request for telecom
     cards (as in practice they all use T=0 protocol), but allow it for non
     telecom cards: in case, the modem will reject the APDU if needed, based on
     the protocol */
  if (req_ptr->is_telecom_card() &&
      (ins & 0xF0) == QCRIL_UIM_INVALID_INS_BYTE_MASK)
  {
    QCRIL_LOG_ERROR( "Invalid INS byte 0x%X", ins);
    ril_response.sw1 = QCRIL_UIM_SW1_INS_CODE_NOT_SUPPORTED;
    ril_response.sw2 = QCRIL_UIM_SW2_NORMAL_END;
    ril_err          = RIL_UIM_E_SUCCESS;
    goto send_apdu_response;
  }

  /* First, we loop through to find if there is a select response data
     that matches the incoming channel id  */
  for (i = 0; i < QCRIL_UIM_MAX_SELECT_RESP_COUNT; i++)
  {
    if ((qcril_uim.select_response_info[i].in_use) &&
        (qcril_uim.select_response_info[i].channel_id == channel_id))
    {
      select_resp_index = i;
      break;
    }
  }

  /* Next if the APDU request was a GET RESPONSE to a previous OPEN CHANNEL,
     we respond immediately with the stored response select response data. */
  if ((ins               == QCRIL_UIM_INS_BYTE_GET_RESPONSE) &&
      (p1                == 0)                               &&
      (p2                == 0)                               &&
      (p3                == 0)                               &&
      (select_resp_index < QCRIL_UIM_MAX_SELECT_RESP_COUNT))
  {
    ril_err = qcril_uim_update_get_response_apdu(select_resp_index,
                                                 &ril_response);
    send_select_rsp_data = TRUE;
  }

  /* In any case cleanup select response info if it was stored previously
     for a particular channel id */
  if (select_resp_index < QCRIL_UIM_MAX_SELECT_RESP_COUNT)
  {
    /* Free the cached buffer, if available*/
    if (qcril_uim.select_response_info[select_resp_index].select_resp_ptr)
    {
      qcril_free(qcril_uim.select_response_info[select_resp_index].select_resp_ptr);
      qcril_uim.select_response_info[select_resp_index].select_resp_ptr = NULL;
    }
    /* Also clean up the entire entry */
    memset(&qcril_uim.select_response_info[select_resp_index],
           0,
           sizeof(qcril_uim_select_response_info_type));
  }

  if (send_select_rsp_data)
  {
    QCRIL_LOG_DEBUG( "RIL_SIM_IO_Response: sw1=0x%X sw2=0x%X data=%s\n",
                      ril_response.sw1, ril_response.sw2,
                      ril_response.simResponse.c_str());
    goto send_apdu_response;
  }

  /* Compose the Send APDU parameters */
  send_apdu_params.slot = slot;

  /*set procedure bytes */
  send_apdu_params.procedure_bytes = qcril_uim.procedure_bytes;

  if (!req_ptr->is_basic_channel())
  {
    send_apdu_params.channel_id         = channel_id;
    send_apdu_params.channel_id_present = QMI_UIM_TRUE;
  }
  else
  {
    send_apdu_params.channel_id_present = QMI_UIM_FALSE;
  }

  /* Calculate total buffer size for APDU data */
  if ((data_ptr == NULL) || (strlen(data_ptr) == 0))
  {
    if (p3 < 0)
    {
      in_apdu_length = QCRIL_UIM_APDU_MIN_SIZE;
    }
    else if(p3 < QCRIL_UIM_APDU_MAX_SHORT_APDU_SIZE)
    {
      in_apdu_length = QCRIL_UIM_APDU_MIN_SIZE_PLUS_P3;
    }
    else
    {
      in_apdu_length = QCRIL_UIM_LONG_APDU_MIN_SIZE_PLUS_P3;
    }
  }
  else if(p3 < QCRIL_UIM_APDU_MAX_SHORT_APDU_SIZE)
  {
    in_apdu_length = QCRIL_UIM_APDU_MIN_SIZE_PLUS_P3 + strlen(data_ptr)/2;
  }
  else
  {
    in_apdu_length = QCRIL_UIM_LONG_APDU_MIN_SIZE_PLUS_P3 + strlen(data_ptr)/2;
  }

  /* Allocate memory and compose the raw APDU data */
  send_apdu_params.apdu.data_ptr = (uint8*) qcril_malloc(in_apdu_length);
  if (send_apdu_params.apdu.data_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Unable to allocate buffer for apdu.data_ptr!");
    ril_err = RIL_UIM_E_NO_MEMORY;
    goto send_apdu_response;
  }

  send_apdu_params.apdu.data_len = in_apdu_length;

  if (qcril_uim_compose_apdu_data(&send_apdu_params.apdu,
                                  cla,
                                  ins,
                                  p1,
                                  p2,
                                  p3,
                                  data_ptr) == FALSE)
  {
    QCRIL_LOG_ERROR("Error composing APDU data!");
    ril_err = RIL_UIM_E_SYSTEM_ERR;
    goto send_apdu_response;
  }

  /* Check for SelectNext request via streaming APDU. If so, use reselect QMI API.
     Also note that for backward compatibility with older modems that doesnt support
     reselect QMI API, this incoming APDU request is stored & used in reselect cb */
  if ((ins         == QCRIL_UIM_INS_BYTE_SELECT)            &&
      (p1          == QCRIL_UIM_P1_VALUE_SELECT_BY_DF_NAME) &&
      ((p2 & 0x03) == QCRIL_UIM_P2_MASK_SELECT_NEXT))
  {
    if (qcril_uim_send_reselect_req(channel_id, slot, send_apdu_params, req_ptr))
    {
      return;
    }
    /* Upon error, send error response back to client */
    ril_err = RIL_UIM_E_MODEM_ERR;
    goto send_apdu_response;
  }

  /* Proceed with send APDU request */
  if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_SEND_APDU,
                                   (void *)&send_apdu_params,
                                   req_ptr) >= 0)
  {
    qcril_free(send_apdu_params.apdu.data_ptr);
    send_apdu_params.apdu.data_ptr = NULL;
    return;
  }
  ril_err = RIL_UIM_E_MODEM_ERR;

send_apdu_response:
  ril_response.err = ril_err;
  ril_response.slot_id = req_ptr->get_slot();

  rsp_ptr = std::make_shared<RIL_UIM_SIM_IO_Response>(ril_response);

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);

  /* Clean up any APDU buffer if allocated */
  if(send_apdu_params.apdu.data_ptr)
  {
    qcril_free(send_apdu_params.apdu.data_ptr);
    send_apdu_params.apdu.data_ptr = NULL;
  }
} /* qcril_uim_request_send_apdu */


/*=========================================================================

  FUNCTION:  qcril_uim_request_set_apdu_behavior

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_HOOK_SLOT_SET_APDU_BEHAVIOR_REQ request from QCRIL.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_set_apdu_behavior
(
  const std::shared_ptr<UimSetAPDUBehaviorRequestMsg> req_ptr
)
{
  qmi_uim_set_apdu_behavior_params_type           set_apdu_behavior_params = {};
  RIL_UIM_SetAPDUBehavior                         set_apdu                 = {};
  RIL_UIM_Errno                                   ril_err                  = RIL_UIM_E_INTERNAL_ERR;
  std::shared_ptr<RIL_UIM_Errno>                  rsp_ptr                  = NULL;

  /* Sanity check */
  if (req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    return;
  }

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  set_apdu  = req_ptr->get_value();
  set_apdu_behavior_params.slot = (qmi_uim_slot_type)(req_ptr->get_slot() + (uint8_t)QMI_UIM_SLOT_1);

  set_apdu_behavior_params.channel_id = set_apdu.channel_id;

  switch(set_apdu.apdu_behavior)
  {
    case RIL_UIM_UICC_APDU_BEHAVIOR_NO_GET_RESPONSE_FOR_WARNING:
      set_apdu_behavior_params.apdu_behavior = QMI_UIM_APDU_BEHAVIOR_NO_GET_RESPONSE_FOR_WARNING;
      break;
    case RIL_UIM_UICC_APDU_BEHAVIOR_AUTOMATIC_GET_RESPONSE_FOR_WARNING:
      set_apdu_behavior_params.apdu_behavior = QMI_UIM_APDU_BEHAVIOR_AUTOMATIC_GET_RESPONSE_FOR_WARNING;
      break;
    default:
      QCRIL_LOG_ERROR( " Invalid apdu_behavior in input: 0x%x\n", set_apdu.apdu_behavior);
      goto set_apdu_behavior_req_error;
  }

  if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_SET_APDU_BEHAVIOR,
                                   (void *)&set_apdu_behavior_params,
                                   req_ptr) >= 0)
  {
    return;
  }
  else
  {
    ril_err = RIL_UIM_E_MODEM_ERR;
  }

set_apdu_behavior_req_error:
  rsp_ptr = std::make_shared<RIL_UIM_Errno>(ril_err);

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_request_set_apdu_behavior */


/*===========================================================================

  FUNCTION:  qcril_uim_process_internal_verify_pin_command_callback

===========================================================================*/
/*!
    @brief
    Handler for QCRIL_EVT_INTERNAL_UIM_VERIFY_PIN_COMMAND_CALLBACK.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_process_internal_verify_pin_command_callback
(
  qcril_uim_callback_params_type             * params_ptr,
  std::shared_ptr<UimVerifyPIN2Msg>            msg_ptr
)
{
  RIL_UIM_Errno   ril_err      = RIL_UIM_E_SUCCESS;
  int32_t         num_retries  = -1;

  if(params_ptr == NULL || msg_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot process response");
    QCRIL_ASSERT(0);
    return;
  }

  auto req_ptr = msg_ptr->get_request_msg();

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (req_ptr == NULL)
  {
    return;
  }

  /* In case of PIN error, return immediately */
  if (params_ptr->qmi_rsp_data.qmi_err_code != 0)
  {
    num_retries = (int32_t)params_ptr->qmi_rsp_data.rsp_data.verify_pin_rsp.num_retries;

    /* Change the error code appropriately */
    switch(params_ptr->qmi_rsp_data.qmi_err_code)
    {
      case QMI_UIM_SERVICE_ERR_INCORRECT_PIN:
      case QMI_UIM_SERVICE_ERR_PIN_PERM_BLOCKED:
        ril_err = RIL_UIM_E_PASSWORD_INCORRECT;
        break;

      case QMI_UIM_SERVICE_ERR_PIN_BLOCKED:
        ril_err = RIL_UIM_E_SIM_PUK2;
        break;

      default:
        ril_err = RIL_UIM_E_INTERNAL_ERR;
        break;
    }
  }

  if (string(UimSetFacilityLockRequestMsg::MESSAGE_NAME) == req_ptr->get_message_name())
  {
    auto msg = std::static_pointer_cast<UimSetFacilityLockRequestMsg>(req_ptr);

    if (ril_err == RIL_UIM_E_SUCCESS)
    {
      qmi_uim_set_service_status_params_type    service_status_params = {};

      service_status_params.session_info = msg_ptr->get_session_info();

      if (msg->get_status())
      {
        service_status_params.fdn_status = QMI_UIM_SERVICE_ENABLE;
      }
      else
      {
        service_status_params.fdn_status = QMI_UIM_SERVICE_DISABLE;
      }

      if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_SET_FDN,
                                       (void *)&service_status_params,
                                       req_ptr) >= 0)
      {
        return;
      }
      else
      {
        ril_err = RIL_UIM_E_MODEM_ERR;
      }
    }

    auto rsp_ptr = std::make_shared<RIL_UIM_SIM_PIN_Response>();

    if (rsp_ptr != NULL)
    {
      rsp_ptr->err = ril_err;
      rsp_ptr->no_of_retries = num_retries;
    }

    msg->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
  }
  else if (string(UimReadTransparentReqMsg::MESSAGE_NAME) == req_ptr->get_message_name())
  {
    auto msg = std::static_pointer_cast<UimReadTransparentReqMsg>(req_ptr);

    if (msg != NULL)
    {
      if (ril_err == RIL_UIM_E_SUCCESS)
      {
        if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_READ_TRANSPARENT,
                                         (void *)msg->get_params(),
                                         msg) >= 0)
        {
          return;
        }
        else
        {
          ril_err = RIL_UIM_E_MODEM_ERR;
        }
      }

      auto rsp_ptr = std::make_shared<RIL_UIM_SIM_IO_Response>();
      auto req     = msg->get_request_msg();

      if (rsp_ptr != NULL)
      {
        rsp_ptr->err = ril_err;
        rsp_ptr->slot_id =
          std::static_pointer_cast<UimSIMIORequestMsg>(req)->get_slot();
      }
      if (req != NULL)
      {
        std::static_pointer_cast<UimSIMIORequestMsg>(req)->sendResponse
                  (req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
      }
    }
  }
  else if (string(UimReadRecordReqMsg::MESSAGE_NAME) == req_ptr->get_message_name())
  {
    auto msg = std::static_pointer_cast<UimReadRecordReqMsg>(req_ptr);

    if (msg != NULL)
    {
      if (ril_err == RIL_UIM_E_SUCCESS)
      {
        if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_READ_RECORD,
                                         (void *)msg->get_params(),
                                         msg) >= 0)
        {
          return;
        }
        else
        {
          ril_err = RIL_UIM_E_MODEM_ERR;
        }
      }

      auto rsp_ptr = std::make_shared<RIL_UIM_SIM_IO_Response>();
      auto req     = msg->get_request_msg();

      if (rsp_ptr != NULL)
      {
        rsp_ptr->err = ril_err;
        rsp_ptr->slot_id =
          std::static_pointer_cast<UimSIMIORequestMsg>(req)->get_slot();
      }
      if (req != NULL)
      {
        std::static_pointer_cast<UimSIMIORequestMsg>(req)->sendResponse
                  (req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
      }
    }
  }
  else if (string(UimUpdateBinaryReqMsg::MESSAGE_NAME) == req_ptr->get_message_name())
  {
    auto msg = std::static_pointer_cast<UimUpdateBinaryReqMsg>(req_ptr);

    if (msg != NULL)
    {
      qmi_uim_write_transparent_params_type  *write_params = msg->get_params();

      if (ril_err == RIL_UIM_E_SUCCESS)
      {
        if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_WRITE_TRANSPARENT,
                                         (void *)write_params,
                                         msg) >= 0)
        {
          return;
        }
        else
        {
          ril_err = RIL_UIM_E_MODEM_ERR;
        }
      }

      auto rsp_ptr = std::make_shared<RIL_UIM_SIM_IO_Response>();
      auto req     = msg->get_request_msg();

      if (rsp_ptr != NULL)
      {
        rsp_ptr->err = ril_err;
        rsp_ptr->slot_id =
          std::static_pointer_cast<UimSIMIORequestMsg>(req)->get_slot();
      }
      if (req != NULL)
      {
        std::static_pointer_cast<UimSIMIORequestMsg>(req)->sendResponse
                  (req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
      }

      if (write_params != NULL && write_params->data.data_ptr != NULL)
      {
        qcril_free(write_params->data.data_ptr);
        write_params->data.data_ptr = NULL;
      }
    }
  }
  else if (string(UimUpdateRecordReqMsg::MESSAGE_NAME) == req_ptr->get_message_name())
  {
    auto msg = std::static_pointer_cast<UimUpdateRecordReqMsg>(req_ptr);

    if (msg != NULL)
    {
      qmi_uim_write_record_params_type  *write_params = msg->get_params();

      if (ril_err == RIL_UIM_E_SUCCESS)
      {
        if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_WRITE_RECORD,
                                           (void *)write_params,
                                           msg) >= 0)
        {
          return;
        }
        else
        {
          ril_err = RIL_UIM_E_MODEM_ERR;
        }
      }

      auto rsp_ptr = std::make_shared<RIL_UIM_SIM_IO_Response>();
      auto req     = msg->get_request_msg();

      if (rsp_ptr != NULL)
      {
        rsp_ptr->err = ril_err;
        rsp_ptr->slot_id =
          std::static_pointer_cast<UimSIMIORequestMsg>(req)->get_slot();
      }
      if (req != NULL)
      {
        std::static_pointer_cast<UimSIMIORequestMsg>(req)->sendResponse
                  (req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
      }

      if (write_params != NULL && write_params->data.data_ptr != NULL)
      {
        qcril_free(write_params->data.data_ptr);
        write_params->data.data_ptr = NULL;
      }
    }
  }
  else
  {
    QCRIL_LOG_ERROR( "%s: Invalid cmd for internal pin2 verification: %s\n",
                      __FUNCTION__, req_ptr->get_message_name().c_str());
  }
} /* qcril_uim_process_internal_verify_pin_command_callback */


/*===========================================================================

  FUNCTION:  qcril_uim_cleanup_ara_m_delay_cache_info

===========================================================================*/
/*!
    @brief
    Cleans the global ARA-M APP open cache and sends error response.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_cleanup_ara_m_delay_cache_info
(
  void
)
{
  std::shared_ptr<RIL_UIM_OpenChannelResponse>  rsp_ptr = NULL;

  memset(&qcril_uim.card_inserted_time, 0x00, sizeof(qcril_uim.card_inserted_time));

  /* Stop ARA-M delay timer if running */
  if(qcril_uim_ara_m_delay_timer.timer_msg_ptr != NULL)
  {
    qcril_uim_ara_m_delay_timer.timer_msg_ptr->cancelled();
    qcril_uim_ara_m_delay_timer.timer_msg_ptr = NULL;

    rsp_ptr = std::make_shared<RIL_UIM_OpenChannelResponse>();

    if (rsp_ptr != NULL)
    {
      rsp_ptr->err = RIL_UIM_E_INTERNAL_ERR;
    }

    qcril_uim_ara_m_delay_timer.req_ptr->sendResponse(qcril_uim_ara_m_delay_timer.req_ptr,
                                                      Message::Callback::Status::SUCCESS,
                                                      rsp_ptr);
  }

  qcril_uim_ara_m_delay_timer.req_ptr = NULL;
} /* qcril_uim_cleanup_ara_m_delay_cache_info */


/*===========================================================================

  FUNCTION:  qcril_uim_direct_get_imsi

===========================================================================*/
/*!
    @brief
    Returns IMSI of requested aid of a slot

    @return
    None

    Note: imsi_ptr need to delete by client
*/
/*=========================================================================*/
RIL_UIM_Errno qcril_uim_direct_get_imsi
(
  uint8            slot,
  const char     * aid_ptr,
  char          ** imsi_ptr
)
{
  qmi_uim_read_transparent_params_type  read_params;
  RIL_UIM_Errno                         err        = RIL_UIM_E_SUCCESS;
  qmi_uim_rsp_data_type                 qmi_rsp_data;

  if(slot >= QMI_UIM_MAX_CARD_COUNT || imsi_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot process request");
    return RIL_UIM_E_INVALID_ARGUMENTS;
  }

  memset(&read_params, 0, sizeof(qmi_uim_read_transparent_params_type));

  err = qcril_uim_get_imsi_path_for_aid(&read_params, aid_ptr, slot);
  if (err != RIL_UIM_E_SUCCESS)
  {
    QCRIL_UIM_FREE_IF_NOT_NULL(read_params.file_id.path.data_ptr);
    return err;
  }

  /* Proceed with read transparent  */
  memset(&qmi_rsp_data, 0, sizeof(qmi_uim_rsp_data_type));

  if (qcril_uim_send_qmi_sync_msg(QCRIL_UIM_REQUEST_READ_TRANSPARENT,
                                  &read_params,
                                  &qmi_rsp_data) >= 0)
  {
    int imsi_len = 0;

    err = qcril_uim_convert_err_value(qmi_rsp_data.qmi_err_code);

    QCRIL_LOG_DEBUG( "qcril_uim_direct_get_imsi_resp: qmi_err_code=%d, session_type=%d \n",
                      qmi_rsp_data.qmi_err_code,
                      read_params.session_info.session_type);

    /* Based on the type of request, fetch appropriate IMSI */
    if (err == RIL_UIM_E_SUCCESS)
    {
      if((read_params.session_info.session_type == QMI_UIM_SESSION_TYPE_PRI_GW_PROV) ||
         (read_params.session_info.session_type == QMI_UIM_SESSION_TYPE_SEC_GW_PROV) ||
         (read_params.session_info.session_type == QMI_UIM_SESSION_TYPE_TER_GW_PROV))
      {
        *imsi_ptr = qcril_uim_parse_gw_imsi(
                     qmi_rsp_data.rsp_data.read_transparent_rsp.content.data_ptr,
                     qmi_rsp_data.rsp_data.read_transparent_rsp.content.data_len,
                     &imsi_len);
      }
      else if((read_params.session_info.session_type == QMI_UIM_SESSION_TYPE_PRI_1X_PROV) ||
              (read_params.session_info.session_type == QMI_UIM_SESSION_TYPE_SEC_1X_PROV) ||
              (read_params.session_info.session_type == QMI_UIM_SESSION_TYPE_TER_1X_PROV))
      {
        *imsi_ptr = qcril_uim_parse_1x_imsi(
                     qmi_rsp_data.rsp_data.read_transparent_rsp.content.data_ptr,
                     qmi_rsp_data.rsp_data.read_transparent_rsp.content.data_len,
                     &imsi_len);
      }
    }
    if (*imsi_ptr == NULL)
    {
      QCRIL_UIM_FREE_IF_NOT_NULL(read_params.file_id.path.data_ptr);
      return RIL_UIM_E_INTERNAL_ERR;
    }

    /* Client needs to free the memory for raw data */
    QCRIL_UIM_FREE_IF_NOT_NULL(qmi_rsp_data.rsp_data.read_transparent_rsp.content.data_ptr);
    QCRIL_UIM_FREE_IF_NOT_NULL(read_params.file_id.path.data_ptr);
    return RIL_UIM_E_SUCCESS;
  }
  QCRIL_UIM_FREE_IF_NOT_NULL(read_params.file_id.path.data_ptr);
  return RIL_UIM_E_MODEM_ERR;
} /* qcril_uim_direct_get_imsi */


/*=========================================================================

  FUNCTION:  qcril_uim_request_set_terminal_capability

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_HOOK_SLOT_SET_TERMINAL_CAPABILITY_LPA_TLV_REQ request
    from QCRIL.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_set_terminal_capability
(
  const std::shared_ptr<UimSetLPATermialCapabilitesRequestMsg> req_ptr
)
{
  qmi_uim_terminal_capability_params_type  set_terminal_capability_params  = {};
  std::shared_ptr<RIL_UIM_Errno>           rsp_ptr                         = NULL;
  uint8_t                                  lpa_value                       = 0;
  /* Sanity check */
  if (req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    return;
  }

  QCRIL_LOG_INFO("%s\n", __FUNCTION__);

  /* Find slot info */
  set_terminal_capability_params.slot = (qmi_uim_slot_type)(req_ptr->get_slot() + (uint8_t)QMI_UIM_SLOT_1);

  /* We currently support set/get terminal capability only for LPA TLV
     (that has a tag of 0x83), which can be further enhanced to other
     TLVs in the future, based on the requirements */
  set_terminal_capability_params.num_tlvs = (uint8_t) 0x01;
  set_terminal_capability_params.tlvs[0].tag = (uint8_t) QMI_UIM_TERMINAL_CAPABILITY_LPA_TLV_TAG;
  lpa_value = req_ptr->get_value();
  set_terminal_capability_params.tlvs[0].value.data_ptr = &lpa_value;
  set_terminal_capability_params.tlvs[0].value.data_len = (unsigned short) QMI_UIM_TERMINAL_CAPABILITY_LPA_TLV_LEN;
  if (req_ptr->is_tlv_present())
  {
    set_terminal_capability_params.tlvs[0].tlv_present = QMI_UIM_TRUE;
  }
  else
  {
    set_terminal_capability_params.tlvs[0].tlv_present = QMI_UIM_FALSE;
  }

  if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_SET_TERMINAL_CAPABILITY,
                                   &set_terminal_capability_params,
                                   req_ptr) >= 0)
  {
    return;
  }

  rsp_ptr = std::make_shared<RIL_UIM_Errno>(RIL_UIM_E_MODEM_ERR);

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_request_set_terminal_capability */


/*=========================================================================

  FUNCTION:  qcril_uim_request_get_terminal_capability

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_HOOK_SLOT_GET_TERMINAL_CAPABILITY_LPA_TLV_REQ request
    from QCRIL.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_get_terminal_capability
(
  const std::shared_ptr<UimGetLPATermialCapabilitesRequestMsg> req_ptr
)
{
  qmi_uim_terminal_capability_params_type     get_terminal_capability_params = {};
  std::shared_ptr<RIL_UIM_TerminalCapabilityLPATLV> rsp_ptr                  = NULL;

  /* Sanity check */
  if (req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    return;
  }

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  /* Find slot info */
  get_terminal_capability_params.slot = (qmi_uim_slot_type)(req_ptr->get_slot() + (uint8_t)QMI_UIM_SLOT_1);

  if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_GET_TERMINAL_CAPABILITY,
                                     &get_terminal_capability_params,
                                     req_ptr) >= 0)
  {
    return;
  }

  rsp_ptr = std::make_shared<RIL_UIM_TerminalCapabilityLPATLV>();
  if (rsp_ptr != NULL)
  {
    rsp_ptr->err = RIL_UIM_E_MODEM_ERR;
  }

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_request_get_terminal_capability */


/*===========================================================================*/
/*!
    @brief
    Processes the GID request

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_get_gid
(
  const std::shared_ptr<UimGetGIDRequestMsg> req_ptr
)
{
  qmi_uim_read_transparent_params_type  read_params      = {};
  RIL_UIM_Errno                         err              = RIL_UIM_E_SUCCESS;
  std::shared_ptr<RIL_UIM_GIDResponse>  rsp_ptr          = NULL;
  std::shared_ptr<UimReadTransparentReqMsg>  req_msg_ptr = NULL;

  if(req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot process response");
    QCRIL_ASSERT(0);
    return;
  }

  QCRIL_LOG_INFO( "qcril_uim_request_get_gid(%s)\n",req_ptr->get_aid().c_str());

  /* Check session type & update file_id */
  err = qcril_uim_file_get_mcc_mnc_get_path_and_session_info(
                                        req_ptr->get_aid().c_str(),
                                        (qmi_uim_app_type)req_ptr->get_app_type(),
                                        &read_params.file_id.path,
                                        &read_params.session_info,
                                        req_ptr->get_slot());
  if (err != RIL_UIM_E_SUCCESS)
  {
    goto send_gid_error;
  }

  switch(req_ptr->get_gid_type())
  {
    case UimGetGIDRequestMsg::GID1:
      read_params.file_id.file_id = QCRIL_UIM_FILEID_EF_GID1;
      break;

    case UimGetGIDRequestMsg::GID2:
      read_params.file_id.file_id = QCRIL_UIM_FILEID_EF_GID2;
      break;

    default:
      err = RIL_UIM_E_INVALID_ARGUMENTS;
      goto send_gid_error;
  }

  req_msg_ptr = std::make_shared<UimReadTransparentReqMsg>(req_ptr,
                                                           read_params);

  if (req_msg_ptr == NULL)
  {
    err = RIL_UIM_E_NO_MEMORY;
    goto send_gid_error;
  }

  /* Proceed with read transparent  */
  if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_READ_TRANSPARENT,
                                   (void *)&read_params,
                                   req_msg_ptr) >= 0)
  {
    QCRIL_UIM_FREE_IF_NOT_NULL(read_params.file_id.path.data_ptr);
    return;
  }
  err = RIL_UIM_E_MODEM_ERR;

send_gid_error:
  rsp_ptr = std::make_shared<RIL_UIM_GIDResponse>();

  if (rsp_ptr != NULL)
  {
    rsp_ptr->err = err;
  }

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);

  QCRIL_UIM_FREE_IF_NOT_NULL(read_params.file_id.path.data_ptr)
} /* qcril_uim_request_get_gid */
