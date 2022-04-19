/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 *
 * Copyright (C) 2007 The Android Open Source Project
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

package vendor.qti.iwlan;

/**
 * {@hide}
 */
public class CommandException extends RuntimeException {
    private Error mError;

    public enum Error {
        INVALID_RESPONSE,
        RADIO_NOT_AVAILABLE,
        GENERIC_FAILURE,
        PASSWORD_INCORRECT,
        SIM_PIN2,
        SIM_PUK2,
        REQUEST_NOT_SUPPORTED,
        OP_NOT_ALLOWED_DURING_VOICE_CALL,
        OP_NOT_ALLOWED_BEFORE_REG_NW,
        SMS_FAIL_RETRY,
        SIM_ABSENT,
        SUBSCRIPTION_NOT_AVAILABLE,
        MODE_NOT_SUPPORTED,
        FDN_CHECK_FAILURE,
        ILLEGAL_SIM_OR_ME,
        MISSING_RESOURCE,
        NO_SUCH_ELEMENT,
        SUBSCRIPTION_NOT_SUPPORTED,
        DIAL_MODIFIED_TO_USSD,
        DIAL_MODIFIED_TO_SS,
        DIAL_MODIFIED_TO_DIAL,
        USSD_MODIFIED_TO_DIAL,
        USSD_MODIFIED_TO_SS,
        USSD_MODIFIED_TO_USSD,
        SS_MODIFIED_TO_DIAL,
        SS_MODIFIED_TO_DIAL_VIDEO,
        SS_MODIFIED_TO_USSD,
        SS_MODIFIED_TO_SS,
        SIM_ALREADY_POWERED_OFF,
        SIM_ALREADY_POWERED_ON,
        SIM_DATA_NOT_AVAILABLE,
        SIM_SAP_CONNECT_FAILURE,
        SIM_SAP_MSG_SIZE_TOO_LARGE,
        SIM_SAP_MSG_SIZE_TOO_SMALL,
        SIM_SAP_CONNECT_OK_CALL_ONGOING,
        LCE_NOT_SUPPORTED,
        NO_MEMORY,
        INTERNAL_ERR,
        SYSTEM_ERR,
        MODEM_ERR,
        INVALID_STATE,
        NO_RESOURCES,
        SIM_ERR,
        INVALID_ARGUMENTS,
        INVALID_SIM_STATE,
        INVALID_MODEM_STATE,
        INVALID_CALL_ID,
        NO_SMS_TO_ACK,
        NETWORK_ERR,
        REQUEST_RATE_LIMITED,
        SIM_BUSY,
        SIM_FULL,
        NETWORK_REJECT,
        OPERATION_NOT_ALLOWED,
        EMPTY_RECORD,
        INVALID_SMS_FORMAT,
        ENCODING_ERR,
        INVALID_SMSC_ADDRESS,
        NO_SUCH_ENTRY,
        NETWORK_NOT_READY,
        NOT_PROVISIONED,
        NO_SUBSCRIPTION,
        NO_NETWORK_FOUND,
        DEVICE_IN_USE,
        ABORTED,
        OEM_ERROR_1,
        OEM_ERROR_2,
        OEM_ERROR_3,
        OEM_ERROR_4,
        OEM_ERROR_5,
        OEM_ERROR_6,
        OEM_ERROR_7,
        OEM_ERROR_8,
        OEM_ERROR_9,
        OEM_ERROR_10,
        OEM_ERROR_11,
        OEM_ERROR_12,
        OEM_ERROR_13,
        OEM_ERROR_14,
        OEM_ERROR_15,
        OEM_ERROR_16,
        OEM_ERROR_17,
        OEM_ERROR_18,
        OEM_ERROR_19,
        OEM_ERROR_20,
        OEM_ERROR_21,
        OEM_ERROR_22,
        OEM_ERROR_23,
        OEM_ERROR_24,
        OEM_ERROR_25,
    }

    public CommandException(Error e) {
        super(e.toString());
        mError = e;
    }

    public CommandException(Error e, String errString) {
        super(errString);
        mError = e;
    }

    public Error getCommandError() {
        return mError;
    }
}
