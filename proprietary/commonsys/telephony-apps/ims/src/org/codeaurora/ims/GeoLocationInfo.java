/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

/* This class is used by:
 *
 * 1. UNSOL_REQUEST_GEOLOCATION to provide the latitude and longitude.
 *
 * 2. REQUEST_SEND_GEOLOCATION_INFO to send the Address information
 *    list corresponding to the latitude and longitude provided. The
 *    latitude and longitude values are passed back to lower layers as
 *    part of this request.
 *    The address information can be null if the HLOS location services
 *    APIs do not provide this information.
 *
 */
public final class GeoLocationInfo {

    private double mLatitude;
    private double mLongitude;

    public GeoLocationInfo(double latitude, double longitude) {
        mLatitude = latitude;
        mLongitude = longitude;
    }

    public double getLat(){
        return mLatitude;
    }

    public double getLon(){
        return mLongitude;
    }

    public String toString() {
        return "GeoLocationInfo latitude: " + mLatitude +
               " longitude: " + mLongitude;
    }
}
