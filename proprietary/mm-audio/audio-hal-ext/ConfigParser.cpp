/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * Copyright (c) 2017 - 2018, The Linux Foundation. All rights reserved.
 */

#define LOG_TAG "ConfigParser"
//#define LOG_NDDEBUG 0

#include <dlfcn.h>
#include "ConfigParser.h"

#ifdef __LP64__
#define VNDK_FWK_LIB_PATH "/vendor/lib64/libqti_vndfwk_detect.so"
#else
#define VNDK_FWK_LIB_PATH "/vendor/lib/libqti_vndfwk_detect.so"
#endif

static void *vndk_fwk_lib_handle = nullptr;

typedef int (*isVendorEnhancedFwk_t)();
static isVendorEnhancedFwk_t isVendorEnhancedFwk = nullptr;

std::string getAudioConfigsXml() {
    vndk_fwk_lib_handle = dlopen(VNDK_FWK_LIB_PATH, RTLD_NOW);
    if (vndk_fwk_lib_handle != nullptr) {
        isVendorEnhancedFwk = (isVendorEnhancedFwk_t)
                    dlsym(vndk_fwk_lib_handle, "isRunningWithVendorEnhancedFramework");
        if (isVendorEnhancedFwk != nullptr)
            if (!isVendorEnhancedFwk())
                return AUDIO_CONFIGS_STOCK_XML_PATH;
    }
    return AUDIO_CONFIGS_XML_PATH;
}

ConfigParser *ConfigParser::getInstance() {
    static ConfigParser instance;
    return &instance;
}

ConfigParser::ConfigParser() {
    initAndParse(getAudioConfigsXml());
}

void ConfigParser::processTag(const XML_Char **attr, ConfigMap &configMap) {
    if (strcmp(attr[0], "name") != 0) {
        ALOGW("%s: Element 'name' not found!", __func__);
        return;
    }

    std::string propName(attr[1]);

    if (strcmp(attr[2], "value") != 0) {
        ALOGW("%s: Element 'value' not found for %s!", __func__, propName.c_str());
        return;
    }

    std::string propValue(attr[3]);

    configMap[propName] = propValue;

    return;
}

void ConfigParser::startTag(void *userdata, const XML_Char *tagName,
        const XML_Char **attr) {
    if ((strcmp(tagName, "property") == 0) ||
        (strcmp(tagName, "feature") == 0) ||
        (strcmp(tagName, "flag") == 0)) {
        processTag(attr, *static_cast<ConfigMap *>(userdata));
    }
    return;
}

void ConfigParser::endTag(void *userdata __unused, const XML_Char *tagName __unused) {
    return;
}

int ConfigParser::initAndParse(std::string configFile) {
    int err = 1;
    XML_Parser parser = NULL;
    FILE *file = NULL;
    void *buf = NULL;
    int bytesRead = 0;

    file = fopen(configFile.c_str(), "r");
    if (!file) {
        ALOGE("%s: Error: %d (%s). Using defaults!",
            __func__, errno, strerror(errno));
        return err;
    }

    // Create Parser
    parser = XML_ParserCreate(NULL);
    if (!parser) {
        ALOGE("%s: Failed to create XML parser!", __func__);
        err = -ENODEV;
        goto fileError;
    }

    // Set XML element handlers
    XML_SetUserData(parser, &mConfigMap);
    XML_SetElementHandler(parser, startTag, endTag);

    // Parse
    while (1) {
        buf = XML_GetBuffer(parser, BUF_SIZE);
        if (buf == NULL) {
            ALOGE("%s: XML_GetBuffer failed", __func__);
            err = -ENOMEM;
            goto parserError;
        }

        bytesRead = fread(buf, 1, BUF_SIZE, file);
        if (bytesRead < 0) {
            ALOGE("%s: fread failed, bytes read = %d", __func__, bytesRead);
            err = bytesRead;
            goto parserError;
        }

        if (XML_ParseBuffer(parser, bytesRead,
                bytesRead == 0) == XML_STATUS_ERROR) {
            ALOGE("%s: XML_ParseBuffer failed, for %s",
                    __func__, configFile.c_str());
            err = -EINVAL;
            goto parserError;
        }
        if (bytesRead == 0)
            break;
    }

// Free parser and close file in error/ at exit
parserError:
    XML_ParserFree(parser);
fileError:
    if (file != NULL)
        fclose(file);
return err;
}
