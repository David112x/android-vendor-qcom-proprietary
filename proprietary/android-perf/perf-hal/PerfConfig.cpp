/******************************************************************************
  @file    PerfConfig.cpp
  @brief   Android performance HAL module

  DESCRIPTION

  ---------------------------------------------------------------------------

  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/

#define PERF_CONFIG_STORE_ROOT "PerfConfigsStore"
#define PERF_CONFIG_STORE_CHILD "PerfConfigs"
#define PERF_CONFIG_STORE_PROP "Prop"
#define PERF_CONFIG_STORE_NAME "Name"
#define PERF_CONFIG_STORE_VALUE "Value"
#define PERF_CONFIG_STORE_TARGET "Target"
#define PERF_CONFIG_STORE_KERNEL "Kernel"
#define PERF_CONFIG_STORE_XML "vendor/etc/perf/perfconfigstore.xml"
#define KERNEL_VERSION_NODE  "/proc/sys/kernel/osrelease"

#include "PerfConfig.h"
#include <unistd.h>
#include <cutils/properties.h>
#include "MpctlUtils.h"

using namespace std;

char *PerfConfigDataStore::mKernelVersion = NULL;
PerfConfigDataStore PerfConfigDataStore::mPerfStorage;

PerfConfigDataStore::PerfConfigDataStore() {
}

PerfConfigDataStore::~PerfConfigDataStore() {
    //delete mappings
    while (!mPerfConfigStore.empty()) {
        PerfConfigStoreInfo *tmp = mPerfConfigStore.back();
        if (tmp) {
            delete tmp;
        }
        mPerfConfigStore.pop_back();
    }

}

PerfConfigDataStore::PerfConfigStoreInfo::PerfConfigStoreInfo(char *prop_name, char *prop_value) {
    strlcpy(mName, prop_name, sizeof(mName));
    strlcpy(mValue, prop_value, sizeof(mValue));
}

void PerfConfigDataStore::readKernelVersion(){
    int fd = -1, n;
    const int MAX_BUF_SIZE = 16;
    char buf[MAX_BUF_SIZE];
    mKernelVersion = new char[MAX_BUF_SIZE];
    fd = open(KERNEL_VERSION_NODE, O_RDONLY);
    if (fd != -1) {
        memset(buf, 0x0, sizeof(buf));
        n = read(fd, buf, MAX_BUF_SIZE-1);
        if (n > 0) {
            if (mKernelVersion != NULL){
                strlcpy(mKernelVersion, buf, n);}
            else
                ALOGE("Couldn't allocate memory for mKernelVer");
       } else
            ALOGE("Kernel Version node not present");
    }
    close(fd);
}

//perf config store xml CB
void PerfConfigDataStore::PerfConfigStoreCB(xmlNodePtr node, void *) {
    char *mName = NULL, *mValue = NULL, *mTarget = NULL, *mKernel = NULL;
    PerfConfigDataStore &store = PerfConfigDataStore::getPerfDataStore();
    bool valid_target = false, valid_kernel = false;
    char trace_prop[PROPERTY_VALUE_MAX];

    /* Enable traces by adding vendor.debug.trace.perf=1 into build.prop */
    if (property_get(PROP_NAME, trace_prop, NULL) > 0) {
        if (trace_prop[0] == '1') {
            perf_debug_output = PERF_SYSTRACE = atoi(trace_prop);
        }
    }

    if(!xmlStrcmp(node->name, BAD_CAST PERF_CONFIG_STORE_PROP)) {
        if(xmlHasProp(node, BAD_CAST PERF_CONFIG_STORE_NAME)) {
              mName = (char *) xmlGetProp(node, BAD_CAST PERF_CONFIG_STORE_NAME);
        } else if (perf_debug_output) {
              ALOGI("Property not found Name=%s", mName ? mName : "NULL");
        }

        if(xmlHasProp(node, BAD_CAST PERF_CONFIG_STORE_VALUE)) {
              mValue = (char *) xmlGetProp(node, BAD_CAST PERF_CONFIG_STORE_VALUE);
        } else if (perf_debug_output) {
              ALOGI("Property not found Name=%s", mValue ? : "NULL");
        }
        if(xmlHasProp(node, BAD_CAST PERF_CONFIG_STORE_TARGET)) {
               mTarget = (char *) xmlGetProp(node, BAD_CAST PERF_CONFIG_STORE_TARGET);
               if (TARGET_NAME != NULL and mTarget != NULL)
                   valid_target = (!strncmp(TARGET_NAME, mTarget, strlen(mTarget)))? true : false;
        }
        if(xmlHasProp(node, BAD_CAST PERF_CONFIG_STORE_KERNEL)) {
              mKernel = (char *) xmlGetProp(node, BAD_CAST PERF_CONFIG_STORE_KERNEL);
              if (mKernelVersion != NULL and mKernel != NULL)
                  valid_kernel = (!strncmp(mKernelVersion, mKernel, strlen(mKernel)))? true : false;
        }

       /* Enable this log by adding vendor.debug.trace.perf=1 into build.prop */
        if (perf_debug_output) {
            ALOGI("Identified Name=%s Value=%s for PerfConfigStore in table", mName ? mName : "NULL", mValue ? mValue : "NULL");
        }

        if (mName != NULL and mValue != NULL) {
            if ((valid_target and (mKernel == NULL)) or
                (valid_kernel and (mTarget == NULL)) or
                (valid_kernel and valid_target)) {
                   UpdatePerfConfig(mName, mValue);
            } else if(!mTarget and !mKernel) {
                        store.mPerfConfigStore.push_back(new PerfConfigStoreInfo(mName, mValue));
            }
        }

        if(mName)
             xmlFree(mName);
        if(mValue)
             xmlFree(mValue);
        if(mTarget)
             xmlFree(mTarget);
        if(mKernel)
             xmlFree(mKernel);
    }
    return;
}

void PerfConfigDataStore::UpdatePerfConfig(char *name, char *value) {
    PerfConfigDataStore &store = PerfConfigDataStore::getPerfDataStore();
    vector<PerfConfigStoreInfo*>::iterator itbegin = store.mPerfConfigStore.begin();
    vector<PerfConfigStoreInfo*>::iterator itend = store.mPerfConfigStore.end();
    for (vector<PerfConfigStoreInfo*>::iterator it = itbegin; it != itend; ++it){
         if ((*it != NULL) and (!strncmp(name, (*it)->mName, strlen((*it)->mName)))) {
              strlcpy((*it)->mValue, value, PROP_VAL_LENGTH);
              return;
         }
    }
    store.mPerfConfigStore.push_back(new PerfConfigStoreInfo(name, value));
}

void PerfConfigDataStore::ConfigStoreInit() {
    readKernelVersion();
    int idnum;
    AppsListXmlParser *xmlParser = new AppsListXmlParser();
    if (NULL == xmlParser) {
        return;
    }
    //perf cofig store
    const string fPerfConfigStore(PERF_CONFIG_STORE_XML);
    const string xmlPerfConfigRoot(PERF_CONFIG_STORE_ROOT);
    const string xmlPerfConfigChild(PERF_CONFIG_STORE_CHILD);

    idnum = xmlParser->Register(xmlPerfConfigRoot, xmlPerfConfigChild, PerfConfigStoreCB, NULL);
    xmlParser->Parse(fPerfConfigStore);
    xmlParser->DeRegister(idnum);

    delete xmlParser;
    return;
}

char* PerfConfigDataStore::GetProperty(const char *name, char *value, const int value_length) {
    if (NULL == name or NULL == value) {
        if (perf_debug_output)
            ALOGI("Couldn't return property, no space");
        return NULL;
    }
    bool prop_found = false;
    vector<PerfConfigStoreInfo*>::iterator itbegin = mPerfConfigStore.begin();
    vector<PerfConfigStoreInfo*>::iterator itend = mPerfConfigStore.end();

    for (vector<PerfConfigStoreInfo*>::iterator it = itbegin; it != itend; ++it){
        if ((*it != NULL) and !strncmp(name, (*it)->mName, strlen(name))) {
            strlcpy(value, (*it)->mValue, value_length);
            prop_found = true;
            break;
        }
    }
    if (!prop_found) {
        if (perf_debug_output)
            ALOGI("Property %s not found", name);
        return NULL;
    }
    return value;
}
