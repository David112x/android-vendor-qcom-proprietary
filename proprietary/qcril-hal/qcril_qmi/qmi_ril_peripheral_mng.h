/******************************************************************************
#  Copyright (c) 2014, 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
/******************************************************************************

  @file    qmi_ril_peripheral_mng.h
  @brief   Provides interface to communicate with peripheral manager

  DESCRIPTION

******************************************************************************/

#ifndef QCRIL_PM_H
#define QCRIL_PM_H

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================

  FUNCTION  qmi_ril_peripheral_mng_init

===========================================================================*/
/*!
    @brief
        Initialize peripheral manager client

    @return
        0 if function is successful.
*/
/*=========================================================================*/

int qmi_ril_peripheral_mng_register_pm_client_for_primary_modem(char *device_name);
int qmi_ril_peripheral_mng_register_pm_client_for_secondary_modem(char *device_name);


/*===========================================================================

  FUNCTION  qmi_ril_peripheral_mng_deinit

===========================================================================*/
/*!
    @brief
        de init peripheral manager client

    @return
        None
*/
/*=========================================================================*/
void qmi_ril_peripheral_mng_deregister_pm_client_for_primary_modem(void);
void qmi_ril_peripheral_mng_deregister_pm_client_for_secondary_modem(void);

/*===========================================================================

  FUNCTION  qmi_ril_peripheral_mng_vote

===========================================================================*/
/*!
    @brief
        Vote for registered device with peripheral manager

    @return
        0 if function is successful.
*/
/*=========================================================================*/
int qmi_ril_peripheral_mng_vote_up_primary_modem(void);
int qmi_ril_peripheral_mng_vote_up_secondary_modem(void);

/*===========================================================================

  FUNCTION  qmi_ril_peripheral_mng_release_vote

===========================================================================*/
/*!
    @brief
        Release vote from peripheral manager

    @return
        0 if function is successful.
*/
/*=========================================================================*/
void qmi_ril_peripheral_mng_vote_down_primary_modem(void);
void qmi_ril_peripheral_mng_vote_down_secondary_modem(void);

#ifdef __cplusplus
}
#endif
#endif /* QCRIL_PM_H */
