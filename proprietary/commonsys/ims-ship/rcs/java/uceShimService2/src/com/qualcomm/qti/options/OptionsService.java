/*********************************************************************
 Copyright (c) 2017,2019 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

package com.qualcomm.qti.options;

import com.android.ims.internal.uce.common.CapInfo;
import com.android.ims.internal.uce.common.StatusCode;
import com.android.ims.internal.uce.common.UceLong;
import com.android.ims.internal.uce.options.OptionsCapInfo;

import com.qualcomm.qti.uceservice.V2_0.CapabilityInfo;
import com.qualcomm.qti.uceservice.V2_0.OptionsCapabilityInfo;
import com.qualcomm.qti.uceservice.V2_0.UceStatusCode;
import com.qualcomm.qti.uceservice.V2_0.UceStatus;

import com.qualcomm.qti.uceShimService.RCSService;

import com.qualcomm.qti.translator.*;

import android.os.RemoteException;
import android.util.Log;
import android.os.Parcel;

import java.util.Arrays;
import java.util.ArrayList;
import java.util.Map;
import java.util.HashMap;
import java.util.List;

public class OptionsService extends com.android.ims.internal.uce.options.IOptionsService.Stub
{
    private final static String logTAG = "RCSService Options-Service";
    private static Map<Integer, com.qualcomm.qti.uceservice.V2_0.IOptionsService> optionsHandleMap =
                          new HashMap<Integer, com.qualcomm.qti.uceservice.V2_0.IOptionsService>();
    private static Map<Integer, com.qualcomm.qti.uceservice.V2_1.IOptionsService> optionsHandleMap_2_1 =
                          new HashMap<Integer, com.qualcomm.qti.uceservice.V2_1.IOptionsService>();
    private AidlToHidlTranslator translateToHidlObj = null;
    private HidlToAidlTranslator translateToAidlObj = null;

    // maintatin list of HIDL Listeners
    private static List<OptionsListener> hidlListenerlist = new ArrayList<OptionsListener>();

    public void setServiceHandle (int serviceHandle) {
        if (RCSService.mHidlService_2_1 != null) {
            com.qualcomm.qti.uceservice.V2_1.IOptionsService localHidlOptionsService = null;
            try {
                localHidlOptionsService = RCSService.mHidlService_2_1.getOptionsService_2_1(serviceHandle);
            }catch(RemoteException | RuntimeException e) {
               Log.w(logTAG, "Unexpected remote exception", e);
            }
            if(localHidlOptionsService != null) {
                optionsHandleMap_2_1.put(serviceHandle, localHidlOptionsService);
            }
        } else {
            com.qualcomm.qti.uceservice.V2_0.IOptionsService localHidlOptionsService = null;
            try {
                localHidlOptionsService = RCSService.mHidlService.getOptionsService(serviceHandle);
            }catch(RemoteException | RuntimeException e) {
               Log.w(logTAG, "Unexpected remote exception", e);
            }
            if(localHidlOptionsService != null) {
                optionsHandleMap.put(serviceHandle, localHidlOptionsService);
            }
        }
    }

    public StatusCode getVersion (int optionsServiceHandle) throws RemoteException {
        StatusCode retStatus = new StatusCode();
        retStatus.setStatusCode(StatusCode.UCE_NOT_SUPPORTED);
        return retStatus;
    }

    public void addHidlListener(OptionsListener hidlListener) {
        hidlListenerlist.add(hidlListener);
    }

    public StatusCode addListener(int optionsServiceHandle,
                                  com.android.ims.internal.uce.options.IOptionsListener optionsListener,
                                  UceLong optionsListenerHdl) throws RemoteException {

        StatusCode retStatus = new StatusCode();
        retStatus.setStatusCode(StatusCode.UCE_FAILURE);
        translateToAidlObj = new HidlToAidlTranslator();

        long clientHandle = optionsListenerHdl.getUceLong();
        OptionsListener hidlOptionsListener = new OptionsListener();
        addHidlListener(hidlOptionsListener);
        hidlOptionsListener.setAidlOptionsServiceListener(optionsListener);
        try {
            if (RCSService.mHidlService_2_1 != null) {
                UceStatus pStatus = optionsHandleMap_2_1.get(optionsServiceHandle)
                    .addListener_2_1((long)optionsServiceHandle, hidlOptionsListener, clientHandle);
                retStatus.setStatusCode(translateToAidlObj.getAidlStatusCode(pStatus.status));
            } else {
                UceStatus pStatus = optionsHandleMap.get(optionsServiceHandle)
                    .addListener((long)optionsServiceHandle, hidlOptionsListener, clientHandle);
                retStatus.setStatusCode(translateToAidlObj.getAidlStatusCode(pStatus.status));
            }
        }
        catch (RemoteException | RuntimeException e) {
            Log.d(logTAG, "Unexpected remote exception", e);
            throw e;
        }
        return retStatus;
    }

    public StatusCode removeListener(int optionsServiceHandle,
            UceLong optionsServiceUserData) throws RemoteException {
        StatusCode retStatus = new StatusCode();
        retStatus.setStatusCode(StatusCode.UCE_FAILURE);
        translateToAidlObj = new HidlToAidlTranslator();

        long optionsServiceListenerHdl = optionsServiceUserData.getUceLong();
        try {
            if (RCSService.mHidlService_2_1 != null) {
                UceStatus pStatus = optionsHandleMap_2_1.get(optionsServiceHandle)
                    .removeListener(optionsServiceHandle, optionsServiceListenerHdl);
                retStatus.setStatusCode(translateToAidlObj.getAidlStatusCode(pStatus.status));
            } else {
                UceStatus pStatus = optionsHandleMap.get(optionsServiceHandle)
                    .removeListener(optionsServiceHandle, optionsServiceListenerHdl);
                retStatus.setStatusCode(translateToAidlObj.getAidlStatusCode(pStatus.status));
            }
        }
        catch (RemoteException | RuntimeException e) {
            Log.d(logTAG, "Unexpected remote exception", e);
            throw e;
        }
        return retStatus;
    }

    public StatusCode getMyInfo(int optionsServiceHandle, int reqUserData)
            throws RemoteException {
        StatusCode retStatus = new StatusCode();
        retStatus.setStatusCode(StatusCode.UCE_FAILURE);
        translateToAidlObj = new HidlToAidlTranslator();

        try {
            if (RCSService.mHidlService_2_1 != null) {
                UceStatus pStatus = optionsHandleMap_2_1.get(optionsServiceHandle)
                    .getCapabilityInfo((long)optionsServiceHandle, (long)reqUserData);
                retStatus.setStatusCode(translateToAidlObj.getAidlStatusCode(pStatus.status));
            } else {
                UceStatus pStatus = optionsHandleMap.get(optionsServiceHandle)
                    .getCapabilityInfo((long)optionsServiceHandle, (long)reqUserData);
                retStatus.setStatusCode(translateToAidlObj.getAidlStatusCode(pStatus.status));
            }
        }
        catch (RemoteException | RuntimeException e) {
            Log.d(logTAG, "Unexpected remote exception", e);
            throw e;
        }
        return retStatus;
    }

    public StatusCode getContactCap(int optionsServiceHandle, String remoteURI,
                                    int reqUserData) throws RemoteException {
        StatusCode retStatus = new StatusCode();
        retStatus.setStatusCode(StatusCode.UCE_FAILURE);
        translateToAidlObj = new HidlToAidlTranslator();

        try {
            if (RCSService.mHidlService_2_1 != null) {
                UceStatus pStatus = optionsHandleMap_2_1.get(optionsServiceHandle)
                    .getContactCapability((long)optionsServiceHandle, remoteURI, (long)reqUserData);
                retStatus.setStatusCode(translateToAidlObj.getAidlStatusCode(pStatus.status));
            } else {
                UceStatus pStatus = optionsHandleMap.get(optionsServiceHandle)
                    .getContactCapability((long)optionsServiceHandle, remoteURI, (long)reqUserData);
                retStatus.setStatusCode(translateToAidlObj.getAidlStatusCode(pStatus.status));
            }
        }
        catch (RemoteException | RuntimeException e) {
            Log.d(logTAG, "Unexpected remote exception", e);
            throw e;
        }

        return retStatus;
    }

    public StatusCode getContactListCap(int optionsServiceHandle,
                                        String[] remoteURIList, int reqUserData) throws RemoteException {
        StatusCode retStatus = new StatusCode();
        retStatus.setStatusCode(StatusCode.UCE_FAILURE);
        translateToAidlObj = new HidlToAidlTranslator();

        ArrayList<String> hidlRemoteURIList = new ArrayList<String>(Arrays.asList(remoteURIList));
        try {
            UceStatus pStatus;
            if (RCSService.mHidlService_2_1 != null) {
                pStatus =  optionsHandleMap_2_1.get(optionsServiceHandle)
                    .getContactListCapability((long)optionsServiceHandle,
                                              hidlRemoteURIList, (long)reqUserData);
            } else {
                pStatus =  optionsHandleMap.get(optionsServiceHandle)
                    .getContactListCapability((long)optionsServiceHandle,
                                              hidlRemoteURIList, (long)reqUserData);
            }
            retStatus.setStatusCode(translateToAidlObj.getAidlStatusCode(pStatus.status));
        }
        catch (RemoteException | RuntimeException e) {
            Log.d(logTAG, "Unexpected remote exception", e);
            throw e;
        }
        return retStatus;
    }

    public StatusCode responseIncomingOptions(int optionsServiceHandle,  int tId,
                   int sipResponseCode, String reasonPhrase, OptionsCapInfo capInfo,
                   boolean bContactInBL) throws RemoteException {

        StatusCode retStatus = new StatusCode();
        retStatus.setStatusCode(StatusCode.UCE_SERVICE_UNKNOWN);
        translateToAidlObj = new HidlToAidlTranslator();
        translateToHidlObj = new AidlToHidlTranslator();

        byte contactInBLByte = (byte)(bContactInBL?1:0);
        try {
            UceStatus pStatus;
            if (RCSService.mHidlService_2_1 != null) {
                com.qualcomm.qti.uceservice.V2_1.OptionsCapabilityInfo hidlCapInfo =
                     new com.qualcomm.qti.uceservice.V2_1.OptionsCapabilityInfo();
                hidlCapInfo = translateToHidlObj.getHidlOptCapInfo_2_1(capInfo);
                pStatus = optionsHandleMap_2_1.get(optionsServiceHandle)
                                 .responseIncomingOptions_2_1((long)optionsServiceHandle,
                                            tId, (short)sipResponseCode, reasonPhrase,
                                            hidlCapInfo, contactInBLByte);
                if (pStatus.status == UceStatusCode.NOT_SUPPORTED) {
                    Log.i(logTAG, "responseIncomingOptions_2_1 not supported, trying legacy version");
                    OptionsCapabilityInfo hidlCapInfo_2_0 = new OptionsCapabilityInfo();
                    hidlCapInfo_2_0 = translateToHidlObj.getHidlOptCapInfo(capInfo);
                    pStatus = optionsHandleMap_2_1.get(optionsServiceHandle)
                                     .responseIncomingOptions((long)optionsServiceHandle,
                                                tId, (short)sipResponseCode, reasonPhrase,
                                                hidlCapInfo_2_0, contactInBLByte);
                }
            } else {
                OptionsCapabilityInfo hidlCapInfo = new OptionsCapabilityInfo();
                hidlCapInfo = translateToHidlObj.getHidlOptCapInfo(capInfo);
                pStatus = optionsHandleMap.get(optionsServiceHandle)
                                     .responseIncomingOptions((long)optionsServiceHandle,
                                                tId, (short)sipResponseCode, reasonPhrase,
                                                hidlCapInfo, contactInBLByte);
            }
            retStatus.setStatusCode(translateToAidlObj.getAidlStatusCode(pStatus.status));
        }
        catch (RemoteException | RuntimeException e) {
            Log.d(logTAG, "Unexpected remote exception", e);
            throw e;
        }
        return retStatus;
    }

    public StatusCode setMyInfo(int optionsServiceHandle, CapInfo capInfo, int reqUserData)
            throws RemoteException {

        StatusCode retStatus = new StatusCode();
        retStatus.setStatusCode(StatusCode.UCE_SERVICE_UNKNOWN);
        translateToAidlObj = new HidlToAidlTranslator();
        translateToHidlObj = new AidlToHidlTranslator();



        try {
            UceStatus pStatus;
            if(RCSService.mHidlService_2_1 != null) {
                com.qualcomm.qti.uceservice.V2_1.CapabilityInfo hidlCapInfo
                   = new com.qualcomm.qti.uceservice.V2_1.CapabilityInfo();
                translateToHidlObj.getHidlCapInfo_2_1(hidlCapInfo, capInfo);
                pStatus = optionsHandleMap_2_1.get(optionsServiceHandle)
                                            .setCapabilityInfo_2_1((long)optionsServiceHandle,
                                                               hidlCapInfo,
                                                               (long)reqUserData);
                if (pStatus.status == UceStatusCode.NOT_SUPPORTED) {
                    Log.i(logTAG, "setCapabilityInfo_2_1 not supported, trying legacy version");
                    CapabilityInfo hidlCapInfo_2_0 = new CapabilityInfo();
                    translateToHidlObj.getHidlCapInfo(hidlCapInfo_2_0, capInfo);
                    pStatus = optionsHandleMap_2_1.get(optionsServiceHandle)
                                            .setCapabilityInfo((long)optionsServiceHandle,
                                                               hidlCapInfo_2_0,
                                                               (long)reqUserData);
                }
            } else {
                CapabilityInfo hidlCapInfo = new CapabilityInfo();
                translateToHidlObj.getHidlCapInfo(hidlCapInfo, capInfo);
                pStatus = optionsHandleMap.get(optionsServiceHandle)
                                            .setCapabilityInfo((long)optionsServiceHandle,
                                                               hidlCapInfo,
                                                               (long)reqUserData);
            }
            retStatus.setStatusCode(translateToAidlObj.getAidlStatusCode(pStatus.status));
        }
        catch (RemoteException | RuntimeException e) {
            Log.d(logTAG, "Unexpected remote exception", e);
            throw e;
        }

        return retStatus;
    }

}
