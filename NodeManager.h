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
    friend class NtfsChecker;
public:
    /**
     * Initializes a new NodeManager, that will opperate on the given partition.
     *
     * @param partition The ntfs partiton.
     */
    explicit NodeManager(Partition &partition);

    /**
     * Get the nodes clusters total capacity.
     *
     * @param node The node.
     *
     * @return The capacity in bytes.
     */
    int32_t GetNodeCapacity(const Node &node) const;

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
     * If the current node capacity is sufficient
     * and isn't unnecessarily big, the node remains unchanged.
     *
     * @param node The node to be resized.
     * @param size The new size of the node contents.
     *
     * @throws NodeManagerNotEnoughFreeClustersException When there are not enough free clusters for the new size.
     * @throws NodeManagerNotEnoughFreeMftItemsException When there are not enough free mft items for the fragments.
     */
    void ResizeNode(Node &node, int32_t size);

    /**
     * Rename the node.
     *
     * @param node The node to be renamed.
     * @param name The new node name.
     */
    void RenameNode(Node &node, std::string name);

    /**
     * Clone the given node into a new node with a different uid and own name.
     * Allocates resources for the new node and copies the cloned node
     * properties and its contents.
     *
     * @param node The node to be cloned.
     * @param name The name of the clone.
     *
     * @throws NodeManagerNotEnoughFreeClustersException When there are not enough free clusters for the clone.
     * @throws NodeManagerNotEnoughFreeMftItemsException When there are not enough free mft items for the number of fragments required by the clone.
     *
     * @return The new node with own uid - clone of the given node.
     */
    Node CloneNode(const Node &node, std::string name);

    /**
     * Find the node with the given uid.
     *
     * @param uid The uid of the node.
     *
     * @throws NodeManagerNodeNotFoundException When the file with the given uid doesn't exist.
     *
     * @return The found node.
     */
    Node FindNode(int32_t uid);

    /**
     * Write data from the given source into the partition clusters owned by the given node.
     * The size of the data is determined by the node size.
     *
     * @param node The node which contents will be written into.
     * @param source The pointer to the data source.
     */
    void WriteIntoNode(const Node &node, void *source);

    /**
     * Write data from the given input stream into the partition clusters owned by the given node.
     * The size of the data is determined by the node size.
     *
     * @param node The node which contents will be written into.
     * @param source The input stream.
     */
    void WriteIntoNode(const Node &node, std::istream &source);

    /**
     * Read data from the partition clusters owned by the given node into the given destination.
     * The size of the data is determined by the node size.
     *
     * @param node The node which contents will be read.
     * @param destination The pointer to the data destination.
     */
    void ReadFromNode(const Node &node, void *destination);

    /**
     * Read data from the partition clusters owned by the given node into the given output stream.
     * The size of the data is determined by the node size.
     *
     * @param node The node which contents will be read.
     * @param destination The output stream.
     */
    void ReadFromNode(const Node &node, std::ostream &destination);

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
