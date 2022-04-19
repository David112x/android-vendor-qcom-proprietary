/******************************************************************************
#  Copyright (c) 2008-2010, 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
/******************************************************************************
  @file    qcril_pbm.h
  @brief   qcril qmi - PB

  DESCRIPTION

******************************************************************************/


#ifndef QCRIL_PBM_H
#define QCRIL_PBM_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include "qcrili.h"
#include "qmi_client.h"
#include "qcril_qmi_client.h"

#include "interfaces/pbm/pbm.h"
#include "interfaces/pbm/QcRilRequestGetAdnRecordMessage.h"
#include "interfaces/pbm/QcRilRequestUpdateAdnRecordMessage.h"
#include "interfaces/pbm/QcRilUnsolAdnRecordsOnSimMessage.h"
#include "interfaces/pbm/QcRilUnsolAdnInitDoneMessage.h"

#include "interfaces/nas/nas_types.h"
/*===========================================================================

                        DEFINITIONS AND TYPES

===========================================================================*/

typedef enum
{
  QCRIL_PBM_DEVICE_TYPE_ECC,
  QCRIL_PBM_DEVICE_TYPE_FDN
} qcril_pbm_device_type;

#define NON_STD_OTASP_NUMBER "*228"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================

                    EXTERNAL FUNCTION PROTOTYPES

===========================================================================*/
void qcril_qmi_pbm_pre_init(void);

void qcril_qmi_pbm_cleanup(void);

void qcril_qmi_pbm_init(void);

int qmi_ril_phone_number_is_emergency(char * number_to_check);

int qmi_ril_phone_number_is_non_std_otasp(const char * number_to_check);

int qmi_ril_number_received_from_pbm(char * number);

int qcril_qmi_pbm_centralized_ecc_support_available(void);

RIL_Errno qcril_qmi_pbm_enable_emergency_number_indications(int enable);

void qmi_ril_fill_ecc_map
(
    char* ecc_numbers,
    qcril::interfaces::RIL_EccMapType& ecc_map,
    const qcril::interfaces::RIL_EccNumberSourceMask sources,
    const char *mcc,
    const char *mnc
);

RIL_Errno qmi_ril_send_ecc_list_indication();

int qmi_ril_set_ecc_property();

void qmi_ril_set_alternate_emergency_call_info(char *number, uint32_t emer_cat);

void qmi_ril_reset_alternate_emergency_call_info();

boolean qmi_ril_is_alternate_emergency_call(char *number);

uint32_t qmi_ril_get_alternate_emergency_call_category();

void qcril_qmi_pbm_unsolicited_indication_cb_helper
(
  unsigned int   msg_id,
  unsigned char *decoded_payload,
  uint32_t       decoded_payload_len
);

RIL_Errno qcril_qmi_pbm_enable_phonebook_ready_indications(int enable);

void qcril_qmi_pbm_get_pb_capacity_hndlr
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_pbm_get_adn_record_hndlr
(
    std::shared_ptr<QcRilRequestGetAdnRecordMessage> msg
);

void qcril_qmi_pbm_update_adn_record_hndlr
(
    std::shared_ptr<QcRilRequestUpdateAdnRecordMessage> msg
);

#ifdef QMI_RIL_UTF
void qcril_qmi_hal_pbm_module_cleanup();
#endif

#ifdef __cplusplus
}
#endif
#endif /* QCRIL_PBM_H */
