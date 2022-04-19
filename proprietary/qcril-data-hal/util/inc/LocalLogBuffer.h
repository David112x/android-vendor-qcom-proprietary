/*
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef _QCRIL_DATA_LOG_BUFFER_H_
#define _QCRIL_DATA_LOG_BUFFER_H_

/*============================================================================
 FILE:         LocalLogBuffer.h

 OVERVIEW:     Log rildata information

 DEPENDENCIES: Logging, C++ STL
 ============================================================================*/

/* External Includes */
#include <deque>
#include <vector>
#include <string>
#include <mutex>
#include <sys/types.h>

namespace rildata {

/* Namespace pollution avoidance */
using ::std::deque;
using ::std::string;
using ::std::vector;

class LocalLogBuffer {
public:
    LocalLogBuffer(string name, int maxLogs);
    void addLog(string /* log */);
    void addLogWithTimestamp(string /* log */);
    void toLogcat();
    static void toFd(string msg, int fd);
    string getName();
    void setName(string /* name */);
    vector<string> getLogs();
private:
    deque<string> mLogs;
    string mName;
    const size_t mMaxLogs;
    std::mutex logMutex;
}; /* LocalLogBuffer */

}
#endif /* _QCRIL_DATA_LOG_BUFFER_H_ */
