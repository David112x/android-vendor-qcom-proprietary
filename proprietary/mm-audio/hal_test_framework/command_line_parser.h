/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef COMMAND_LINE_PARSER_H_
#define COMMAND_LINE_PARSER_H_

#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace qti_hal_test
{
class CommandLineParser
{
public:
    CommandLineParser();
    CommandLineParser(int argc, char const * const argv[]);
    //explicit CommandLineParser(CommandLineParser const & rhs);

    ~CommandLineParser();

    void Load(int argc, char const * const argv[]);

    bool Find(std::string const & key) const;
    bool Find(std::string const & key, std::string & value) const;

    size_t NumPairs() const noexcept;

    using CommandLineParserLUT = std::map<std::string, std::string>;

    using ConstIterator = CommandLineParserLUT::const_iterator;
    ConstIterator begin() const;
    ConstIterator end() const;

    void Display() const;

private:
    void Parse(std::vector<std::string> const& arg_list);
    bool HasLeadingDash(std::string const & arg) const;
    void Insert(std::string const & key, std::string const & value);

    CommandLineParserLUT command_line_parser_lut_;
};
}

#endif // COMMAND_LINE_PARSER_H_