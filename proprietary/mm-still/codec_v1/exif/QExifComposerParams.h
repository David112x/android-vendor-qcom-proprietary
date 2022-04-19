/*****************************************************************************
* Copyright (c) 2012-2016 Qualcomm Technologies, Inc.  All Rights Reserved.  *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QEXIF_COMPOSER_PARAMS_H__
#define __QEXIF_COMPOSER_PARAMS_H__

extern "C" {
#include "exif.h"
#include "exif_private.h"
#include <stdlib.h>
}
#include "QEncodeParams.h"
#include "QICommon.h"

#define SW_3A_VER_SIZE 8
#define FMT_3A_VER_SIZE 4

/*===========================================================================
 * Class: QExifComposerParams
 *
 * Description: This class represents the exif composer parameters
 *
 * Notes: none
 *==========================================================================*/
class QExifComposerParams
{
public:

  /** MutiImageInfo
   *
   *  @image_type : Image type
   *  @is_primary: Is the image a primary image in the sequence
   *  @num_of_images: Total num of images in the sequence
   *  @large_thumbnail_present: Large thumbnail present
   **/

  typedef struct {
    QIImageType image_type;
    bool is_primary;
    uint32_t num_of_images;
    bool large_thumbnail_present;

  } MutiImageInfo;

  /** QExifComposerParams
   *
   *  constructor
   **/
  QExifComposerParams();

  /** App2HeaderLen
   *
   *  returns app2 marker header length
   **/
  inline uint32_t App2HeaderLen()
  {
    return mApp2Len;
  }

  /** Exif
   *
   *  returns exif tag object
   **/
  inline exif_info_obj_t *Exif()
  {
    return mExifObjInfo;
  }

  /** EncodeParams
   *  @aThumb: flag to indicate the encode parameters belong to
   *         thumbnail or main image
   *
   *  returns encode parameters
   **/
  inline QIEncodeParams& EncodeParams(bool aThumb = false)
  {
    return (aThumb) ? *mThumbEncodeParams : *mMainEncodeParams;
  }

  /** Subsampling
   *  @aThumb: flag to indicate the subsampling belong to
   *         thumbnail or main image
   *
   *  returns image subsampling
   **/
  inline QISubsampling& Subsampling(bool aThumb = false)
  {
    return (aThumb) ? mThumbSS : mMainSS;
  }

  /** SetAppHeaderLen
   *  @aApp2Len: app2 marker header length
   *
   *  sets app2 marker length
   **/
  inline void SetAppHeaderLen(uint32_t aApp2Len)
  {
    mApp2Len = aApp2Len;
  }

  /** SetExif
   *  @aExif: exif object
   *
   *  sets exif info
   **/
  inline void SetExif(exif_info_obj_t *aExif)
  {
    mExifObjInfo = aExif;
  }

  /** SetEncodeParams
   *  @aParams: encode parameters
   *  @aThumb: flag to indicate the subsampling belong to
   *         thumbnail or main image
   *
   *  sets encode parameters
   **/
  void SetEncodeParams(QIEncodeParams &aParams, bool aThumb = false);

  /** SetSubSampling
   *  @aSS: image subsampling
   *  @aThumb: flag to indicate the subsampling belong to
   *         thumbnail or main image
   *
   *  sets encode parameters
   **/
  void SetSubSampling(QISubsampling aSS, bool aThumb = false);

  /** SetMobicat
   *  @aMobicatStr: mobicat str
   *
   *  sets mobicat str
   **/
  inline void SetMobicat(char* aMobicatStr)
  {
    mMobicatStr = aMobicatStr;
  }

  /** MobicatStr
   *
   *  returns mobicat str
   **/
  inline char* GetMobicat()
  {
    return mMobicatStr;
  }

  /** Set3ASWVer
   *  @m3ASWVersion: 3A SW vesion
   *
   *  sets 3A SW vesion
   **/
  inline void Set3ASWVer(uint16_t* p3ASWVersion)
  {
    m3ASWVersion = p3ASWVersion;
  }

  /** 3ASWVersion
   *
   *  returns 3A SW vesion
   **/
  inline uint16_t* Get3ASWVer()
  {
    return m3ASWVersion;
  }

  /** SetMobicatFlag
   *  @aMobicatFlag: flag
   *
   *  enables mobicat to true or false
   **/
  inline void SetMobicatFlag(bool aMobicatFlag)
  {
    mEnableMobicat = aMobicatFlag;
  }

  /** GetMobicatFlag
   *
   *  returns if mobicat is enabled or not
   **/
  inline bool GetMobicatFlag()
  {
    return mEnableMobicat;
  }

  /** SetAEData
   *  @payload: 3A stats payload
   *  @size: size of payload
   *
   *  sets 3A stats payload and size
   **/
  inline void Set3A(char* payload, uint32_t size)
  {
    mStatsPayload = payload;
    mStatsPayloadSize = size;
  }

  /** Get3A
   *
   *  returns 3A stats payload
   **/
  inline char* Get3A()
  {
    return mStatsPayload;
  }

  /** Get3ASize
   *
   *  returns size of stats payload
   **/
  inline uint32_t Get3ASize()
  {
    return mStatsPayloadSize;
  }

  /** Set3AFlag
   *
   *  returns whether 3a stats are to
   *  be written to appmarker or not
   **/
  inline void Set3AFlag(bool flag)
  {
    mEnable3A = flag;
  }

  /** Get3AFlag
   *
   *  returns whether AE params are to
   *  be written to appmarker or not
   **/
  inline bool Get3AFlag()
  {
    return mEnable3A;
  }

  /**SetMultiImageInfo
  *
  * Set multiple image info
  */
  inline void SetMultiImageInfo(MutiImageInfo aMPInfo)
  {
    mMPInfo = aMPInfo;
  }

  /**SetMultiImageInfo
  *
  * Set multiple image info
  */
  inline MutiImageInfo GetMultiImageInfo()
  {
    return mMPInfo;
  }

  /** SetMIMetaFlag
   *
   *  returns whether MI meta info are to be written to appmarker
   *  or not
   **/
  inline void SetMIMetaFlag(bool flag)
  {
    mEnableMI = flag;
  }

  /** GetMIMetaFlag
   *
   *  returns whether MI meta info are to
   *  be written to appmarker or not
   **/
  inline bool GetMIMetaFlag()
  {
    return mEnableMI;
  }

  /** SetMIMetadeta
   *  @payload: 3A stats payload
   *  @size: size of payload
   *
   *  sets 3A stats payload and size
   **/
  inline void SetMIMetadata(char* payload, uint32_t size)
  {
    mMIMetaPayload = payload;
    mMIMetaPayloadSize = size;
  }

  /** Get3A
   *
   *  returns 3A stats payload
   **/
  inline char* GetMIMetadata()
  {
    return mMIMetaPayload;
  }

  /** GetMIMetaSize
  *
  * returns MI metadata payload
  */
  inline uint32_t GetMIMetaSize()
  {
    return mMIMetaPayloadSize;
  }
private:

  /** mApp2Len
   *
   *  App2 header length
   **/
  uint32_t mApp2Len;

  /** mExifObjInfo
   *
   *  exif object info
   **/
  exif_info_obj_t *mExifObjInfo;

  /** mMainEncodeParams
   *
   *  main encode parameters
   **/
  QIEncodeParams *mMainEncodeParams;

  /** mThumbEncodeParams
   *
   *  thumbnail encode parameters
   **/
  QIEncodeParams *mThumbEncodeParams;

  /** mMainSS
   *
   *  main subsampling
   **/
  QISubsampling mMainSS;

  /** mMainSS
   *
   *  thumbnail subsampling
   **/
  QISubsampling mThumbSS;

  /** mMobicatStr
   *
   *  parsed Mobicat data
   **/
  char* mMobicatStr;

  /** m3ASWVersion
   *
   *  3A SW Version info
   **/
  uint16_t* m3ASWVersion;

  /** mEnableMobicat
   *
   *  mobicat enabled flag
   **/
  bool mEnableMobicat;

  /** mStatsPayload
   *
   *  3A stats payload
   **/
  char* mStatsPayload;

  /** mStatsPayloadSize
   *
   *  3A stats payload size
   **/
  uint32_t mStatsPayloadSize;

  /** mEnable3A
   *
   *  flag to write 3A stats to appmarker
   **/
  bool mEnable3A;

  /** mMPInfo
  *
  * Multi Image Info
  */
  MutiImageInfo mMPInfo;

  /** mEnableMI
   *
   *  flag to write 3A stats to appmarker
   **/
  bool mEnableMI;

  /** mMIMetaPayload
   *
   *  Multi Image metadata payload
   **/
  char* mMIMetaPayload;

  /** mIMetaPayloadSize
   *
   *  Multi Image metadata payload size
   **/
  uint32_t mMIMetaPayloadSize;

};

#endif //__QEXIF_COMPOSER_PARAMS_H__

