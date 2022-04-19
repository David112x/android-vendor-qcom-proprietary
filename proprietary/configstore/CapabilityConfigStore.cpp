/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "CapabilityConfigStore.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace capabilityconfigstore {
namespace V1_0 {
namespace implementation {

CapabilityConfigStore::CapabilityConfigStore(){
}

CapabilityConfigStore::~CapabilityConfigStore() {
}

int parse_configs(char* filepath);
static int debug_mode = 0;
int xml_ext_len = strlen(XML_FILE_EXT);

typedef map < string, string > config;
typedef map < string, config > configs_map;

vector<char *> files ;
configs_map configs_maps;
config configs;


void set_debug_flag(){

   char debug_mode_string[PROPERTY_VALUE_MAX] = {'\0'};
   property_get(CAP_CONFIGSTORE_DEBUG_PROP, debug_mode_string, "false");
   if (!strncmp(debug_mode_string, "true", PROPERTY_VALUE_MAX)) {
      debug_mode = 1;
   }
}

void get_configs_from_directory() {

    DIR *dr = opendir(CONFIGSTORE_XML_PATH);
    struct dirent *de;
    if (dr == NULL) {
        ALOGE("Could not open directory: %s" CONFIGSTORE_XML_PATH);
    }
    else {
     // loop through directory and sub-directory
     while ((de = readdir(dr)) != NULL) {
         int file_length = strlen(de->d_name);
         if(strlen(de->d_name) > xml_ext_len && !strcmp(de->d_name + file_length-xml_ext_len, XML_FILE_EXT)!= NULL){
            char * filename = (char *)calloc(file_length+1, sizeof(char));
            if(filename) {
                strlcpy(filename,de->d_name,file_length+1);
                files.push_back(filename);
            }
            else {
             ALOGE("Failed to alloc for filename");
            }
        }
     }
     closedir(dr);
    }
}

/** Go over each xml file in the CONFIGSTORE_XML_PATH and send it to parse_configs to parse the data to get stored in memory
 *
**/

int parse_configs(char* filepath) {

    ALOGI("Parsing config for %s", filepath);

    xmlDocPtr doc;
    xmlNodePtr curNode;
    xmlNodePtr curNodeTech;
    char *name = NULL;
    char *value = NULL;
    char *techConfig = NULL;

    if(filepath == NULL) {
        ALOGE("Empty file path");
        return -1;
    }

    ALOGE("starting to load xml file: '%s'", filepath);

    doc = xmlReadFile(filepath, "UTF-8", XML_PARSE_RECOVER);
    if(NULL == doc) {
        ALOGE("Document(%s) not parsed successfully.", filepath);
        goto error;
    }

    curNode = xmlDocGetRootElement(doc);
    if(NULL == curNode) {
        ALOGE("'%s' is empty document", filepath);
        goto error;
    }

    if(xmlStrcmp(curNode->name, BAD_CAST "CapabilityStore")) {
        ALOGE("document(%s) of the wrong type, root node != CapabilityStore", filepath);
        goto error;
    }

    curNodeTech = curNode->xmlChildrenNode;
    while(curNodeTech != NULL){
         if(curNodeTech->type != XML_TEXT_NODE) {
           if(xmlStrcmp(curNodeTech->name, BAD_CAST "namespace")) {
             ALOGE("Document(%s) of the wrong format, child node != namespace");
             goto error;
           }
           techConfig =  (char *) xmlGetProp(curNodeTech, BAD_CAST "name");
           if(techConfig == NULL) {
              ALOGE("namespace tag should be followed by \"name\" ");
              goto error;
           }
           curNode =  curNodeTech->xmlChildrenNode;
           while(curNode != NULL) {
                if(!xmlStrcmp(curNode->name, BAD_CAST "capability")) {
                  name =  (char *) xmlGetProp(curNode, BAD_CAST "key");
                  value = (char *) xmlGetProp(curNode, BAD_CAST "value");
                  if(name != NULL && value != NULL) {
                    CONFIGSTORE_ALOGI("Tech namespace = %s, key = %s, value = %s", techConfig, name , value);
                    configs_maps[techConfig][name] = value;
                  }
                }
                curNode = curNode->next;
           }
         }
         curNodeTech = curNodeTech->next;
    }
    xmlFreeDoc(doc);
    return 0;

error:
    xmlFreeDoc(doc);
    return -1;
}

/**
 * @param xml file that will be parsed and stored in map
 * @return -1 for failed in parsing
 * @        0  on sucess
 *
 * There will be no check if the key already exists before adding a new key. Assumption is second occurance of the key will
 * override the first one.
 * **/

void parse_xml(){

    ALOGI("Init of QTI configstorehal");
    set_debug_flag();

    get_configs_from_directory();
    sort(files.begin(), files.end());

    configs_maps.clear();
    for (int i=0; i< files.size(); i++){
            size_t len = strlen(CONFIGSTORE_XML_PATH) + strlen(files[i]) + 2;
            char* filename_path = (char*)malloc(len+1);
            if(!filename_path)
              ALOGE("Failed to alloc memory to store config path");
            else {
             snprintf(filename_path, len, "%s/%s", CONFIGSTORE_XML_PATH, files[i]);
             int j = parse_configs(filename_path);
             //If failed log message and  continue for the others
             if (j == -1)
                ALOGE("failed to parse %s", files[i]);
             free(filename_path);
            }
            free(files[i]);
    }
    files.clear();
}
/**
 * Query the value via area and key
**/

Return<void>  CapabilityConfigStore::getConfig(const hidl_string& area, const hidl_string& key, getConfig_cb _hidl_cb) {
    CommandResult ret ;
    ret.result_type = Result::NOT_FOUND;
    map<string , map <string, string>>::iterator iter;
    map<string, string>::iterator innerIter;
    const char* value = NULL;
    CONFIGSTORE_ALOGI("Configstore for area and key : %s %s",area.c_str(),key.c_str());

    iter = configs_maps.find(area.c_str());
    if (iter != configs_maps.end()){
        innerIter = iter->second.find(key.c_str());
        if (innerIter != iter->second.end()){
            CONFIGSTORE_ALOGI("Found config value %s" , innerIter->second.c_str());
            value = innerIter->second.c_str();
            ret.result_type = Result::SUCCESS;
            ret.value       = value;
        }
        else {
              CONFIGSTORE_ALOGI("Config value not found");
        }
    }
     _hidl_cb(ret);
     return Void();
}

void CapabilityConfigStore::parse_capability_data() {
    parse_xml();
}
ICapabilityConfigStore* HIDL_FETCH_ICapabilityConfigStore(const char* /* name */) {
    ALOGI("start HIDL_FETCH_ICapabilityConfigStore");
    CapabilityConfigStore *configStore = new CapabilityConfigStore;
    configStore->parse_capability_data();
    return configStore;
}

}
}  // namespace V1_0
}  // namespace capabilityconfigstore
}  // hardware
}  // namespace qti
}  // namespace vendor

