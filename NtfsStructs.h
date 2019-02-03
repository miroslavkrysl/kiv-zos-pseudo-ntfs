#pragma once

#include <cstdint>


const std::size_t NODE_NAME_SIZE{12};                   // the size of the node name field (including the termination symbol)
const int32_t MFT_FRAGMENTS_COUNT{32};                  // the max number of fragments per one mft item
const int32_t FRAGMENT_UNUSED_START{-1};        // the max number of fragments per one mft item
const int32_t UID_ITEM_FREE{0};                         // the uid of a free mft item
const int32_t UID_ROOT{1};                              // the uid of the root directory
const bool BIT_CLUSTER_FREE{false};                     // the boolean value of bit in a bitmap representing a free cluster
const double MFT_SIZE_RELATIVE_TO_PARTITION_SIZE{0.1};  // the ratio of size, that takes the mft relative to the total partition size
const int32_t CLUSTER_SIZE{1024};                       // the size of one cluster in bytes

/**
 * The representation of ntfs boot record as it lays in memory
 */
struct boot_record
{
    char signature[9];                                  // filesystem authors login
    char description[251];                              // filesystem description
    int32_t partition_size;                             // total partition size
    int32_t cluster_size;                               // size of one cluster
    int32_t cluster_count;                              // the total number of clusters
    int32_t mft_start_address;                          // the mft start address on partition
    int32_t bitmap_start_address;                       // the bitmap start address on partition
    int32_t data_start_address;                         // the data start address on partition
    int32_t mft_max_fragment_count;                     // the max number of fragment per one mft item
};

/**
 * The representation of mft fragment as it lays in memory
 */
struct mft_fragment
{
    int32_t start;                              // the start address of the first cluster
    int32_t count;                                      // the number of clusters
};

/**
 * The representation of mft item as it lays in memory
 */
struct mft_item
{
    int32_t uid;                                        // the uid of the node
    bool is_directory;                                  // is a directory or file
    int8_t order;                                       // the order of mft within the node
    int8_t count;                                       // the total count of mft items within the node
    char name[NODE_NAME_SIZE];                          // the name of the file 8 + 3 + `/0`
    int32_t size;                                  // the size of the node in bytes
    struct mft_fragment fragments[MFT_FRAGMENTS_COUNT]; // the fragments of the node
};

/**
 * Helper structure to hold the mft item together with its index in mft
 */
struct MftItem
{
    int32_t index;                                      // the index of the mft item in the mft
    mft_item item;                                      // the mft item itself
};


const uint32_t MAX_PARTITION_SIZE = INT32_MAX;                    // the max size of the partition in bytes
const uint32_t MIN_PARTITION_SIZE = sizeof(boot_record)
    + sizeof(mft_item) * 2 // min two mft items (roo + 1 node)
    + 1 // minimum 1 byte for bitmap
    + CLUSTER_SIZE;                                     // the min size of the partition