#ifndef QCRIL_GSTK_LOG_H
#define QCRIL_GSTK_LOG_H
/*===========================================================================

  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

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
);

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
);

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
);

#endif /* QCRIL_GSTK_LOG_H */

