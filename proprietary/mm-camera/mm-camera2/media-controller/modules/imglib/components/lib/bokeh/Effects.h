/***************************************************************************
* Copyright (c) 2017 Qualcomm Technologies, Inc.                           *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
***************************************************************************/

#ifndef __QCAMERA_EFFECTS_H__
#define __QCAMERA_EFFECTS_H__

#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglextQCOM.h>
#include "RenderTexture.h"

class Effects {
    public:
        Effects();
        ~Effects();
        RenderTexture* Blur(RenderTexture* srcTexture);
        RenderTexture* Blur(int srcTexture, int w, int h, int f);
        RenderTexture* Downscale(RenderTexture* srcTexture, int factor = 2);
        RenderTexture* Blit(RenderTexture* srcTexture);
        void           Blit(RenderTexture* src, int* viewport);
        bool CropParamters(int inputWidth,
                           int inputHeight,
                           int goodROISrcX,
                           int goodROISrcY,
                           int goodROISrcWidth,
                           int goodROISrcHeight);
        RenderTexture* Bokeh(int b,
                             float* blurRadiusLevels,
                             RenderTexture* color,
                             RenderTexture* depth,
                             RenderTexture* blended);
        RenderTexture* Blend(float low,
                             float high,
                             RenderTexture* color,
                             RenderTexture* depth,
                             RenderTexture* blended);
        RenderTexture* BlurAndDownscale(RenderTexture* srcTexture);
        RenderTexture* BlurAndDownscale(int srcTexture,
                                        int w, int h, int f);
        void           Show(RenderTexture* rt,
                            int screenWidth,
                            int screenHeight);
        bool           Initialize(int inputWidth,
                                  int inputHeight,
                                  int goodROISrcX,
                                  int goodROISrcY,
                                  int goodROISrcWidth,
                                  int goodROISrcHeight,
                                  GLuint outRGBTextureId);
        bool           Deinitialize();

        bool           ConvertYUV2RGB(GLuint sourceTextureId);
        bool           ConvertRGB2YUV(GLuint inputId, GLuint outputId);
        GLuint         createShader(GLenum shaderType, const char* pSource);
        GLuint         createProgram(GLuint vertexShader, GLuint fragShader);
    private:
        int    m_prgPassthru;
        int    m_prgGaussianBlur;
        int    m_prgBlendLevels;
        int    m_prgBlurAndDownscale;
        int    m_prgConvertYUV2RGB;
        int    m_prgConvertRGB2YUV;
        int    m_isTracingEnabled;
        int    m_width;
        int    m_height;
        GLuint m_convertFBO;
};

#endif
