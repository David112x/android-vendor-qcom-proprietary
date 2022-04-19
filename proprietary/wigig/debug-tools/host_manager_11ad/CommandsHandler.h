/*
* Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
/*
* Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above
*       copyright notice, this list of conditions and the following
*       disclaimer in the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of The Linux Foundation nor the names of its
*       contributors may be used to endorse or promote products derived
*       from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
* ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
* IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _COMMANDSHANDLER_H_
#define _COMMANDSHANDLER_H_

#include <map>
#include <string>

#include "Utils.h"
#include "HostDefinitions.h"

class Host;

// *************************************************************************************************

class CommandsHandler
{
public:
    /*
    * pCommandFunction is used for the functions map - each function gets two arguments:
    * vector of strings which holds the arguments and the number of arguments in that vector
    */
    typedef ResponseMessage(CommandsHandler::*pCommandFunction)(const std::vector<std::string>&, unsigned int);

    /*
    * The constructor inserts each one of the available functions into the map - m_functionHandler - according to server type (TCP/UDP)
    */
    CommandsHandler(ServerType type, Host& host);

    ConnectionStatus ExecuteCommand(const std::string& message, ResponseMessage &referencedResponse);
    ConnectionStatus ExecuteBinaryCommand(uint8_t* binaryInput, ResponseMessage &referencedResponse);

private:
    //m_functionHandler is a map that maps a string = command name, to a function
    std::map<std::string, pCommandFunction> m_functionHandler;
    Host& m_host; // a reference to the host (enables access to deviceManager and hostInfo)

    enum CommandsHandlerResponseStatus
    {
        chrsInvalidNumberOfArguments,
        chrsInvalidArgument,
        chrsOperationFailure,
        chrsLinuxSupportOnly,
        chrsDeviceIsSilent,
        chrsSuccess
    };

    std::string GetCommandsHandlerResponseStatusString(CommandsHandlerResponseStatus status);

    /*
    FormatResponseMessage
    Decorate the response message with time stamp and a success status
    @param: successStatus - true for a successful operation, false otherwise
    @param: message - the content of the response
    @return: the decorated response
    */
    std::string DecorateResponseMessage(bool successStatus, const std::string& message = "");

    // **********************************Commands Functions:****************************************
    ResponseMessage GetInterfaces(const std::vector<std::string>& arguments, unsigned int numberOfArguments);

    ResponseMessage Read(const std::vector<std::string>& arguments, unsigned int numberOfArguments);

    ResponseMessage Write(const std::vector<std::string>& arguments, unsigned int numberOfArguments);

    ResponseMessage ReadBlock(const std::vector<std::string>& arguments, unsigned int numberOfArguments);

    ResponseMessage ReadBlockFast(const std::vector<std::string>& arguments, unsigned int numberOfArguments);

    ResponseMessage ReadRadarData(const std::vector<std::string>& arguments, unsigned int numberOfArguments);

    ResponseMessage WriteBlock(const std::vector<std::string>& arguments, unsigned int numberOfArguments);

    ResponseMessage InterfaceReset(const std::vector<std::string>& arguments, unsigned int numberOfArguments);

    ResponseMessage DisableASPM(const std::vector<std::string>& arguments, unsigned int numberOfArguments);
    ResponseMessage EnableASPM(const std::vector<std::string>& arguments, unsigned int numberOfArguments);
    ResponseMessage GetASPM(const std::vector<std::string>& arguments, unsigned int numberOfArguments);

    ResponseMessage GetPmcAgentVersion(const std::vector<std::string>& arguments, unsigned int numberOfArguments);
    ResponseMessage AllocPmc(const std::vector<std::string>& arguments, unsigned int numberOfArguments);
    ResponseMessage DeallocPmc(const std::vector<std::string>& arguments, unsigned int numberOfArguments);
    ResponseMessage CreatePmcFile(const std::vector<std::string>& arguments, unsigned int numberOfArguments);
    ResponseMessage FindPmcDataFile(const std::vector<std::string>& arguments, unsigned int numberOfArguments);
    ResponseMessage FindPmcRingFile(const std::vector<std::string>& arguments, unsigned int numberOfArguments);

    ResponseMessage GetTime(const std::vector<std::string>& arguments, unsigned int numberOfArguments);

    ResponseMessage SetDriverMode(const std::vector<std::string>& arguments, unsigned int numberOfArguments);

    ResponseMessage GetHostManagerVersion(const std::vector<std::string>& arguments, unsigned int numberOfArguments);
    ResponseMessage GetHostOS(const std::vector<std::string>& arguments, unsigned int numberOfArguments);

    ResponseMessage GetHostEnumarationMode(const std::vector<std::string>& arguments, unsigned int numberOfArguments);
    ResponseMessage SetHostEnumarationMode(const std::vector<std::string>& arguments, unsigned int numberOfArguments);

    ResponseMessage DriverControl(const std::vector<std::string>& arguments, unsigned int numberOfArguments);

    ResponseMessage DriverCommand(const std::vector<std::string>& arguments, unsigned int numberOfArguments);

    ResponseMessage GenericDriverIO(const std::vector<std::string>& arguments, void* inputBuf, unsigned int inputBufSize);

    ResponseMessage GetDeviceSilenceMode(const std::vector<std::string>& arguments, unsigned int numberOfArguments);

    ResponseMessage SetDeviceSilenceMode(const std::vector<std::string>& arguments, unsigned int numberOfArguments);

    ResponseMessage GetConnectedUsers(const std::vector<std::string>& arguments, unsigned int numberOfArguments);

    ResponseMessage GetDeviceCapabilities(const std::vector<std::string>& arguments, unsigned int numberOfArguments);

    ResponseMessage GetFwHealthState(const std::vector<std::string>& arguments, unsigned int numberOfArguments);

    bool ValidArgumentsNumber(const std::string& functionName, size_t numberOfArguments, size_t expectedNumOfArguments, std::string& responseMessage);

    bool ReadBlockImpl(const std::vector<std::string>& arguments, unsigned int numberOfArguments, ResponseMessage& response, std::vector<DWORD>& values);

    /*
    GetHostNetworkInfo
    Return host's IP and host's alias as the Response
    @param: an empty vector
    @return: a response with a string that contains both  host's IP and host's alias
    */
    ResponseMessage GetHostNetworkInfo(const std::vector<std::string>& arguments, unsigned int numberOfArguments);

    /*
    SetHostAlias
    Get a new alias and define it as the new host's alias
    @param: a vector with one string representing the new alias
    @return: a response with feedback about the operation status (success/failure)
    */
    ResponseMessage GetHostName(const std::vector<std::string>& arguments, unsigned int numberOfArguments);
    ResponseMessage GetHostCapabilities(const std::vector<std::string>& arguments, unsigned int numberOfArguments);
    ResponseMessage LogCollectorHandler(const std::vector<std::string>& arguments, unsigned int numberOfArguments);
    ResponseMessage FlashControlHandler(const std::vector<std::string>& arguments, unsigned int numberOfArguments);
    ResponseMessage DriverConfiguration(const std::vector<std::string>& arguments, unsigned int numberOfArguments);

    const char m_device_delimiter = ' ';

    const char m_array_delimiter = ' ';

    const char m_reply_feilds_delimiter = '|';

    static const std::string TRUE_STR;

    static const std::string FALSE_STR;
};


#endif // !_COMMANDSHANDLER_H_
