/*
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include "Factory.h"
#include "socket.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace factory {
namespace V1_0 {
namespace implementation {

Factory::Factory(){
    create_socket_thread();
}

Factory::~Factory() {
}

/*
* Start App test via command
*
*/
Return<void> Factory::runApp(const hidl_string& name,const hidl_string& params, bool isStart, runApp_cb _hidl_cb) {
    ALOGI("start to runApp with: %s, start: %d",name.c_str(),isStart);
    msg_t *msg = NULL;
    FactoryResult factoryResult;

    if(check_socket_ready()){

        if(isStart)
            msg = run_app(name.c_str(),params.c_str(),params.size());
        else
            msg = stop_app(name.c_str());
    }else{
           ALOGE("socket not ready");
           factoryResult.result = IResultType::FAILED;
    }

    factoryResult.result = IResultType::SUCCESS;
     _hidl_cb(factoryResult);
    return Void();
}

/*
* Get SMB status
*
*/
Return<void> Factory::getSmbStatus(getSmbStatus_cb _hidl_cb) {
    ALOGI("start to getSmbStatus");
    FactoryResult factoryResult;
    msg_t *t = smb_status();
    if(t->result == 0)
        factoryResult.result = IResultType::SUCCESS;
    else
        factoryResult.result = IResultType::FAILED;

    _hidl_cb(factoryResult);
    return Void();
}

/*
* purpose
*     enter ship mode
*
* requires
*
* guarantees
*       set /sys/module/qpnp_power_on/parameters/ship_mode_en = 1
*       return true when the value set
*/
Return<bool> Factory::enterShipMode() {
    ALOGI("start to enterShipMode");
    return enter_ship_mode();
}
/*
* purpose
*     enable/diable charger
*
* requires
*
* guarantees
*       set charger diable prop = 1
*       return true when the value set
*/
Return<bool> Factory::chargerEnable(bool enable) {
    ALOGI("start to chargerEnable enable: %d",enable);
    charger_enable(enable);
    return true;
}
/*
* purpose
*     enable/diable charger
*
* requires
*
* guarantees
*       set charger diable prop = 1
*       return true when the value set
*/
Return<bool> Factory::wifiEnable(bool enable) {
    ALOGI("start to wifiEnable enable: %d",enable);
    wifi_enable(enable);
    return true;
}

/*
* purpose
*     eraseAllFiles
*
* requires filePath
*
* guarantees
*
*       return success when the erase all files
*/

static bool is_file(const char *path) {
    struct stat st;
    if(lstat(path, &st) == 0)
        return S_ISREG(st.st_mode) != 0;
    return false;
}

Return<IResultType> Factory::eraseAllFiles(const hidl_string& path) {
    IResultType ret = IResultType::FAILED;
    ALOGI("start to eraseAllFiles: %s",path.c_str());

    struct dirent *de;
    DIR *dir;
    char filepath[PATH_MAX];

    dir = opendir(path.c_str());
    if(dir != 0) {
        while((de = readdir(dir))) {
            if((strncmp(de->d_name, ".", 2) == 0)
               || (strncmp(de->d_name, "..", 3) == 0))
                continue;
            snprintf(filepath, sizeof(filepath), "%s%s", path.c_str(), de->d_name);
            if(is_file(filepath)) {
                remove(filepath);
                ALOGI("file(%s) be removed", filepath);
            }
        }
        closedir(dir);
    } else {
        ALOGI("dir(%s) open fail, error=%s", path.c_str(), strerror(errno));
    }

    ret = IResultType::SUCCESS;
    return ret;
}

/*
* purpose
*     dirListFileNameToFile
*
* requires filePath/fileName
*
* guarantees
*
*       return success when the erase all files
*/
Return<IResultType> Factory:: dirListFileNameToFile(const hidl_string& path, const hidl_string& name) {
    IResultType ret = IResultType::FAILED;
    struct dirent *de;
    DIR *dir;
    FILE *fp = NULL;
    char filepath[PATH_MAX];
    char buf[255];

    memset(buf, 0, 255);

    snprintf(filepath, sizeof(filepath), "%s%s", path.c_str(), name.c_str());
    ALOGI("List the dir(%s) contents to file(%s)", path.c_str(), filepath);
    fp = fopen(filepath, "w");
    if(!fp) {
        ALOGI("file(%s) open fail, error=%s", filepath, strerror(errno));
        return ret;
    }

    dir = opendir(path.c_str());
    if(dir != 0) {
        while((de = readdir(dir))) {
            if(!strncmp(de->d_name, ".", 1) || !strcmp(de->d_name, ".."))
                continue;

            ALOGI("%s \n", de->d_name);
            snprintf(buf, sizeof(buf), "%s\n", de->d_name);
            fwrite(buf, strlen(buf), 1, fp);
        }
        closedir(dir);
    } else {
        ALOGI("dir(%s) open fail, error=%s", path.c_str(), strerror(errno));
    }

    fclose(fp);

    ret = IResultType::SUCCESS;
    return ret;
}

/*
* purpose
*     readFile
*
* requires path/req
*
* guarantees
*
*       return read data
*/
Return<void> Factory:: readFile(const hidl_string& path, const ReadFileReq& req, readFile_cb _hidl_cb) {
    ReadFileResult result;

    FILE *fp = NULL;
    char filepath[255] = { 0 };
    uint16_t iSize = 0;
    uint8_t Data[PACK_SIZE];

    /*parse parameter */
    char *pFileName = (char *)req.file_name.c_str();
    uint32_t offset = req.offset;
    uint16_t max_size = req.max_size;

    /*get dir */
    memset(Data, 0, sizeof(Data));

    snprintf(filepath, sizeof(filepath), "%s%s", path.c_str(), pFileName);
    ALOGI("start to readFile %s",filepath);

    fp = fopen(filepath, "rb");
    if(fp) {
        fseek(fp, 0, SEEK_END); /*non-portable */
        result.file_size = ftell(fp);

        ALOGI("readFile: offset=%d,filesize=%ld \n", offset, result.file_size);

        if(offset < result.file_size) {
            fseek(fp, offset, SEEK_SET);
            iSize = fread(Data, 1, max_size, fp);
            result.result_type = IResultType::SUCCESS;
        } else
            result.result_type = IResultType::OPEN_ERROR;

        fclose(fp);
        fp = NULL;
    } else {
        result.result_type = IResultType::NOT_EXIST_ERR;
        ALOGI("file(%s) open fail, error=%s\n", filepath, strerror(errno));
    }

    result.offset = offset;
    result.size = iSize;
    result.data.resize(sizeof(Data));
    memcpy(result.data.data(), Data, sizeof(Data));
     _hidl_cb(result);
    return Void();
}

/*
* purpose
*     writeFile
*
* requires path/req
*
* guarantees
*
*       return success when write file
*/
Return<IResultType> Factory:: writeFile(const hidl_string& path, const WriteFileReq& req) {
    IResultType ret = IResultType::FAILED;
    ALOGI("start to writeFile: %s", path.c_str());

    char filepath[255] = { 0 };
    static FILE *fp = NULL;

    uint8_t append_data = req.append_data;
    uint16_t i_size = req.i_size;
    unsigned char *pData = (unsigned char *)req.data.c_str();
    snprintf(filepath, sizeof(filepath), "%s%s", path.c_str(),  req.file_name.c_str());

    if(!append_data)
        fp = fopen(filepath, "w");
    else
        fp = fopen(filepath, "a+");

    if(fp) {
        if(i_size > 0)
            fwrite(pData, 1, i_size, fp);

        ALOGI("write file: i_size=%d", i_size);
        ret = IResultType::SUCCESS;
        fclose(fp);
        fp = NULL;
    } else {
        ret = IResultType::OPEN_ERROR;
    }

    return ret;
}

/*
* Execute command with value
*
*/
Return<void> Factory::delegate(const hidl_string& cmd, const hidl_string& value, delegate_cb _hidl_cb) {
    ALOGI("start to delegate cmd: %s, value: %s",cmd.c_str(),value.c_str());
    V1_1::FactoryResult factoryResult;

    if (strcmp(cmd.c_str(), "setprop") == 0){
        ALOGI("setprop:");
        vector<string> contents;
        contents = split_string(value, "=");

        if(contents.size() == 2){
            factoryResult = set_prop((contents[0]).c_str(), (contents[1]).c_str());
        } else {
            factoryResult.result = V1_1::IResultType_1_1::WRONG_PARAMS;
            ALOGI("set prop fail, wrong params");
        }
    } else if (strcmp(cmd.c_str(), "getprop") == 0){
        ALOGI("getprop:");
        factoryResult = get_prop(value.c_str());
    }

    _hidl_cb(factoryResult);
    return Void();
}

V1_1::FactoryResult Factory::set_prop(const char* prop, const char* value){
    ALOGI("set_prop: prop: %s, value: %s",prop, value);
    V1_1::FactoryResult factoryResult;
    factoryResult.result = V1_1::IResultType_1_1::SUCCESS;

    if (prop == NULL) {
        ALOGE("Failed to set prop (error NULL)");
        factoryResult.result = V1_1::IResultType_1_1::WRONG_PARAMS;
        return factoryResult;
    }

    int rc = property_set(prop, value);
    if (rc != 0){
        ALOGE("Failed to set %s as %s  (error %d)", prop, value, rc);
        factoryResult.result = V1_1::IResultType_1_1::FAILED;
        return factoryResult;
    }

    return factoryResult;
}

V1_1::FactoryResult Factory::get_prop(const char* prop){
    ALOGI("get_prop: prop: %s",prop);
    V1_1::FactoryResult factoryResult;
    factoryResult.result = V1_1::IResultType_1_1::SUCCESS;

    if (prop == NULL) {
        ALOGE("Failed to get prop (error NULL)");
        factoryResult.result = V1_1::IResultType_1_1::WRONG_PARAMS;
        return factoryResult;
    }

    char* buffer = new char[PROPERTY_VALUE_MAX];
    auto res = property_get(prop, buffer, NULL);
    ALOGI("get_prop: value: %s",buffer);

    if (res < 0) {
        ALOGE("Failed to get prop for %s  (error %d)", prop, res);
        factoryResult.result = V1_1::IResultType_1_1::FAILED;
        return factoryResult;
    }

    factoryResult.data = buffer;
    return factoryResult;
}


vector<string> Factory::split_string(const string& str, const string& divider){
    ALOGI("split_string:  string: %s, divider: %s", str.c_str(), divider.c_str());
    vector<string> tokens;
    size_t prev = 0;
    size_t pos = 0;
    do {
        pos = str.find(divider, prev);
        if (pos == string::npos) {
            pos = str.length();
        }
        string token = str.substr(prev, pos-prev);
        if (!token.empty()) {
            ALOGI("split content:  %s", token.c_str());
            tokens.push_back(token);
        }
        prev = pos + divider.length();
    }while (pos < str.length() && prev < str.length());

    return tokens;
}

IFactory* HIDL_FETCH_IFactory(const char* /* name */) {
    ALOGI("start to HIDL_FETCH_IFactory");
    return new Factory();
}

}
}  // namespace V1_0
}  // namespace factory
}  // namespace hardware
}  // namespace qti
}  // namespace vendor

