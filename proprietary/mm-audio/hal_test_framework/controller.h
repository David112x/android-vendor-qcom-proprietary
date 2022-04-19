/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include <stdexcept>

#include "logger.h"

namespace qti_hal_test
{
class Controller
{
public:
    Controller();

    virtual ~Controller();

    virtual void operator()(std::exception_ptr & controller_exception) =0;
};
}

#endif // CONTROLLER_H_