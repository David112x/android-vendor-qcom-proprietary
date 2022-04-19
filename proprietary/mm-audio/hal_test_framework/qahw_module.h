/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef  QAHW_MODULE_H_
#define  QAHW_MODULE_H_

#include <cassert>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>

#include "logger.h"
#include "platform_dependencies.h"

namespace qti_hal_test
{
class QAHWModule
{
public:
    QAHWModule();
    explicit QAHWModule(QAHWModule const & rhs) = delete; // TO DO: Understand move semantics for this class
    ~QAHWModule();

    std::shared_ptr<qahw_module_handle_t *> GetHandle();

private:
     static qahw_module_handle_t * qahw_module_handle_;
     std::mutex module_lock_;
};
}
#endif // QAHW_MODULE_H_