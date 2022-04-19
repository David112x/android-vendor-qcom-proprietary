/***************************************************************************
* Copyright (c) 2017 Qualcomm Technologies, Inc.                           *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
***************************************************************************/

const char* convert_rgb_to_yuv_shader = ""
    "#version 300 es                                                       \n"
    "#extension GL_EXT_YUV_target : require                                \n"
    "precision highp float;                                                \n"
    "                                                                      \n"
    "layout(binding = 0) uniform sampler2D u_sTexture;                     \n"
    "                                                                      \n"
    "in vec2 uv;                                                           \n"
    "layout (yuv) out vec4 color;                                          \n"
    "                                                                      \n"
    "void main()                                                           \n"
    "{                                                                     \n"
    "    vec3 rgbColor = texture(u_sTexture, uv).rgb;                      \n"
    "    color = vec4(rgb_2_yuv(rgbColor, itu_601_full_range), 1.0);       \n"
    "}                                                                     \n";
