#include <utility>
#include <algorithm>
#include <sstream>

#include "Ntfs.h"
#include "Exceptions/NtfsExceptions.h"
#include "Exceptions/NodeManagerExceptions.h"

//done
Ntfs::Ntfs(std::string partitionPath)
    : m_partition{std::move(partitionPath)},
      m_nodeManager{m_partition},
      m_currentDirectory{UID_ROOT}
{}

// done
bool Ntfs::IsOpened()
{
    return m_partition.IsOpened();
}

// done
std::string Ntfs::Pwd()
{
    std::list<std::string> pathNodes;
    Node dir = m_nodeManager.FindNode(m_currentDirectory);

    while (dir.GetUid() != UID_ROOT) {
        pathNodes.emplace_front(dir.GetName());

        auto items = GetDirectoryContents(dir);

        dir = items.front();
    }

    std::string path{"/"};

    for (auto &node : pathNodes) {
        path += node;
        path += "/";
    }

    return path;
}

// done
void Ntfs::Cd(std::string path)
{
    auto parsedPath = ParsePath(std::move(path));

    if (parsedPath.second.back() == "/") {
        parsedPath.second.pop_back();
    }

    try {
        // find the directory node
        Node directory = FindNode(parsedPath.first, parsedPath.second);

        if (!directory.IsDirectory()) {
            throw NtfsPathNotFoundException{"directory not found"};
        }

        m_currentDirectory = directory.GetUid();
    }
    catch (NtfsNodeNotFoundException &exception) {
        throw NtfsPathNotFoundException{"directory not found"};
    }
}

// done
std::list<Node> Ntfs::Ls(std::string path)
{
    auto parsedPath = ParsePath(std::move(path));

    if (!parsedPath.second.empty() && parsedPath.second.back() == "/") {
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
    auto parsedPath = ParsePath(std::move(path));

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
    catch (NtfsNotADirectoryException &exception) {
        // parent node is not a directory
        m_nodeManager.ReleaseNode(directory);
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
        Node directory = FindNode(parent.GetUid(), std::list<std::string>{directoryName});

        if (!directory.IsDirectory()) {
            throw NtfsFileNotFoundException{"directory not found"};
        }

        if (directory.GetSize() > sizeof(int32_t)) {
            throw NtfsDirectoryNotEmptyException{"the directory is not empty"};
        }

        RemoveFromDirectory(parent, directory);
        m_nodeManager.ReleaseNode(directory);
    }
    catch (NtfsNodeNotFoundException &exception) {
        throw NtfsFileNotFoundException{"directory not found"};
    }
}

// done
void Ntfs::Mkfile(const std::string path, std::istream &contents, int32_t size)
{
    auto parsedPath = ParsePath(path);

    if (parsedPath.second.back() == "/") {
        throw NtfsPathNotFoundException{"file not found"};
    }

    // take the file name from path
    std::string fileName = parsedPath.second.back();
    parsedPath.second.pop_back();

    Node file;

    try {
        // find the parent directory, where the file will be created
        Node parent = FindNode(parsedPath.first, parsedPath.second);

        // create node for the file
        file = m_nodeManager.CreateNode(fileName, false, size);

        AddIntoDirectory(parent, file);

        m_nodeManager.WriteIntoNode(file, contents);
    }
    catch (NtfsNodeNotFoundException &exception) {
        throw NtfsPathNotFoundException{"parent directory not found"};
    }
    catch (NtfsNotADirectoryException &exception) {
        // parent node is not a directory
        m_nodeManager.ReleaseNode(file);
        throw NtfsPathNotFoundException{"parent directory not found"};
    }
    catch (NodeManagerException &exception) {
        // resources allocation failed
        m_nodeManager.ReleaseNode(file);
        throw;
    }
    catch (NtfsNodeAlreadyExistsException &exception) {
        m_nodeManager.ReleaseNode(file);
        throw;
    }
}

// done
void Ntfs::Rmfile(std::string path)
{
    auto parsedPath = ParsePath(std::move(path));

    if (parsedPath.second.back() == "/") {
        throw NtfsFileNotFoundException{"file not found"};
    }

    // take the file name from path
    std::string fileName = parsedPath.second.back();
    parsedPath.second.pop_back();

    try {
        // find the parent directory of the file being removed
        Node parent = FindNode(parsedPath.first, parsedPath.second);

        // find the file being removed itself
        Node file = FindNode(parent.GetUid(), std::list<std::string>{fileName});

        if (file.IsDirectory()) {
            throw NtfsFileNotFoundException{"file not found"};
        }

        RemoveFromDirectory(parent, file);
        m_nodeManager.ReleaseNode(file);
    }
    catch (NtfsNodeNotFoundException &exception) {
        throw NtfsFileNotFoundException{"file not found"};
    }
}

// done
void Ntfs::Mv(std::string sourcePath, std::string destinationPath)
{
    auto srcPath = ParsePath(std::move(sourcePath));

    bool srcIsDir = false;

    if (srcPath.second.back() == "/") {
        srcIsDir = true;
        srcPath.second.pop_back();
    }

    // take the src name from path
    std::string srcName = srcPath.second.back();
    srcPath.second.pop_back();

    Node src;
    Node parent;

    try {
        // find the src parent directory
        parent = FindNode(srcPath.first, srcPath.second);

        // find the src node itself
        src = FindNode(parent.GetUid(), std::list<std::string>{srcName});

        if (srcIsDir && !src.IsDirectory()) {
            throw NtfsFileNotFoundException{"source file not found"};
        }
    }
    catch (NtfsNodeNotFoundException &exception) {
        throw NtfsFileNotFoundException{"source file not found"};
    }


    auto destPath = ParsePath(std::move(destinationPath));

    // take the dest name from path or from the original name
    std::string destName;

    if (destPath.second.back() == "/") {
        destName = src.GetName();
    }
    else {
        destName = destPath.second.back();
    }
    destPath.second.pop_back();

    Node dest;

    try {
        // find the dest node
        dest = FindNode(destPath.first, destPath.second);

        m_nodeManager.RenameNode(src, destName);

        if (dest.GetUid() == parent.GetUid()) {
            return;
        }

        RemoveFromDirectory(parent, src);
        AddIntoDirectory(dest, src);
    }
    catch (NtfsNodeNotFoundException &exception) {
        throw NtfsPathNotFoundException{"destination directory not found"};
    }
    catch (NtfsNotADirectoryException &exception) {
        // dest node is not a directory
        m_nodeManager.RenameNode(src, srcName);
        throw NtfsPathNotFoundException{"destination directory not found"};
    }
    catch (NodeManagerException &exception) {
        // resources allocation failed
        AddIntoDirectory(parent, src);
        m_nodeManager.RenameNode(src, srcName);
        throw;
    }
    catch (NtfsNodeAlreadyExistsException &exception) {
        m_nodeManager.RenameNode(src, srcName);
        throw;
    }
}

//done
void Ntfs::Cpfile(std::string sourcePath, std::string destinationPath)
{
    auto srcPath = ParsePath(std::move(sourcePath));

    if (srcPath.second.back() == "/") {
        throw NtfsFileNotFoundException{"source file not found"};
    }

    Node src;

    try {
        // find the src node
        src = FindNode(srcPath.first, srcPath.second);

        if (src.IsDirectory()) {
            throw NtfsFileNotFoundException{"source file not found"};
        }
    }
    catch (NtfsNodeNotFoundException &exception) {
        throw NtfsFileNotFoundException{"source file not found"};
    }


    auto destPath = ParsePath(std::move(destinationPath));

    // take the dest name from path or from the original name
    std::string destName;

    if (destPath.second.back() == "/") {
        destName = src.GetName();
    }
    else {
        destName = destPath.second.back();
    }
    destPath.second.pop_back();

    Node dest;
    Node nodeCopy;

    try {
        // find the dest node
        dest = FindNode(destPath.first, destPath.second);

        nodeCopy = m_nodeManager.CloneNode(src, destName);

        AddIntoDirectory(dest, nodeCopy);
    }
    catch (NtfsNodeNotFoundException &exception) {
        throw NtfsPathNotFoundException{"destination directory not found"};
    }
    catch (NtfsNotADirectoryException &exception) {
        // dest node is not a directory
        m_nodeManager.ReleaseNode(nodeCopy);
        throw NtfsPathNotFoundException{"destination directory not found"};
    }
    catch (NodeManagerException &exception) {
        // resources allocation failed
        throw;
    }
    catch (NtfsNodeAlreadyExistsException &exception) {
        m_nodeManager.ReleaseNode(nodeCopy);
        throw;
    }
}

// done
void Ntfs::Cat(std::string path, std::ostream &output)
{
    auto parsedPath = ParsePath(std::move(path));

    if (parsedPath.second.back() == "/") {
        throw NtfsFileNotFoundException{"file not found"};
    }

    try {
        // find the file node
        Node file = FindNode(parsedPath.first, parsedPath.second);

        if (file.IsDirectory()) {
            throw NtfsFileNotFoundException{"file not found"};
        }

        m_nodeManager.ReadFromNode(file, output);
    }
    catch (NtfsNodeNotFoundException &exception) {
        throw NtfsFileNotFoundException{"file not found"};
    }
}

//done
void Ntfs::Format(int32_t size, std::string signature, std::string description)
{
    m_partition.Format(size, std::move(signature), std::move(description));
}

// done
Node Ntfs::FindNode(std::string path)
{
    auto parsedPath = ParsePath(std::move(path));

    bool nodeIsDir = false;

    if (parsedPath.second.back() == "/") {
        nodeIsDir = true;
        parsedPath.second.pop_back();
    }

    Node file = FindNode(parsedPath.first, parsedPath.second);

    if (nodeIsDir && !file.IsDirectory()) {
        throw NtfsNodeNotFoundException{"node not found"};
    }

    return file;
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
std::pair<int32_t, std::list<std::string>> Ntfs::ParsePath(std::string path)
{
    std::list<std::string> pathNodes;
    int32_t start;

    if (path.front() == '/') {
        start = UID_ROOT;
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

    return {start, std::move(pathNodes)};
}

// done
Node Ntfs::FindNode(int32_t directory, const std::list<std::string> &path)
{
    Node currentNode = m_nodeManager.FindNode(directory);

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
