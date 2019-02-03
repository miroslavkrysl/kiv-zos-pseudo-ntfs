#pragma once

#include <string>
#include <fstream>
#include <vector>

#include "NtfsStructs.h"

/**
 * The class Partition is a wrapper for the ntfs partition file.
 */
class Partition
{
public:
    /**
     * Initialize a Partition instance bound to the given file.
     * Tries to open the file, read boot record and does the basic check.
     * If one of that fails, partition remains closed and need to be formatted
     *
     * @param path The path of the partition file.
     */
    explicit Partition(std::string path);

    /**
     * Create a file if it doesn't exist or overwrite the old one,
     * compute required mft size, bitmap size, data segment size
     * and the final total disk size. Finally it writes all required
     * structs into the file.
     * Resulting file is of equal or smaller size than the given size.
     *
     * @param size The size of the partition.
     * @param signature The creators login name.
     * @param description The description of the partition.
     * @throws PartitionFileNotOpenedException If it fails to open the partition file.
     */
    void Format(int32_t size, const std::string &signature, const std::string &description);

    /**
     * Read the mft item on the given index from the partition.
     *
     * @param index The index of the mft item to be read.
     *
     * @throws PartitionMftOutOfBoundsException When the mft item index is out of bounds.
     */
    MftItem ReadMftItem(int32_t index);

    /**
     * Read all mft items with the given uid from the partition
     * and sort them by their order.
     * It calls the ReadMftItem function in the loop.
     *
     * @param items The vector of MftItems to be read.
     */
    std::vector<MftItem> ReadMftItems(int32_t uid);

    /**
     * Write the mft item into its position on the partition.
     *
     * @param item The MftItem to be written.
     *
     * @throws PartitionMftOutOfBoundsException When the mft item index is out of bounds.
     */
    void WriteMftItem(MftItem &item);

    /**
     * Write the given mft items into their position on the partition.
     * It calls the WriteMftItem function in the loop.
     *
     * @param items The vector of MftItems to be written.
     */
    void WriteMftItems(std::vector<MftItem> &items);

    /**
     * Read the value of the bitmap bit on the given index.
     *
     * @param index The index of the bit in the bitmap.
     *
     * @throws PartitionBitmapOutOfBoundsException When the bit index is out of bounds.
     *
     * @return The value of the bit on the given index.
     */
    bool ReadBitmapBit(int32_t index);

    /**
     * Write the bitmap bit on the given index.
     *
     * @param index The index of the bit in the bitmap.
     * @param bit The value of the bit to be written.
     *
     * @throws PartitionBitmapOutOfBoundsException When the bit index is out of bounds.
     */
    void WriteBitmapBit(int32_t index, bool bit);

    /**
     * Read the data from the cluster into the destination address.
     *
     * @param index The index of the cluster.
     * @param destination The pointer to the data destination.
     * @param dataSize The size of the data in bytes.
     *
     * @throws PartitionDataOutOfBoundsException When the cluster index is out of bounds.
     * @throws PartitionClusterOverflowException When the dataSize is bigger than the cluster capacity.
     */
    void ReadCluster(int32_t index, void *destination, size_t dataSize);

    /**
     * Write the data into the cluster from the source address.
     *
     * @param index The index of the cluster.
     * @param source The pointer to the data source.
     * @param dataSize The size of the data in bytes.
     *
     * @throws PartitionDataOutOfBoundsException When the cluster index is out of bounds.
     * @throws PartitionClusterOverflowException When the dataSize is bigger than the cluster capacity.
     */
    void WriteCluster(int32_t index, void *source, size_t dataSize);

    /**
     * Read the data from the clusters into the destination address.
     * It calls the ReadCluster function in the loop.
     *
     * @param indexes The vector of the cluster indexes.
     * @param destination The pointer to the data destination.
     * @param dataSize The size of the data in bytes.
     *
     * @throws PartitionClusterOverflowException When the dataSize is bigger than the clusters capacity.
     */
    void ReadClusters(std::vector<int32_t> indexes, void *destination, size_t dataSize);

    /**
     * Write the data from the source address into the clusters.
     * It calls the WriteCluster function in the loop.
     *
     * @param indexes The vector of the cluster indexes.
     * @param source The pointer to the data source.
     * @param dataSize The size of the data in bytes.
     *
     * @throws PartitionClusterOverflowException When the dataSize is bigger than the clusters capacity.
     */
    void WriteClusters(std::vector<int32_t> indexes, void *source, size_t dataSize);

    /**
     * Check whether the partition file is opened.
     *
     * @return True if so, false otherwise.
     */
    bool IsOpened();

    /**
     * Get the partition boot record.
     *
     * @throws PartitionFileNotOpenedException When the partition file isn't opened.
     * @return The partition boot record.
     */
    boot_record GetBootRecord();

    /**
     * Get the partition signature (creators login name).
     *
     * @throws PartitionFileNotOpenedException When the partition file isn't opened.
     * @return The partition signature.
     */
    std::string GetSignature();

    /**
     * Get the partition description
     *
     * @throws PartitionFileNotOpenedException When the partition file isn't opened.
     * @return The partition description.
     */
    std::string GetDescription();

    /**
     * Get the partition mft start address.
     *
     * @throws PartitionFileNotOpenedException When the partition file isn't opened.
     * @return The partition mft start address.
     */
    int32_t GetMftStartAddress();

    /**
     * Get the partition bitmap start address.
     *
     * @throws PartitionFileNotOpenedException When the partition file isn't opened.
     * @return The partition bitmap start address.
     */
    int32_t GetBitmapStartAddress();

    /**
     * Get the partition data start address.
     *
     * @throws PartitionFileNotOpenedException When the partition file isn't opened.
     * @return The partition data start address.
     */
    int32_t GetDataStartAddress();

    /**
     * Get the partition mft item count.
     *
     * @throws PartitionFileNotOpenedException When the partition file isn't opened.
     * @return The partition mft item count.
     */
    int32_t GetMftItemCount();

    /**
     * Get the partition mft max fragments count.
     *
     * @throws PartitionFileNotOpenedException When the partition file isn't opened.
     * @return The partition mft max fragments count.
     */
    int32_t GetMftMaxFragmentsCount();

    /**
     * Get the partition cluster count.
     *
     * @throws PartitionFileNotOpenedException When the partition file isn't opened.
     * @return The partition cluster count.
     */
    int32_t GetClusterCount();

    /**
     * Get the partition cluster size.
     *
     * @throws PartitionFileNotOpenedException When the partition file isn't opened.
     * @return The partition cluster size.
     */
    int32_t GetClusterSize();

    /**
     * Get the partition size.
     *
     * @throws PartitionFileNotOpenedException When the partition file isn't opened.
     * @return The partition size.
     */
    int32_t GetPartitionSize();

private:
    /**
     * The ntfs partition file path.
     */
    std::string m_path;

    /**
     * The ntfs partition file stream.
     */
    std::fstream m_file;

    /**
     * The ntfs boot record loaded from the partition file.
     */
    boot_record m_bootRecord;

    /**
     * Read data from the given position on the partition.
     *
     * @param position The read position.
     * @param destination The pointer to the data destination.
     * @param size The size of the data in bytes.
     *
     * @throws PartitionFileNotOpenedException When the partition file isn't opened.
     * @throws PartitionOutOfBoundsException When the position is out of partition bounds.
     */
    void Read(int32_t position, void *destination, size_t size);

    /**
     * Write data to the given position on the partition.
     *
     * @param position The write position.
     * @param source The pointer to the data source.
     * @param size The size of the data in bytes.
     *
     * @throws PartitionFileNotOpenedException When the partition file isn't opened.
     * @throws PartitionOutOfBoundsException When the position is out of partition bounds.
     */
    void Write(int32_t position, void *source, size_t size);

    /**
     * Do a basic boot record values validation.
     *
     * @param bootRecord The boot record to be validated.
     * @return True if the boot record values passed, false otherwise.
     */
    bool ValidateBootRecord(boot_record &bootRecord);

    /**
     * Compute the total count of mft items in partition of the given size.
     *
     * @param partitionSize The size of the partition.
     * @return The count of mft items.
     */
    int32_t ComputeMftItemCount(int32_t partitionSize);

    /**
     * Compute the total count of clusters that will fit into the given size
     * of bitmap and data segment together.
     * @param bitmapAndDataBlockSize The size that remains for the bitmap and the data segment.
     * @return The count of clusters.
     */
    int32_t ComputeClusterCount(int32_t bitmapAndDataBlockSize);
};


