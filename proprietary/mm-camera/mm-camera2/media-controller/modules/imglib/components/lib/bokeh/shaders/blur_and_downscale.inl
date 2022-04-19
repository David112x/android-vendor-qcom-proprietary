/***************************************************************************
* Copyright (c) 2017 Qualcomm Technologies, Inc.                           *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
***************************************************************************/

const char* blur_and_downscale_shader = ""
    "#version 300 es                                                            \n"
    "#extension GL_OES_EGL_image_external_essl3 : require                       \n"
    "precision highp float;                                                     \n"
    "layout(binding = 0) uniform sampler2D _MainTex;                            \n"
    "layout(location = 1) uniform vec4 _Size;                                   \n"
    "in vec2 uv;                                                                \n"
    "out vec4 fs_color;                                                         \n"
    "                                                                           \n"
    "void main()                                                                \n"
    "{                                                                          \n"
    "   vec4 aa = texture(_MainTex, (uv + vec2(-0.75f, -0.75f) * _Size.xy));    \n"
    "   vec4 cc = texture(_MainTex, (uv + vec2(+0.75f, -0.75f) * _Size.xy));    \n"
    "   vec4 ii = texture(_MainTex, (uv + vec2(-0.75f, +0.75f) * _Size.xy));    \n"
    "   vec4 kk = texture(_MainTex, (uv + vec2(+0.75f, +0.75f) * _Size.xy));    \n"
    "   fs_color = 0.25f * (aa + cc +  ii + kk);                                \n"
    "}                                                                          \n";
