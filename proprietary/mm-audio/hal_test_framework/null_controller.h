/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef NULL_CONTROLLER_H_
#define NULL_CONTROLLER_H_

#include "controller.h"
#include "logger.h"

namespace qti_hal_test
{
class NullController : public Controller
{
public:
    NullController();

    ~NullController();

    void operator()(std::exception_ptr & controller_exception) override;
};
}

#endif // NULL_CONTROLLER_H_