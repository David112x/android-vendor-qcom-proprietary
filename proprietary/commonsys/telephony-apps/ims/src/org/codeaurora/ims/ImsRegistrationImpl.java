/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

import android.net.Uri;

import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.stub.ImsRegistrationImplBase;
import android.telephony.ServiceState;

import com.qualcomm.ims.utils.Log;

/**
 * Extension of the IMS Registration module of IMS Service, that communicates the
 * status of IMS Service's registration with the network, and the radio access
 * technology it is registered on.
 *
 * This class invokes the methods of its parent class to propagate information.
 */
public class ImsRegistrationImpl extends ImsRegistrationImplBase {

    /**
     * Notifies the framework when IMS Service is connected to the IMS network.
     */
    public void registeredWithRadioTech(int imsRadioTech) {
        Log.d(this, "registeredWithRadioTech :: imsRadioTech=" + imsRadioTech);
        onRegistered(getImsRegTechFromRadioTech(imsRadioTech));
    }

    /**
     * Notifies the framework when IMS Service is trying to connect the IMS network.
     */
    public void registeringWithRadioTech(int imsRadioTech) {
        Log.d(this, "registeringWithRadioTech :: imsRadioTech=" + imsRadioTech);
        onRegistering(getImsRegTechFromRadioTech(imsRadioTech));
    }

    /**
     * Notifies the framework when IMS Service is disconnected from the IMS network.
     */
    public void registrationDisconnected(ImsReasonInfo info) {
        Log.d(this, "registrationDisconnected :: info=" + info);
        onDeregistered(info);
    }

    /**
     * A failure has occurred when trying to handover registration to another technology type.
     */
    public void registrationChangeFailed(int imsRadioTech, ImsReasonInfo info) {
        Log.d(this, "registrationChangeFailed :: imsRadioTech=" + imsRadioTech + " info=" + info);
        onTechnologyChangeFailed(getImsRegTechFromRadioTech(imsRadioTech), info);
    }

    /**
     * Returns a list of subscriber {@link Uri}s associated with this IMS subscription when
     * it changes.
     */
    public void registrationAssociatedUriChanged(Uri[] uris) {
        Log.d(this, "registrationAssociatedUriChanged :: uris.length=" + uris.length);
        onSubscriberAssociatedUriChanged(uris);
    }

    /**
     * Utility method to convert IMS Radio Technology values from
     * ServiceState constants to constants in ImsRegistrationBaseImpl.
     */
    private @ImsRegistrationTech int getImsRegTechFromRadioTech(int imsRadioTech) {
        int regTech = REGISTRATION_TECH_NONE;
        switch (imsRadioTech) {
            case ServiceState.RIL_RADIO_TECHNOLOGY_LTE:
                regTech = REGISTRATION_TECH_LTE;
                break;
            case ServiceState.RIL_RADIO_TECHNOLOGY_IWLAN:
                regTech = REGISTRATION_TECH_IWLAN;
                break;
            case ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN:
            default:
                regTech = REGISTRATION_TECH_NONE;
        }
        return regTech;
    }
}
