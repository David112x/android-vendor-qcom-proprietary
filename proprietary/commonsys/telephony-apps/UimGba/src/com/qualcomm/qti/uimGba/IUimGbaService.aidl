/******************************************************************************
 * @file    IUimGbaService.aidl
 * @brief   This interface provides the APIs for GBA
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

import com.qualcomm.qti.uimGba.IUimGbaServiceCallback;

interface IUimGbaService {
     /**
     * registerCallback will be used by a client to register a callback to be
     * notified asynchronously
     *
     * @param cb
     *    Defines the callback interface
     *
     * @return
     *    UIM_GBA_SUCCESS         = 0;
     *    UIM_GBA_GENERIC_FAILURE = 1;
     */
    int registerCallback(in IUimGbaServiceCallback cb);

    /**
     * deregisterCallback will be used by a client to deregister a callback to
     * be notified asynchronously
     *
     * @param cb
     *    Defines the callback interface
     *
     * @return
     *    UIM_GBA_SUCCESS          = 0;
     *    UIM_GBA_GENERIC_FAILURE  = 1;
     */
    int deregisterCallback(in IUimGbaServiceCallback cb);

    /**
     * gbaInit will be used by client to perform bootstrapping procedures(GBA)
     * to generate keys.
     *
     * @param token
     *    This is used for identification of the request.
     *
     * @param securityProtocol
     *    Security protocol identifier NULL terminated string of 5 octets.
     *    See 3GPP TS 33.220 Annex H.
     *
     * @param nafFullyQualifiedDomainName
     *    NAF fully qualified domain name with maximum size of 255 bytes.
     *
     * @param slotId
     *    Slot identifier in which SIM is inserted.
     *    Value 0(Slot 1) 1(Slot 2) 3(Slot 3)
     *
     * @param applicationType
     *    Application Type of the card application.
     *    Value 0(UNKNOWN) 1(SIM) 2(USIM) 3(RUIM) 4(CSIM) 5(ISIM)
     *
     * @param forceBootStrapping
     *    true=force bootstrapping, false=do not force bootstrapping.
     *
     *  @param apn
     *     Access point name
     *     This is optional i.e. If empty string is passed, then
     *     APN is selected based on NV configuration or other policies
     *     in the modem
     *
     * @return
     *     UIM_GBA_SUCCESS         = 0;
     *     UIM_GBA_GENERIC_FAILURE = 1;
     */
    int gbaInit(in int token, in byte[] securityProtocol,
                in String nafFullyQualifiedDomainName, in int slotId,
                in byte applicationType, in boolean forceBootStrapping,
                in String apn);

    /**
     * getImpi will be used by client to read IMPI.
     *
     * @param token
     *    This is used for identification of the request.
     *
     * @param slotId
     *    Slot identifier in which SIM is inserted.
     *    Value 0(Slot 1) 1(Slot 2) 3(Slot 3)
     *
     * @param applicationType
     *    Application Type of the card application.
     *    Value 0(UNKNOWN) 1(SIM) 2(USIM) 3(RUIM) 4(CSIM) 5(ISIM)
     *
     * @param secure
     *    true=return encrypted IMPI value, false=return plain text IMPI value
     *
     * @return
     *     UIM_GBA_SUCCESS         = 0;
     *     UIM_GBA_GENERIC_FAILURE = 1;
     */
    int getImpi(in int token, in int slotId, in byte applicationType,
                in boolean secure);
 }
