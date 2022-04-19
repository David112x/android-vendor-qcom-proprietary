/*
 * Copyright (c) 2019, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "cutils/properties.h"
#include <string.h>
#include <string>
#include <stdlib.h>
#include <utils/String16.h>
#include <vendor/qti/hardware/capabilityconfigstore/1.0/ICapabilityConfigStore.h>
#include <vendor/qti/hardware/capabilityconfigstore/1.0/types.h>
#include <time.h>

void testCapConfigStoreService();
int createCapConfigStoreClient();
