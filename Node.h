#pragma once

#include <vector>
#include <string>
#include <cstdint>

#include "NtfsStructs.h"

/**
 * Class Node represents a ntfs file or a directory of some size
 * with an uid and a name. It contains an info about mft items
 * and clusters assigned to this node.
 * The creating and removing the node from the partition
 * as well as writing and reading from the node handles the NtfsNodeManager.
 */
class Node
{
    /**
     * Only the NodeManager can create a node.
     */
    friend class NodeManager;

public:
    /**
     * Get the node uid.
     *
     * @return The uid.
     */
    int32_t GetUid() const;

    /**
     * Get the node name.
     *
     * @return The name.
     */
    std::string GetName() const;

    /**
     * Check whether the node is a directory or a file.
     *
     * @return True if it's a directory, false if it's a file.
     */
    bool IsDirectory() const;

    /**
     * Get the node size.
     *
     * @return Size in bytes.
     */
    int32_t GetSize() const;

    /**
     * Get the mft items acquired by this file.
     *
     * @return The vector of mft items.
     */
    const std::vector<MftItem> &GetMftItems() const;

    /**
     * Get the fragments acquired by this node.
     *
     * @return The vector of fragments.
     */
    std::vector<mft_fragment> GetFragments() const;

    /**
     * Get the indexes of the clusters acquired by this node.
     *
     * @return The vector of cluster indexes.
     */
    std::vector<int32_t> GetClusters() const;

private:
    /**
     * The vector of sorted mft items being prepared for this file.
     */
    std::vector<MftItem> m_mftItems;

    /**
     * Initialize a new NtfsNode.
     * The given mft items must be filled with all the required
     * values (uid, name, size, isDirectory, order, count, fragments)
     * and be sorted by the order.
     *
     * @param mftItems The vector of the nodes mft items.
     *
     * @throws NodeException When the mftItems vector is empty.
     */
    explicit Node(std::vector<MftItem> mftItems);
};


