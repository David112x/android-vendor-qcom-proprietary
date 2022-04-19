/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef METADATA_H_
#define METADATA_H_

#include <algorithm>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>

#include "command_line_parser.h" // TO DO: Remove this eventually.
#include "deserializable.h"
#include "xml_config.h"

namespace qti_hal_test
{
class Metadata : public Deserializable
{
public:
    Metadata() = delete;

    Metadata(XmlConfig & xml_config);

    Metadata(std::vector<std::string> const & keys,
             std::string const & values_as_string);

    Metadata(CommandLineParser const & command_line_parser,
             std::vector<std::string> const & keys);

    ~Metadata();

    void Deserialize(XmlConfig & xml_config) override;

    std::string GetPairs() const; // Note also that this currently forces clients
                                  // to call .c_str() on the returned value.
    void Display() const;

private:
    // TO DO: IMPORTANT: Pair class assumes value is always in int! Confirm.
    struct Pair : public Deserializable // TO DO: Likely a STL type that's a better fit.
    {
        std::string key_;
        int value_;

        Pair(std::string key, int value) :
            key_{ key },
            value_{ value }
        { };

        Pair(XmlConfig & xml_config)
        {
            Deserialize(xml_config);
        };

        void Deserialize(XmlConfig & xml_config) override
        {
            key_ = xml_config.GetElementText("Key");
            value_ = xml_config.GetElementTextAsInt32_t("Value");
        }
    };

    void Load(std::vector<std::string> const & keys,
              std::string const & values_as_string);

    void ParseCommandLine(CommandLineParser const & command_line_parser,
                          std::string & values_as_string);

    std::vector<std::string> PartitionValuesString(std::string const & values_as_string) const;

    std::vector<Pair> kvps_;
};
}
#endif // METADATA_H_
