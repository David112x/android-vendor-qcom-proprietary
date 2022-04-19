/*
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

public class RadioTech {

    private RadioTech() {}

    //RadioTechType
    /* Network type can be any, used when a specific service setting
        * is applicable for all access technologies, example: disable
        * VOLTE on all access technology - When multiple network types
        * are listed then ANY takes priority */
    public static final int RADIO_TECH_ANY = -1;

    /* Network type is unknown */
    public static final int RADIO_TECH_UNKNOWN = 0;

    /* Network type is GPRS */
    public static final int RADIO_TECH_GPRS = 1;

    /* Network type is EDGE */
    public static final int RADIO_TECH_EDGE = 2;

    /* Network type is UMTS */
    public static final int RADIO_TECH_UMTS = 3;

    /* Network type is IS95A */
    public static final int RADIO_TECH_IS95A = 4;

    /* Network type is IS95B */
    public static final int RADIO_TECH_IS95B = 5;

    /* Network type is 1xRTT*/
    public static final int RADIO_TECH_1xRTT = 6;

    /* Network type is EVDO revision 0*/
    public static final int RADIO_TECH_EVDO_0 = 7;

    /* Network type is EVDO revision A*/
    public static final int RADIO_TECH_EVDO_A = 8;

    /* Network type is HSDPA */
    public static final int RADIO_TECH_HSDPA = 9;

    /* Network type is HSUPA */
    public static final int RADIO_TECH_HSUPA = 10;

    /* Network type is HSPA */
    public static final int RADIO_TECH_HSPA = 11;

    /* Network type is EVDO revision B*/
    public static final int RADIO_TECH_EVDO_B = 12;

    /* Network type is eHRPD */
    public static final int RADIO_TECH_EHRPD = 13;

    /* Network type is LTE */
    public static final int RADIO_TECH_LTE = 14;

    /* Network type is HSPA+ */
    public static final int RADIO_TECH_HSPAP = 15;

    /* Network type is GSM */
    public static final int RADIO_TECH_GSM = 16;

    /* Network type is TD_SCDMA */
    public static final int RADIO_TECH_TD_SCDMA = 17;

    /* Network type is WIFI */
    public static final int RADIO_TECH_WIFI = 18;

    /* Network type is IWLAN */
    public static final int RADIO_TECH_IWLAN = 19;

    /* Network type is NR5G*/
    public static final int RADIO_TECH_NR5G = 20;

    /* Network type is INVALID */
    public static final int RADIO_TECH_INVALID = Integer.MAX_VALUE;
}
