/* ======================================================================
*  Copyright (c) 2018 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*  ====================================================================*/

package com.qti.location.sdk;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.qti.location.sdk.IZatWiFiDBReceiver.IZatAPSpecialInfo.IZatAPSpecialInfoTypes;
import com.qti.location.sdk.IZatStaleDataException;

/** @addtogroup IZatDBCommon
@{ */

/** Common WiFi and WWAN definitions
*
*/
public abstract class IZatDBCommon {

    /**
    * Horizontal confidence constant (99)
    */
    public final static Integer HORIZONTAL_CONFIDENCE = 99;

    /**
    * Vertical confidence constant (68)
    */
    public final static Integer VERTICAL_CONFIDENCE = 68;

    /**
    * Speed confidence constant (99)
    */
    public final static Integer SPEED_CONFIDENCE = 99;

    /**
     * Enumeration of IZat cell types.
     */
    public enum IZatCellTypes {
        GSM,     /**< GSM cell type. */
        WCDMA,   /**< WCDMA cell type. */
        CDMA,    /**< CDMA cell type. */
        LTE      /**< LTE cell type. */
    }

    /**
     * Class IZatCellInfo.
     */
    public static class IZatCellInfo {

        protected final int mRegionID1;
        protected final int mRegionID2;
        protected final int mRegionID3;
        protected final int mRegionID4;
        protected IZatCellTypes mType;

        protected int mCellLocalTimestamp;
        protected int mValidBits;

        public static final int IZAT_CELL_WITH_TIMESTAMP = 0x1;
        /**
         * Constructor - IZatCellInfo.
         *
         * @param regionID1 Mobile country code (MCC). For CDMA set to 0.
         * @param regionID2 Mobile network code (MNC). For CDMA, set to
         *                  system ID (SID).
         * @param regionID3 GSM: Local area code (LAC) \n
         *                  WCDMA: LAC; set to 0 if not available \n
         *                  CDMA: Network ID (NID) \n
         *                  LTE: Tracking area code (TAC); set to 0 if not available
         * @param regionID4 Cell ID (CID); for CDMA, set to base station
         *                  ID (BSID).
         * @param type      Cell type.
         *
         * @return
         * None.
         */
        public IZatCellInfo(int regionID1, int regionID2, int regionID3,
                            int regionID4, IZatCellTypes type) {
            mRegionID1 = regionID1;
            mRegionID2 = regionID2;
            mRegionID3 = regionID3;
            mRegionID4 = regionID4;
            mType = type;
            mValidBits = 0;
        }

        /**
         * Constructor - IZatCellInfo.
         *
         * @param regionID1 Mobile country code (MCC). For CDMA set to 0.
         * @param regionID2 Mobile network code (MNC). For CDMA, set to
         *                  system ID (SID).
         * @param regionID3 GSM: Local area code (LAC) \n
         *                  WCDMA: LAC; set to 0 if not available \n
         *                  CDMA: Network ID (NID) \n
         *                  LTE: Tracking area code (TAC); set to 0 if not available
         * @param regionID4 Cell ID (CID); for CDMA, set to base station
         *                  ID (BSID).
         * @param type      Cell type index.
         *
         * @return
         * None.
         */
        public IZatCellInfo(int regionID1, int regionID2, int regionID3,
                            int regionID4, Integer type) {
            this(regionID1, regionID2, regionID3, regionID4, IZatCellTypes.values()[type]);
        }

        /**
         * Get the Cell local timestamp. Will be 0 for WWAN crowdsourced data.
         *
         * @throws IZatStaleDataException Timestamp is not available.
         * @return Cell local timestamp.
         */
        public int getCellLocalTimestamp() {
            if(0 == (IZAT_CELL_WITH_TIMESTAMP & mValidBits)) {
                throw new IZatStaleDataException("Local timestamp is not valid");
            }
            return mCellLocalTimestamp;
        }

        /**
         * Set the cell local timestamp
         *
         * @param cellLocalTimestamp Cell local timestamp
         */
        public void setCellLocalTimestamp(int cellLocalTimestamp) {
            this.mCellLocalTimestamp = cellLocalTimestamp;
            mValidBits |= IZAT_CELL_WITH_TIMESTAMP;
        }

        /**
         * Get the Region id 1
         *
         * @return Region id 1
         */
        public int getRegionID1() {
            return mRegionID1;
        }

        /**
         * Get the Region id 2
         *
         * @return Region id 2
         */
        public int getRegionID2() {
            return mRegionID2;
        }

        /**
         * Get the Region id 3
         *
         * @return Region id 3
         */
        public int getRegionID3() {
            return mRegionID3;
        }

        /**
         * Get the Region id 4
         *
         * @return Region id 4
         */
        public int getRegionID4() {
            return mRegionID4;
        }

        /**
         * Get the Cell type.
         *
         * @return Cell type.
         */
        public IZatCellTypes getType() {
            return mType;
        }

        /**
         * Set the Cell type
         *
         * @param type Cell type.
         */
        public void setType(IZatCellTypes type) {
            this.mType = type;
        }
    }

    /**
     * Enumeration of spacial info types.
     */
    public enum IZatAPBSSpecialInfoTypes {
        /** No information is available.
         */
        NO_INFO_AVAILABLE,
        /** Moving APor BS.
         */
        MOVING_AP_BS,
        /** AP/BS could not be resolved  */
        NOT_RESOLVED;
    }

    /**
     * Class IZatAPSSIDInfo.
     */
    public static class IZatAPSSIDInfo {

        public byte[] mSSID = new byte[0];

        /**
         * Contructor - IZatAPSSIDInfo.
         *
         * @param ssid Available bytes in the AP's SSID. This can be
         *             smaller than the actual SSID size. The number of
         *             available bytes from the SSID is indicated by
         *             validBytesCount.
         * @param validBytesCount Actual number of AP SSID bytes
         *                         that are available in the SSID.
         *
         * @return
         * None.
         */
        public IZatAPSSIDInfo(byte[] ssid, short validBytesCount) {
            if (validBytesCount > 0) {
                mSSID = ssid.clone();
            }
        }
    }

    public enum IZatApBsListStatus {
        /** AP/BS list is partial, expect more to come. */
        STD_CONT(0),
        /** AP/BS list is the final one. */
        STD_FINAL(1),
        /** AP list match a full scan, and is the final one. */
        SCAN_FINAL(2);
        // Enforce corrects value from C enum definition
        private final int cValue;

        private IZatApBsListStatus(int cValue) {
            this.cValue = cValue;
        }

        private static final Map<Integer, IZatApBsListStatus> typesByValue = new HashMap<Integer,
                IZatApBsListStatus>();

        static {
            for (IZatApBsListStatus type : IZatApBsListStatus.values()) {
                typesByValue.put(type.cValue, type);
            }
        }

        protected static IZatApBsListStatus fromInt(int cValue) {
            return typesByValue.get(cValue);
        }
    }
}
/** @} */ /* end_addtogroup IZatDBCommon */
