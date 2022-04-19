/*
 * Copyright (c) 2014, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

public class ServiceStatus {

    // enum StatusType
    public static final int STATUS_DISABLED = 0;
    public static final int STATUS_PARTIALLY_ENABLED = 1;
    public static final int STATUS_ENABLED = 2;

    /* Example no modem support for IMS */
    public static final int STATUS_NOT_SUPPORTED = 3;


    public boolean isValid = false;
    public int type; /* Refer "enum CallType" from imsIF.proto file
                      * for possible types
                      */
    public int status; /*
                        * Overall Status (eg. enabled, disabled)
                        * best case for this type across all
                        * access techs
                        * Refer "enum StatusType" from imsIF.proto file
                        * for possible status values
                        */
    public byte[] userdata;
    public StatusForAccessTech[] accessTechStatus;
    public int rttMode;

    public boolean isRadioTech5G() {
        if (accessTechStatus == null || accessTechStatus[0] == null) {
            return false;
        }
        return (accessTechStatus[0].networkMode == RadioTech.RADIO_TECH_NR5G);
    }

    public static class StatusForAccessTech {
        public int networkMode; /* Refer "enum RadioTechType" from
                                 * imsIF.proto file for possible
                                 * networkMode values
                                 */
        public int status; /* Refer "enum StatusType" from imsIF.proto
                            * file for possible status values
                            */
        public int restrictCause;
        public int registered; /* Refer "enum RegState" from imsIF.proto
                                * file for possible values
                                */

        /**
         * @return string representation.
         */
        @Override
        public String toString() {
            return " mode = " + networkMode + " Status = " + status + " restrictCause = "
                    + restrictCause + " registered = " + registered;
        }
    }
}
