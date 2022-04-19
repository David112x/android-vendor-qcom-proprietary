/******************************************************************************
#  Copyright (c) 2015-2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#
#  Not a Contribution.
#  Apache license notifications and license are retained
#  for attribution purposes only.
#
#  Copyright (C) 2006 The Android Open Source Project
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#******************************************************************************/

/******************************************************************************
  @file    qcril_qmi_cell_info.c
  @brief   qcril qmi - CELL_INFO

  DESCRIPTION
    Handles CELL_INFO_LIST responses

******************************************************************************/

#include "utils_standard.h"
#include "qcril_qmi_cell_info.h"
#include "qcril_qmi_cell_info_v12.h"
#include "qcril_log.h"
#include "modules/nas/qcril_qmi_nas.h"
#include "qcril_memory_management.h"
#include <modules/nas/NasModule.h>
#include <string.h>

void qcril_qmi_nas_init_cell_info_struct_v12(RIL_CellInfo_v12 *cell_info)
{
    cell_info->cellInfoType = RIL_CELL_INFO_TYPE_NONE;
    cell_info->registered = 0;
    cell_info->timeStampType = RIL_TIMESTAMP_TYPE_UNKNOWN;
    cell_info->timeStamp = 0;
    cell_info->connStatus = RIL_CELL_CONNECTION_STATUS_NONE;

    /***********************************************************************
     ********************************* GSM *********************************
     ***********************************************************************/

    /***********************
     **** Cell Identity ****
     ***********************/

    // 3-digit Mobile Country Code, 0..999, INT_MAX if unknown
    memset(cell_info->CellInfo.gsm.cellIdentityGsm.mcc, 0, sizeof(cell_info->CellInfo.gsm.cellIdentityGsm.mcc));

    // 2 or 3-digit Mobile Network Code, 0..999, INT_MAX if unknown
    memset(cell_info->CellInfo.gsm.cellIdentityGsm.mnc, 0, sizeof(cell_info->CellInfo.gsm.cellIdentityGsm.mnc));

    // 16-bit Location Area Code, 0..65535, INT_MAX if unknown
    cell_info->CellInfo.gsm.cellIdentityGsm.lac = INT_MAX;

    // 16-bit GSM Cell Identity described in TS 27.007, 0..65535, INT_MAX if unknown
    cell_info->CellInfo.gsm.cellIdentityGsm.cid = INT_MAX;

    // 16-bit GSM Absolute RF channel number; this value must be reported
    cell_info->CellInfo.gsm.cellIdentityGsm.arfcn = -1;

    // 6-bit Base Station Identity Code; 0xFF if unknown
    cell_info->CellInfo.gsm.cellIdentityGsm.bsic = 0xFF;

    /*************************
     **** Signal Strength ****
     *************************/

    // Valid values are (0-31, 99) as defined in TS 27.007 8.5
    cell_info->CellInfo.gsm.signalStrengthGsm.signalStrength = -1;

    // bit error rate (0-7, 99) as defined in TS 27.007 8.5
    cell_info->CellInfo.gsm.signalStrengthGsm.bitErrorRate = -1;

    /* Timing Advance in bit periods. 1 bit period = 48/13 us.
     * INT_MAX denotes invalid value */
    cell_info->CellInfo.gsm.signalStrengthGsm.timingAdvance = INT_MAX;

    /************************************************************************
     ********************************* CDMA *********************************
     ************************************************************************/

    /***********************
     **** Cell Identity ****
     ***********************/

    // Network Id 0..65535, INT_MAX if unknown
    cell_info->CellInfo.cdma.cellIdentityCdma.networkId = INT_MAX;

    // CDMA System Id 0..32767, INT_MAX if unknown
    cell_info->CellInfo.cdma.cellIdentityCdma.systemId = INT_MAX;

    // Base Station Id 0..65535, INT_MAX if unknown
    cell_info->CellInfo.cdma.cellIdentityCdma.basestationId = INT_MAX;

    /* Longitude is a decimal number as specified in 3GPP2 C.S0005-A v6.0.
     * It is represented in units of 0.25 seconds and ranges from -2592000
     * to 2592000, both values inclusive (corresponding to a range of -180
     * to +180 degrees). INT_MAX if unknown */
    cell_info->CellInfo.cdma.cellIdentityCdma.longitude = INT_MAX;

    /* Latitude is a decimal number as specified in 3GPP2 C.S0005-A v6.0.
     * It is represented in units of 0.25 seconds and ranges from -1296000
     * to 1296000, both values inclusive (corresponding to a range of -90
     * to +90 degrees). INT_MAX if unknown */
    cell_info->CellInfo.cdma.cellIdentityCdma.latitude = INT_MAX;

    /******************************
     **** CDMA Signal Strength ****
     ******************************/

    /* Valid values are positive integers.  This value is the actual RSSI value
     * multiplied by -1.  Example: If the actual RSSI is -75, then this response
     * value will be 75. */
    cell_info->CellInfo.cdma.signalStrengthCdma.dbm = -1;

    /* Valid values are positive integers.  This value is the actual Ec/Io multiplied
     * by -10.  Example: If the actual Ec/Io is -12.5 dB, then this response value
     * will be 125. */
    cell_info->CellInfo.cdma.signalStrengthCdma.ecio = -1;

    /******************************
     **** EVDO Signal Strength ****
     ******************************/

    /* Valid values are positive integers.  This value is the actual RSSI value
     * multiplied by -1.  Example: If the actual RSSI is -75, then this response
     * value will be 75. */
    cell_info->CellInfo.cdma.signalStrengthEvdo.dbm = -1;

    /* Valid values are positive integers.  This value is the actual Ec/Io multiplied
     * by -10.  Example: If the actual Ec/Io is -12.5 dB, then this response value
     * will be 125. */
    cell_info->CellInfo.cdma.signalStrengthEvdo.ecio = -1;

    // Valid values are 0-8. 8 is the highest signal to noise ratio.
    cell_info->CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio = -1;

    /*************************************************************************
     ********************************* WCDMA *********************************
     *************************************************************************/

    /***********************
     **** Cell Identity ****
     ***********************/

    // 3-digit Mobile Country Code, 0..999, INT_MAX if unknown
    memset(cell_info->CellInfo.wcdma.cellIdentityWcdma.mcc, 0, sizeof(cell_info->CellInfo.wcdma.cellIdentityWcdma.mcc));

    // 2 or 3-digit Mobile Network Code, 0..999, INT_MAX if unknown
    memset(cell_info->CellInfo.wcdma.cellIdentityWcdma.mnc, 0, sizeof(cell_info->CellInfo.wcdma.cellIdentityWcdma.mnc));

    // 16-bit Location Area Code, 0..65535, INT_MAX if unknown
    cell_info->CellInfo.wcdma.cellIdentityWcdma.lac = INT_MAX;

    // 28-bit UMTS Cell Identity described in TS 25.331, 0..268435455, INT_MAX if unknown
    cell_info->CellInfo.wcdma.cellIdentityWcdma.cid = INT_MAX;

    // 9-bit UMTS Primary Scrambling Code described in TS 25.331, 0..511; this value must be reported
    cell_info->CellInfo.wcdma.cellIdentityWcdma.psc = -1;

    // 16-bit UMTS Absolute RF Channel Number; this value must be reported
    cell_info->CellInfo.wcdma.cellIdentityWcdma.uarfcn = -1;

    /*************************
     **** Signal Strength ****
     *************************/

    // Valid values are (0-31, 99) as defined in TS 27.007 8.5
    cell_info->CellInfo.wcdma.signalStrengthWcdma.signalStrength = -1;

    // bit error rate (0-7, 99) as defined in TS 27.007 8.5
    cell_info->CellInfo.wcdma.signalStrengthWcdma.bitErrorRate = -1;

    /***************************************************************************
     ********************************* TDSCDMA *********************************
     ***************************************************************************/

    /***********************
     **** Cell Identity ****
     ***********************/

    // 3-digit Mobile Country Code, 0..999, INT_MAX if unknown
    memset(cell_info->CellInfo.tdscdma.cellIdentityTdscdma.mcc, 0, sizeof(cell_info->CellInfo.tdscdma.cellIdentityTdscdma.mcc));

    // 2 or 3-digit Mobile Network Code, 0..999, INT_MAX if unknown
    memset(cell_info->CellInfo.tdscdma.cellIdentityTdscdma.mnc, 0, sizeof(cell_info->CellInfo.tdscdma.cellIdentityTdscdma.mnc));

    // 16-bit Location Area Code, 0..65535, INT_MAX if unknown
    cell_info->CellInfo.tdscdma.cellIdentityTdscdma.lac = INT_MAX;

    // 28-bit UMTS Cell Identity described in TS 25.331, 0..268435455, INT_MAX if unknown
    cell_info->CellInfo.tdscdma.cellIdentityTdscdma.cid = INT_MAX;

    // 8-bit Cell Parameters ID described in TS 25.331, 0..127, INT_MAX if unknown
    cell_info->CellInfo.tdscdma.cellIdentityTdscdma.cpid = INT_MAX;

    /*************************
     **** Signal Strength ****
     *************************/

    /* The Received Signal Code Power in dBm multiplied by -1.
     * Range : 25 to 120
     * INT_MAX: 0x7FFFFFFF denotes invalid value.
     * Reference: 3GPP TS 25.123, section 9.1.1.1 */
    cell_info->CellInfo.tdscdma.signalStrengthTdscdma.rscp = INT_MAX;

    /***********************************************************************
     ********************************* LTE *********************************
     ***********************************************************************/

    /***********************
     **** Cell Identity ****
     ***********************/

    // 3-digit Mobile Country Code, 0..999, INT_MAX if unknown
    memset(cell_info->CellInfo.lte.cellIdentityLte.mcc, 0, sizeof(cell_info->CellInfo.lte.cellIdentityLte.mcc));

    // 2 or 3-digit Mobile Network Code, 0..999, INT_MAX if unknown
    memset(cell_info->CellInfo.lte.cellIdentityLte.mnc, 0, sizeof(cell_info->CellInfo.lte.cellIdentityLte.mnc));

    // 28-bit Cell Identity described in TS ???, INT_MAX if unknown
    cell_info->CellInfo.lte.cellIdentityLte.ci = INT_MAX;

    // physical cell id 0..503; this value must be reported
    cell_info->CellInfo.lte.cellIdentityLte.pci = -1;

    // 16-bit tracking area code, INT_MAX if unknown
    cell_info->CellInfo.lte.cellIdentityLte.tac = INT_MAX;

    // 18-bit LTE Absolute RF Channel Number; this value must be reported
    cell_info->CellInfo.lte.cellIdentityLte.earfcn = -1;

    memset(&cell_info->CellInfo.lte.cellIdentityLte.operatorNames,
            0,
            sizeof(cell_info->CellInfo.lte.cellIdentityLte.operatorNames));
    cell_info->CellInfo.lte.cellIdentityLte.bandwidth = INT_MAX;

    /*************************
     **** Signal Strength ****
     *************************/

    // Valid values are (0-31, 99) as defined in TS 27.007 8.5
    cell_info->CellInfo.lte.signalStrengthLte.signalStrength = -1;

    /* The current Reference Signal Receive Power in dBm multipled by -1.
     * Range: 44 to 140 dBm
     * INT_MAX: 0x7FFFFFFF denotes invalid value.
     * Reference: 3GPP TS 36.133 9.1.4 */
    cell_info->CellInfo.lte.signalStrengthLte.rsrp = INT_MAX;

    /* The current Reference Signal Receive Quality in dB multiplied by -1.
     * Range: 20 to 3 dB.
     * INT_MAX: 0x7FFFFFFF denotes invalid value.
     * Reference: 3GPP TS 36.133 9.1.7 */
    cell_info->CellInfo.lte.signalStrengthLte.rsrq = INT_MAX;

    /* The current reference signal signal-to-noise ratio in 0.1 dB units.
     * Range: -200 to +300 (-200 = -20.0 dB, +300 = 30dB).
     * INT_MAX : 0x7FFFFFFF denotes invalid value.
     * Reference: 3GPP TS 36.101 8.1.1 */
    cell_info->CellInfo.lte.signalStrengthLte.rssnr = INT_MAX;

    /* The current Channel Quality Indicator.
     * Range: 0 to 15.
     * INT_MAX : 0x7FFFFFFF denotes invalid value.
     * Reference: 3GPP TS 36.101 9.2, 9.3, A.4 */
    cell_info->CellInfo.lte.signalStrengthLte.cqi = INT_MAX;

    /* timing advance in micro seconds for a one way trip from cell to device.
     * Approximate distance can be calculated using 300m/us * timingAdvance.
     * Range: 0 to 0x7FFFFFFE
     * INT_MAX : 0x7FFFFFFF denotes invalid value.
     * Reference: 3GPP 36.321 section 6.1.3.5
     * also: http://www.cellular-planningoptimization.com/2010/02/timing-advance-with-calculation.html */
    cell_info->CellInfo.lte.signalStrengthLte.timingAdvance = INT_MAX;

    /***********************
     **** NR Cell Identity ****
     ***********************/

    // 3-digit Mobile Country Code, 0..999, INT_MAX if unknown
    memset(cell_info->CellInfo.nr.cellIdentityNr.mcc, 0, sizeof(cell_info->CellInfo.nr.cellIdentityNr.mcc));

    // 2 or 3-digit Mobile Network Code, 0..999, INT_MAX if unknown
    memset(cell_info->CellInfo.nr.cellIdentityNr.mnc, 0, sizeof(cell_info->CellInfo.nr.cellIdentityNr.mnc));

    // 64-bit NR Cell Identity described in 3GPP TS 38.331, INT_MAX if unknown
    cell_info->CellInfo.nr.cellIdentityNr.nci = INT_MAX;

    // 32-bit Physical cell id described in 3GPP TS 38.331, INT_MAX if unknown
    cell_info->CellInfo.nr.cellIdentityNr.pci = INT_MAX;

    // 16-bit tracking area code, INT_MAX if unknown
    cell_info->CellInfo.nr.cellIdentityNr.tac = INT_MAX;

    // NR Absolute Radio Frequency Channel Number Reference: 3GPP TS 38.101-1 and 3GPP TS 38.101-2 section 5.4.2.1
    cell_info->CellInfo.nr.cellIdentityNr.nrarfcn = -1;

    memset(&cell_info->CellInfo.nr.cellIdentityNr.operatorNames,
            0,
            sizeof(cell_info->CellInfo.nr.cellIdentityNr.operatorNames));

    /*************************
     **** NR Signal Strength ****
     *************************/

    /**
     * SS reference signal received power.
     *
     * Reference: 3GPP TS 38.215.
     *
     * Range [-44, -140], INT_MAX means invalid/unreported.
     */
    cell_info->CellInfo.nr.signalStrengthNr.ssRsrp = INT_MAX;

    /**
     * SS reference signal received quality, multipled by -1..
     *
     * Reference: 3GPP TS 38.215.
     *
     * Range [-3, -20], INT_MAX means invalid/unreported.
     */
    cell_info->CellInfo.nr.signalStrengthNr.ssRsrq = INT_MAX;

    /**
     * SS signal-to-noise and interference ratio.
     *
     * Reference: 3GPP TS 38.215 section 5.1.*, 3GPP TS 38.133 section 10.1.16.1.
     *
     * Range [-23, 40], INT_MAX means invalid/unreported.
     */
    cell_info->CellInfo.nr.signalStrengthNr.ssSinr = INT_MAX;

    /**
     * CSI reference signal received power, multipled by -1.
     *
     * Reference: 3GPP TS 38.215.
     *
     * Range [44, 140], INT_MAX means invalid/unreported.
     */
    cell_info->CellInfo.nr.signalStrengthNr.csiRsrp = INT_MAX;

    /**
     * CSI reference signal received quality, multipled by -1.
     *
     * Reference: 3GPP TS 38.215.
     *
     * Range [3, 20], INT_MAX means invalid/unreported.
     */
    cell_info->CellInfo.nr.signalStrengthNr.csiRsrq = INT_MAX;

    /**
     * CSI signal-to-noise and interference ratio.
     *
     * Reference: 3GPP TS 138.215 section 5.1.*, 3GPP TS 38.133 section 10.1.16.1.
     *
     * Range [-23, 40], INT_MAX means invalid/unreported.
     */
    cell_info->CellInfo.nr.signalStrengthNr.csiSinr = INT_MAX;
}

void qcril_qmi_nas_init_gsm_cell_identity_v12
(
  RIL_CELL_INFO_LIST_GSM_ID_TYPE *cell,
  char *plmn,
  uint8_t  lac_valid,
  uint16_t lac,
  uint8_t  cid_valid,
  uint32_t cid,
  uint8_t  arfcn_valid,
  uint16_t arfcn,
  uint8_t bsic_valid,
  uint8_t bsic
)
{
  qcril_qmi_nas_init_gsm_cell_identity_v6(
    cell,
    plmn,
    lac_valid,
    lac,
    cid_valid,
    cid
  );
  if (cell)
  {
    cell->arfcn = INT_MAX;
    cell->bsic  = 0xff;

    if (arfcn_valid)
    {
        cell->arfcn = arfcn;
    }

    if (bsic_valid)
    {
        cell->bsic = bsic;
    }
  }
}

void qcril_qmi_nas_init_gsm_cell_ss_v12
(
  RIL_CELL_INFO_LIST_GSM_SS_TYPE *cell,
  int radio_if,
  uint8_t  rx_lev_valid,
  uint16_t rx_lev,
  uint8_t  ber_valid,
  int      bitErrorRate,
  uint8_t  ta_valid,
  uint32_t ta
)
{
  qcril_qmi_nas_init_gsm_cell_ss_v6(
    cell,
    radio_if,
    rx_lev_valid,
    rx_lev,
    ber_valid,
    bitErrorRate
  );

  cell->timingAdvance = (!ta_valid || ta == 0xffffffff) ? INT_MAX : ta;
}

void qcril_qmi_nas_init_wcdma_cell_identity_v12
(
  RIL_CELL_INFO_LIST_WCDMA_ID_TYPE *cell,
  char *plmn,
  uint8_t  lac_valid,
  uint16_t lac,
  uint8_t  cid_valid,
  uint32_t cid,
  uint8_t  psc_valid,
  uint16_t psc,
  uint8_t  uarfcn_valid,
  uint16_t uarfcn
)
{
  qcril_qmi_nas_init_wcdma_cell_identity_v6(
          cell,
          plmn,
          lac_valid,
          lac,
          cid_valid,
          cid,
          psc_valid,
          psc
  );
  if (cell)
  {
    cell->uarfcn = INT_MAX;

    if (uarfcn_valid)
    {
        cell->uarfcn = uarfcn;
    }
  }
}

void qcril_qmi_nas_init_lte_cell_identity_v12
(
  RIL_CELL_INFO_LIST_LTE_ID_TYPE *cell,
  uint8_t *plmn,
  uint8_t  tac_valid,
  uint16_t tac,
  uint8_t  cid_valid,
  uint32_t cid,
  uint8_t  pci_valid,
  uint16_t pci,
  uint8_t  earfcn_valid,
  uint32_t earfcn
)
{
  qcril_qmi_nas_init_lte_cell_identity_v6(
    cell,
    plmn,
    tac_valid,
    tac,
    cid_valid,
    cid,
    pci_valid,
    pci
  );

  if (cell)
  {
    cell->earfcn = INT_MAX;

    if (earfcn_valid)
    {
        cell->earfcn = earfcn;
    }
  }
}

void qcril_qmi_nas_cell_identity_plmn_name
(
  char *alphaShort,
  size_t alphaShortLen,
  char *alphaLong,
  size_t alphaLongLen,
  char *plmn,
  nas_radio_if_enum_v01 rat
)
{
    int mcc;
    int mnc;
    char mcc_str[NAS_MCC_MNC_MAX_SIZE];
    char mnc_str[NAS_MCC_MNC_MAX_SIZE];

    mcc = QCRIL_MCC_FROM_BCD_PLMN(plmn);
    mnc = QCRIL_MNC_FROM_BCD_PLMN(plmn);
    snprintf( mcc_str, NAS_MCC_MNC_MAX_SIZE, "%03d", (int)mcc);
    if ( (mnc > 99) ||  QCRIL_MNC_INCLUDES_PCS(plmn) )
    {
        snprintf (mnc_str, NAS_MCC_MNC_MAX_SIZE, "%03d", (int)mnc);
    }
    else
    {
        snprintf (mnc_str, NAS_MCC_MNC_MAX_SIZE, "%02d", (int)mnc);
    }

    qcril_qmi_util_handle_centralized_short_long_eons(mcc_str,
        mnc_str,
        alphaShort,
        alphaShortLen,
        alphaLong,
        alphaLongLen,
        FALSE,
        rat,
        FALSE,
        0);
}


int qcril_cell_location_to_ril_cell_info_v12(struct qcril_chain_data *data)
{
  uint64_t *timestamp;
  unsigned totcells, ncells;
  int buflen;
  uint32_t i, j;
  int ret = FALSE;

  RIL_CELL_INFO_LIST_TYPE *cellinfobuf = NULL, *curr_cel, *srv_cell = NULL;
  nas_get_cell_location_info_resp_msg_v01 *li;
  QCRIL_LOG_DEBUG(
          "data: %x, data->parm1: %x,"
          " data->input: %x",
         data, data->parm1,
         data->input
  );
  if (QCRIL_RIL_VERSION >= 12 && data)
  {
    timestamp = (uint64_t *) data->parm1;
    li = (nas_get_cell_location_info_resp_msg_v01 *)data->input;
    do
    {
      if (!li || !timestamp || data->output)
      {
        break;
      }
      QCRIL_LOG_DEBUG("Running v12 conversion");
      totcells = ncells = qcril_qmi_nas_count_cell_location_info_cells(li);
      if (!ncells)
      {
        break;
      }
      buflen = ncells * sizeof(RIL_CELL_INFO_LIST_TYPE);
      qcril_malloc2(cellinfobuf, ncells);
      QCRIL_LOG_DEBUG("CellInfo slots allocated: %d. Buflen: %d, buf: %x", ncells, buflen, cellinfobuf);
      curr_cel = cellinfobuf;
      if (cellinfobuf)
      {
        if ( li->geran_info_valid )
        {
          qcril_qmi_nas_init_gsm_cell_v6(
                  (RIL_CELL_INFO_LIST_TYPE *)curr_cel,
                  TRUE,
                  timestamp
          );
          qcril_qmi_nas_init_gsm_cell_identity_v12(
                  &curr_cel->CellInfo.gsm.cellIdentityGsm,
                  li->geran_info.plmn,
                  TRUE,
                  li->geran_info.lac,
                  TRUE,
                  li->geran_info.cell_id,
                  TRUE,
                  li->geran_info.arfcn,
                  TRUE,
                  li->geran_info.bsic
          );
          qcril_qmi_nas_init_gsm_cell_ss_v12(
                  &curr_cel->CellInfo.gsm.signalStrengthGsm,
                  NAS_RADIO_IF_GSM,
                  TRUE,
                  li->geran_info.rx_lev,
                  FALSE,
                  99,
                  TRUE,
                  li->geran_info.timing_advance
          );

          qcril_qmi_nas_cell_identity_plmn_name(
            curr_cel->CellInfo.gsm.cellIdentityGsm.operatorNames.alphaShort,
            MAX_OPERATOR_NAME_LEN,
            curr_cel->CellInfo.gsm.cellIdentityGsm.operatorNames.alphaLong,
            MAX_OPERATOR_NAME_LEN,
            li->geran_info.plmn,
            NAS_RADIO_IF_GSM_V01
            );

          ++curr_cel;

          for (i = 0; i < li->geran_info.nmr_cell_info_len &&
                      (unsigned)(curr_cel - cellinfobuf) < ncells; i++)
          {
            qcril_qmi_nas_init_gsm_cell_v6(
                    (RIL_CELL_INFO_LIST_TYPE *)curr_cel,
                    FALSE,
                    timestamp
            );
            qcril_qmi_nas_init_gsm_cell_identity_v12(
                    &curr_cel->CellInfo.gsm.cellIdentityGsm,
                    li->geran_info.nmr_cell_info[i].nmr_plmn,
                    TRUE,
                    li->geran_info.nmr_cell_info[i].nmr_lac,
                    TRUE,
                    li->geran_info.nmr_cell_info[i].nmr_cell_id,
                    TRUE,
                    li->geran_info.nmr_cell_info[i].nmr_arfcn,
                    TRUE,
                    li->geran_info.nmr_cell_info[i].nmr_bsic
            );
            qcril_qmi_nas_init_gsm_cell_ss_v12(
                    &curr_cel->CellInfo.gsm.signalStrengthGsm,
                    NAS_RADIO_IF_GSM,
                    TRUE,
                    li->geran_info.nmr_cell_info[i].nmr_rx_lev,
                    FALSE,
                    99,
                    FALSE,
                    0xffffffff
            );

            qcril_qmi_nas_cell_identity_plmn_name(
              curr_cel->CellInfo.gsm.cellIdentityGsm.operatorNames.alphaShort,
              MAX_OPERATOR_NAME_LEN,
              curr_cel->CellInfo.gsm.cellIdentityGsm.operatorNames.alphaLong,
              MAX_OPERATOR_NAME_LEN,
              li->geran_info.nmr_cell_info[i].nmr_plmn,
              NAS_RADIO_IF_GSM_V01
              );

            ++curr_cel;
          }
        }

        if (li->cdma_info_valid)
        {
          RIL_SignalStrength ss;

          qcril_qmi_nas_initialize_cdma_ril_cellinfo(curr_cel,
                                                     TRUE,
                                                     RIL_TIMESTAMP_TYPE_OEM_RIL,
                                                     timestamp);
          curr_cel->CellInfo.cdma.cellIdentityCdma.networkId = li->cdma_info.nid;
          curr_cel->CellInfo.cdma.cellIdentityCdma.systemId = li->cdma_info.sid;
          curr_cel->CellInfo.cdma.cellIdentityCdma.basestationId = li->cdma_info.base_id;
          curr_cel->CellInfo.cdma.cellIdentityCdma.longitude = li->cdma_info.base_long;
          curr_cel->CellInfo.cdma.cellIdentityCdma.latitude = li->cdma_info.base_lat;

          // QMI_NAS_GET_CELL_LOCATION_INFO doesn't have SS information for CDMA.
          // Get from cache
          if (qcril_qmi_nas_signal_strength_con_conv_cache2ril(&ss) == E_SUCCESS)
          {
            if (ss.CDMA_SignalStrength.dbm != NAS_SIGNAL_STRENGTH_UNK)
            {
              curr_cel->CellInfo.cdma.signalStrengthCdma.dbm = ss.CDMA_SignalStrength.dbm;
            }
            if (ss.CDMA_SignalStrength.ecio != NAS_SIGNAL_STRENGTH_UNK)
            {
              curr_cel->CellInfo.cdma.signalStrengthCdma.ecio = ss.CDMA_SignalStrength.ecio;
            }

            if (ss.EVDO_SignalStrength.dbm != NAS_SIGNAL_STRENGTH_UNK)
            {
              curr_cel->CellInfo.cdma.signalStrengthEvdo.dbm = ss.EVDO_SignalStrength.dbm;
            }
            if (ss.EVDO_SignalStrength.ecio != NAS_SIGNAL_STRENGTH_UNK)
            {
              curr_cel->CellInfo.cdma.signalStrengthEvdo.ecio = ss.EVDO_SignalStrength.ecio;
            }
            if (ss.EVDO_SignalStrength.signalNoiseRatio != NAS_SIGNAL_STRENGTH_UNK)
            {
              curr_cel->CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio = ss.EVDO_SignalStrength.signalNoiseRatio;
            }
          }
          else
          {
            QCRIL_LOG_INFO("qcril_qmi_nas_signal_strength_con_conv_cache2ril returns error."
                           " No SS available for cdma/evdo");
          }

          curr_cel ++;
        }

        if (li->umts_info_valid)
        {
          qcril_qmi_nas_init_wcdma_cell_v6(
                  curr_cel,
                  TRUE,
                  timestamp
          );
          qcril_qmi_nas_init_wcdma_cell_identity_v12(
                  &curr_cel->CellInfo.wcdma.cellIdentityWcdma,
                  li->umts_info.plmn,
                  TRUE,
                  li->umts_info.lac,
                  li->umts_cell_id_valid,
                  li->umts_cell_id,
                  TRUE,
                  li->umts_info.psc,
                  TRUE,
                  li->umts_info.uarfcn
          );
          qcril_qmi_nas_init_wcdma_cell_ss_v6(
                  &curr_cel->CellInfo.wcdma.signalStrengthWcdma,
                  NAS_RADIO_IF_UMTS,
                  TRUE,
                  li->umts_info.rscp,
                  TRUE,
                  li->umts_info.ecio,
                  FALSE,
                  99
          );

          qcril_qmi_nas_cell_identity_plmn_name(
            curr_cel->CellInfo.wcdma.cellIdentityWcdma.operatorNames.alphaShort,
            MAX_OPERATOR_NAME_LEN,
            curr_cel->CellInfo.wcdma.cellIdentityWcdma.operatorNames.alphaLong,
            MAX_OPERATOR_NAME_LEN,
            li->umts_info.plmn,
            NAS_RADIO_IF_UMTS_V01
            );

          curr_cel ++;

          // UMTS Neighbors
          for (i = 0; i < li->umts_info.umts_monitored_cell_len && (unsigned)(curr_cel - cellinfobuf) < ncells; i++)
          {
            qcril_qmi_nas_init_wcdma_cell_v6(
                    curr_cel,
                    FALSE,
                    timestamp
            );
            qcril_qmi_nas_init_wcdma_cell_identity_v12(
                    &curr_cel->CellInfo.wcdma.cellIdentityWcdma,
                    NULL,
                    FALSE,
                    0xffff,
                    FALSE,
                    0xffff,
                    TRUE,
                    li->umts_info.umts_monitored_cell[i].umts_psc,
                    TRUE,
                    li->umts_info.umts_monitored_cell[i].umts_uarfcn
            );
            qcril_qmi_nas_init_wcdma_cell_ss_v6(
                    &curr_cel->CellInfo.wcdma.signalStrengthWcdma,
                    NAS_RADIO_IF_UMTS,
                    TRUE,
                    li->umts_info.umts_monitored_cell[i].umts_rscp,
                    TRUE,
                    li->umts_info.umts_monitored_cell[i].umts_ecio,
                    FALSE,
                    99
            );

            curr_cel++;
          }

          // GERAN Neighbors
          for (i = 0; i < li->umts_info.umts_geran_nbr_cell_len && (unsigned)(curr_cel - cellinfobuf) < ncells; i++)
          {
            qcril_qmi_nas_init_gsm_cell_v6(
                    curr_cel,
                    FALSE,
                    timestamp
            );
            qcril_qmi_nas_init_gsm_cell_identity_v12(
                    &curr_cel->CellInfo.gsm.cellIdentityGsm,
                    NULL,
                    FALSE,
                    0xffff,
                    FALSE,
                    0xffffffff,
                    TRUE,
                    li->umts_info.umts_geran_nbr_cell[i].geran_arfcn,
                    TRUE,
                    QCRIL_MAKE_BSIC(
                            li->umts_info.umts_geran_nbr_cell[i].geran_bsic_ncc,
                            li->umts_info.umts_geran_nbr_cell[i].geran_bsic_bcc
                    )
            );
            qcril_qmi_nas_init_gsm_cell_ss_v12(
                    &curr_cel->CellInfo.gsm.signalStrengthGsm,
                    NAS_RADIO_IF_UMTS,
                    TRUE,
                    li->umts_info.umts_geran_nbr_cell[i].geran_rssi,
                    FALSE,
                    99,
                    FALSE,
                    0xffffffff
            );
            curr_cel++;
          }

          if (i < li->umts_info.umts_geran_nbr_cell_len)
          {
            QCRIL_LOG_ERROR("Not enough RIL_CellInfo slots for Geran Neighbors. "
                            "Slots: %d. Need: %d more",
                            ncells,
                            li->umts_info.umts_geran_nbr_cell_len - i);
          }
        }

        if ( li->lte_intra_valid )
        {
          uint8_t srv_done = FALSE;

          srv_cell = curr_cel;
          curr_cel++;

          qcril_qmi_nas_init_lte_cell_v6(
            srv_cell,
            TRUE,
            timestamp
          );

          if(li->lte_intra_earfcn_valid)
          {
            qcril_qmi_nas_init_lte_cell_identity_v12(
              &srv_cell->CellInfo.lte.cellIdentityLte,
              li->lte_intra.plmn,
              TRUE,
              li->lte_intra.tac,
              TRUE,
              li->lte_intra.global_cell_id,
              TRUE,
              li->lte_intra.serving_cell_id,
              TRUE,
              li->lte_intra_earfcn
            );
          }
          else
          {
            qcril_qmi_nas_init_lte_cell_identity_v12(
              &srv_cell->CellInfo.lte.cellIdentityLte,
              li->lte_intra.plmn,
              TRUE,
              li->lte_intra.tac,
              TRUE,
              li->lte_intra.global_cell_id,
              TRUE,
              li->lte_intra.serving_cell_id,
              TRUE,
              li->lte_intra.earfcn
            );
          }

          qcril_qmi_nas_init_lte_cell_ss_v8(
            &srv_cell->CellInfo.lte.signalStrengthLte,
            NAS_RADIO_IF_LTE,
            FALSE,
            99,
            FALSE,
            0xffff,
            FALSE,
            0xffff,
            FALSE,
            INT_MAX,
            FALSE,
            INT_MAX,
            li->timing_advance_valid,
            li->timing_advance
          );
          srv_cell->CellInfo.lte.cellIdentityLte.bandwidth = getNasModule().getLteBw();

          qcril_qmi_nas_cell_identity_plmn_name(
            srv_cell->CellInfo.lte.cellIdentityLte.operatorNames.alphaShort,
            MAX_OPERATOR_NAME_LEN,
            srv_cell->CellInfo.lte.cellIdentityLte.operatorNames.alphaLong,
            MAX_OPERATOR_NAME_LEN,
            (char*)li->lte_intra.plmn,
            NAS_RADIO_IF_LTE_V01
            );

          // Neighbors
          for (i = 0; i < li->lte_intra.cells_len && ((unsigned)(curr_cel - cellinfobuf) < ncells || !srv_done); i++)
          {
            if (li->lte_intra.cells[i].pci == li->lte_intra.serving_cell_id)
            {
              /* This is for the Serving Cell */
              qcril_qmi_nas_init_lte_cell_ss_v8(
                      &srv_cell->CellInfo.lte.signalStrengthLte,
                      NAS_RADIO_IF_LTE,
                      TRUE,
                      li->lte_intra.cells[i].rssi,
                      TRUE,
                      li->lte_intra.cells[i].rsrp,
                      TRUE,
                      li->lte_intra.cells[i].rsrq,
                      FALSE,
                      INT_MAX,
                      FALSE,
                      INT_MAX,
                      li->timing_advance_valid,
                      li->timing_advance
              );

              QCRIL_LOG_DEBUG("Serving cell rsrp: %d. rsrq: %d. rssi: %d. srxlev: %d. output signal Strength: %d ta: %d",
                      li->lte_intra.cells[i].rsrp,
                      li->lte_intra.cells[i].rsrq,
                      li->lte_intra.cells[i].rssi,
                      li->lte_intra.cells[i].srxlev,
                      srv_cell->CellInfo.lte.signalStrengthLte.signalStrength,
                      li->timing_advance);

              QCRIL_LOG_DEBUG("Output rsrp: %d, rsrq: %d, rssnr: %d, cqi: %d, ta: %d"
                      " bandwidth: %d",
                      srv_cell->CellInfo.lte.signalStrengthLte.rsrp,
                      srv_cell->CellInfo.lte.signalStrengthLte.rsrq,
                      srv_cell->CellInfo.lte.signalStrengthLte.rssnr,
                      srv_cell->CellInfo.lte.signalStrengthLte.cqi,
                      srv_cell->CellInfo.lte.signalStrengthLte.timingAdvance,
                      srv_cell->CellInfo.lte.cellIdentityLte.bandwidth);
              srv_done = TRUE;
            }
            else if ( (unsigned)(curr_cel - cellinfobuf) >= ncells && !srv_done)
            {
                continue;
            }
            else
            {
              /* This is for a neighbor Cell */
              qcril_qmi_nas_init_lte_cell_v6(
                      curr_cel,
                      FALSE,
                      timestamp
              );

              if(li->lte_intra_earfcn_valid)
              {
                qcril_qmi_nas_init_lte_cell_identity_v12(
                      &curr_cel->CellInfo.lte.cellIdentityLte,
                      NULL,
                      FALSE,
                      0xffff,
                      FALSE,
                      INT_MAX,
                      TRUE,
                      li->lte_intra.cells[i].pci,
                      TRUE,
                      li->lte_intra_earfcn
                );
              }
              else
              {
                qcril_qmi_nas_init_lte_cell_identity_v12(
                      &curr_cel->CellInfo.lte.cellIdentityLte,
                      NULL,
                      FALSE,
                      0xffff,
                      FALSE,
                      INT_MAX,
                      TRUE,
                      li->lte_intra.cells[i].pci,
                      TRUE,
                      li->lte_intra.earfcn
                );
              }
              curr_cel->CellInfo.lte.cellIdentityLte.bandwidth = INT_MAX; // We don't know bandwidth for neighbors

              qcril_qmi_nas_init_lte_cell_ss_v8(
                      &curr_cel->CellInfo.lte.signalStrengthLte,
                      NAS_RADIO_IF_LTE,
                      TRUE,
                      li->lte_intra.cells[i].rssi,
                      TRUE,
                      li->lte_intra.cells[i].rsrp,
                      TRUE,
                      li->lte_intra.cells[i].rsrq,
                      FALSE,
                      INT_MAX,
                      FALSE,
                      INT_MAX,
                      FALSE,
                      INT_MAX
              );

              QCRIL_LOG_DEBUG("Neighbor cell rsrp: %d. rssi: %d. srxlev: %d. output signal Strength: %d"
                      " bandwidth: %d",
                      -(li->lte_intra.cells[i].rsrp / 10),
                      li->lte_intra.cells[i].rssi,
                      li->lte_intra.cells[i].srxlev,
                      curr_cel->CellInfo.lte.signalStrengthLte.signalStrength,
                      curr_cel->CellInfo.lte.cellIdentityLte.bandwidth);

              curr_cel++;
            }
          }

          if (i < li->lte_intra.cells_len)
          {
            QCRIL_LOG_ERROR("Not enough RIL_CellInfo slots for LTE - Intra. "
                            "Slots: %d. Want %d more",
                            ncells,
                            li->lte_intra.cells_len - i);
          }
        }

        if ( li->lte_inter_valid )
        {
          for (i = 0; i < li->lte_inter.freqs_len; i++)
          {
            for (j = 0; j < li->lte_inter.freqs[i].cells_len && (unsigned)(curr_cel - cellinfobuf) < ncells; j++)
            {
              qcril_qmi_nas_init_lte_cell_v6(
                      curr_cel,
                      FALSE,
                      timestamp
              );
              qcril_qmi_nas_init_lte_cell_identity_v12(
                      &curr_cel->CellInfo.lte.cellIdentityLte,
                      NULL,
                      FALSE,
                      0xffff,
                      FALSE,
                      INT_MAX,
                      TRUE,
                      li->lte_inter.freqs[i].cells[j].pci,
                      TRUE,
                      li->lte_inter.freqs[i].earfcn
              );
              curr_cel->CellInfo.lte.cellIdentityLte.bandwidth = INT_MAX; // We don't know bandwidth for neighbors
              qcril_qmi_nas_init_lte_cell_ss_v8(
                      &curr_cel->CellInfo.lte.signalStrengthLte,
                      NAS_RADIO_IF_LTE,
                      TRUE,
                      li->lte_inter.freqs[i].cells[j].rssi,
                      TRUE,
                      li->lte_inter.freqs[i].cells[j].rsrp,
                      TRUE,
                      li->lte_inter.freqs[i].cells[j].rsrq,
                      FALSE,
                      INT_MAX,
                      FALSE,
                      INT_MAX,
                      FALSE,
                      INT_MAX
              );

              QCRIL_LOG_DEBUG("Neighbor cell (inter) rsrp: %d. rssi: %d. srxlev: %d. output signal Strength: %d"
                      " bandwidth: %d",
                      -(li->lte_intra.cells[i].rsrp / 10),
                      li->lte_intra.cells[i].rssi,
                      li->lte_intra.cells[i].srxlev,
                      curr_cel->CellInfo.lte.signalStrengthLte.signalStrength,
                      curr_cel->CellInfo.lte.cellIdentityLte.bandwidth);

              curr_cel++;
            }
            if ( j < li->lte_inter.freqs[i].cells_len )
            {
              QCRIL_LOG_ERROR("Not enough RIL_CellInfo slots for LTE - Inter, freq %d. "
                              "Slots: %d. Want %d more",
                              i,
                              ncells,
                              li->lte_inter.freqs[i].cells_len - j);
            }
          }
        }

        if ( li->lte_gsm_valid )
        {
          for (i = 0; i < li->lte_gsm.freqs_len; i++)
          {
            for (j = 0; j < li->lte_gsm.freqs[i].cells_len && (unsigned)(curr_cel - cellinfobuf) < ncells; j++)
            {
              qcril_qmi_nas_init_gsm_cell_v6(
                      curr_cel,
                      FALSE,
                      timestamp
              );
              qcril_qmi_nas_init_gsm_cell_identity_v12(
                      &curr_cel->CellInfo.gsm.cellIdentityGsm,
                      NULL,
                      FALSE,
                      0xffff,
                      FALSE,
                      0xffffffff,
                      TRUE,
                      li->lte_gsm.freqs[i].cells[j].arfcn,
                      li->lte_gsm.freqs[i].cells[j].cell_id_valid,
                      li->lte_gsm.freqs[i].cells[j].bsic_id
              );
              qcril_qmi_nas_init_gsm_cell_ss_v12(
                      &curr_cel->CellInfo.gsm.signalStrengthGsm,
                      NAS_RADIO_IF_LTE,
                      TRUE,
                      li->lte_gsm.freqs[i].cells[j].rssi,
                      FALSE,
                      99,
                      FALSE,
                      0xffffffff
              );

              curr_cel++;
            }
            if ( j < li->lte_gsm.freqs[i].cells_len )
            {
              QCRIL_LOG_ERROR("Not enough RIL_CellInfo slots for LTE - GSM Neighbors, freq %d. "
                              "Slots: %d. Want %d more",
                              i,
                              ncells,
                              li->lte_gsm.freqs[i].cells_len - j);
            }
          }
        }

        if ( li->lte_wcdma_valid )
        {
          for (i = 0; i < li->lte_wcdma.freqs_len; i++)
          {
            for (j = 0; j < li->lte_wcdma.freqs[i].cells_len && (unsigned)(curr_cel - cellinfobuf) < ncells; j++)
            {
              qcril_qmi_nas_init_wcdma_cell_v6(
                      (RIL_CELL_INFO_LIST_TYPE *)curr_cel,
                      FALSE,
                      timestamp
              );
              qcril_qmi_nas_init_wcdma_cell_identity_v12(
                      &curr_cel->CellInfo.wcdma.cellIdentityWcdma,
                      NULL,
                      FALSE,
                      0xffff,
                      FALSE,
                      0xffff,
                      TRUE,
                      li->lte_wcdma.freqs[i].cells[j].psc,
                      TRUE,
                      li->lte_wcdma.freqs[i].uarfcn
              );
              qcril_qmi_nas_init_wcdma_cell_ss_v6(
                      &curr_cel->CellInfo.wcdma.signalStrengthWcdma,
                      NAS_RADIO_IF_LTE,
                      TRUE,
                      li->lte_wcdma.freqs[i].cells[j].cpich_rscp,
                      TRUE,
                      li->lte_wcdma.freqs[i].cells[j].cpich_ecno,
                      FALSE,
                      99
              );

              curr_cel++;
            }
            if ( j < li->lte_wcdma.freqs[i].cells_len )
            {
              QCRIL_LOG_ERROR("Not enough RIL_CellInfo slots for LTE - WCDMA Neighbors, freq %d. "
                              "Slots: %d. Want %d more",
                              i,
                              ncells,
                              li->lte_wcdma.freqs[i].cells_len - j);
            }
          }
        }
      }
      data->output = cellinfobuf;

      data->ret1.v.ui = ((unsigned)(curr_cel - cellinfobuf) * sizeof(RIL_CELL_INFO_LIST_TYPE));
      data->ret2.v.ui = (unsigned long)(curr_cel - cellinfobuf);

      ret = TRUE;
    }while(0);
  }
  return ret;
}

