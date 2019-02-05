#include <utility>
#include <algorithm>
#include <sstream>
#include <iostream>

#include "Ntfs.h"
#include "Exceptions/NtfsExceptions.h"
#include "Exceptions/NodeManagerExceptions.h"

Ntfs::Ntfs(std::string partitionPath)
    : m_partition{std::move(partitionPath)},
      m_nodeManager{m_partition},
      m_currentDirectory{FindRoot()}
{}

Partition &Ntfs::GetPartition()
{
    return m_partition;
}

std::vector<Node> Ntfs::Ls(const std::string &path)
{
    return std::vector<Node>{};
}

// done
void Ntfs::Mkdir(const std::string &path)
{
    auto parsedPath = ParsePath(path);

    // take the directory name from path
    std::string directoryName = parsedPath.second.back();
    parsedPath.second.pop_back();

    // find the parent directory, where the directory will be created
    Node parent = FindNode(parsedPath.first, parsedPath.second);

    if (!parent.IsDirectory()) {
        throw NtfsNotADirectoryException{"the found parent node isn't a directory"};
    }

    // check for the names conflict
    if (IsInDirectory(parent, directoryName)) {
        throw NtfsNodeAlreadyExistsException{"the node " + path + " already exists"};
    }

    Node directory = m_nodeManager.CreateNode(directoryName, true, sizeof(int32_t));

    try {
        AddIntoDirectory(parent, directory);

        int32_t parentUid = parent.GetUid();
        m_nodeManager.WriteIntoNode(directory, &parentUid);
    }
    catch (NodeManagerException &exception) {
        m_nodeManager.ReleaseNode(directory);
        throw;
    }
}

void Ntfs::Rmdir(const std::string &path)
{
    auto parsedPath = ParsePath(path);

    // take the directory name from path
    std::string directoryName = parsedPath.second.back();
    parsedPath.second.pop_back();

    // find the parent directory of the directory being removed
    Node parent = FindNode(parsedPath.first, parsedPath.second);

    if (!parent.IsDirectory()) {
        throw NtfsNodeNotFoundException{"can't find the directory"};
    }

    // find the directory being removed itself
    Node directory = FindNode(parent, std::list<std::string>{directoryName});

    if (!directory.IsDirectory()) {
        throw NtfsNotADirectoryException{"the node is not a directory"};
    }

    if (directory.GetSize() > sizeof(int32_t)) {
        throw NtfsDirectoryNotEmptyException{"the directory is not empty"};
    }

    RemoveFromDirectory(parent, directory);
    m_nodeManager.ReleaseNode(directory);
}

void Ntfs::Mkfile(const std::string &path, std::istream &contents, int32_t size)
{

}

void Ntfs::Rm(const std::string &path)
{

}

void Ntfs::Mv(const std::string &sourcePath, std::string &destinationPath)
{

}

void Ntfs::Cp(std::string &sourcePath, std::string &destinationPath)
{

}

void Ntfs::Cat(const std::string &path, std::ostream &output)
{

}

void Ntfs::Format(int32_t size, std::string signature, std::string description)
{

}

// done
std::list<Node> Ntfs::GetDirectoryContents(const Node &directory)
{
    if (!directory.IsDirectory()) {
        throw NtfsNotADirectoryException("the given node is not a directory - can't do dir manipulations");
    }

    std::vector<int32_t> uids;
    uids.resize(directory.GetSize() / sizeof(int32_t));

    m_nodeManager.ReadFromNode(directory, uids.data());

    std::list<Node> items;

    for (auto &uid : uids) {
        items.emplace_back(m_nodeManager.FindNode(uid));
    }

    return items;
}

// done
void Ntfs::AddIntoDirectory(Node &directory, const Node &node)
{
    auto items = GetDirectoryContents(directory);

    for (auto &item : items) {
        if (item.GetName() == node.GetName()) {
            // name conflict

            if (item.GetUid() == node.GetUid()) {
                // node already in dir
                return;
            }

            throw NtfsNodeAlreadyExistsException{
                "a node with the name: " + node.GetName() + " already exists in the directory: " + directory.GetName()};
        }
    }

    items.emplace_back(node);
    std::vector<int32_t> uids;
    uids.reserve(items.size());

    // fill uids into the vector
    for (auto &item : items) {
        uids.emplace_back(item.GetUid());
    }

    // resize directory node to its contents
    auto newSize = static_cast<int32_t>(uids.size() * sizeof(int32_t));
    m_nodeManager.ResizeNode(directory, newSize);

    m_nodeManager.WriteIntoNode(directory, uids.data());
}

// done
void Ntfs::RemoveFromDirectory(Node &directory, const Node &node)
{
    auto items = GetDirectoryContents(directory);

    for (auto itItem = items.begin(); itItem != items.end(); itItem++) {
        if (itItem->GetUid() == node.GetUid()) {
            // item found

            items.erase(itItem);

            std::vector<int32_t> uids;
            uids.reserve(items.size());

            // fill uids into the vector
            for (auto &item : items) {
                uids.emplace_back(item.GetUid());
            }

            // resize directory node to its contents
            auto newSize = static_cast<int32_t>(uids.size() * sizeof(int32_t));
            m_nodeManager.ResizeNode(directory, newSize);

            m_nodeManager.WriteIntoNode(directory, uids.data());

            return;
        }
    }
}

// done
std::pair<Node, std::list<std::string>> Ntfs::ParsePath(std::string path)
{
    std::list<std::string> pathNodes;
    Node start;

    if (path.back() == '/') {
        path.pop_back();
    }

    if (path.front() == '/') {
        start = FindRoot();
        path.erase(0, 1);
    }
    else {
        start = m_currentDirectory;
    }

    std::string item;
    std::stringstream ss(path);

    while (std::getline(ss, item, '/')) {
        pathNodes.push_back(item);
    }

    return {std::move(start), std::move(pathNodes)};
}

// done
Node Ntfs::FindNode(const Node &directory, const std::list<std::string> &path)
{
    Node currentNode = directory;

    for (auto &pathNode : path) {
        if (pathNode == ".") {
            continue;
        }

        bool notFound = true;
        auto items = GetDirectoryContents(currentNode);

        if (pathNode == "..") {
            currentNode = std::move(items.front());
            continue;
        }

        // ignore parent dir
        items.pop_front();

        for (auto &item : items) {
            if (item.GetName() == pathNode) {
                currentNode = std::move(item);
                notFound = false;
                break;
            }
        }

        if (notFound) {
            throw NtfsNodeNotFoundException{"node on the given path from the given directory not exists"};
        }
    }

    return currentNode;
}

// done
Node Ntfs::FindRoot()
{
    try {
        return m_nodeManager.FindNode(UID_ROOT);
    }
    catch (NodeManagerNodeNotFoundException &exception) {
        throw NtfsRootNotFoundException{"can't find the partition root directory"};
    }
}

// done
bool Ntfs::IsInDirectory(const Node &directory, std::string name)
{

    auto items = GetDirectoryContents(directory);

    // ignore parent dir
    items.pop_front();

    for (auto &item : items) {
        if (item.GetName() == name) {
            return true;
        }
    }

    return false;
}
