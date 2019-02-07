#include <iostream>

#include "Shell.h"
#include "Exceptions/ShellExceptions.h"
#include "Exceptions/NtfsExceptions.h"

// done
Shell::Shell(Ntfs &ntfs)
    : m_input(std::cin),
      m_output(std::cout),
      m_ntfs(ntfs),
      m_ntfsChecker(m_ntfs)
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
        return;
    }

    Command command = Shell::m_actions[commandName];

    try {
        (this->*command)(arguments);
    }
    catch (AppException &exception) {
        m_output << "ERROR: " << exception.what() << std::endl;
    }
}

// done
void Shell::CmdExit(std::vector<std::string> arguments)
{
    if (arguments.size() != 1) {
        throw ShellWrongArgumentsException("exit takes no arguments");
    }

    m_shouldTerminate = true;
}

// done
void Shell::CmdOpened(std::vector<std::string> arguments)
{
    if (arguments.size() != 1) {
        throw ShellWrongArgumentsException("opened takes no arguments");
    }

    m_output << (m_ntfs.IsOpened() ? "YES" : "NO") << std::endl;
}

// done
void Shell::CmdPwd(std::vector<std::string> arguments)
{
    if (arguments.size() != 1) {
        throw ShellWrongArgumentsException("pwd takes no arguments");
    }
    m_output << m_ntfs.Pwd() << std::endl;
}

//done
void Shell::CmdCd(std::vector<std::string> arguments)
{
    if (arguments.size() != 2) {
        throw ShellWrongArgumentsException("cd takes exactly one argument");
    }

    try {
        m_ntfs.Cd(arguments[1]);
    }
    catch (NtfsPathNotFoundException &exception) {
        m_output << "PATH NOT FOUND" << std::endl;
    }
}

// done
void Shell::CmdInfo(std::vector<std::string> arguments)
{
    if (arguments.size() != 2) {
        throw ShellWrongArgumentsException("info takes exactly one argument");
    }

    try {
        Node node = m_ntfs.FindNode(arguments[1]);

        m_output << "Name: " << node.GetName() << std::endl;
        m_output << "Uid: " << node.GetUid() << std::endl;
        m_output << "Type: " << (node.IsDirectory() ? "D" : "F") << std::endl;
        m_output << "Size: " << node.GetSize() << " B" << std::endl;

        m_output << "Fragments: " << std::endl;

        for (auto &fragment : node.GetFragments()) {
            m_output << "    [start=" << fragment.start << ", count=" << fragment.count << "]" << std::endl;
        }

        m_output << "Clusters: ";

        bool first = true;
        for (auto &cluster : node.GetClusters()) {
            if (!first) {
                m_output << ", ";
            } else {
                first = false;
            }
            m_output << cluster;
        }
        m_output << std::endl;
    }
    catch (NtfsNodeNotFoundException &exception) {
        m_output << "FILE NOT FOUND" << std::endl;
    }
}

// done
void Shell::CmdLs(std::vector<std::string> arguments)
{
    if (arguments.size() > 2) {
        throw ShellWrongArgumentsException("ls takes one argument or no arguments");
    }

    std::string path = ".";

    if (arguments.size() == 2) {
        path = arguments[1];
    }

    try {
        auto items = m_ntfs.Ls(path);

        // skip parent node
        items.pop_front();

        for (auto &item : items) {

            if (item.IsDirectory()) {
                m_output << "+";
            } else {
                m_output << "-";
            }

            m_output << item.GetName() << std::endl;
        }
    }
    catch (NtfsPathNotFoundException &exception) {
        m_output << "PATH NOT FOUND" << std::endl;
    }
}

// done
void Shell::CmdCat(std::vector<std::string> arguments)
{
    if (arguments.size() != 2) {
        throw ShellWrongArgumentsException("cat takes exactly one argument");
    }

    try {
        m_ntfs.Cat(arguments[1], m_output);
        m_output << std::endl;
    }
    catch (NtfsFileNotFoundException &exception) {
        m_output << "FILE NOT FOUND" << std::endl;
    }
}

// done
void Shell::CmdMkdir(std::vector<std::string> arguments)
{
    if (arguments.size() != 2) {
        throw ShellWrongArgumentsException("mkdir takes exactly one argument");
    }

    try {
        m_ntfs.Mkdir(arguments[1]);
        m_output << "OK" << std::endl;
    }
    catch (NtfsFileNotFoundException &exception) {
        m_output << "PATH NOT FOUND" << std::endl;
    }
    catch (NtfsNodeAlreadyExistsException &exception) {
        m_output << "EXISTS" << std::endl;
    }
}

// done
void Shell::CmdRmdir(std::vector<std::string> arguments)
{
    if (arguments.size() != 2) {
        throw ShellWrongArgumentsException("rmdir takes exactly one argument");
    }

    try {
        m_ntfs.Rmdir(arguments[1]);
        m_output << "OK" << std::endl;
    }
    catch (NtfsFileNotFoundException &exception) {
        m_output << "PATH NOT FOUND" << std::endl;
    }
    catch (NtfsDirectoryNotEmptyException &exception) {
        m_output << "NOT EMPTY" << std::endl;
    }
}




// done
void Shell::CmdBootrecord(std::vector<std::string> arguments)
{
    if (arguments.size() != 1) {
        throw ShellWrongArgumentsException("bootrecord takes no arguments");
    }

    m_ntfsChecker.PrintBootRecord(m_output);
}

// done
void Shell::CmdMft(std::vector<std::string> arguments)
{
    if (arguments.size() > 2) {
        throw ShellWrongArgumentsException("mft takes no arguments or the `all` switch");
    }

    bool printAll{false};

    if (arguments.size() == 2 && arguments[1] == "all") {
        printAll = true;
    }

    m_ntfsChecker.PrintMft(m_output, printAll);
}

// done
void Shell::CmdBitmap(std::vector<std::string> arguments)
{
    if (arguments.size() != 1) {
        throw ShellWrongArgumentsException("bitmap takes no arguments");
    }

    m_ntfsChecker.PrintBitmap(m_output);
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
