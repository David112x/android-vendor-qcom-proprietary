/***************************************************************************
* Copyright (c) 2017 Qualcomm Technologies, Inc.                           *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
***************************************************************************/

#include "RenderTexture.h"
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglextQCOM.h>

#include <android/log.h>

RenderTexture* RenderTexture::listOfTemporary[64] = {0};
bool RenderTexture::IsInUse[64] = {0};

//-----------------------------------------------------------------------------
RenderTexture::RenderTexture()
//-----------------------------------------------------------------------------
{
    fb = -1;
    texture = -1;
    format = -1;
}

//-----------------------------------------------------------------------------
RenderTexture* RenderTexture::Create(int w, int h, int f)
//-----------------------------------------------------------------------------
{
    GLint internalFormat;
    GLenum glformat;
    GLenum type;

    width = w;
    height = h;
    format = f;

    switch(format)
    {
        case 0:
            internalFormat = GL_R8;
            glformat = GL_RED;
            type = GL_UNSIGNED_BYTE;
            break;
        case 1:
        default:
            internalFormat = GL_RGB8;
            glformat = GL_RGB;
            type = GL_UNSIGNED_BYTE;
            break;
    }

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 internalFormat,
                 width,
                 height,
                 0,
                 glformat,
                 type,
                 NULL);

    glGenFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           texture,
                           0);
    glCheckFramebufferStatus(GL_FRAMEBUFFER);

    return this;
}

//-----------------------------------------------------------------------------
RenderTexture* RenderTexture::Wrap(Texture* texture)
//-----------------------------------------------------------------------------
{
    RenderTexture* rt = GetTemporary(texture->width,
                                     texture->height,
                                     texture->format);
    if (rt == NULL)
    {
      IDBG_ERROR("Invalid value from GetTemporary");
      return NULL;
    }

    if (rt->texture != texture->tex)
    {
      glDeleteTextures(1, &rt->texture);
    }

    if (rt->fb > 0)
    {
      glDeleteFramebuffers(1, &rt->fb);
    }

    rt->width = texture->width;
    rt->height= texture->height;
    rt->format = texture->format;
    rt->texture = texture->tex;
    rt->fb = 0;

    return rt;
}

//-----------------------------------------------------------------------------
RenderTexture* RenderTexture::Wrap(int id, int w, int h, int f)
//-----------------------------------------------------------------------------
{
    RenderTexture* rt = new RenderTexture();
    rt->width = w;
    rt->height= h;
    rt->texture = id;
    rt->format = f;
    rt->fb = 0;
    return rt;
}

//-----------------------------------------------------------------------------
RenderTexture::~RenderTexture()
//-----------------------------------------------------------------------------
{
    width = -1;
    height = -1;
    format = -1;
    if (texture > 0)
    {
        glDeleteTextures(1, &texture);
    }
    if (fb > 0)
    {
        glDeleteFramebuffers(1, &fb);
    }
}

//-----------------------------------------------------------------------------
int RenderTexture::Width()
//-----------------------------------------------------------------------------
{
    return width;
}

//-----------------------------------------------------------------------------
int RenderTexture::Height()
//-----------------------------------------------------------------------------
{
    return height;
}

//-----------------------------------------------------------------------------
int RenderTexture::Format()
//-----------------------------------------------------------------------------
{
    return format;
}

//-----------------------------------------------------------------------------
RenderTexture* RenderTexture::GetFree(int w, int h, int f)
//-----------------------------------------------------------------------------
{
    RenderTexture* rt = 0;
    for(int i=0;i<64;i++)
    {
        RenderTexture* rtl = listOfTemporary[i];
        if( (rtl != 0) && (IsInUse[i] == false) )
        {
            if( (rtl->width == w) && (rtl->height == h) && (rtl->format == f) )
            {
                rt = rtl;
                IsInUse[i] = true;
                break;
            }
        }

    }

    return rt;
}

//-----------------------------------------------------------------------------
int RenderTexture::GetFreeEntry()
//-----------------------------------------------------------------------------
{
    int index = -1;
    for(int i=0;i<64;i++)
    {
        if( (IsInUse[i] == false) && listOfTemporary[i] == 0)
        {
            index = i;
            break;
        }
    }

    return index;
}

//-----------------------------------------------------------------------------
void RenderTexture::SetEntry(int index, RenderTexture* rt)
//-----------------------------------------------------------------------------
{
    if( listOfTemporary[index] != 0 )
    {
        delete listOfTemporary[index];
    }
    listOfTemporary[index] = rt;
    IsInUse[index] = true;
}

//-----------------------------------------------------------------------------
int RenderTexture::FindEntry(RenderTexture* rt)
//-----------------------------------------------------------------------------
{
    int index = -1;
    for (int i=0;i<64;i++)
    {
        if( rt == listOfTemporary[i] )
        {
            index = i;
        }
    }

    return index;
}

//-----------------------------------------------------------------------------
RenderTexture* RenderTexture::GetTemporary(int w, int h, int f)
//-----------------------------------------------------------------------------
{
    RenderTexture* rt = GetFree(w, h, f);
    if( rt == 0 )
    {
        int index = GetFreeEntry();
        if( index >= 0 )
        {
            rt = new RenderTexture();
            rt->Create(w, h, f);
            SetEntry(index, rt);
        }
    }

    return rt;
}

//-----------------------------------------------------------------------------
void RenderTexture::ReleaseTemporary(RenderTexture* rt)
//-----------------------------------------------------------------------------
{
    int index = FindEntry(rt);
    if( index >= 0 )
    {
        IsInUse[index] = false;
    }
}

//-----------------------------------------------------------------------------
void RenderTexture::ReleaseAll()
//-----------------------------------------------------------------------------
{
    for(int i=0;i<64;i++)
    {
        IsInUse[i] = false;
    }
}

//-----------------------------------------------------------------------------
void RenderTexture::FreeAll()
//-----------------------------------------------------------------------------
{
    for(int i=0;i<64;i++)
    {
        IsInUse[i] = false;
        delete listOfTemporary[i];
        listOfTemporary[i] = 0;
    }
}
