/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "qahw_module.h"

namespace qti_hal_test
{
qahw_module_handle_t * QAHWModule::qahw_module_handle_{ nullptr };

QAHWModule::QAHWModule()
{
    LOG_ENTRY;
    std::lock_guard<std::mutex> guard(module_lock_);
    if (qahw_module_handle_ == nullptr)
    {
        LOG_API("qahw_load_module");
        qahw_module_handle_ = qahw_load_module(QAHW_MODULE_ID_PRIMARY);

        if (!qahw_module_handle_)
        {
            throw std::runtime_error{ "qahw_load_module failed" };
        }
    }
}

QAHWModule::~QAHWModule()
{
    LOG_ENTRY;
    std::lock_guard<std::mutex> guard(module_lock_);
    if (qahw_module_handle_ != nullptr)
    {
        LOG_API("qahw_unload_module");
        int status = qahw_unload_module(qahw_module_handle_); // TO DO: Cannot throw on unload error from here.
        if (status != 0)
        {
            // TO DO: Log error, no throw.
        }

        qahw_module_handle_ = nullptr;
    }
}

std::shared_ptr<qahw_module_handle_t *> QAHWModule::GetHandle()
{
    LOG_ENTRY;

    assert(qahw_module_handle_);

    return std::make_shared<qahw_module_handle_t *>(qahw_module_handle_);
}
}