/*
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 *
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SECURE_PROCESSOR_1_0_CONFIG_H
#define SECURE_PROCESSOR_1_0_CONFIG_H

#include <mutex>
#include <utils/Errors.h>
#include <utils/String8.h>
#include <utils/Vector.h>

using namespace android;

namespace vendor {
namespace qti {
namespace hardware {
namespace secureprocessor{
namespace common {
namespace V1_0 {

struct ConfigHeader;
struct ConfigBufferEntry;

/**
 * The concept for configuration marshaling and de-marshaling is borrowed from
 * AOSP Camera MetaData. References -
 *     Camera Metadata: platform/system/media/camera/src/camera_metadata.h[/c]
 *     Camera HAL: platform/hardware/interfaces/camera/metadata
 *
 */

/**
 * ConfigEntry
 *   This structure is used to return the entry present in the
 *   config buffer. It consists of TAG, it's TYPE, sizeof(data)
 *   and DATA.
 */
struct ConfigEntry {
    uint32_t tag;
    uint8_t type;
    uint32_t count;
    union {
        uint8_t *u8;
        int32_t *i32;
        int64_t *i64;
    } data;
};

/**
  * Secure memcpy
  *   Copy source buffer content to destination buffer only if
  *   size permits.
  *
  * @param dst Destination buffer
  * @param dstSize Destination buffer size
  * @param src Source buffer
  * @param srcSize Source buffer size
  *
  * @return Number of bytes copied succesfully.
  */
#ifndef memcpy_s
size_t memcpy_s(void *dst, size_t dstSize, const void *src, size_t srcSize);
#endif

/**
 * SecureProcessorConfig class
 *   This class is a helper class to marshal and de-marshal the
 *   secure processor configurations to the backing buffer. The
 *   backing buffer can be sent over hidl interface to hal
 *   service and vice-versa for setting and getting the
 *   supported configurations. It expects each configuration to
 *   be a (tag, value) pair. The same class provide the
 *   interface for marshaling (client side) and de-marshaling
 *   (service side) of configurations for a setConfig operation.
 */
class SecureProcessorConfig {
  public:
   /**
    * SecureProcessorConfig
    *   Create secure processor config helper class with backing
    *   buffer to hold requested number of entries and data size.
    *   This constructor shall be used for marshaling requirements.
    *
    * @param entryLimit Max number of entries required
    * @param dataLimit Data size required (depends on config TAGs
    *                  to be added)
    */
    SecureProcessorConfig(uint32_t entryLimit, uint32_t dataLimit = 64);

   /**
    * SecureProcessorConfig
    *   Create secure processor config helper class with an
    *   existing backing buffer. This constructor shall be used for
    *   de-marshaling requirements.
    *
    * @param buffer Backing buffer containing configuration data
    */
    SecureProcessorConfig(void *buffer);

   /**
    * ~SecureProcessorConfig
    *   Destructor for secure processor config helper class.
    */
    ~SecureProcessorConfig();

   /**
    * addEntry
    *   Marshal the requested (tag, value) pair and create an entry
    *   in the backing buffer. This API supports various data
    *   types.
    *
    * @param tag Configuration identifier (tag)
    * @param data Configuration data (value)
    * @param data_count Size of configuration data (in specified
    *                   data type; not bytes)
    *
    * @return status_t Return status of operation
    *   OK: entry created successfully,
    *   BAD_VALUE: Invalid argument,
    *   NO_INIT: No backing buffer present,
    *   NO_MEMORY: Insufficient memory to add new entry,
    *   BAD_TYPE: Entry is already present with different datatype.
    */
    status_t addEntry(uint32_t tag, const uint8_t *data,
                      uint32_t data_count);
    status_t addEntry(uint32_t tag, const int32_t *data,
                      uint32_t data_count);
    status_t addEntry(uint32_t tag, const int64_t *data,
                      uint32_t data_count);
    status_t addEntry(uint32_t tag, const String8 &string);
    status_t addEntry(uint32_t tag);

   /**
    * releaseAndGetBuffer
    *   Once all the configuration entries are marshalled in the
    *   backing buffer, client can take the ownership of the
    *   buffers using this API and the buffer is detached fron the
    *   this helper class object. Client can further send the
    *   backing buffer over HIDL to hal service (and vice versa).
    *
    * @return void* Backing buffer pointer.
    */
    void* releaseAndGetBuffer();

   /**
    * getEntry
    *   Retrieve a configuration entry using tag as key from
    *   backing buffer and demarshal it for client processing.
    *
    * @param tag Configuration identifier (tag)
    *
    * @return ConfigEntry Configuration entry containing (tag,
    *         value) pair.
    */
    ConfigEntry getEntry(uint32_t tag);

   /**
    * getEntryByIndex
    *    Retrieve configuration entries from backing buffer one by
    *    one using the index and demarshal it for client
    *    processing.
    *
    * @param index Index from which entry to be retrieved
    *
    * @return ConfigEntry Configuration entry containing (tag,
    *         value) pair.
    */
    ConfigEntry getEntryByIndex(uint32_t index);

   /**
    * Following are helper functions
    *   getVersion: get version
    *   clear: clear backing buffer
    *   isEmpty: check if any entry is present or not
    *   getSize: get the size of backing buffer in bytes
    *   entryCount: number of entries present in backing buffer
    *   checkEntry: check if a perticular entry is already present
    */
    uint32_t getVersion() const;
    void clear();
    bool isEmpty() const;
    uint32_t getSize() const;
    uint32_t entryCount() const;
    bool checkEntry(uint32_t tag) const;

  private:
    ConfigHeader *mConfigBuffer;
    mutable std::mutex mLock;

    uint32_t _computeSize(uint32_t entryLimit, uint32_t dataLimit);
    void _initConfigBuffer(void *buffer, uint32_t size, uint32_t entryLimit,
                           uint32_t dataLimit);
    ConfigHeader *_allocateConfigBuffer(uint32_t entryLimit, uint32_t dataLimit);
    bool _validateConfigBuffer(void *buffer);
    uint8_t _getDataTypeSize(uint8_t type);
    ConfigBufferEntry *_getBufferEntries(ConfigHeader *configBuffer) const;
    uint8_t *_getBufferData(ConfigHeader *configBuffer) const;
    status_t _addEntry(uint32_t tag, uint8_t type, const void *data,
                       uint32_t data_count);
    void _createBufferEntry(uint32_t tag, uint8_t type,
                            const void *data, uint32_t data_count,
                            uint32_t sizeBytes);
    void _updateBufferEntry(uint32_t tag, const void *data,
                            uint32_t data_count, uint32_t sizeBytes);
    status_t _copyConfigBuffer(ConfigHeader *dest, ConfigHeader *src);
    uint32_t _getEntryIndex(uint32_t tag);
    bool _isEmpty() const;
    uint32_t _getSize() const;
    uint32_t _entryCount() const;
    ConfigEntry _getEntry(uint32_t tag);
    ConfigEntry _getEntryByIndex(uint32_t index);
    /*
     * MAJOR Number: Upper 16 bits
     * MINOR Number: Lower 16 bits
     */
    uint32_t mVersion;
};

} // namespace V1_0
} // namespace common
} // namespace secureprocessor
} // namespace hardware
} // namespace qti
} // namespace vendor

#endif
