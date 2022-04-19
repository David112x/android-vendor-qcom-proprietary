/***************************************************************************
* Copyright (c) 2017 Qualcomm Technologies, Inc.                           *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
***************************************************************************/

const char* passthru_shader = ""
"#version 300 es                                                           \n"
    "#extension GL_OES_EGL_image_external_essl3 : require                  \n"
    "precision highp float;                                                \n"
    "layout(binding = 0) uniform sampler2D mainTex;                        \n"
    "in vec2 uv;                                                           \n"
    "out vec4 fs_color;                                                    \n"
    "void main()                                                           \n"
    "{                                                                     \n"
    "    fs_color = texture(mainTex, uv);                                  \n"
    "}                                                                     \n";
