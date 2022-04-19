/* Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;
import com.qualcomm.ims.utils.Log;

import android.os.Bundle;
import android.telephony.ims.ImsCallForwardInfo;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsSsData;
import android.telephony.ims.ImsSsInfo;
import android.telephony.ims.ImsUtListener;

public class ImsUtListenerImpl{
    public ImsUtListener mListener;

    /**
     * Notifies the result of the supplementary service configuration update.
     */
    public void utConfigurationUpdated(final int id) {
        startThread(() -> {
            if (mListener != null) {
                Log.i(this, "utConfigurationUpdated :: id = " + id);
                mListener.onUtConfigurationUpdated(id);
            }
        });
    }

    public void utConfigurationUpdateFailed(final int id,
                                            final ImsReasonInfo error) {
        startThread(() -> {
            if (mListener != null) {
                Log.i(this, "utConfigurationUpdateFailed :: id = " + id
                        + " error = " + error);
                mListener.onUtConfigurationUpdateFailed(id, error);
            }
        });
    }

    /**
     * Notifies the result of the supplementary service configuration query.
     */
    public void utConfigurationQueried(final int id,
                                       final Bundle ssInfo) {
        startThread(() -> {
            if (mListener != null) {
                Log.i(this, "utConfigurationQueried :: id = " + id
                        + " ssInfo = " + ssInfo);
                mListener.onUtConfigurationQueried(id, ssInfo);
            }
        });
    }

    public void utConfigurationQueryFailed(final int id,
                                           final ImsReasonInfo error) {
        startThread(() -> {
            if (mListener != null) {
                Log.i(this, "utConfigurationQueryFailed :: id = " + id
                        + " error = " + error);
                mListener.onUtConfigurationQueryFailed(id, error);
            }
        });
    }

    /**
     * Notifies the status of the call barring supplementary service.
     */
    public void utConfigurationCallBarringQueried(final int id,
                                                  final ImsSsInfo[] cbInfo) {
        startThread(() -> {
            if (mListener != null) {
                Log.i(this, "utConfigurationCallBarringQueried :: id = " + id
                        + " cbInfo = " + cbInfo);
                mListener.onUtConfigurationCallBarringQueried(id, cbInfo);
            }
        });
    }

    /**
     * Notifies the status of the call forwarding supplementary service.
     */
    public void utConfigurationCallForwardQueried(final int id,
                                                  final ImsCallForwardInfo[] cfInfo) {
        startThread(() -> {
            if (mListener != null) {
                Log.i(this, "onUtConfigurationCallForwardQueried :: id = " + id
                        + " cfInfo = " + cfInfo);
                mListener.onUtConfigurationCallForwardQueried(id, cfInfo);
            }
        });
    }

    /**
     * Notifies the status of the call waiting supplementary service.
     */
    public void utConfigurationCallWaitingQueried(final int id,
                                                  final ImsSsInfo[] cwInfo) {
        startThread(() -> {
            if (mListener != null) {
                Log.i(this, "utConfigurationCallWaitingQueried :: id = " + id
                        + " cwInfo = " + cwInfo);
                mListener.onUtConfigurationCallWaitingQueried(id, cwInfo);
            }
        });
    }

    /**
     * Notifies the supplementary service indication.
     */
    public void onSupplementaryServiceIndication(final ImsSsData ssData) {
        startThread(() -> {
            if (mListener != null) {
                Log.i(this, "onSupplementaryServiceIndication :: ssData = "
                        + ssData);
                mListener.onSupplementaryServiceIndication(ssData);
            }
        });
    }

    void startThread(Runnable r) {
        Thread t = new Thread(r, "ImsUtListenerImplThread");
        t.start();
    }
}
