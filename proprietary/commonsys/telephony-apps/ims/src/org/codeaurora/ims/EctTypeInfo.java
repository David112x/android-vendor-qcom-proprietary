/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

public final class EctTypeInfo {

    private EctTypeInfo() {}

    /* Transfer the call without establishing a session with
     * the transfer target
     */
    public static final int BLIND_TRANSFER = 0;

    /* Same as blind transfer, except Transferer waits for
     * session establishment between Transferee and Transfer target
     * for releasing the Transferer-Transferee call
     */
    public static final int ASSURED_TRANSFER = 1;

    /* Call with transfer target is established before initiating
     * call transfer, and required to pass the call id of the call
     * with Transfer target
     */
    public static final int CONSULTATIVE_TRANSFER = 2;
}

