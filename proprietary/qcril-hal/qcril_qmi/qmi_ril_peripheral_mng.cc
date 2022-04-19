/******************************************************************************
#  Copyright (c) 2014, 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
/******************************************************************************

  @file    qmi_ril_peripheral_mng.c
  @brief   Provides interface to communicate with peripheral manager

  DESCRIPTION

******************************************************************************/

#include "qmi_ril_peripheral_mng.h"
#include "qcril_log.h"

#ifndef QMI_RIL_UTF
#include "pm-service.h"
#include "mdm_detect.h"
#endif

#define QMI_RIL_PM_QCRIL_CLIENT_NAME  "QCRIL"

typedef struct
{
    /* reference returned by register() */
    void           *id;

#ifndef QMI_RIL_UTF
    /* peripehral device name */
    char      device_name[MAX_NAME_LEN];
#endif

} qmi_ril_peripheral_conn_info;

qmi_ril_peripheral_conn_info qmi_ril_peripheral_mng_conn_info[2];
bool is_secondary_modem_online = false;

#ifndef QMI_RIL_UTF
int qmi_ril_peripheral_mng_init(char *device_name,
                                qmi_ril_peripheral_conn_info *conn_info,
                                enum pm_event *event,
                                pm_client_notifier notifier);
void qmi_ril_peripheral_mng_deinit(qmi_ril_peripheral_conn_info *conn_info);
int qmi_ril_peripheral_mng_vote(qmi_ril_peripheral_conn_info *conn_info);
void qmi_ril_peripheral_mng_release_vote(qmi_ril_peripheral_conn_info *conn_info);

/*===========================================================================

  FUNCTION  qmi_ril_pm_event_notifier

===========================================================================*/
/*!
    @brief
        peripheral manager event call back

    @return
        None
*/
/*=========================================================================*/
void qmi_ril_pm_event_notifier_primary_modem(void *client_cookie,
                                             enum pm_event event)
{
    qmi_ril_peripheral_conn_info *info = static_cast<qmi_ril_peripheral_conn_info*>(client_cookie);

    pm_client_event_acknowledge(info->id, event);

    switch (event)
    {
        case EVENT_PERIPH_GOING_OFFLINE:
            QCRIL_LOG_INFO("Primary modem [%s] is going offline",
                           info->device_name);
            break;

        case EVENT_PERIPH_IS_OFFLINE:
            QCRIL_LOG_INFO("Primary modem [%s] is offline",
                           info->device_name);
            break;

        case EVENT_PERIPH_GOING_ONLINE:
            QCRIL_LOG_INFO("Primary modem [%s] is going online",
                           info->device_name);
            break;

        case EVENT_PERIPH_IS_ONLINE:
            QCRIL_LOG_INFO("Primary modem [%s] is online",
                           info->device_name);
            break;

        default:
            QCRIL_LOG_INFO("Invalid event for primary modem [%s]",
                           info->device_name);
            break;
    }
}

void qmi_ril_pm_event_notifier_secondary_modem(void *client_cookie,
                                               enum pm_event event)
{
    qmi_ril_peripheral_conn_info *info = static_cast<qmi_ril_peripheral_conn_info*>(client_cookie);

    pm_client_event_acknowledge(info->id, event);

    switch (event)
    {
        case EVENT_PERIPH_GOING_OFFLINE:
            QCRIL_LOG_INFO("Secondary modem [%s] is going offline",
                           info->device_name);
            break;

        case EVENT_PERIPH_IS_OFFLINE:
            QCRIL_LOG_INFO("Secondary modem [%s]  is offline",
                           info->device_name);
            is_secondary_modem_online = false;
            break;

        case EVENT_PERIPH_GOING_ONLINE:
            QCRIL_LOG_INFO("Secondary modem [%s] going online",
                           info->device_name);
            break;

        case EVENT_PERIPH_IS_ONLINE:
            is_secondary_modem_online = true;
            QCRIL_LOG_INFO("Secondary modem [%s] is online",
                           info->device_name);
            break;

        default:
            QCRIL_LOG_INFO("Invalid event for secondary modem [%s]",
                           info->device_name);
            break;
    }
}

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
int qmi_ril_peripheral_mng_init(char *device_name,
                                qmi_ril_peripheral_conn_info *conn_info,
                                enum pm_event *event,
                                pm_client_notifier notifier)
{
    int ret = RIL_E_SUCCESS;
#ifndef QMI_RIL_UTF
    int ret_code;

    QCRIL_LOG_FUNC_ENTRY();

    if (!conn_info)
    {
        return RIL_E_GENERIC_FAILURE;
    }

    memset(conn_info, 0, sizeof(*conn_info));
    strlcpy(conn_info->device_name,
            device_name, sizeof(conn_info->device_name));

    ret_code = pm_client_register(notifier,
                                  conn_info,
                                  conn_info->device_name,
                                  QMI_RIL_PM_QCRIL_CLIENT_NAME,
                                  event,
                                  &conn_info->id);

    if (ret_code != PM_RET_SUCCESS)
    {
        QCRIL_LOG_ERROR("Failed to register for %s\n",
                        conn_info->device_name);
        ret = RIL_E_GENERIC_FAILURE;
        QCRIL_LOG_ESSENTIAL("QCRIL_ERROR:INIT: unable to register with error: %d", ret);
    }
    else
    {
        QCRIL_LOG_INFO("Successful registration\n");
    }

#else
    ret = RIL_E_GENERIC_FAILURE;
#endif
    QCRIL_LOG_FUNC_RETURN();
    return ret;
}

int qmi_ril_peripheral_mng_register_pm_client_for_primary_modem(char *device_name)
{
    enum pm_event event;
    return qmi_ril_peripheral_mng_init(device_name,
                                       &qmi_ril_peripheral_mng_conn_info[0],
                                       &event,
                                       qmi_ril_pm_event_notifier_primary_modem);
}

int qmi_ril_peripheral_mng_register_pm_client_for_secondary_modem(char *device_name)
{
    enum pm_event event;
    int ret_code = qmi_ril_peripheral_mng_init(device_name,
                                               &qmi_ril_peripheral_mng_conn_info[1],
                                               &event,
                                               qmi_ril_pm_event_notifier_secondary_modem);

    if (ret_code == PM_RET_SUCCESS)
    {
        switch (event)
        {
            case EVENT_PERIPH_IS_OFFLINE:
                QCRIL_LOG_INFO("%s is offline", device_name);
                is_secondary_modem_online = false;
                break;

            case EVENT_PERIPH_IS_ONLINE:
                QCRIL_LOG_INFO("%s is online", device_name);
                is_secondary_modem_online = true;
                break;

            default:
                QCRIL_LOG_INFO("invalid event for %s", device_name);
                break;
        }
    }

    return ret_code;
}

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
void qmi_ril_peripheral_mng_deinit(qmi_ril_peripheral_conn_info *conn_info)
{
    QCRIL_LOG_FUNC_ENTRY();

#ifndef QMI_RIL_UTF
    if (conn_info && conn_info->id)
    {
        pm_client_unregister(conn_info->id);
    }
#endif

    QCRIL_LOG_FUNC_RETURN();
}

void qmi_ril_peripheral_mng_deregister_pm_client_for_primary_modem(void)
{
    qmi_ril_peripheral_mng_deinit(&qmi_ril_peripheral_mng_conn_info[0]);
}

void qmi_ril_peripheral_mng_deregister_pm_client_for_secondary_modem(void)
{
    qmi_ril_peripheral_mng_deinit(&qmi_ril_peripheral_mng_conn_info[1]);
}

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
int qmi_ril_peripheral_mng_vote(qmi_ril_peripheral_conn_info *conn_info)
{
    int ret = RIL_E_GENERIC_FAILURE;

    QCRIL_LOG_FUNC_ENTRY();

#ifndef QMI_RIL_UTF
    if (conn_info && conn_info->id)
    {
        ret = pm_client_connect(conn_info->id);
        if (ret)
        {
            QCRIL_LOG_ERROR("%s connect failed %d", conn_info->device_name,
                                                    ret);
        }
    }
#endif

    QCRIL_LOG_FUNC_RETURN();
    return ret;
}

int qmi_ril_peripheral_mng_vote_up_primary_modem(void)
{
    return qmi_ril_peripheral_mng_vote(&qmi_ril_peripheral_mng_conn_info[0]);
}

int qmi_ril_peripheral_mng_vote_up_secondary_modem(void)
{
    return qmi_ril_peripheral_mng_vote(&qmi_ril_peripheral_mng_conn_info[1]);
}

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
void qmi_ril_peripheral_mng_release_vote(qmi_ril_peripheral_conn_info *conn_info)
{
    int ret;

    QCRIL_LOG_FUNC_ENTRY();

#ifndef QMI_RIL_UTF
    if (conn_info && conn_info->id)
    {
        ret = pm_client_disconnect(conn_info->id);
        if (ret)
        {
            QCRIL_LOG_ERROR("%s disconnect failed %d", conn_info->device_name,
                                                       ret);
        }
    }
#endif

    QCRIL_LOG_FUNC_RETURN();
    return;
}

void qmi_ril_peripheral_mng_vote_down_primary_modem(void)
{
    qmi_ril_peripheral_mng_release_vote(&qmi_ril_peripheral_mng_conn_info[0]);
}

void qmi_ril_peripheral_mng_vote_down_secondary_modem(void)
{
    qmi_ril_peripheral_mng_release_vote(&qmi_ril_peripheral_mng_conn_info[1]);
}

bool qmi_ril_peripheral_mng_is_secondary_modem_online(void)
{
    return is_secondary_modem_online;
}
#endif
