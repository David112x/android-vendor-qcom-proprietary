/***************************************************************************
* Copyright (c) 2017 Qualcomm Technologies, Inc.                           *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
***************************************************************************/

#ifndef __QCAMERA_SPATIALLY_VARYING_BLUR_H__
#define __QCAMERA_SPATIALLY_VARYING_BLUR_H__

#include "RenderTexture.h"
#include "Effects.h"
#include "Texture.h"

RenderTexture* SpatiallyVaryingBlur(Effects* pEffects,
                                    Texture* colorMap,
                                    Texture* blurRadiusMap,
                                    int maxBlurRadius);

#endif
