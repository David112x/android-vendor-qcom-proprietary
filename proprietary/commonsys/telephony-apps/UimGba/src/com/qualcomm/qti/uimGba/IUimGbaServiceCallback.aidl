/******************************************************************************
 * @file    IUimGbaServiceCallback.aidl
* @brief    This interface describes the APIs for the callback that a client
 *          which uses IUimGbaService should implement in order to
 *          be notified of asynchronous responses.
 *
 * @version 00.00.01
 *
 * ---------------------------------------------------------------------------
 * Copyright (c) 2014, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * ---------------------------------------------------------------------------
 *
 ******************************************************************************/

 package com.qualcomm.qti.uimGba;

 interface IUimGbaServiceCallback {

    /**
     * gbaInitResponse
     *
     * @param token
     *    This indicates the request for which this response corresponds to.
     *
     * @param responseCode
     *    UIM_GBA_SUCCESS         = 0;
     *    UIM_GBA_GENERIC_FAILURE = 1;
     *
     * @param ksnafType
     *    0 indicates that KsNafResponse is in non-encrypted form
     *    1 indicates that KsNafResponse is in encrypted form
     *
     * @param KsNafResponse
     *    KS NAF in Base64 format. See 3GPP TS 33.220 4.5.2.
     *
     * @param bootstrappingId
     *    Transaction ID obtained as part of Bootstrapping with BSF
     *
     * @param lifeTime
     *    KS lifetime in UTC format. Tailing "Z" indicates GMT.
     *    See ISO 8601.
     *
     */
    void gbaInitResponse(in int token, in int responseCode, boolean ksnafType,
                         in byte[] KsNafResponse, in String bootstrappingId,
                         in String lifeTime);

    /**
     * getImpiResponse
     *
     * @param token
     *    This indicates the request for which this response corresponds to.
     *
     * @param responseCode
     *    UIM_GBA_SUCCESS         = 0;
     *    UIM_GBA_GENERIC_FAILURE = 1;
     *
     * @param isEncrypted
     *    false indicates the IMPI value is not in encrypted form.
     *    true indicates that IMPI value is in encrypted form.
     *
     * @param ImpiData
     *    IMPI data value
     */
    void getImpiResponse(in int token, in int responseCode, boolean isEncrypted,
                         in byte[] ImpiData);
 }
