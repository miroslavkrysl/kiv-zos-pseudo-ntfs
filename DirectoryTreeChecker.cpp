#include <thread>
#include <unordered_map>
#include <unordered_set>
#include "DirectoryTreeChecker.h"

// done
DirectoryTreeChecker::DirectoryTreeChecker(Ntfs &ntfs, std::ostream &output)
    : m_ntfs(ntfs),
      m_output(output)
{}

// done
bool DirectoryTreeChecker::Run()
{
    bool success = true;

    std::unordered_set<int32_t> nodesReachable;
    std::unordered_set<int32_t> nodesReachableMultipleTimes;

    // go through the directory tree and remember node occurrences
    std::vector<Node> nodeStack;

    Node root = m_ntfs.m_nodeManager.FindNode(UID_ROOT);
    nodeStack.emplace_back(root);

    while (!nodeStack.empty()) {
        Node node = std::move(nodeStack.back());
        nodeStack.pop_back();

        auto found = nodesReachable.find(node.GetUid());

        if (found == nodesReachable.end()) {
            nodesReachable.emplace(node.GetUid());
        } else {
            nodesReachableMultipleTimes.emplace(node.GetUid());
            continue;
        }

        if (!node.IsDirectory()) {
            continue;
        }

        auto items = m_ntfs.GetDirectoryContents(node);

        // skip parent
        items.pop_front();

        for (auto &item : items) {
            nodeStack.emplace_back(std::move(item));
        }
    }

    // scan every file in partition an check if it is present in exactly one directory
    std::unordered_set<int32_t> checkedNodes;

    for (int i = 0; i < m_ntfs.m_partition.GetMftItemCount(); ++i) {
        MftItem item = m_ntfs.m_partition.ReadMftItem(i);

        if (item.item.uid == UID_ITEM_FREE) {
            continue;
        }

        if (checkedNodes.find(item.item.uid) != checkedNodes.end()) {
            continue;
        }

        checkedNodes.emplace(item.item.uid);

        if (nodesReachable.find(item.item.uid) == nodesReachable.end()) {
            success = false;
            m_output
                << "WARNING: the node " << item.item.uid
                << " is not reachable from the directory structure"
                << std::endl;
        }
        else if (nodesReachableMultipleTimes.find(item.item.uid) != nodesReachableMultipleTimes.end()){
            success = false;
            m_output
                << "WARNING: the node " << item.item.uid
                << " is present in multiple directories"
                << std::endl;
        }
    }

    return success;
}
