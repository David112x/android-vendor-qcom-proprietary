/***************************************************************************
* Copyright (c) 2017 Qualcomm Technologies, Inc.                           *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
***************************************************************************/

const char* vertex_shader = ""
    "#version 300 es                                                       \n"
    "precision highp float;                                                \n"
    "layout(location = 0) in vec2 in_pos;                                  \n"
    "layout(location = 1) in vec2 in_uv;                                   \n"
    "                                                                      \n"
    "out vec2 uv;                                                          \n"
    "                                                                      \n"
    "void main()                                                           \n"
    "{                                                                     \n"
    "    gl_Position = vec4(in_pos, 0.0, 1.0);                             \n"
    "    uv = in_uv;                                                       \n"
    "}                                                                     \n";
