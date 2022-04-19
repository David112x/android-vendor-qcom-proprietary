/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "metadata.h"

namespace qti_hal_test
{
Metadata::Metadata(XmlConfig & xml_config)
{
    Deserialize(xml_config);
}

Metadata::Metadata(std::vector<std::string> const & keys,
                   std::string const & values_as_string)
{
    Load(keys, values_as_string);
}

Metadata::Metadata(CommandLineParser const & command_line_parser,
    std::vector<std::string> const & keys)
{
    std::string values_as_string;

    ParseCommandLine(command_line_parser, values_as_string);

    Load(keys, values_as_string);
}

Metadata::~Metadata()
{
}

void Metadata::Deserialize(XmlConfig & xml_config)
{
    // Convert xml_config strem into kvps_

#if 1
    std::string tmp = xml_config.GetName();
    while (xml_config.MoveToElement("Pair"))
    {
        Pair pair(xml_config);

        kvps_.push_back(pair);
    }
#else
    std::string tmp = xml_config.GetName();
    xml_config.MoveToElement("Pair");  // No pair here seems like error condition
    while (xml_config.IsMatch("Pair"))
    {
        Pair pair(xml_config);

        kvps_.push_back(pair);

        xml_config.MoveToElement("Pair");
    }
#endif
}

void Metadata::Load(std::vector<std::string> const & keys,
                    std::string const & values_as_string)
{
    std::vector<std::string> values = PartitionValuesString(values_as_string);

    if (values.size() != keys.size())
    {
        std::ostringstream error_msg;
        error_msg << "Incorrect number of keys. Expected " << keys.size() << ", received " << values.size();

        throw std::runtime_error(error_msg.str());
    }

#if 1
    else
    {
        std::cout << "Expected " << keys.size()
                  << " keys, received " << values.size() << std::endl;
    }
#endif

#if 1
    std::cout << "keys (" << keys.size() << ")" << std::endl;
    for (auto key : keys)
    {
        std::cout << key << std::endl;
    }

    std::cout << "values (" << values.size() << ")" << std::endl;
    for (auto value : values)
    {
        std::cout << value << std::endl;
    }
#endif

    for (size_t i = 0; i < values.size(); ++i)
    {
        int value = 0;
        try
        {
            value = stoi(values[i]);
        }
        catch (std::logic_error const & e) // stoi can throw either invalid_argument or out_of_range.
        {
            throw std::runtime_error{ "Metadata could not convert value of key-value pair to int." };
        }

        Pair kvp(keys[i], value);
        kvps_.push_back(kvp);
    }
}

void Metadata::ParseCommandLine(CommandLineParser const & command_line_parser,
                                std::string & values_as_string)
{
    std::string argument;

    if (!command_line_parser.Find("-metadata", argument))
    {
        throw std::runtime_error{ "metadata not specified" };
    }

    values_as_string = argument;
}

std::vector<std::string> Metadata::PartitionValuesString(std::string const & values_as_string) const
{
    std::string tmp_values_as_string{values_as_string};

    std::replace(tmp_values_as_string.begin(), tmp_values_as_string.end(), ',', ' ');

    //std::cout << "values_as_string with commas replaced by spaces: " << values_as_string << std::endl;

    std::istringstream values_stream(tmp_values_as_string);

    std::vector<std::string> values;

    std::copy(std::istream_iterator<std::string>(values_stream),
        std::istream_iterator<std::string>(),
        std::back_inserter(values));

    return values;
}

std::string Metadata::GetPairs() const  // TO DO: Why reconstruct every time?
{

    std::ostringstream strs;

    for (size_t i = 0; i < kvps_.size(); ++i)
    {
        strs << kvps_[i].key_ << "=" << kvps_[i].value_ << ";";  // TO DO: trailing semicolon ok?
    }

    return strs.str();
}

void Metadata::Display() const
{
    for (size_t i = 0; i < kvps_.size(); ++i)
    {
        std::cout << kvps_[i].key_ << "=" << kvps_[i].value_ << ";" << std::endl;
    }
}
}