/*********************************************************************
 Copyright (c) 2017,2019 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

package com.qualcomm.qti.presence;

import com.android.ims.internal.uce.common.UceLong;
import com.android.ims.internal.uce.common.StatusCode;
import com.android.ims.internal.uce.presence.*;

import com.qualcomm.qti.uceservice.V2_0.UceStatus;
import com.qualcomm.qti.uceservice.V2_0.UceStatusCode;
import com.qualcomm.qti.uceservice.V2_0.PresenceCapabilityInfo;
import com.qualcomm.qti.uceservice.V2_0.PresServiceInfo;
import com.qualcomm.qti.uceservice.V2_0.RcsFeatureTag;

import com.qualcomm.qti.translator.*;

import android.os.RemoteException;
import android.os.Parcel;
import android.util.Log;

import java.util.Arrays;
import java.util.ArrayList;
import java.util.List;
import java.util.Vector;
import java.util.Map;
import java.util.HashMap;

import com.qualcomm.qti.uceShimService.RCSService;


public class PresService extends IPresenceService.Stub {

    private final static String logTAG = "QRCSService PresService";
    private AidlToHidlTranslator translateToHidlObj = null;
    private HidlToAidlTranslator translateToAidlObj = null;
    private static Map<Integer, com.qualcomm.qti.uceservice.V2_0.IPresenceService> presHandleMap =
                          new HashMap<Integer, com.qualcomm.qti.uceservice.V2_0.IPresenceService>();
    private static Map<Integer, com.qualcomm.qti.uceservice.V2_1.IPresenceService> presHandleMap_2_1 =
                          new HashMap<Integer, com.qualcomm.qti.uceservice.V2_1.IPresenceService>();
    // maintatin list of HIDL Listeners
    private static List<PresListener> hidlListenerlist = new ArrayList<PresListener>();

    public void setServiceHandle (int serviceHandle) {
        if (RCSService.mHidlService_2_1 != null) {
            com.qualcomm.qti.uceservice.V2_1.IPresenceService localHidlPresenceService = null;
            try {
                localHidlPresenceService = RCSService.mHidlService_2_1.getPresenceService_2_1(serviceHandle);
            }catch(RemoteException | RuntimeException e) {
                Log.w(logTAG, "Unexpected remote exception", e);
            }
            if(localHidlPresenceService != null) {
                presHandleMap_2_1.put(serviceHandle, localHidlPresenceService);
            }
        } else {
            com.qualcomm.qti.uceservice.V2_0.IPresenceService localHidlPresenceService = null;
            try {
                localHidlPresenceService = RCSService.mHidlService.getPresenceService(serviceHandle);
            }catch(RemoteException | RuntimeException e) {
               Log.w(logTAG, "Unexpected remote exception", e);
            }
            if(localHidlPresenceService != null) {
                presHandleMap.put(serviceHandle, localHidlPresenceService);
            }
        }
    }
    public StatusCode getVersion(int pPresServiceHdl)
            throws RemoteException {

        StatusCode retStatus = new StatusCode();
        retStatus.setStatusCode(StatusCode.UCE_NOT_SUPPORTED);
        return retStatus;
    }

    public void addHidlListener(PresListener hidlListener) {
        hidlListenerlist.add(hidlListener);
    }

    public StatusCode addListener(int pPresServiceHdl,
            IPresenceListener pPresServiceListener,
            UceLong pPresServiceListenerHdl) throws RemoteException {
        StatusCode retStatus = new StatusCode();
        retStatus.setStatusCode(StatusCode.UCE_FAILURE);
        translateToAidlObj = new HidlToAidlTranslator();

        long clientHandle = pPresServiceListenerHdl.getUceLong();
        PresListener hidlPresListener = new PresListener();
        addHidlListener(hidlPresListener);
        hidlPresListener.setAidlPresListener(pPresServiceListener);
        try {
            UceStatus pStatus;
            if (RCSService.mHidlService_2_1 != null) {
                pStatus = presHandleMap_2_1.get(pPresServiceHdl)
                               .addListener_2_1((long)pPresServiceHdl,
                                             hidlPresListener, clientHandle);
            } else {
                Log.d(logTAG, "adding 2.0 hal listener");
                pStatus = presHandleMap.get(pPresServiceHdl)
                                   .addListener((long)pPresServiceHdl,
                                                 hidlPresListener, clientHandle);
            }
            retStatus.setStatusCode(translateToAidlObj.getAidlStatusCode(pStatus.status));
        } catch(RemoteException | RuntimeException e) {
           Log.w(logTAG, "Unexpected remote exception", e);
           throw e;
        }
        return retStatus;
    }

    public StatusCode removeListener(int pPresServiceHdl,
            UceLong pPresServiceListenerHdl) throws RemoteException {

        StatusCode retStatus = new StatusCode();
    retStatus.setStatusCode(StatusCode.UCE_FAILURE);
    translateToAidlObj = new HidlToAidlTranslator();

        long presServiceListenerHdl = pPresServiceListenerHdl.getUceLong();
        try {
			UceStatus pStatus = null;
			if (RCSService.mHidlService_2_1 != null)
		    {
			    pStatus = presHandleMap_2_1.get(pPresServiceHdl)
                                  .removeListener(pPresServiceHdl, presServiceListenerHdl);
			}
			else
                pStatus = presHandleMap.get(pPresServiceHdl).removeListener(pPresServiceHdl, presServiceListenerHdl);
			
            retStatus.setStatusCode(translateToAidlObj.getAidlStatusCode(pStatus.status));
        } catch(RemoteException | RuntimeException e) {
           Log.w(logTAG, "Unexpected remote exception", e);
           throw e;
        }
        return retStatus;
    }
//no o/p type mentioned for reEnableService
    public StatusCode reenableService(int pPresServiceHdl,
                                      int pUserData) throws RemoteException {

        StatusCode retStatus = new StatusCode();
    retStatus.setStatusCode(StatusCode.UCE_FAILURE);
    translateToAidlObj = new HidlToAidlTranslator();
        UceStatus pStatus = null;
    try {
		if (RCSService.mHidlService_2_1 != null)
		{
			pStatus = presHandleMap_2_1.get(pPresServiceHdl).reEnableService(pPresServiceHdl, pUserData);
			retStatus.setStatusCode(pStatus.status);
		}
		else
	    {
			pStatus = presHandleMap.get(pPresServiceHdl).reEnableService(pPresServiceHdl, pUserData);
            //hidlPresenceService.reEnableService(pPresServiceHdl, pUserData);
            retStatus.setStatusCode(pStatus.status); // to replace with
            //translateToAidlObj.getAidlStatusCode (pStatus.status)
		}
        return retStatus;
        }
        catch( RemoteException | RuntimeException e ) {
         Log.w(logTAG, "Unexpected remote exception", e);
         throw e;
        }
    }

    public StatusCode publishMyCap(int pPresServiceHdl,
            PresCapInfo  pMyCapInfo, int pUserData) throws RemoteException {
        StatusCode retStatus = new StatusCode();
        retStatus.setStatusCode(StatusCode.UCE_FAILURE);
        translateToAidlObj = new HidlToAidlTranslator();
        translateToHidlObj = new AidlToHidlTranslator();
        try {
            UceStatus pStatus;
            if (RCSService.mHidlService_2_1 != null) {
                com.qualcomm.qti.uceservice.V2_1.PresenceCapabilityInfo pHidlCapInfo = 
                    new com.qualcomm.qti.uceservice.V2_1.PresenceCapabilityInfo();
                pHidlCapInfo = translateToHidlObj.getHidlPresCapInfo_2_1(pMyCapInfo);

                pStatus = presHandleMap_2_1.get(pPresServiceHdl)
                            .publishCapability_2_1(pPresServiceHdl,
                                               pHidlCapInfo, pUserData);
                if (pStatus.status == UceStatusCode.NOT_SUPPORTED) {
                    Log.i(logTAG, "publishcapability_2_1 not supported, trying legacy version");
                    PresenceCapabilityInfo pHidlCapInfo_2_0 = new PresenceCapabilityInfo();
                    pHidlCapInfo_2_0 = translateToHidlObj.getHidlPresCapInfo(pMyCapInfo);
                    pStatus = presHandleMap_2_1.get(pPresServiceHdl)
                                    .publishCapability(pPresServiceHdl, pHidlCapInfo_2_0, pUserData);
                }
            } else {
                PresenceCapabilityInfo pHidlCapInfo = new PresenceCapabilityInfo();
                pHidlCapInfo = translateToHidlObj.getHidlPresCapInfo(pMyCapInfo);

                pStatus = presHandleMap.get(pPresServiceHdl)
                                .publishCapability(pPresServiceHdl,
                                                   pHidlCapInfo, pUserData);
            }
            retStatus.setStatusCode(translateToAidlObj.getAidlStatusCode(pStatus.status));
        } catch(RemoteException | RuntimeException e) {
           Log.w(logTAG, "Unexpected remote exception", e);
           throw e;
        }
    return retStatus;
    }

    public StatusCode getContactCap(int pPresServiceHdl,
            String pRemoteURI, int pUserData) throws RemoteException {

        StatusCode retStatus = new StatusCode();
    retStatus.setStatusCode(StatusCode.UCE_FAILURE);
    translateToAidlObj = new HidlToAidlTranslator();
        try {
			UceStatus pStatus;
			if(RCSService.mHidlService_2_1 != null)
			{
				pStatus = presHandleMap_2_1.get(pPresServiceHdl)
                                 .getContactCapability(pPresServiceHdl,
                                                       pRemoteURI, pUserData);
			}
			else {
                pStatus = presHandleMap.get(pPresServiceHdl)
                                 .getContactCapability(pPresServiceHdl,
                                                       pRemoteURI, pUserData);
			}
            retStatus.setStatusCode(translateToAidlObj.getAidlStatusCode(pStatus.status));
        } catch(RemoteException | RuntimeException e) {
           Log.w(logTAG, "Unexpected remote exception", e);
           throw e;
        }
        return retStatus;
    }

    public StatusCode getContactListCap(int pPresServiceHdl,
            String[] pRemoteURIList, int pUserData)
            throws RemoteException {

    StatusCode retStatus = new StatusCode();
    retStatus.setStatusCode(StatusCode.UCE_FAILURE);
    translateToAidlObj = new HidlToAidlTranslator();

    ArrayList<String> stringRemoteURIList = new ArrayList<String>(Arrays.asList(pRemoteURIList));
        try {
            UceStatus pStatus;
			if(RCSService.mHidlService_2_1 != null)
			{
				pStatus = presHandleMap_2_1.get(pPresServiceHdl)
                                .getContactListCapability((long)pPresServiceHdl,
                                                           stringRemoteURIList,
                                                           (long) pUserData);
			}
			else
			{
				pStatus = presHandleMap.get(pPresServiceHdl)
                                .getContactListCapability((long)pPresServiceHdl,
                                                           stringRemoteURIList,
                                                           (long) pUserData);
			}
        
            retStatus.setStatusCode(translateToAidlObj.getAidlStatusCode(pStatus.status));
        } catch(RemoteException | RuntimeException e) {
           Log.w(logTAG, "Unexpected remote exception", e);
           throw e;
        }
        return retStatus;
    }

    public StatusCode setNewFeatureTag(int pPresServiceHdl, String pFeatureTag,
            com.android.ims.internal.uce.presence.PresServiceInfo pServiceInfo, int pUserData)
            throws RemoteException {

    StatusCode retStatus = new StatusCode();
    retStatus.setStatusCode(StatusCode.UCE_FAILURE);
    translateToAidlObj = new HidlToAidlTranslator();
    translateToHidlObj = new AidlToHidlTranslator();

    RcsFeatureTag hidlRcsFeatureTag = new RcsFeatureTag();
    hidlRcsFeatureTag.featureTag = pFeatureTag;
    PresServiceInfo hidlPresSvcInfo = new PresServiceInfo();
    hidlPresSvcInfo = translateToHidlObj.getHidlPresSvcInfo(pServiceInfo);
        try {
			UceStatus pStatus;
			if(RCSService.mHidlService_2_1 != null)
			{
				pStatus = presHandleMap_2_1.get(pPresServiceHdl)
                               .setNewFeatureTag((long)pPresServiceHdl, hidlRcsFeatureTag,
                               hidlPresSvcInfo, (long)pUserData);
			}
            else
			{
				pStatus = presHandleMap.get(pPresServiceHdl)
                               .setNewFeatureTag((long)pPresServiceHdl, hidlRcsFeatureTag,
                               hidlPresSvcInfo, (long)pUserData);
			}
        
            retStatus.setStatusCode(translateToAidlObj.getAidlStatusCode(pStatus.status));
        } catch(RemoteException | RuntimeException e) {
           Log.w(logTAG, "Unexpected remote exception", e);
           throw e;
        }
    return retStatus;
    }
}
