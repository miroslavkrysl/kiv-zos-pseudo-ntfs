#pragma once

#include <memory>
#include <list>

#include "Partition.h"
#include "Node.h"
#include "NodeManager.h"

class Ntfs
{
public:
    explicit Ntfs(std::string partitionPath);

    Partition &GetPartition();

    std::vector<Node> Ls(const std::string &path = ".");

    /**
     * Create a directory on the given path.
     *
     * @param path The directory path - absolute or relative to the current working directory.
     *
     * @throws NtfsNodeNotFoundException When the node is not found.
     * @throws NtfsNotADirectoryException When the parent of the directory being created is not a directory.
     * @throws NtfsNodeAlreadyExistsException When the node of the given path already exists.
     */
    void Mkdir(const std::string &path);

    /**
     * Remove a directory on the given path.
     *
     * @param path The directory path - absolute or relative to the current working directory.
     *
     * @throws NtfsNodeNotFoundException When the node is not found.
     * @throws NtfsNotADirectoryException When the node is not a directory.
     * @throws NtfsDirectoryNotEmptyException When the directory is not empty.
     */
    void Rmdir(const std::string &path);

    void Mkfile(const std::string &path, std::istream &contents, int32_t size); // find node with parent
    void Rm(const std::string &path); // find node with parent
    void Mv(const std::string &sourcePath, std::string &destinationPath); // find node with parent
    void Cp(std::string &sourcePath, std::string &destinationPath); // find node with parent, clone node
    void Cat(const std::string &path, std::ostream &output); // find node
    void Format(int32_t size, std::string signature, std::string description);

//private:
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
    Node m_currentDirectory;

    /**
     * Get the directory contents.
     *
     * @param directory The directory to be listed.
     *
     * @throws NtfsNotADirectoryException When the given directory node is not a directory.
     *
     * @return The list of the directory child nodes.
     */
    std::list<Node> GetDirectoryContents(const Node &directory);

    /**
     * Add the node into the directory.
     *
     * @param directory The directory which the node will append into.
     * @param node The node to be appended into the directory.
     *
     * @throws NtfsNotADirectoryException When the given directory node is not a directory.
     * @throws NtfsNodeAlreadyExistsException When a node with the same name is already in the directory.
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

    /**
     * Parse the path into the individual path nodes
     * and a starting directory.
     * If the path starts with `\`, the root node will be returned
     * as a start directory, else the current working directory
     * will be the start directory.
     *
     * @param path The path to be parsed.
     *
     * @return The pair - first is starting directory, second is the list of path nodes.
     */
    std::pair<Node, std::list<std::string>> ParsePath(std::string path);

    /**
     * Find the node inside the partition directory tree
     * starting from the given directory node.
     * The path node symbol `..` means jump one directory up,
     * `.` means do not jump anywhere.
     *
     * @param directory The start directory node.
     * @param path The node path.
     *
     * @return The found node.
     */
    Node FindNode(const Node &directory, const std::list<std::string> &path);

    /**
     * Found the root directory (the one with uid = 1).
     *
     * @throws NtfsNodeNotFoundException When the node is not found.
     *
     * @return The root directory node.
     */
    Node FindRoot();

    /**
     * Check whether the node of the given name is in the given directory.
     *
     * @param directory The directory to check.
     * @param name The name of the node.
     *
     * @return True, if the node of the given name is present in the directory, false otherwise.
     */
    bool IsInDirectory(const Node &directory, std::string name);
};

