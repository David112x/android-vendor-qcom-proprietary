/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

package com.qualcomm.qti.internal.nrNetworkService.hidl;

import org.codeaurora.internal.DcParam;
import org.codeaurora.internal.SignalStrength;
import org.codeaurora.internal.Status;
import org.codeaurora.internal.Token;

import android.os.RemoteException;

public interface IHidlConnectionInterface {
    public Token enable5g(int slotId) throws RemoteException;

    public Token disable5g(int slotId) throws RemoteException;

    public Token enable5gOnly(int slotId) throws RemoteException;

    public Token query5gStatus(int slotId) throws RemoteException;

    public Token queryNrDcParam(int slotId) throws RemoteException;

    public Token queryNrBearerAllocation(int slotId) throws RemoteException;

    public Token queryNrSignalStrength(int slotId) throws RemoteException;

    public Token queryUpperLayerIndInfo(int slotId) throws RemoteException;

    public Token query5gConfigInfo(int slotId) throws RemoteException;

    public Token queryNrIconType(int slotId) throws RemoteException;

    public Token enableEndc(int slotId, boolean enabled) throws RemoteException;

    public Token queryEndcStatus(int slotId) throws RemoteException;

    public void registerCallback(IHidlConnectionCallback callback);

    public void unRegisterCallback(IHidlConnectionCallback callback);
}
