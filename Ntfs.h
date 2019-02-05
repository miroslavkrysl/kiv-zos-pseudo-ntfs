#pragma once

#include <memory>

#include "Partition.h"
#include "Node.h"
#include "NodeManager.h"

class Ntfs
{
public:
    explicit Ntfs(std::string partitionPath);

    Partition &GetPartition();

    std::vector<Node> Ls(const std::string &path = ".");
    void Mkdir(const std::string &path);
    void Rmdir(const std::string &path);
    void Mkfile(const std::string &path, std::istream &contents, int32_t size);
    void Rm(const std::string &path);
    void Mv(const std::string &sourcePath, std::string &destinationPath);
    void Cp(std::string &sourcePath, std::string &destinationPath);
    void Cat(const std::string &path, std::ostream &output);
    void Format(int32_t size, std::string signature, std::string description);

private:
    /**
     * The ntfs partition which this ntfs operates on.
     */
    Partition m_partition;

    /**
     * The node manager instance bound to the same ntfs partition.
     */
    NodeManager m_nodeManager;

    /**
     * The current working directory.
     */
    std::unique_ptr<Node> m_currentDirectory;

    /**
     * Get the directory contents.
     *
     * @param directory The directory to be listed.
     *
     * @throws NtfsNotADirectoryException When the given directory node is not a directory.
     *
     * @return The vector of the directory child nodes.
     */
    std::vector<Node> GetDirectoryContents(const Node &directory);

    /**
     * Add the node into the directory.
     *
     * @param directory The directory which the node will append into.
     * @param node The node to be appended into the directory.
     *
     * @throws NtfsNotADirectoryException When the given directory node is not a directory.
     */
    void AddIntoDirectory(Node &directory, const Node &node);

    /**
     * Remove the node from the directory.
     *
     * @param directory The directory which the node will removed from.
     * @param node The node to be removed from the directory.
     *
     * @throws NtfsNotADirectoryException When the given directory node is not a directory.
     */
    void RemoveFromDirectory(Node &directory, const Node &node);
};

