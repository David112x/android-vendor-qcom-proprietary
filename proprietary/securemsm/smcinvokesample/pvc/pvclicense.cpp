/********************************************************************
---------------------------------------------------------------------
 Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
----------------------------------------------------------------------
 PVCLicense Client app.
*********************************************************************/

#include <utils/Log.h>
#include <unistd.h>
#include <string.h>
#include "CPVCLicense.h"
#include "IClientEnv.h"
#include "IPVCLicense.h"
#include "ITzdRegister.h"
#include "minkipc.h"
#include "object.h"
#include "TZCom.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "PVCLICENSE_SAMPLE: "

/**
 *  Dummy PVC License.
 *
 * WARNING: THIS LICENSE WILL UNLOCK THE PVC FEATURE ONLY ON NON-PRODUCTION
 *          DEVICES.
 *          License validation will take place on non-production as well as
 *          production devices, and the result will be reported as the last
 *          parameter in the call to IPVCLicense_setHavenLicense().
 *
 *          The PVC feature though will be enabled, in non-production devices,
 *          REGARDLESS OF THE OUTCOME OF THE VALIDATION.
 *          I.e., on non-production devices, ANY LICENSE BLOB passed to
 *          IPVCLicense_setHavenLicense() will successfully enable the PVC
 *          feature.
 * */
static const uint8_t license[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};


/**
 * To run as daemon, add this program to one of the init.rc for the target platform (e.g. init.target.rc)
 *
 *    service pvclicense_sample /vendor/bin/pvclicense_sample
 *      class main
 *      user system
 *      group system
 *      oneshot
 *
 * No SEPolicy changes needed.
 *
 * */


int main(int argc, char *argv[])
{
  Object clientEnv      = Object_NULL;
  Object pvcLicense     = Object_NULL;
  Object opener         = Object_NULL;
  MinkIPC *client       = nullptr;
  uint8_t credentials[] = {0xa1, 0x01, 0x19, 0x03, 0xe8};
  int ret               = Object_OK;
  int havenErr          = Object_OK;

  do {

    client = MinkIPC_connect("/dev/socket/ssgtzd", &opener);
    if (!client) {
      opener = Object_NULL;
      ALOGE("MinkIPC_connect failed");
      ret = Object_ERROR;
      break;
    }

    ret = ITzdRegister_getTrustedClientEnv(opener, "pvclicense", strlen("pvclicense"), credentials, sizeof(credentials), &clientEnv);
    if (Object_isERROR(ret)) {
      clientEnv = Object_NULL;
      ALOGE("ITzdRegister_getTrustedClientEnv failed: %d", ret);
      break;
    }

    // obtain the proxy object for the PVCLicense service
    ret = IClientEnv_open(clientEnv, CPVCLicense_UID, &pvcLicense);
    if (Object_isERROR(ret)) {
      ALOGE("FAILED to obtain pvcLicense object: %d (0x%08X)", ret, ret);
      pvcLicense = Object_NULL;
      break;
    }

    do {
      // pass the Haven License to enable PVC
      ret = IPVCLicense_setHavenLicense(pvcLicense, license, sizeof(license), &havenErr);
      if (ret == IPVCLicense_ERROR_HAVEN_LICENSE_SERVICE_NOT_AVAILABLE) {
        ALOGD("Waiting for License validation service to be ready");
        usleep(100*1000); // retry in 100ms;
      }
    } while (ret == IPVCLicense_ERROR_HAVEN_LICENSE_SERVICE_NOT_AVAILABLE);

    if (Object_isERROR(ret)) {
      ALOGE("FAILED to set the PVC license: %d (0x%08X)", ret, ret);
      break;
    }

    // check the return code from the license validation
    if (Object_isERROR(havenErr)) {
      ALOGE("Haven License validation for PVC failed: %d (0x%08X)", havenErr, havenErr);
      break;
    }

    ALOGW("*** WARNING *** : DUMMY PVC LICENSE SUPPLIED");
    ALOGW("*** WARNING *** : NOT VALID ON PRODUCTION DEVICES");

    ALOGD("PVC License validation SUCCEEDED");
  } while (0);

  // tear down the environment
  Object_ASSIGN_NULL(pvcLicense);
  Object_ASSIGN_NULL(clientEnv);
  Object_ASSIGN_NULL(opener);
  if (client != nullptr) {
    MinkIPC_release(client);
  }

  return ret;
}
