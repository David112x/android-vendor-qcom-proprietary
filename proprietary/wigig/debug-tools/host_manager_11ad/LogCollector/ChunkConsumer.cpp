/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/


#include <string>
#include "ChunkConsumer.h"


const std::string log_collector::ChunkConsumer::BufferOverrunMessage = "\nhost_manager_11ad error: Found buffer overrun - missed DWORDS: ";
const std::string log_collector::ChunkConsumer::DeviceRestartedMessage = "\nhost_manager_11ad error: Device was restarted \n";
const std::string log_collector::ChunkConsumer::ReadOverrunMessage = "\nhost_manager_11ad error: Read more lines than were created \n";
const std::string log_collector::ChunkConsumer::CorruptedEntryMessage = "\nhost_manager_11ad error: Got corrupted entry, signature: ";
const std::string log_collector::ChunkConsumer::DeviceRemovedMessage = "\nhost_manager_11ad error: Device was removed! \n";
const std::string log_collector::ChunkConsumer::DeviceDiscoveredMessage = "\nhost_manager_11ad error: Device was discovered! \n";
const std::string log_collector::ChunkConsumer::TracerWasRestartedMessage = "\nhost_manager_11ad error: Tracer was restarted \n";