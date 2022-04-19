/******************************************************************************
  @file    TargetInit.cpp
  @brief   Implementation of targets

  DESCRIPTION

  ---------------------------------------------------------------------------
  Copyright (c) 2011-2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/

#define LOG_TAG "ANDR-PERF-TARGET-INIT"
#include <cutils/log.h>
#include <fcntl.h>
#include <cstdlib>
#include <string.h>
#include <sys/utsname.h>

#include "Request.h"

#include "Target.h"
#include "OptsData.h"
#include "MpctlUtils.h"
#include "BoostConfigReader.h"

#define KPM_NUM_CLUSTERS        "/sys/module/msm_performance/parameters/num_clusters"
#define KPM_MANAGED_CPUS        "/sys/module/msm_performance/parameters/managed_cpus"
#define KPM_CPU_MAX_FREQ_NODE       "/sys/module/msm_performance/parameters/cpu_max_freq"

/* A single TargetInit function for all the targets which
   parses XML file for target configs.
 * For adding a new Target, create a targetconfig.xml file.
   And for any other the target specific initializations provided
   an ExtendedTargetInit fucntion which works on Socids.
*/

//Default frequency map for value mapping
int msmDefaultFreqMap[FREQ_MAP_MAX] = {
    800,    /*LOWEST_FREQ*/
    1100,   /*LEVEL1_FREQ*/
    1300,   /*LEVEL2_FREQ*/
    1500,   /*LEVEL3_FREQ*/
    1650    /*HIGHEST_FREQ*/
};

//Default pre-defined cluster map .
int msmDefaultPreDefinedClusterMap[MAX_PRE_DEFINE_CLUSTER - START_PRE_DEFINE_CLUSTER] = {
    0,  /*LAUNCH_CLUSTER*/
    1,  /*SCROLL_CLUSTER*/
    1,  /*ANIMATION_CLUSTER*/
};

/* Init Sequence
 * 1. Pouplate the target Config
 * 2. Init the target
 * 3. Populate the target resources config
 * 4. Init the resources node
 */

void Target::InitializeTarget() {
    int ret = 0;
    int socid = 0;
    char tmp_s[NODE_MAX];
    int rc = 0;
    int qindx = 0;
    char clk_scaling_s[NODE_MAX];
    int res = 0;
    ResourceInfo tmpr;

    QLOGI("Inside InitializeTarget");
    mSetAllResourceSupported();
    mInitAllResourceValueMap();
    mResetResourceSupported(MISC_MAJOR_OPCODE, UNSUPPORTED_OPCODE);
    mInitGpuAvailableFreq();
    mInitGpuBusAvailableFreq();

    socid = readSocID();

    //get resolution
    (void) readResolution();
    res = getResolution();

    mGovInstanceType = CLUSTER_BASED_GOV_INSTANCE;

    // Identifying storage device type. (UFS/EMMC)
    FREAD_STR(STORAGE_UFS_CLK_SCALING_DISABLE, clk_scaling_s, NODE_MAX, rc);
    if (rc > 0) {
        strlcpy(mStorageNode, STORAGE_UFS_CLK_SCALING_DISABLE,
                strlen(STORAGE_UFS_CLK_SCALING_DISABLE)+1);
    } else {
        strlcpy(mStorageNode, STORAGE_EMMC_CLK_SCALING_DISABLE,
                strlen(STORAGE_EMMC_CLK_SCALING_DISABLE)+1);
    }


    rc = 0;
    //populate boost conigs, target configs  & params mapping tables
    mPerfDataStore = PerfDataStore::getPerfDataStore();
    mPerfDataStore->Init();

    /* All the target related information parsed from XML file are initialized in the TargetInit()
    function which remains same for all the targets. For any target specific initializations provided
    an ExtendedTargetInit() function, which works on SocId. */

    TargetInit();
    ExtendedTargetInit();

    /* Init for per target resource file.
     * Moved the call after TargetInit as it need target name
     */
    mPerfDataStore->TargetResourcesInit();

    /*
     * Initialize kernel version
     */
    PerfConfigDataStore &mPerfConfig = PerfConfigDataStore::getPerfDataStore();
    char *KernelVer = mPerfConfig.getKernelVersion();
    mKernelVersion = -1;
    mKernelMajorVersion = -1;
    mSchedBoostConcurrencySupported = false;
    mSchedBoostNestingSupported = false;
    if (KernelVer != NULL) {
        rc = sscanf(KernelVer, "%d.%d.", &mKernelVersion,
                &mKernelMajorVersion);
        if (rc != 2) {
            mKernelVersion = -1;
            mKernelMajorVersion = -1;
            QLOGE("sscanf failed, kernel version set to -1");
        } else {
            if (min_kernel_version_with_sched_boost_concurrency_support ==
                    mKernelVersion) {
                mSchedBoostConcurrencySupported = true;
                mSchedBoostNestingSupported =
                    (min_kernel_major_version_with_sched_boost_nesting_support <=
                     mKernelMajorVersion);
            } else if (min_kernel_version_with_sched_boost_concurrency_support <
                    mKernelVersion) {
                mSchedBoostConcurrencySupported = true;
                mSchedBoostNestingSupported = true;
            }
            QLOGI("kernel ver is %d.%d, sched_boost Concurrency:%d, Nesting: %d",
                    mKernelVersion, mKernelMajorVersion,
                    mSchedBoostConcurrencySupported,
                    mSchedBoostNestingSupported);
        }
    }

    //cluster map from xml
    ret = mPerfDataStore->GetClusterMap(&mPredefineClusterMap, mTargetName);

    //default cluster map if not available
    if ((NULL == mPredefineClusterMap) || !ret) {
        mPredefineClusterMap = msmDefaultPreDefinedClusterMap;
    }

    tmpr.SetMajor(CPUFREQ_MAJOR_OPCODE);
    tmpr.SetMinor(CPUFREQ_MIN_FREQ_OPCODE);
    qindx = tmpr.DiscoverQueueIndex();

    mValueMap[qindx].mapSize = mPerfDataStore->GetFreqMap(res, &mValueMap[qindx].map, mTargetName);

    //default it to a map if no mappings exists
    if ((NULL == mValueMap[qindx].map) || !mValueMap[qindx].mapSize) {
        mValueMap[qindx].mapSize = FREQ_MAP_MAX;
        mValueMap[qindx].map = msmDefaultFreqMap;
    }

    //Define for max_freq. Freq mapped in Mhz.
    tmpr.SetMinor(CPUFREQ_MAX_FREQ_OPCODE);
    qindx = tmpr.DiscoverQueueIndex();

    mValueMap[qindx].mapSize = mPerfDataStore->GetFreqMap(res,
                                           &mValueMap[qindx].map, mTargetName);

    //default it to a map if no mappings exists
    if ((NULL == mValueMap[qindx].map) || !mValueMap[qindx].mapSize) {
        mValueMap[qindx].mapSize = FREQ_MAP_MAX;
        mValueMap[qindx].map = msmDefaultFreqMap;
    }
}

void invoke_wa_libs() {
#define KERNEL_WA_NODE "/sys/module/app_setting/parameters/lib_name"
   int rc;
   const char *wl_libs[] = {
      "libvoH264Dec_v7_OSMP.so",
      "libvossl_OSMP.so",
      "libTango.so"
   };
   int i;

   int len = sizeof (wl_libs) / sizeof (*wl_libs);

   for(i = 0; i < len; i++) {
      FWRITE_STR(KERNEL_WA_NODE, wl_libs[i], strlen(wl_libs[i]), rc);
      QLOGI("Writing to node (%s)  (%s) rc:%d\n", KERNEL_WA_NODE, wl_libs[i], rc);
   }

}

void invoke_tinyu_wa(int socid) {
  static struct {
       char* node;
       char* val;
  }tinyu_wa_detl [] = {
      {"/proc/sys/kernel/sched_lib_mask_check", "0x80"},
      {"/proc/sys/kernel/sched_lib_mask_force", "0xf0"},
      {"/proc/sys/kernel/sched_lib_name", "libunity.so, libfb.so"},
  };
  int rc;
  int len = sizeof (tinyu_wa_detl) / sizeof (tinyu_wa_detl[0]);

  /*for lito,atoll, we have only two gold cors, so mask is changed accrodingly*/
  if(socid == 400 || socid == 407) {
       tinyu_wa_detl[1].val = "0xc0";
  }

  for(int i = 0; i < len; i++) {
      FWRITE_STR(tinyu_wa_detl[i].node, tinyu_wa_detl[i].val, strlen(tinyu_wa_detl[i].val), rc);
      QLOGE("Writing to node (%s)  (%s) rc:%d\n", tinyu_wa_detl[i].node, tinyu_wa_detl[i].val, rc);
   }
}

void Target::TargetInit() {

    char tmp_s[NODE_MAX];
    int rc = 0;

    if (NULL == mPerfDataStore) {
        QLOGE("Initialization of PerfDataStore failed");
        return;
    }

    PerfDataStore::TargetConfigInfo *config = mPerfDataStore->getTargetConfig(mSocID);
    if (NULL == config) {
        QLOGE("Initialization of TargetConfigInfo Object failed");
        return;
    }

    if (!config->mUpdateTargetInfo) {
        QLOGE("Target Initialized with default available values, as mistake in target config XML file");
        return;
    }

    if(config->mCalculatedCores != config->mTotalNumCores) {
        QLOGE("Target Initialized with default values, as mismatch between the TotalNumCores and CalculatedCores.");
        return;
    }

    if (config->mTargetName) {
        mTargetName = new char[TARG_NAME_LEN];
        if (mTargetName) {
            strlcpy(mTargetName, config->mTargetName, TARG_NAME_LEN);
        }
    }
    QLOGI("Init %s",mTargetName ? mTargetName : "Target");

    mNumCluster = config->mNumCluster;
    mTotalNumCores = config->mTotalNumCores;
    mCorePerCluster = new int[mNumCluster];
    if (mCorePerCluster) {
        for (int i=0;i<mNumCluster;i++) {
            mCorePerCluster[i] = config->mCorepercluster[i];
            mCpuMaxFreqResetVal[i] = config->mCpumaxfrequency[i];
        }
    } else {
        QLOGE("Error: Could not initialize cores in cluster \n");
    }

    mSyncCore = config->mSyncCore;
    if (config->mCoreCtlCpu < 0 || config->mCoreCtlCpu >= mTotalNumCores) {
        QLOGW("CoreCtlCpu is incorrectly specified in XML file, So Initializing to -1");
    } else {
        mCoreCtlCpu = config->mCoreCtlCpu;
    }

    if (config->mMinCoreOnline < 0 || config->mMinCoreOnline > mTotalNumCores) {
        QLOGW("MinCoreOnline is incorrectly specified in XML file, So Initializing to 0");
    } else {
        mMinCoreOnline = config->mMinCoreOnline;
    }

    if (config->mGovInstanceType >= MAX_GOVINSTANCETYPE) {
        QLOGW("GovInstanceType is incorrectly specified in XML file, So Initializing to CLUSTER_BASED_GOV_INSTANCE");
    } else {
        mGovInstanceType = config->mGovInstanceType;
    }

    if (config->mCpufreqGov >= MAX_CPUFREQGOV) {
        QLOGW("CpufreqGov is incorrectly specified in XML file, So Initializing to INTERACTIVE");
    } else {
        mCpufreqGov = config->mCpufreqGov;
    }

    readPmQosWfiValue();
    mCalculateCoreIdx();
    if (config->mType == 0) {
        //cluster 0 is big
        mLogicalPerfMapPerfCluster();
    } else {
        //cluster 0 is little
        mLogicalPerfMapPowerCluster();
    }

    /*Deleting target configs vector, after target initialized with the required values
      which are parsed from the XML file. As the vector is not needed anymore.*/
    while (!mPerfDataStore->mTargetConfigs.empty()) {
        config = mPerfDataStore->mTargetConfigs.back();
        if (config) {
            delete config;
        }
        mPerfDataStore->mTargetConfigs.pop_back();
    }
}

//KPM node initialization
void Target::InitializeKPMnodes() {
    char tmp_s[NODE_MAX];
    int rc = 0;

    FREAD_STR(KPM_MANAGED_CPUS, tmp_s, NODE_MAX, rc);
    if (rc < 0) {
        QLOGE("Error reading KPM nodes. Does KPM exist\n");
    } else {
        snprintf(tmp_s, NODE_MAX , "%d", mNumCluster);
        FWRITE_STR(KPM_NUM_CLUSTERS, tmp_s, strlen(tmp_s), rc);

        if(!mCorePerCluster)
            return;

        //Initializing KPM nodes for each cluster, with their cpu ranges and max possible frequencies.
        for (int i=0, prevcores = 0; i<mNumCluster; i++) {
            snprintf(tmp_s, NODE_MAX , "%d-%d", prevcores, prevcores+mCorePerCluster[i]-1);
            FWRITE_STR(KPM_MANAGED_CPUS, tmp_s, strlen(tmp_s), rc);

            rc = update_freq_node(prevcores, prevcores+mCorePerCluster[i]-1 , mCpuMaxFreqResetVal[i] , tmp_s, NODE_MAX);
            if (rc >= 0) {
                FWRITE_STR(KPM_CPU_MAX_FREQ_NODE, tmp_s, strlen(tmp_s), rc);
            }
            prevcores += mCorePerCluster[i];
        }
    }
}

void Target::ExtendedTargetInit() {
    char tmp_s[NODE_MAX];
    int rc = 0;

    switch (mSocID) {
    case 246: /*8996*/
    case 291: /*8096*/
        invoke_wa_libs();
        break;

    case 305: /*8996Pro*/
    case 312: /*8096Pro*/
        invoke_wa_libs();
        break;

    case 339: /*msmnile*/
    case 361: /*msmnile without modem and SDX55*/
    case 356: /*kona*/
    case 394: /*trinket*/
    case 417: /*bengal*/
    case 420: /*bengal*/
    case 444: /*bengal*/
    case 445: /*bengal*/
    case 400: /*lito*/
    case 407: /*atoll*/
       invoke_tinyu_wa(mSocID);
       break;
    }

    InitializeKPMnodes();
    Dump();
}
