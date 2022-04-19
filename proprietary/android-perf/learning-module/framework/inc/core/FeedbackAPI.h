/******************************************************************************
 *   @file    FeedbackAPI.h
 *   @brief   feedback apis
 *
 *   DESCRIPTION
 *   Feedback apis
 *
 *---------------------------------------------------------------------------
 *    Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
 *    All Rights Reserved.
 *    Confidential and Proprietary - Qualcomm Technologies, Inc.
 *---------------------------------------------------------------------------
 *******************************************************************************/

#ifndef _FEEDBACK_API_H_
#define _FEEDBACK_API_H_

#include "AsyncData.h"

#define FEEDBACK_API_TAG "FeedbackAPI"

#define APP_CLASSIFIER "AppClassifier"
#define LAUNCH_END_POINT "AdaptLaunch"

class PerfOutputFeedback {
private:
    FeatureLocator *mFeatureLocator;

private:
    static PerfOutputFeedback *mInstance;
    static pthread_mutex_t mMutex;

    //ctor, copy ctor, assignment overloading
    PerfOutputFeedback() {
        mFeatureLocator = nullptr;
    }
    PerfOutputFeedback(PerfOutputFeedback const& copy);               //Not to be implemented
    PerfOutputFeedback& operator=(PerfOutputFeedback const& copy);    //Not to be implemented

public:

    virtual ~PerfOutputFeedback() {}

    int getFeedback(int hintID, std::string appName, int hintType) {
        featureData appType = {0};
        std::string featureName = "";

        switch (hintID) {
        case VENDOR_FEEDBACK_WORKLOAD_TYPE:
            featureName = APP_CLASSIFIER;
            break;
        case VENDOR_FEEDBACK_LAUNCH_END_POINT:
            featureName = LAUNCH_END_POINT;
            break;
        }

        if (mFeatureLocator != nullptr)
             mFeatureLocator->getData(featureName, appName, appType, hintType);

        DEBUGV(FEEDBACK_API_TAG, "app:%s feature:%s feedback = %d", appName.c_str(), featureName.c_str(), appType.data);
        return appType.data;
    }

    void setFeatureLocator(FeatureLocator *fl) {
        mFeatureLocator = fl;
    }

    static PerfOutputFeedback* getInstance() {
        pthread_mutex_lock(&mMutex);
        if (!mInstance) {
            mInstance = new PerfOutputFeedback();
            if (!mInstance) {
                DEBUGE(FEEDBACK_API_TAG, "AdaptLaunchAction::getInstance failure");
                pthread_mutex_unlock(&mMutex);
                return NULL;
            }
        }
        pthread_mutex_unlock(&mMutex);
        return mInstance;
    }
};

#endif /* _FEEDBACK_API_H_ */
