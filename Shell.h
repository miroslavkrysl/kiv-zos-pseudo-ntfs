#pragma once

#include <string>
#include <vector>
#include <istream>
#include <ostream>
#include <functional>
#include <regex>
#include <unordered_map>

#include "Ntfs.h"
#include "NtfsChecker.h"

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
     * @param input The shell input stream.
     * @param output The shell output stream.
     */
    explicit Shell(Ntfs &ntfs, std::istream &input, std::ostream &output);

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
        {"load", &Shell::CmdLoad},
        {"opened", &Shell::CmdOpened},
        {"format", &Shell::CmdFormat},
        {"pwd", &Shell::CmdPwd},
        {"cd", &Shell::CmdCd},
        {"info", &Shell::CmdInfo},
        {"ls", &Shell::CmdLs},
        {"cat", &Shell::CmdCat},
        {"mkdir", &Shell::CmdMkdir},
        {"rmdir", &Shell::CmdRmdir},
        {"incp", &Shell::CmdIncp},
        {"outcp", &Shell::CmdOutcp},
        {"rm", &Shell::CmdRm},
        {"mv", &Shell::CmdMv},
        {"cp", &Shell::CmdCp},
        {"bootrecord", &Shell::CmdBootrecord},
        {"mft", &Shell::CmdMft},
        {"bitmap", &Shell::CmdBitmap},
        {"check", &Shell::CmdCheck},
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
     * The ntfs checker.
     */
    NtfsChecker m_ntfsChecker;

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
     * @param arguments Only the command name.
     */
    void CmdExit(std::vector<std::string> arguments);

    /**
     * Load the file with ntfs shell commands and process them.
     *
     * @param arguments The command name and the commands file path.
     */
    void CmdLoad(std::vector<std::string> arguments);

    /**
     * Check whether the ntfs partition is formatted.
     *
     * @param arguments Only the command name.
     */
    void CmdOpened(std::vector<std::string> arguments);

    /**
     * Format the partition.
     *
     * @param arguments The command name, the size of the partition and optionally the partition description.
     */
    void CmdFormat(std::vector<std::string> arguments);

    /**
     * Print the current working directory path.
     *
     * @param arguments Only the command name.
     */
    void CmdPwd(std::vector<std::string> arguments);

    /**
     * Change the current working directory.
     *
     * @param arguments The command name and the directory path.
     */
    void CmdCd(std::vector<std::string> arguments);

    /**
     * Print info about the node.
     *
     * @param arguments The command name and the node path.
     */
    void CmdInfo(std::vector<std::string> arguments);

    /**
     * Print the directory contents.
     *
     * @param arguments The command name and optionally the directory path.
     */
    void CmdLs(std::vector<std::string> arguments);

    /**
     * Print the file contents.
     *
     * @param arguments The command name and the file path.
     */
    void CmdCat(std::vector<std::string> arguments);

    /**
     * Make a new directory.
     *
     * @param arguments The command name and the directory path.
     */
    void CmdMkdir(std::vector<std::string> arguments);

    /**
     * Remove the directory.
     *
     * @param arguments The command name and the directory path.
     */
    void CmdRmdir(std::vector<std::string> arguments);

    /**
     * Copy the file from outside into the partition.
     *
     * @param arguments The command name, the source file path and the destination file path.
     */
    void CmdIncp(std::vector<std::string> arguments);

    /**
     * Copy the file outside from the partition.
     *
     * @param arguments The command name, the source file path and the destination file path.
     */
    void CmdOutcp(std::vector<std::string> arguments);

    /**
     * Remove the file.
     *
     * @param arguments The command name and the file path.
     */
    void CmdRm(std::vector<std::string> arguments);

    /**
     * Move the file or the directory to the new destination and optionally rename it.
     *
     * @param arguments The command name, the source node path and the destination node path.
     */
    void CmdMv(std::vector<std::string> arguments);

    /**
     * Copy the file to the new destination and optionally rename it.
     *
     * @param arguments The command name, the source file path and the destination file path.
     */
    void CmdCp(std::vector<std::string> arguments);

    /**
     * Print the boot record values.
     *
     * @param arguments Only the command name.
     */
    void CmdBootrecord(std::vector<std::string> arguments);

    /**
     * Print the mft.
     *
     * @param arguments The command name and optionally the `all` switch.
     */
    void CmdMft(std::vector<std::string> arguments);

    /**
     * Print the bitmap.
     *
     * @param arguments Only the command name.
     */
    void CmdBitmap(std::vector<std::string> arguments);

    /**
     * Check the partition consistency.
     *
     * @param arguments Only the command name.
     */
    void CmdCheck(std::vector<std::string> arguments);
};