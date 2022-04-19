/* ======================================================================
*  Copyright (c) 2018 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*  ====================================================================*/

package com.qti.location.sdk;

import android.content.Context;
import android.os.Bundle;

import com.qti.debugreport.IZatDebugConstants;
import java.util.List;
import java.util.Set;
import java.util.Map;
import java.util.HashMap;

/** @addtogroup IZatGnssConfigService
    @{ */

public interface IZatGnssConfigService {

    /**
     * Enum IzatGnssSvType.
     *
     * IzatGnssSvType specified constellations which can be enabled or
     * disabled via:
     * {@link com.qti.location.sdk.IZatGnssConfigService.setGnssSvTypeConfig()}
     */

    public enum IzatGnssSvType {

        UNKNOWN(0),
        GPS(1),
        SBAS(2),
        GLONASS(3),
        QZSS(4),
        BEIDOU(5),
        GALILEO(6);

        private final int value;
        private IzatGnssSvType(int value) {
            this.value = value;
        }
        public int getValue() {
            return value;
        }

        private static final Map<Integer, IzatGnssSvType> valueMap =
                new HashMap<Integer, IzatGnssSvType>();

        static {
            for (IzatGnssSvType type : IzatGnssSvType.values()) {
                valueMap.put(type.value, type);
            }
        }

        protected static IzatGnssSvType fromInt(int value) {
            return valueMap.get(value);
        }
    }

    /**
     * Requests the current GNSS SV Type Configuration.
     * <p>
     * This function sends a request to GNSS engine to
     * fetch the current GNSS SV Type Configuration </p>
     *
     * @param gnssConfigCb Callback to receive the SV Type configuration.
     *         This parameter cannot be NULL, otherwise a
     *         {@link IZatIllegalArgumentException} is thrown.
     * @throws IZatIllegalArgumentException The gnssConfigCb parameter is NULL.
     *
     * @return
     * None.
     */

    void getGnssSvTypeConfig(IZatGnssConfigCallback gnssConfigCb)
            throws IZatIllegalArgumentException;

    /**
     * Sets the GNSS SV Type configuration.
     * <p>
     * This function specifies the GNSS SV Types (constellations) to be
     * enabled and disabled.</p>
     *
     * @param enabledSvTypeSet Set of IzatGnssSvType to be enabled.
     *         Pass null or an empty set if no enablement required.
     * @param disabledSvTypeSet Set of IzatGnssSvType to be disabled.
     *         Pass null or an empty set if no disablement required.
     * @throws IZatIllegalArgumentException The enabledSvTypeSet and
     *          disabledSvTypeSet parameters both are NULL.
     *          Both enabledSvTypeSet and disabledSvTypeSet contain
     *          the same element.
     *
     * @return
     * None.
     */

    void setGnssSvTypeConfig(Set<IzatGnssSvType> enabledSvTypeSet,
                             Set<IzatGnssSvType> disabledSvTypeSet)
            throws IZatIllegalArgumentException;

    /**
     * Resets the GNSS SV Type configuration.
     * <p>
     * This function resets the GNSS SV Type configuration to
     * default values in the underlying service.</p>
     *
     * @param
     * None
     *
     * @return
     * None.
     */

    void resetGnssSvTypeConfig();

    /**
     * Class IZatGnssConfigCallback.
     *
     * IZatGnssConfigCallback provides the GNSS Config fetched
     * from the underlying service.
     */

    interface IZatGnssConfigCallback {

        /**
         * GNSS SV Type Config Callback.
         * <p>
         * This API is called by the underlying service back
         * to applications in response to the getGnssSvTypeConfig
         * request.</p>
         *
         * @param enabledSvTypeSet Set of IzatGnssSvType.
         * @param disabledSvTypeSet Set of IzatGnssSvType.
         */

        void getGnssSvTypeConfigCb(Set<IzatGnssSvType> enabledSvTypeSet,
                                   Set<IzatGnssSvType> disabledSvTypeSet);
    }
}
/** @} */ /* end_addtogroup IZatGnssConfigService */
