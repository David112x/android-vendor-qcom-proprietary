/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#define LOG_TAG "dynamicdds_vts"

#include <log/log.h>
#include <vendor/qti/data/factory/1.0/IFactory.h>
#include <vendor/qti/hardware/data/dynamicdds/1.0/ISubscriptionManager.h>
#include <VtsHalHidlTargetTestBase.h>
#include <VtsHalHidlTargetCallbackBase.h>

using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::sp;

using ::vendor::qti::data::factory::V1_0::IFactory;

using ::vendor::qti::hardware::data::dynamicdds::V1_0::ISubscriptionManager;
using ::vendor::qti::hardware::data::dynamicdds::V1_0::IToken;
using ::vendor::qti::hardware::data::dynamicdds::V1_0::StatusCode;
using ::vendor::qti::hardware::data::dynamicdds::V1_0::ISetAppPreferencesCallback;
using ::vendor::qti::hardware::data::dynamicdds::V1_0::IGetAppPreferencesCallback;
using ::vendor::qti::hardware::data::dynamicdds::V1_0::IDddsCallback;
using ::vendor::qti::hardware::data::dynamicdds::V1_0::Carrier;
using ::vendor::qti::hardware::data::dynamicdds::V1_0::SubscriptionConfig;

#define NUM_OF_STATUS_CODE 4

//Need to sync with types.hal
std::string statusCode[NUM_OF_STATUS_CODE] = {
    "OK",
    "INVALID_ARGUMENTS",
    "NOT_SUPPORTED",
    "FAILED",
};

class Token : public IToken {
};

class DataFactoryTestBase : public ::testing::VtsHalHidlTargetTestBase {
public:
    virtual void SetUp() override {
        setUpFactory();
    }

    void setUpFactory() {
        factory = IFactory::getService();
        ASSERT_NE(nullptr, factory.get()) << "Could not get Factory";
    }

    sp<ISubscriptionManager> createManager
        (const ::vendor::qti::data::factory::V1_0::IFactory::StatusCode expected_result) {

        sp<ISubscriptionManager> subManager;

        auto cb = [&](::vendor::qti::data::factory::V1_0::IFactory::StatusCode status,
                      sp<ISubscriptionManager> manager) {
            ASSERT_EQ(expected_result, status);

            subManager = manager;
            ASSERT_NE(nullptr, subManager.get()) << "Could not create ISubscriptionManager";
        };

        sp<IToken> token = new Token();
        const Return<void> ret = factory->createDynamicddsISubscriptionManager(token, cb);
        return subManager;
    }

    sp<IFactory> factory;
};

// positive test. Ensure IFactory can request an ISubscriptionManager
TEST_F(DataFactoryTestBase, TestCreateSubscriptionManager) {
    sp<ISubscriptionManager> manager = createManager(
       ::vendor::qti::data::factory::V1_0::IFactory::StatusCode::OK);
    ASSERT_NE(nullptr, manager.get()) << "Could not get Subscription Manager instance";
}

class DynamicDDSTestBase : public DataFactoryTestBase {
public:
    virtual void SetUp() override {
        setUpFactory();
        manager = createManager(
           ::vendor::qti::data::factory::V1_0::IFactory::StatusCode::OK);
    }

    sp<ISubscriptionManager> manager;
};

/* ---------------------------- getAppPreferences test elements------------------------------ */
class GetAppPreferencesCallbackArgs {
public:
    GetAppPreferencesCallbackArgs(StatusCode setStatus,
                                  const hidl_vec<SubscriptionConfig>& setPreferences) :
        status(setStatus), preferences(setPreferences) {}

    StatusCode getStatus() {return status;};
    const hidl_vec<SubscriptionConfig>& getPreferences() {return preferences;};

private:
    StatusCode status;
    const hidl_vec<SubscriptionConfig>& preferences;
};

class MockGetAppPreferencesCallback : public IGetAppPreferencesCallback,
    public ::testing::VtsHalHidlTargetCallbackBase<GetAppPreferencesCallbackArgs> {

public:
    MockGetAppPreferencesCallback() {};
    virtual ~MockGetAppPreferencesCallback() {};

    Return<void> onResult(StatusCode status,
                          const hidl_vec<SubscriptionConfig>& preferences) override {
        ALOGI("GetAppPreferencesCallback arrived");
        dumpStatusCode(status);
        if (status == StatusCode::OK) {
            for (SubscriptionConfig subConfig : preferences) {
                dumpSubscriptionConfig(subConfig);
            }
        }
        GetAppPreferencesCallbackArgs args(status, preferences);
        NotifyFromCallback("onResult", args);
        return Void();
    };

private:
    //Helper function to dump status code
    static void dumpStatusCode(const StatusCode& status) {
        ALOGI("Status = %s", statusCode[(int)status].c_str());
    }

    //Helper function to dump application preference
    static void dumpSubscriptionConfig(const SubscriptionConfig& subconfig) {
        ALOGI("appName = %s", subconfig.appName.c_str());
        ALOGI("priority = %d", subconfig.priority);
        ALOGI("Carriers = ");
        for (const Carrier carrier : subconfig.carriers) {
            ALOGI("%s, %d;", carrier.iin.c_str(), carrier.preference);
        }
    }
};

/* ---------------------------- setAppPreferences test elements ----------------------------- */
class SetAppPreferencesCallbackArgs {
public:
    SetAppPreferencesCallbackArgs(StatusCode setStatus, string setReason) :
        status(setStatus), reason(setReason){}

    StatusCode getStatus() {return status;};

private:
    StatusCode status;
    string reason;
};

class MockSetAppPreferencesCallback : public ISetAppPreferencesCallback,
    public ::testing::VtsHalHidlTargetCallbackBase<SetAppPreferencesCallbackArgs> {

public:
    MockSetAppPreferencesCallback() {};
    virtual ~MockSetAppPreferencesCallback() {};

    Return<void> onResult(StatusCode status, const hidl_string& reason) override {
        ALOGI("SetAppPreferencesCallback arrived");
        dumpStatusCode(status);
        dumpReason(reason);
        SetAppPreferencesCallbackArgs args(status, reason);
        NotifyFromCallback("onResult", args);
        return Void();
    };

private:
    //Helper function to dump status code
    static void dumpStatusCode(const StatusCode& status) {
        ALOGI("Status = %s", statusCode[(int)status].c_str());
    }

    //Helper function to dump reason
    static void dumpReason(const hidl_string& reason) {
        ALOGI("Detailed Reason = %s", reason.c_str());
    }
};

class AppPreferencesBuilder {
public:
    class SubscriptionConfigType {
    public:

        SubscriptionConfigType(){};
        virtual ~SubscriptionConfigType(){};

        void setAppName(std::string appName) {this->appName = appName;};
        std::string getAppName() {return appName;};

        void addCarrier(std::string iin, uint8_t preference) {
            Carrier carrier;
            carrier.iin = iin;
            carrier.preference = preference;
            carriersList.push_back(carrier);
        };

        void setPriority(uint8_t priority) {this->priority = priority;};
        uint8_t getPriority() {return priority;};

        SubscriptionConfig getSubscriptionConfig() {
            SubscriptionConfig subscriptionConfig;
            subscriptionConfig.appName = this->appName;
            subscriptionConfig.priority = this->priority;
            subscriptionConfig.carriers = this->carriersList;
            return subscriptionConfig;
        };

        void dumpSubscriptionConfig() {
            ALOGI("SubscriptionConfig: appName = %s", appName.c_str());
            ALOGI("SubscriptionConfig: priority = %d", priority);
            ALOGI("SubscriptionConfig: CarrierList = ");
            for (const Carrier carrier : this->carriersList) {
                ALOGI("%s, %d;", carrier.iin.c_str(), carrier.preference);
            }
        }

        void clear() {
            appName = "";
            priority = 0;
            carriersList.clear();
        }
    private:
        std::string appName;
        uint8_t priority;
        std::vector<Carrier> carriersList;

    };

    AppPreferencesBuilder(){};
    virtual ~AppPreferencesBuilder(){};

    void addSubscriptionConfig(SubscriptionConfigType subconfig){
        preferences.push_back(subconfig);
    }

    void dumpInputBox() {
        for (SubscriptionConfigType subConfig : preferences) {
            subConfig.dumpSubscriptionConfig();
        }
    }

    std::vector<SubscriptionConfig> getPreferences() {
        std::vector<SubscriptionConfig> result;
        for (SubscriptionConfigType subConfig : preferences) {
            result.push_back(subConfig.getSubscriptionConfig());
        }
        return result;
    }

private:
    std::vector<SubscriptionConfigType> preferences;
};

/* ---------------------- registerForDynamicSubChanges test elements------------------------- */
class DddsCallbackArgs {
public:
    DddsCallbackArgs(bool setAvailable) :
        available(setAvailable){}

    bool getIsAvailable() {return available;};

private:
    bool available;
};

class MockDddsCallback : public IDddsCallback,
    public ::testing::VtsHalHidlTargetCallbackBase<DddsCallbackArgs> {

public:
    MockDddsCallback() {};
    virtual ~MockDddsCallback() {};

    Return<void> onFeatureAvailable(bool available) override {
        ALOGI("DddsCallback onFeatureAvailable arrived with available = %d", available);
        DddsCallbackArgs args(available);
        NotifyFromCallback("onFeatureAvailable", args);
        return Void();
    };

    Return<void> onSubChanged(uint32_t slotId) override {
        ALOGI("DddsCallback onSubChanged arrived with slotId = %d", slotId);
        return Void();
    };

};

/* ---------------------- begin of DynamicDDS HAL Test cases ------------------------------- */
// positive test. setDynamicSubscriptionChange with enable: true
TEST_F(DynamicDDSTestBase, TestSetDynamicSubscriptionChange_On) {
    StatusCode status = manager->setDynamicSubscriptionChange(true);
    ASSERT_EQ(StatusCode::OK, status);
}

// positive test. clearAppPreferences to clear all preconfigured application preferences
TEST_F(DynamicDDSTestBase, TestClearAppPreferences) {
    StatusCode status  = manager->clearAppPreferences();
    ASSERT_EQ(StatusCode::OK, status);
}

// negative test. getAppPreferences to fetch modem configured application preferences.
// However, there is no preconfigured application preferences in modem
TEST_F(DynamicDDSTestBase, TestGetAppPreferences_EmptyPreferences) {
    MockGetAppPreferencesCallback* mCb = new MockGetAppPreferencesCallback();

    StatusCode status  = manager->getAppPreferences(mCb);

    auto res = mCb->WaitForCallback("onResult");

    ASSERT_EQ(StatusCode::OK, status);
    EXPECT_TRUE(res.no_timeout);
    if (res.no_timeout == true) {
        EXPECT_EQ(StatusCode::OK, res.args->getStatus());
        EXPECT_EQ((unsigned int)0, res.args->getPreferences().size());
    }
}

// negative test. setAppPreferences try to set empty preference
TEST_F(DynamicDDSTestBase, TestSetAppPreferences_EmptyPreference) {
    MockSetAppPreferencesCallback* mCb = new MockSetAppPreferencesCallback();
    AppPreferencesBuilder appPreferencesBuilder;
    AppPreferencesBuilder::SubscriptionConfigType subConfig;

    StatusCode status  = manager->setAppPreferences(appPreferencesBuilder.getPreferences(), mCb);
    ASSERT_EQ(StatusCode::INVALID_ARGUMENTS, status);
}

// negative test. setAppPreferences try to set empty appName
TEST_F(DynamicDDSTestBase, TestSetAppPreferences_EmptyAppName) {
    MockSetAppPreferencesCallback* mCb = new MockSetAppPreferencesCallback();
    AppPreferencesBuilder appPreferencesBuilder;
    AppPreferencesBuilder::SubscriptionConfigType subConfig;

    subConfig.setAppName("");
    subConfig.setPriority(7);
    subConfig.addCarrier("8986003",5);
    subConfig.addCarrier("8986001",4);
    appPreferencesBuilder.addSubscriptionConfig(subConfig);
    subConfig.clear();

    StatusCode status  = manager->setAppPreferences(appPreferencesBuilder.getPreferences(), mCb);
    ASSERT_EQ(StatusCode::INVALID_ARGUMENTS, status);
}

// negative test. setAppPreferences try to set invalid priority
TEST_F(DynamicDDSTestBase, TestSetAppPreferences_InvalidPriority) {
    MockSetAppPreferencesCallback* mCb = new MockSetAppPreferencesCallback();
    AppPreferencesBuilder appPreferencesBuilder;
    AppPreferencesBuilder::SubscriptionConfigType subConfig;

    subConfig.setAppName("com.android.wechat");
    subConfig.setPriority(12);  //12 is invalid priority
    subConfig.addCarrier("8986003",5);
    subConfig.addCarrier("8986001",4);
    appPreferencesBuilder.addSubscriptionConfig(subConfig);
    subConfig.clear();

    StatusCode status  = manager->setAppPreferences(appPreferencesBuilder.getPreferences(), mCb);
    ASSERT_EQ(StatusCode::INVALID_ARGUMENTS, status);
}

// negative test. setAppPreferences try to set application with empty carriers
TEST_F(DynamicDDSTestBase, TestSetAppPreferences_EmptyCarrier) {
    MockSetAppPreferencesCallback* mCb = new MockSetAppPreferencesCallback();
    AppPreferencesBuilder appPreferencesBuilder;
    AppPreferencesBuilder::SubscriptionConfigType subConfig;

    subConfig.setAppName("com.android.wechat");
    subConfig.setPriority(7);
    //Carriers is empty
    appPreferencesBuilder.addSubscriptionConfig(subConfig);
    subConfig.clear();

    StatusCode status  = manager->setAppPreferences(appPreferencesBuilder.getPreferences(), mCb);
    ASSERT_EQ(StatusCode::INVALID_ARGUMENTS, status);
}

// negative test. setAppPreferences try to set invalid IIN
TEST_F(DynamicDDSTestBase, TestSetAppPreferences_InvalidIIN) {
    MockSetAppPreferencesCallback* mCb = new MockSetAppPreferencesCallback();
    AppPreferencesBuilder appPreferencesBuilder;
    AppPreferencesBuilder::SubscriptionConfigType subConfig;

    subConfig.setAppName("com.android.wechat");
    subConfig.setPriority(7);
    subConfig.addCarrier("8986003",5);
    subConfig.addCarrier("",4); //"" is invalid IIN
    appPreferencesBuilder.addSubscriptionConfig(subConfig);
    subConfig.clear();

    StatusCode status  = manager->setAppPreferences(appPreferencesBuilder.getPreferences(), mCb);
    ASSERT_EQ(StatusCode::INVALID_ARGUMENTS, status);
}

// negative test. setAppPreferences try to set invalid carrier preference
TEST_F(DynamicDDSTestBase, TestSetAppPreferences_InvalidCarrierPreference) {
    MockSetAppPreferencesCallback* mCb = new MockSetAppPreferencesCallback();
    AppPreferencesBuilder appPreferencesBuilder;
    AppPreferencesBuilder::SubscriptionConfigType subConfig;

    subConfig.setAppName("com.android.wechat");
    subConfig.setPriority(7);
    subConfig.addCarrier("8986003",11); //11 is invalid Carrier Preference
    subConfig.addCarrier("8986001",4);
    appPreferencesBuilder.addSubscriptionConfig(subConfig);
    subConfig.clear();

    StatusCode status  = manager->setAppPreferences(appPreferencesBuilder.getPreferences(), mCb);
    ASSERT_EQ(StatusCode::INVALID_ARGUMENTS, status);
}

// negative test. setAppPreferences try to set more than 100 preferences
TEST_F(DynamicDDSTestBase, TestSetAppPreferences_TooManyPreferences) {
    MockSetAppPreferencesCallback* mCb = new MockSetAppPreferencesCallback();
    AppPreferencesBuilder appPreferencesBuilder;
    AppPreferencesBuilder::SubscriptionConfigType subConfig;

    for (unsigned int i = 0; i < 108; ++i) { //108 is more than the max size 100
        subConfig.setAppName("com.android.wechat_" + std::to_string(i));
        subConfig.setPriority(7);
        subConfig.addCarrier("8986003",5);
        subConfig.addCarrier("8986001",4);
        appPreferencesBuilder.addSubscriptionConfig(subConfig);
        subConfig.clear();
    }

    StatusCode status  = manager->setAppPreferences(appPreferencesBuilder.getPreferences(), mCb);
    ASSERT_EQ(StatusCode::INVALID_ARGUMENTS, status);
}

// positive test. setAppPreferences to set below application preference in modem
TEST_F(DynamicDDSTestBase, TestSetAppPreferences_ValidPreferences) {
    MockSetAppPreferencesCallback* mCb = new MockSetAppPreferencesCallback();
    AppPreferencesBuilder appPreferencesBuilder;
    AppPreferencesBuilder::SubscriptionConfigType subConfig;

    subConfig.setAppName("com.android.wechat");
    subConfig.setPriority(7);
    subConfig.addCarrier("8986003",5);
    subConfig.addCarrier("8986001",4);
    appPreferencesBuilder.addSubscriptionConfig(subConfig);
    subConfig.clear();

    subConfig.setAppName("com.android.weibo");
    subConfig.setPriority(4);
    subConfig.addCarrier("8986003",8);
    subConfig.addCarrier("8986001",2);
    appPreferencesBuilder.addSubscriptionConfig(subConfig);

    StatusCode status  = manager->setAppPreferences(appPreferencesBuilder.getPreferences(), mCb);
    auto res = mCb->WaitForCallback("onResult");

    ASSERT_EQ(StatusCode::OK, status);
    EXPECT_TRUE(res.no_timeout);
    if (res.no_timeout == true) {
        EXPECT_EQ(StatusCode::OK, res.args->getStatus());
    }
}

// positive test. getAppPreferences to fetch modem configured application preferences
TEST_F(DynamicDDSTestBase, TestGetAppPreferences_ValidPreferences) {
    MockGetAppPreferencesCallback* mCb = new MockGetAppPreferencesCallback();

    StatusCode status  = manager->getAppPreferences(mCb);
    auto res = mCb->WaitForCallback("onResult");

    ASSERT_EQ(StatusCode::OK, status);
    EXPECT_TRUE(res.no_timeout);
    if (res.no_timeout == true) {
        EXPECT_EQ(StatusCode::OK, res.args->getStatus());
    }
}

// positive test. setDynamicSubscriptionChange with enable: false
TEST_F(DynamicDDSTestBase, TestSetDynamicSubscriptionChange_Off) {
    StatusCode status  = manager->setDynamicSubscriptionChange(false);
    ASSERT_EQ(StatusCode::OK, status);
}

// positive test. Test registerForDynamicSubChanges and IDddsCallback.onFeatureAvailable
// Need to be invoke when feature is enable/disable
TEST_F(DynamicDDSTestBase, TestRegisterForDynamicSubChanges) {
    MockDddsCallback* mCb = new MockDddsCallback();

    StatusCode status  = manager->registerForDynamicSubChanges(mCb);
    ASSERT_EQ(StatusCode::OK, status);

    status = manager->setDynamicSubscriptionChange(true);
    auto res = mCb->WaitForCallback("onFeatureAvailable");

    ASSERT_EQ(StatusCode::OK, status);
    EXPECT_TRUE(res.no_timeout);
    if (res.no_timeout == true) {
        EXPECT_TRUE(res.args->getIsAvailable());
    }

    status = manager->setDynamicSubscriptionChange(false);
    auto res2 = mCb->WaitForCallback("onFeatureAvailable");

    ASSERT_EQ(StatusCode::OK, status);
    EXPECT_TRUE(res2.no_timeout);
    if (res2.no_timeout == true) {
        EXPECT_FALSE(res2.args->getIsAvailable());
    }
}

int main(int argc, char** argv) {
    ALOGI("Enter DynamicDDS VTS test suite");
    ::testing::InitGoogleTest(&argc, argv);
    int status = RUN_ALL_TESTS();
    ALOGI("Test result with status = %d", status);
    return status;
}
