/*****************************************************************************
* Copyright (c) 2013-2016, 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*****************************************************************************/
#include "QMobicatComposer.h"

extern "C" {
#include "mct_stream.h"
#include "eztune_vfe_diagnostics.h"
#include "eztune_diagnostics.h"
#include <stdlib.h>
}
#include "QICommon.h"

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

static const char ae_string[] = "QCAEC";
static const char awb_string[] = "QCAWB";
static const char af_string[] = "QC_AF";
static const char haf_string[] = "QCHAF";
static const char tof_string[] = "QCTOF";
static const char dciaf_string[] = "QCDAF";
static const char pdaf_string[] = "QCPAF";
static const char asd_string[] = "QCASD";
static const char stats_string[] = "QCSTS";
static const char bestats_string[] = "QCBES";
static const char af_stats_string[] = "QCAFS";
/*
 * Stats string for Bayer Histogram stats information.
 */
static const char bhist_stats_string[] = "QCHST";
/*
 * Stats string for 3A Tuning information.
 */
static const char q3a_tuning_string[] = "QCTUN";

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
/* =======================================================================
**                          Macro Definitions
** ======================================================================= */

#define MAX_MOBICAT_LENGTH 60000
#define MAX_STATS_DATA_LENGTH (65535*4) /* temp fix to increase stats buffer size */
#define MAX_MI_DATA_LENGTH 65535
/**
 * PARSE_F - parse a float value
 * @T - mobicat tag
 * @V - value to parse
 */
#define PARSE_F(T,V)    (parseVal<float>("%f", T, V))

/**
 * PARSE_F_UNIT - parse a float value
 * @B - buffer info
 * @T - mobicat tag
 * @V - value to parse
 * @U - Unit to append
 */
#define PARSE_F_UNIT(B,T,V,U)    (parseValUnit<float>(B, "%f", T, V, U))

/**
 * PARSE_U - parse an unsigned value
 * @T - mobicat tag
 * @V - value to parse
 */
#define PARSE_U(T,V)    (parseVal<uint32_t>("%u", T, V))

/**
 * PARSE_U_Unit - parse an unsigned value
 * @B - buffer info
 * @T - mobicat tag
 * @V - value to parse
 * @U - Unit to append
 */
#define PARSE_U_UNIT(B,T,V,U)    (parseValUnit<uint32_t>(B, "%u", T, V, U))

/**
 * PARSE_U16 - parse an uint16_t value
 * @T - mobicat tag
 * @V - value to parse
 */
#define PARSE_U16(T,V)    (parseVal<uint16_t>("%hu", T, V))

/**
 * PARSE_U16_UNIT - parse an uint16_t value
 * @B - buffer Info
 * @T - mobicat tag
 * @V - value to parse
 * @U - Unit to append
 */
#define PARSE_U16_UNIT(B,T,V,U)    (parseValUnit<uint16_t>(B, "%hu", T, V, U))

/**
 * PARSE_I16 - parse an uint16_t value
 * @T - mobicat tag
 * @V - value to parse
 */
#define PARSE_I16(T,V)    (parseVal<uint16_t>("%hd", T, V))

/**
 * PARSE_S_UNTT - parse a string value
 * @B - Buffer info
 * @T - mobicat tag
 * @V - value to parse
 * @U - unit to append
 */
#define PARSE_S_UNIT(B,T,V,U)    (parseValUnit<const char*>(B,"%s", T, V, U))

/**
 * PARSE_C - parse a char value
 * @T - mobicat tag
 * @V - value to parse
 */
#define PARSE_C(T,V)    (parseVal<char>("%hhd", T, V))

/**
 * PARSE_C - parse a char value
 * @T - mobicat tag
 * @V - value to parse
 */
#define PARSE_UC(T,V)    (parseVal<unsigned char>("%hhu", T, V))

/**
 * PARSE_I - parse an int value
 * @T - mobicat tag
 * @V - value to parse
 */
#define PARSE_I(T,V)    (parseVal<int>("%d", T, V))

/**
 * PARSE_I - parse an int value
 * @B - Buffer info
 * @T - mobicat tag
 * @V - value to parse
 * @U - unit to append
 */
#define PARSE_I_UNIT(B,T,V,U)    (parseValUnit<int>(B, "%d", T, V, U))

/**
 * PARSE_SI - parse a short int value
 * @T - mobicat tag
 * @V - value to parse
 */
#define PARSE_SI(T,V)   (parseVal<short>("%hd", T, V))

/**
 * PARSE_F_A1 - parse a 1D float array
 * @T - mobicat tag
 * @V - array to parse
 */
#define PARSE_F_A1(T,V) (parseValArr<float>("%f", T, &(V[0]), \
  (int32_t)(sizeof(V)/sizeof(V[0]))))

/**
 * PARSE_ALL_F_A1 - parse a 1D float array
 * @B - Buffer info
 * @T - mobicat tag
 * @V - array to parse
 */
#define PARSE_ALL_F_A1(B,T,V) (parseValArr<float>(B, "%f", T, &(V[0]), \
  (int32_t)(sizeof(V)/sizeof(V[0]))))

/**
 * PARSE_I_A1 - parse a 1D int array
 * @T - mobicat tag
 * @V - array to parse
 */
#define PARSE_I_A1(T,V) (parseValArr<int>("%d", T, &(V[0]), \
  (int32_t)(sizeof(V)/sizeof(V[0]))))

/**
 * PARSE_SI_A1 - parse a 1D int array
 * @T - mobicat tag
 * @V - array to parse
 */
#define PARSE_SI_A1(T,V) (parseValArr<short>("%hd", T, &(V[0]), \
  (int32_t)(sizeof(V)/sizeof(V[0]))))

/**
 * PARSE_U_A1 - parse a 1D unsigned array
 * @T - mobicat tag
 * @V - array to parse
 */
#define PARSE_U_A1(T,V) (parseValArr<uint32_t>("%lu", T, &(V[0]), \
  (int32_t)(sizeof(V)/sizeof(V[0]))))

/**
 * PARSE_U16_A1 - parse an uint16_t value
 * @T - mobicat tag
 * @V - value to parse
 */
#define PARSE_U16_A1(T,V) (parseValArr<uint16_t>("%hu", T, &(V[0]), \
  (int32_t)(sizeof(V)/sizeof(V[0]))))

/**
 * PARSE_F_A2 - parse a 2D float array
 * @T - mobicat tag
 * @V - array to parse
 */
#define PARSE_F_A2(T,V) (parseValArr<float>("%f", T, &(V[0][0]), \
  (int32_t)(sizeof(V)/sizeof(V[0][0]))))

#define ARR_SZ(a) (sizeof(a)/sizeof(a[0]))

/* =======================================================================
**                          Function Definitions
** ======================================================================= */

/*===========================================================================
 * Function: QMobicatComposer
 *
 * Description: Constuctor
 *
 * Input parameters: none
 *
 * Return values: none
 *
 * Notes: none
 *==========================================================================*/
QMobicatComposer::QMobicatComposer()
{
  mScratchBuf = 0;
  mMobicatStr = 0;
  mStatsPayload = 0;
  mStats_payload_size = 0;
  mMIPayload = 0;
  mMIPayload_size = 0;
}

/*===========================================================================
 * Function: ~QMobicatComposer
 *
 * Description: destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QMobicatComposer::~QMobicatComposer()
{
  mStats_payload_size = 0;
  if (mScratchBuf) {
    free(mScratchBuf);
  }
  if (mMobicatStr) {
    free(mMobicatStr);
  }
  if (mStatsPayload) {
    free(mStatsPayload);
  }
  if (mMIPayload) {
    free(mMIPayload);
  }
}

/*===========================================================================
 * Function: Compose3AStatsPayload
 *
 * Description: Compose 3A payload with
 *              AEC, AWB, AF, and ASD data
 *
 * Input parameters:
 *   metadata - metadata
 *
 * Return values:
 *   char* - composed payload
 *
 * Notes: none
 *==========================================================================*/
char* QMobicatComposer::Compose3AStatsPayload(uint8_t *metadata)
{
  metadata_buffer_t *lMeta;
  mct_stream_session_metadata_info *lSessionMeta;
  uint8_t curr_entry;

  /* typecast private meta data */
  lMeta = (metadata_buffer_t *) metadata;
  if (!lMeta) {
    QIDBG_ERROR("%s:%d] Error retrieving metadata", __func__, __LINE__);
    return 0;
  }
  lSessionMeta = (mct_stream_session_metadata_info *)
    POINTER_OF_META(CAM_INTF_META_PRIVATE_DATA, lMeta);

  mStatsPayload = (char *)malloc(MAX_STATS_DATA_LENGTH * sizeof(char));
  if (mStatsPayload == NULL) {
    QIDBG_ERROR("%s:%d] Error, cannot malloc 3A buf", __func__, __LINE__);
    return 0;
  }
  memset(mStatsPayload, 0x0, MAX_STATS_DATA_LENGTH * sizeof(char));

  IF_META_AVAILABLE(cam_q3a_tuning_info_t, q3a_tuning_ptr,
      CAM_INTF_META_EXIF_DEBUG_3A_TUNING, lMeta) {
    Extract3ATuningData(lMeta, q3a_tuning_ptr, TRUE);
  } else {
    Extract3ATuningData(lMeta, NULL, FALSE);
  }

  IF_META_AVAILABLE(cam_ae_exif_debug_t, ae_ptr, CAM_INTF_META_EXIF_DEBUG_AE, lMeta) {
    ExtractAECData(lMeta, ae_ptr, TRUE);
  } else {
    ExtractAECData(lMeta, NULL, FALSE);
  }

  IF_META_AVAILABLE(cam_awb_exif_debug_t, awb_ptr, CAM_INTF_META_EXIF_DEBUG_AWB, lMeta) {
    ExtractAWBData(lMeta, awb_ptr, TRUE);
  } else {
    ExtractAWBData(lMeta, NULL, FALSE);
  }

  IF_META_AVAILABLE(cam_af_exif_debug_t, af_ptr, CAM_INTF_META_EXIF_DEBUG_AF, lMeta) {
    ExtractAFData(lMeta, af_ptr, TRUE);
  } else {
    ExtractAFData(lMeta, NULL, FALSE);
  }

  IF_META_AVAILABLE(cam_asd_exif_debug_t, asd_ptr, CAM_INTF_META_EXIF_DEBUG_ASD, lMeta) {
    ExtractASDData(lMeta, asd_ptr, TRUE);
  } else {
    ExtractASDData(lMeta, NULL, FALSE);
  }

  IF_META_AVAILABLE(cam_stats_buffer_exif_debug_t, stats_ptr, CAM_INTF_META_EXIF_DEBUG_STATS, lMeta) {
    ExtractStatsData(lMeta, stats_ptr, TRUE);
  } else {
    ExtractStatsData(lMeta, NULL, FALSE);
  }

  IF_META_AVAILABLE(cam_bestats_buffer_exif_debug_t, bestats_ptr, CAM_INTF_META_EXIF_DEBUG_BESTATS, lMeta) {
    ExtractBEStatsData(lMeta, bestats_ptr, TRUE);
  } else {
    ExtractBEStatsData(lMeta, NULL, FALSE);
  }

  IF_META_AVAILABLE(cam_bhist_buffer_exif_debug_t, bhist_ptr,
      CAM_INTF_META_EXIF_DEBUG_BHIST, lMeta) {
    ExtractBHistData(lMeta, bhist_ptr, TRUE);
  } else {
    ExtractBHistData(lMeta, NULL, FALSE);
  }


  QIDBG_MED("%s:%d] [MOBICAT_DBG] stats debug payload size %d", __func__,
    __LINE__, mStats_payload_size);
  return mStatsPayload;
}

/*===========================================================================
 * Function: ExtractAECData
 *
 * Description: retrieve AEC data
 *
 * Input parameters:
 *   lMeta - metadata
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QMobicatComposer::ExtractAECData(metadata_buffer_t *lMeta,
  cam_ae_exif_debug_t *ae_ptr, boolean valid)
{
  cam_ae_exif_debug_t *data_ptr = 0;
  int32_t data_size = 0;
  uint64_t sw_version_num = 0;

 if (valid) {
    data_ptr = ae_ptr;
    data_size = data_ptr->aec_debug_data_size;
  }

  if (data_size == 0 && lMeta->is_valid[CAM_INTF_META_EXIF_DEBUG_AE]) {
      cam_ae_exif_debug_t *statsdebug_ae_data = POINTER_OF_META(CAM_INTF_META_EXIF_DEBUG_AE, lMeta);
      QIDBG_ERROR("%s:%d] [MOBICAT_DBG] AEC data not valid, retreiving from "
          "saved metadata params", __func__, __LINE__);
      if (NULL != statsdebug_ae_data) {
        data_ptr = statsdebug_ae_data;
        data_size = data_ptr->aec_debug_data_size;
      }
  }

 QIDBG_MED("%s:%d] [MOBICAT_DBG] AEC: size %d" , __func__, __LINE__,data_size);

  if (data_size > 0) {
    /* Write AEC Debug Information Identifier */
    memcpy(mStatsPayload + mStats_payload_size, ae_string, sizeof(ae_string));
    mStats_payload_size += sizeof(ae_string);

    /* Write AEC Debug Information Size, 4 bytes */
    memcpy(mStatsPayload + mStats_payload_size, &data_size, 4);
    mStats_payload_size += 4;

    /* Write AEC Debug Information */
    memcpy(mStatsPayload + mStats_payload_size,
      &data_ptr->aec_private_debug_data, data_size);
    mStats_payload_size += data_size;
  } else {
    QIDBG_ERROR("%s:%d] [MOBICAT_DBG] AEC size invalid, not adding to "
      "stats debug header", __func__, __LINE__);
  }
}

/*===========================================================================
 * Function: ExtractAWBData
 *
 * Description: retrieve AWB data
 *
 * Input parameters:
 *   lMeta - metadata
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QMobicatComposer::ExtractAWBData(metadata_buffer_t *lMeta,
  cam_awb_exif_debug_t *awb_ptr, boolean valid)
{
  cam_awb_exif_debug_t *data_ptr = 0;
  int32_t data_size = 0;

  if (valid) {
    data_ptr = awb_ptr;
    data_size = data_ptr->awb_debug_data_size;
  }
  if (data_size == 0 && lMeta->is_valid[CAM_INTF_META_EXIF_DEBUG_AWB]) {
    cam_awb_exif_debug_t *statsdebug_awb_data =
        POINTER_OF_META(CAM_INTF_META_EXIF_DEBUG_AWB, lMeta);
    QIDBG_ERROR("%s:%d] [MOBICAT_DBG] AWB data not valid, retreiving from "
      "saved metadata params", __func__, __LINE__);
    if (NULL != statsdebug_awb_data) {
      data_ptr = statsdebug_awb_data;
      data_size = data_ptr->awb_debug_data_size;
    }
  }

  QIDBG_MED("%s:%d] [MOBICAT_DBG] AWB: size %d", __func__, __LINE__,
    data_size);

  if (data_size > 0) {
    /* Write AWB Debug Information Identifier */
    memcpy(mStatsPayload + mStats_payload_size, awb_string, sizeof(awb_string));
    mStats_payload_size += sizeof(awb_string);

    /* Write AWB Debug Information Size, 4 bytes */
    memcpy(mStatsPayload + mStats_payload_size, &data_size, 4);
    mStats_payload_size += 4;

    /* Write AWB Debug Information */
    memcpy(mStatsPayload + mStats_payload_size,
      &data_ptr->awb_private_debug_data, data_size);
    mStats_payload_size += data_size;
  } else {
    QIDBG_ERROR("%s:%d] [MOBICAT_DBG] AWB size invalid, not adding to "
      "stats debug header", __func__, __LINE__);
  }
}

/*===========================================================================
 * Function: ExtractAFData
 *
 * Description: retrieve AF data
 *
 * Input parameters:
 *   lMeta - metadata
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QMobicatComposer::ExtractAFData(metadata_buffer_t *lMeta,
  cam_af_exif_debug_t *af_ptr, boolean valid)
{
  cam_af_exif_debug_t *data_ptr = 0;
  int32_t af_size = 0, haf_size = 0, tof_size = 0;
  int32_t dciaf_size = 0, pdaf_size = 0, af_stats_size = 0;

  if (valid) {
    data_ptr = af_ptr;
    af_size = data_ptr->af_debug_data_size;
  }
  if (af_size == 0 && lMeta->is_valid[CAM_INTF_META_EXIF_DEBUG_AF]) {
    cam_af_exif_debug_t *statsdebug_af_data = POINTER_OF_META(CAM_INTF_META_EXIF_DEBUG_AF, lMeta);
    QIDBG_ERROR("%s:%d] [MOBICAT_DBG] AF data not valid, retreiving from "
      "saved metadata params", __func__, __LINE__);
    if (NULL != statsdebug_af_data) {
      data_ptr = statsdebug_af_data;
      af_size = data_ptr->af_debug_data_size;
    }
  }

  QIDBG_MED("%s:%d] [MOBICAT_DBG] size AF: %d", __func__, __LINE__, af_size);

  if (af_size > 0) {
    /* Write AF Debug Information Identifier */
    memcpy(mStatsPayload + mStats_payload_size, af_string, sizeof(af_string));
    mStats_payload_size += sizeof(af_string);

    /* Write AF Debug Information Size, 4 bytes */
    memcpy(mStatsPayload + mStats_payload_size, &af_size, 4);
    mStats_payload_size += 4;

    /* Write AF Debug Information */
    memcpy(mStatsPayload + mStats_payload_size,
         data_ptr->af_private_debug_data, af_size);
    mStats_payload_size += af_size;

  } else {
    QIDBG_ERROR("%s:%d] [MOBICAT_DBG] AF size invalid, not adding to "
      "stats debug header", __func__, __LINE__);
  }
}

/*===========================================================================
 * Function: ExtractASDData
 *
 * Description: retrieve ASD data
 *
 * Input parameters:
 *   lMeta - metadata
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QMobicatComposer::ExtractASDData(metadata_buffer_t *lMeta,
  cam_asd_exif_debug_t *asd_ptr, boolean valid)
{
  cam_asd_exif_debug_t *data_ptr = 0;
  int32_t data_size = 0;

  if (valid) {
      data_ptr = asd_ptr;
      data_size = data_ptr->asd_debug_data_size;
  }
  if (data_size == 0 && lMeta->is_valid[CAM_INTF_META_EXIF_DEBUG_ASD]) {
      cam_asd_exif_debug_t *statsdebug_asd_data =
          POINTER_OF_META(CAM_INTF_META_EXIF_DEBUG_ASD, lMeta);
      QIDBG_ERROR("%s:%d] [MOBICAT_DBG] ASD data not valid, retreiving from "
        "saved metadata params", __func__, __LINE__);
      if (NULL != statsdebug_asd_data) {
        data_ptr = statsdebug_asd_data;
        data_size = data_ptr->asd_debug_data_size;
      }
  }

  QIDBG_MED("%s:%d] [MOBICAT_DBG] ASD: size %d" , __func__, __LINE__,
   data_size);

  if (data_size > 0) {
    /* Write ASD Debug Information Identifier */
    memcpy(mStatsPayload + mStats_payload_size, asd_string, sizeof(asd_string));
    mStats_payload_size += sizeof(asd_string);

    /* Write ASD Debug Information Size, 4 bytes */
    memcpy(mStatsPayload + mStats_payload_size, &data_size, 4);
    mStats_payload_size += 4;

    /* Write ASD Debug Information */
    memcpy(mStatsPayload + mStats_payload_size,
      &data_ptr->asd_private_debug_data, data_size);
    mStats_payload_size += data_size;
  } else {
    QIDBG_ERROR("%s:%d] [MOBICAT_DBG] ASD size invalid, not adding to "
      "stats debug header", __func__, __LINE__);
  }
}


/*===========================================================================
 * Function: ExtractStatsData
 *
 * Description: retrieve Stats data
 *
 * Input parameters:
 *   lMeta - metadata
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QMobicatComposer::ExtractStatsData(metadata_buffer_t *lMeta,
  cam_stats_buffer_exif_debug_t *stats_ptr, boolean valid)
{
  cam_stats_buffer_exif_debug_t *data_ptr = 0;
  int32_t data_size = 0;

  if (valid) {
    data_ptr = stats_ptr;
    data_size = data_ptr->bg_stats_buffer_size +
      data_ptr->bg_config_buffer_size;
  }

  if (data_size == 0 && lMeta->is_valid[CAM_INTF_META_EXIF_DEBUG_STATS]) {
      cam_stats_buffer_exif_debug_t *statsdebug_stats_buffer_data =
          POINTER_OF_META(CAM_INTF_META_EXIF_DEBUG_STATS, lMeta);
      QIDBG_ERROR("%s:%d] [MOBICAT_DBG] STATS data not valid, retreiving from "
         "saved metadata params", __func__, __LINE__);
      if (NULL != statsdebug_stats_buffer_data) {
        data_ptr = statsdebug_stats_buffer_data;
        data_size = data_ptr->bg_stats_buffer_size +
                  data_ptr->bg_config_buffer_size;
      }
  }

  QIDBG_MED("%s:%d] [MOBICAT_DBG] STATS: size %d" , __func__, __LINE__,
    data_size);

  if (data_size > 0) {
    /* Write Stats Debug Information Identifier */
    memcpy(mStatsPayload + mStats_payload_size,
      stats_string, sizeof(stats_string));
    mStats_payload_size += sizeof(stats_string);

    /* Write Stats Debug Information Size, 4 bytes */
    memcpy(mStatsPayload + mStats_payload_size, &data_size, 4);
    mStats_payload_size += 4;

    /* Write Stats Debug Information */
    memcpy(mStatsPayload + mStats_payload_size,
      &data_ptr->stats_buffer_private_debug_data, data_size);
    mStats_payload_size += data_size;
  } else {
    QIDBG_ERROR("%s:%d] [MOBICAT_DBG] Stats size invalid, not adding to "
      "stats debug header", __func__, __LINE__);
  }
}

/*===========================================================================
 * Function: ExtractBEStatsData
 *
 * Description: retrieve BE Stats data
 *
 * Input parameters:
 *   lMeta - metadata
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
 void QMobicatComposer::ExtractBEStatsData(metadata_buffer_t *lMeta,
    cam_bestats_buffer_exif_debug_t *bestats_ptr, boolean valid)
{
  cam_bestats_buffer_exif_debug_t *data_ptr = 0;
  int32_t data_size = 0;

  if (valid) {
    data_ptr = bestats_ptr;
    data_size = data_ptr->be_stats_buffer_size +
                data_ptr->be_config_buffer_size;
  }

  if (data_size == 0 && lMeta->is_valid[CAM_INTF_META_EXIF_DEBUG_BESTATS]) {
      cam_bestats_buffer_exif_debug_t *statsdebug_bestats_buffer_data =
          POINTER_OF_META(CAM_INTF_META_EXIF_DEBUG_BESTATS, lMeta);
      QIDBG_ERROR("[MOBICAT_DBG] BE STATS data not valid, retreiving from "
          "saved metadata params");
      if (NULL != statsdebug_bestats_buffer_data) {
        data_ptr = statsdebug_bestats_buffer_data;
        data_size = data_ptr->be_stats_buffer_size +
                  data_ptr->be_config_buffer_size;
      }
  }

  QIDBG_MED("[MOBICAT_DBG] BE STATS: size %d", data_size);

  if (data_size > 0) {
    /* Write BE Stats Debug Information Identifier */
    memcpy(mStatsPayload + mStats_payload_size,
      bestats_string, sizeof(bestats_string));
    mStats_payload_size += sizeof(bestats_string);

    /* Write BE Stats Debug Information Size, 4 bytes */
    memcpy(mStatsPayload + mStats_payload_size, &data_size, 4);
    mStats_payload_size += 4;

    /* Write BE Stats Debug Information */
    memcpy(mStatsPayload + mStats_payload_size,
      &data_ptr->bestats_buffer_private_debug_data, data_size);
    mStats_payload_size += data_size;
  } else {
    QIDBG_ERROR("[MOBICAT_DBG] BE Stats size invalid, not adding to stats debug header");
  }
}

/*===========================================================================
 * Function: ExtractBHistData
 *
 * Description: Retrieve BHIST data
 *
 * Input parameters:
 *   lMeta - metadata
 *   bhist_ptr - pointer to the BHIST EXIF data
 *   valid - flag that indicates whether the pointer is valid
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QMobicatComposer::ExtractBHistData(metadata_buffer_t *lMeta,
  cam_bhist_buffer_exif_debug_t *bhist_ptr, boolean valid)
{
  cam_bhist_buffer_exif_debug_t *data_ptr = 0;
  int32_t data_size = 0;

  if (valid) {
    data_ptr = bhist_ptr;
    data_size = data_ptr->bhist_stats_buffer_size;
  }
  if (data_size == 0 && lMeta->is_valid[CAM_INTF_META_EXIF_DEBUG_BHIST]) {
      cam_bhist_buffer_exif_debug_t *statsdebug_bhist_data =
          POINTER_OF_META(CAM_INTF_META_EXIF_DEBUG_BHIST, lMeta);
      QIDBG_ERROR("%s:%d] [MOBICAT_DBG] BHIST data not valid, retreiving from "
          "saved metadata params", __func__, __LINE__);
      if (NULL != statsdebug_bhist_data) {
        data_ptr =  statsdebug_bhist_data;
        data_size = statsdebug_bhist_data->bhist_stats_buffer_size;
      }
  }

  QIDBG_MED("%s:%d] [MOBICAT_DBG] BHIST: size %d" , __func__, __LINE__,
    data_size);

  if (data_size > 0) {
    /* Write BHist Debug Information Identifier */
    memcpy(mStatsPayload + mStats_payload_size,
      bhist_stats_string, sizeof(bhist_stats_string));
    mStats_payload_size += sizeof(bhist_stats_string);

    /* Write BHist Debug Information Size, 4 bytes */
    memcpy(mStatsPayload + mStats_payload_size, &data_size, 4);
    mStats_payload_size += 4;

    /* Write Bhist Debug Information */
    memcpy(mStatsPayload + mStats_payload_size,
      &data_ptr->bhist_private_debug_data, data_size);
    mStats_payload_size += data_size;
  } else {
    QIDBG_ERROR("%s:%d] [MOBICAT_DBG] BHIST stats size invalid, not adding to "
      "stats debug header", __func__, __LINE__);
  }
}

/*===========================================================================
 * Function: Extract3ATuningData
 *
 * Description: Retrieve 3A Tuning data
 *
 * Input parameters:
 *   lMeta - metadata
 *   q3a_tuning_ptr - pointer to the 3A Tuning EXIF data
 *   valid - flag that indicates whether the pointer is valid
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QMobicatComposer::Extract3ATuningData(metadata_buffer_t *lMeta,
  cam_q3a_tuning_info_t *q3a_tuning_ptr, boolean valid)
{
  cam_q3a_tuning_info_t*data_ptr = 0;
  int32_t data_size = 0;

  if (valid && q3a_tuning_ptr) {
    data_ptr = q3a_tuning_ptr;
    data_size = q3a_tuning_ptr->tuning_info_buffer_size;
  }

  if (data_size == 0 && lMeta->is_valid[CAM_INTF_META_EXIF_DEBUG_3A_TUNING]) {
      cam_q3a_tuning_info_t *statsdebug_3a_tuning_data =
          POINTER_OF_META(CAM_INTF_META_EXIF_DEBUG_3A_TUNING, lMeta);
      QIDBG_ERROR("%s:%d] [MOBICAT_DBG] 3A Tuning data not valid, retreiving "
          "from saved metadata params", __func__, __LINE__);
      if (NULL != statsdebug_3a_tuning_data) {
        data_ptr = statsdebug_3a_tuning_data;
        data_size = statsdebug_3a_tuning_data->tuning_info_buffer_size;
      }
  }

  QIDBG_MED("%s:%d] [MOBICAT_DBG] 3A Tuning data: size %d" , __func__,
      __LINE__, data_size);

  if (data_size > 0) {

    /* Write 3A Tuning Debug Information Identifier */
    memcpy(mStatsPayload + mStats_payload_size,
      q3a_tuning_string, sizeof(q3a_tuning_string));
    mStats_payload_size += sizeof(q3a_tuning_string);

    /* Write 3A Tuning Debug Information Size, 4 bytes */
    memcpy(mStatsPayload + mStats_payload_size, &data_size, 4);
    mStats_payload_size += 4;

    /* Write the 3A Tuning Debug Information.*/
    memcpy(mStatsPayload + mStats_payload_size,
      data_ptr->tuning_info_private_debug_data, data_size);
    mStats_payload_size += data_size;

  } else {
    QIDBG_ERROR("%s:%d] [MOBICAT_DBG] 3A tuning size invalid, not adding to "
      "stats debug header", __func__, __LINE__);
  }
}

/*===========================================================================
 * Function: Get3AStatsSize
 *
 * Description: return size of the stats payload
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   uint32_t - size
 *==========================================================================*/
uint32_t QMobicatComposer::Get3AStatsSize()
{
  return mStats_payload_size;
}

/*===========================================================================
 * Function: ParseMobicatData
 *
 * Description: Parse metadata into mobicat tags and compose string
 *
 * Input parameters:
 *   metadata - metadata
 *
 * Return values:
 *   char* - composed mobicat string
 *
 * Notes: none
 *==========================================================================*/
char* QMobicatComposer::ParseMobicatData(uint8_t *metadata)
{
  int i = 0;

  /* Metadata v3 declaration */
  metadata_buffer_t *lMeta;

  mct_stream_session_metadata_info *lSessionMeta;
  awb_update_t lAwbData;
  stats_get_data_t lAecData;
  vfe_diagnostics_t *lvfeDiag = NULL;
  ez_pp_params_t *lEzPpParams = NULL;
  uint8_t is_aec_valid = 0;
  uint8_t is_chromatix_lite_isp_valid = 0;
  uint8_t is_chromatix_lite_pp_valid = 0;
  uint8_t curr_entry;

  /* typecast private meta data */
  lMeta = (metadata_buffer_t *) metadata;

  if (!lMeta) {
    QIDBG_ERROR("%s:%d] Error retrieving metadata", __func__, __LINE__);
    return 0;
  }
  lSessionMeta = (mct_stream_session_metadata_info *)
    POINTER_OF_META(CAM_INTF_META_PRIVATE_DATA, lMeta);


  memcpy(&lAwbData, &lSessionMeta->isp_stats_awb_data.private_data,
    sizeof(lAwbData));
  memcpy(&lAecData, &lSessionMeta->stats_aec_data.private_data,
    sizeof(lAecData));

  IF_META_AVAILABLE(cam_3a_params_t, lAecParams,
        CAM_INTF_META_AEC_INFO, lMeta) {
    is_aec_valid = 1;
  } else if (lMeta->is_valid[CAM_INTF_META_AEC_INFO]) {
      cam_3a_params_t *mobicat_aec_params = POINTER_OF_META(CAM_INTF_META_AEC_INFO, lMeta);
      lAecParams = mobicat_aec_params;
      is_aec_valid = 1;
  }

  IF_META_AVAILABLE(cam_chromatix_lite_isp_t, ispData,
        CAM_INTF_META_CHROMATIX_LITE_ISP, lMeta) {
    lvfeDiag = (vfe_diagnostics_t *) &ispData->private_isp_data;
    is_chromatix_lite_isp_valid = 1;
  }

  IF_META_AVAILABLE(cam_chromatix_lite_pp_t, ppData,
        CAM_INTF_META_CHROMATIX_LITE_PP, lMeta) {
    lEzPpParams = (ez_pp_params_t *) &ppData->private_pp_data;
    is_chromatix_lite_pp_valid = 1;
  }

  mMobicatStr = (char *)malloc(MAX_MOBICAT_LENGTH * sizeof(char));
  if (mMobicatStr == NULL) {
      QIDBG_ERROR("%s:%d] Error no memory", __func__, __LINE__);
      return 0;
  }
  memset(mMobicatStr, 0x0, MAX_MOBICAT_LENGTH * sizeof(char));

  mScratchBuf = (char *)malloc(MAX_MOBICAT_LENGTH * sizeof(char));
  if (mScratchBuf == NULL) {
      QIDBG_ERROR("%s:%d] Error no memory", __func__, __LINE__);
      free(mMobicatStr);
      return 0;
  }
  memset(mScratchBuf, 0x0, MAX_MOBICAT_LENGTH * sizeof(char));

  /* Parse AWB */
  PARSE_F("awb-rGain=", lAwbData.gain.r_gain);
  PARSE_F("awb-gGain=", lAwbData.gain.g_gain);
  PARSE_F("awb-bGain=", lAwbData.gain.b_gain);
  PARSE_U("awb-colorTemp=", lAwbData.color_temp);
  PARSE_C("awb-boundingBoxLumaYMin=", lAwbData.bounding_box.y_min);
  PARSE_C("awb-boundingBoxLumaYMax=", lAwbData.bounding_box.y_max);
  PARSE_C("awb-slopeRegion1=", lAwbData.bounding_box.m1);
  PARSE_C("awb-slopeRegion1=", lAwbData.bounding_box.m2);
  PARSE_C("awb-slopeRegion1=", lAwbData.bounding_box.m3);
  PARSE_C("awb-slopeRegion1=", lAwbData.bounding_box.m4);
  PARSE_SI("awb-cbOffset1=", lAwbData.bounding_box.c1);
  PARSE_SI("awb-cbOffset1=", lAwbData.bounding_box.c2);
  PARSE_SI("awb-cbOffset1=", lAwbData.bounding_box.c3);
  PARSE_SI("awb-cbOffset1=", lAwbData.bounding_box.c4);
  PARSE_U("awb-extremeColor1=", lAwbData.exterme_color_param.t1);
  PARSE_U("awb-extremeColor2=", lAwbData.exterme_color_param.t2);
  PARSE_U("awb-extremeColor3=", lAwbData.exterme_color_param.t3);
  PARSE_U("awb-extremeColor4=", lAwbData.exterme_color_param.t4);
  PARSE_U("awb-extremeColor5=", lAwbData.exterme_color_param.t5);
  PARSE_U("awb-extremeColorMG=", lAwbData.exterme_color_param.mg);
  PARSE_I("awb-wbMode=", lAwbData.wb_mode);
  PARSE_I("awb-bestMode=", lAwbData.best_mode);
  PARSE_I_A1("awb-sampleDecision=", lAwbData.sample_decision);
  PARSE_I("awb-ccmFlag=", lAwbData.ccm_update.awb_ccm_enable);
  PARSE_F_A2("awb-curCcm=", lAwbData.ccm_update.ccm);

  /* Parse AEC */
  if (is_aec_valid && lAecParams) {
    PARSE_I("aec-iso_value=", lAecParams->iso_value);
    PARSE_U("aec-flash_needed=", lAecParams->flash_needed);
    PARSE_I("aec-settled=", lAecParams->settled);
    PARSE_I("aec-wb_mode=", lAecParams->wb_mode);
    PARSE_U("aec-metering_mode=", lAecParams->metering_mode);
    PARSE_U("aec-exposure_program=", lAecParams->exposure_program);
    PARSE_U("aec-exposure_mode=", lAecParams->exposure_mode);
    PARSE_U("aec-scenetype=", lAecParams->scenetype);
    PARSE_F("aec-brightness=", lAecParams->brightness);
    PARSE_F("Snapshot-ExposureTime=", lAecParams->exp_time);
  }  else {
      QIDBG_ERROR("%s:%d] AEC mobicat data not valid!", __func__, __LINE__);
  }

  /* Parse VFE diagnostics data */
  if (is_chromatix_lite_isp_valid && lvfeDiag) {
    colorcorrection_t *lColCorr = &lvfeDiag->prev_colorcorr;
    chromaenhancement_t *lChromaEnhancement = &lvfeDiag->colorconv;
    rolloff_t *lRolloff = &lvfeDiag->prev_rolloff;
    demuxchannelgain_t *lDemuxChanGain = &lvfeDiag->prev_demuxchannelgain;
    asfsharpness5x5_t *lAsfSharp =  &lvfeDiag->prev_asf5x5;
    lumaadaptation_t *lLumaAdapt = &lvfeDiag->prev_lumaadaptation;
    chromasuppression_t *lChromaSupp = &lvfeDiag->prev_chromasupp;
    memorycolorenhancement_t *lMemColorEnhan = &lvfeDiag->prev_memcolorenhan;
    badcorrection_t *lBpc = &lvfeDiag->prev_bpc;
    badcorrection_t *lBcc = &lvfeDiag->prev_bcc;
    abffilterdata_t *lAbFilter = &lvfeDiag->prev_abfilter;
    demosaic3_t *lDemosaic = &lvfeDiag->prev_demosaic;
    skincolorenhancement_t *lSkinColEnhan = &lvfeDiag->prev_skincolorenhan;
    linearization_t *lLinear = &lvfeDiag->prev_linear;
    chromalumafiltercoeff_t *lChromaLuma = &lvfeDiag->prev_chromalumafilter;

    config_cntrl_t *lControlGicdiag = &lvfeDiag->control_gicdiag;
    gicdiag_t *lPrevGicDiag = &lvfeDiag->prev_gicdiag;
    config_cntrl_t *lControlGtmDiag = &lvfeDiag->control_gtmdiag;
    gtmdiag_t *lPrevGtmDiag = &lvfeDiag->prev_gtmdiag;
    config_cntrl_t *lControlPedestalDiag = &lvfeDiag->control_pedestaldiag;
    pedestaldiag_t *lPrevPedestalDiag = &lvfeDiag->prev_pedestaldiag;

    PARSE_I("vfe-ColorCorr-coefRtoR=", lColCorr->coef_rtor);
    PARSE_I("vfe-ColorCorr-coefGtoR=", lColCorr->coef_gtor);
    PARSE_I("vfe-ColorCorr-coefBtoR=", lColCorr->coef_btor);
    PARSE_I("vfe-ColorCorr-coefRtoG=", lColCorr->coef_rtog);
    PARSE_I("vfe-ColorCorr-coefGtoG=", lColCorr->coef_gtog);
    PARSE_I("vfe-ColorCorr-coefBtoG=", lColCorr->coef_btog);
    PARSE_I("vfe-ColorCorr-coefRtoB=", lColCorr->coef_rtob);
    PARSE_I("vfe-ColorCorr-coefGtoB=", lColCorr->coef_gtob);
    PARSE_I("vfe-ColorCorr-coefBtoB=", lColCorr->coef_btob);
    PARSE_I("vfe-ColorCorr-coefRtoR=", lColCorr->coef_rtor);


    PARSE_I("vfe-colorConv-paramAp=", lChromaEnhancement->param_ap);
    PARSE_I("vfe-colorConv-paramAm=", lChromaEnhancement->param_am);
    PARSE_I("vfe-colorConv-paramBp=", lChromaEnhancement->param_bp);
    PARSE_I("vfe-colorConv-paramBm=", lChromaEnhancement->param_bm);
    PARSE_I("vfe-colorConv-paramCp=", lChromaEnhancement->param_cp);
    PARSE_I("vfe-colorConv-paramCm=", lChromaEnhancement->param_cm);
    PARSE_I("vfe-colorConv-paramDp=", lChromaEnhancement->param_dp);
    PARSE_I("vfe-colorConv-paramDm=", lChromaEnhancement->param_dm);
    PARSE_I("vfe-colorConv-paramKcb=", lChromaEnhancement->param_kcb);
    PARSE_I("vfe-colorConv-paramKcr=", lChromaEnhancement->param_kcr);
    PARSE_I("vfe-colorConv-paramRtoY=", lChromaEnhancement->param_rtoy);
    PARSE_I("vfe-colorConv-paramGtoY=", lChromaEnhancement->param_gtoy);
    PARSE_I("vfe-colorConv-paramBtoY=", lChromaEnhancement->param_btoy);
    PARSE_I("vfe-colorConv-paramYOffset=", lChromaEnhancement->param_yoffset);


    PARSE_F_A1("vfe-Rolloff-coefftableR=", lRolloff->coefftable_R);
    PARSE_F_A1("vfe-Rolloff-coefftableGr=", lRolloff->coefftable_Gr);
    PARSE_F_A1("vfe-Rolloff-coefftableGb=", lRolloff->coefftable_Gb);
    PARSE_F_A1("vfe-Rolloff-coefftableB=", lRolloff->coefftable_B);

    PARSE_U("vfe-DemuxChanGain-greenEvenRow=",
        lDemuxChanGain->greenEvenRow);
    PARSE_U("vfe-DemuxChanGain-oddEvenRow=",
        lDemuxChanGain->greenOddRow);
    PARSE_U("vfe-DemuxChanGain-Blue=",
        lDemuxChanGain->blue);
    PARSE_U("vfe-DemuxChanGain-Red=",
        lDemuxChanGain->red);


    PARSE_U("vfe-Asf5x5-smoothfilterEnabled=", lAsfSharp->smoothfilterEnabled);
    PARSE_U("vfe-Asf5x5-sharpMode=", lAsfSharp->sharpMode);
    PARSE_U("vfe-Asf5x5-lpfMode=", lAsfSharp->lpfMode);
    PARSE_U("vfe-Asf5x5-smoothcoefCenter=", lAsfSharp->smoothcoefCenter);
    PARSE_U("vfe-Asf5x5-smoothcoefSurr=", lAsfSharp->smoothcoefSurr);
    PARSE_U("vfe-Asf5x5-pipeflushCount=", lAsfSharp->pipeflushCount);
    PARSE_U("vfe-Asf5x5-pipeflushOvd=", lAsfSharp->sharpMode);
    PARSE_U("vfe-Asf5x5-flushhaltOvd=", lAsfSharp->flushhaltOvd);
    PARSE_U("vfe-Asf5x5-cropEnable=", lAsfSharp->cropEnable);
    PARSE_U("vfe-Asf5x5-normalizeFactor=", lAsfSharp->normalizeFactor);
    PARSE_U("vfe-Asf5x5-sharpthreshE1=", lAsfSharp->sharpthreshE1);
    PARSE_U("vfe-Asf5x5-sharpthreshE2=", lAsfSharp->sharpthreshE2);
    PARSE_U("vfe-Asf5x5-sharpthreshE3=", lAsfSharp->sharpthreshE3);
    PARSE_U("vfe-Asf5x5-sharpthreshE4=", lAsfSharp->sharpthreshE4);
    PARSE_U("vfe-Asf5x5-sharpthreshE5=", lAsfSharp->sharpthreshE5);
    PARSE_U("vfe-Asf5x5-sharpK1=", lAsfSharp->sharpK1);
    PARSE_U("vfe-Asf5x5-sharpK1=", lAsfSharp->sharpK1);


    PARSE_I("vfe-Asf5x5-F1Coef0=", lAsfSharp->f1coef0);
    PARSE_I("vfe-Asf5x5-F1Coef1=", lAsfSharp->f1coef1);
    PARSE_I("vfe-Asf5x5-F1Coef2=", lAsfSharp->f1coef2);
    PARSE_I("vfe-Asf5x5-F1Coef3=", lAsfSharp->f1coef3);
    PARSE_I("vfe-Asf5x5-F1Coef4=", lAsfSharp->f1coef4);
    PARSE_I("vfe-Asf5x5-F1Coef5=", lAsfSharp->f1coef5);
    PARSE_I("vfe-Asf5x5-F1Coef6=", lAsfSharp->f1coef6);
    PARSE_I("vfe-Asf5x5-F1Coef7=", lAsfSharp->f1coef7);
    PARSE_I("vfe-Asf5x5-F2Coef0=", lAsfSharp->f2coef0);
    PARSE_I("vfe-Asf5x5-F2Coef1=", lAsfSharp->f2coef1);
    PARSE_I("vfe-Asf5x5-F2Coef2=", lAsfSharp->f2coef2);
    PARSE_I("vfe-Asf5x5-F2Coef3=", lAsfSharp->f2coef3);
    PARSE_I("vfe-Asf5x5-F2Coef4=", lAsfSharp->f2coef4);
    PARSE_I("vfe-Asf5x5-F2Coef5=", lAsfSharp->f2coef5);
    PARSE_I("vfe-Asf5x5-F2Coef6=", lAsfSharp->f2coef6);
    PARSE_I("vfe-Asf5x5-F2Coef7=", lAsfSharp->f2coef7);
    PARSE_I("vfe-Asf5x5-F3Coef0=", lAsfSharp->f3coef0);
    PARSE_I("vfe-Asf5x5-F3Coef1=", lAsfSharp->f3coef1);
    PARSE_I("vfe-Asf5x5-F3Coef2=", lAsfSharp->f3coef2);
    PARSE_I("vfe-Asf5x5-F3Coef3=", lAsfSharp->f3coef3);
    PARSE_I("vfe-Asf5x5-F3Coef4=", lAsfSharp->f3coef4);
    PARSE_I("vfe-Asf5x5-F3Coef5=", lAsfSharp->f3coef5);
    PARSE_I("vfe-Asf5x5-F3Coef6=", lAsfSharp->f3coef6);
    PARSE_I("vfe-Asf5x5-F3Coef7=", lAsfSharp->f3coef7);

    PARSE_I_A1("vfe-LumaAdaptation-lutYratio=", lLumaAdapt->lut_yratio);


    PARSE_U("vfe-ChromaSupp-ysup1=", lChromaSupp->ysup1);
    PARSE_U("vfe-ChromaSupp-ysup2=", lChromaSupp->ysup2);
    PARSE_U("vfe-ChromaSupp-ysup3=", lChromaSupp->ysup3);
    PARSE_U("vfe-ChromaSupp-ysupM1=", lChromaSupp->ysupM1);
    PARSE_U("vfe-ChromaSupp-ysupM3=", lChromaSupp->ysupM3);
    PARSE_U("vfe-ChromaSupp-ysupS1=", lChromaSupp->ysupS1);
    PARSE_U("vfe-ChromaSupp-ysupS3=", lChromaSupp->ysupS3);
    PARSE_U("vfe-ChromaSupp-csup1=", lChromaSupp->csup1);
    PARSE_U("vfe-ChromaSupp-csup2=", lChromaSupp->csup2);
    PARSE_U("vfe-ChromaSupp-csupM1=", lChromaSupp->csupM1);
    PARSE_U("vfe-ChromaSupp-csupS1=", lChromaSupp->csupS1);


    PARSE_U("vfe-MemColorEnhan-qk=", lMemColorEnhan->qk);
    PARSE_U("vfe-MemColorEnhan-red-y1=", lMemColorEnhan->red.y1);
    PARSE_U("vfe-MemColorEnhan-red-y2=", lMemColorEnhan->red.y2);
    PARSE_U("vfe-MemColorEnhan-red-y3=", lMemColorEnhan->red.y3);
    PARSE_U("vfe-MemColorEnhan-red-y4=", lMemColorEnhan->red.y4);
    PARSE_U("vfe-MemColorEnhan-red-yM1=", lMemColorEnhan->red.yM1);
    PARSE_U("vfe-MemColorEnhan-red-yM3=", lMemColorEnhan->red.yM3);
    PARSE_U("vfe-MemColorEnhan-red-yS1=", lMemColorEnhan->red.yS1);
    PARSE_U("vfe-MemColorEnhan-red-yS3=", lMemColorEnhan->red.yS3);
    PARSE_U("vfe-MemColorEnhan-red-transWidth=", lMemColorEnhan->red.transWidth);
    PARSE_U("vfe-MemColorEnhan-red-transTrunc=", lMemColorEnhan->red.transTrunc);
    PARSE_I("vfe-MemColorEnhan-red-crZone=", lMemColorEnhan->red.crZone);
    PARSE_I("vfe-MemColorEnhan-red-cbZone=", lMemColorEnhan->red.cbZone);
    PARSE_I("vfe-MemColorEnhan-red-translope=", lMemColorEnhan->red.translope);
    PARSE_I("vfe-MemColorEnhan-red-k=", lMemColorEnhan->red.k);

    PARSE_U("vfe-Bpc-fminThreshold=", lBpc->fminThreshold);
    PARSE_U("vfe-Bpc-fmaxThreshold=", lBpc->fmaxThreshold);
    PARSE_U("vfe-Bpc-rOffsetLo=", lBpc->rOffsetLo);
    PARSE_U("vfe-Bpc-rOffsetHi=", lBpc->rOffsetHi);
    PARSE_U("vfe-Bpc-grOffsetLo=", lBpc->grOffsetLo);
    PARSE_U("vfe-Bpc-gbOffsetLo=", lBpc->gbOffsetLo);
    PARSE_U("vfe-Bpc-gbOffsetHi=", lBpc->gbOffsetHi);
    PARSE_U("vfe-Bpc-grOffsetHi=", lBpc->grOffsetHi);
    PARSE_U("vfe-Bpc-bOffsetLo=", lBpc->bOffsetLo);
    PARSE_U("vfe-Bpc-bOffsetHi=", lBpc->bOffsetHi);

    PARSE_U("vfe-Bcc-fminThreshold=", lBcc->fminThreshold);
    PARSE_U("vfe-Bcc-fmaxThreshold=", lBcc->fmaxThreshold);
    PARSE_U("vfe-Bcc-rOffsetLo=", lBcc->rOffsetLo);
    PARSE_U("vfe-Bcc-rOffsetHi=", lBcc->rOffsetHi);
    PARSE_U("vfe-Bcc-grOffsetLo=", lBcc->grOffsetLo);
    PARSE_U("vfe-Bcc-gbOffsetLo=", lBcc->gbOffsetLo);
    PARSE_U("vfe-Bcc-gbOffsetHi=", lBcc->gbOffsetHi);
    PARSE_U("vfe-Bcc-grOffsetHi=", lBcc->grOffsetHi);
    PARSE_U("vfe-Bcc-bOffsetLo=", lBcc->bOffsetLo);
    PARSE_U("vfe-Bcc-bOffsetHi=", lBcc->bOffsetHi);


    PARSE_U16_A1("vfe-AbFilter-red-Threshold=", lAbFilter->red.threshold);
    PARSE_U16_A1("vfe-AbFilter-red-Pos=", lAbFilter->red.pos);
    PARSE_SI_A1("vfe-AbFilter-red-Neg=", lAbFilter->red.neg);
    PARSE_U16_A1("vfe-AbFilter-green-Threshold=", lAbFilter->green.threshold);
    PARSE_U16_A1("vfe-AbFilter-green-Pos=", lAbFilter->green.pos);
    PARSE_SI_A1("vfe-AbFilter-green-Neg=", lAbFilter->green.neg);
    PARSE_U16_A1("vfe-AbFilter-blue-Threshold=", lAbFilter->blue.threshold);
    PARSE_U16_A1("vfe-AbFilter-blue-Pos=", lAbFilter->blue.pos);
    PARSE_SI_A1("vfe-AbFilter-blue-Neg=", lAbFilter->blue.neg);


    for (uint32_t i = 0; i < ARR_SZ(lDemosaic->lut); i++) {
      char tag_str[MAX_MOBICAT_LENGTH];
      snprintf(&tag_str[0], MAX_MOBICAT_LENGTH, "vfe-Demosaic-lut[%d]-wk=", i);
      PARSE_U(tag_str, lDemosaic->lut[i].wk);
      snprintf(&tag_str[0], MAX_MOBICAT_LENGTH, "vfe-Demosaic-lut[%d]-bk=", i);
      PARSE_U(tag_str, lDemosaic->lut[i].bk);
      snprintf(&tag_str[0], MAX_MOBICAT_LENGTH, "vfe-Demosaic-lut[%d]-lk=", i);
      PARSE_U(tag_str, lDemosaic->lut[i].lk);
      snprintf(&tag_str[0], MAX_MOBICAT_LENGTH, "vfe-Demosaic-lut[%d]-tk=", i);
      PARSE_U(tag_str, lDemosaic->lut[i].tk);
    }

    PARSE_U("vfe-Demosaic-aG=", lDemosaic->aG);
    PARSE_U("vfe-Demosaic-bL=", lDemosaic->bL);

    PARSE_I("vfe-SkinColorEnhan-CrCoord-vertex00=",  lSkinColEnhan->crcoord.vertex00);
    PARSE_I("vfe-SkinColorEnhan-CrCoord-vertex01=",  lSkinColEnhan->crcoord.vertex01);
    PARSE_I("vfe-SkinColorEnhan-CrCoord-vertex02=",  lSkinColEnhan->crcoord.vertex02);
    PARSE_I("vfe-SkinColorEnhan-CrCoord-vertex10=",  lSkinColEnhan->crcoord.vertex10);
    PARSE_I("vfe-SkinColorEnhan-CrCoord-vertex11=",  lSkinColEnhan->crcoord.vertex11);
    PARSE_I("vfe-SkinColorEnhan-CrCoord-vertex12=",  lSkinColEnhan->crcoord.vertex12);
    PARSE_I("vfe-SkinColorEnhan-CrCoord-vertex20=",  lSkinColEnhan->crcoord.vertex20);
    PARSE_I("vfe-SkinColorEnhan-CrCoord-vertex21=",  lSkinColEnhan->crcoord.vertex21);
    PARSE_I("vfe-SkinColorEnhan-CrCoord-vertex22=",  lSkinColEnhan->crcoord.vertex22);
    PARSE_I("vfe-SkinColorEnhan-CrCoord-vertex30=",  lSkinColEnhan->crcoord.vertex30);
    PARSE_I("vfe-SkinColorEnhan-CrCoord-vertex31=",  lSkinColEnhan->crcoord.vertex31);
    PARSE_I("vfe-SkinColorEnhan-CrCoord-vertex32=",  lSkinColEnhan->crcoord.vertex32);
    PARSE_I("vfe-SkinColorEnhan-CrCoord-vertex40=",  lSkinColEnhan->crcoord.vertex40);
    PARSE_I("vfe-SkinColorEnhan-CrCoord-vertex41=",  lSkinColEnhan->crcoord.vertex41);
    PARSE_I("vfe-SkinColorEnhan-CrCoord-vertex42=",  lSkinColEnhan->crcoord.vertex42);

    PARSE_I("vfe-SkinColorEnhan-CbCoord-vertex00=",  lSkinColEnhan->cbcoord.vertex00);
    PARSE_I("vfe-SkinColorEnhan-CbCoord-vertex01=",  lSkinColEnhan->cbcoord.vertex01);
    PARSE_I("vfe-SkinColorEnhan-CbCoord-vertex02=",  lSkinColEnhan->cbcoord.vertex02);
    PARSE_I("vfe-SkinColorEnhan-CbCoord-vertex10=",  lSkinColEnhan->cbcoord.vertex10);
    PARSE_I("vfe-SkinColorEnhan-CbCoord-vertex11=",  lSkinColEnhan->cbcoord.vertex11);
    PARSE_I("vfe-SkinColorEnhan-CbCoord-vertex12=",  lSkinColEnhan->cbcoord.vertex12);
    PARSE_I("vfe-SkinColorEnhan-CbCoord-vertex20=",  lSkinColEnhan->cbcoord.vertex20);
    PARSE_I("vfe-SkinColorEnhan-CbCoord-vertex21=",  lSkinColEnhan->cbcoord.vertex21);
    PARSE_I("vfe-SkinColorEnhan-CbCoord-vertex22=",  lSkinColEnhan->cbcoord.vertex22);
    PARSE_I("vfe-SkinColorEnhan-CbCoord-vertex30=",  lSkinColEnhan->cbcoord.vertex30);
    PARSE_I("vfe-SkinColorEnhan-CbCoord-vertex31=",  lSkinColEnhan->cbcoord.vertex31);
    PARSE_I("vfe-SkinColorEnhan-CbCoord-vertex32=",  lSkinColEnhan->cbcoord.vertex32);
    PARSE_I("vfe-SkinColorEnhan-CbCoord-vertex40=",  lSkinColEnhan->cbcoord.vertex40);
    PARSE_I("vfe-SkinColorEnhan-CbCoord-vertex41=",  lSkinColEnhan->cbcoord.vertex41);
    PARSE_I("vfe-SkinColorEnhan-CbCoord-vertex42=",  lSkinColEnhan->cbcoord.vertex42);


    PARSE_I("vfe-SkinColorEnhan-CrCoeff-coef00=", lSkinColEnhan->crcoeff.coef00);
    PARSE_I("vfe-SkinColorEnhan-CrCoeff-coef01=", lSkinColEnhan->crcoeff.coef01);
    PARSE_I("vfe-SkinColorEnhan-CrCoeff-coef10=", lSkinColEnhan->crcoeff.coef10);
    PARSE_I("vfe-SkinColorEnhan-CrCoeff-coef11=", lSkinColEnhan->crcoeff.coef11);
    PARSE_I("vfe-SkinColorEnhan-CrCoeff-coef20=", lSkinColEnhan->crcoeff.coef20);
    PARSE_I("vfe-SkinColorEnhan-CrCoeff-coef21=", lSkinColEnhan->crcoeff.coef21);
    PARSE_I("vfe-SkinColorEnhan-CrCoeff-coef30=", lSkinColEnhan->crcoeff.coef30);
    PARSE_I("vfe-SkinColorEnhan-CrCoeff-coef31=", lSkinColEnhan->crcoeff.coef31);
    PARSE_I("vfe-SkinColorEnhan-CrCoeff-coef40=", lSkinColEnhan->crcoeff.coef40);
    PARSE_I("vfe-SkinColorEnhan-CrCoeff-coef41=", lSkinColEnhan->crcoeff.coef41);
    PARSE_I("vfe-SkinColorEnhan-CrCoeff-coef50=", lSkinColEnhan->crcoeff.coef50);
    PARSE_I("vfe-SkinColorEnhan-CrCoeff-coef51=", lSkinColEnhan->crcoeff.coef51);

    PARSE_I("vfe-SkinColorEnhan-CbCoeff-coef00=", lSkinColEnhan->cbcoeff.coef00);
    PARSE_I("vfe-SkinColorEnhan-CbCoeff-coef01=", lSkinColEnhan->cbcoeff.coef01);
    PARSE_I("vfe-SkinColorEnhan-CbCoeff-coef10=", lSkinColEnhan->cbcoeff.coef10);
    PARSE_I("vfe-SkinColorEnhan-CbCoeff-coef11=", lSkinColEnhan->cbcoeff.coef11);
    PARSE_I("vfe-SkinColorEnhan-CbCoeff-coef20=", lSkinColEnhan->cbcoeff.coef20);
    PARSE_I("vfe-SkinColorEnhan-CbCoeff-coef21=", lSkinColEnhan->cbcoeff.coef21);
    PARSE_I("vfe-SkinColorEnhan-CbCoeff-coef30=", lSkinColEnhan->cbcoeff.coef30);
    PARSE_I("vfe-SkinColorEnhan-CbCoeff-coef31=", lSkinColEnhan->cbcoeff.coef31);
    PARSE_I("vfe-SkinColorEnhan-CbCoeff-coef40=", lSkinColEnhan->cbcoeff.coef40);
    PARSE_I("vfe-SkinColorEnhan-CbCoeff-coef41=", lSkinColEnhan->cbcoeff.coef41);
    PARSE_I("vfe-SkinColorEnhan-CbCoeff-coef50=", lSkinColEnhan->cbcoeff.coef50);
    PARSE_I("vfe-SkinColorEnhan-CbCoeff-coef51=", lSkinColEnhan->cbcoeff.coef51);

    PARSE_I("vfe-SkinColorEnhan-CrOffset-offset0=", lSkinColEnhan->croffset.offset0);
    PARSE_I("vfe-SkinColorEnhan-CrOffset-offset1=", lSkinColEnhan->croffset.offset1);
    PARSE_I("vfe-SkinColorEnhan-CrOffset-offset2=", lSkinColEnhan->croffset.offset2);
    PARSE_I("vfe-SkinColorEnhan-CrOffset-offset3=", lSkinColEnhan->croffset.offset3);
    PARSE_I("vfe-SkinColorEnhan-CrOffset-offset4=", lSkinColEnhan->croffset.offset4);
    PARSE_I("vfe-SkinColorEnhan-CrOffset-offset5=", lSkinColEnhan->croffset.offset5);
    PARSE_U("vfe-SkinColorEnhan-CrOffset-shift0=", lSkinColEnhan->croffset.shift0);
    PARSE_U("vfe-SkinColorEnhan-CrOffset-shift1=", lSkinColEnhan->croffset.shift1);
    PARSE_U("vfe-SkinColorEnhan-CrOffset-shift2=", lSkinColEnhan->croffset.shift2);
    PARSE_U("vfe-SkinColorEnhan-CrOffset-shift3=", lSkinColEnhan->croffset.shift3);
    PARSE_U("vfe-SkinColorEnhan-CrOffset-shift4=", lSkinColEnhan->croffset.shift4);
    PARSE_U("vfe-SkinColorEnhan-CrOffset-shift5=", lSkinColEnhan->croffset.shift5);

    PARSE_I("vfe-SkinColorEnhan-CrOffset-offset0=", lSkinColEnhan->croffset.offset0);
    PARSE_I("vfe-SkinColorEnhan-CrOffset-offset1=", lSkinColEnhan->croffset.offset1);
    PARSE_I("vfe-SkinColorEnhan-CrOffset-offset2=", lSkinColEnhan->croffset.offset2);
    PARSE_I("vfe-SkinColorEnhan-CrOffset-offset3=", lSkinColEnhan->croffset.offset3);
    PARSE_I("vfe-SkinColorEnhan-CrOffset-offset4=", lSkinColEnhan->croffset.offset4);
    PARSE_I("vfe-SkinColorEnhan-CrOffset-offset5=", lSkinColEnhan->croffset.offset5);
    PARSE_U("vfe-SkinColorEnhan-CbOffset-shift0=", lSkinColEnhan->cboffset.shift0);
    PARSE_U("vfe-SkinColorEnhan-CbOffset-shift1=", lSkinColEnhan->cboffset.shift1);
    PARSE_U("vfe-SkinColorEnhan-CbOffset-shift2=", lSkinColEnhan->cboffset.shift2);
    PARSE_U("vfe-SkinColorEnhan-CbOffset-shift3=", lSkinColEnhan->cboffset.shift3);
    PARSE_U("vfe-SkinColorEnhan-CbOffset-shift4=", lSkinColEnhan->cboffset.shift4);
    PARSE_U("vfe-SkinColorEnhan-CbOffset-shift5=", lSkinColEnhan->cboffset.shift5);


    PARSE_U16_A1("vfe-Linear-rlut_pl=", lLinear->rlut_pl);
    PARSE_U16_A1("vfe-Linear-grlut_pl=", lLinear->grlut_pl);
    PARSE_U16_A1("vfe-Linear-gblut_pl=", lLinear->gblut_pl);
    PARSE_U16_A1("vfe-Linear-blut_pl=",  lLinear->blut_pl);
    PARSE_U16_A1("vfe-Linear-rlut_base=", lLinear->rlut_base);
    PARSE_U16_A1("vfe-Linear-grlut_base=", lLinear->grlut_base);
    PARSE_U16_A1("vfe-Linear-gblut_base=", lLinear->gblut_base);
    PARSE_U16_A1("vfe-Linear-blut_base=",  lLinear->blut_base);
    PARSE_U_A1("vfe-Linear-rlut_delta=", lLinear->rlut_delta);
    PARSE_U_A1("vfe-Linear-grlut_delta=", lLinear->grlut_delta);
    PARSE_U_A1("vfe-Linear-gblut_delta=", lLinear->gblut_delta);
    PARSE_U_A1("vfe-Linear-blut_delta=",  lLinear->blut_delta);

    PARSE_F_A1("vfe-ChromaLumaFilter-Chroma-hcoeff=", lChromaLuma->chromafilter.hcoeff);
    PARSE_F_A1("vfe-ChromaLumaFilter-Chroma-vcoeff=", lChromaLuma->chromafilter.vcoeff);

    PARSE_U16_A1("vfe-ChromaLumaFilter-Luma-thresholdRed=", lChromaLuma->lumafilter.threshold_red);
    PARSE_F_A1("vfe-ChromaLumaFilter-Luma-scalefactorRed=", lChromaLuma->lumafilter.scalefactor_red);
    PARSE_U16_A1("vfe-ChromaLumaFilter-Luma-thresholdGreen=", lChromaLuma->lumafilter.threshold_green);
    PARSE_F_A1("vfe-ChromaLumaFilter-Luma-scalefactorGreenr=", lChromaLuma->lumafilter.scalefactor_green);
    PARSE_U16_A1("vfe-ChromaLumaFilter-Luma-thresholdBlue=", lChromaLuma->lumafilter.threshold_blue);
    PARSE_F_A1("vfe-ChromaLumaFilter-Luma-scalefactorBlue=", lChromaLuma->lumafilter.scalefactor_blue);
    PARSE_F_A1("vfe-ChromaLumaFilter-Luma-tablepos=", lChromaLuma->lumafilter.tablepos);
    PARSE_F_A1("vfe-ChromaLumaFilter-Luma-tableneg=", lChromaLuma->lumafilter.tableneg);

    /* VFE GIC Diag params */
    PARSE_I("vfe-GicDiagControl-enable=", lControlGicdiag->enable);
    PARSE_I("vfe-GicDiagControl-cntrlenable=", lControlGicdiag->cntrlenable);

    PARSE_U_A1("vfe-GicDiag-NoiseStd2LUTLevel0=", lPrevGicDiag->NoiseStd2LUTLevel0);
    PARSE_U_A1("vfe-GicDiag-Signal2LUTLevel0=", lPrevGicDiag->Signal2LUTLevel0);
    PARSE_U("vfe-GicDiag-SoftThreshNoiseScale=", lPrevGicDiag->SoftThreshNoiseScale);
    PARSE_U("vfe-GicDiag-SoftThreshNoiseShift=", lPrevGicDiag->SoftThreshNoiseShift);
    PARSE_U("vfe-GicDiag-FilterStrength=", lPrevGicDiag->FilterStrength);
    PARSE_U("vfe-GicDiag-NoiseScale0=", lPrevGicDiag->NoiseScale0);
    PARSE_U("vfe-GicDiag-NoiseScale1=", lPrevGicDiag->NoiseScale1);
    PARSE_U("vfe-GicDiag-NoiseOffset=", lPrevGicDiag->NoiseOffset);

    /* VFE GTM Diag params */
    PARSE_I("vfe-GtmDiagControl-enable=", lControlGtmDiag->enable);
    PARSE_I("vfe-GtmDiagControl-cntrlenable=", lControlGtmDiag->cntrlenable);

    PARSE_U_A1("vfe-GtmDiag-Xarr=", lPrevGtmDiag->Xarr);
    PARSE_F_A1("vfe-GtmDiag-YRatioBase=", lPrevGtmDiag->YRatioBase);
    PARSE_I_A1("vfe-GtmDiag-YRatioSlope=", lPrevGtmDiag->YRatioSlope);
    PARSE_F("vfe-GtmDiag-AMiddleTone=", lPrevGtmDiag->AMiddleTone);
    PARSE_I("vfe-GtmDiag-MaxValThresh=", lPrevGtmDiag->MaxValThresh);
    PARSE_I("vfe-GtmDiag-KeyMinThresh=", lPrevGtmDiag->KeyMinThresh);
    PARSE_I("vfe-GtmDiag-KeyMaxThresh=", lPrevGtmDiag->KeyMaxThresh);
    PARSE_F("vfe-GtmDiag-KeyHistBinWeight=", lPrevGtmDiag->KeyHistBinWeight);
    PARSE_I("vfe-GtmDiag-YoutMaxVal=", lPrevGtmDiag->YoutMaxVal);
    PARSE_F("vfe-GtmDiag-TemporalW=", lPrevGtmDiag->TemporalW);
    PARSE_F("vfe-GtmDiag-MiddleToneW=", lPrevGtmDiag->MiddleToneW);
    PARSE_U("vfe-GtmDiag-LUTBankSel=", lPrevGtmDiag->LUTBankSel);

    /* VFE Pedestal Diag params */
    PARSE_I("vfe-PedestalDiagControl-enable=", lControlPedestalDiag->enable);
    PARSE_I("vfe-PedestalDiagControl-cntrlenable=", lControlPedestalDiag->cntrlenable);

    PARSE_U16_A1("vfe-PedestalDiag-MeshTableT1_R=", lPrevPedestalDiag->MeshTableT1_R);
    PARSE_U16_A1("vfe-PedestalDiag-MeshTableT1_Gr=", lPrevPedestalDiag->MeshTableT1_Gr);
    PARSE_U16_A1("vfe-PedestalDiag-MeshTableT1_Gb=", lPrevPedestalDiag->MeshTableT1_Gb);
    PARSE_U16_A1("vfe-PedestalDiag-MeshTableT1_B=", lPrevPedestalDiag->MeshTableT1_B);
    PARSE_U16_A1("vfe-PedestalDiag-MeshTableT2_R=", lPrevPedestalDiag->MeshTableT2_R);
    PARSE_U16_A1("vfe-PedestalDiag-MeshTableT2_Gr=", lPrevPedestalDiag->MeshTableT2_Gr);
    PARSE_U16_A1("vfe-PedestalDiag-MeshTableT2_Gb=", lPrevPedestalDiag->MeshTableT2_Gb);
    PARSE_U16_A1("vfe-PedestalDiag-MeshTableT2_B=", lPrevPedestalDiag->MeshTableT2_B);
    PARSE_U("vfe-PedestalDiag-HDREnable=", lPrevPedestalDiag->HDREnable);
    PARSE_U("vfe-PedestalDiag-ScaleBypass=", lPrevPedestalDiag->ScaleBypass);
    PARSE_U("vfe-PedestalDiag-IntpFactor=", lPrevPedestalDiag->IntpFactor);
    PARSE_U("vfe-PedestalDiag-BWidth=", lPrevPedestalDiag->BWidth);
    PARSE_U("vfe-PedestalDiag-BHeight=", lPrevPedestalDiag->BHeight);
    PARSE_U("vfe-PedestalDiag-XDelta=", lPrevPedestalDiag->XDelta);
    PARSE_U("vfe-PedestalDiag-YDelta=", lPrevPedestalDiag->YDelta);
    PARSE_U("vfe-PedestalDiag-LeftImageWD=", lPrevPedestalDiag->LeftImageWD);
    PARSE_U("vfe-PedestalDiag-Enable3D=", lPrevPedestalDiag->Enable3D);
    PARSE_U("vfe-PedestalDiag-MeshGridBWidth=", lPrevPedestalDiag->MeshGridBWidth);
    PARSE_U("vfe-PedestalDiag-MeshGridBHeight=", lPrevPedestalDiag->MeshGridBHeight);
    PARSE_U("vfe-PedestalDiag-LXStart=", lPrevPedestalDiag->LXStart);
    PARSE_U("vfe-PedestalDiag-LYStart=", lPrevPedestalDiag->LYStart);
    PARSE_U("vfe-PedestalDiag-BXStart=", lPrevPedestalDiag->BXStart);
    PARSE_U("vfe-PedestalDiag-BYStart=", lPrevPedestalDiag->BYStart);
    PARSE_U("vfe-PedestalDiag-BXD1=", lPrevPedestalDiag->BXD1);
    PARSE_U("vfe-PedestalDiag-BYE1=", lPrevPedestalDiag->BYE1);
    PARSE_U("vfe-PedestalDiag-BYInitE1=", lPrevPedestalDiag->BYInitE1);
  } else {
    QIDBG_ERROR("%s:%d] Vfe data not valid!", __func__, __LINE__);
  }

  /* Parse PP diagnostics data */
  if (is_chromatix_lite_pp_valid && lEzPpParams) {
    asfsharpness7x7_t *lAsfSharp7x7 = &lEzPpParams->snap_asf7x7;
    asfsharpness9x9_t *lAsfSharp9x9= &lEzPpParams->snap_asf9x9;
    wavelet_t *lWnr = &lEzPpParams->snap_wnr;

    QIDBG_MED("%s:%d] 7x7 valid %d, 9x9 valid %d", __func__, __LINE__,
      lAsfSharp7x7->valid, lAsfSharp9x9->valid);

    if (lAsfSharp7x7->valid) {
      PARSE_I("pp-Asf7x7-smoothPercent=", lAsfSharp7x7->smoothpercent);
      PARSE_UC("pp-Asf7x7-negAbsY1=", lAsfSharp7x7->neg_abs_y1);
      PARSE_UC("pp-Asf7x7-dynaClampEn=", lAsfSharp7x7->dyna_clamp_en);
      PARSE_UC("pp-Asf7x7-spEffEn=", lAsfSharp7x7->sp_eff_en);
      PARSE_I16("pp-Asf7x7-clampHh=", lAsfSharp7x7->clamp_hh);
      PARSE_I16("pp-Asf7x7-clampHl=", lAsfSharp7x7->clamp_hl);
      PARSE_I16("pp-Asf7x7-clampVh=", lAsfSharp7x7->clamp_vh);
      PARSE_I16("pp-Asf7x7-clampVl=", lAsfSharp7x7->clamp_vl);
      PARSE_I("pp-Asf7x7-clampScaleMax=", lAsfSharp7x7->clamp_scale_max);
      PARSE_I("pp-Asf7x7-clampScaleMin=", lAsfSharp7x7->clamp_scale_min);
      PARSE_U16("pp-Asf7x7-clampOffsetMax=", lAsfSharp7x7->clamp_offset_max);
      PARSE_U16("pp-Asf7x7-clampOffsetMin=", lAsfSharp7x7->clamp_offset_min);
      PARSE_U("pp-Asf7x7-NzFlag=", lAsfSharp7x7->nz_flag);
      PARSE_I_A1("pp-Asf7x7-SobelHCoeff=", lAsfSharp7x7->sobel_h_coeff);
      PARSE_I_A1("pp-Asf7x7-SobelVCoeff=", lAsfSharp7x7->sobel_v_coeff);
      PARSE_I_A1("pp-Asf7x7-HpfHCoeff=", lAsfSharp7x7->hpf_h_coeff);
      PARSE_I_A1("pp-Asf7x7-HpfVCoeff=", lAsfSharp7x7->hpf_v_coeff);
      PARSE_I_A1("pp-Asf7x7-LpfVCoeff=", lAsfSharp7x7->lpf_coeff);
      PARSE_I_A1("pp-Asf7x7-lut1=", lAsfSharp7x7->lut1);
      PARSE_I_A1("pp-Asf7x7-lut2=", lAsfSharp7x7->lut2);
      PARSE_I_A1("pp-Asf7x7-lut3=", lAsfSharp7x7->lut3);
    } else if (lAsfSharp9x9->valid) {
      PARSE_I("pp-Asf9x9-sp=", lAsfSharp9x9->sp);
      PARSE_I_A1("pp-Asf9x9-horizontal_nz=", lAsfSharp9x9->horizontal_nz);
      PARSE_I_A1("pp-Asf9x9-veritical_nz=", lAsfSharp9x9->vertical_nz);
      PARSE_SI_A1("pp-Asf9x9-sobel_H_coeff=", lAsfSharp9x9->sobel_H_coeff);
      PARSE_SI_A1("pp-Asf9x9-sobel_se_diagonal_coeff=", lAsfSharp9x9->sobel_se_diagonal_coeff);
      PARSE_SI_A1("pp-Asf9x9-hpf_h_coeff=", lAsfSharp9x9->hpf_h_coeff);
      PARSE_SI_A1("pp-Asf9x9-hpf_se_diagonal_coeff=", lAsfSharp9x9->hpf_se_diagonal_coeff);
      PARSE_SI_A1("pp-Asf9x9-hpf_symmetric_coeff=", lAsfSharp9x9->hpf_symmetric_coeff);
      PARSE_SI_A1("pp-Asf9x9-lpf_coeff=", lAsfSharp9x9->lpf_coeff);
      PARSE_SI_A1("pp-Asf9x9-activity_lpf_coeff=", lAsfSharp9x9->activity_lpf_coeff);
      PARSE_SI_A1("pp-Asf9x9-activity_band_pass_coeff=", lAsfSharp9x9->activity_band_pass_coeff);
      PARSE_F_A1("pp-Asf9x9-activity_normalization_lut=", lAsfSharp9x9->activity_normalization_lut);
      PARSE_F_A1("pp-Asf9x9-weight_modulation_lut=", lAsfSharp9x9->weight_modulation_lut);
      PARSE_U_A1("pp-Asf9x9-soft_threshold_lut=", lAsfSharp9x9->soft_threshold_lut);
      PARSE_F_A1("pp-Asf9x9-gain_lut=", lAsfSharp9x9->gain_lut);
      PARSE_F_A1("pp-Asf9x9-gain_weight_lut=", lAsfSharp9x9->gain_weight_lut);
      PARSE_F("pp-Asf9x9-gain_cap=", lAsfSharp9x9->gain_cap);
      PARSE_U("pp-Asf9x9-gamma_corrected_luma_target=", lAsfSharp9x9->gamma_corrected_luma_target);
      PARSE_I("pp-Asf9x9-en_dyna_clamp=", lAsfSharp9x9->en_dyna_clamp);
      PARSE_F("pp-Asf9x9-smax=", lAsfSharp9x9->smax);
      PARSE_I("pp-Asf9x9-omax=", lAsfSharp9x9->omax);
      PARSE_F("pp-Asf9x9-smin=", lAsfSharp9x9->smin);
      PARSE_I("pp-Asf9x9-omin=", lAsfSharp9x9->omin);
      PARSE_I("pp-Asf9x9-clamp_UL=", lAsfSharp9x9->clamp_UL);
      PARSE_I("pp-Asf9x9-clamp_LL=", lAsfSharp9x9->clamp_LL);
      PARSE_F("pp-Asf9x9-perpendicular_scale_factor=", lAsfSharp9x9->perpendicular_scale_factor);
      PARSE_U("pp-Asf9x9-max_value_threshold=", lAsfSharp9x9->max_value_threshold);
      PARSE_F("pp-Asf9x9-norm_scale=", lAsfSharp9x9->norm_scale);
      PARSE_U("pp-Asf9x9-activity_clamp_threshold=", lAsfSharp9x9->activity_clamp_threshold);
      PARSE_UC("pp-Asf9x9-L2_norm_en=", lAsfSharp9x9->L2_norm_en);
      PARSE_F("pp-Asf9x9-median_blend_upper_offset=", lAsfSharp9x9->median_blend_upper_offset);
      PARSE_F("pp-Asf9x9-median_blend_lower_offset=", lAsfSharp9x9->median_blend_lower_offset);
    }

    PARSE_I_A1("pp-Wnr-BilateralScalecore0=", lWnr->bilateral_scalecore0);
    PARSE_I_A1("pp-Wnr-BilateralScalecore1=", lWnr->bilateral_scalecore1);
    PARSE_I_A1("pp-Wnr-BilateralScalecore2=", lWnr->bilateral_scalecore2);
    PARSE_I_A1("pp-Wnr-NoiseThresholdCore0=", lWnr->noise_thresholdcore0);
    PARSE_I_A1("pp-Wnr-NoiseThresholdCore1=", lWnr->noise_thresholdcore1);
    PARSE_I_A1("pp-Wnr-NoiseThresholdCore2=", lWnr->noise_thresholdcore2);
    PARSE_I_A1("pp-Wnr-WeightCore0=", lWnr->weightcore0);
    PARSE_I_A1("pp-Wnr-WeightCore1=", lWnr->weightcore1);
    PARSE_I_A1("pp-Wnr-WeightCore2=", lWnr->weightcore2);
  } else {
    QIDBG_ERROR("%s:%d] PP data not valid!", __func__, __LINE__);
  }

  IF_META_AVAILABLE(cam_intf_meta_imglib_t, lMetaImglib, CAM_INTF_META_IMGLIB, lMeta) {
    cam_intf_meta_imglib_input_aec_t* lInputAec = &lMetaImglib->meta_imglib_input_aec;
    PARSE_U("imglib-reproc-input-frames-aec=", lInputAec->frame_count);
    for (uint32_t i = 0;
        i < QIMIN(lInputAec->frame_count, QIARRAY_SIZE(lInputAec->aec_data)); i++) {
      PARSE_U("imglib-reproc-frame=", i);
      PARSE_F("imglib-reproc-input-gain=", lInputAec->aec_data[i].real_gain);
      PARSE_F("imglib-reproc-input-lux-idx=", lInputAec->aec_data[i].lux_idx);
      PARSE_F("imglib-reproc-input-exp-time=", lInputAec->aec_data[i].exp_time);
    }

  }

  return mMobicatStr;
}

/*===========================================================================
 * Function: GetMIPayloadSize
 *
 * Description: return size of the multi image payload
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   uint32_t - size
 *==========================================================================*/
uint32_t QMobicatComposer::GetMIPayloadSize()
{
  return mMIPayload_size;
}

/*===========================================================================
 * Function: ComposeMultiImageData
 *
 * Description: Parse metadata data into tags and compose string
 *
 * Input parameters:
 *   metadata - Metadata Pointer
 *   calibration_data - Calibration data
 *
 * Return values:
 *   char* - composed Multi Image Payload
 *
 * Notes: none
 *==========================================================================*/
char* QMobicatComposer::ComposeMultiImageData(uint8_t *metadata,
  uint8_t *jpeg_metadata)
{
  cam_jpeg_metadata_t *lstatic_meta;
  cam_related_system_calibration_data_t *lcal_data = NULL;
  mobicatBufferInfo lBuffer;
  char *lScratchBuffer;
  metadata_buffer_t *lMeta;
  cam_stream_crop_info_t lstream_crop;
  cam_focal_length_ratio_t lflen_ratio;

  /* typecast private meta data */
  lstatic_meta = (cam_jpeg_metadata_t*)jpeg_metadata;
  lMeta = (metadata_buffer_t *)metadata;

  if ((metadata == NULL)&& (lstatic_meta == NULL)){
    QIDBG_ERROR("%s:%d] Input Data is NULL", __func__, __LINE__);
    return 0;
  }

  if (lstatic_meta != NULL) {
    lcal_data = &lstatic_meta->otp_calibration_data;
  } else {
    QIDBG_ERROR("%s:%d] Static meta/Calibration data is NULL",
      __func__,__LINE__);
  }

  if (mMIPayload) {
    free(mMIPayload);
  }
  mMIPayload = (char *)calloc(MAX_MI_DATA_LENGTH, sizeof(char));
  if (mMIPayload == NULL) {
    QIDBG_ERROR("%s:%d] Error no memory", __func__, __LINE__);
    return 0;
  }

  lScratchBuffer = (char *)calloc(MAX_MI_DATA_LENGTH, sizeof(char));
  if (lScratchBuffer == NULL) {
    QIDBG_ERROR("%s:%d] Error no memory", __func__, __LINE__);
    free(mMIPayload);
    mMIPayload = NULL;
    return 0;
  }

  /*Set Buffer info*/
  memset(&lBuffer, 0x0, sizeof(mobicatBufferInfo));
  lBuffer.outStr = mMIPayload;
  lBuffer.scratchBuf = lScratchBuffer;
  lBuffer.bufLength = MAX_MI_DATA_LENGTH;

  if (lMeta != NULL) {
    //Get Sensor Stream Crop Info
    IF_META_AVAILABLE(cam_stream_crop_info_t, stream_crop,
      CAM_INTF_META_SNAP_CROP_INFO_SENSOR, lMeta) {
      lstream_crop = *stream_crop;
      PARSE_I_UNIT(&lBuffer, "Sensor Crop left = ",
        lstream_crop.crop.left, NULL);
      PARSE_I_UNIT(&lBuffer, "Sensor Crop top = ",
        lstream_crop.crop.top, NULL);
      PARSE_I_UNIT(&lBuffer, "Sensor Crop width = ",
        lstream_crop.crop.width, NULL);
      PARSE_I_UNIT(&lBuffer, "Sensor Crop height = ",
        lstream_crop.crop.height, NULL);
      PARSE_I_UNIT(&lBuffer, "Sensor ROI Map left = ",
        lstream_crop.roi_map.left, NULL);
      PARSE_I_UNIT(&lBuffer, "Sensor ROI Map top = ",
        lstream_crop.roi_map.top, NULL);
      PARSE_I_UNIT(&lBuffer, "Sensor ROI Map width = ",
        lstream_crop.roi_map.width, NULL);
      PARSE_I_UNIT(&lBuffer, "Sensor ROI Map height = ",
        lstream_crop.roi_map.height, NULL);
    }
    //Get Camif Stream Crop Info
    IF_META_AVAILABLE(cam_stream_crop_info_t, camif_stream_crop,
      CAM_INTF_META_SNAP_CROP_INFO_CAMIF, lMeta) {
      lstream_crop = *camif_stream_crop;
      PARSE_I_UNIT(&lBuffer, "CAMIF Crop left = ",
        lstream_crop.crop.left, NULL);
      PARSE_I_UNIT(&lBuffer, "CAMIF Crop top = ",
        lstream_crop.crop.top, NULL);
      PARSE_I_UNIT(&lBuffer, "CAMIF Crop width = ",
        lstream_crop.crop.width, NULL);
      PARSE_I_UNIT(&lBuffer, "CAMIF Crop height = ",
        lstream_crop.crop.height, NULL);
      PARSE_I_UNIT(&lBuffer, "CAMIF ROI Map left = ",
        lstream_crop.roi_map.left, NULL);
      PARSE_I_UNIT(&lBuffer, "CAMIF ROI Map top = ",
        lstream_crop.roi_map.top, NULL);
      PARSE_I_UNIT(&lBuffer, "CAMIF ROI Map width = ",
        lstream_crop.roi_map.width, NULL);
      PARSE_I_UNIT(&lBuffer, "CAMIF ROI Map height = ",
        lstream_crop.roi_map.height, NULL);
    }
    //Get ISP Crop Info
    IF_META_AVAILABLE(cam_stream_crop_info_t, isp_stream_crop,
      CAM_INTF_META_SNAP_CROP_INFO_ISP, lMeta) {
      lstream_crop = *isp_stream_crop;
      PARSE_I_UNIT(&lBuffer, "ISP Crop left = ",
        lstream_crop.crop.left, NULL);
      PARSE_I_UNIT(&lBuffer, "ISP Crop top = ",
        lstream_crop.crop.top, NULL);
      PARSE_I_UNIT(&lBuffer, "ISP Crop width = ",
        lstream_crop.crop.width, NULL);
      PARSE_I_UNIT(&lBuffer, "ISP Crop height = ",
        lstream_crop.crop.height, NULL);
      PARSE_I_UNIT(&lBuffer, "ISP ROI Map left = ",
        lstream_crop.roi_map.left, NULL);
      PARSE_I_UNIT(&lBuffer, "ISP ROI Map top = ",
        lstream_crop.roi_map.top, NULL);
      PARSE_I_UNIT(&lBuffer, "ISP ROI Map width = ",
        lstream_crop.roi_map.width, NULL);
      PARSE_I_UNIT(&lBuffer, "ISP ROI Map height = ",
        lstream_crop.roi_map.height, NULL);
    }
    //Get CPP Crop Info
    IF_META_AVAILABLE(cam_stream_crop_info_t, cpp_stream_crop,
      CAM_INTF_META_SNAP_CROP_INFO_CPP, lMeta) {
      lstream_crop = *cpp_stream_crop;
      PARSE_I_UNIT(&lBuffer, "CPP Crop left = ",
        lstream_crop.crop.left, NULL);
      PARSE_I_UNIT(&lBuffer, "CPP Crop top = ",
        lstream_crop.crop.top, NULL);
      PARSE_I_UNIT(&lBuffer, "CPP Crop width = ",
        lstream_crop.crop.width, NULL);
      PARSE_I_UNIT(&lBuffer, "CPP Crop height = ",
        lstream_crop.crop.height, NULL);
      PARSE_I_UNIT(&lBuffer, "CPP ROI Map left = ",
        lstream_crop.roi_map.left, NULL);
      PARSE_I_UNIT(&lBuffer, "CPP ROI Map top = ",
        lstream_crop.roi_map.top, NULL);
      PARSE_I_UNIT(&lBuffer, "CPP ROI Map width = ",
        lstream_crop.roi_map.width, NULL);
      PARSE_I_UNIT(&lBuffer, "CPP ROI Map height = ",
        lstream_crop.roi_map.height, NULL);
    }

    //Get JPEG Crop Info
    IF_META_AVAILABLE(cam_stream_crop_info_t, jpeg_stream_crop,
      CAM_INTF_PARM_JPEG_ENCODE_CROP, lMeta) {
      lstream_crop = *jpeg_stream_crop;
      PARSE_I_UNIT(&lBuffer, "JPEG Crop left = ",
        lstream_crop.crop.left, NULL);
      PARSE_I_UNIT(&lBuffer, "JPEG Crop top = ",
        lstream_crop.crop.top, NULL);
      PARSE_I_UNIT(&lBuffer, "JPEG Crop width = ",
        lstream_crop.crop.width, NULL);
      PARSE_I_UNIT(&lBuffer, "JPEG Crop height = ",
        lstream_crop.crop.height, NULL);
      PARSE_I_UNIT(&lBuffer, "JPEG ROI Map left = ",
        lstream_crop.roi_map.left, NULL);
      PARSE_I_UNIT(&lBuffer, "JPEG ROI Map top = ",
        lstream_crop.roi_map.top, NULL);
      PARSE_I_UNIT(&lBuffer, "JPEG ROI Map width = ",
        lstream_crop.roi_map.width, NULL);
      PARSE_I_UNIT(&lBuffer, "JPEG ROI Map height = ",
        lstream_crop.roi_map.height, NULL);
    }

    //Add Placeholder for DDM Crop. Will be filled later
    PARSE_S_UNIT(&lBuffer, "DDM Crop left = ",
      "0000", NULL);
    PARSE_S_UNIT(&lBuffer, "DDM Crop top = ",
      "0000", NULL);
    PARSE_S_UNIT(&lBuffer, "DDM Crop width = ",
      "0000", NULL);
    PARSE_S_UNIT(&lBuffer, "DDM Crop height = ",
      "0000", NULL);

    //Get Focal length ratio
    IF_META_AVAILABLE(cam_focal_length_ratio_t, flen_ratio,
      CAM_INTF_META_AF_FOCAL_LENGTH_RATIO, lMeta) {
      lflen_ratio = *flen_ratio;
      PARSE_F_UNIT(&lBuffer, "Focal length Ratio = ",
        lflen_ratio.focalLengthRatio, NULL);
    }

    //Get current pipeline flip and rotational parameters
    IF_META_AVAILABLE(int32_t, current_pipeline_flip,
      CAM_INTF_PARM_FLIP, lMeta) {
      PARSE_U_UNIT(&lBuffer, "Current pipeline mirror flip setting = ",
        *current_pipeline_flip, NULL);
    }

    IF_META_AVAILABLE(cam_rotation_info_t, current_pipeline_rotation,
      CAM_INTF_PARM_ROTATION, lMeta) {
      uint32_t rotation = 0;
      switch (current_pipeline_rotation->rotation) {
        case ROTATE_0:
          rotation = 0;
          break;
        case ROTATE_90:
          rotation = 90;
          break;
        case ROTATE_180:
          rotation = 180;
          break;
        case ROTATE_270:
          rotation = 270;
          break;
        default:
          break;
      }
      PARSE_U_UNIT(&lBuffer, "Current pipeline rotation setting = ",
        rotation, NULL);
    }
    IF_META_AVAILABLE(cam_area_t, af_roi,
      CAM_INTF_META_AF_ROI, lMeta) {
        PARSE_I_UNIT(&lBuffer, "AF ROI left = ",
          af_roi->rect.left, NULL);
        PARSE_I_UNIT(&lBuffer, "AF ROI top = ",
          af_roi->rect.top, NULL);
        PARSE_I_UNIT(&lBuffer, "AF ROI width = ",
          af_roi->rect.width, NULL);
        PARSE_I_UNIT(&lBuffer, "AF ROI height = ",
          af_roi->rect.height, NULL);
    }
  } else {
    QIDBG_HIGH("%s %d Metadata is NULL", __func__, __LINE__);
  }

  /*Parse static metadata*/
  if (lstatic_meta != NULL) {
    QIDBG_MED("%s: Sensor Default Flip Present", __func__);
    PARSE_U_UNIT(&lBuffer, "Sensor default mirror flip setting = ",
      lstatic_meta->default_sensor_flip, NULL);

    QIDBG_MED("%s: Sensor Mount Angle Present", __func__);
    PARSE_U_UNIT(&lBuffer, "Sensor mount angle = ",
      lstatic_meta->sensor_mount_angle, NULL);
  }

  /*Parse Calibration data*/
  if (lcal_data != NULL) {
    QIDBG_MED("%s: Calibration data Present", __func__);
    PARSE_U_UNIT(&lBuffer, "Calibration OTP format version = ",
      lcal_data->calibration_format_version, NULL);
    /*Main Camera Data*/
    PARSE_F_UNIT(&lBuffer, "Main Normalized Focal length = ",
      lcal_data->main_cam_specific_calibration.normalized_focal_length, "px");
    PARSE_U16_UNIT(&lBuffer, "Main Native Sensor Resolution width = ",
      lcal_data->main_cam_specific_calibration.native_sensor_resolution_width,
      "px");
    PARSE_U16_UNIT(&lBuffer, "Main Native Sensor Resolution height = ",
      lcal_data->main_cam_specific_calibration.native_sensor_resolution_height,
      "px");
    PARSE_U16_UNIT(&lBuffer, "Main Calibration Resolution width = ",
      lcal_data->main_cam_specific_calibration.calibration_sensor_resolution_width,
      "px");
    PARSE_U16_UNIT(&lBuffer, "Main Calibration Resolution height = ",
      lcal_data->main_cam_specific_calibration.calibration_sensor_resolution_height,
      "px");
    PARSE_F_UNIT(&lBuffer, "Main Focal length ratio = ",
      lcal_data->main_cam_specific_calibration.focal_length_ratio, NULL);

    /*Aux Camera Data*/
    PARSE_F_UNIT(&lBuffer, "Aux Normalized Focal length = ",
      lcal_data->aux_cam_specific_calibration.normalized_focal_length, "px");
    PARSE_U16_UNIT(&lBuffer, "Aux Native Sensor Resolution width = ",
      lcal_data->aux_cam_specific_calibration.native_sensor_resolution_width,
      "px");
    PARSE_U16_UNIT(&lBuffer, "Aux Native Sensor Resolution height = ",
      lcal_data->aux_cam_specific_calibration.native_sensor_resolution_height,
      "px");
    PARSE_U16_UNIT(&lBuffer, "Aux Calibration Resolution width = ",
      lcal_data->aux_cam_specific_calibration.calibration_sensor_resolution_width,
      "px");
    PARSE_U16_UNIT(&lBuffer, "Aux Calibration Resolution height = ",
      lcal_data->aux_cam_specific_calibration.calibration_sensor_resolution_height,
      "px");
    PARSE_F_UNIT(&lBuffer, "Aux Focal length ratio = ",
      lcal_data->aux_cam_specific_calibration.focal_length_ratio, NULL);

    PARSE_ALL_F_A1(&lBuffer, "Relative Rotation matrix [0] through [8] = ",
      lcal_data->relative_rotation_matrix);
    PARSE_ALL_F_A1(&lBuffer, "Relative Geometric surface parameters [0] through [31] = ",
      lcal_data->relative_geometric_surface_parameters);
    PARSE_F_UNIT(&lBuffer, "Relative Principal point X axis offset (ox) = ",
      lcal_data->relative_principle_point_x_offset, "px");
    PARSE_F_UNIT(&lBuffer, "Relative Principal point Y axis offset (oy) = ",
      lcal_data->relative_principle_point_y_offset, "px");
    PARSE_U16_UNIT(&lBuffer, "Relative position flag = ",
      lcal_data->relative_position_flag, NULL);
    PARSE_F_UNIT(&lBuffer, "Baseline distance = ",
      lcal_data->relative_baseline_distance, "mm");
    PARSE_U16_UNIT(&lBuffer, "Main sensor mirror and flip setting = ",
      lcal_data->main_sensor_mirror_flip_setting, NULL);
    PARSE_U16_UNIT(&lBuffer, "Aux sensor mirror and flip setting = ",
      lcal_data->aux_sensor_mirror_flip_setting, NULL);
    PARSE_U16_UNIT(&lBuffer, "Module orientation during calibration = ",
      lcal_data->module_orientation_during_calibration, NULL);
    PARSE_U16_UNIT(&lBuffer, "Rotation flag = ",
      lcal_data->rotation_flag, NULL);
  }

  free(lScratchBuffer);
  mMIPayload_size = strlen(mMIPayload);
  QIDBG_MED("%s %d:] MPO metadata payload size %d", __func__, __LINE__,
    mMIPayload_size);

  return mMIPayload;

}

/*===========================================================================
 * Function: parseVal
 *
 * Description: Parses a value of type T into a string, using format string fmt
 *
 * Input parameters:
 *   fmt, aTag, aVal
 *
 * Return values:
 *   void
 *
 * Notes: none
 *==========================================================================*/
template <typename T> void QMobicatComposer::parseVal(const char *fmt,
  const char *aTag, T aVal)
{

  snprintf(mScratchBuf, MAX_MOBICAT_LENGTH, "%s", aTag);
  strlcat(mMobicatStr, mScratchBuf, MAX_MOBICAT_LENGTH);

  snprintf(mScratchBuf, MAX_MOBICAT_LENGTH, fmt, aVal);
  strlcat(mMobicatStr, mScratchBuf, MAX_MOBICAT_LENGTH);

  strlcat(mMobicatStr, "\n", MAX_MOBICAT_LENGTH);

}

/*===========================================================================
 * Function: parseVal
 *
 * Description: Parses an array of type T into a string, using format string fmt
 *
 * Input parameters:
 *   fmt, aTag, aValPtr, aLen
 *
 * Return values:
 *   void
 *
 * Notes: none
 *==========================================================================*/
template <typename T> void QMobicatComposer::parseValArr(const char *fmt,
  const char *aTag, T *aValPtr, int aLen)
{
  snprintf(mScratchBuf, MAX_MOBICAT_LENGTH, "%s", aTag);
  strlcat(mMobicatStr, mScratchBuf, MAX_MOBICAT_LENGTH);

  while (aLen > 0) {
    snprintf(mScratchBuf, MAX_MOBICAT_LENGTH, fmt, *aValPtr);
    strlcat(mMobicatStr, mScratchBuf, MAX_MOBICAT_LENGTH);

    if (aLen != 1) {
      strlcat(mMobicatStr, ",", MAX_MOBICAT_LENGTH);
    }
    aValPtr++;
    aLen--;
  }
  strlcat(mMobicatStr, "\n", MAX_MOBICAT_LENGTH);
}

/*===========================================================================
 * Function: parseValUnit
 *
 * Description: Parses a value of type T into a string, using format string fmt
 * and append a unit string. Also takes buffer info to write into.
 *
 * Input parameters:
 *   bufInfo, fmt, aTag, aVal, unit
 *
 * Return values:
 *   void
 *
 * Notes: none
 *==========================================================================*/
template <typename T> void QMobicatComposer::parseValUnit(
  mobicatBufferInfo *bufInfo, const char *fmt, const char *aTag,
  T aVal, const char *unit)
{
  snprintf(bufInfo->scratchBuf, bufInfo->bufLength, "%s", aTag);
  strlcat(bufInfo->outStr, bufInfo->scratchBuf,
    (bufInfo->bufLength - strlen(bufInfo->outStr) - 1));

  snprintf(bufInfo->scratchBuf, bufInfo->bufLength, fmt, aVal);
  strlcat(bufInfo->outStr, bufInfo->scratchBuf,
    (bufInfo->bufLength - strlen(bufInfo->outStr) - 1));

  if (NULL != unit) {
    snprintf(bufInfo->scratchBuf, bufInfo->bufLength, "%s", unit);
    strlcat(bufInfo->outStr, bufInfo->scratchBuf,
      (bufInfo->bufLength - strlen(bufInfo->outStr) - 1));
  }

  strlcat(bufInfo->outStr, "\n",
    (bufInfo->bufLength - strlen(bufInfo->outStr) - 1));
}

/*===========================================================================
 * Function: parseValArr
 *
 * Description: Parses an array of type T into a string, using format string fmt
 *
 * Input parameters:
 *   fmt, aTag, aValPtr, aLen
 *
 * Return values:
 *   void
 *
 * Notes: none
 *==========================================================================*/
template <typename T> void QMobicatComposer::parseValArr(mobicatBufferInfo
  *bufInfo, const char *fmt, const char *aTag, T *aValPtr, int aLen)
{
  snprintf(bufInfo->scratchBuf, bufInfo->bufLength, "%s", aTag);
  strlcat(bufInfo->outStr, bufInfo->scratchBuf,
    (bufInfo->bufLength - strlen(bufInfo->outStr) - 1));

  while (aLen > 0) {
    snprintf(bufInfo->scratchBuf, bufInfo->bufLength, fmt, *aValPtr);
    strlcat(bufInfo->outStr, bufInfo->scratchBuf,
      (bufInfo->bufLength - strlen(bufInfo->outStr) - 1));
    if (aLen != 1) {
      strlcat(bufInfo->outStr, ",",
        (bufInfo->bufLength - strlen(bufInfo->outStr) - 1));
    }
    aValPtr++;
    aLen--;
  }
  strlcat(bufInfo->outStr, "\n",
    (bufInfo->bufLength - strlen(bufInfo->outStr) - 1));
}
