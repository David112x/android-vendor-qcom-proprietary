/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef DESERIALIZABLE_H_
#define DESERIALIZABLE_H_

#include "xml_config.h"

namespace qti_hal_test
{
class Deserializable // TO DO: Should be generic, with return type T of object being deserialized
{
public:
   virtual void Deserialize(XmlConfig & xml_config) = 0;

   virtual ~Deserializable();
};
}

#endif // DESERIALIZABLE_H_