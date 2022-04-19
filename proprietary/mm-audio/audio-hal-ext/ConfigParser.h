/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * Copyright (c) 2017 - 2018, The Linux Foundation. All rights reserved.
 */

#ifndef __CONFIG_PARSER_H__
#define __CONFIG_PARSER_H__

#include <expat.h>
#include <errno.h>
#include <log/log.h>
#include <map>
#include <string>
#include <sys/types.h>

#define DISALLOW_COPY_AND_ASSIGN(name) \
    name(const name&); \
    name &operator=(const name&)

#define BUF_SIZE 1024
#define AUDIO_CONFIGS_XML_PATH "/vendor/etc/audio_configs.xml"
#define AUDIO_CONFIGS_STOCK_XML_PATH "/vendor/etc/audio_configs_stock.xml"

#ifdef __cplusplus
    extern "C" {
#endif

typedef std::map<std::string, std::string> ConfigMap;

class ConfigParser {
public:
    ConfigParser();
    virtual ~ConfigParser() {};
    static ConfigParser *getInstance();
    static void startTag(void *userdata, const XML_Char *tagName, const XML_Char **attr);
    static void endTag(void *userdata __unused, const XML_Char *tagName __unused);
    static void processTag(const XML_Char **attr, ConfigMap &confMap);

    ConfigMap mConfigMap;
private:
    int initAndParse(std::string configFile);

    DISALLOW_COPY_AND_ASSIGN(ConfigParser);
};

#ifdef __cplusplus
}
#endif
#endif // __CONFIG_PARSER_H__
