#pragma once

#include <mutex>
#include <unordered_set>
#include <atomic>
#include <memory>

#include "Ntfs.h"

/**
 * The class NodeSizeChecker contains a logic for the ntfs nodes sizes check.
 */
class NodeSizeChecker
{
public:
    /**
     * Initializes a new NodeSizeChecker.
     *
     * @param ntfs The ntfs which the file size checker will operate.
     * @param output The output to print messages.
     */
    NodeSizeChecker(Ntfs &ntfs, std::ostream &output);

    /**
     * Run the node size checking.
     * Checks every node on the partition,
     * if its size corresponds with the
     * number of clusters assigned for it.
     *
     * @param threadCount The number of threads to use for checking.
     *
     * @return True if everything is OK, false otherwise.
     */
    bool Run(size_t threadCount);

private:
    /**
     * The ntfs which this checker operates on.
     */
    Ntfs &m_ntfs;

    /**
     * The output to print messages.
     */
    std::ostream &m_output;

    /**
     * The mutex to block multiple threads
     * to access the GetNextFile function concurrently.
     */
    std::mutex m_mutexGetFile;

    /**
     * The mutex to block multiple threads
     * to access the PrintMessage function concurrently.
     */
    std::mutex m_mutexPrintMessage;

    /**
     * The number of mft items present on the partition.
     */
    int32_t m_mftItemCount;

    /**
     * Indicates whether the nodes check succeeded.
     */
    std::atomic<bool> m_success;

    /**
     * The index of the next mft item to be processed
     */
    int32_t m_nextMftItemIndex;

    /**
     * The set of uids of already checked nodes.
     */
    std::unordered_set<int32_t> m_checkedNodes;

    /**
     * Get the next node to process;
     *
     * @return The next node to check of the node to check, nullptr if there are no more nodes to check.
     */
    std::unique_ptr<Node> GetNextNode();

    /**
     * Print message into the output.
     *
     * @param message The message.
     */
    void PrintMessage(const std::string &message);

    /**
     * Run the function that repeatedly asks for a node to check
     * and checks its size.
     */
    void RunSubChecker();
};


