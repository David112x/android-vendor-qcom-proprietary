/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "command_line_parser.h"

namespace qti_hal_test
{
CommandLineParser::CommandLineParser()
{}

CommandLineParser::CommandLineParser(int argc, char const * const argv[])
{
    Load(argc, argv);
}

#if 0
CommandLineParser::CommandLineParser(CommandLineParser const & rhs)
{
    if (this != &rhs)
    {
        command_line_parser_lut_ = rhs.command_line_parser_lut_;
    }
}
#endif // 0


CommandLineParser::~CommandLineParser()
{}

void CommandLineParser::Load(int argc, char const * const argv[])
{
    std::vector<std::string> arg_list;

    for (int i = 1; i < argc; ++i)
    {
        arg_list.push_back(argv[i]);
    }

    Parse(arg_list);
}

bool CommandLineParser::Find(std::string const & key) const
{
    if (command_line_parser_lut_.count(key))
    {
        return true;
    }

    return false;
}

bool CommandLineParser::Find(std::string const & key, std::string & value) const
{
    if (command_line_parser_lut_.count(key))
    {
        try
        {
            value = command_line_parser_lut_.at(key);  // Research map::find
        }
        catch (std::out_of_range const & e)
        {
            return false;
        }

        return true;
    }

    return false;
}

size_t CommandLineParser::NumPairs() const noexcept
{
    return command_line_parser_lut_.size();
}

CommandLineParser::ConstIterator CommandLineParser::begin() const
{
    return command_line_parser_lut_.begin();
}
CommandLineParser::ConstIterator CommandLineParser::end() const
{
    return command_line_parser_lut_.end();
}

void CommandLineParser::Display() const
{
    for (auto const & a : command_line_parser_lut_)
    {
        std::cout << a.first << "    " << a.second << std::endl;
    }
}

void CommandLineParser::Parse(std::vector<std::string> const& arg_list)
{
    int arg_index = 0;
    while (arg_index < static_cast<int>(arg_list.size()))
    {
        int args_remaining = arg_list.size() - arg_index;

        std::string const & current_arg = arg_list[arg_index];

        if (args_remaining > 1)
        {
            std::string const & next_arg = arg_list[arg_index + 1];

            if (HasLeadingDash(current_arg) && !HasLeadingDash(next_arg))
            {
                Insert(current_arg, next_arg);
                arg_index += 2;
            }
            else if (HasLeadingDash(current_arg) && HasLeadingDash(next_arg))
            {
                Insert(current_arg, "true");
                arg_index += 1;
            }
            else
            {
                throw std::runtime_error{ "Invalid command line." };
            }
        }
        else
        {
            if (HasLeadingDash(current_arg))
            {
                Insert(current_arg, "true");
                arg_index += 1;
            }
            else
            {
                throw std::runtime_error{ "Invalid command line." };
            }
        }
    }

#if 1
    for (auto const & a : command_line_parser_lut_)
    {
        std::cout << a.first << "    " << a.second << std::endl;
    }
#endif

}

bool CommandLineParser::HasLeadingDash(std::string const & arg) const
{
    if (arg[0] == '-')
    {
        return true;
    }

    return false;
}

void CommandLineParser::Insert(std::string const & key,
    std::string const & value)
{
    if (!command_line_parser_lut_.count(key))
    {
        command_line_parser_lut_[key] = value;
    }
    else
    {
        std::ostringstream error_msg;
        error_msg << std::endl << "Duplicate key: " << key;

        throw std::runtime_error{ error_msg.str() };
    }
}
}