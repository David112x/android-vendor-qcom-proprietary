/* ======================================================================
*  Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*  ====================================================================*/

package com.qti.location.sdk;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.qti.location.sdk.IZatDBCommon.IZatCellInfo;
import com.qti.location.sdk.IZatDBCommon.IZatAPSSIDInfo;
import com.qti.location.sdk.IZatDBCommon.IZatApBsListStatus;
import com.qti.location.sdk.IZatDBCommon.IZatAPBSSpecialInfoTypes;

import android.location.Location;

/*
 * <p>Copyright (c) 2017 Qualcomm Technologies, Inc.</p>
 * <p>All Rights Reserved.</p>
 * <p>Confidential and Proprietary - Qualcomm Technologies, Inc</p>
 * <br/>
 * <p><b>IZatWiFiDBReceiver</b> interface @version 1.0.0 </p>
 */

/** @addtogroup IZatWiFiDBReceiver
@{ */

 /** APIs for
 * injecting a Wi-Fi AP location database to a  Qualcomm Location framework.
 *
 */
public abstract class IZatWiFiDBReceiver {
    protected IZatWiFiDBReceiverResponseListener mResponseListener = null;
    protected IZatWiFiDBReceiverResponseListenerExt mResponseListenerExt = null;
    /* Maximum number of elements allowed to inject in a single call to pushWiFiDB*/
    public static final int MAXIMUM_INJECTION_ELEMENTS = 500;

    /**
     * Constructor - IZatWiFiDBReceiver.
     *
     * @param listener Listener to receive Wi-Fi DB receiver
     *         responses. This parameter cannot be NULL, otherwise
     *         a {@link IZatIllegalArgumentException} is
     *         thrown.
     * @throws IZatIllegalArgumentException The listener parameter is NULL.
     * @return
     * None.
     */
    protected IZatWiFiDBReceiver(Object listener)
                                throws IZatIllegalArgumentException {
        if(null == listener) {
            throw new IZatIllegalArgumentException("Unable to obtain IZatWiFiDBReceiver instance");
        }

        if (listener instanceof IZatWiFiDBReceiverResponseListener) {
            mResponseListener = (IZatWiFiDBReceiverResponseListener) listener;
        } else if (listener instanceof IZatWiFiDBReceiverResponseListenerExt) {
            mResponseListenerExt = (IZatWiFiDBReceiverResponseListenerExt) listener;
        }
    }

    /**
     * Requests a list of access points.
     * <p>
     * This function enables Wi-Fi database providers to request a list of APs
     * that require location information.
     * </p>
     *
     * @param expire_in_days Number of days until
     *                        the associated location of an AP to be
     *                        fetched by this request (if
     *                        available) will expire. Optional parameter.
     *                        If 0 is provided, only APs that have an
     *                        already expired location or no
     *                        location associated with it will be
     *                        fetched.
     * @return
     * None.
     */
    public abstract void requestAPList(int expire_in_days);

    /**
     * Requests a list of access points for the last scan.
     * <p>
     * This function enables Wi-Fi database providers to request a list of APs
     * that require location information.
     * </p>
     *
     * @return
     * None.
     */
    public abstract void requestScanList();

    /**
     * Requests a Wi-Fi database update.
     * <p>
     * This function enables a Wi-Fi database provider to insert a list of APs
     * with location information.
     * </p>
     *
     * @param location_data Location information of access points.
     *                      If not available, a NULL/empty list can be
     *                      provided. The maximum number of elements is allowed,
     *                      defined by {@link MAXIMUM_INJECTION_ELEMENTS}.
     * @param special_info Special information on an access point. If
     *                     not available, a NULL/empty list can be
     *                     provided. The maximum number of elements is allowed,
     *                      defined by {@link MAXIMUM_INJECTION_ELEMENTS}.
     * @param days_valid Number of days for which location_data and
     *                   special_info will be valid. Optional
     *                   parameter. Defaults to 15 days if 0 is
     *                   provided.
     *
     * @return
     * None.
     */
    public abstract void pushWiFiDB(List<IZatAPLocationData> location_data,
                                    List<IZatAPSpecialInfo> special_info,
                                    int days_valid);

    /**
     * Send AP positions after a onLookupRequest is received
     * <p>
     * This function enables a Wi-Fi database provider to insert a list of APs
     * with location information upon lookup request.
     * </p>
     *
     * @param location_data Location information of access points.
     *                      If not available, a NULL/empty list can be
     *                      provided.
     * @param special_info Special information on an access point. If
     *                     not available, a NULL/empty list can be
     *                     provided.
     *
     * @return
     * None.
     */
    public abstract void pushLookupResult(List<IZatAPLocationData> location_data,
                                          List<IZatAPSpecialInfo> special_info);

/** @} */ /* end_addtogroup IZatWiFiDBReceiver */

/** @addtogroup IZatWiFiDBReceiver
@{ */

    /**
     * Interface class IZatWiFiDBReceiverResponseListener.
     *
     * This interface
     * receives responses from a Wi-Fi database receiver in a Qualcomm
     * Location framework.
     */
    @Deprecated
    public interface IZatWiFiDBReceiverResponseListener {
        /**
         * Response to an AP list request.
         * <p>
         * This API is called by the underlying service back
         * to applications when a list of APs is available.
         * Applications should implement this interface.</p>
         *
         * @param ap_list List of APs.
         *
         * @return
         * None.
         */
        void onAPListAvailable(List<IZatAPInfo> ap_list);

        /**
         * Response for an AP location injection request.
         * <p>
         * This API is called by the underlying service back
         * to applications when an AP location injection completes.
         * Applications should implement this interface.</p>
         *
         * @param is_success Injection of AP locations success or
         *                   failure.
         * @param error Error details if the AP location injection
         *                   failed.
         *
         * @return
         * None.
         */
        void onStatusUpdate(boolean is_success, String error);

        /**
         * Service request to a Wi-Fi database provider.
         * <p>
         * This API is called by the underlying service back
         * to applications when they need service. Applications should
         * implement this interface.</p>
         *
         * @return
         * None.
         */
        void onServiceRequest();
    }

     /**
      * Interface class IZatWiFiDBReceiverResponseListenerExt.
      *
      * This interface
      * receives responses from a Wi-Fi database receiver in a Qualcomm
      * Location framework.
      */
     public interface IZatWiFiDBReceiverResponseListenerExt {
         /**
          * Response to an AP list request.
          * <p>
          * This API is called by the underlying service back
          * to applications when a list of APs is available.
          * Applications should implement this interface.</p>
          *
          * @param ap_list List of APs.
          * @param ap_status Status of the AP list.
          * @param location Location of area of interest.
          *
          * @return
          * None.
          */
         void onAPListAvailable(List<IZatAPInfo> ap_list,
                                IZatApBsListStatus ap_status,
                                Location location);

         /**
          * Lookup request from positioning engine
          * <p>
          * This API is called by the underlying service back
          * to applications when the positioning engine needs
          * the location of a set of APs.
          * Applications should implement this interface.</p>
          *
          * @param ap_list List of APs.
          * @param location Location of area of interest.
          *
          * @return
          * None.
          */
         void onLookupRequest(List<IZatAPInfo> ap_list, Location location);

         /**
          * Response for an AP location injection request.
          * <p>
          * This API is called by the underlying service back
          * to applications when an AP location injection completes.
          * Applications should implement this interface.</p>
          *
          * @param is_success Injection of AP locations success or
          *                   failure.
          * @param error Error details if the AP location injection
          *                   failed.
          *
          * @return
          * None.
          */
         void onStatusUpdate(boolean is_success, String error);

         /**
          * Service request to a Wi-Fi database provider.
          * <p>
          * This API is called by the underlying service back
          * to applications when they need service. Applications should
          * implement this interface.</p>
          *
          * @return
          * None.
          */
         void onServiceRequest();
     }
    /** @} */ /* end_addtogroup IZatWiFiDBReceiver */

/** @addtogroup IZatWiFiDBReceiver
@{ */

    /**
     * Class IZatAPInfo.
     */
    public static class IZatAPInfo {
        String mMacAddress;
        IZatAPInfoExtra mExtra;
        protected int mApLocalTimestamp;

        /**
         * Enumeration of fdal status
         */
        public enum IZatFdalStatus {
            NOT_IN_FDAL(0),    /**< The AP is NOT in the FDAL cache */
            IN_FDAL(1),        /**< The AP is in the FDAL cache */
            BLACKLISTED(2),    /**< The AP is blacklisted */
            NA(3);              /**< FDAL status does not apply in this case or unknown */

            // Enforce corrects value from C enum definition
            private final int cValue;

            private IZatFdalStatus(int cValue) {
                this.cValue = cValue;
            }

            private static final Map<Integer, IZatFdalStatus> typesByValue = new HashMap<Integer,
                    IZatFdalStatus>();

            static {
                for (IZatFdalStatus type : IZatFdalStatus.values()) {
                    typesByValue.put(type.cValue, type);
                }
            }

            protected static IZatFdalStatus fromInt(int cValue) {
                return typesByValue.get(cValue);
            }
        }

        protected IZatFdalStatus mFdalStatus = IZatFdalStatus.NA;

        /**
         * IZatAPInfo constructor.
         *
         * @param mac MAC address if the Wi-Fi access point is in the form of a
         *            six-byte MAC address: {@code XX:XX:XX:XX:XX:XX}
         * @param extra Extra context information for this AP.
         *
         * @return
         * None.
         */
        public IZatAPInfo(String mac, int timestamp, IZatAPInfoExtra extra) {
            mMacAddress = mac;
            mApLocalTimestamp = timestamp;
            mExtra = extra;
        }

        protected void setFdalStatus(int fdal_status) {
            mFdalStatus = IZatFdalStatus.fromInt(fdal_status);
        }

        /**
         * Gets the MAC address of this AP.
         *
         * @return String in the form of a six-byte MAC address: {@code
         *         XX:XX:XX:XX:XX:XX}
         */
        public String getMacAddress() {
            return mMacAddress;
        }

        /**
         * Gets the timestamp of the AP
         *
         * @return Int timestamp
         */
        public int getTimestamp() {
            return mApLocalTimestamp;
        }

        /**
         * Gets extra context information on this AP.
         *
         * @return IZatAPInfoExtra.
         */
        public IZatAPInfoExtra getExtra() {
            return mExtra;
        }

        /**
         * Gets the FDAL status of this AP.
         *
         * @return IZatFdalStatus.
         */
        public IZatFdalStatus getFDALStatus() {
            return mFdalStatus;
        }

        /**
         * Checks whether extra information for this AP is available
         *
         * @return Boolean TRUE or FALSE.
         */
        public boolean isExtraAvailable() {
            return (null == mExtra ? false : mExtra.isAvailable());
        }
    }

/** @} */ /* end_addtogroup IZatWiFiDBReceiver */

/** @addtogroup IZatWiFiDBReceiver
@{ */

    /**
     * Class IZatAPInfoExtra.
     */
    public static class IZatAPInfoExtra {
        IZatCellInfo mCellInfo;
        IZatAPSSIDInfo mSSID;

        /**
         *  Constructor - IZatAPInfoExtra.
         *
         * @param cellInfo Cell information in which this AP is observed.
         * @param ssid SSID of this access point.
         *
         * @return
         * None.
         */
        public IZatAPInfoExtra(IZatCellInfo cellInfo, IZatAPSSIDInfo ssid) {
            mCellInfo = cellInfo;
            mSSID = ssid;
        }

        /**
         * Gets the cell information.
         * <p>
         *  Gets the cell information in which this AP is observed. This information
         *  is currently not supported.</p>
         *
         * @return IZatCellInfo.
         */
        public IZatCellInfo getCellInfo() {
            return mCellInfo;
        }

        /**
         * Gets the SSID.
         * <p>
         *  Returns the service set identifier (SSID) of this AP.</p>
         *
         * @return IZatAPSSIDInfo.
         */
        public IZatAPSSIDInfo getSSID() {
            return mSSID;
        }

        /**
         * Checks whether extra context information is available for this AP.
         *
         * @return Boolean TRUE or FALSE.
         */
        public boolean isAvailable() {
            return ((null != mSSID) || (null != mCellInfo));
        }
    }

/** @} */ /* end_addtogroup IZatWiFiDBReceiver */

/** @addtogroup IZatWiFiDBReceiver
@{ */

    /**
     * Class IZatAPLocationData.
     */
    public static class IZatAPLocationData {
        private String mMacAddress;
        private float mLatitude;
        private float mLongitude;
        private float mMaxAntenaRange;
        private float mHorizontalError;

        /**
         * Enumeration of reliablity types.
         */
        public enum IZatReliablityTypes {
            VERY_LOW,    /**< The probability of a position outlier is 1
                              1 in one hundred or even more likely. */
            LOW,         /**< The probability of a position outlier is about
                              1 in one thousand. */
            MEDIUM,      /**< The probability of a position outlier is about
                              1 in 100 thousand. */
            HIGH,        /**< The  probability of a position outlier is about
                              1 in 10 million. */
            VERY_HIGH    /**< The probability of a position outlier is about 1 in
                              1 in a thousand million. \n
                              Until sufficient experience is obtained, the reliability
                              input value should remain unset or set to LOW. */
        }
        IZatReliablityTypes mReliability;

        public static final int IZAT_AP_LOC_WITH_LAT_LON         = 0x0;
        public static final int IZAT_AP_LOC_MAR_VALID            = 0x1;
        public static final int IZAT_AP_LOC_HORIZONTAL_ERR_VALID = 0x2;
        public static final int IZAT_AP_LOC_RELIABILITY_VALID    = 0x4;
        int mValidBits;

        /**
         *
         * @param mac
         */
        public IZatAPLocationData(String mac) {
            mMacAddress = mac;
        }

        /**
         * Constructor - IZatAPLocationData.
         *
         * @param mac MAC address in the form of a six-byte MAC address string: \n
         *         {@code XX:XX:XX:XX:XX:XX}
         * @param latitude Latitude of the AP.
         * @param longitude Longitude of the AP.
         * @return
         * None.
         */
        public IZatAPLocationData(String mac, float latitude, float longitude) {
            mMacAddress = mac;
            mLatitude = latitude;
            mLongitude = longitude;
            mValidBits = IZAT_AP_LOC_WITH_LAT_LON;
        }

        /**
         * Sets the latitude.
         *
         * @param latitude Latitude of the AP.
         *
         * @return
         * None.
         */
        public void setLatitude(float latitude) {
            mLatitude = latitude;
        }

        /**
         * Sets the longitude.
         *
         * @param longitude Longitude of the AP.
         *
         * @return
         * None.
         */
        public void setLongitude(float longitude) {
            mLongitude = longitude;
        }

        /**
         * Sets the maximum antenna range of the AP.
         *
         * @param mar Maximum antenna range of the AP.
         *
         * @return
         * None.
         */
        public void setMaxAntenaRange(float mar) {
            mMaxAntenaRange = mar;
            mValidBits |= IZAT_AP_LOC_MAR_VALID;
        }

        /**
         * Sets the horizontal error.
         *
         * @param he Horizontal error in meters at 99% confidence. Use 25m if unknown.
         *
         * @return
         * None.
         */
        public void setHorizontalError(float he) {
            mHorizontalError = he;
            mValidBits |= IZAT_AP_LOC_HORIZONTAL_ERR_VALID;
        }

        /**
         * Sets the reliability of the provided location information.
         *
         * @param reliability Reliability values; see enum IZatReliablityTypes.
         *
         * @return
         * None.
         */
        public void setReliability(IZatReliablityTypes reliability) {
            mReliability = reliability;
            mValidBits |= IZAT_AP_LOC_RELIABILITY_VALID;
        }

        /**
         * Gets the MAC address.
         * <p>
         *  Gets the MAC address in the form of a six-byte MAC address string:
         *  {@code XX:XX:XX:XX:XX:XX}
         * @return
         * MAC address string.
         */
        public String getMacAddress() {
            return mMacAddress;
        }

        /**
         * Gets the latitude.
         *
         * @return
         * Latitude as a float value.
         */
        public float getLatitude() {
            return mLatitude;
        }

        /**
         * Gets the longitude.
         *
         * @return
         * Longitude as a float value.
         */
        public float getLongitude() {
            return mLongitude;
        }

        /**
         * Gets the maximum antenna range.
         *
         * @throws IZatStaleDataException Range data is stale.
         * @return
         * Maximum antenna range as a float value.
         */
        public float getMaxAntenaRange() throws IZatStaleDataException {
            if(0 == (IZAT_AP_LOC_MAR_VALID & mValidBits)) {
                throw new IZatStaleDataException("Maximum Antena Range information is not valid");
            }
            return mMaxAntenaRange;
        }

        /**
         * Gets the horizontal error.
         *
         * @throws IZatStaleDataException Error data is stale.
         * @return
         * Horizontal error as a float value.
         */
        public float getHorizontalError() throws IZatStaleDataException {
            if(0 == (IZAT_AP_LOC_HORIZONTAL_ERR_VALID & mValidBits)) {
                throw new IZatStaleDataException("Horizontal error information is not valid");
            }
            return mHorizontalError;
        }

        /**
         * Gets the reliability.
         *
         * @throws IZatStaleDataException  Reliability data is stale.
         * @return
         * A value from IZatReliablityTypes.
         */
        public IZatReliablityTypes getReliability() throws IZatStaleDataException {
            if(0 == (IZAT_AP_LOC_RELIABILITY_VALID & mValidBits)) {
                throw new IZatStaleDataException("Reliability information is not valid");
            }
            return mReliability;
        }
    }

/** @} */ /* end_addtogroup IZatWiFiDBReceiver */

/** @addtogroup IZatWiFiDBReceiver
@{ */

    /**
     * class IZatAPSpecialInfo.
     */
    public static class IZatAPSpecialInfo {
        /**
         * Enumeration of spacial info types.
         */
        @Deprecated
        public enum IZatAPSpecialInfoTypes {
            /** No information is available.
             */
            NO_INFO_AVAILABLE,
            /** Moving APor BS.
             */
            MOVING_AP
        }
        public final String mMacAddress;
        public final IZatAPBSSpecialInfoTypes mInfo;

        /**
         * Constructor - IZatAPSpecialInfo.
         *
         * @param mac MAC address in the form of a six-byte MAC address string:
         *         {@code XX:XX:XX:XX:XX:XX}
         * @param info Information on this AP.
         * @return
         * None.
         */
        @Deprecated
        public IZatAPSpecialInfo(String mac, IZatAPSpecialInfoTypes info) {
            mMacAddress = mac;
            if (IZatAPSpecialInfoTypes.MOVING_AP == info) {
                mInfo = IZatAPBSSpecialInfoTypes.MOVING_AP_BS;
            } else {
                mInfo = IZatAPBSSpecialInfoTypes.NO_INFO_AVAILABLE;
            }
        }

        /**
         * Constructor - IZatAPSpecialInfo.
         *
         * @param mac MAC address in the form of a six-byte MAC address string:
         *         {@code XX:XX:XX:XX:XX:XX}
         * @param info Information on this AP.
         * @return
         * None.
         */
        @Deprecated
        public IZatAPSpecialInfo(String mac, IZatAPBSSpecialInfoTypes info) {
            mMacAddress = mac;
            mInfo = info;
        }
    }

}
/** @} */ /* end_addtogroup IZatWiFiDBReceiver */