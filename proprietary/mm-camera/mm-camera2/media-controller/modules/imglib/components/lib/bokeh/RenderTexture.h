/***************************************************************************
* Copyright (c) 2017 Qualcomm Technologies, Inc.                           *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
***************************************************************************/

#ifndef __QCAMERA_RENDER_TEXTURE_H__
#define __QCAMERA_RENDER_TEXTURE_H__

#include "Texture.h"

class RenderTexture {
    static RenderTexture* listOfTemporary[64];
    static bool IsInUse[64];

    private:
        int width;
        int height;
        int format;
    public:
        unsigned int fb;
        unsigned int texture;

    public:
        RenderTexture();
        RenderTexture* Create(int w, int h, int format);
        ~RenderTexture();
        int Width();
        int Height();
        int Format();
        static RenderTexture* Wrap(Texture* texture);
        static RenderTexture* Wrap(int, int, int, int);

    public:
        static RenderTexture* GetTemporary(int w, int h, int f);
        static void ReleaseTemporary(RenderTexture*);
        static RenderTexture* GetFree(int w, int h, int f);
        static int GetFreeEntry();
        static void SetEntry(int index, RenderTexture* rt);
        static int FindEntry(RenderTexture*);
        static void ReleaseAll();
        static void FreeAll();
};

#endif
