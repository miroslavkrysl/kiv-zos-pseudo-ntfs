#include <thread>
#include <sstream>

#include "NodeSizeChecker.h"

// done
NodeSizeChecker::NodeSizeChecker(Ntfs &ntfs, std::ostream &output)
    : m_ntfs(ntfs),
      m_output(output),
      m_mftItemCount(ntfs.m_partition.GetMftItemCount())
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
        std::thread thread{&NodeSizeChecker::RunSubChecker, this};

        threads.push_back(std::move(thread));
    }

    for (auto &thread : threads) {
        thread.join();
    }

    return m_success;
}

// done
std::unique_ptr<Node> NodeSizeChecker::GetNextNode()
{
    std::unique_lock<std::mutex> lock{m_mutexGetFile};

    while (true) {
        if (m_nextMftItemIndex >= m_mftItemCount) {
            return nullptr;
        }

        MftItem item = m_ntfs.m_partition.ReadMftItem(m_nextMftItemIndex);
        m_nextMftItemIndex++;

        if (item.item.uid == UID_ITEM_FREE) {
            continue;
        }

        if (m_checkedNodes.find(item.item.uid) == m_checkedNodes.end()) {

            m_checkedNodes.emplace(item.item.uid);

            return std::make_unique<Node>(m_ntfs.m_nodeManager.FindNode(item.item.uid));
        }
    }
}

// done
void NodeSizeChecker::PrintMessage(const std::string &message)
{
    std::unique_lock<std::mutex> lock{m_mutexPrintMessage};

    m_output << message;
}

// done
void NodeSizeChecker::RunSubChecker()
{
    while (true) {
        auto node = GetNextNode();

        if (node == nullptr) {
            return;
        }

        auto clusters = node->GetClusters();

        if (clusters.size() * m_ntfs.m_partition.GetClusterSize() < node->GetSize()) {
            std::stringstream ss;
            ss
                << "WARNING: the node " << node->GetUid()
                << " has " << clusters.size() << " clusters - "
                << "fewer than is needed for the node size " << node->GetSize() << " bytes"
                << std::endl;

            PrintMessage(ss.str());
        }
        else if ((clusters.size() - 1) * m_ntfs.m_partition.GetClusterSize() > node->GetSize()) {
            std::stringstream ss;
            ss
                << "WARNING: the node " << node->GetUid()
                << " has " << clusters.size() << " clusters - "
                << "more than is needed for the node size " << node->GetSize() << " bytes"
                << std::endl;

            PrintMessage(ss.str());
        }
    }
}
