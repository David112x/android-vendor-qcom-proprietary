/******************************************************************************
  @file    atClient.h
  ---------------------------------------------------------------------------

  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/
#include <string>
#include <vector>
using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __ATCLIENT_H__
#define __ATCLIENT_H__

int perf_hint_qspm(int, const char *, int, int);
void tokenizePkgInfo(const char* name, string const &delim, vector<string> &result);

#endif

#ifdef __cplusplus
}
#endif
