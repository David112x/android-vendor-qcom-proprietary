/*
 * Copyright (c) 2017, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <capabilitytest.h>
using android::hardware::hidl_string;

using namespace android;
using CommandResult = ::vendor::qti::hardware::capabilityconfigstore::V1_0::CommandResult;
using Result = ::vendor::qti::hardware::capabilityconfigstore::V1_0::Result;
using ICapabilityConfigStore = ::vendor::qti::hardware::capabilityconfigstore::V1_0::ICapabilityConfigStore;

using namespace std::chrono;
using namespace std;

sp<ICapabilityConfigStore> service ;
void testCapConfigStoreService(const char* area, const char* config) {

    CommandResult Result1 ;
    Result1.result_type = Result::NOT_FOUND;
    service->getConfig(area, config, [&](const ::vendor::qti::hardware::capabilityconfigstore::V1_0::CommandResult& Result1){
    if(Result1.result_type != Result::NOT_FOUND){
        printf("getConfig: For functional_area := %s and config := %s  returned value := %s\n", area, config, Result1.value.c_str());
    }
    else {
        printf("getConfig: config not found");
    }
    });

}

int createCapConfigStoreClient() {
    service = ICapabilityConfigStore::getService();
    if(service) {
      return 0;
    }
    return 1;

}

int print_options(){

    printf("\nUsage ./capabilityconfigstoretest [options]\n");
    printf("Options\n");
    printf("     -h help  show this help message\n");
    printf("     functional_area config_name\n");
    printf("     functional_area config_name -t count\n");
    return -1;

}

int main( int argc, char* argv[]) {

    printf("Start capabilityonfigstorehaltest\n");

    if(argc < 3)
      return print_options();

    int count = 1;
    if( argv[3] != NULL &&  !strncmp(argv[3],"-t",2)) {
        if (argv[4]!= NULL){
          count = atoi(argv[4]);
         }
         else {
           return print_options();
         }
    }
    if(count < 1){
      count = 1;
    }
    const char* area = argv[1];
    const char* config_name = argv[2];
    int i=0;
    if(!createCapConfigStoreClient()){
       auto start_time = high_resolution_clock::now();
       do {
         testCapConfigStoreService(area, config_name);
         i++;
       }
       while(i < count);
       auto stop_time = high_resolution_clock::now();
       auto duration = duration_cast<milliseconds>(stop_time - start_time);
       printf("Time : %.2fms \n",(double)duration.count());
       return 0;
    }
    else {
      printf("Failed to initialize the service, bailing out");
      return -1;
    }

}
