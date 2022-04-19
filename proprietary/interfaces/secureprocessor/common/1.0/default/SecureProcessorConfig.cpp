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

#include <vendor/qti/hardware/secureprocessor/device/1.0/ISecureProcessor.h>
#include <vendor/qti/hardware/secureprocessor/config/1.0/types.h>
#include "SecureProcessorConfig.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace secureprocessor {
namespace common {
namespace V1_0 {

/**
 * The concept for configuration marshaling and de-marshaling is borrowed
 * from AOSP Camera MetaData. References -
 *     Camera Metadata: platform/system/media/camera/src/camera_metadata.h[/c]
 *     Camera HAL: platform/hardware/interfaces/camera/metadata
 *
 * A packet of secure processor configuration which consists of a header
 * followed by config entries. Each entry has a offset for it's associated
 * data within this configuration packet.
 *
 * |------------------------------------------------|
 * |     ConfigHeader                               |
 * |------------------------------------------------|
 * |     ConfigBufferEntry -> 0th                   |
 * |------------------------------------------------|
 * |     ...                                        |
 * |------------------------------------------------|
 * |     ConfigBufferEntry -> (used - 1)th          |
 * |------------------------------------------------|
 * |     Empty = (limit - used) * ConfigBufferEntry |
 * |------------------------------------------------|
 * |     ConfigBufferData -> 0th                    |
 * |------------------------------------------------|
 * |     ...                                        |
 * |------------------------------------------------|
 * |     ConfigBufferData -> (used - 1)th           |
 * |------------------------------------------------|
 * |     Empty = (limit - used) * ConfigBufferData  |
 * |------------------------------------------------|
 *
 */

struct ConfigBase {
    uint32_t limit;
    uint32_t used;
    uint32_t start;
};

struct ConfigHeader {
    uint32_t size;
    uint32_t version;
    uint32_t flags;
    uint32_t index;
    ConfigBase entry;
    ConfigBase data;
};

struct ConfigBufferEntry {
    uint32_t tag;
    uint32_t count;
    union {
        uint32_t offset;
        uint8_t value[4];
    } data;
    uint8_t type;
    uint8_t reserved[3];
};

/**
 * ConfigBufferData is used to demonstrate the data marshalling
 * logic above in config buffer. This datatype is not used
 * otherwise in implementation.
 */
union ConfigBufferData {
    uint8_t u8;
    int32_t i32;
    int64_t i64;
};

#define MAX_ENTRY_LIMIT 256
#define MAX_DATA_LIMIT 1024
#define INVALID_INDEX (0xFFFFFFFF)
#define SECURE_PROCESSOR_CONFIG_VERSION 1

#define SIZE_OF_HEADER() \
    (sizeof(ConfigHeader))
#define SIZE_OF_ENTRIES(num) \
    (sizeof(ConfigBufferEntry) * (num))

/*
 * Version update is mandatory on interface change
 * Recommendation is to keep this module version in sync with
 * vendor.qti.hardware.secureprocessor.common package.
 * Rules for versioning:
 *   - Increment MINOR Number for interface extension with backward compatibility
 *   - Increment MAJOR Number for interface changes without backward compatibility
 */
#define MAJOR 1
#define MINOR 0

using ::vendor::qti::hardware::secureprocessor::config::V1_0::ConfigType;
using ::vendor::qti::hardware::secureprocessor::config::V1_0::ConfigTag;

#ifndef memcpy_s
size_t memcpy_s(void *dst, size_t dstSize, const void *src, size_t srcSize) {
    if (!dst || !src || (dstSize < srcSize)) {
        return 0;
    }
    memcpy(dst, src, srcSize);
    return srcSize;
}
#endif

SecureProcessorConfig::SecureProcessorConfig(uint32_t entryLimit,
                                             uint32_t dataLimit) {
    mVersion = ((MAJOR << 16) | MINOR);
    mConfigBuffer = _allocateConfigBuffer(entryLimit, dataLimit);
}

SecureProcessorConfig::SecureProcessorConfig(void *buffer) {
    mConfigBuffer = nullptr;
    mVersion = ((MAJOR << 16) | MINOR);

    if (!buffer) {
        return;
    }

    if (!_validateConfigBuffer(buffer)) {
        return;
    }

    mConfigBuffer = (ConfigHeader *) buffer;
}

SecureProcessorConfig::~SecureProcessorConfig() {
    {
        std::lock_guard<std::mutex> lock(mLock);
        if (mConfigBuffer) {
            free(mConfigBuffer);
        }
    }
}

void* SecureProcessorConfig::releaseAndGetBuffer() {
    void* buffer = nullptr;

    std::lock_guard<std::mutex> lock(mLock);
    if (mConfigBuffer) {
        buffer = mConfigBuffer;
        mConfigBuffer = nullptr;
    }

    return buffer;
}

uint32_t SecureProcessorConfig::getVersion() const {
    std::lock_guard<std::mutex> lock(mLock);
    return mVersion;
}

void SecureProcessorConfig::clear() {
    std::lock_guard<std::mutex> lock(mLock);

    if (!mConfigBuffer) {
        return;
    }

    uint32_t size = mConfigBuffer->size;
    uint32_t entryLimit = mConfigBuffer->entry.limit;
    uint32_t dataLimit = mConfigBuffer->data.limit;
    void *buffer = mConfigBuffer;

    std::memset(buffer, 0, size);
    _initConfigBuffer(buffer, size, entryLimit, dataLimit);
}

bool SecureProcessorConfig::isEmpty() const {
    std::lock_guard<std::mutex> lock(mLock);
    return _isEmpty();
}

uint32_t SecureProcessorConfig::getSize() const {
    std::lock_guard<std::mutex> lock(mLock);
    return _getSize();
}

uint32_t SecureProcessorConfig::entryCount() const {
    std::lock_guard<std::mutex> lock(mLock);
    return _entryCount();
}

bool SecureProcessorConfig::checkEntry(uint32_t tag) const {
    bool present = false;

    std::lock_guard<std::mutex> lock(mLock);

    if (!mConfigBuffer) {
        return present;
    }

    ConfigBufferEntry *entries = _getBufferEntries(mConfigBuffer);
    for (uint32_t i = 0; i < _entryCount(); i++) {
       if (entries[i].tag == tag) {
          present = true;
          break;
       }
    }

    return present;
}

status_t SecureProcessorConfig::addEntry(uint32_t tag,
                      const uint8_t *data,
                      uint32_t data_count) {
    uint8_t type = (uint8_t) ConfigType::BYTE;
    return _addEntry(tag, type, data, data_count);
}

status_t SecureProcessorConfig::addEntry(uint32_t tag,
                      const int32_t *data,
                      uint32_t data_count) {
    uint8_t type = (uint8_t) ConfigType::INT32;
    return _addEntry(tag, type, data, data_count);
}

status_t SecureProcessorConfig::addEntry(uint32_t tag,
                      const int64_t *data,
                      uint32_t data_count) {
    uint8_t type = (uint8_t) ConfigType::INT64;
    return _addEntry(tag, type, data, data_count);
}

status_t SecureProcessorConfig::addEntry(uint32_t tag,
                      const String8 &string) {
    uint8_t type = (uint8_t) ConfigType::BYTE;
    void *data = (void *) string.string();
    uint32_t data_count = string.size() + 1;
    return _addEntry(tag, type, data, data_count);
}

status_t SecureProcessorConfig::addEntry(uint32_t tag) {
    uint8_t type = (uint8_t) ConfigType::BYTE;
    uint8_t data = 0;
    return _addEntry(tag, type, &data, 1);
}

ConfigEntry SecureProcessorConfig::getEntry(uint32_t tag) {
    std::lock_guard<std::mutex> lock(mLock);
    return _getEntry(tag);
}

ConfigEntry SecureProcessorConfig::getEntryByIndex(uint32_t index) {
    std::lock_guard<std::mutex> lock(mLock);
    return _getEntryByIndex(index);
}

/* Private functions */
uint32_t SecureProcessorConfig::_computeSize(uint32_t entryLimit,
                                             uint32_t dataLimit) {
    uint32_t size = sizeof(ConfigHeader);
    size += (sizeof(ConfigBufferEntry) * entryLimit);
    size += (sizeof(uint8_t) * dataLimit);

    return size;
}

void SecureProcessorConfig::_initConfigBuffer(void *buffer,
                                              uint32_t size,
                                              uint32_t entryLimit,
                                              uint32_t dataLimit) {
    ConfigHeader *configBuffer = (ConfigHeader *) buffer;
    configBuffer->version = mVersion;
    configBuffer->flags = 0;
    configBuffer->index = 0;
    configBuffer->size = size;
    configBuffer->entry.limit = entryLimit;
    configBuffer->entry.used = 0;
    configBuffer->entry.start = SIZE_OF_HEADER();
    configBuffer->data.limit = dataLimit;
    configBuffer->data.used = 0;
    configBuffer->data.start = configBuffer->entry.start + SIZE_OF_ENTRIES(entryLimit);
}

ConfigHeader *SecureProcessorConfig::_allocateConfigBuffer(uint32_t entryLimit,
                                                           uint32_t dataLimit) {
    uint32_t size = 0;
    void *buffer = nullptr;

    if (entryLimit == 0 || entryLimit > MAX_ENTRY_LIMIT) {
        goto EXIT;
    }

    if (dataLimit == 0 || dataLimit > MAX_DATA_LIMIT) {
        goto EXIT;
    }

    size = _computeSize(entryLimit, dataLimit);
    buffer = calloc(1, size);
    if (buffer) {
        _initConfigBuffer(buffer, size, entryLimit, dataLimit);
    }

EXIT:
    return (ConfigHeader *) buffer;
}

bool SecureProcessorConfig::_validateConfigBuffer(void *buffer) {

    ConfigHeader *configBuffer = (ConfigHeader *) buffer;

    // Expects that caller has used correct version for marshalling
    if (configBuffer->version != mVersion) {
        return false;
    }

    if (configBuffer->flags != 0) {
        return false;
    }

    uint32_t size = _computeSize(configBuffer->entry.limit, configBuffer->data.limit);
    if (size != configBuffer->size) {
        return false;
    }

    if (configBuffer->entry.start != SIZE_OF_HEADER()) {
        return false;
    }

    if (configBuffer->data.start != (configBuffer->entry.start +
                                    SIZE_OF_ENTRIES(configBuffer->entry.limit))) {
        return false;
    }

    return true;
}

ConfigBufferEntry *SecureProcessorConfig::_getBufferEntries(ConfigHeader *configBuffer) const {
    return (ConfigBufferEntry *) (((uint8_t *) configBuffer) + configBuffer->entry.start);
}

uint8_t *SecureProcessorConfig::_getBufferData(ConfigHeader *configBuffer) const {
    return (((uint8_t *) configBuffer) + configBuffer->data.start);
}

uint8_t SecureProcessorConfig::_getDataTypeSize(uint8_t type) {
   uint8_t size = 0;
   ConfigType cfgType = (ConfigType) type;

   switch (cfgType) {
   case ConfigType::BYTE:
      size = sizeof(uint8_t);
      break;
   case ConfigType::INT32:
      size = sizeof(int32_t);
      break;
   case ConfigType::INT64:
      size = sizeof(int64_t);
      break;
   }

   return size;
}

status_t SecureProcessorConfig::_addEntry(uint32_t tag,
                                          uint8_t type,
                                          const void *data,
                                          uint32_t data_count) {
    if (!data || !data_count) {
        return BAD_VALUE;
    }

    std::lock_guard<std::mutex> lock(mLock);

    if (!mConfigBuffer) {
        return NO_INIT;
    }

    // Check if a new entry can be accomodated
    if (mConfigBuffer->entry.limit == mConfigBuffer->entry.used) {
        return NO_MEMORY;
    }

    // Check if the buffer passed is not from config buffer
    uint32_t size = _getSize();
    uintptr_t configBufferAddr = (uintptr_t) mConfigBuffer;
    uintptr_t inputAddr = (uintptr_t) data;

    if ((inputAddr >= configBufferAddr) &&
        (inputAddr < (configBufferAddr + size))) {
       return BAD_VALUE;
    }

    // Calculate needed data size
    uint32_t dataSize = data_count * _getDataTypeSize(type);
    if ((dataSize > 4) && (dataSize > (mConfigBuffer->data.limit - mConfigBuffer->data.used))) {
        // Resize needed (only dataLimit increase allowed)
        uint32_t entryLimit = mConfigBuffer->entry.limit;
        uint32_t dataLimit = mConfigBuffer->data.limit * 2;

        ConfigHeader *newConfigBuffer = _allocateConfigBuffer(entryLimit, dataLimit);
        if (!newConfigBuffer) {
            return NO_MEMORY;
        }

        // Transfer entries from current to new config buffer
        status_t status = _copyConfigBuffer(newConfigBuffer, mConfigBuffer);
        if (status != OK) {
            free(newConfigBuffer);
            return status;
        }

        // Free current config buffer and switch to newly allocated one
        free(mConfigBuffer);
        mConfigBuffer = newConfigBuffer;
    }

    // Check if entry is already present
    ConfigEntry entry = _getEntry(tag);
    if (entry.tag == tag) {
        // Entry already present, check it's type
       if (entry.type != type) {
          return BAD_TYPE;
       }

       // We are good to update the existing entry
        _updateBufferEntry(tag, data, data_count, dataSize);
    } else {
        // We are good to add new entry
        _createBufferEntry(tag, type, data, data_count, dataSize);
    }

    return OK;
}

void SecureProcessorConfig::_createBufferEntry(uint32_t tag,
                                               uint8_t type,
                                               const void *data,
                                               uint32_t data_count,
                                               uint32_t sizeBytes) {
    size_t copySize = 0;
    ConfigBufferEntry *entries = _getBufferEntries(mConfigBuffer);
    ConfigBufferEntry *newEntry = &entries[mConfigBuffer->entry.used];
    newEntry->tag = tag;
    newEntry->type = type;
    newEntry->count = data_count;
    if (sizeBytes > 4) {
        newEntry->data.offset = mConfigBuffer->data.used;
        copySize = memcpy_s((_getBufferData(mConfigBuffer) + newEntry->data.offset),
                            (mConfigBuffer->data.limit - mConfigBuffer->data.used),
                            data, sizeBytes);
        if (copySize == sizeBytes) {
            mConfigBuffer->data.used += copySize;
        }
    } else {
        copySize = memcpy_s(newEntry->data.value, 4, data, sizeBytes);
    }

    if (copySize == sizeBytes) {
        mConfigBuffer->entry.used++;
    }
}

void SecureProcessorConfig::_updateBufferEntry(uint32_t tag,
                                               const void *data,
                                               uint32_t data_count,
                                               uint32_t sizeBytes) {
    ConfigBufferEntry *entries = _getBufferEntries(mConfigBuffer);
    size_t copySize = 0;
    uint32_t index = _getEntryIndex(tag);
    if (index == INVALID_INDEX) {
        return;
    }

    ConfigBufferEntry *entry = &entries[index];

    // New data_count is of same size as of existing entry
    if (entry->count == data_count) {
        if (sizeBytes > 4) {
            copySize = memcpy_s((_getBufferData(mConfigBuffer) + entry->data.offset),
                                (entry->count * _getDataTypeSize(entry->type)),
                                data, sizeBytes);
        } else {
            copySize = memcpy_s(entry->data.value, 4, data, sizeBytes);
        }
    } else {
        // Remove data of existing entry and adjust offset of all other entries
        uint32_t entryDataSize = entry->count * _getDataTypeSize(entry->type);
        if (entryDataSize > 4) {
            uint8_t *start = _getBufferData(mConfigBuffer) + entry->data.offset;
            uint8_t *end = start + entryDataSize;
            uint32_t size = mConfigBuffer->data.used -
                            (entry->data.offset + entryDataSize);
            if (size) {
               memmove(start, end, size);
            }

            // Adjust offset accordingly
            for (uint32_t i = 0; i < _entryCount(); i++) {
                ConfigBufferEntry *tmpEntry = &entries[i];
                if ((tmpEntry->count * _getDataTypeSize(tmpEntry->type) > 4) &&
                    (tmpEntry->data.offset > entry->data.offset)) {
                    tmpEntry->data.offset -= entryDataSize;
                }
            }

            // Adjust data used
            mConfigBuffer->data.used -= entryDataSize;
        }

        // update new data and it's offset to the existing entry
        entry->count = data_count;
        if (sizeBytes > 4) {
            entry->data.offset = mConfigBuffer->data.used;
            copySize = memcpy_s((_getBufferData(mConfigBuffer) + entry->data.offset),
                                (mConfigBuffer->data.limit - mConfigBuffer->data.used),
                                data, sizeBytes);
            if (copySize == sizeBytes) {
                mConfigBuffer->data.used += copySize;
            }
        } else {
            copySize = memcpy_s(entry->data.value, 4, data, sizeBytes);
        }
    }
}

status_t SecureProcessorConfig::_copyConfigBuffer(ConfigHeader *dest,
                                                  ConfigHeader *src) {
    size_t copySize = 0;
    status_t status = OK;

    if (!dest || !src) {
        status = BAD_VALUE;
        goto exit;
    }

    // Check overall config buffer size
    if (dest->size < src->size) {
        status = NO_MEMORY;
        goto exit;
    }

    // Check Entry limit
    if (dest->entry.limit < src->entry.limit) {
        status = NO_MEMORY;
        goto exit;
    }

    // Check Buffer limit
    if (dest->data.limit < src->data.limit) {
        status = NO_MEMORY;
        goto exit;
    }

    // Check destination entry/buffer used
    // (expect destination to be empty)
    if (dest->entry.used || dest->data.used) {
        status = NO_MEMORY;
        goto exit;
    }

    dest->version = src->version;
    dest->flags = src->flags;
    dest->index = src->index;
    dest->entry.used = src->entry.used;
    dest->data.used = src->data.used;

    // Copy entries from src to destination
    copySize = memcpy_s(_getBufferEntries(dest),
                        SIZE_OF_ENTRIES(dest->entry.limit),
                        _getBufferEntries(src),
                        SIZE_OF_ENTRIES(src->entry.used));
    if (copySize != SIZE_OF_ENTRIES(src->entry.used)) {
        status = NO_MEMORY;
        goto exit;
    }

    // Copy data from src to destination
    copySize = memcpy_s(_getBufferData(dest), dest->data.limit,
                        _getBufferData(src), src->data.used);
    if (copySize != src->data.used) {
        status = NO_MEMORY;
        goto exit;
    }

exit:
    return status;
}

uint32_t SecureProcessorConfig::_getEntryIndex(uint32_t tag) {
    ConfigBufferEntry *entries = _getBufferEntries(mConfigBuffer);
    for (uint32_t i = 0; i < _entryCount(); i++) {
        if (entries[i].tag == tag) {
            return i;
        }
    }

    return INVALID_INDEX;
}

bool SecureProcessorConfig::_isEmpty() const {
    return (_entryCount() == 0);
}

uint32_t SecureProcessorConfig::_getSize() const {
    return ((mConfigBuffer == nullptr) ? 0 : mConfigBuffer->size);
}

uint32_t SecureProcessorConfig::_entryCount() const {
    return ((mConfigBuffer == nullptr) ? 0 : mConfigBuffer->entry.used);
}

ConfigEntry SecureProcessorConfig::_getEntry(uint32_t tag) {
    ConfigEntry entry = {};

    if (!mConfigBuffer) {
        return entry;
    }

    ConfigBufferEntry *entries = _getBufferEntries(mConfigBuffer);
    for (uint32_t i = 0; i < _entryCount(); i++) {
        if (entries[i].tag == tag) {
            entry.tag = entries[i].tag;
            entry.type = entries[i].type;
            entry.count = entries[i].count;
            if (entry.count * _getDataTypeSize(entry.type) > 4) {
                entry.data.u8 = (_getBufferData(mConfigBuffer) + entries[i].data.offset);
            } else {
                entry.data.u8 = entries[i].data.value;
            }
            break;
        }
    }

    return entry;
}

ConfigEntry SecureProcessorConfig::_getEntryByIndex(uint32_t index) {
    ConfigEntry entry = {};

    if (!mConfigBuffer) {
        return entry;
    }

    if (index >= _entryCount()) {
        return entry;
    }

    ConfigBufferEntry *entries = _getBufferEntries(mConfigBuffer);
    entry.tag = entries[index].tag;
    entry.type = entries[index].type;
    entry.count = entries[index].count;
    if ((entry.count * _getDataTypeSize(entry.type)) > 4) {
        entry.data.u8 = (_getBufferData(mConfigBuffer) + entries[index].data.offset);
    } else {
        entry.data.u8 = entries[index].data.value;
    }

    return entry;
}

}  // namespace V1_0
}  // namespace common
}  // namespace secureprocessor
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
