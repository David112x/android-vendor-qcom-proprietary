/*
 * Copyright (c) 2015, 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright (c) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.codeaurora.ims;

import com.qualcomm.ims.utils.Log;

import android.os.Handler;
import android.os.HandlerThread;

import android.telephony.ims.stub.ImsCallSessionImplBase;
import android.telephony.ims.ImsCallSessionListener;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsConferenceState;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsStreamMediaProfile;
import android.telephony.ims.ImsSuppServiceNotification;

/**
 * IMS Service class that invokes ImsCallSession-related callbacks
 * in a separate Handler thread.
 */
public class ImsCallSessionCallbackHandler {
    public ImsCallSessionListener mListener;

    private HandlerThread mHandlerThread;
    private Handler mCallbackHandler;

    public ImsCallSessionCallbackHandler() {
        Log.i(this, "Constructor: start handler thread for callbacks.");
        mHandlerThread = new HandlerThread(ImsCallSessionCallbackHandler.class.getSimpleName());
        mHandlerThread.start();
        mCallbackHandler = new Handler(mHandlerThread.getLooper());
    }

    public ImsCallSessionCallbackHandler(ImsCallSessionCallbackHandler copyFrom) {
        Log.i(this, "Copy Constructor: Pass the thread and callback handler refs.");
        this.mListener = copyFrom.mListener;
        mHandlerThread = new HandlerThread(ImsCallSessionCallbackHandler.class.getSimpleName());
        mHandlerThread.start();
        mCallbackHandler = new Handler(mHandlerThread.getLooper());
    }

    public void callSessionInitiated(final ImsCallProfile profile) {
        postRunnable(() -> {
            if (mListener != null) {
                Log.i(this, "callSessionInitiated :: profile=" + profile);
                try {
                    mListener.callSessionInitiated(profile);
                } catch (RuntimeException r) {
                    Log.e(this, "callSessionInitiated :: RuntimeException " + r);
                }
            }
        });
    }

    public void callSessionInitiatedFailed(final ImsReasonInfo reason) {
        postRunnable(() -> {
            if (mListener != null) {
                Log.i(this, "callSessionInitiatedFailed :: reason=" + reason);
                try {
                    mListener.callSessionInitiatedFailed(reason);
                } catch (RuntimeException r) {
                    Log.e(this, "callSessionInitiatedFailed :: RuntimeException " + r);
                }
            }
        });
    }

    public void callSessionProgressing(final ImsStreamMediaProfile profile) {
        postRunnable(() -> {
            if (mListener != null) {
                Log.i(this, "callSessionProgressing :: profile=" + profile);
                try {
                    mListener.callSessionProgressing(profile);
                } catch (RuntimeException r) {
                    Log.e(this, "callSessionProgressing :: RuntimeException " + r);
                }
            }
        });
    }

    public void callSessionTerminated(final ImsReasonInfo reason) {
        postRunnable(() -> {
            if (mListener != null) {
                Log.i(this, "callSessionTerminated :: reason=" + reason);
                try {
                    mListener.callSessionTerminated(reason);
                } catch (RuntimeException r) {
                    Log.e(this, "callSessionTerminated :: RuntimeException " + r);
                }
            }
        });
    }

    public void callSessionHandover(final int srcAccessTech, final int targetAccessTech,
            final ImsReasonInfo reasonInfo) {
        postRunnable(() -> {
            if (mListener != null) {
                Log.i(this, "callSessionHandover :: srcAccessTech="
                        + srcAccessTech + " targetAccessTech=" + targetAccessTech
                        + " reasonInfo=" + reasonInfo);
                try {
                    mListener.callSessionHandover(srcAccessTech, targetAccessTech, reasonInfo);
                } catch (RuntimeException r) {
                    Log.e(this, "callSessionHandover :: RuntimeException " + r);
                }
            }
        });
    }

    public void callSessionHandoverFailed(final int srcAccessTech, final int targetAccessTech,
            final ImsReasonInfo reasonInfo) {
        postRunnable(() -> {
            if (mListener != null) {
                Log.i(this, "callSessionHandoverFailed :: srcAccessTech="
                                + srcAccessTech + " targetAccessTech=" + targetAccessTech
                                + " reasonInfo=" + reasonInfo);
                try {
                    mListener.callSessionHandoverFailed(srcAccessTech, targetAccessTech,
                            reasonInfo);
                } catch (RuntimeException r) {
                    Log.e(this, "callSessionHandoverFailed :: RuntimeException " + r);
                }
            }
        });
    }

    public void callSessionMayHandover(int srcAccessTech, int targetAccessTech) {
        postRunnable(() -> {
            if (mListener != null) {
                Log.i(this, "callSessionMayHandover :: srcAccessTech="
                                + srcAccessTech + " targetAccessTech=" + targetAccessTech);
                try {
                    mListener.callSessionMayHandover(srcAccessTech, targetAccessTech);
                } catch (RuntimeException r) {
                    Log.e(this, "callSessionMayHandover :: RuntimeException " + r);
                }
            }
        });
    }

    public void callSessionTtyModeReceived(final int ttyMode) {
        postRunnable(() -> {
            if (mListener != null) {
                Log.i(this, "callSessionTtyModeReceived :: ttyMode=" + ttyMode);
                try {
                    mListener.callSessionTtyModeReceived(ttyMode);
                } catch (RuntimeException r) {
                    Log.e(this, "callSessionTtyModeReceived :: RuntimeException " + r);
                }
            }
        });
    }

    /**
     * Notifies the result of the call hold/resume operation.
     */
    public void callSessionHeld(final ImsCallProfile profile) {
        postRunnable(() -> {
            if (mListener != null) {
                Log.i(this, "callSessionHeld :: profile=" + profile);
                try {
                    mListener.callSessionHeld(profile);
                } catch (RuntimeException r) {
                    Log.e(this, "callSessionHeld :: RuntimeException " + r);
                }
            }
        });
    }

    public void callSessionHoldFailed(final ImsReasonInfo reasonInfo) {
        postRunnable(() -> {
            if (mListener != null) {
                Log.i(this, "callSessionHoldFailed :: reasonInfo=" + reasonInfo);
                try {
                    mListener.callSessionHoldFailed(reasonInfo);
                } catch (RuntimeException r) {
                    Log.e(this, "callSessionHoldFailed :: RuntimeException " + r);
                }
            }
        });
    }

    public void callSessionHoldReceived(final ImsCallProfile profile) {
        postRunnable(() -> {
            if (mListener != null) {
                Log.i(this, "callSessionHoldReceived :: profile=" + profile);
                try {
                    mListener.callSessionHoldReceived(profile);
                } catch (RuntimeException r) {
                    Log.e(this, "callSessionHoldReceived :: RuntimeException " + r);
                }
            }
        });
    }

    public void callSessionResumed(final ImsCallProfile profile) {
        postRunnable(() -> {
            if (mListener != null) {
                Log.i(this, "callSessionResumed :: profile=" + profile);
                try {
                    mListener.callSessionResumed(profile);
                } catch (RuntimeException r) {
                    Log.e(this, "callSessionResumed :: RuntimeException " + r);
                }
            }
        });
    }

    public void callSessionResumeFailed(final ImsReasonInfo reasonInfo) {
        postRunnable(() -> {
            if (mListener != null) {
                Log.i(this, "callSessionResumeFailed :: reasonInfo=" + reasonInfo);
                try {
                    mListener.callSessionResumeFailed(reasonInfo);
                } catch (RuntimeException r) {
                    Log.e(this, "callSessionResumeFailed :: RuntimeException " + r);
                }
            }
        });
    }

    public void callSessionResumeReceived(final ImsCallProfile profile) {
        postRunnable(() -> {
            if (mListener != null) {
                Log.i(this, "callSessionResumeReceived :: profile=" + profile);
                try {
                    mListener.callSessionResumeReceived(profile);
                } catch (RuntimeException r) {
                    Log.e(this, "callSessionResumeReceived :: RuntimeException " + r);
                }
            }
        });
    }

    /**
     * Notifies when the call session merge has started.
     *
     * @param session The session to merge.
     * @param newSession The new merged session.
     * @param profile The call profile.
     */
    public void callSessionMergeStarted(final ImsCallSessionImpl newSession,
            final ImsCallProfile profile) {
        postRunnable(() -> {
            if (mListener != null) {
                Log.i(this, "callSessionMergeStarted :: newSession=" + newSession
                        + " profile=" + profile);
                try {
                    mListener.callSessionMergeStarted((ImsCallSessionImplBase)newSession,
                            profile);
                } catch (RuntimeException r) {
                    Log.e(this, "callSessionMergeStarted :: RuntimeException " + r);
                }
            }
        });
    }

    /**
     * Notifies when the call session merge has successfully completed.
     *
     * @param session The merged call session.
     */
    public void callSessionMergeComplete(final ImsCallSessionImpl activeCallSession) {
        postRunnable(() -> {
            if (mListener != null) {
                Log.i(this, "callSessionMergeComplete :: activeCallSession =" + activeCallSession);
                try {
                    mListener.callSessionMergeComplete((ImsCallSessionImplBase)activeCallSession);
                } catch (RuntimeException r) {
                    Log.e(this, "callSessionMergeComplete :: RuntimeException " + r);
                }
            }
        });
    }

    /**
     * Notifies when a call session merge has failed to complete.
     *
     * @param session The session which failed to merge.
     * @param reasonInfo The reason for the failure.
     */
    public void callSessionMergeFailed(final ImsReasonInfo reasonInfo) {
        postRunnable(() -> {
            if (mListener != null) {
                Log.i(this, "callSessionMergeFailed :: reasonInfo=" + reasonInfo);
                try {
                    mListener.callSessionMergeFailed(reasonInfo);
                } catch (RuntimeException r) {
                    Log.e(this, "callSessionMergeFailed :: RuntimeException " + r);
                }
            }
        });
    }

    /**
     * Notifies the result of call upgrade / downgrade or any other call updates.
     */
    public void callSessionUpdated(final ImsCallProfile profile) {
        postRunnable(() -> {
            if (mListener != null) {
                Log.i(this, "callSessionUpdated :: profile=" + profile);
                try {
                    mListener.callSessionUpdated(profile);
                } catch (RuntimeException r) {
                    Log.e(this, "callSessionUpdated :: RuntimeException " + r);
                }
            }
        });
    }

    /**
     * Notifies the supplementary service notification
     */
    public void callSessionSuppServiceReceived(
            final ImsSuppServiceNotification suppServiceInfo) {
        postRunnable(() -> {
            if (mListener != null) {
                Log.i(this, "callSessionSuppServiceReceived :: suppServiceInfo="
                        + suppServiceInfo);
                try {
                    mListener.callSessionSuppServiceReceived(suppServiceInfo);
                } catch (RuntimeException r) {
                    Log.e(this, "callSessionSuppServiceReceived :: RuntimeException " + r);
                }
            }
        });
    }

    /**
     * Notifies the result of the participant invitation / removal to/from the conference session.
     */
    public void callSessionInviteParticipantsRequestDelivered() {
        postRunnable(() -> {
            if (mListener != null) {
                Log.i(this, "callSessionInviteParticipantsRequestDelivered");
                try {
                    mListener.callSessionInviteParticipantsRequestDelivered();
                } catch (RuntimeException r) {
                    Log.e(this,
                            "callSessionInviteParticipantsRequestDelivered :: RuntimeException "
                            + r);
                }
            }
        });
    }

    public void callSessionInviteParticipantsRequestFailed(final ImsReasonInfo reasonInfo) {
        postRunnable(() -> {
            if (mListener != null) {
                Log.i(this, "callSessionInviteParticipantsRequestFailed"
                        + " reasonInfo=" + reasonInfo);
                try {
                    mListener.callSessionInviteParticipantsRequestFailed(reasonInfo);
                } catch (RuntimeException r) {
                    Log.e(this, "callSessionInviteParticipantsRequestFailed :: RuntimeException "
                            + r);
                }
            }
        });
    }

    /**
     * Notifies the changes of the conference info. the conference session.
     */
    public void callSessionConferenceStateUpdated(final ImsConferenceState state) {
        postRunnable(() -> {
            if (mListener != null) {
                Log.i(this, "callSessionConferenceStateUpdated :: state=" + state);
                try {
                    mListener.callSessionConferenceStateUpdated(state);
                } catch (RuntimeException r) {
                    Log.e(this, "callSessionConferenceStateUpdated :: RuntimeException " + r);
                }
            }
        });
    }

   /**
     * Notifies of a change to the multiparty state for this {@code ImsCallSession}.
     *
     * @param isMultiParty {@code true} if the session became multiparty, {@code false} otherwise.
     */
    public void callSessionMultipartyStateChanged(final boolean isMultiParty) {
        postRunnable(() -> {
            if (mListener != null) {
                Log.i(this, "callSessionMultipartyStateChanged :: isMultiParty="
                        + isMultiParty);
                try {
                    mListener.callSessionMultipartyStateChanged(isMultiParty);
                } catch (RuntimeException r) {
                    Log.e(this, "callSessionMultipartyStateChanged :: RuntimeException " + r);
                }
            }
        });
    }

    public void callSessionRttModifyResponseReceived(final int status) {
        postRunnable(() -> {
            if (mListener != null) {
                Log.i(this, "callSessionRttModifyResponseReceived :: status=" + status);
                try {
                    mListener.callSessionRttModifyResponseReceived(status);
                } catch (RuntimeException r) {
                    Log.e(this, "callSessionRttModifyResponseReceived :: RuntimeException " + r);
                }
            }
        });
    }

    public void callSessionRttMessageReceived(final String rttMessage) {
        postRunnable(() -> {
            if (mListener != null) {
                Log.i(this, "callSessionRttMessageReceived :: rttMessage=" + rttMessage);
                try {
                    mListener.callSessionRttMessageReceived(rttMessage);
                } catch (RuntimeException r) {
                    Log.e(this, "callSessionRttMessageReceived :: RuntimeException " + r);
                }
            }
        });
    }

    public void callSessionRttModifyRequestReceived(final ImsCallProfile profile) {
        postRunnable(() -> {
            if (mListener != null) {
                Log.i(this, "callSessionRttModifyRequestReceived :: profile=" + profile);
                try {
                    mListener.callSessionRttModifyRequestReceived(profile);
                } catch (RuntimeException r) {
                    Log.e(this, "callSessionRttModifyRequestReceived :: RuntimeException " + r);
                }
            }
        });
    }

    public void callSessionRttAudioIndicatorChanged(final ImsStreamMediaProfile profile) {
        postRunnable(() -> {
            if (mListener != null) {
                Log.i(this, "callSessionRttAudioIndicatorChanged :: profile=" + profile);
                try {
                    mListener.callSessionRttAudioIndicatorChanged(profile);
                } catch (RuntimeException r) {
                    Log.e(this, "callSessionRttAudioIndicatorChanged :: RuntimeException " + r);
                }
            }
        });
    }


    public void dispose() {
        postRunnable(() -> {
            Log.i(this, "dispose");
            cleanup();
        });
    }

    private void cleanup() {
        Log.i(this, "cleanup");
        if (mHandlerThread != null) {
            mHandlerThread.quitSafely();
            mHandlerThread = null;
            mCallbackHandler = null;
        }
    }

    private void postRunnable(Runnable r) {
        Log.i(this, "posting to handler");
        if (mCallbackHandler != null) {
            mCallbackHandler.post(r);
        } else {
            Log.e(this, "Handler is null. Can't post runnable!");
        }
    }
}
