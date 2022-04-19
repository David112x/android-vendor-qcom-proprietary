/***************************************************************************
* Copyright (c) 2017 Qualcomm Technologies, Inc.                           *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
***************************************************************************/

#include "SpatiallyVaryingBlur.h"
#include "Effects.h"
#include "List.hpp"

#include <android/log.h>


//-----------------------------------------------------------------------------
float GetBlurRadiusForLevel(int level)
//-----------------------------------------------------------------------------
{
    float blurRadius = 0.0f;
    if( level == 0 ) blurRadius = 0;
    else blurRadius = (1<<(level-1))*1.5f;

    return blurRadius;
}

//-----------------------------------------------------------------------------
int GetMipLevelCount(float maxRadius)
//-----------------------------------------------------------------------------
{
    int levels = 0;

    for(int i = 0; ; i++)
    {
        if( GetBlurRadiusForLevel(i) > maxRadius )
        {
            levels = i;
            break;
        }
    }
    return levels;
}

//-----------------------------------------------------------------------------
void GenerateBlurredMipMaps(Effects* pEffects,
                            List<RenderTexture*>* mipMaps,
                            Texture* srcTexture,
                            int levels)
//-----------------------------------------------------------------------------
{
    mipMaps->Clear();

    RenderTexture* rt = RenderTexture::Wrap(srcTexture);
    if (rt == NULL)
    {
      IDBG_ERROR("Invalid return value from RenderTexture::Wrap");
      return;
    }

    for (int i = 0; i <= levels; i++)
    {
      mipMaps->Add(rt);
      rt = pEffects->BlurAndDownscale(rt);
      if (rt == NULL)
      {
        IDBG_ERROR("Invalid return value from BlurAndDownscale");
        return;
      }
    }
}

// BottomUpBlend
// colorMips: Input Color Texture Pyramid
// blurRadiusMips: Input blur Radius Map Pyramid
//-----------------------------------------------------------------------------
RenderTexture* BottomUpBlend(Effects* pEffects,
                             List<RenderTexture*>* colorMips,
                             List<RenderTexture*>* blurRadiusMips)
//-----------------------------------------------------------------------------
{
    RenderTexture* result = 0;
    RenderTexture* blended = colorMips->Last();

    for (int i = colorMips->Size()-2; i >= 0; i--)
    {
        float l = GetBlurRadiusForLevel(i);
        float h = GetBlurRadiusForLevel(i+1);

        result = pEffects->Blend(l, h, colorMips->Get(i), blurRadiusMips->Get(i), blended);

        blended = result;
    }

    return result;
}

// SpatiallyVaryingBlur
// colorMap: Input Color Texture
// blurRadiusMap: Input Blur Radius Map
// maxBlurRadius: Maximum Blur Radius
//-----------------------------------------------------------------------------
RenderTexture* SpatiallyVaryingBlur(Effects* pEffects,
                                    Texture* colorMap,
                                    Texture* blurRadiusMap,
                                    int maxBlurRadius)
//-----------------------------------------------------------------------------
{
    int mipLevels = GetMipLevelCount(maxBlurRadius);

    List<RenderTexture*> colorMipMaps;
    List<RenderTexture*> blurRadiusMaps;

    GenerateBlurredMipMaps(pEffects, &colorMipMaps, colorMap, mipLevels);
    GenerateBlurredMipMaps(pEffects, &blurRadiusMaps, blurRadiusMap, mipLevels);

    RenderTexture* result = BottomUpBlend(pEffects, &colorMipMaps, &blurRadiusMaps);

    RenderTexture::ReleaseAll();

    return result;
}
