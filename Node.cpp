#include <utility>

#include "Node.h"
#include "Exceptions/NodeExceptions.h"

// done
int32_t Node::GetUid()
{
    return m_mftItems.front().item.uid;
}

// done
std::string Node::GetName()
{
    return m_mftItems.front().item.name;
}

// done
bool Node::IsDirectory()
{
    return m_mftItems.front().item.is_directory;
}

// done
int32_t Node::GetSize()
{
    return m_mftItems.front().item.size;
}

// done
const std::vector<MftItem> &Node::GetMftItems()
{
    return m_mftItems;
}

// done
std::vector<mft_fragment> Node::GetFragments()
{
    std::vector<mft_fragment> fragments;

    for (auto &mftItem : m_mftItems) {
        mft_item &item = mftItem.item;

        // loop over fragments and add them to the vector
        for (int i = 0; i < MFT_FRAGMENTS_COUNT; i++) {

            if (item.fragments[i].start == FRAGMENT_UNUSED_START) {
                // fragment is unused
                break;
            }

            fragments.emplace_back(item.fragments[i]);
        }
    }

    return fragments;
}

// done
std::vector<int32_t> Node::GetClusters()
{
    std::vector<int32_t> clusters;
    auto fragments = GetFragments();

    for (auto &fragment : fragments) {

        if (fragment.start == FRAGMENT_UNUSED_START) {
            break;
        }

        for (int j = 0; j < fragment.count; j++) {
            clusters.push_back(fragment.start + j);
        }
    }

    return clusters;
}

// done
Node::Node(std::vector<MftItem> mftItems)
    : m_mftItems(std::move(mftItems))
{
    if (mftItems.empty()) {
        throw NodeException{"no mft items given for the node creation"};
    }
}
