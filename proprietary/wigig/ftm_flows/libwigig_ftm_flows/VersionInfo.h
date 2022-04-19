/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#pragma once

#include <stdio.h>
#include <string.h>

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

// The only place for version definition/editing
#define TOOL_VERSION_MAJOR  1
#define TOOL_VERSION_MINOR  0
#define TOOL_VERSION_MAINT  0
#define TOOL_VERSION_INTERM 0

#define TOOL_VERSION            TOOL_VERSION_MAJOR,TOOL_VERSION_MINOR,TOOL_VERSION_MAINT,TOOL_VERSION_INTERM
#define TOOL_VERSION_STR        STR(TOOL_VERSION_MAJOR.TOOL_VERSION_MINOR.TOOL_VERSION_MAINT.TOOL_VERSION_INTERM)

inline const char* get_ftm_flows_version()
{
    return TOOL_VERSION_STR;
}

inline const char* get_ftm_flows_build_info(char* buffer, size_t size)
{
    int total_count = snprintf(buffer, size, "WIGIG FTM Flows Library Build Info\nVersion:          %s\n", TOOL_VERSION_STR);
#ifdef BUILD_ID
    if (strlen(BUILD_ID) > 0)
    {
        total_count += snprintf(buffer + total_count, size - total_count, "Build ID:         %s\n", BUILD_ID);
    }
#endif
#ifdef BUILD_DATE
    total_count += snprintf(buffer + total_count, size - total_count, "Build Date:       %s\n", BUILD_DATE);
#endif
#ifdef LAST_CHANGE_ID
    if (strlen(LAST_CHANGE_ID) > 0)
    {
        total_count += snprintf(buffer + total_count, size - total_count, "Last Change ID:   %s\n", LAST_CHANGE_ID);
    }
#endif
#ifdef LAST_CHANGE_DATE
    if (strlen(LAST_CHANGE_DATE) > 0)
    {
        total_count += snprintf(buffer + total_count, size - total_count, "Last Change Date: %s\n", LAST_CHANGE_DATE);
    }
#endif
    (void)total_count; /* prevent unused variable warning */
    return buffer;
}
