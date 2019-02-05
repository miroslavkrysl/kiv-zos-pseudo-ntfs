#include <utility>
#include <algorithm>

#include "Ntfs.h"
#include "Exceptions/NtfsExceptions.h"

Ntfs::Ntfs(std::string partitionPath)
    : m_partition{std::move(partitionPath)},
      m_nodeManager{m_partition}
{}

Partition &Ntfs::GetPartition()
{
    return m_partition;
}

std::vector<Node> Ntfs::Ls(const std::string &path)
{
    return std::vector<Node>();
}

void Ntfs::Mkdir(const std::string &path)
{

}

void Ntfs::Rmdir(const std::string &path)
{

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
std::vector<Node> Ntfs::GetDirectoryContents(const Node &directory)
{
    if (!directory.IsDirectory()) {
        throw NtfsNotADirectoryException("the given node is not a directory - can't do dir manipulations");
    }

    std::vector<int32_t> uids;
    uids.resize(directory.GetSize() / sizeof(int32_t));

    m_nodeManager.ReadFromNode(directory, uids.data());

    std::vector<Node> items;
    items.reserve(uids.size());

    for (auto &uid : uids) {
        items.emplace_back(m_nodeManager.FindNode(uid));
    }

    return items;
}

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

            throw NtfsNodeNameConflict{
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
        }
    }
}