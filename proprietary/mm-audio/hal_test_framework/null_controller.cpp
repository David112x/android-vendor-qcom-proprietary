
/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "null_controller.h"

namespace qti_hal_test
{
NullController::NullController()
{
    LOG_ENTRY;
}

NullController::~NullController()
{
    LOG_ENTRY;
}

void NullController::operator()(std::exception_ptr & controller_exception)
{
    LOG_ENTRY;
}
}
