#include <utility>
#include <algorithm>

#include "Partition.h"
#include "Exceptions/PartitionExceptions.h"
#include "ConsistencyChecker.h"

Partition::Partition(std::string path)
    : m_path(std::move(path))
{
    // try to open file for reading
    // it does not overwrite existing file and does not create a new file if it not exists
    m_file.open(path, std::ios::in);


    if (!m_file.is_open()) {
        // file does not exist, partition is not formatted

        m_file.close();
        return;
    }

    // open file for reading and writing
    m_file.open(path, std::ios::in | std::ios::out | std::ios::binary);

    if (!m_file.is_open()) {
        throw PartitionFileNotOpenedException{"can not open file " + path};
    }

    // try to read boot record
    try {
        m_bootRecord = ReadBootRecord();

        if (!ConsistencyChecker::validateBootRecord(m_bootRecord)) {
            m_file.close();
            throw PartitionCorruptedException{"the partitions boot record contains invalid data"};
        }

        if (!ConsistencyChecker::checkPartitionSize(m_file, m_bootRecord.partition_size)) {
            m_file.close();
            throw PartitionCorruptedException{"the partition size stated in boot record doesn't correspond with the actual size"};
        }
    }
    catch (PartitionOutOfBoundsException &exception) {
        // cant read boot record
        m_file.close();
        throw PartitionCorruptedException{"can't read the partitions boot record"};
    }
}

// done
MftItem Partition::ReadMftItem(int32_t index)
{
    if (index < 0 || index >= GetMftItemCount()) {
        throw PartitionMftOutOfBoundsException{"mft item index " + std::to_string(index) + " is out of bounds"};
    }

    int32_t address = GetMftStartAddress() + index * sizeof(mft_item);

    MftItem item;
    item.index = index;
    Read(address, &item.item, sizeof(mft_item));

    return item;
}

// done
std::vector<MftItem> Partition::ReadMftItems(int32_t uid)
{
    std::vector<MftItem> items;

    for (int i = 0; i < GetMftItemCount(); i++) {
        MftItem item = ReadMftItem(i);

        if (item.item.uid == uid) {
            items.emplace_back(item);
        }
    }

    std::sort(items.begin(), items.end(), [](MftItem &item1, MftItem &item2) {
        return item1.item.order < item2.item.order;
    });

    return items;
}

// done
void Partition::WriteMftItem(MftItem &item)
{
    if (item.index < 0 || item.index >= GetMftItemCount()) {
        throw PartitionMftOutOfBoundsException{"mft item index " + std::to_string(item.index) + " is out of bounds"};
    }

    int32_t address = GetMftStartAddress() + item.index * sizeof(mft_item);

    Write(address, &item.item, sizeof(mft_item));
}

// done
void Partition::WriteMftItems(std::vector<MftItem> &items)
{
    for (auto &item : items) {
        WriteMftItem(item);
    }
}

// done
bool Partition::ReadBitmapBit(int32_t index)
{
    if (index < 0 || index >= GetClusterCount()) {
        throw PartitionBitmapOutOfBoundsException{"bitmap bit index " + std::to_string(index) + " is out of bounds"};
    }

    int32_t byteIndex = index / 8;
    int32_t bitOffset = index % 8;

    uint8_t byte;
    Read(GetBitmapStartAddress() + byteIndex, &byte, sizeof(uint8_t));

    return static_cast<bool>(byte & (1 << bitOffset));
}

// done
void Partition::WriteBitmapBit(int32_t index, bool bit)
{
    if (index < 0 || index >= GetClusterCount()) {
        throw PartitionBitmapOutOfBoundsException{"bitmap bit index " + std::to_string(index) + " is out of bounds"};
    }

    int32_t byteIndex = index / 8;
    int32_t bitOffset = index % 8;

    uint8_t byte;
    Read(GetBitmapStartAddress() + byteIndex, &byte, sizeof(uint8_t));

    if (bit) {
        byte |= (1 << bitOffset);
    } else {
        byte &= ~(1 << bitOffset);
    }

    Write(GetBitmapStartAddress() + byteIndex, &byte, sizeof(uint8_t));
}

// done
void Partition::ReadCluster(int32_t index, void *destination, size_t dataSize)
{
    if (index < 0 || index >= GetClusterCount()) {
        throw PartitionDataOutOfBoundsException{"cluster index " + std::to_string(index) + " is out of bounds"};
    }

    if (dataSize > GetClusterSize()) {
        throw PartitionClusterOverflowException{"trying to read more data than is the cluster size"};
    }

    int32_t address = GetDataStartAddress() + index * GetClusterSize();

    Write(address, destination, dataSize);
}

// done
void Partition::ReadClusters(std::vector<int32_t> indexes, void *destination, size_t dataSize)
{
    if (dataSize > GetClusterSize() * indexes.size()) {
        throw PartitionClusterOverflowException{"trying to read more data than is the clusters total size"};
    }

    auto dest = static_cast<char *>(destination);
    int32_t clusterSize = GetClusterSize();

    for (auto &index : indexes) {
        if (dataSize <= 0) {
            break;
        }

        ReadCluster(index, dest, dataSize > clusterSize ? clusterSize : dataSize);

        dataSize -= clusterSize;
        dest += clusterSize;
    }
}

// done
void Partition::WriteCluster(int32_t index, void *source, size_t dataSize)
{
    if (index < 0 || index >= GetClusterCount()) {
        throw PartitionDataOutOfBoundsException{"cluster index " + std::to_string(index) + " is out of bounds"};
    }

    if (dataSize > GetClusterSize()) {
        throw PartitionClusterOverflowException{"trying to write more data than fits into the cluster"};
    }

    int32_t address = GetDataStartAddress() + index * GetClusterSize();

    Read(address, source, dataSize);
}

// done
void Partition::WriteClusters(std::vector<int32_t> indexes, void *source, size_t dataSize)
{
    if (dataSize > GetClusterSize() * indexes.size()) {
        throw PartitionClusterOverflowException{"trying to write more data than fits into the clusters"};
    }

    auto *src = static_cast<char *>(source);
    int32_t clusterSize = GetClusterSize();

    for (auto &index : indexes) {
        if (dataSize <= 0) {
            break;
        }

        WriteCluster(index, src, dataSize > clusterSize ? clusterSize : dataSize);

        dataSize -= clusterSize;
        src += clusterSize;
    }
}

// done
bool Partition::IsOpened()
{
    return m_file.is_open();
}

// done
std::string Partition::GetSignature()
{
    if (!IsOpened()) {
        throw PartitionFileNotOpenedException{"partition file is not opened, probably not formatted"};
    }

    return std::string{m_bootRecord.signature};
}

// done
std::string Partition::GetDescription()
{
    if (!IsOpened()) {
        throw PartitionFileNotOpenedException{"partition file is not opened, probably not formatted"};
    }

    return std::string{m_bootRecord.description};
}

// done
int32_t Partition::GetMftStartAddress()
{
    if (!IsOpened()) {
        throw PartitionFileNotOpenedException{"partition file is not opened, probably not formatted"};
    }

    return m_bootRecord.mft_start_address;
}

// done
int32_t Partition::GetBitmapStartAddress()
{
    if (!IsOpened()) {
        throw PartitionFileNotOpenedException{"partition file is not opened, probably not formatted"};
    }

    return m_bootRecord.bitmap_start_address;
}

// done
int32_t Partition::GetDataStartAddress()
{
    if (!IsOpened()) {
        throw PartitionFileNotOpenedException{"partition file is not opened, probably not formatted"};
    }

    return m_bootRecord.data_start_address;
}

// done
int32_t Partition::GetMftItemCount()
{
    if (!IsOpened()) {
        throw PartitionFileNotOpenedException{"partition file is not opened, probably not formatted"};
    }

    return (m_bootRecord.bitmap_start_address - m_bootRecord.mft_start_address) / sizeof(mft_item);
}

// done
int32_t Partition::GetMftMaxFragmentsCount()
{
    if (!IsOpened()) {
        throw PartitionFileNotOpenedException{"partition file is not opened, probably not formatted"};
    }

    return m_bootRecord.mft_max_fragment_count;
}

// done
int32_t Partition::GetClusterCount()
{
    if (!IsOpened()) {
        throw PartitionFileNotOpenedException{"partition file is not opened, probably not formatted"};
    }

    return m_bootRecord.cluster_count;
}

// done
int32_t Partition::GetClusterSize()
{
    if (!IsOpened()) {
        throw PartitionFileNotOpenedException{"partition file is not opened, probably not formatted"};
    }

    return m_bootRecord.cluster_size;
}

// done
int32_t Partition::GetPartitionSize()
{
    if (!IsOpened()) {
        throw PartitionFileNotOpenedException{"partition file is not opened, probably not formatted"};
    }

    return m_bootRecord.partition_size;
}

// done
void Partition::Read(int32_t position, void *destination, size_t size)
{
    if (!IsOpened()) {
        throw PartitionFileNotOpenedException{"partition file is not opened, probably not formatted"};
    }

    if (position < 0 || position + size - 1 >= GetPartitionSize()) {
        throw PartitionOutOfBoundsException{"trying to read outside of the partition"};
    }

    m_file.seekg(position);
    m_file.read(static_cast<char *>(destination), size);
}

// done
void Partition::Write(int32_t position, void *source, size_t size)
{
    if (!IsOpened()) {
        throw PartitionFileNotOpenedException{"partition file is not opened, probably not formatted"};
    }

    if (position < 0 || position + size - 1 >= GetPartitionSize()) {
        throw PartitionOutOfBoundsException{"trying to write outside of the partition"};
    }

    m_file.seekp(position);
    m_file.write(static_cast<const char *>(source), size);
}

// done
boot_record Partition::ReadBootRecord()
{
    boot_record bootRecord;
    Read(0, &bootRecord, sizeof(bootRecord));
    return bootRecord;
}
