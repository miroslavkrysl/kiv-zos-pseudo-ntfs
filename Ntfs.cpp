#include <utility>

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

// done
std::list<Node> Ntfs::Ls(std::string path)
{
    auto parsedPath = ParsePath(std::move(path));

    if (parsedPath.second.back() == "/") {
        parsedPath.second.pop_back();
    }

    try {
        // find the directory node
        Node directory = FindNode(parsedPath.first, parsedPath.second);
        return GetDirectoryContents(directory);
    }
    catch (NtfsException &exception) {
        throw NtfsPathNotFoundException{"directory not found"};
    }
}

// done
void Ntfs::Mkdir(std::string path)
{
    auto parsedPath = ParsePath(path);

    if (parsedPath.second.back() == "/") {
        parsedPath.second.pop_back();
    }

    // take the directory name from path
    std::string directoryName = parsedPath.second.back();
    parsedPath.second.pop_back();

    Node directory;

    try {
        // find the parent directory, where the directory will be created
        Node parent = FindNode(parsedPath.first, parsedPath.second);

        // create node for the directory
        directory = m_nodeManager.CreateNode(directoryName, true, sizeof(int32_t));
        AddIntoDirectory(parent, directory);

        // write parent uid into the directory
        int32_t parentUid = parent.GetUid();
        m_nodeManager.WriteIntoNode(directory, &parentUid);
    }
    catch (NtfsNodeNotFoundException &exception) {
        throw NtfsPathNotFoundException{"parent directory not found"};
    }
    catch (NodeManagerException &exception) {
        // resources allocation failed
        m_nodeManager.ReleaseNode(directory);
        throw;
    }
    catch (NtfsNodeAlreadyExistsException &exception) {
        m_nodeManager.ReleaseNode(directory);
        throw;
    }
}

// done
void Ntfs::Rmdir(std::string path)
{
    auto parsedPath = ParsePath(std::move(path));

    if (parsedPath.second.back() == "/") {
        parsedPath.second.pop_back();
    }

    // take the directory name from path
    std::string directoryName = parsedPath.second.back();
    parsedPath.second.pop_back();

    try {
        // find the parent directory of the directory being removed
        Node parent = FindNode(parsedPath.first, parsedPath.second);

        // find the directory being removed itself
        Node directory = FindNode(parent, std::list<std::string>{directoryName});

        if (!directory.IsDirectory()) {
            throw NtfsPathNotFoundException{"directory not found"};
        }

        if (directory.GetSize() > sizeof(int32_t)) {
            throw NtfsDirectoryNotEmptyException{"the directory is not empty"};
        }

        RemoveFromDirectory(parent, directory);
        m_nodeManager.ReleaseNode(directory);
    }
    catch (NtfsNodeNotFoundException &exception) {
        throw NtfsPathNotFoundException{"directory not found"};
    }
}

void Ntfs::Mkfile(const std::string path, std::istream &contents, int32_t size)
{
    auto parsedPath = ParsePath(path);

    // take the file name from path
    std::string fileName = parsedPath.second.back();
    parsedPath.second.pop_back();

    // find the parent directory, where the file will be created
    Node parent = FindNode(parsedPath.first, parsedPath.second);

    if (!parent.IsDirectory()) {
        throw NtfsNotADirectoryException{"the found parent node isn't a directory"};
    }

    // check for the names conflict
    if (IsInDirectory(parent, fileName)) {
        throw NtfsNodeAlreadyExistsException{"the node " + path + " already exists"};
    }

    Node node = m_nodeManager.CreateNode(fileName, false, size);

    try {
        AddIntoDirectory(parent, node);

        m_nodeManager.WriteIntoNode(node, contents);
    }
    catch (NodeManagerException &exception) {
        m_nodeManager.ReleaseNode(node);
        throw;
    }
}

void Ntfs::Rm(std::string path)
{
    auto parsedPath = ParsePath(std::move(path));

    // take the node name from path
    std::string directoryName = parsedPath.second.back();
    parsedPath.second.pop_back();

    // find the parent directory of the file being removed
    Node parent = FindNode(parsedPath.first, parsedPath.second);

    if (!parent.IsDirectory()) {
        throw NtfsNodeNotFoundException{"can't find the node"};
    }

    // find the node being removed itself
    Node node = FindNode(parent, std::list<std::string>{directoryName});

    if (node.IsDirectory()) {
        throw NtfsNotAFileException{"the node is not a file"};
    }

    RemoveFromDirectory(parent, node);
    m_nodeManager.ReleaseNode(node);
}

void Ntfs::Mv(const std::string sourcePath, std::string destinationPath)
{

}

void Ntfs::Cp(std::string sourcePath, std::string destinationPath)
{

}

void Ntfs::Cat(const std::string path, std::ostream &output)
{

}

//done
void Ntfs::Format(int32_t size, std::string signature, std::string description)
{
    m_partition.Format(size, signature, description);
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

    if (path.front() == '/') {
        start = FindRoot();
        path.erase(0, 1);
    }
    else {
        start = m_currentDirectory;
    }

    bool endSlash = false;

    if (path.back() == '/') {
        endSlash = true;
        path.pop_back();
    }

    std::string item;
    std::stringstream ss(path);

    while (std::getline(ss, item, '/')) {
        pathNodes.emplace_back(item);
    }

    if (endSlash) {
        pathNodes.emplace_back("/");
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

        std::list<Node> items;

        try {
            items = GetDirectoryContents(currentNode);
        }
        catch (NtfsNotADirectoryException &exception) {
            throw NtfsNodeNotFoundException{"node on the given path from the given directory not exists"};
        }

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
