/* ======================================================================
*  Copyright (c) 2018 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*  ====================================================================*/
package com.qti.location.sdk;

import android.location.Location;

import com.qti.location.sdk.IZatDBCommon;
import com.qti.location.sdk.IZatDBCommon.IZatAPBSSpecialInfoTypes;
import com.qti.location.sdk.IZatDBCommon.IZatCellInfo;
import com.qti.location.sdk.IZatDBCommon.IZatApBsListStatus;
import com.qti.location.sdk.IZatWWANDBReceiver.IZatBSLocationDataBase;
import com.qti.location.sdk.IZatStaleDataException;

import java.util.List;

/*
 * <p>Copyright (c) 2017 Qualcomm Technologies, Inc.</p>
 * <p>All Rights Reserved.</p>
 * <p>Confidential and Proprietary - Qualcomm Technologies, Inc</p>
 * <br/>
 * <p><b>IZatWWANDBProvider</b> interface @version 1.0.0 </p>
 */

/** @addtogroup IZatWWANDBProvider
@{ */

/** API for
 * injecting a WWAN Cell Location database to a Qualcomm Location framework.
 */
public abstract class IZatWWANDBProvider {
    protected final IZatWWANDBProviderResponseListener mResponseListener;

    /**
     * <p>
     * Constructor - IZatWWANDBProvider. </p>
     *
     * @param listener Listener to receive WWAN DB Receiver
     *         responses. This parameter cannot be NULL, otherwise
     *         a {@link IZatIllegalArgumentException} is
     *         thrown.
     * @throws IZatIllegalArgumentException The listener parameter is NULL.
     * @return
     * None.
     */
    protected IZatWWANDBProvider(IZatWWANDBProviderResponseListener listener)
                                throws IZatIllegalArgumentException {
        if(null == listener) {
            throw new IZatIllegalArgumentException("Unable to obtain IZatWWANDBProvider instance");
        }
        mResponseListener = listener;
    }

    /**
     * Requests list of base stations.
     * <p>
     * This enables WWAN database providers to request a list of BSs
     * that require location information.
     * </p>
     *
     * @return
     * None.
     */
    public abstract void requestBSObsLocData();

/** @} */ /* end_addtogroup IZatWWANDBProvider */

/** @addtogroup IZatWWANDBProvider
@{ */

    /**
     * Interface class IZatWWANDBProviderResponseListener.
     *
     * <p>This interface
     * receives responses from a WWAN database receiver.</p>
     */
    public interface IZatWWANDBProviderResponseListener {

        /**
         * Response to a BS list request.
         * <p>
         * This API is called by the underlying service back
         * to applications when a list of BSs is available.
         * Applications should implement this interface.</p>
         *
         * @param bs_list List of BSs.
         * @param bs_status List of BSs.
         * @param location Location of area of interest.
         *
         * @return
         * None.
         */
        void onBSObsLocDataAvailable(List<IZatBSObsLocationData> bs_list,
                                     IZatApBsListStatus bs_status);

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

/** @} */ /* end_addtogroup IZatWWANDBProvider */


/** @addtogroup IZatWWANDBProvider
@{ */

    /**
     * Class IZatBSObsLocationData.
     */
    public static class IZatBSObsLocationData extends IZatBSLocationDataBase {
        private long mTimeStamp;
        public static final int IZAT_BS_LOC_TIMESTAMP_VALID = 0x10;

        /**
         *
         * @param iZatCellInfo
         * @param location
         * @param horizontalReliability
         * @param altitudeReliability
         */
        public IZatBSObsLocationData(IZatCellInfo iZatCellInfo, Location location,
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
         * @param timeStamp
         */
        public IZatBSObsLocationData(IZatCellInfo iZatCellInfo, Location location,
                                     IZatReliablityTypes horizontalReliability,
                                     IZatReliablityTypes altitudeReliability,
                                     long timeStamp) {
            super(iZatCellInfo, location, horizontalReliability, altitudeReliability);
            mTimeStamp = timeStamp;
            mValidBits |= IZAT_BS_LOC_TIMESTAMP_VALID;

        }

        /**
         *
         * @return Timestamp
         */
        public long getTimestamp() {
            if(0 == (IZAT_BS_LOC_TIMESTAMP_VALID & mValidBits)) {
                throw new IZatStaleDataException("Timestamp information is not valid");
            }
            return mTimeStamp;
        }

        /**
         * Set the timestamp
         *
         * @param timestamp The timestamp
         */
        public void setTimestamp(long timestamp) {
            this.mTimeStamp = timestamp;
            mValidBits |= IZAT_BS_LOC_TIMESTAMP_VALID;
        }
    }

/** @} */ /* end_addtogroup IZatWWANDBProvider */

/** @addtogroup IZatWWANDBProvider
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
         * Gets the Cell info.
         *
         * @return The Cell info.
         */
        public IZatCellInfo getCellInfo() {
            return mCellInfo;
        }

        /**
         * Gets the special info.
         *
         * @return The cpecial info.
         */
        public IZatAPBSSpecialInfoTypes getInfo() {
            return mInfo;
        }
    }
}
/** @} */ /* end_addtogroup IZatWWANDBProvider */
