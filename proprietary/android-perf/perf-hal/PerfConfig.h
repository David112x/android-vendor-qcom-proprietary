/******************************************************************************
  @file    PerfConfig.h
  @brief   Android performance HAL module

  DESCRIPTION

  ---------------------------------------------------------------------------

  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/

#include <vector>
#include <log/log.h>
#include "mp-ctl.h"
#include "XmlParser.h"

using namespace std;
class PerfConfigDataStore {
private:
    /*Perf Cofig Store Info class*/
    class PerfConfigStoreInfo {
    public:
        explicit PerfConfigStoreInfo(char *prop_name, char *prop_value);
        char mName[PROP_VAL_LENGTH];
        char mValue[PROP_VAL_LENGTH];
    };

private:
    //perf config store
    vector<PerfConfigStoreInfo*> mPerfConfigStore;
    // Singelton object of this class
    static PerfConfigDataStore mPerfStorage;
    //store Kernel Version
    static char *mKernelVersion;
public:
    static PerfConfigDataStore &getPerfDataStore() {
        return mPerfStorage;
    }

private:
    //perf config store xml CB
    static void PerfConfigStoreCB(xmlNodePtr node, void *);
    //ctor, copy ctor, assignment overloading
    PerfConfigDataStore();
    PerfConfigDataStore(PerfConfigDataStore const& oh);
    PerfConfigDataStore& operator=(PerfConfigDataStore const& oh);
    //read Kernel Version of the Target
    void readKernelVersion();
    //Update property value based on target/kernel
    static void UpdatePerfConfig(char *name, char *value);
public:
    char *getKernelVersion() {
         return mKernelVersion;
    }
    char* GetProperty(const char *name, char *value, const int value_length);
    void ConfigStoreInit();
    ~PerfConfigDataStore();
};
