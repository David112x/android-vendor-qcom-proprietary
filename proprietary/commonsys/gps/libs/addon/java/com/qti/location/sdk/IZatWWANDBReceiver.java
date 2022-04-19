/* ======================================================================
*  Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*  ====================================================================*/

package com.qti.location.sdk;

import java.util.List;
import com.qti.location.sdk.IZatDBCommon.IZatCellInfo;
import com.qti.location.sdk.IZatDBCommon.IZatCellTypes;
import com.qti.location.sdk.IZatStaleDataException;
import com.qti.location.sdk.IZatDBCommon.IZatAPBSSpecialInfoTypes;
import com.qti.location.sdk.IZatDBCommon.IZatApBsListStatus;

import android.location.Location;

import static com.qti.location.sdk.IZatDBCommon.HORIZONTAL_CONFIDENCE;
import static com.qti.location.sdk.IZatDBCommon.SPEED_CONFIDENCE;
import static com.qti.location.sdk.IZatDBCommon.VERTICAL_CONFIDENCE;

/*
 * <p>Copyright (c) 2017 Qualcomm Technologies, Inc.</p>
 * <p>All Rights Reserved.</p>
 * <p>Confidential and Proprietary - Qualcomm Technologies, Inc</p>
 * <br/>
 * <p><b>IZatWWANDBReceiver</b> interface @version 1.0.0 <p>
 */

/** @addtogroup IZatWWANDBReceiver
@{ */

/** API for
 * injecting a WWAN Cell Location database to a Qualcomm Location framework.
 */
public abstract class IZatWWANDBReceiver {
    protected IZatWWANDBReceiverResponseListener mResponseListener = null;
    protected IZatWWANDBReceiverResponseListenerExt mResponseListenerExt = null;
    /* Maximum number of elements allowed to inject in a single call to pushWWANDB*/
    public static final int MAXIMUM_INJECTION_ELEMENTS = 20;

    /**
     * <p>
     * Constructor - IZatWWANDBReceiver. </p>
     *
     * @param listener Listener to receive WWAN DB Receiver
     *         responses. This parameter cannot be NULL, otherwise
     *         a {@link IZatIllegalArgumentException} is
     *         thrown.
     * @throws IZatIllegalArgumentException The listener parameter is NULL.
     * @return
     * None.
     */
    protected IZatWWANDBReceiver(Object listener)
                                throws IZatIllegalArgumentException {
        if(null == listener) {
            throw new IZatIllegalArgumentException("Unable to obtain IZatWWANDBReceiver instance");
        }
        if (listener instanceof IZatWWANDBReceiverResponseListener) {
            mResponseListener = (IZatWWANDBReceiverResponseListener) listener;
        } else if (listener instanceof IZatWWANDBReceiverResponseListenerExt) {
            mResponseListenerExt = (IZatWWANDBReceiverResponseListenerExt) listener;
        }
    }

    /**
     * Requests list of base stations.
     * <p>
     * This enables WWAN database providers to request a list of BSs
     * that require location information.
     * </p>
     *
     * @param expire_in_days Number of days until
     *                        the associated location of a BS (if
     *                        available) will expire.
     *                        Optional parameter.
     *                        If 0 is provided, only BSs that have an
     *                        already expired location or no
     *                        location associated with it will be
     *                        fetched.
     * @return
     * None.
     */
    public abstract void requestBSList(int expire_in_days);

    /**
     * Requests a WWAN DB update.
     * <p>
     * This enables a WWAN database provider to insert a list of BSs
     * with location information.
     * </p>
     *
     * @param location_data Location information of base stations.
     *                          If not available, a NULL/empty list can be
     *                          provided. The maximum number of elements is allowed,
     *                          defined by {@link MAXIMUM_INJECTION_ELEMENTS}.
     * @param special_info Special information on a base station. If
     *                         not available, a NULL/empty list can be
     *                         provided. The maximum number of elements is allowed,
     *                         defined by {@link MAXIMUM_INJECTION_ELEMENTS}.
     * @param days_valid  Number of days for which location_data and
     *                   special_info will be valid. Optional
     *                   parameter. Defaults to 15 days if 0 is
     *                   provided.
     * @return
     * None.
     */
    public abstract void pushWWANDB(List<IZatBSLocationData> location_data,
                                    List<IZatBSSpecialInfo> special_info,
                                    int days_valid);

/** @} */ /* end_addtogroup IZatWWANDBReceiver */

/** @addtogroup IZatWWANDBReceiver
@{ */


    // For backwards compatibility only
    // Same structure as new common IZatCellInfo
    @Deprecated
    public static class IZatBSCellInfo extends IZatCellInfo {
        @Deprecated
        public final int mRegionID1;
        @Deprecated
        public final int mRegionID2;
        @Deprecated
        public final int mRegionID3;
        @Deprecated
        public final int mRegionID4;
        @Deprecated
        public IZatCellTypes mType;

        public IZatBSCellInfo(int regionID1, int regionID2, int regionID3,
                            int regionID4, IZatCellTypes type) {
            super(regionID1, regionID2, regionID3, regionID4, type);
            mRegionID1 = regionID1;
            mRegionID2 = regionID2;
            mRegionID3 = regionID3;
            mRegionID4 = regionID4;
            mType = type;
        }

        public IZatBSCellInfo(IZatCellInfo cellInfo) {
            super(cellInfo.mRegionID1, cellInfo.mRegionID2, cellInfo.mRegionID3,
                  cellInfo.mRegionID4, cellInfo.mType);
            mRegionID1 = cellInfo.mRegionID1;
            mRegionID2 = cellInfo.mRegionID2;
            mRegionID3 = cellInfo.mRegionID3;
            mRegionID4 = cellInfo.mRegionID4;
            mType = cellInfo.mType;
        }
    }

    @Deprecated
    public static class IZatBSInfo {
        IZatBSCellInfo mCellInfo;

        public IZatBSInfo(IZatBSCellInfo cellInfo) {
            mCellInfo = cellInfo;
        }

        public IZatBSInfo(IZatCellInfo cellInfo) {
            mCellInfo = new IZatBSCellInfo(cellInfo);
        }

        public IZatBSCellInfo getCellInfo() {
            return mCellInfo;
        }
    }

     /**
     * Interface class IZatWWANDBReceiverResponseListener.
     *
     * <p>This interface
     * receives responses from a WWAN database receiver in a Qualcomm
     * Location framework.</p>
     */
     @Deprecated
    public interface IZatWWANDBReceiverResponseListener {

        /**
         * Response to a BS list request.
         * <p>
         * This API is called by the underlying service back
         * to applications when a list of BSs is available.
         * Applications should implement this interface.</p>
         *
         * @param bs_list   List of BSs.
         *
         * @return
         * None.
         */
        void onBSListAvailable(List<IZatBSInfo> bs_list);


        /**
         * Response for a BS location injection request.
         * <p>
         * This API is called by the underlying service back
         * to applications when a BS location injection completes.
         * Applications should implement this interface.</p>
         *
         * @param is_success Injection of BS locations success or
         *                        failure.
         * @param error Error details if the BS location injection
         *                   failed.
         *
         * @return
         * None.
         */
        void onStatusUpdate(boolean is_success, String error);

        /**
         * Service request to a WWAN DB provider.
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
     * Interface class IZatWWANDBReceiverResponseListener.
     *
     * <p>This interface
     * receives responses from a WWAN database receiver in a Qualcomm
     * Location framework.</p>
     */
    public interface IZatWWANDBReceiverResponseListenerExt{

        /**
         * Response to a BS list request.
         * <p>
         * This API is called by the underlying service back
         * to applications when a list of BSs is available.
         * Applications should implement this interface.</p>
         *
         * @param bs_list   List of BSs.
         * @param bs_status Status of the BS list.
         * @param location  Location of area of interest.
         *
         * @return
         * None.
         */
        void onBSListAvailable(List<IZatCellInfo> bs_list,
                               IZatApBsListStatus bs_status,
                               Location location);

        /**
         * Response for a BS location injection request.
         * <p>
         * This API is called by the underlying service back
         * to applications when a BS location injection completes.
         * Applications should implement this interface.</p>
         *
         * @param is_success Injection of BS locations success or
         *                        failure.
         * @param error Error details if the BS location injection
         *                   failed.
         *
         * @return
         * None.
         */
        void onStatusUpdate(boolean is_success, String error);

        /**
         * Service request to a WWAN DB provider.
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

/** @} */ /* end_addtogroup IZatWWANDBReceiver */


/** @addtogroup IZatWWANDBReceiver
@{ */

    /**
     * Class IZatBSLocationData.
     */
    public abstract static class IZatBSLocationDataBase {
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
        protected IZatCellInfo mIZatCellInfo;
        protected Location mLocation;
        protected IZatReliablityTypes mHorizontalReliability;
        protected IZatReliablityTypes mAltitudeReliability;
        protected int mValidBits;

        // BS Location masks
        public static final int IZAT_BS_LOC_WITH_LOCATION = 0x1;
        public static final int IZAT_BS_LOC_WITH_CELL_INFO = 0x2;
        public static final int IZAT_BS_LOC_HORIZONTAL_RELIABILITY_VALID = 0x4;
        public static final int IZAT_BS_LOC_ALTITUDE_RELIABILITY_VALID = 0x8;

        /**
         *
         * @param iZatCellInfo
         * @param location
         * @param horizontalReliability
         * @param altitudeReliability
         */
        public IZatBSLocationDataBase(IZatCellInfo iZatCellInfo, Location location,
                                      IZatReliablityTypes horizontalReliability,
                                      IZatBSLocationData.IZatReliablityTypes altitudeReliability) {
            this.mIZatCellInfo = iZatCellInfo;
            this.mLocation = location;
            this.mHorizontalReliability = horizontalReliability;
            this.mAltitudeReliability = altitudeReliability;
            mValidBits = 0;
            mValidBits |= IZAT_BS_LOC_WITH_LOCATION;
            mValidBits |= IZAT_BS_LOC_WITH_CELL_INFO;
            mValidBits |= IZAT_BS_LOC_HORIZONTAL_RELIABILITY_VALID;
            mValidBits |= IZAT_BS_LOC_ALTITUDE_RELIABILITY_VALID;
        }

        /**
         *
         * @param iZatCellInfo
         * @param location
         */
        public IZatBSLocationDataBase(IZatCellInfo iZatCellInfo, Location location) {
            this.mIZatCellInfo = iZatCellInfo;
            this.mLocation = location;
            mValidBits = 0;
            mValidBits |= IZAT_BS_LOC_WITH_LOCATION;
            mValidBits |= IZAT_BS_LOC_WITH_CELL_INFO;
        }

        public IZatBSLocationDataBase(IZatBSLocationDataBase bsLocationDataBase) {
            this.mIZatCellInfo = bsLocationDataBase.mIZatCellInfo;
            this.mLocation = bsLocationDataBase.mLocation;
            this.mHorizontalReliability = bsLocationDataBase.mHorizontalReliability;
            this.mAltitudeReliability = bsLocationDataBase.mAltitudeReliability;
            this.mValidBits = bsLocationDataBase.mValidBits;
        }

        /**
         *
         * @param horizontalReliability
         */
        public void setHorizontalReliability(IZatReliablityTypes horizontalReliability) {
            this.mHorizontalReliability = horizontalReliability;
            mValidBits |= IZAT_BS_LOC_HORIZONTAL_RELIABILITY_VALID;
        }

        /**
         *
         * @param altitudeReliability
         */
        public void setAltitudeReliability(IZatReliablityTypes altitudeReliability) {
            this.mAltitudeReliability = altitudeReliability;
            mValidBits |= IZAT_BS_LOC_ALTITUDE_RELIABILITY_VALID;
        }


        /**
         *
         * @return
         * @throws IZatStaleDataException
         */
        public Location getLocation() throws IZatStaleDataException {
            if(0 == (IZAT_BS_LOC_WITH_LOCATION & mValidBits)) {
                throw new IZatStaleDataException("Location information is not valid");
            }
            return mLocation;
        }

        /**
         *
         * @return
         * @throws IZatStaleDataException
         */
        public IZatCellInfo getIZatCellInfo() throws IZatStaleDataException {
            if(0 == (IZAT_BS_LOC_WITH_CELL_INFO & mValidBits)) {
                throw new IZatStaleDataException("Cell info information is not valid");
            }
            return mIZatCellInfo;
        }

        /**
         *
         * @return
         * @throws IZatStaleDataException
         */
        public IZatReliablityTypes getHorizontalReliability() throws IZatStaleDataException {
            if(0 == (IZAT_BS_LOC_HORIZONTAL_RELIABILITY_VALID & mValidBits)) {
                throw new IZatStaleDataException("Horizontal reliability information is not valid");
            }
            return mHorizontalReliability;
        }

        /**
         *
         * @return
         * @throws IZatStaleDataException
         */
        public IZatReliablityTypes getAltitudeReliability() throws IZatStaleDataException {
            if(0 == (IZAT_BS_LOC_ALTITUDE_RELIABILITY_VALID & mValidBits)) {
                throw new IZatStaleDataException("Altitude reliability information is not valid");
            }
            return mAltitudeReliability;
        }
    }

    public static class IZatBSLocationData extends IZatBSLocationDataBase {
        private float mHorizontalCoverageRadius;

        public static final int IZAT_BS_LOC_COVERAGE_VALID = 0x10;

        /**
         * @param iZatCellInfo
         * @param location
         */
        public IZatBSLocationData(IZatCellInfo iZatCellInfo, Location location) {
            super(iZatCellInfo, location);
        }

        /**
         * @param iZatCellInfo
         * @param location
         * @param horizontalReliability
         * @param altitudeReliability
         */
        public IZatBSLocationData(IZatCellInfo iZatCellInfo, Location location,
                                  IZatReliablityTypes horizontalReliability,
                                  IZatReliablityTypes altitudeReliability) {
            super(iZatCellInfo, location, horizontalReliability, altitudeReliability);
        }

        /**
         *
         * @param iZatCellInfo
         * @param location
         * @param horizontalReliability
         * @param altitudeReliability
         * @param horizontalCoverageRadius
         */
        public IZatBSLocationData(IZatCellInfo iZatCellInfo, Location location,
                                  IZatReliablityTypes horizontalReliability,
                                  IZatReliablityTypes altitudeReliability,
                                  float horizontalCoverageRadius) {
            super(iZatCellInfo, location, horizontalReliability, altitudeReliability);
            this.mHorizontalCoverageRadius = horizontalCoverageRadius;
            mValidBits |= IZAT_BS_LOC_COVERAGE_VALID;
        }

        private static IZatBSLocationData makeIZatBSLocationData(int cellType,
                                                                int cellRegionID1,
                                                                int cellRegionID2,
                                                                int cellRegionID3,
                                                                int cellRegionID4,
                                                                float latitude, float longitude) {
            IZatCellInfo cellInfo = new IZatCellInfo(cellRegionID1,
                    cellRegionID2,
                    cellRegionID3,
                    cellRegionID4,
                    IZatCellTypes.values()[cellType]);
            Location location = new Location("");
            location.setLatitude(latitude);
            location.setLongitude(longitude);
            return new IZatBSLocationData(cellInfo, location);
        }

        public IZatBSLocationData(IZatBSLocationData bsLocationData) {
            super(bsLocationData);
            mHorizontalCoverageRadius = bsLocationData.mHorizontalCoverageRadius;
            mValidBits |= IZAT_BS_LOC_COVERAGE_VALID;
        }

        @Deprecated
        public IZatBSLocationData(int cellType,
                                  int cellRegionID1, int cellRegionID2,
                                  int cellRegionID3, int cellRegionID4,
                                  float latitude, float longitude) {
            this(makeIZatBSLocationData(cellType, cellRegionID1, cellRegionID2, cellRegionID3,
                    cellRegionID4, latitude, longitude));
        }
        /**
         *
         * @return
         */
        public float getHorizontalCoverageRadius() {
            if(0 == (IZAT_BS_LOC_COVERAGE_VALID & mValidBits)) {
                throw new IZatStaleDataException("Horizontal coverage information is not valid");
            }
            return mHorizontalCoverageRadius;
        }

        /**
         * @deprecated Use getLocation().getLatitude()
         * @return
         */
        @Deprecated
        public float getLatitude() {
            return (float) mLocation.getLatitude();
        }

        /**
         * @deprecated Use getLocation().getLongitude()
         * @return
         */
        @Deprecated
        public float getLongitude() {
            return (float) mLocation.getLongitude();
        }

        /**
         * @deprecated Use getLocation().getAltitude()
         * @return
         */
        @Deprecated
        public float getAltitude() {
            return (float) mLocation.getAltitude();
        }

        /**
         * @deprecated Use getLocation().getVerticalAccuracyMeters()
         * @return
         */
        @Deprecated
        public float getAltitudeUncertainty() {
            return (float) mLocation.getVerticalAccuracyMeters();
        }

        /**
         * @deprecated Confidence is a constant now.
         * @return
         */
        @Deprecated
        public float getAltitudeConfidence() {
            return VERTICAL_CONFIDENCE;
        }

        /**
         * @deprecated Confidence is a constant now.
         * @return
         */
        @Deprecated
        public float getHorizontalConfidence() {
            return HORIZONTAL_CONFIDENCE;
        }

        /**
         * Gets cell region ID 1.
         *
         * @return CellRegionID1 as an integer.
         */
        @Deprecated
        public int getCellRegionID1() {
            return mIZatCellInfo.getRegionID1();
        }

        /**
         * Gets cell region ID 2.
         *
         * @return CellRegionID2 as an integer.
         */
        @Deprecated
        public int getCellRegionID2() {
            return mIZatCellInfo.getRegionID2();
        }

        /**
         * Gets cell region ID 3.
         *
         * @return CellRegionID3 as an integer.
         */
        @Deprecated
        public int getCellRegionID3() {
            return mIZatCellInfo.getRegionID3();
        }

        /**
         * Gets cell region ID 4.
         *
         * @return CellRegionID4 as an integer.
         */
        @Deprecated
        public int getCellRegionID4() {
            return mIZatCellInfo.getRegionID4();
        }

        /**
         *
         * @param location
         */
        public void setLocation(Location location) {
            mLocation = location;
        }

        /**
         * Sets the cell type.
         *
         * @param ct Cell type.
         *
         * @return None.
         */
        @Deprecated
        public void setCellType(int ct) {
            if (ct >= 0 && ct <= 4) {
                mIZatCellInfo.setType(IZatCellTypes.values()[ct]);
            }
        }

        /**
         * Sets the latitude
         *
         * @param latitude Latitude of the BS.
         *
         * @return None.
         */
        @Deprecated
        public void setLatitude(float latitude) {
            if (null != mLocation) {
                mLocation.setLatitude(latitude);
            }
        }

        /**
         * Sets the longitude.
         *
         * @param longitude Longitude of the BS.
         *
         * @return None.
         */
        @Deprecated
        public void setLongitude(float longitude) {
            if (null != mLocation) {
                mLocation.setLongitude(longitude);
            }
        }

        /**
         * Sets the horizontal coverage radius.
         *
         * @param hcr Horizontal coverage radius.
         *
         * @return None.
         */
        @Deprecated
        public void setHorizontalCoverageRadius(float hcr) {
            mHorizontalCoverageRadius = hcr;
            mValidBits |= IZAT_BS_LOC_COVERAGE_VALID;
        }

        /**
         * Sets the horizontal confidence of the location information provided.
         *  Range 0-100
         *  </p>
         * @param hc Horizontal confidence. Range: 0 to 100.
         *
         * @return None.
         */
        @Deprecated
        public void setHorizontalConfidence(int hc) {
            if (null != mLocation) {
                mLocation.setAccuracy(hc);
            }
        }

        /**
         * Sets the horizontal reliability. See enum IZatReliablityTypes.
         *
         * @param reliability Horizontal reliability.
         *
         * @return None.
         */
        @Deprecated
        public void setHorizontalReliability(IZatReliablityTypes reliability) {
            mHorizontalReliability = reliability;
            mValidBits |= IZAT_BS_LOC_HORIZONTAL_RELIABILITY_VALID;
        }

        /**
         * Sets the altitude.
         *
         * @param alt Altitude.
         *
         * @return None.
         */
        @Deprecated
        public void setAltitude(float alt) {
            if (null != mLocation) {
                mLocation.setAltitude(alt);
            }
        }

        /**
         * Sets the altitude uncertainty.
         *
         * @param alu Altitude uncertainty.
         *
         * @return None.
         */
        @Deprecated
        public void setAltitudeUncertainty(float alu) {
            if (null != mLocation) {
                mLocation.setVerticalAccuracyMeters(alu);
            }
        }

        /**
         * Sets the altitude confidence.
         *
         * @param alc Altitude confidence.
         *
         * @return None.
         */
        @Deprecated
        public void setAltitudeConfidence(int alc) {
            // Default confidence is now VERTICAL_CONFIDENCE
        }

        /**
         * Sets the altitude reliability. See enum IZatReliablityTypes.
         *
         * @param reliability Altitude reliability.
         *
         * @return None.
         */
        @Deprecated
        public void setAltitudeReliability(IZatReliablityTypes reliability) {
            mAltitudeReliability = reliability;
            mValidBits |= IZAT_BS_LOC_ALTITUDE_RELIABILITY_VALID;
        }

    }
/** @} */ /* end_addtogroup IZatWWANDBReceiver */

/** @addtogroup IZatWWANDBReceiver
@{ */

    /**
     * Class IZatBSSpecialInfo.
     */
    public static class IZatBSSpecialInfo {
        private final IZatCellInfo mCellInfo;
        private final IZatAPBSSpecialInfoTypes mInfo;

        /**
         * Constructor - IZatBSSpecialInfo.
         *
         * @param cellInfo
         * @param info
         */
        public IZatBSSpecialInfo(IZatCellInfo cellInfo,
                                 IZatAPBSSpecialInfoTypes info) {
            mCellInfo = cellInfo;
            mInfo = info;
        }

        /**
         *
         * @return Cell info.
         */
        public IZatCellInfo getCellInfo() {
            return mCellInfo;
        }

        /**
         *
         * @return Special info for this cell.
         */
        public IZatAPBSSpecialInfoTypes getInfo() {
            return mInfo;
        }
    }
}
/** @} */ /* end_addtogroup IZatWWANDBReceiver */
