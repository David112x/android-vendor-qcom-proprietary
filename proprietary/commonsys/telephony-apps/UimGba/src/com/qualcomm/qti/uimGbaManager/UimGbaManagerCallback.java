/******************************************************************************
  @file    uimGbaManagerCallback.java
  @brief   uimGba asyncronous response callback definition. Client code can
           override the same and pass it to uimGbaManager.

  ---------------------------------------------------------------------------
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/

package com.qualcomm.qti.uimGbaManager;

import com.qualcomm.qti.uimGba.IUimGbaServiceCallback;

public class UimGbaManagerCallback extends IUimGbaServiceCallback.Stub {
    /**
     * @param token
     *    Used to identify the request to which respone corresponds to.
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
     *    KsNaf Response
     *
     * @param bootstrappingId
     *    Bootstrapping ID
     *
     * @param lifeTime
     *    KS lifetime in UTC format. Tailing "Z" indicates GMT.
     *    See ISO 8601.
     * @return
     *    None
     */
    public void gbaInitResponse(int token, int responseCode, boolean ksnafType,
                                byte[] KsNafResponse, String bootstrappingId,
                                String lifeTime) {
        // Dummy function
    }

    /**
     * @param token
     *    Used to identify the request to which respone corresponds to.
     *
     * @param responseCode
     *    UIM_GBA_SUCCESS         = 0;
     *    UIM_GBA_GENERIC_FAILURE = 1;
     *
     * @param isEncrypted
     *    false indicates the IMPI value is not in encrypted form.
     *    true indicates that IMPI value is encrypted from.
     *
     * @param ImpiData
     *    IMPI value
     *
     * @return
     *    None
     */
     public void getImpiResponse(int token, int responseCode, boolean isEncrypted,
                                 byte[] ImpiData) {
         //Dummy function
     }
}
