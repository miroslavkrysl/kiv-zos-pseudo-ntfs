#include <iostream>

#include "Shell.h"
#include "Exceptions/ShellExceptions.h"
#include "Exceptions/NtfsExceptions.h"
#include "Exceptions/PartitionExceptions.h"

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
void Shell::CmdFormat(std::vector<std::string> arguments)
{
    if (arguments.size() != 2) {
        throw ShellWrongArgumentsException("format takes exactly one argument");
    }

    std::smatch match;

    if (!std::regex_match(arguments[1], match, m_sizeRegex)) {
        throw ShellWrongArgumentsException("size is in bad format");
    }

    int32_t number;
    std::string units;

    std::stringstream sizeStream{match[1]};
    sizeStream >> number;

    units = match[2];

    if (sizeStream.fail()) {
        throw ShellWrongArgumentsException("size is too big");
    }

    int64_t size = number;

    if (units == "K") {
        size *= 1000;
    }
    else if (units == "M") {
        size *= 1000000;
    }
    else if (units == "G") {
        size *= 1000000000;
    }

    if (size > INT32_MAX) {
        throw ShellWrongArgumentsException("size is too big");
    }


    std::string signature = "admin";
    std::string description = "pseudo ntfs partition";

    try {
        m_ntfs.Format(static_cast<int32_t>(size), signature, description);
        m_output << "OK" << std::endl;
    }
    catch (PartitionFileNotOpenedException &exception) {
        m_output << "CANNOT CREATE FILE" << std::endl;
    }
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

        auto fragments = node.GetFragments();

        m_output << "Fragments: (" << fragments.size() << ")" << std::endl;

        for (auto &fragment : fragments) {
            m_output << "    [start=" << fragment.start << ", count=" << fragment.count << "]" << std::endl;
        }

        auto clusters = node.GetClusters();

        m_output << "Clusters: (" << clusters.size() << ")" << std::endl;
        m_output << "    ";

        bool first = true;
        for (auto &cluster : clusters) {
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
void Shell::CmdIncp(std::vector<std::string> arguments)
{
    if (arguments.size() != 3) {
        throw ShellWrongArgumentsException("incp takes exactly two arguments");
    }

    std::ifstream inFile{arguments[1]};

    if (!inFile.is_open()) {
        m_output << "FILE NOT FOUND" << std::endl;
        return;
    }

    // compute the file size
    inFile.ignore(std::numeric_limits<std::streamsize>::max());
    std::streamsize size = inFile.gcount();
    inFile.clear();
    inFile.seekg(0, std::ios_base::beg);

    try {
        m_ntfs.Mkfile(arguments[2], inFile, size);
        m_output << "OK" << std::endl;
    }
    catch (NtfsPathNotFoundException &exception) {
        m_output << "PATH NOT FOUND" << std::endl;
    }
    catch (NtfsNodeAlreadyExistsException &exception) {
        m_output << "EXISTS" << std::endl;
    }
}

// done
void Shell::CmdOutcp(std::vector<std::string> arguments)
{
    if (arguments.size() != 3) {
        throw ShellWrongArgumentsException("outcp takes exactly two arguments");
    }

    std::ofstream outFile{arguments[2]};

    if (!outFile.is_open()) {
        m_output << "PATH NOT FOUND" << std::endl;
        return;
    }

    try {
        m_ntfs.Cat(arguments[1], outFile);
        m_output << "OK" << std::endl;
    }
    catch (NtfsFileNotFoundException &exception) {
        m_output << "FILE NOT FOUND" << std::endl;
    }
}

// done
void Shell::CmdRm(std::vector<std::string> arguments)
{
    if (arguments.size() != 2) {
        throw ShellWrongArgumentsException("rm takes exactly one argument");
    }

    try {
        m_ntfs.Rmfile(arguments[1]);
        m_output << "OK" << std::endl;
    }
    catch (NtfsFileNotFoundException &exception) {
        m_output << "FILE NOT FOUND" << std::endl;
    }
}

// done
void Shell::CmdMv(std::vector<std::string> arguments)
{
    if (arguments.size() != 3) {
        throw ShellWrongArgumentsException("mv takes exactly two arguments");
    }

    try {
        m_ntfs.Mv(arguments[1], arguments[2]);
        m_output << "OK" << std::endl;
    }
    catch (NtfsFileNotFoundException &exception) {
        m_output << "FILE NOT FOUND" << std::endl;
    }
    catch (NtfsPathNotFoundException &exception) {
        m_output << "PATH NOT FOUND" << std::endl;
    }
    catch (NtfsNodeAlreadyExistsException &exception) {
        m_output << "EXISTS" << std::endl;
    }
}

// done
void Shell::CmdCp(std::vector<std::string> arguments)
{
    if (arguments.size() != 3) {
        throw ShellWrongArgumentsException("cp takes exactly two arguments");
    }

    try {
        m_ntfs.Cpfile(arguments[1], arguments[2]);
        m_output << "OK" << std::endl;
    }
    catch (NtfsFileNotFoundException &exception) {
        m_output << "FILE NOT FOUND" << std::endl;
    }
    catch (NtfsPathNotFoundException &exception) {
        m_output << "PATH NOT FOUND" << std::endl;
    }
    catch (NtfsNodeAlreadyExistsException &exception) {
        m_output << "EXISTS" << std::endl;
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
