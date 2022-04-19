/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef LOGGER_H_
#define LOGGER_H_

#include <iomanip>
#include <iostream>
#include <thread>

#ifdef DISABLE_QAHW_API
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

//#define ENABLE_LOG_MACROS
#ifdef ENABLE_LOG_MACROS
#define LOG_ENTRY                                (std::cout << "          " << std::setw(3) << "   "       << "  " << std::hex << " thread_id " << std::setw(8) << std::this_thread::get_id() << " obj_addr: " << std::setw(10) << (this)       << " " << __PRETTY_FUNCTION__ << std::dec << std::endl)
#define LOG_STREAM_ENTRY(stream_id)              (std::cout << "stream_id " << std::setw(3) << (stream_id) << "  " << std::hex << " thread_id " << std::setw(8) << std::this_thread::get_id() << " obj_addr: " << std::setw(10) << (this)       << " " << __PRETTY_FUNCTION__ << std::dec << std::endl)
#define LOG_STREAM_API(stream_id, function_name) (std::cout << "stream_id " << std::setw(3) << (stream_id) << "  " << std::hex << " thread_id " << std::setw(8) << std::this_thread::get_id() << " obj_addr: " << std::setw(10) << (this)       << " " << "API: " << (function_name) << std::dec << std::endl)
#define LOG_API(function_name)                   (std::cout << "          " << std::setw(3) << "   "       << "  " << std::hex << " thread_id " << std::setw(8) << std::this_thread::get_id() << " obj_addr: " << std::setw(10) << (this)       << " " << "API: " << (function_name) << std::dec << std::endl)
#define LOG_FUNCTION                             (std::cout << "          " << std::setw(3) << "   "       << "  " << std::hex << " thread_id " << std::setw(8) << std::this_thread::get_id() << " obj_addr: " << std::setw(10) << "        NA" << " " << __PRETTY_FUNCTION__ << std::dec << std::endl)
#else
#define LOG_ENTRY
#define LOG_STREAM_ENTRY(stream_id)
#define LOG_STREAM_API(stream_id, function_name)
#define LOG_API(function_name)
#define LOG_FUNCTION
#endif

namespace qti_hal_test
{
class Logger
{
public:
    Logger();
    ~Logger();
};
}
#endif // LOGGER_H_