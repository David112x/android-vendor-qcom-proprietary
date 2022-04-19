/***************************************************************************
* Copyright (c) 2017 Qualcomm Technologies, Inc.                           *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
***************************************************************************/

const char* convert_yuv_to_rgb_shader = ""
    "#version 300 es                                                       \n"
    "#extension GL_OES_EGL_image_external_essl3 : require                  \n"
    "precision highp float;                                                \n"
    "                                                                      \n"
    "layout(binding = 0) uniform samplerExternalOES u_sTexture;            \n"
    "                                                                      \n"
    "in vec2 uv;                                                           \n"
    "out vec3 color;                                                       \n"
    "                                                                      \n"
    "void main()                                                           \n"
    "{                                                                     \n"
    "    color = texture(u_sTexture, uv).rgb;                              \n"
    "}                                                                     \n";
