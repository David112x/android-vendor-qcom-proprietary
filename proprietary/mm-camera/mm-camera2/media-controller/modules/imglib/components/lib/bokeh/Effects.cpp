/***************************************************************************
* Copyright (c) 2017-2018 Qualcomm Technologies, Inc.                           *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
***************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "Effects.h"

#include "shaders/vertex_shader.inl"
#include "shaders/passthru.inl"
#include "shaders/gaussian_blur.inl"
#include "shaders/blend_levels.inl"
#include "shaders/blur_and_downscale.inl"
#include "shaders/convert_yuv_to_rgb.inl"
#include "shaders/convert_rgb_to_yuv.inl"

#include <android/log.h>

static GLfloat texcoordAttribsConvert2[] =
{
    0.0f, 2.0f,
    0.0f, 0.0f,
    2.0f, 0.0f,
    2.0f, 2.0f
};

static GLfloat texcoordAttribsConvert[] =
{
    0.0f, 2.0f,
    0.0f, 0.0f,
    2.0f, 0.0f
};

static const GLfloat fullscreenTexcoords[] =
{
    0.0f, 2.0f,
    0.0f, 0.0f,
    2.0f, 0.0f
};

static const float fullscreen_vertices2[] =
{
    -1.0f, -1.0f,
     1.0f, -1.0f,
    -1.0f,  1.0f,
     1.0f,  1.0f
};


static const float fullscreen_vertices[] =
{
    -1.0f,  3.0f,
    -1.0f, -1.0f,
     3.0f, -1.0f
};


//-----------------------------------------------------------------------------
Effects::Effects()
//-----------------------------------------------------------------------------
{
    m_prgPassthru = 0;
    m_prgGaussianBlur = 0;
    m_prgBlendLevels = 0;
    m_prgBlurAndDownscale = 0;
    m_prgConvertYUV2RGB = 0;
    m_prgConvertRGB2YUV = 0;
    m_isTracingEnabled = 0;
    m_width = 0;
    m_height = 0;
    m_convertFBO = 0;
}

//-----------------------------------------------------------------------------
Effects::~Effects()
//-----------------------------------------------------------------------------
{
}

//-----------------------------------------------------------------------------
RenderTexture* Effects::BlurAndDownscale(int srcTexture, int w, int h, int f)
//-----------------------------------------------------------------------------
{

    RenderTexture* blur = 0;
    {
        if (0 != m_isTracingEnabled)
        {
          atrace_begin(ATRACE_TAG_ALWAYS, "bokeh_blurdownscale");
        }
        blur = RenderTexture::GetTemporary(w/2, h/2, f);
        if (blur == NULL)
        {
          IDBG_ERROR("Invalid return value from RenderTexture::GetTemporary");
          return NULL;
        }
        glUseProgram(m_prgBlurAndDownscale);

        float _Size[4] = {1.0f/w, 1.0f/h, 0, 0};
        glUniform4f(1, _Size[0], _Size[1], _Size[2], _Size[3]);

        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, srcTexture);

        glBindFramebuffer(GL_FRAMEBUFFER, blur->fb);
        glViewport(0, 0, blur->Width(), blur->Height());

        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, fullscreen_vertices);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, fullscreenTexcoords);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        if (0 != m_isTracingEnabled)
        {
          atrace_end(ATRACE_TAG_ALWAYS);
        }
    }
    return blur;
}

//-----------------------------------------------------------------------------
GLuint Effects::createShader(GLenum shaderType, const char* pSource)
//-----------------------------------------------------------------------------
{
    GLuint shader = glCreateShader(shaderType);
    if (shader)
    {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled)
        {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen)
            {
                char* buf = (char*)malloc(infoLen);
                if (buf)
                {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    LOGE("Unable to compile shader %d:\n%s\n",
                        shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

//-----------------------------------------------------------------------------
GLuint Effects::createProgram(GLuint vertexShader, GLuint fragShader)
//-----------------------------------------------------------------------------
{
    GLuint program = glCreateProgram();
    if (program)
    {
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragShader);
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE)
        {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength)
            {
                char* buf = (char*)malloc(bufLength);
                if (buf)
                {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    LOGE("Unable to link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
}

//-----------------------------------------------------------------------------
bool Effects::Initialize(int inputWidth,
                        int inputHeight,
                        int goodROISrcX,
                        int goodROISrcY,
                        int goodROISrcWidth,
                        int goodROISrcHeight,
                        GLuint outRGBTextureId)
//-----------------------------------------------------------------------------
{
    char prop[PROPERTY_VALUE_MAX];
    memset(prop, 0, sizeof(prop));
    property_get("persist.vendor.camera.rtb.tracing", prop, "0");
    m_isTracingEnabled = atoi(prop);

    m_width = inputWidth;
    m_height = inputHeight;

    texcoordAttribsConvert[0] = (GLfloat)(2.0f * (float)goodROISrcX/(float)inputWidth);
    texcoordAttribsConvert[1] = (GLfloat)(2.0f * (float)goodROISrcHeight/(float)inputHeight);
    texcoordAttribsConvert[2] = (GLfloat)(2.0f * (float)goodROISrcX/(float)inputWidth);
    texcoordAttribsConvert[3] = (GLfloat)(2.0f * (float)goodROISrcY/(float)inputHeight);
    texcoordAttribsConvert[4] = (GLfloat)(2.0f * (float)goodROISrcWidth/(float)inputWidth);
    texcoordAttribsConvert[5] = (GLfloat)(2.0f * (float)goodROISrcY/(float)inputHeight);

    glGenFramebuffers(1, &m_convertFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_convertFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           outRGBTextureId,
                           0);

    GLuint vertexShader = createShader(GL_VERTEX_SHADER, vertex_shader);
    if (!vertexShader) {
        return false;
    }
    GLuint fragShader = createShader(GL_FRAGMENT_SHADER, passthru_shader);
    if (!fragShader) {
        return false;
    }
    m_prgPassthru = createProgram(vertexShader, fragShader);
    glDeleteShader(fragShader);

    fragShader = createShader(GL_FRAGMENT_SHADER, gaussian_blur_shader);
    if (!fragShader) {
        return false;
    }
    m_prgGaussianBlur = createProgram(vertexShader, fragShader);
    glDeleteShader(fragShader);

    fragShader = createShader(GL_FRAGMENT_SHADER, blend_levels_shader);
    if (!fragShader) {
        return false;
    }
    m_prgBlendLevels = createProgram(vertexShader, fragShader);
    glDeleteShader(fragShader);

    fragShader = createShader(GL_FRAGMENT_SHADER, blur_and_downscale_shader);
    if (!fragShader) {
        return false;
    }
    m_prgBlurAndDownscale = createProgram(vertexShader, fragShader);
    glDeleteShader(fragShader);

    fragShader = createShader(GL_FRAGMENT_SHADER, convert_yuv_to_rgb_shader);
    if (!fragShader) {
        return false;
    }
    m_prgConvertYUV2RGB = createProgram(vertexShader, fragShader);
    glDeleteShader(fragShader);

    fragShader = createShader(GL_FRAGMENT_SHADER, convert_rgb_to_yuv_shader);
    if (!fragShader) {
        return false;
    }
    m_prgConvertRGB2YUV = createProgram(vertexShader, fragShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragShader);

    return true;
}

bool Effects::CropParamters(int inputWidth,
                        int inputHeight,
                        int goodROISrcX,
                        int goodROISrcY,
                        int goodROISrcWidth,
                        int goodROISrcHeight){
    texcoordAttribsConvert2[0] = (GLfloat)((goodROISrcX)/(float)inputWidth);
    texcoordAttribsConvert2[1] = (GLfloat)((goodROISrcY)/(float)inputHeight);
    texcoordAttribsConvert2[2] = (GLfloat)((goodROISrcX + goodROISrcWidth)/(float)inputWidth);
    texcoordAttribsConvert2[3] = (GLfloat)((goodROISrcY)/(float)inputHeight);
    texcoordAttribsConvert2[4] = (GLfloat)((goodROISrcX)/(float)inputWidth);
    texcoordAttribsConvert2[5] = (GLfloat)((goodROISrcY + goodROISrcHeight)/(float)inputHeight);
    texcoordAttribsConvert2[6] = (GLfloat)((goodROISrcX + goodROISrcWidth)/(float)inputWidth);
    texcoordAttribsConvert2[7] = (GLfloat)((goodROISrcY + goodROISrcHeight)/(float)inputHeight);
    return true;
}

//-----------------------------------------------------------------------------
bool Effects::Deinitialize( )
//-----------------------------------------------------------------------------
{
    if (m_prgPassthru)
    {
        glDeleteProgram(m_prgPassthru);
        m_prgPassthru = 0;
    }
    if (m_prgGaussianBlur)
    {
        glDeleteProgram(m_prgGaussianBlur);
        m_prgGaussianBlur = 0;
    }
    if (m_prgBlendLevels)
    {
        glDeleteProgram(m_prgBlendLevels);
        m_prgBlendLevels = 0;
    }
    if (m_prgBlurAndDownscale)
    {
        glDeleteProgram(m_prgBlurAndDownscale);
        m_prgBlurAndDownscale = 0;
    }
    if (m_prgConvertYUV2RGB)
    {
        glDeleteProgram(m_prgConvertYUV2RGB);
        m_prgConvertYUV2RGB = 0;
    }
    if (m_prgConvertRGB2YUV)
    {
        glDeleteProgram(m_prgConvertRGB2YUV);
        m_prgConvertRGB2YUV = 0;
    }
    glDeleteFramebuffers(1, &m_convertFBO);
    RenderTexture::FreeAll();
    return true;
}

//-----------------------------------------------------------------------------
bool Effects::ConvertYUV2RGB(GLuint sourceTextureId)
//-----------------------------------------------------------------------------
{
    if (0 != m_isTracingEnabled)
    {
        atrace_begin(ATRACE_TAG_ALWAYS, "bokeh_yuv2rgb");
    }
    glUseProgram(m_prgConvertYUV2RGB);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, sourceTextureId);
    glBindFramebuffer(GL_FRAMEBUFFER, m_convertFBO);

    glViewport(0, 0, m_width, m_height);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, fullscreen_vertices);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, texcoordAttribsConvert);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    if (0 != m_isTracingEnabled)
    {
        atrace_end(ATRACE_TAG_ALWAYS);
    }
    return true;
}

//-----------------------------------------------------------------------------
bool Effects::ConvertRGB2YUV(GLuint inputId, GLuint outputId)
//-----------------------------------------------------------------------------
{
    if (0 != m_isTracingEnabled)
    {
        atrace_begin(ATRACE_TAG_ALWAYS, "bokeh_rgb2yuv");
    }
    glUseProgram(m_prgConvertRGB2YUV);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, inputId);
    glBindFramebuffer(GL_FRAMEBUFFER, outputId);
    glViewport(0, 0, m_width, m_height);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, fullscreen_vertices2);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, texcoordAttribsConvert2);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    if (0 != m_isTracingEnabled)
    {
        atrace_end(ATRACE_TAG_ALWAYS);
    }
    return true;
}

// Downscales the provided texture by specified factor
//-----------------------------------------------------------------------------
RenderTexture* Effects::Downscale(RenderTexture* srcTexture, int factor)
//-----------------------------------------------------------------------------
{
    if (0 != m_isTracingEnabled)
    {
        atrace_begin(ATRACE_TAG_ALWAYS, "bokeh_downscale");
    }
    if (srcTexture == NULL)
    {
      IDBG_ERROR("Invalid NULL input to Effects::Downscale");
      return NULL;
    }
    RenderTexture* downScaled = RenderTexture::GetTemporary(srcTexture->Width()/factor,
                                                            srcTexture->Height()/factor,
                                                            srcTexture->Format());
    if (downScaled == NULL)
    {
      IDBG_ERROR("Invalid return value from RenderTexture::GetTemporary");
      return NULL;
    }
    glUseProgram(m_prgPassthru);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, srcTexture->texture);
    glBindFramebuffer(GL_FRAMEBUFFER, downScaled->fb);
    glViewport(0, 0, downScaled->Width(), downScaled->Height());

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, fullscreen_vertices);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, fullscreenTexcoords);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    if (0 != m_isTracingEnabled)
    {
        atrace_end(ATRACE_TAG_ALWAYS);
    }
    return downScaled;
}

//-----------------------------------------------------------------------------
RenderTexture* Effects::Blit(RenderTexture* srcTexture)
//-----------------------------------------------------------------------------
{
    return Downscale(srcTexture, 1);
}

//-----------------------------------------------------------------------------
RenderTexture* Effects::Blur(int srcTexture, int w, int h, int f)
//-----------------------------------------------------------------------------
{
    if (0 != m_isTracingEnabled)
    {
        atrace_begin(ATRACE_TAG_ALWAYS, "bokeh_blur");
    }
    RenderTexture* horz = 0;
    {
        horz = RenderTexture::GetTemporary(w, h, f);
        if (horz == NULL)
        {
          IDBG_ERROR("Invalid return value from RenderTexture::GetTemporary");
          return NULL;
        }
        glUseProgram(m_prgGaussianBlur);
        float _Direction[4] = {1, 0, 0, 0};
        float _Size[4] = {0.55802f/w, 0.55802f/h, 0, 0};
        glUniform4f(1, _Direction[0], _Direction[1], _Direction[2], _Direction[3]);
        glUniform4f(2, _Size[0], _Size[1], _Size[2], _Size[3]);
        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, srcTexture);
        glBindFramebuffer(GL_FRAMEBUFFER, horz->fb);
        glViewport(0, 0, horz->Width(), horz->Height());

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, fullscreen_vertices);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, fullscreenTexcoords);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    RenderTexture* vert = 0;
    {
        vert = RenderTexture::GetTemporary(w, h, f);
        if (vert == NULL)
        {
          IDBG_ERROR("Invalid return value from RenderTexture::GetTemporary");
          return NULL;
        }
        glUseProgram(m_prgGaussianBlur);
        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, horz->texture);
        float _Direction[4] = {0, 1, 0, 0};
        float _Size[4] = {0.55802f/horz->Width(), 0.55802f/horz->Height(), 0, 0};
        glUniform4f(1, _Direction[0], _Direction[1], _Direction[2], _Direction[3]);
        glUniform4f(2, _Size[0], _Size[1], _Size[2], _Size[3]);
        glBindFramebuffer(GL_FRAMEBUFFER, vert->fb);
        glViewport(0, 0, vert->Width(), vert->Height());

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, fullscreen_vertices);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, fullscreenTexcoords);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    if (0 != m_isTracingEnabled)
    {
        atrace_end(ATRACE_TAG_ALWAYS);
    }
    return vert;
}

//-----------------------------------------------------------------------------
RenderTexture* Effects::Blend(float low,
                     float high,
                     RenderTexture* color,
                     RenderTexture* depth,
                     RenderTexture* blended)
//-----------------------------------------------------------------------------
{
    if (0 != m_isTracingEnabled)
    {
        atrace_begin(ATRACE_TAG_ALWAYS, "bokeh_blend");
    }
    if (blended == NULL)
    {
      IDBG_ERROR("Invalid input value to Effects::Blend");
      return NULL;
    }
    RenderTexture* result = RenderTexture::GetTemporary(color->Width(),
                                                        color->Height(),
                                                        color->Format());
    if (result == NULL)
    {
     IDBG_ERROR("Invalid return value from RenderTexture::GetTemporary");
      return NULL;
    }
    glUseProgram(m_prgBlendLevels);
    float _Direction[4] = {low, high, 0, 0};
    glUniform4f(1, _Direction[0], _Direction[1], _Direction[2], _Direction[3]);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, color->texture);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, depth->texture);
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, blended->texture);
    glBindFramebuffer(GL_FRAMEBUFFER, result->fb);
    glViewport(0, 0, result->Width(), result->Height());

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, fullscreen_vertices);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, fullscreenTexcoords);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    if (0 != m_isTracingEnabled)
    {
        atrace_end(ATRACE_TAG_ALWAYS);
    }
    return result;
}

//-----------------------------------------------------------------------------
RenderTexture* Effects::Bokeh(int b,
                    float* blurRadiusLevels,
                    RenderTexture* color,
                    RenderTexture* depth,
                    RenderTexture* blended)
//-----------------------------------------------------------------------------
{
    RenderTexture* result = RenderTexture::GetTemporary(color->Width(),
                                                        color->Height(),
                                                        color->Format());
    if (result == NULL)
    {
      IDBG_ERROR("Invalid return value from RenderTexture::GetTemporary");
      return NULL;
    }
    glUseProgram(m_prgBlendLevels);
    float _Direction[4] = {blurRadiusLevels[b], blurRadiusLevels[b+1], 0, 0};
    glUniform4f(1, _Direction[0], _Direction[1], _Direction[2], _Direction[3]);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, color->texture);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, depth->texture);
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, blended->texture);
    glBindFramebuffer(GL_FRAMEBUFFER, result->fb);
    glViewport(0, 0, result->Width(), result->Height());

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, fullscreen_vertices);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, fullscreenTexcoords);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    return result;
}

//-----------------------------------------------------------------------------
RenderTexture* Effects::Blur(RenderTexture* srcTexture)
//-----------------------------------------------------------------------------
{
    if (srcTexture == NULL)
    {
      IDBG_ERROR("Invalid NULL input to Effects::Blur");
      return NULL;
    }
    RenderTexture* dst = Blur(srcTexture->texture,
                              srcTexture->Width(),
                              srcTexture->Height(),
                              srcTexture->Format());

    return dst;
}


//-----------------------------------------------------------------------------
RenderTexture* Effects::BlurAndDownscale(RenderTexture* srcTexture)
//-----------------------------------------------------------------------------
{
    RenderTexture* dst = BlurAndDownscale(srcTexture->texture, srcTexture->Width(), srcTexture->Height(), srcTexture->Format());

    return dst;
}

//-----------------------------------------------------------------------------
void Effects::Blit(RenderTexture* src, int* viewport)
//-----------------------------------------------------------------------------
{
    glUseProgram(m_prgPassthru);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, src->texture);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, fullscreen_vertices);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, fullscreenTexcoords);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}
