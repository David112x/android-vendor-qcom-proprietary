/***************************************************************************
* Copyright (c) 2017 Qualcomm Technologies, Inc.                           *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
***************************************************************************/

#ifndef __QCAMERA_TEXTURE_H__
#define __QCAMERA_TEXTURE_H__

class Texture {
    public:
        unsigned int tex;
        unsigned int width;
        unsigned int height;
        int format;

    public:
        Texture(int id, int w, int h, int f)
        {
            tex = id;
            width = w;
            height = h;
            format = f;
        }
};

#endif
