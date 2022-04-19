/*****************************************************************************
* Copyright (c) 2013-2016 Qualcomm Technologies, Inc.  All Rights Reserved.  *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QMOBICATCOMPOSER_H__
#define __QMOBICATCOMPOSER_H__

extern "C" {
#include "mct_stream.h"
}

/*===========================================================================
 * Class: QMobicatComposer
 *
 * Description: This class represents the mobicat composer utility
 *
 * Notes: none
 *==========================================================================*/
class QMobicatComposer {

public:

  /** mobicatBufferInfo
   * Buffer info to compose string in mobicat format
   *
   * @scratchBuf: Scratch Buffer
   * @bufLength: Buffer length
   * @outStr: Output String
  */
  typedef struct {
    char *scratchBuf;
    int bufLength;
    char *outStr;
  } mobicatBufferInfo;

  /** QMobicatComposer
   *
   *  Constructor
   **/
  QMobicatComposer();

  /** ~QMobicatComposer
   *
   *  virtual destructor
   **/
  ~QMobicatComposer();

   /** ParseMobicatData
   *  @metadata: contains metadata info
   *
   *  Parse metadata into mobicat tags and return string
   *
   **/
  char* ParseMobicatData(uint8_t *metadata);

   /** Compose3AStatsPayload
   *  @metadata: contains metadata info
   *
   *  compose stats payload
   **/
  char* Compose3AStatsPayload(uint8_t *metadata);

  /** Get3AStatsSize
   *
   *  return length of stats payload
   **/
  uint32_t Get3AStatsSize();

   /** ExtractAECData
   *  @lMeta: contains metadata info
   *
   *  Get AEC data from metadata and add to stats payload
   **/
  void ExtractAECData(metadata_buffer_t *lMeta,
    cam_ae_exif_debug_t *ae_ptr, boolean valid);

   /** ExtractAWBData
   *  @lMeta: contains metadata info
   *
   *  Get AWB data from metadata and add to stats payload
   **/
  void ExtractAWBData(metadata_buffer_t *lMeta,
    cam_awb_exif_debug_t *awb_ptr, boolean valid);

  /** ExtractAFData
   *  @lMeta: contains metadata info
   *
   *  Get AF data from metadata and add to stats payload
   **/
  void ExtractAFData(metadata_buffer_t *lMeta,
    cam_af_exif_debug_t *af_ptr, boolean valid);

  /** ExtractASDData
   *  @lMeta: contains metadata info
   *
   *  Get ASD data from metadata and add to stats payload
   **/
  void ExtractASDData(metadata_buffer_t *lMeta,
   cam_asd_exif_debug_t *asd_ptr, boolean valid);

  /** ExtractStatsData
   *  @lMeta: contains metadata info
   *
   *  Get Stats data from metadata and add to stats payload
   **/
  void ExtractStatsData(metadata_buffer_t *lMeta,
    cam_stats_buffer_exif_debug_t *stats_ptr, boolean valid);

  /** ExtractBEStatsData
   *  @lMeta: contains metadata info
   *
   *  Get BE Stats data from metadata and add to stats payload
   **/
  void ExtractBEStatsData(metadata_buffer_t *lMeta,
    cam_bestats_buffer_exif_debug_t *bestats_ptr, boolean valid);

  /** ExtractBHistData
   *  @lMeta: contains metadata info
   *
   *  Get Bayer histogram stats data from metadata and add to stats payload
   **/
  void ExtractBHistData(metadata_buffer_t *lMeta,
    cam_bhist_buffer_exif_debug_t *bhist_ptr, boolean valid);

  /** Extract3ATuningData
    *  @lMeta: contains metadata info
    *
    *  Get Bayer histogram stats data from metadata and add to stats payload
    **/
   void Extract3ATuningData(metadata_buffer_t *lMeta,
     cam_q3a_tuning_info_t *tuning_ptr, boolean valid);

  /** GetMIPayloadSize
   *
   *  return length of Multi Image payload
   **/
  uint32_t GetMIPayloadSize();

  /** ComposeMultiImageData
   *  @metadata: contains metadata info
   *  @calibration_data : Static metadata
   *
   *  Parse metadata/calibration data into mobicat tags and return
   *  string
   *
   **/
  char* ComposeMultiImageData(uint8_t *metadata,
    uint8_t *calibration_data);

private:

  /** mScratchBuf
   *
   *  Temp scratch buffer
   **/
  char *mScratchBuf;

  /** mMobicatStr
   *
   *  Parsed Mobicat String
   **/
  char *mMobicatStr;

  /** mStatsPayload
   *
   *  Stats payload
   **/
  char *mStatsPayload;

  /** mStats_payload_size
   *
   *  Stats payload size
   **/
  uint32_t mStats_payload_size;

  /** mMIPayload
   *
   *  Multi Image Paylod
   */
  char *mMIPayload;

  /** mMIPayload_size
   *
   *  Multi Image Paylod Size
   **/
  uint32_t mMIPayload_size;

  /** parseVal
   *
   * @fmt - output format string
   * @aTag - mobicat tag
   * @aVal - value to parse
   *
   * Parse a value of type T
   */
  template <typename T> void parseVal(const char *fmt,
      const char *aTag, T aVal);

  /** parseValArr
   *
   * @fmt - output format string
   * @aTag - mobicat tag
   * @aValPtr - array to parse
   * @aLen - Length of array
   *
   * Parse an array of type T
   */
  template <typename T> void parseValArr(const char *fmt,
      const char *aTag, T *aValPtr, int aLen);

  /** parseValUnit
   *
   * @bufInfo - Scratch and o/p buffer info
   * @fmt - output format string
   * @aTag - mobicat tag
   * @aVal - value to parse
   * @unit - Unit to be appended, if any
   *
   * Parse an array of type T
   */
  template <typename T> void parseValUnit(mobicatBufferInfo *bufInfo,
    const char *fmt, const char *aTag,
    T aVal, const char *unit);

  /** parseValArr
   *
   * @bufInfo - Scratch and o/p buffer info
   * @fmt - output format string
   * @aTag - mobicat tag
   * @aValPtr - array to parse
   * @aLen - Length of array
   *
   * Parse an array of type T
   */
  template <typename T> void parseValArr(mobicatBufferInfo *bufInfo,
    const char *fmt, const char *aTag, T *aValPtr, int aLen);

};

#endif //__QMOBICATCOMPOSER_H__
