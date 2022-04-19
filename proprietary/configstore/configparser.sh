#!/bin/sh
#
# Copyright (c) 2019, Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.

HOST_OUT_EXECUTABLES="out/host/linux-x86/bin"
XMLPARSER="configstore_xmlparser"
./$HOST_OUT_EXECUTABLES/$XMLPARSER $1

if [ $? -eq 0 ]; then
    echo " PASSED "
else
    echo " FAILED "
fi

