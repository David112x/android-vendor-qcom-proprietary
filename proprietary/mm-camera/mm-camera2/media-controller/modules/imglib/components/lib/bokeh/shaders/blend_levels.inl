/***************************************************************************
* Copyright (c) 2017 Qualcomm Technologies, Inc.                           *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
***************************************************************************/

const char* blend_levels_shader = ""
    "#version 300 es                                                       \n"
    "#extension GL_OES_EGL_image_external_essl3 : require                  \n"
    "precision highp float;                                                \n"
    "layout(binding = 0) uniform sampler2D _MainTex;                       \n"
    "layout(binding = 1) uniform sampler2D _BlurRadiusMap;                 \n"
    "layout(binding = 2) uniform sampler2D _BlendedTex;                    \n"
    "layout(location = 1) uniform vec4 _BlurDistance;                      \n"
    "in vec2 uv;                                                           \n"
    "out vec4 fs_color;                                                    \n"
    "void main()                                                           \n"
    "{                                                                     \n"
    "    vec4 colorMap = texture(_MainTex, uv);                            \n"
    "    vec4 blurRadiusMap = texture(_BlurRadiusMap, uv);                 \n"
    "    vec4 blendedTex = texture(_BlendedTex, uv);                       \n"
    "    float blendFactor = clamp((blurRadiusMap.x*255.0f - _BlurDistance.x) / (_BlurDistance.y - _BlurDistance.x), 0.0f, 1.0f);\n"
    "    fs_color = mix(colorMap, blendedTex, blendFactor);                \n"
    "}                                                                     \n";
