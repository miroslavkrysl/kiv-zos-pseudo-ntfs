#pragma once

#include <mutex>
#include <set>
#include <atomic>
#include <memory>

#include "Ntfs.h"

/**
 * The class DirectoryTreeChecker contains a logic for the ntfs nodes sizes check.
 */
class DirectoryTreeChecker
{
public:
    /**
     * Initializes a new DirectoryTreeChecker.
     *
     * @param ntfs The ntfs which the directory tree checker will operate.
     * @param output The output to print messages.
     */
    DirectoryTreeChecker(Ntfs &ntfs, std::ostream &output);

    /**
     * Run the directory tree checking.
     * Goes through the directory tree and remembers
     * visited nodes. Than scans every node on the partition,
     * and check if it is present in exactly one directory.
     *
     * @return True if everything is OK, false otherwise.
     */
    bool Run();

private:
    /**
     * The ntfs which this checker operates on.
     */
    Ntfs &m_ntfs;

    /**
     * The output to print messages.
     */
    std::ostream &m_output;
};
