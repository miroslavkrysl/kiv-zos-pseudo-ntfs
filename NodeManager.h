#pragma once

#include <cstdint>
#include <istream>

#include "Partition.h"
#include "Node.h"

/**
 * The class NodeManager handles the ntfs nodes creation and destruction
 * as well as allocation and releasing the resources needed for the nodes.
 */
class NodeManager
{
public:
    /**
     * Initializes a new NodeManager, that will opperate on the given partition.
     *
     * @param partition The ntfs partiton.
     */
    explicit NodeManager(Partition &partition);

    /**
     * Create a new node, find free resources on partition for it
     * and save it.
     *
     * @param name The name of the node (max 11 characters).
     * @param isDirectory True if so, false otherwise.
     * @param size The size of the node content in bytes.
     *
     * @throws NodeManagerNotEnoughFreeClustersException When there are not enough free clusters for the node of the given size.
     * @throws NodeManagerNotEnoughFreeMftItemsException When there are not enough free mft items for the number of fragments required by the node.
     *
     * @return The created node.
     */
    Node CreateNode(std::string name, bool isDirectory, int32_t size);

    /**
     * Write the node mft items to partition and mark the node clusters
     * as used on the partition bitmap.
     *
     * @param node
     */
    void SaveNode(const Node &node);

    /**
     * Mark the given node mft items and clusters as free on the partition.
     *
     * @param node The node to be released.
     */
    void ReleaseNode(const Node &node);

    /**
     * Try to acquire resources for the new size of the node.
     * Initializes a new node with the same properties as the old node
     * and releases resources of the old one.
     * If it fails, the original node remains unchanged.
     *
     * @param node The node to be resized.
     * @param size The new size of the node contents.
     *
     * @throws NodeManagerNotEnoughFreeClustersException When there are not enough free clusters for the new size.
     * @throws NodeManagerNotEnoughFreeMftItemsException When there are not enough free mft items for the fragments.
     *
     * @return The newly initialized Node instance with resources allocated for the new size.
     */
    Node ResizeNode(const Node &node, int32_t size);

    /**
     * Find the node with the given uid.
     *
     * @param uid The uid of the node.
     *
     * @throws NodeManagerNodeNotFoundException When the file with the given uid doesn't exist.
     *
     * @return The found node.
     */
    Node findNode(int32_t uid);


    void WriteToNode(const Node &node, void *source, size_t size);
    void WriteToNode(const Node &node, std::istream &source, size_t size);

    void ReadFromNode(const Node &node, void *destination, size_t size);
    void ReadFromNode(const Node &node, std::ostream &destination, size_t size);

private:
    /**
     * The ntfs partition on which will this node manager operate.
     */
    Partition &m_partition;

    /**
     * Get a free unique id within the partition mft.
     *
     * @return A free unique id.
     */
    int32_t GetFreeUid();

    /**
     * Find sufficient amount of free clusters for the node of the given size.
     * First tries to find one undivided fragment, if it fails, tries to find
     * free clusters in multiple fragments.
     *
     * @param size The minimal capacity of found clusters.
     * @return The vector of free fragments.
     */
    std::vector<mft_fragment> FindFreeFragments(int32_t size);

    /**
     * Find sufficient amount of free mft items for the given number of fragments.
     *
     * @param fragmentCount The number of fragments that must fit into the mft items.
     * @return The found free mft items.
     */
    std::vector<MftItem> FindFreeMftItems(size_t fragmentCount);

    /**
     * Set the values of mft items according the to given node properties, fill them
     * with the given fragments, set appropriate order and count of the mft items
     * and sort the by their order.
     *
     * @param mftItems The mft items to be set up.
     * @param uid The uid of node.
     * @param name The name of the node.
     * @param isDirectory True if the node is a directory, false if it's a file.
     * @param size The size of the node contents.
     * @param fragments The node fragments.
     */
    void SetupMftItems(std::vector<MftItem>& mftItems, int32_t uid, std::string name, bool isDirectory, int32_t size, const std::vector<mft_fragment> &fragments);
};
