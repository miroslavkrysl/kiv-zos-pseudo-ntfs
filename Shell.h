#pragma once

#include <string>
#include <vector>
#include <istream>
#include <ostream>
#include <functional>
#include <regex>
#include <unordered_map>

#include "Ntfs.h"

/**
 * Simple shell to control the ntfs.
 */
class Shell
{
public:
    /**
     * Initializes a new Shell bound to the given ntfs.
     *
     * @param ntfs The ntfs which the shell will operate on.
     */
    explicit Shell(Ntfs &ntfs);

    /**
     * Start reading the commands from input.
     */
    void Run();

private:
    /**
     * The typedef for the Shell method, which will serve as a command handler.
     */
    typedef void (Shell::*Command)(std::vector<std::string>);

    /**
     * The map of available shell commands and their handlers.
     */
    std::unordered_map<std::string, Command> m_actions{
        {"exit", &Shell::CmdExit},
//        {"load", &Shell::load_},
        {"opened", &Shell::CmdOpened},
//        {"format", &Shell::formatCmd},
//        {"pwd", &Shell::pwd_},
//        {"cd", &Shell::CmdCd},
//        {"info", &Shell::info_},
//        {"ls", &Shell::lsCmd},
//        {"cat", &Shell::cat_},
//        {"mkdir", &Shell::MkdirCmd},
//        {"rmdir", &Shell::rmdir_},
//        {"incp", &Shell::incp_},
//        {"outcp", &Shell::outcp_},
//        {"mv", &Shell::mv_},
//        {"cp", &Shell::cp_},
    };

    /**
     * The regex for the partition size.
     */
    std::regex m_sizeRegex{R"((\d+)([KMG])?)"};

    /**
     * The string which will be displayed as a command prompt.
     */
    std::string m_prompt = ">";

    /**
     * The shell input.
     */
    std::istream &m_input;

    /**
     * The shell output.
     */
    std::ostream &m_output;

    /**
     * The ntfs which the shell operates on.
     */
    Ntfs &m_ntfs;

    /**
     * The shell termination condition.
     */
    bool m_shouldTerminate = false;

    /**
     * Handle the command.
     *
     * @param command The command
     */
    void Handle(std::string command);

    /**
     * Stop the shell.
     *
     * @param arguments
     */
    void CmdExit(std::vector<std::string> arguments);

    /**
     * Check whether the ntfs partition is formatted.
     *
     * @param arguments
     */
    void CmdOpened(std::vector<std::string> arguments);
};