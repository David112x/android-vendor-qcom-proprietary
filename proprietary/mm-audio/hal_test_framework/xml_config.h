/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef XML_CONFIG_H_
#define XML_CONFIG_H_

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>

#include "characters_reader.hpp"
#include "characters_writer.hpp"
#include "xml_inspector.hpp"

namespace qti_hal_test
{
class XmlConfig
{
using XmlInspector = Xml::Inspector<Xml::Encoding::Utf8Writer>;

public:
    XmlConfig(std::string const & xml_config_file_name);

    // Invariant: All element-operations leave the window over
    // the next element, not the closing tag
    void MoveToNextElement();
    bool MoveToElement(std::string const & element_name);    // TO DO Why return bool and why different than MoveToNextElement()?
    bool DescendToElement(std::string const & element_name);
    bool DescendToElement(std::string const & element_name, std::string const & exit_on_name);

    bool IsMatch(std::string const & name);

    std::string GetValue();
    std::string GetName();

    std::string GetElementText();
    std::string GetElementText(std::string const & element_name);

    int64_t GetElementTextAsInt64_t(std::string const & element_name, // TO DO: Use template
                                    int base = 10);
    int32_t GetElementTextAsInt32_t(std::string const & element_name,
                                    int base = 10);
    int16_t GetElementTextAsInt16_t(std::string const & element_name,
                                    int base = 10);

    uint64_t GetElementTextAsUint64_t(std::string const & element_name,
                                      int base = 10);
    uint32_t GetElementTextAsUint32_t(std::string const & element_name,
                                      int base = 10);
    uint16_t GetElementTextAsUint16_t(std::string const & element_name,
                                      int base = 10);

    float GetElementTextAsFloat(std::string const & element_name);

    template<typename T>
    void GetElementAsVector(std::string const & container_element_name,
                            std::string const & element_name,
                            std::vector<T> & vec)
    {
        DescendToElement(container_element_name);
        while(DescendToElement(element_name, container_element_name))
        {
            vec.emplace_back(*this);
        }
    };

    void DisplayNode(std::string const & inspected) const;
    void DisplayParse();

private:
    bool FileExists(std::string const & file_name) const;
    void AdvancePastWhitespace();
    void ValidateParse() const;
    std::string RowAndColumn() const;

    XmlInspector inspector_;
};
}

#endif // XML_CONFIG_H_