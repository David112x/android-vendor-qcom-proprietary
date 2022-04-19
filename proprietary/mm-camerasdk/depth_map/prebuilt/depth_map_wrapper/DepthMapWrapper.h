#ifndef __DEPTHMAP_WRAPPER_H__
#define __DEPTHMAP_WRAPPER_H__

/**=============================================================================

Copyright (c) 2017 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================**/


//==============================================================================
// Included modules
//==============================================================================
#include <stdint.h>

//==============================================================================
// MACROS
//==============================================================================
#ifdef _MSC_VER
#define CP_DLL_PUBLIC __declspec(dllexport)
#else
#define CP_DLL_PUBLIC __attribute__ ((visibility ("default")))
#endif

#define IMG_UNUSED(x) (void)(x)

//==============================================================================
// DECLARATIONS
//==============================================================================

typedef struct pc_pack{
  float X;
  float Y;
  float Z;
  float c;
}pc_pkt_t;

typedef enum {
  WRAPPER_CAM_FORMAT_NONE,
  WRAPPER_CAM_FORMAT_IR,
  WRAPPER_CAM_FORMAT_DEPTH16,
  WRAPPER_CAM_FORMAT_DEPTH_POINT_CLOUD,
} WrapperDepthFormat;

typedef enum {
  WRAPPER_EXP_MODE_ME,
  WRAPPER_EXP_MODE_AE,
  WRAPPER_EXP_MODE_NONE
} WrapperDepthExpModeType;

typedef struct {
  float min_fps;
  float max_fps;
  float video_min_fps;
  float video_max_fps;
} WrapperDepthCamFpsRange;

typedef struct {
  int64_t in_depth_map_me_val;
  int64_t out_depth_map_ae_val;
  int64_t out_depth_map_laser_strength_val;
  WrapperDepthExpModeType in_depth_map_exp_mode;
  WrapperDepthCamFpsRange in_depth_map_fps_range;
} WrapperDepthCamConfig;

//==============================================================================
// API CLASS
//==============================================================================
class CP_DLL_PUBLIC DepthMapWrapper
{
  public:
  DepthMapWrapper(int width, int height, int dm_width, int dm_height);

  bool  processFrame(WrapperDepthFormat wrapper_depth_format,
    const char *in_image,
    char *out_depth_image,
    unsigned long int out_image_lenth,
    pc_pkt_t *out_pcloud_image,
    unsigned int *count_pt_cloud,
    WrapperDepthCamConfig *wrapper_depth_map_config);

  ~DepthMapWrapper() ;

  //for future use
  static unsigned int getWrapperVersion();

private:
  struct DepthWrapperData;
  DepthWrapperData *pDepthData;

   DepthMapWrapper(); // no default constructor
   DepthMapWrapper( const DepthMapWrapper& ); // no copy constructor
   DepthMapWrapper& operator=( const DepthMapWrapper& ); // no copy assignment operator

};

#endif //__DEPTHMAP_WRAPPER_H__

