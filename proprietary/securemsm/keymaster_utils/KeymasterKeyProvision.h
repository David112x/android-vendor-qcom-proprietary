/*
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef KEYMASTERKEYPROVISION_H_
#define KEYMASTERKEYPROVISION_H_

#include <cstdlib>
#include <map>
#include <vector>


#include <hardware/keymaster0.h>
#include <hardware/keymaster1.h>
#include <hardware/keymaster2.h>

#include <KeymasterUtils.h>
#include <KeymasterBuffer.h>

using namespace keymasterutils;
using namespace std;

namespace keymasterdevice {

#define MAX_KEYS_IN_KEYBOX  2

typedef struct {
    keymaster_algorithm_t algo;
    keymaster_key_blob_t key;
    keymaster_cert_chain_t cert_chain;
} keybox_t;

typedef struct {
    keymaster_blob_t device_id;
    keybox_t keybox[MAX_KEYS_IN_KEYBOX];
} km_install_keybox_t;

class KeymasterKeyProvision {
 public:
    KeymasterKeyProvision(keymaster_security_level_t security_level) :
        utils(new KeymasterUtils(security_level)),
        msecurityLevel(security_level) {}
    virtual ~KeymasterKeyProvision() {}

    int KeyMasterProvisionInit();

    int KeyMasterInstallKeybox(km_install_keybox_t *keybox);
    int KeyMasterProvisionDeviceId(string Brand, string Device, string Product,
            string SerialNum, string Manufacturer, string Model,string Imei, string Meid);
    int SetProvisioniningSuccess();
    int CleanProvisioningState();
    int GetPreSharedSecret(uint8_t *preSharedSecret, size_t preSharedSecretSize);

    int GenerateAttkKeyPair(size_t copy_num);
    int VerifyAttkKeyPair();
    int ExportAttkKeyPair(uint8_t *PubKey, size_t *PubKeySize);
    int GetDeviceId(uint8_t *DeviceId, size_t *DeviceIdSize);
 private:

    int ProvisionDeviceId(const keymaster_key_param_set_t* in_params);
    keymaster_error_t params_serialize(std::unique_ptr<KeymasterBuffer> &buffer,
            keymaster_key_param_t* a, size_t params_count);
    int KeyMasterInstallKeyboxOld(km_install_keybox_t *keybox);
    int KeyMasterInstallKeyboxNew(km_install_keybox_t *keybox);
    std::unique_ptr<KeymasterUtils> utils;
    keymaster_security_level_t msecurityLevel;
};

}  // namespace keymasterdevice

#endif //KEYMASTERKEYPROVISION_H_
