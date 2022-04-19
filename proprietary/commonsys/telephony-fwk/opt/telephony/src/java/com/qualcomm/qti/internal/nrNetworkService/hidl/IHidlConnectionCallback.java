/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

package com.qualcomm.qti.internal.nrNetworkService.hidl;

import org.codeaurora.internal.BearerAllocationStatus;
import org.codeaurora.internal.DcParam;
import org.codeaurora.internal.NrConfigType;
import org.codeaurora.internal.SignalStrength;
import org.codeaurora.internal.Status;
import org.codeaurora.internal.Token;
import org.codeaurora.internal.UpperLayerIndInfo;
import org.codeaurora.internal.NrIconType;

public interface IHidlConnectionCallback {
    public void on5gStatus(int slotId, Token token, Status status, boolean enableStatus);

    public void onNrDcParam(int slotId, Token token, Status status, DcParam dcParam);

    public void onAnyNrBearerAllocation(int slotId, Token token, Status status,
                                        BearerAllocationStatus bearerStatus);

    public void onUpperLayerIndInfo(int slotId, Token token, Status status,
                                    UpperLayerIndInfo upperLayerInfo);

    public void onSignalStrength(int slotId, Token token, Status status,
            SignalStrength signalStrength);

    public void on5gConfigInfo(int slotId, Token token, Status status,
                                 NrConfigType nrConfigType);

    public void onNrIconType(int slotId, Token token, Status status,
                               NrIconType nrIconType);

    public void onEnableEndc(int slotId, Token token, Status status);

    public void onEndcStatus(int slotId, Token token, Status status, boolean enableStatus);
}
