#include <iostream>

#include "Shell.h"

// done
Shell::Shell(Ntfs &ntfs)
    : m_input(std::cin),
      m_output(std::cout),
      m_ntfs(ntfs)
{}

// done
void Shell::Run()
{
    while (!m_shouldTerminate) {
        m_output << m_prompt;

        std::string command;
        getline(m_input, command);

        if (m_input.eof()) {
            break;
        }

        Handle(command);
    }
}

// done
void Shell::Handle(std::string line)
{
    std::stringstream lineStream{line};
    std::vector<std::string> arguments{
        std::istream_iterator<std::string>{lineStream},
        std::istream_iterator<std::string>{}};

    if (arguments.empty()) {
        return;
    }

    std::string commandName{arguments[0]};

    if (Shell::m_actions.find(commandName) == Shell::m_actions.end()) {
        m_output << "UNKNOWN COMMAND" << std::endl;
    }

    Command command = Shell::m_actions[commandName];

    (this->*command)(arguments);
}

// done
void Shell::CmdExit(std::vector<std::string> arguments)
{
    m_shouldTerminate = true;
}

// done
void Shell::CmdOpened(std::vector<std::string> arguments)
{
    m_output << (m_ntfs.IsOpened() ? "YES" : "NO") << std::endl;
}

//void Shell::formatCmd(std::vector<std::string> arguments)
//{
//    if (arguments.size() != 2) {
//        throw TooFewArgumentsException("size of the partition not given");
//    }
//
//    std::smatch match;
//
//    if (!std::regex_match(arguments[1], match, m_sizeRegex)) {
//        throw BadArgumentException("size is in bad format");
//    }
//
//    int32_t number;
//    std::string units;
//
//    std::stringstream sizeStream{match[1]};
//    sizeStream >> number;
//
//    units = match[2];
//
//    if (sizeStream.fail()) {
//        throw BadArgumentException("size is too big");
//    }
//
//    int64_t size = number;
//
//    if (units == "K") {
//        size *= 1'000;
//    }
//    else if (units == "M") {
//        size *= 1'000'000;
//    }
//    else if (units == "G") {
//        size *= 1'000'000'000;
//    }
//
//    if (size > INT32_MAX) {
//        throw BadArgumentException("size is too big");
//    }
//
//    // todo: add creators login name
//    // todo: add optional description
//
//    partition->format(partitionPath, static_cast<int32_t>(size), "admin", "hello this is very funny partition");
//}
//
//void Shell::formattedCmd(std::vector<std::string> arguments)
//{
//    m_output << (partition != nullptr) << std::endl;
//}
//
//void Shell::mkdirCmd(std::vector<std::string> arguments)
//{
//    if (partition == nullptr) {
//        throw NtfsNotFormattedException("partition is not formatted");
//    }
//
//    if (arguments.size() != 2) {
//        throw TooFewArgumentsException("path of the directory not given");
//    }
//
//    partition->mkdir(arguments[1]);
//}
//
//void Shell::lsCmd(std::vector<std::string> arguments)
//{
//    if (partition == nullptr) {
//        throw NtfsNotFormattedException("partition is not formatted");
//    }
//
//    if (arguments.size() == 2) {
//        partition->ls(arguments[1], m_output);
//    } else {
//        partition->ls("", m_output);
//    }
//}
