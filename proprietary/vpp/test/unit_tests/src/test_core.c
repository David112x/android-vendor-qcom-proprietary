/*!
 * @file test_core.c
 *
 * @cr
 * Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.

 * @services
 */

#include <sys/types.h>

#include "dvpTest.h"
#include "dvpTest_tb.h"

/************************************************************************
 * Local definitions
 ***********************************************************************/

/************************************************************************
 * Local static variables
 ***********************************************************************/
uint64_t u64LogLevelUnit = 0xffffffff;

/************************************************************************
 * Forward Declarations
 ************************************************************************/
#define ENABLE_SUITE_QUEUE      1
#define ENABLE_SUITE_BUF        1
#define ENABLE_SUITE_UC         1
#define ENABLE_SUITE_GPU        1
#define ENABLE_SUITE_PL         1
#define ENABLE_SUITE_VPP        1
#define ENABLE_SUITE_HVX        1
#define ENABLE_SUITE_ME         1
#define ENABLE_SUITE_MC         1
#define ENABLE_SUITE_IP         1
#define ENABLE_SUITE_STATS      1

#if ENABLE_SUITE_QUEUE
TEST_SUITE_DECLARE(QueueSuite);
#endif

#if ENABLE_SUITE_BUF
TEST_SUITE_DECLARE(BufSuite);
#endif

#if ENABLE_SUITE_UC
TEST_SUITE_DECLARE(UsecaseSuite);
#endif

#if ENABLE_SUITE_GPU
#ifdef VPP_TARGET_USES_GPU
TEST_SUITE_DECLARE(IpGpuSuite);
TEST_SUITE_DECLARE(IpGpuNoInitSuite);
#endif
#endif

#if ENABLE_SUITE_PL
TEST_SUITE_DECLARE(PipelineSuite);
#endif

#if ENABLE_SUITE_VPP
TEST_SUITE_DECLARE(VppSuite);
#endif

#if ENABLE_SUITE_HVX
TEST_SUITE_DECLARE(IpHvxSuite);
#endif

#if ENABLE_SUITE_ME
TEST_SUITE_DECLARE(IpMeSuite);
#endif

#if ENABLE_SUITE_MC
TEST_SUITE_DECLARE(IpMcSuite);
#endif

#if ENABLE_SUITE_IP
TEST_SUITE_DECLARE(IpSuite);
#endif

#if ENABLE_SUITE_STATS
TEST_SUITE_DECLARE(StatsSuite);
#endif

/************************************************************************
 * Local Functions
 ***********************************************************************/

/************************************************************************
 * Global Functions
 ***********************************************************************/
int main()
{
    // Initialize the test bench
    vDvpTb_Init();

    // Add the test suites to the framework
#if ENABLE_SUITE_QUEUE
    TEST_SUITE_INSTALL(QueueSuite);
#endif

#if ENABLE_SUITE_BUF
    TEST_SUITE_INSTALL(BufSuite);
#endif

#if ENABLE_SUITE_UC
    TEST_SUITE_INSTALL(UsecaseSuite);
#endif

#if ENABLE_SUITE_GPU
#ifdef VPP_TARGET_USES_GPU
    TEST_SUITE_INSTALL(IpGpuSuite);
    TEST_SUITE_INSTALL(IpGpuNoInitSuite);
#endif
#endif

#if ENABLE_SUITE_HVX
#ifdef VPP_TARGET_USES_HVX
    TEST_SUITE_INSTALL(IpHvxSuite);
#endif
#endif

#if ENABLE_SUITE_PL
    TEST_SUITE_INSTALL(PipelineSuite);
#endif

#if ENABLE_SUITE_VPP
    TEST_SUITE_INSTALL(VppSuite);
#endif

#if ENABLE_SUITE_ME
#ifdef VPP_TARGET_USES_FRC
    TEST_SUITE_INSTALL(IpMeSuite);
#endif
#endif

#if ENABLE_SUITE_MC
#ifdef VPP_TARGET_USES_FRC
    TEST_SUITE_INSTALL(IpMcSuite);
#endif
#endif

#if ENABLE_SUITE_IP
    TEST_SUITE_INSTALL(IpSuite);
#endif

#if ENABLE_SUITE_STATS
    TEST_SUITE_INSTALL(StatsSuite);
#endif

    // Start running the test bench
    vDvpTb_RunTests();

    // Terminate the test bench
    vDvpTb_Term();

    return 0;
}
