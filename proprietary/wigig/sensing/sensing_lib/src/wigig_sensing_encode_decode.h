/******************************************************************************
  @file    wigig_sensing_encode_decode.h
  @brief   The WiGig Sensing QMI Library Protobuf Handling Header file.

  DESCRIPTION
  APIs for wigig sensing protobuf handling

  ---------------------------------------------------------------------------
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
*******************************************************************************/

#pragma once
#include "pb_decode.h"
#include "pb_encode.h"

#include "sns_wigig.pb.h"
#include "sns_suid.pb.h"
#include "sns_client.pb.h"
#include "wigig_sensing_process.h"
#include "sns_std_sensor.pb.h"

/**
 * Utility structure used to encode a string or already encoded message
 * into another Protocol Buffer message.
 */
typedef struct sns_buffer_arg
{
    /* Buffer to be written, or reference to read buffer */
    void const *buf;
    /* Length of buf */
    size_t buf_len;
} sns_buffer_arg;

/**
 * Copy an already encoded payload into a message field.  For the purpose of
 * this function, an encoded submessage is identical to a character string.
 */
bool wigig_sensing_encode_payload
(
    pb_ostream_t *stream,
    const pb_field_t *field,
    void *const *arg
);

/**
 * Create a SUID lookup request for the specified data type.
 * Allocates memory for the encoded message encoded_req.
 * encoded_req needs to be free later after usage.
 *
 * @param[i] data_type Sensor type to search for
 * @param[o] encoded_req Generated encoded request
 *
 * @return Length of encoded_req
 */
size_t wigig_sensing_get_encoded_suid_req
(
    char const *data_type,
    void **encoded_req
);

/**
 * Create a set recovery request.
 * Allocates memory for the encoded message encoded_req.
 * encoded_req needs to be free later after usage.
 *
 * @param[o] encoded_req Generated encoded request
 *
 * @return Length of encoded_req
 */
size_t wigig_sensing_get_encoded_auto_recovery_req
(
    void **encoded_req
);

/**
 * Create a change mode request.
 * Allocates memory for the encoded message encoded_req.
 * encoded_req needs to be free later after usage.
 *
 * @param[o] encoded_req Generated encoded request
 *
 * @return Length of encoded_req
 */
size_t wigig_sensing_get_encoded_change_mode_req
(
    void **encoded_req,
    sns_wigig_mode mode,
    uint32_t channel
);

/**
 * Create a get parameters request.
 * Allocates memory for the encoded message encoded_req.
 * encoded_req needs to be free later after usage.
 *
 * @param[o] encoded_req Generated encoded request
 *
 * @return Length of encoded_req
 */
size_t wigig_sensing_get_encoded_get_params_req
(
    void **encoded_req
);

/**
 * Create a debug request.
 * Allocates memory for the encoded message encoded_req.
 * encoded_req needs to be free later after usage.
 *
 * @param[o] encoded_req Generated encoded request
 *
 * @return Length of encoded_req
 */
size_t wigig_sensing_get_encoded_debug_req
(
    void **encoded_req,
    uint32_t req_id,
    void *req_buf,
    uint32_t buf_len
);

/**
 * Create a get mode request.
 * Allocates memory for the encoded message encoded_req.
 * encoded_req needs to be free later after usage.
 *
 * @param[o] encoded_req Generated encoded request
 *
 * @return Length of encoded_req
 */
size_t wigig_sensing_get_encoded_get_mode_req
(
    void **encoded_req
);

/*
 * Decode an element of sns_client_event_msg::events.  This function will be
 * called by nanopb once per each element in the array.
 *
 * @param[i] arg Sensor-specific decode function
 */
bool wigig_sensing_decode_events
(
    pb_istream_t *stream,
    const pb_field_t *field,
    void **arg
);

/**
 * Decode the payload of the event message, i.e. the SUID Event
 *
 * @param[i] arg Message ID
 */
bool wigig_sensing_decode_suid_event
(
    pb_istream_t *stream,
    const pb_field_t *field,
    void **arg
);

/**
 * Decode an event message received from the wigig sensing Sensor.
 */
bool wigig_sensing_decode_sensing_events
(
    pb_istream_t *stream,
    const pb_field_t *field,
    void **arg
);