/******************************************************************************
Copyright (c) 2019 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/
#include "Servicetracker.h"
#include <map>
#include <vector>
#include <hidl/HidlSupport.h>
#include <android-base/logging.h>
namespace vendor {
namespace qti {
namespace hardware {
namespace servicetracker {
namespace V1_1 {
namespace implementation {

using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;

using vendor::qti::hardware::servicetracker::V1_0::Status;
using vendor::qti::hardware::servicetracker::V1_0::ClientData;
using vendor::qti::hardware::servicetracker::V1_0::ServiceData;
using vendor::qti::hardware::servicetracker::V1_0::ClientConnection;
using vendor::qti::hardware::servicetracker::V1_0::ServiceConnection;
using vendor::qti::hardware::servicetracker::V1_0::ClientRecord;
using vendor::qti::hardware::servicetracker::V1_0::ServiceRecord;

struct service_record {
    hidl_string packageName;
    hidl_string processName;
    int32_t pid;
    bool serviceB;
    std::vector<ServiceConnection> conn;
    double lastActivity;
};

struct client_record {
    hidl_string processName;
    int32_t pid;
    std::vector<ClientConnection> conn;
};

//maps to store services and clients details
std::map<hidl_string, service_record> mServices;
std::map<hidl_string, client_record> mClients;

// used for synchronized access of map
std::mutex mLock;

//private APIs
void createServiceData(ServiceData,service_record*);
void createClientData(ClientData,client_record*);
void addServiceConnection(ClientData,service_record*);
void addClientConnection(ServiceData,client_record*);
void removeServiceConnection(hidl_string,service_record*);
void removeClientConnection(hidl_string,client_record*);

// Methods from ::vendor::qti::hardware::servicetracker::V1_0::IServicetracker follow.
Return<void> Servicetracker::startService(const ::vendor::qti::hardware::servicetracker::V1_0::ServiceData& serviceData) {
    std::lock_guard<std::mutex> lock(mLock);
    LOG(INFO) << "startService() is called for service" << serviceData.processName ;
    std::map<hidl_string, service_record>::iterator itr = mServices.find(serviceData.processName);
    if (itr != mServices.end()) {
        itr->second.lastActivity = serviceData.lastActivity;
        itr->second.serviceB = serviceData.serviceB;
        itr->second.pid = serviceData.pid;
        itr->second.processName = serviceData.processName;
    } else {
        service_record s;
        createServiceData(serviceData,&s);
        mServices.insert({serviceData.processName,s});
    }
    return Void();
}

void createServiceData(ServiceData s_data, service_record *s_record) {
    s_record->packageName = s_data.packageName;
    s_record->processName = s_data.processName;
    s_record->pid = s_data.pid;
    s_record->lastActivity = s_data.lastActivity;
    s_record->serviceB = s_data.serviceB;
}

void createClientData(ClientData c_data, client_record *c_record) {
    c_record->processName= c_data.processName;
    c_record->pid = c_data.pid;
}

Return<void> Servicetracker::bindService(const ::vendor::qti::hardware::servicetracker::V1_0::ServiceData& serviceData, const ::vendor::qti::hardware::servicetracker::V1_0::ClientData& clientData) {
    std::lock_guard<std::mutex> lock(mLock);
    LOG(INFO) << "bindService is called for service : " << serviceData.processName << " and for client " << clientData.processName;
    std::map<hidl_string, service_record>::iterator itr = mServices.find(serviceData.processName);
    if (itr != mServices.end()) {
        itr->second.lastActivity = serviceData.lastActivity;
        itr->second.serviceB = serviceData.serviceB;
        addServiceConnection(clientData,&itr->second);
    } else {
        service_record s;
        createServiceData(serviceData,&s);
        addServiceConnection(clientData,&s);
        mServices.insert({serviceData.processName,s});
    }
    std::map<hidl_string, client_record>::iterator itr_2 = mClients.find(clientData.processName);
    if (itr_2 != mClients.end()) {
        addClientConnection(serviceData,&itr_2->second);
    } else {
        client_record c;
        createClientData(clientData,&c);
        addClientConnection(serviceData,&c);
        mClients.insert({clientData.processName,c});
    }
    return Void();
}

void addClientConnection(ServiceData s_data,client_record *c_record) {
    bool flag = false;
    for (auto it = c_record->conn.begin(); it != c_record->conn.end(); it++) {
        if (it->serviceName == s_data.processName) {
            flag=true;
            it->count +=1;
            break;
        }
    }
    if (!flag) {
        //createconnection
        c_record->conn.resize(c_record->conn.size() + 1);
        auto it = c_record->conn.end() -1;
        it->serviceName = s_data.processName;
        it->servicePid = s_data.pid;
        it->count = 1;
    }
    LOG(INFO) << "total connections for client : "  << c_record->processName << "are :" << c_record->conn.size();
}

void addServiceConnection(ClientData c_data, service_record *s_record) {
    bool flag = false;
    for(auto it = s_record->conn.begin(); it != s_record->conn.end(); it++) {
        if (it->clientPid == c_data.pid) {
            flag = true;
            it->count +=1;
            break;
        }
    }
    if (!flag) {
        //createconnection
        s_record->conn.resize(s_record->conn.size() + 1);
        auto itr = s_record->conn.end() -1;
        itr->clientName = c_data.processName;
        itr->clientPid = c_data.pid;
        itr->count = 1;
    }
    LOG(INFO) << "total connections for service : " << s_record->processName << "are :" << s_record->conn.size();
}

Return<void> Servicetracker::unbindService(const ::vendor::qti::hardware::servicetracker::V1_0::ServiceData& serviceData, const ::vendor::qti::hardware::servicetracker::V1_0::ClientData& clientData) {
    std::lock_guard<std::mutex> lock(mLock);
    LOG(INFO) << "unbindService is called for service : " << serviceData.processName << " and for client " << clientData.processName;
    std::map<hidl_string, service_record>::iterator itr = mServices.find(serviceData.processName);
    if (itr != mServices.end()) {
        //only update the needed info and remove  the connection
        itr->second.lastActivity = serviceData.lastActivity;
        itr->second.serviceB = serviceData.serviceB;
        removeServiceConnection(clientData.processName,&itr->second);
    } else {
        //error this should not happen
        LOG(INFO) << "unbind service " << serviceData.processName << "not found";
    }
    std::map<hidl_string, client_record>::iterator itr_2 = mClients.find(clientData.processName);
    if (itr_2 != mClients.end()) {
        //remove the connection
        removeClientConnection(serviceData.processName,&itr_2->second);
    } else {
        //error this should not happen
        LOG(INFO) << "unbind client " << clientData.processName << "not  found";
    }
    return Void();
}

Return<void> Servicetracker::destroyService(const ::vendor::qti::hardware::servicetracker::V1_0::ServiceData& serviceData) {
    std::lock_guard<std::mutex> lock(mLock);
    LOG(INFO) << "destroyService is called for service : " << serviceData.processName ;
    std::map<hidl_string, service_record>::iterator itr = mServices.find(serviceData.processName);
    if (itr != mServices.end()) {
        for(auto it = ((itr->second).conn.begin()); it != ((itr->second).conn.end()); ++it) {
            std::map<hidl_string, client_record>::iterator itr_2 = mClients.find(it->clientName);
            if (itr_2 != mClients.end()) {
                removeClientConnection(serviceData.processName,&itr_2->second);
            }
        }
    //remove service entry from map
    mServices.erase(itr);
    } else {
     //error this should not happen
    LOG(INFO) << "destroy service " << serviceData.processName << "not  found";
    }
    return Void();
}

void removeServiceConnection(hidl_string c_name, service_record *s_record) {
    for(auto it = s_record->conn.begin(); it != s_record->conn.end(); it++) {
        if (it->clientName == c_name) {
            it->count -=1;
            //remove the element if count is 0 AND BREAK
            if (it->count ==0) {
                s_record->conn.erase(it);
                LOG(INFO) << "size of service connections for service: " << s_record->processName << "after removal is " << s_record->conn.size();
                break;
            }
        }
    }
}

void removeClientConnection(hidl_string s_name, client_record *c_record){
    for(auto it = c_record->conn.begin(); it != c_record->conn.end(); ++it) {
        if (it->serviceName == s_name) {
            it->count -=1;
            //remove the element if count is 0 AND BREAK
            if (it->count ==0) {
                c_record->conn.erase(it);
                LOG(INFO) << "size of client connections for client: " << c_record->processName << "after removal is " << c_record->conn.size();
                break;
            }
        }
    }
}
Return<void> Servicetracker::killProcess(int32_t pid) {
    //lets say it is a service than check its connection and for each connection remove the corresponding connection entry from its clients
    std::lock_guard<std::mutex> lock(mLock);
    LOG(INFO) << "killProcess is called for pid : " << pid ;
    for(auto itr = mServices.begin(); itr != mServices.end(); itr++) {
        if (itr->second.pid == pid){
            LOG(INFO) << "process with pid " << pid << " is service" ;
            for(auto it = ((itr->second).conn.begin()); it != ((itr->second).conn.end()); ++it) {
                std::map<hidl_string, client_record>::iterator itr_2 = mClients.find(it->clientName);
                if (itr_2 != mClients.end()) {
                    removeClientConnection(itr->second.processName,&itr_2->second);
                }
            }
        }
    }
    for(auto itr = mClients.begin(); itr != mClients.end();) {
        if (itr->second.pid == pid) {
            LOG(INFO) << "process with pid " << pid << " is client" ;
            for(auto it = ((itr->second).conn.begin()); it != ((itr->second).conn.end()); ++it) {
                std::map<hidl_string, service_record>::iterator itr_2 = mServices.find(it->serviceName);
                if (itr_2 != mServices.end()) {
                    removeServiceConnection(itr->second.processName,&itr_2->second);
                }
            }
            //remove client entry from list
            LOG(INFO) << "removing client with pid " <<itr->second.pid << "process name" << itr->second.processName  ;
            itr = mClients.erase(itr);
        } else {
            ++itr;
        }
    }
    return Void();
}

Return<void> Servicetracker::getclientInfo(const hidl_string& clientName, getclientInfo_cb _hidl_cb) {
    LOG(INFO) << "getclientInfo called for client: " << clientName;
    std::lock_guard<std::mutex> lock(mLock);
    std::map<hidl_string, client_record>::iterator itr = mClients.find(clientName);
    ClientRecord c;
    if (itr != mClients.end()) {
        c.processName = itr->second.processName;
        c.pid = itr->second.pid;
        c.conn = (hidl_vec<ClientConnection>)(itr->second.conn);
        _hidl_cb(Status::SUCCESS,c);
    } else {
        _hidl_cb(Status::ERROR_NOT_AVAILABLE,c);
    }
    return Void();
}

Return<void> Servicetracker::getserviceInfo(const hidl_string& serviceName, getserviceInfo_cb _hidl_cb) {
    LOG(INFO) << "getserviceInfo called for service: " << serviceName;
    std::lock_guard<std::mutex> lock(mLock);
    std::map<hidl_string, service_record>::iterator itr = mServices.find(serviceName);
    ServiceRecord s ;
    if (itr != mServices.end()) {
        s.packageName = itr->second.packageName;
        s.processName = itr->second.processName;
        s.pid = itr->second.pid;
        s.serviceB = itr->second.serviceB;
        s.lastActivity = itr->second.lastActivity;
        s.conn = (hidl_vec<ServiceConnection>)(itr->second.conn);
        _hidl_cb(Status::SUCCESS,s);
    } else {
        _hidl_cb(Status::ERROR_NOT_AVAILABLE,s);
    }
    return Void();
}

Return<void> Servicetracker::getServiceConnections(const hidl_string& serviceName, getServiceConnections_cb _hidl_cb) {
    LOG(INFO) << "getServiceConnections called for service: " << serviceName;
    std::lock_guard<std::mutex> lock(mLock);
    std::map<hidl_string, service_record>::iterator itr = mServices.find(serviceName);
    hidl_vec<ServiceConnection> s_conn;
    if (itr != mServices.end()) {
        s_conn = (hidl_vec<ServiceConnection>)(itr->second.conn);
        _hidl_cb(Status::SUCCESS,s_conn);
    } else {
        _hidl_cb(Status::ERROR_NOT_AVAILABLE,s_conn);
    }
    return Void();
}

Return<void> Servicetracker::getClientConnections(const hidl_string& clientName, getClientConnections_cb _hidl_cb) {
    LOG(INFO) << "getClientConnections called for client: " << clientName;
    std::lock_guard<std::mutex> lock(mLock);
    std::map<hidl_string, client_record>::iterator itr = mClients.find(clientName);
    hidl_vec<ClientConnection> c_conn;
    if (itr != mClients.end()) {
        c_conn = (hidl_vec<ClientConnection>)(itr->second.conn);
        _hidl_cb(Status::SUCCESS,c_conn);
    } else {
        _hidl_cb(Status::ERROR_NOT_AVAILABLE,c_conn);
    }
    return Void();
}

Return<void> Servicetracker::getPid(const hidl_string& processName, getPid_cb _hidl_cb) {
    LOG(INFO) << "getPid called for process: " << processName;
    std::lock_guard<std::mutex> lock(mLock);
    int32_t pid = -1;
    std::map<hidl_string, service_record>::iterator itr = mServices.find(processName);
    if (itr != mServices.end()) {
        pid = itr->second.pid;
        _hidl_cb(Status::SUCCESS,pid);
    } else {
        _hidl_cb(Status::ERROR_NOT_AVAILABLE,pid);
    }
    return Void();
}

Return<void> Servicetracker::getPids(const hidl_vec<hidl_string>& serviceList, getPids_cb _hidl_cb) {
    std::lock_guard<std::mutex> lock(mLock);
    std::vector<int32_t>servicePids;
    for(auto it = serviceList.begin(); it != serviceList.end(); ++it) {
        std::map<hidl_string, service_record>::iterator itr = mServices.find(*it);
        if (itr != mServices.end()) {
            servicePids.push_back(itr->second.pid);
        } else {
            servicePids.push_back(-1);
        }
    }
    _hidl_cb(Status::SUCCESS,(hidl_vec<int32_t>)(servicePids));
    return Void();
}

Return<void> Servicetracker::isServiceB(const hidl_string& serviceName, isServiceB_cb _hidl_cb) {
    std::lock_guard<std::mutex> lock(mLock);
    bool serviceB = false;
    std::map<hidl_string, service_record>::iterator itr = mServices.find(serviceName);
    if (itr != mServices.end()) {
        serviceB = itr->second.serviceB;
        _hidl_cb(Status::SUCCESS,serviceB);
    } else {
        _hidl_cb(Status::ERROR_NOT_AVAILABLE,serviceB);
    }
    return Void();
}

Return<void> Servicetracker::getServiceBCount(getServiceBCount_cb _hidl_cb) {
    std::lock_guard<std::mutex> lock(mLock);
    std::vector<ServiceRecord> bServiceList;
    int32_t count = 0;
    for(auto it = mServices.begin(); it != mServices.end(); ++it) {
        if (it->second.serviceB) {
            ServiceRecord s;
            s.packageName = it->second.packageName;
            s.processName = it->second.processName;
            s.pid = it->second.pid;
            s.serviceB = it->second.serviceB;
            s.lastActivity = it->second.lastActivity;
            s.conn = (hidl_vec<ServiceConnection>)(it->second.conn);
            bServiceList.push_back(s);
            count +=1;
        }
    }
    _hidl_cb(Status::SUCCESS,(hidl_vec<ServiceRecord>)bServiceList,count);
    return Void();
}


// Methods from ::vendor::qti::hardware::servicetracker::V1_1::IServicetracker follow.
Return<void> Servicetracker::getRunningServicePid(getRunningServicePid_cb _hidl_cb) {
    std::lock_guard<std::mutex> lock(mLock);
    std::vector<int32_t>runningPids;
    for(auto it = mServices.begin(); it != mServices.end(); ++it) {
        if (it->second.pid != -1) {
            runningPids.push_back(it->second.pid);
        }
    }
    _hidl_cb(Status::SUCCESS,(hidl_vec<int32_t>)(runningPids));
    return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

IServicetracker* HIDL_FETCH_IServicetracker(const char* /* name */) {
    return new Servicetracker();
}
//
}  // namespace implementation
}  // namespace V1_1
}  // namespace servicetracker
}  // namespace hardware
}  // namespace qti
}  // namespace vendor

