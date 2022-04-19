/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "xml_config.h"

namespace qti_hal_test
{
XmlConfig::XmlConfig(std::string const & xml_config_file_name) :
    inspector_{ xml_config_file_name }  // TO DO: This form of initialization does not return an error if the file does not exist.
{
    if (!FileExists(xml_config_file_name))
    {
        std::ostringstream error_msg;
        error_msg << std::endl << "File " << xml_config_file_name << " not found.";

        throw std::runtime_error{ error_msg.str() };
    }
}

bool XmlConfig::FileExists(std::string const & file_name) const
{
    std::ifstream file(file_name);

    bool file_exists = file.good();

    file.close();

    return file_exists;
}

// Find next start tag and park the stream window on it.
void XmlConfig::MoveToNextElement()
{
    while (inspector_.Inspect())
    {
        switch (inspector_.GetInspected())
        {
        case Xml::Inspected::StartTag:
            return;
        }
    }

    ValidateParse();
}

// Find start tag element_name and park the stream window on it.
// No interceding start tags are accepted.
bool XmlConfig::MoveToElement(std::string const & element_name)
{
    if (inspector_.GetInspected() == Xml::Inspected::StartTag &&
        inspector_.GetName() == element_name) // Window is already directly on top of element_name
    {
        return true;
    }

    bool found = false;
    bool done = false;
    while (!done && inspector_.Inspect())
    {
        switch (inspector_.GetInspected())
        {
        case Xml::Inspected::StartTag:
            std::string tmp = inspector_.GetName();
            if (inspector_.GetName() == element_name)
            {
                found = true;
            }
            else
            {
                found = false;
            }
            done = true;
            break;
        }
    }

    ValidateParse();

    return found;
}

// Find start tag element_name and park the stream window on it.
// Interceding start tags are accepted.
bool XmlConfig::DescendToElement(std::string const & element_name)
{
    if (inspector_.GetInspected() == Xml::Inspected::StartTag &&
        inspector_.GetName() == element_name) // Window is already directly on top of element_name
    {
        return true;
    }

    bool found = false;
    bool done = false;
    while (!done && inspector_.Inspect())
    {
        switch (inspector_.GetInspected())
        {
        case Xml::Inspected::StartTag:  // TO DO: No need for a switch here
            std::string tmp = inspector_.GetName();
            if (inspector_.GetName() == element_name)
            {
                found = true;
                done = true;
                break;
            }
        }
    }

    ValidateParse();

    return found;
}

// Find start tag element_name and park the stream window on it.
// Interceding start tags are accepted.
bool XmlConfig::DescendToElement(std::string const & element_name, std::string const & exit_on_name)
{
    if (inspector_.GetInspected() == Xml::Inspected::StartTag &&
        inspector_.GetName() == element_name) // Window is already directly on top of element_name
    {
        return true;
    }

    bool found = false;
    bool done = false;
    while (!done && inspector_.Inspect())
    {
        std::string tmp{};

        switch (inspector_.GetInspected())
        {
        case Xml::Inspected::StartTag:
            tmp = inspector_.GetName();
            if (inspector_.GetName() == element_name)
            {
                found = true;
                done = true;
                break;
            }
        case Xml::Inspected::EndTag:
            tmp = inspector_.GetName();
            if (inspector_.GetName() == exit_on_name)
            {
                found = false;
                done = true;
                break;
            }
        }
    }

    ValidateParse();

    return found;
}

bool XmlConfig::IsMatch(std::string const & elementName)
{
    bool is_match = inspector_.GetInspected() == Xml::Inspected::StartTag && // TO DO: May be overly restrictive
                                        inspector_.GetName() == elementName;
    ValidateParse();

    return is_match;
}

std::string XmlConfig::GetValue()
{
    return inspector_.GetValue();
}

std::string XmlConfig::GetName()
{
    return inspector_.GetName();
}

// Precondition:  Parse window has been positioned using MoveToElement.  Note
// that this function will leave the window on the closing tag
std::string XmlConfig::GetElementText()
{
    std::string text;

    bool done = false;
    while (!done && inspector_.Inspect())
    {
        switch (inspector_.GetInspected())
        {
        case Xml::Inspected::Text:
            text = inspector_.GetValue();
            break;
        case Xml::Inspected::EndTag:
            done = true;
            break;
        }
    }

    ValidateParse();

    return text;
}

std::string XmlConfig::GetElementText(std::string const & element_name)
{
    if (!MoveToElement(element_name))
    {
        std::ostringstream error_msg;
        error_msg << "Failed to move to element " << element_name;
        error_msg << RowAndColumn();
        throw std::runtime_error{ error_msg.str() };
    }

    return GetElementText(); // TO DO: A more symmetrical design would advance past end tag here.
}

int64_t XmlConfig::GetElementTextAsInt64_t(std::string const & element_name,
                                           int base /* = 10 */)
{
    int64_t value = 0;

    try
    {
        value = std::stoll(GetElementText(element_name), nullptr, base);
    }
    catch (std::logic_error const & e) // stoi can throw either invalid_argument or out_of_range.
    {
        std::ostringstream error_msg;
        error_msg << "Failed to convert value of XML element \"" << element_name << "\" to int. ";
        error_msg << RowAndColumn();
        throw std::runtime_error{ error_msg.str() };
    }

    return value;
}

int32_t XmlConfig::GetElementTextAsInt32_t(std::string const & element_name,
                                           int base /* = 10 */)
{
    return static_cast<int32_t>(GetElementTextAsInt64_t(element_name, base));
}

int16_t XmlConfig::GetElementTextAsInt16_t(std::string const & element_name,
    int base /* = 10 */)
{
    return static_cast<int16_t>(GetElementTextAsInt64_t(element_name, base));
}

uint64_t XmlConfig::GetElementTextAsUint64_t(std::string const & element_name,
    int base /* = 10 */)
{
    uint64_t value{ 0 };

    try
    {
        value = std::stoull(GetElementText(element_name), nullptr, base);
    }
    catch (std::logic_error const & e) // stoi can throw either invalid_argument or out_of_range.
    {
        std::ostringstream error_msg;
        error_msg << "Failed to convert value of XML element \"" << element_name << "\" to int. ";
        error_msg << RowAndColumn();
        throw std::runtime_error{ error_msg.str() };
    }

    return value;
}

uint32_t XmlConfig::GetElementTextAsUint32_t(std::string const & element_name,
    int base /* = 10 */)
{
    return static_cast<uint32_t>(GetElementTextAsUint64_t(element_name, base));
}

uint16_t XmlConfig::GetElementTextAsUint16_t(std::string const & element_name,
    int base /* = 10 */)
{
    return static_cast<uint16_t>(GetElementTextAsUint64_t(element_name, base));
}

float XmlConfig::GetElementTextAsFloat(std::string const & element_name)
{
    return std::stof(GetElementText(element_name), nullptr);     // TO DO: stof can throw an exception; isolate
}

void XmlConfig::DisplayParse()
{
    while (inspector_.Inspect())
    {
        switch (inspector_.GetInspected())
        {
        case Xml::Inspected::Whitespace:
            DisplayNode("Whitespace");
            break;
        case Xml::Inspected::StartTag:
            DisplayNode("StartTag");
            break;
        case Xml::Inspected::EndTag:
            DisplayNode("EndTag");
            break;
        case Xml::Inspected::Text:
            DisplayNode("Text");
            break;
        case Xml::Inspected::XmlDeclaration:
            DisplayNode("XmlDeclaration");
            break;
        case Xml::Inspected::Comment:
            DisplayNode("Comment");
            break;
        case Xml::Inspected::CDATA:
            DisplayNode("CDATA");
            break;
        case Xml::Inspected::EmptyElementTag:
            DisplayNode("EmptyElementTag");
            break;
        case Xml::Inspected::ProcessingInstruction:
            DisplayNode("ProcessingInstruction");
            break;
        case Xml::Inspected::EntityReference:
            DisplayNode("EntityReference");
            break;
        case Xml::Inspected::None:
            DisplayNode("None");
            break;
        case Xml::Inspected::DocumentType:
            DisplayNode("DocumentType");
            break;
        }
    }

    ValidateParse();
}

void XmlConfig::DisplayNode(std::string const & inspected) const
{
    std::cout << "Node type: " << inspected << std::endl
              << "    Name: " << inspector_.GetName() << std::endl;

    if (inspector_.GetValue() == "\n")
    {
        std::cout << "    Value: " << "\\n" << std::endl;
    }
    else if (inspector_.GetValue() == "")
    {
        std::cout << "    Value: " << "(empty string)" << std::endl;
    }
    else
    {
        std::cout << "    Value: " << inspector_.GetValue() << std::endl;
    }

    std::cout << "    Row: " << inspector_.GetRow() << ", "
              << "Column: " << inspector_.GetColumn() << std::endl;

    std::cout << std::endl;
}

void XmlConfig::AdvancePastWhitespace()
{
    bool done = false;
    while (!done)
    {
        if (inspector_.GetInspected() != Xml::Inspected::Whitespace)
        {
            inspector_.Inspect();
        }
        else
        {
            done = true;
        }
    }

    ValidateParse();
}

void XmlConfig::ValidateParse() const
{
    if (inspector_.GetErrorCode() != Xml::ErrorCode::None)
    {
        std::cout << inspector_.GetErrorMessage()
            << RowAndColumn()
            << std::endl;

        throw std::runtime_error{ "Parse error" };
    }
}

std::string XmlConfig::RowAndColumn() const
{
    std::ostringstream row_col;

    row_col  << " row: " << inspector_.GetRow() << ","
             << " column: " << inspector_.GetColumn();

    return row_col.str();
}
}