/* ======================================================================
*  Copyright (c) 2018 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*  ====================================================================*/
package com.qti.location.sdk;

import android.location.Location;

import com.qti.location.sdk.IZatDBCommon.IZatCellInfo;
import com.qti.location.sdk.IZatDBCommon.IZatAPSSIDInfo;
import com.qti.location.sdk.IZatDBCommon.IZatApBsListStatus;
import com.qti.location.sdk.IZatIllegalArgumentException;

import java.util.List;

/*
 * <p>Copyright (c) 2017 Qualcomm Technologies, Inc.</p>
 * <p>All Rights Reserved.</p>
 * <p>Confidential and Proprietary - Qualcomm Technologies, Inc</p>
 * <br/>
 * <p><b>IZatWiFiDBReceiver</b> interface @version 1.0.0 </p>
 */

/** @addtogroup IZatWiFiDBProvider
@{ */

/** APIs for
* injecting a Wi-Fi AP location database to a Qualcomm Location framework.
*
*/
public abstract class IZatWiFiDBProvider {
   protected final IZatWiFiDBProviderResponseListener mResponseListener;

   /**
    * Constructor - IZatWiFiDBProvider.
    *
    * @param listener Listener to receive Wi-Fi DB receiver
    *         responses. This parameter cannot be NULL, otherwise
    *         a {@link IZatIllegalArgumentException} is
    *         thrown.
    * @throws IZatIllegalArgumentException The listener parameter is NULL.
    * @return
    * None.
    */
   protected IZatWiFiDBProvider(IZatWiFiDBProviderResponseListener listener)
                               throws IZatIllegalArgumentException {
       if(null == listener) {
           throw new IZatIllegalArgumentException("Unable to obtain IZatWiFiDBProvider instance");
       }
       mResponseListener = listener;
   }


   /**
    * Requests a list of access points.
    * <p>
    * This function enables Wi-Fi database providers to request a list of APs
    * that require location information.
    * </p>
    *
    * @return
    * None.
    */
   public abstract void requestAPObsLocData();

/** @} */ /* end_addtogroup IZatWiFiDBProvider */

/** @addtogroup IZatWiFiDBProvider
@{ */

   /**
    * Interface class IZatWiFiDBProviderResponseListener.
    *
    * This interface
    * receives responses from a Wi-Fi database receiver in a Qualcomm
    * Location framework.
    */
   public interface IZatWiFiDBProviderResponseListener {

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
       void onApObsLocDataAvailable(List<IZatAPObsLocData> ap_obs_list,
                                    IZatApBsListStatus ap_obs_status);

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

/** @} */ /* end_addtogroup IZatWiFiDBProvider */

/** @addtogroup IZatWiFiDBProvider
@{ */

    /**
    * Class IZatAPObsLocData.
    */
    public static class IZatAPObsLocData {
        private Location mLocation;
        private IZatCellInfo mCellInfo;
        private long mScanTimestamp;
        private List<IZatAPScan> mApScanList;

        /**
         * Constructor - IZatAPObsLocData.
         *
         * @param location Location of the observation.
         * @param cellInfo Cell information of the observation.
         * @param apScanList List of AP scans.
         */
        public IZatAPObsLocData(Location location, IZatCellInfo cellInfo, long scanTimestamp,
                                List<IZatAPScan> apScanList) {
            this.mLocation = location;
            this.mCellInfo = cellInfo;
            this.mScanTimestamp = scanTimestamp;
            this.mApScanList = apScanList;
        }

        /**
         * Gets the location of this observation.
         *
         * @return Location of the observation.
         */
        public Location getLocation() {
            return mLocation;
        }

        /**
         * Gets the Cell information of the observation.
         *
         * @return Cell information of the observation.
         */
        public IZatCellInfo getCellInfo() {
            return mCellInfo;
        }

        /**
         * Gets the List of AP scans.
         *
         * @return List of AP scans.
         */
        public List<IZatAPScan> getApScanList() {
            return mApScanList;
        }

        /**
         * Gets the Scan timestamp, UTC seconds since UNIX epoch.
         *
         * @return Scan timestamp
         */
        public long getScanTimestamp() {
            return mScanTimestamp;
        }
    }

/** @} */ /* end_addtogroup IZatWiFiDBProvider */

/** @addtogroup IZatWiFiDBProvider
@{ */

    /**
    * Class IZatAPScan.
    */
    public static class IZatAPScan {
        private String mMacAddress;
        private float mRssi;
        private int mDeltatime;
        private IZatAPSSIDInfo mSSID;
        private int mAPChannelNumber;

        /**
         * Constructor - IZatAPScan.
         *
         * @param macAddress The MAC address of the AP.
         * @param rssi The RSSI signal received in the scan.
         * @param deltatime Delta time since the scan started.
         * @param SSID SSID of the AP.
         * @param APChannelNumber Channel number of the AP.
         */
        public IZatAPScan(String macAddress, float rssi, int deltatime, IZatAPSSIDInfo SSID,
                          int APChannelNumber) {
           this.mMacAddress = macAddress;
           this.mRssi = rssi;
           this.mDeltatime = deltatime;
           this.mSSID = SSID;
           this.mAPChannelNumber = APChannelNumber;
        }

        /**
         * Gets the MAC address of the AP.
         *
         * @return The MAC address of the AP.
         */
        public String getMacAddress() {
           return mMacAddress;
        }

        /**
         * Gets the RSSI signal received in the scan.
         *
         * @return The RSSI signal received in the scan.
         */
        public float getRssi() {
           return mRssi;
        }

        /**
         * Gets delta time since the scan started, in microseconds.
         *
         * @return Delta time since the scan started.
         */
        public int getDeltatime() {
           return mDeltatime;
        }

        /**
         * Gets the SSID of the AP.
         *
         * @return SSID of the AP.
         */
        public IZatAPSSIDInfo getSSID() {
           return mSSID;
        }

        /**
         * Gets the channel number of the AP.
         *
         * @return Channel number of the AP.
         */
        public int getAPChannelNumber() {
           return mAPChannelNumber;
        }
   }

/** @} */ /* end_addtogroup IZatWiFiDBProvider */

}
/** @} */ /* end_addtogroup IZatWiFiDBProvider */