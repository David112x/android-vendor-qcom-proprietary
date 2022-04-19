/*===========================================================================

  Copyright (c) 2018 - 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include <stdint.h>
#include "qcril_gstk_log.h"
#include "qcril_gstk.h"

#define TAG "QCRIL_GSTK_LOG"

/*===========================================================================

    FUNCTION:  length_of_length_value_in_tlv

===========================================================================*/
/*!
    @brief
    This function is used to obtain number of bytes used for the encoding
    of length field in a TLV

    @return
    Number of bytes in which the length field is encoded
*/
/*=========================================================================*/
static uint8_t length_of_length_value_in_tlv
(
  const uint8_t *inp_data_ptr,
  uint32_t       inp_data_length
)
{
  uint8_t length = 0;

  if (inp_data_ptr == NULL || inp_data_length <= 1)
  {
    return 0;
  }

  /* Length value can be encoded in 1 or 2 bytes */
  if(inp_data_ptr[0] <= 0x7F)
  {
    length = 1;
  }
  else if(inp_data_ptr[0] == 0x81)
  {
    length = 2;
  }

  return length;
}/* length_of_length_value_in_tlv */


/*===========================================================================

  FUNCTION:  qcril_gstk_log_proactive_command

 ==========================================================================*/
/*!
    @brief
    This function is used to log only non privacy data in the proactive command

    @Input
    Input is assumed to be interpreted in ASCII Hex format

    @return
    None
*/
/*=========================================================================*/
void qcril_gstk_log_proactive_command
(
  uint8_t          cmd_type,
  const uint8_t   *cmd_data_ptr,
  uint32_t         cmd_length,
  bool             logging_of_private_data
)
{
   /* logging_length indicates the point till which logging can be done */
   uint8_t  logging_length = cmd_length;

  if (cmd_data_ptr == NULL || cmd_length <= 2 * QCRIL_GSTK_PROACTIVE_CMD_TAG_LENGTH)
  {
    return;
  }

  if(!logging_of_private_data)
  {
    switch (cmd_type)
    {
      case QCRIL_GSTK_QMI_CMD_STK_DISPLAY_TEXT:
      case QCRIL_GSTK_QMI_CMD_STK_GET_INKEY:
      case QCRIL_GSTK_QMI_CMD_STK_GET_INPUT:
      case QCRIL_GSTK_QMI_CMD_STK_SET_UP_MENU:
      case QCRIL_GSTK_QMI_CMD_STK_SEND_SHORT_MESSAGE:
      case QCRIL_GSTK_QMI_CMD_STK_SET_UP_CALL:
      case QCRIL_GSTK_QMI_CMD_STK_PERFORM_CARD_APDU:
      case QCRIL_GSTK_QMI_CMD_STK_SELECT_ITEM:
      case QCRIL_GSTK_QMI_CMD_STK_SET_UP_IDLE_MODE_TEXT:
      case QCRIL_GSTK_QMI_CMD_STK_RUN_AT_CMD:
      case QCRIL_GSTK_QMI_CMD_STK_SEND_DTMF:
      case QCRIL_GSTK_QMI_CMD_STK_OPEN_CHANNEL:
      case QCRIL_GSTK_QMI_CMD_STK_SEND_DATA:
      case QCRIL_GSTK_QMI_CMD_STK_LAUNCH_BROWSER:

        /* Need to log only non privacy data as per GSMA spec TS.31
           We multiplied by 2 as the input is in ASCII hex format */
        logging_length =  2 * (QCRIL_GSTK_PROACTIVE_CMD_TAG_LENGTH +
                                 QCRIL_GSTK_QMI_COMMAND_DETAILS_SIZE + QCRIL_GSTK_QMI_DEVICE_IDENTITIES_SIZE);
        break;
      default:
        break;
     }
  }

  if(logging_length != cmd_length && !logging_of_private_data)
  {
    uint8_t length_field[2] = {0};
    qcril_gstk_qmi_hexstring_to_bin(length_field , &cmd_data_ptr[2 * QCRIL_GSTK_PROACTIVE_CMD_TAG_LENGTH], 4);

    logging_length += 2 * (length_of_length_value_in_tlv(length_field, 2));

    /* Print cmd data till logging length only since privacy data should not be logged */
    QCRIL_LOG_INFO("Pcmd data is %.*s%s with length %d \n ", logging_length, cmd_data_ptr, "****..", cmd_length);
  }
  else
  {
  /* This implies that the input proactive cmd is not in the list of proactive cmds
     containing privacy data or privacy logging is enabled.
     In this scenario, we can log the whole command*/
    QCRIL_LOG_INFO("Pcmd data %.*s, length :%d \n", cmd_length, cmd_data_ptr, cmd_length);
  }
}/* qcril_gstk_log_proactive_command */


/*===========================================================================

    FUNCTION:  qcril_gstk_log_terminal_response

===========================================================================*/
/*!
    @brief
    This function is used to log only non privacy data in the Terminal response

    @Input
    Input data is assumed to be interpreted in binary format

    @return
    None
 */
/*=========================================================================*/
void qcril_gstk_log_terminal_response
(
  const uint8_t *tr_data_ptr,
  uint32_t       tr_length,
  bool           logging_of_private_data
)
{
  /* logging_length indicates the point till which logging can be done */
  uint8_t  logging_length      = tr_length;
  uint8_t  length_value_length = 0;

  if (tr_data_ptr == NULL || tr_length < QMI_CAT_TERMINAL_RESPONSE_MIN_LENGTH)
  {
    return;
  }

  if(!logging_of_private_data)
  {
    switch(tr_data_ptr[QCRIL_GSTK_QMI_COMMAND_DETAILS_SIZE - 2])
    {
      case QCRIL_GSTK_QMI_CMD_STK_GET_INKEY:
      case QCRIL_GSTK_QMI_CMD_STK_GET_INPUT:
      case QCRIL_GSTK_QMI_CMD_STK_PERFORM_CARD_APDU:
      case QCRIL_GSTK_QMI_CMD_STK_RUN_AT_CMD:
      case QCRIL_GSTK_QMI_CMD_STK_PROVIDE_LOCAL_INFO:
      case QCRIL_GSTK_QMI_CMD_STK_RECEIVE_DATA:
        logging_length = QCRIL_GSTK_QMI_COMMAND_DETAILS_SIZE + QCRIL_GSTK_QMI_DEVICE_IDENTITIES_SIZE
                          + QCRIL_GSTK_TLV_TAG_LENGTH;

        length_value_length = length_of_length_value_in_tlv(&tr_data_ptr[logging_length], tr_length);

        if(length_value_length == 1)
        {
          logging_length += tr_data_ptr[logging_length];
        }
        else if (length_value_length == 2)
        {
          logging_length += tr_data_ptr[logging_length + 1];
        }
        break;
      default:
        break;
    }
  }

  QCRIL_LOG_DEBUG("Terminal response with length %d and logging length %d", tr_length, logging_length);

  for(uint8_t index = 0; index < logging_length; index++)
  {
    QCRIL_LOG_DEBUG("0x%02X ", tr_data_ptr[index]);
  }

}/* qcril_gstk_log_terminal_response */


/*===========================================================================

    FUNCTION:  qcril_gstk_log_env_command

===========================================================================*/
/*!
    @brief
    This function is used to log only non privacy data in the envelope command

    @Input
    Input data is assumed to be interpreted in binary format

    @return
    None
*/
/*=========================================================================*/
void qcril_gstk_log_env_command
(
  const uint8_t  *env_data_ptr,
  uint32_t        env_length,
  bool            logging_of_private_data
)
{
  /* logging length indicates the point till which logging can be done */
  uint8_t logging_length = env_length;

  if (env_data_ptr == NULL || env_length <= 1)
  {
    return;
  }

  if(!logging_of_private_data)
  {
    switch(env_data_ptr[0])
    {
      case QCRIL_GSTK_ENV_CMD_MENU_SELECTION_TAG:
      case QCRIL_GSTK_ENV_CMD_CALL_CONTROL_TAG:
      case QCRIL_GSTK_ENV_CMD_SMS_PP_DOWNLOAD_TAG:
      case QCRIL_GSTK_ENV_CMD_GEO_LOC_RPT_TAG:
        logging_length += QCRIL_GSTK_ENV_CMD_TAG_LENGTH + length_of_length_value_in_tlv(&env_data_ptr[1], env_length - 1)
                           + QCRIL_GSTK_QMI_DEVICE_IDENTITIES_SIZE;
        break;
      case QCRIL_GSTK_ENV_CMD_EVT_DOWNLOAD_TAG:
        logging_length += QCRIL_GSTK_ENV_CMD_TAG_LENGTH + length_of_length_value_in_tlv(&env_data_ptr[1], env_length - 1)
                    + QCRIL_GSTK_QMI_DEVICE_IDENTITIES_SIZE + QCRIL_GSTK_ENVELOPE_CMD_EVENT_LIST_LEN;
        break;
      default:
        break;
    }
  }

  QCRIL_LOG_DEBUG("Envelope command with length %d and logging_length %d ", env_length, logging_length);

  for(uint8_t index = 0; index < logging_length; index++)
  {
    QCRIL_LOG_DEBUG("0x%02X ",env_data_ptr[index]);
  }
}/* qcril_gstk_log_env_command */
