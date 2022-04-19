/******************************************************************************
#  Copyright (c) 2013, 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
/***************************************************************************************************
    @file
    util_synchronization.c

    @brief
    Implements functions supported in util_synchronization.h.

***************************************************************************************************/

#include "util_synchronization.h"
#include "util_log.h"

int qmi_shutdown_sync_thread = 0;

#if 0
static const char* util_sync_bit_field_names[] =
{
    "COND_VAR_USED",
};
#endif





/***************************************************************************************************
    @function
    util_sync_data_init

    @implementation detail
    None.
***************************************************************************************************/
void util_sync_data_init(util_sync_data_type *sync_data, util_bit_field_type sync_data_bit_field)
{
    if(sync_data)
    {
        sync_data->sync_data_bit_field = sync_data_bit_field;
    }
}

/***************************************************************************************************
    @function
    util_sync_data_destroy

    @implementation detail
    Sync object's bit field is reset.
***************************************************************************************************/
void util_sync_data_destroy(util_sync_data_type *sync_data)
{
    if(sync_data)
    {
        sync_data->sync_data_bit_field = NIL;
    }
}

/***************************************************************************************************
    @function
    util_sync_data_acquire_mutex

    @implementation detail
    None.
***************************************************************************************************/
int util_sync_data_acquire_mutex(util_sync_data_type *sync_data)
{
    int ret;

    ret = ESUCCESS;

    if(sync_data)
    {
      sync_data->sync_mutex.lock();
    }

    return ret;
}

/***************************************************************************************************
    @function
    util_sync_data_release_mutex

    @implementation detail
    None.
***************************************************************************************************/
int util_sync_data_release_mutex(util_sync_data_type *sync_data)
{
    int ret;

    ret = ESUCCESS;

    if(sync_data)
    {
      sync_data->sync_mutex.unlock();
    }

    return ret;
}

/***************************************************************************************************
    @function
    util_sync_data_wait_on_cond

    @implementation detail
    get_time_of_day is used to provide the absolute time to timed wait.
***************************************************************************************************/
int util_sync_data_wait_on_cond(util_sync_data_type *sync_data,
                                int wait_for_time_seconds)
{
    int ret;

    ret = ESUCCESS;

    if(sync_data)
    {
        if(wait_for_time_seconds)
        {
            std::chrono::seconds timeout(wait_for_time_seconds);
            sync_data->sync_cond.wait_for(sync_data->sync_mutex, timeout);
        }
        else
        {
            sync_data->sync_cond.wait(sync_data->sync_mutex);
#ifdef QMI_RIL_UTF
            if ( qmi_shutdown_sync_thread != 0 )
            {
              pthread_exit(NULL);
            }
#endif
        }
    }

    return ret;
}

/***************************************************************************************************
    @function
    util_sync_data_signal_on_cond

    @implementation detail
    None.
***************************************************************************************************/
int util_sync_data_signal_on_cond(util_sync_data_type *sync_data)
{
    int ret;

    ret = ESUCCESS;

    if(sync_data)
    {
      sync_data->sync_cond.notify_one();
    }

    return ret;
}

