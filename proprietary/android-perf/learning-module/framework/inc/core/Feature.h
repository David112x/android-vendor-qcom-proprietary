/******************************************************************************
 *   @file    Feature.h
 *   @brief   Feature abstract base class
 *
 *   DESCRIPTION
 *      "Feature"s take measurements, "learn" at runtime and apply actions
 *   based on such learning. Feature Class provides the abstract base class
 *   for Feature implementers to derive from.
 *
 *---------------------------------------------------------------------------
 *    Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
 *    All Rights Reserved.
 *    Confidential and Proprietary - Qualcomm Technologies, Inc.
 *---------------------------------------------------------------------------
 *******************************************************************************/

#ifndef FEATURE_H
#define FEATURE_H

#include <string>
#include <atomic>
#include "FeatureInfo.h"
#include "FeatureState.h"
#include "MetaMeter.h"
#include "Algorithm.h"
#include "Action.h"


struct featureData {
    int data;
};

class FeatureLocator;
class PeriodicFeatureRunner;

class Feature {
private:
    std::string mFeatureName;
protected:
    /*
     * Features should implement allocation, maintenance and destruction of
     * resources for MetaMeter, Algorithm, Action and InputDb
     * Features should assign the respective pointer to base members
     * LM checks for null and calls the pointer whenever it wants to use
     * a Feature component.
     */
    MetaMeter* mMetaMeter;
    Algorithm* mAlgorithm;
    Action* mAction;
    FeatureInfo mfeatureInfo;
    FeatureState mFeatureState;
    FeatureLocator *mFeatureLocator;
    PeriodicFeatureRunner *mFeatureRunner;
    bool mIsDisabled;

public:
    bool mRunFlag = false;
    unsigned long int mSleepDurationInMs = -1;
    Feature(FeatureInfo featureInfo) :
        mFeatureName(featureInfo.name),
        mMetaMeter(nullptr),
        mAlgorithm(nullptr),
        mAction(nullptr),
        mfeatureInfo(featureInfo),
        mFeatureState(featureInfo.name),
        mFeatureLocator(nullptr),
        mFeatureRunner(nullptr),
        mIsDisabled(false) {

    }

    virtual ~Feature() {}
    std::string getFeatureName() {
        return mFeatureName;
    }

    bool isEnabled() {
        return !mIsDisabled;
    }

    void Disable() {
        mIsDisabled = true;
    }

    /*
     * Features should implement the logic for Async Triggers in runAsync()
     * LM calls runAsync() for each AsyncTrigger the Feature is interested in
     */
    virtual void runAsync(AsyncTriggerPayload payload);


    /*
     * Features should implement the logic for Idle Triggers in runAsyncIdle()
     * This is generally used to call algorithms when the system is idle
     * LM calls runAsyncIdle() of the Feature when it decides the system is idle
     */
    virtual void runAsyncIdle(const std::atomic_bool& interrupt);
    /*
     * Features should implement the logic for Idle Triggers in runAsyncIdle()
     * This is generally used to call algorithms when the system is idle
     * LM calls runAsyncIdle() of the Feature when it decides the system is idle
     */
    virtual void runAsyncPeriodic(const std::atomic_bool& interrupt);

    void registerServiceLocator(FeatureLocator* FeatureLocatorService) {
        mFeatureLocator = FeatureLocatorService;
    }
    void registerPeriodicRunner(PeriodicFeatureRunner* featureRunner) {
        mFeatureRunner = featureRunner;
    }
    virtual int getData(std::string key __attribute__((unused)), featureData &data, int type __attribute__((unused))) {
        data.data = {-1};
        return 0;
    }
};

class FeatureLocator {
private:
    std::vector<Feature*> mFeatureList;

    Feature *lookup(std::string featureName);
public:

    int getData(std::string featureName, std::string appName, featureData &data, int type);
    void registerFeatureList(std::vector<Feature*>);
};

class PeriodicFeatureRunner {
private:
    std::vector<Feature*> mFeatureList;

public:
    std::condition_variable mRunPeriodic;
    std::mutex mWaitForThreads;

    void notify() {
        mRunPeriodic.notify_all();
    }
    void registerFeatureList(std::vector<Feature*> featureList) {
        mFeatureList = featureList;
    }
};
#endif /* FEATURE_H */

