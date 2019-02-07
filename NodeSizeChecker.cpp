#include <thread>

#include "NodeSizeChecker.h"

// done
NodeSizeChecker::NodeSizeChecker(Ntfs &ntfs, std::ostream &output)
    : m_ntfs(ntfs),
      m_output(output)
{}

// done
bool NodeSizeChecker::Run(size_t threadCount)
{
    m_nextMftItemIndex = 0;
    m_checkedNodes.clear();
    m_success = true;

    std::vector<std::thread> threads;
    threads.reserve(threadCount);

    for (int i = 0; i < threadCount; i++) {
        std::thread thread{&NodeSizeChecker::GetNextNode, this};

        threads.push_back(std::move(thread));
    }

    for (auto &thread : threads) {
        thread.join();
    }

    return m_success;
}

// done
int32_t NodeSizeChecker::GetNextNode()
{
    std::unique_lock<std::mutex> lock{m_mutex};

    while (true) {
        if (m_nextMftItemIndex >= m_ntfs.m_partition.GetMftItemCount()) {
            return -1;
        }

        MftItem item = m_ntfs.m_partition.ReadMftItem(m_nextMftItemIndex);
        m_nextMftItemIndex++;

        if (item.item.uid == UID_ITEM_FREE) {
            continue;
        }

        if (m_checkedNodes.find(item.item.uid) == m_checkedNodes.end()) {
            m_checkedNodes.emplace(item.item.uid);
            return item.item.uid;
        }
    }
}

// done
void NodeSizeChecker::RunSubChecker()
{
    while (true) {
        int32_t uid = GetNextNode();

        if (uid == -1) {
            return;
        }

        Node node = m_ntfs.m_nodeManager.FindNode(uid);

        auto clusters = node.GetClusters();

        if (clusters.size() * m_ntfs.m_partition.GetClusterSize() < node.GetSize()) {
            m_output
                << "WARNING: the node " << node.GetUid()
                << " has " << clusters.size() << " clusters - "
                << "fewer than is needed for the node size " << node.GetSize() << " bytes"
                << std::endl;
        }
        else if ((clusters.size() - 1) * m_ntfs.m_partition.GetClusterSize() > node.GetSize()) {
            m_output
                << "WARNING: the node " << node.GetUid()
                << " has " << clusters.size() << " clusters - "
                << "more than is needed for the node size " << node.GetSize() << " bytes"
                << std::endl;
        }
    }
}
