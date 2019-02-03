#include <utility>

#include <random>
#include <cstring>
#include <iostream>

#include "NodeManager.h"
#include "Exceptions/NodeManagerExceptions.h"

// done
NodeManager::NodeManager(Partition &partition)
    : m_partition(partition)
{}

// done
Node NodeManager::CreateNode(std::string name, bool isDirectory, int32_t size)
{
    auto fragments = FindFreeFragments(size);
    auto mftItems = FindFreeMftItems(fragments.size());
    auto uid = GetFreeUid();

    SetupMftItems(mftItems, uid, std::move(name), isDirectory, size, fragments);

    Node node{std::move(mftItems)};
    SaveNode(node);

    return node;
}

// done
void NodeManager::SaveNode(const Node &node)
{
    for (auto &cluster : node.GetClusters()) {
        m_partition.WriteBitmapBit(cluster, true);
    }

    for (auto &mftItem : node.GetMftItems()) {
        m_partition.WriteMftItem(mftItem);
    }
}

// done
void NodeManager::ReleaseNode(const Node &node)
{
    for (auto &cluster : node.GetClusters()) {
        m_partition.WriteBitmapBit(cluster, false);
    }

    for (auto &mftItem : node.GetMftItems()) {
        MftItem item = mftItem;
        item.item.uid = UID_ITEM_FREE;

        m_partition.WriteMftItem(item);
    }
}

// done
Node NodeManager::ResizeNode(const Node &node, int32_t size)
{
    ReleaseNode(node);

    try {
        auto fragments = FindFreeFragments(size);
        auto mftItems = FindFreeMftItems(fragments.size());

        SetupMftItems(mftItems, node.GetUid(), node.GetName(), node.IsDirectory(), size, fragments);

        Node newNode{mftItems};
        SaveNode(newNode);

        return newNode;
    }
    catch (NodeManagerException &exception) {
        SaveNode(node);
        throw;
    }
}

// done
Node NodeManager::findNode(int32_t uid)
{
    auto mftItems = m_partition.ReadMftItems(uid);

    if (mftItems.empty()) {
        throw NodeManagerNodeNotFoundException{"the node with the uid " + std::to_string(uid) + " doesn't exist"};
    }

    return Node{std::move(mftItems)};
}


void NodeManager::WriteToNode(const Node &node, void *source, size_t size)
{

}

void NodeManager::WriteToNode(const Node &node, std::istream &source, size_t size)
{

}

void NodeManager::ReadFromNode(const Node &node, void *destination, size_t size)
{

}

void NodeManager::ReadFromNode(const Node &node, std::ostream &destination, size_t size)
{

}

// done
int32_t NodeManager::GetFreeUid()
{
    std::default_random_engine generator;
    std::uniform_int_distribution<int32_t> distribution(1, INT32_MAX);

    int32_t uid{0};
    bool unique{false};

    while (!unique) {
        unique = true;
        uid = distribution(generator);

        // check whether the generated uid is not present in mft
        for (int i = 0; i < m_partition.GetMftItemCount(); ++i) {
            if (m_partition.ReadMftItem(i).item.uid == uid) {
                unique = false;
                break;
            }
        }
    }

    return uid;
}

// done
std::vector<mft_fragment> NodeManager::FindFreeFragments(int32_t size)
{
    std::vector<mft_fragment> fragments;

    mft_fragment fragment{
        FRAGMENT_UNUSED_START,
        0
    };

    int32_t clustersNeeded = size / m_partition.GetClusterSize() + 1;

    // first try to find one undivided fragment
    // loop over all clusters
    for (int32_t clusterIndex = 0; clusterIndex < m_partition.GetClusterCount(); clusterIndex++) {

        if (!m_partition.ReadBitmapBit(clusterIndex)) {
            // cluster is free

            if (fragment.start == FRAGMENT_UNUSED_START) {
                // looking for the first cluster of the fragment

                fragment.start = clusterIndex;
            }

            fragment.count++;
        } else {
            // cluster is taken

            fragment.start = FRAGMENT_UNUSED_START;
            fragment.count = 0;
        }

        if (fragment.count == clustersNeeded) {
            // succeeded to find undivided fragment

            fragments.emplace_back(fragment);
            return fragments;
        }
    }

    // secondly try to find clusters divided into multiple fragments

    int32_t foundClusters{0};

    for (int32_t clusterIndex = 0; clusterIndex < m_partition.GetClusterCount(); clusterIndex++) {

        if (!m_partition.ReadBitmapBit(clusterIndex)) {
            // cluster is free

            if (fragment.start == -1) {
                // looking for the first cluster of the fragment

                fragment.start = clusterIndex;
                fragment.count = 0;
            }

            fragment.count++;
            foundClusters++;
        } else {
            // cluster is taken

            if (fragment.start != -1) {

                // reached end of one fragment
                fragments.emplace_back(fragment);

                fragment.start = -1;
            }
        }

        if (foundClusters == clustersNeeded) {
            // succeeded to find all clusters

            // check whether the last fragment was added to the vector and add it eventually
            if (fragment.start != -1) {

                fragments.emplace_back(fragment);
            }

            return fragments;
        }
    }

    // the needed amount of clusters was not found
    throw NodeManagerNotEnoughFreeClustersException{
        "there are not enough free clusters for the node of size " + std::to_string(size)};
}

// done
std::vector<MftItem> NodeManager::FindFreeMftItems(size_t fragmentCount)
{
    std::vector<MftItem> items;

    auto itemsNeeded =
        static_cast<int32_t>(std::ceil(static_cast<double>(fragmentCount) / m_partition.GetMftMaxFragmentsCount()));

    // loop over all items and find free
    for (int i = 0; i < m_partition.GetMftItemCount(); i++) {

        MftItem mftItem = m_partition.ReadMftItem(i);

        // check if the item is free
        if (mftItem.item.uid == UID_ITEM_FREE) {

            items.emplace_back(mftItem);
        }

        if (items.size() == itemsNeeded) {
            // all needed items found
            return items;
        }
    }

    throw NodeManagerNotEnoughFreeMftItemsException{
        "there are not enough free mft items for the " + std::to_string(fragmentCount) + " fragments"};
}

// done
void NodeManager::SetupMftItems(std::vector<MftItem> &mftItems,
                                int32_t uid,
                                std::string name,
                                bool isDirectory,
                                int32_t size,
                                const std::vector<mft_fragment> &fragments)
{
    int8_t itemOrder = 0;
    int32_t fragmentsWritten = 0;

    for (auto &mftItem : mftItems) {
        mft_item &item = mftItem.item;

        item.uid = uid;
        item.is_directory = isDirectory;
        item.size = size;

        std::strncpy(item.name, name.c_str(), sizeof(mft_item::name));
        item.name[sizeof(mft_item::name) - 1] = '\0';

        item.order = itemOrder++;
        item.count = static_cast<int8_t>(mftItems.size());

        // write the maximum possible fragments to the mft item
        for (int i = 0; i < m_partition.GetMftMaxFragmentsCount(); i++) {
            if (fragmentsWritten < fragments.size()) {
                // fragment is used

                item.fragments[i] = fragments[i];
                fragmentsWritten++;
            }
            else {
                // fragment is unused
                item.fragments[i].start = FRAGMENT_UNUSED_START;
            }
        }
    }
}
