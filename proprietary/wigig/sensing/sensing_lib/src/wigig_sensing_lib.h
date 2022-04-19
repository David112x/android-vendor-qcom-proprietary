#ifndef WIGIG_SENSING_LIB_H
#define WIGIG_SENSING_LIB_H

/******************************************************************************
  @file    wigig_sensing_lib.h
  @brief   The WiGig Sensing QMI library Header file.

  DESCRIPTION
  WiGig Sensing QMI client routines.
  WiGig Sensing clients will be built on top of these routines for initializing,
  sending QMI Sensing messages and receiving responses / indications.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  wigig_sensing_lib_register() needs to be called before sending or receiving any
  service specific messages

  ---------------------------------------------------------------------------
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
*******************************************************************************/

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**  maximum size of burst payload  */
#define WIGIG_SENSING_BURST_MAX_SIZE 30720

/**  minimum size of a burst:
8 DWORDs for header, 1 DWORD for meta, 3 DWORDs for CIR  */
#define WIGIG_SENSING_BURST_MIN_SIZE (12 * 4)

/**  maximum size of parameters payload  */
#define WIGIG_SENSING_PARAMETERS_MAX_SIZE 9216

#define CYCLIC_BUF_MAX_SIZE (8 * 1024 * 1024)

typedef enum {
    WIGIG_SENSING_LIB_SEARCH_MODE = 1,
    WIGIG_SENSING_LIB_FACIAL_MODE = 2,
    WIGIG_SENSING_LIB_GESTURE_MODE = 3,
    WIGIG_SENSING_LIB_STOP_MODE = 7,
    WIGIG_SENSING_LIB_CUSTOME_MODE = 15,
} wigig_sensing_lib_mode_type;

typedef enum {
    WIGIG_SENSING_CHANNEL_1   = 0x00,
    WIGIG_SENSING_CHANNEL_2   = 0x01,
    WIGIG_SENSING_CHANNEL_3   = 0x02,
    WIGIG_SENSING_CHANNEL_4   = 0x03,
    WIGIG_SENSING_CHANNEL_5   = 0x04,
    WIGIG_SENSING_CHANNEL_6   = 0x05,
    WIGIG_SENSING_CHANNEL_9   = 0x06,
    WIGIG_SENSING_CHANNEL_10  = 0x07,
    WIGIG_SENSING_CHANNEL_11  = 0x08,
    WIGIG_SENSING_CHANNEL_12  = 0x09,
} wigig_sensing_channel;

#define WIGIG_SENSING_LIB_NO_ERR                0
#define WIGIG_SENSING_LIB_INTERNAL_ERR          (-1)
#define WIGIG_SENSING_LIB_SERVICE_ERR           (-2)
#define WIGIG_SENSING_LIB_TIMEOUT_ERR           (-3)
#define WIGIG_SENSING_LIB_SENSING_DISABLED      (-4)
#define WIGIG_SENSING_LIB_PORT_NOT_OPEN_ERR     (-5)
#define WIGIG_SENSING_LIB_MEMORY_ERROR          (-6)
#define WIGIG_SENSING_LIB_TRANSPORT_ERR         (-7)
#define WIGIG_SENSING_LIB_PARAM_ERR             (-8)
#define WIGIG_SENSING_LIB_INVALID_CLNT          (-9)
#define WIGIG_SENSING_LIB_CLIENT_FW_NOT_UP      (-10)
#define WIGIG_SENSING_LIB_XPORT_BUSY_ERR        (-11)
#define WIGIG_SENSING_LIB_SENSING_FW_RESET      (-12)
#define WIGIG_SENSING_LIB_NO_DATA               (-13)
#define WIGIG_SENSING_LIB_INVALID_BURST_SIZE    (-14)
#define WIGIG_SENSING_LIB_MUTEX_ERR             (-15)

typedef int    wigig_sensing_lib_error_type;
typedef struct wigig_sensing_lib_client_struct *wigig_sensing_lib_client_type;

typedef struct
{
    uint32_t    fw_magic_number;
    uint32_t    tsf_interrupt_posedge;
    uint32_t    tsf_first_pulse;
    uint16_t    Tp;
    uint16_t    Np;
    uint16_t    burst_index;
    uint8_t     Ns;
    uint8_t     md_size;
    uint8_t     channel;
    uint8_t     *reserved[11];
} wigig_sensing_burst_header;

/*===========================================================================
  FUNCTION  wigig_sensing_lib_register
===========================================================================*/
/*!
@brief

  This function is used for registering to WiGig Sensing service.
  Only one client can register at a time.

@param[in]   notify_cb       notification callback function
@param[in]   data_ready_cb   data ready callback function
@param[out]  user_handle     Handle used by the infrastructure to identify the
                             registered client.
@param[in]   auto_recovery  Flag for auto recovery handling in case of sensing error.
                            When auto_recovery is set and sensing was in search mode
                            prior to the error, the mode will be automatically
                            set to search mode after the recovery. In all other modes
                            the mode after the recovery will be set to stop mode and it
                            is the user responsibility to switch to any desired mode.

@return
  Sets the user_handle and returns WIGIG_SENSING_LIB_NO_ERR if successful,
  error code if not successful
  WIGIG_SENSING_LIB_SERVICE_ERR if QMI server cannot be reached

@note

  - Dependencies
    - None

  - Side Effects
    - QMI connection is opened with the service
*/
/*=========================================================================*/
wigig_sensing_lib_error_type
wigig_sensing_lib_register
(
    wigig_sensing_lib_client_type   *user_handle,
    bool                            auto_recovery
);

/*===========================================================================
  FUNCTION  wigig_sensing_lib_change_mode
===========================================================================*/
/*!
@brief

  This function is used for changing the WiGig Sensing mode.

@param[in]  user_handle         Handle used by the infrastructure to
                                identify the registered client.
@param[in]  mode                The new sensing mode
@param[in]  channel             Default channel recommended by the user for sensing.
                                Sensing FW can decide to use a different channel.
                                The chosen channel is updated in the header section of each
                                burst.
                                When switching to stop mode, Channel number, if set,
                                shall be ignored.
@param[out] burst_size          Burst size matches to the new sensing mode.
                                The burst size shall be used by the sensing user
                                for buffer allocation for wigig_sensing_lib_read_data.
                                During stop mode burst_size shall be ignored.

@return
  Sets burst_size and returns WIGIG_SENSING_LIB_NO_ERR if successful,
  error code if not successful
  WIGIG_SENSING_LIB_SERVICE_ERR if QMI server cannot be reached

@note

  - Dependencies
    - None

  - Side Effects
    - All the unread data recieved in the previous mode is flushed.
*/
/*=========================================================================*/
wigig_sensing_lib_error_type
wigig_sensing_lib_change_mode
(
    wigig_sensing_lib_client_type   user_handle,
    wigig_sensing_lib_mode_type     mode,
    uint32_t                        channel,
    uint32_t                        *burst_size
);

/*===========================================================================
  FUNCTION  wigig_sensing_lib_unregister
===========================================================================*/
/*!
@brief

  This function is used to unregister client from the lib.

@param[in]  user_handle         Handle used by the infrastructure to
                                identify the registered client.

@return
  Sets the user_handle and returns WIGIG_SENSING_LIB_NO_ERR if successful,
  error code if not successful
  WIGIG_SENSING_LIB_SERVICE_ERR if QMI server cannot be reached

@note

  - Dependencies
    - None

  - Side Effects
    - all data structures, including callbacks, handle and flags are zeroed
*/
/*=========================================================================*/
wigig_sensing_lib_error_type
wigig_sensing_lib_unregister
(
    wigig_sensing_lib_client_type   user_handle
);

/*===========================================================================
  FUNCTION  wigig_sensing_lib_get_parameters
===========================================================================*/
/*!
@brief

  This function is used for getting the sensing parameters from the sensing FW.
  This function can be called only when sensing mode is set to "stop".

@param[in]  user_handle         Handle used by the infrastructure to
                                identify the registered client.
@param[out] params_buf          Pointer to a buffer for the returned parameters
@param[in]  params_buf_len      Length of the parameters buffer
@param[out] params_recv_len     Length of the received parameters

@return
  Copies the parameters data to params_buf, sets data length in params_recv_len and
  returns WIGIG_SENSING_LIB_NO_ERR if successful,
  error code if not successful
  WIGIG_SENSING_LIB_SERVICE_ERR if QMI server cannot be reached

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
wigig_sensing_lib_error_type
wigig_sensing_lib_get_parameters
(
    wigig_sensing_lib_client_type   user_handle,
    void                            *params_buf,
    uint32_t                        params_buf_len,
    uint32_t                        *params_recv_len
);


/*===========================================================================
  FUNCTION  wigig_sensing_lib_send_debug_msg
===========================================================================*/
/*!
@brief
  Sends a synchronous QMI service message on the specified connection.
  This function expects the user to encode the message before sending
  and decode the message after receiving.

@param[in]   user_handle        Handle used by the infrastructure to
                                identify the registered client.
@param[in]   reg_q              Request ID
@param[in]   data_valid         Flag to use debug input data
@param[in]   req_buf            Pointer to the request
@param[in]   req_buf_len        Length of the request
@param[out]  resp_buf           Pointer to where the response will be stored
@param[in]   resp_buf_len       Length of the response buffer
@param[out]  resp_buf_recv_len  Length of the response received
@param[in]   timeout_msecs      Time-out in milliseconds. 0 = no timeout

@return
  Copies the response to resp_buf, sets resp_buf_recv_len and returns
  WIGIG_SENSING_LIB_NO_ERR if function is successful,
  error code otherwise.
  WIGIG_SENSING_LIB_SERVICE_ERR if QMI server cannot be reached

@note
  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
wigig_sensing_lib_error_type
wigig_sensing_lib_send_debug_msg
(
    wigig_sensing_lib_client_type   user_handle,
    uint32_t                        req_id,
    bool                            data_valid,
    void                            *req_buf,
    uint32_t                        req_buf_len,
    void                            *resp_buf,
    uint32_t                        resp_buf_len,
    uint32_t                        *resp_buf_recv_len,
    uint32_t                        timeout_msecs
);

/*===========================================================================
  FUNCTION  wigig_sensing_lib_read_data
===========================================================================*/
/*!
@brief
  Read sensing data.
  This function can be called either in polling mode or after
  wigig_sensing_lib_data_ready_cb is called.
  The function returns as many bursts as possible and their DRI TSF, taking
  into account dri_tsf_arr_size and data_buf_len.

@param[in]   user_handle        Handle used by the infrastructure to
                                identify the registered client.
@param[out]  data_buf           Pointer to the buffer the data should be stored at
@param[in]   data_buf_len       Length of the data buffer
@param[out]  data_buf_recv_len  Length of the data received
@param[out]  dri_tsf_arr        Array of Time-Stamps of "data ready DRI" reception
                                in SLPI, for each of the bursts included in the data buffer
@param[in]   dri_tsf_arr_size   Size of dri_tsf_arr
@param[out]  num_of_bursts      Number of bursts included in the data buffer. User shall use
                                this number to get dri_tsf for each burst from dri_tsf_arr
@param[out]  dropped_cnt_from_last_read Number of bursts that were dropped since
                                        the last read
@param[out]  remaining_bursts   Number of remaining bursts available for read

@return
  Copies the data to data_buf, sets data_buf_recv_len and returns
  WIGIG_SENSING_LIB_NO_ERR if function is successful,
  error code otherwise.
  WIGIG_SENSING_LIB_SERVICE_ERR if QMI server cannot be reached

@note
  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
wigig_sensing_lib_error_type
wigig_sensing_lib_read_data
(
    wigig_sensing_lib_client_type   user_handle,
    void                            *data_buf,
    uint32_t                        data_buf_len,
    uint32_t                        *data_buf_recv_len,
    uint64_t                        *dri_tsf_arr,
    uint32_t                        dri_tsf_arr_size,
    uint32_t                        *num_of_bursts
);

wigig_sensing_lib_error_type
wigig_sensing_lib_read_stats
(
    wigig_sensing_lib_client_type   user_handle,
    uint32_t                        *dropped_cnt_from_last_read,
    uint32_t                        *remaining_bursts
);

/*===========================================================================
  FUNCTION  wigig_sensing_lib_get_num_avail_bursts
===========================================================================*/
/*!
@brief
  Get the number of bursts available for read.
  The actual number of available bursts can be changed until the data is read
  due to incoming data or sensing buffer overflow (in which old data is flushed).
  This function can be called either in polling mode or after recieving
  WIGIG_SENSING_QMI_SENSING_DATA_READY_EVT notification.

@param[in]   user_handle        Handle used by the infrastructure to
                                identify the registered client.
@param[out]  num_avail_bursts   Number of bursts available for read.

@return
  Sets num_avail_bursts and returns WIGIG_SENSING_LIB_NO_ERR if function is
  successful, error code otherwise.
  WIGIG_SENSING_LIB_SERVICE_ERR if QMI server cannot be reached

@note
  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
wigig_sensing_lib_error_type
wigig_sensing_lib_get_num_avail_bursts
(
    wigig_sensing_lib_client_type   user_handle,
    uint32_t                        *num_avail_bursts
);

/*===========================================================================
  FUNCTION  wigig_sensing_lib_get_mode
===========================================================================*/
/*!
@brief
  Get the current mode from SLPI driver.
  Lib is not maintaining the current mode thus request has to be sent
  over QMI.

@param[in]   user_handle        Handle used by the infrastructure to
                                identify the registered client.
@param[out]  mode               Current sensing mode

@return
  Sets num_avail_bursts and returns WIGIG_SENSING_LIB_NO_ERR if function is
  successful, error code otherwise.
  WIGIG_SENSING_LIB_SERVICE_ERR if QMI server cannot be reached

@note
  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
wigig_sensing_lib_error_type
wigig_sensing_lib_get_mode
(
    wigig_sensing_lib_client_type   user_handle,
    wigig_sensing_lib_mode_type     *mode
);

/*===========================================================================
  FUNCTION  wigig_sensing_lib_translate_error
===========================================================================*/
/*!
@brief
  Translates a sensing lib error code to a descriptive sentence to the user.
                                identify the registered client.
@param[in]  lib_error          Sensing lib error code

@return
  Returns the description of the error code.

@note
  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
extern const char*
wigig_sensing_lib_translate_error
(
    uint32_t                        lib_error
);

/*===========================================================================
  FUNCTION  wigig_sensing_lib_clear_data
===========================================================================*/
/*!
@brief
  Clears the previous sensing data.
  This function can be called without changing to stop mode,
  it will release the previous saved data in the buffer.

@param[in]   user_handle        Handle used by the infrastructure to
                                identify the registered client.

@return
  WIGIG_SENSING_LIB_NO_ERR if function is successful,
  error code otherwise.
  WIGIG_SENSING_LIB_SERVICE_ERR if QMI server cannot be reached

@note
  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
wigig_sensing_lib_error_type
wigig_sensing_lib_clear_data
(
    wigig_sensing_lib_client_type   user_handle
);

/*===========================================================================
  FUNCTION  wigig_sensing_lib_read_stats
===========================================================================*/
/*!
@brief
  Gets data statistics: dropped bursts since last read and number of
  remaining bursts.

@param[in]   user_handle        Handle used by the infrastructure to
                                identify the registered client.
@param[out]  dropped_cnt_from_last_read        Dropped bursts from last read
@param[out]  remaining_bursts        Number of remaining bursts

@return
  WIGIG_SENSING_LIB_NO_ERR if function is successful,
  error code otherwise.

@note
  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
wigig_sensing_lib_error_type
wigig_sensing_lib_read_stats
(
    wigig_sensing_lib_client_type   user_handle,
    uint32_t                        *dropped_cnt_from_last_read,
    uint32_t                        *remaining_bursts
);

/*===========================================================================
  FUNCTION  wigig_sensing_lib_set_timeout
===========================================================================*/
/*!
@brief
  Sets the timeout time for how long to wait before getting expected event

@param[in]   user_handle        Handle used by the infrastructure to
                                identify the registered client.
@param[in]   timeout_ms         Timeout in miliseconds

@return
  WIGIG_SENSING_LIB_NO_ERR if function is successful,
  error code otherwise.

@note
  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
wigig_sensing_lib_error_type
wigig_sensing_lib_set_timeout(
    wigig_sensing_lib_client_type   user_handle,
    int                             timeout_ms
);

/*===========================================================================
  FUNCTION  wigig_sensing_lib_cancel_read
===========================================================================*/
/*!
@brief
  Cancels the blocking read. Calling will result in wigig_sensing_lib_read_data()
  to exit immediately.
  Caller should unregister only after wigig_sensing_lib_read_data() exits.

@param[in]   user_handle        Handle used by the infrastructure to
                                identify the registered client.

@return
  WIGIG_SENSING_LIB_NO_ERR if function is successful,
  error code otherwise.

@note
  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
wigig_sensing_lib_error_type
wigig_sensing_lib_cancel_read(
    wigig_sensing_lib_client_type   user_handle
);


#ifdef __cplusplus
}
#endif

#endif
