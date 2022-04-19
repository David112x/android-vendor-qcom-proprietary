/**********************************************************************
*  Copyright (c) 2017 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "DepthMapWrapper.h"
//include depth map library header file

struct DepthMapWrapper::DepthWrapperData {
  //declare Depth map object.
};

DepthMapWrapper::DepthMapWrapper(int width, int height, int dm_width, int dm_height) {
  pDepthData = new DepthWrapperData;
  if (pDepthData) {
   //instantiate depth library object
  }
}

bool
DepthMapWrapper::processFrame(WrapperDepthFormat wrapper_depth_format,
  const char *in_image,
  char *out_depth_image,
  unsigned long int out_image_lenth,
  pc_pkt_t *out_pcloud_image,
  unsigned int *count_pt_cloud,
  WrapperDepthCamConfig *wrapper_depth_map_config) {

  bool rc = true;

  if (!pDepthData ||!out_depth_image ||
   ((wrapper_depth_format == WRAPPER_CAM_FORMAT_DEPTH_POINT_CLOUD ) &&
   (!out_pcloud_image || ! count_pt_cloud)) ||
   !wrapper_depth_map_config) {
     return false;
  }

  switch(wrapper_depth_format){
    case WRAPPER_CAM_FORMAT_IR:
    // Call relevant depth  API
    break;
    case WRAPPER_CAM_FORMAT_DEPTH16:
    // Call relevant depth API
    break;
    case WRAPPER_CAM_FORMAT_DEPTH_POINT_CLOUD:
    //Call relevant depth API
    break;
    default:
     rc = false;
    break;
  }
  return rc;
}

DepthMapWrapper::~DepthMapWrapper() {
  if (pDepthData) {
    //delete Depth map object
    delete pDepthData;
    pDepthData = NULL;
  }
}


