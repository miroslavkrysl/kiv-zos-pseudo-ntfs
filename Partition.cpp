#include <utility>
#include <algorithm>
#include <cmath>
#include <cstring>

#include "Partition.h"
#include "Exceptions/PartitionExceptions.h"

Partition::Partition(std::string path)
    : m_path(std::move(path))
{
    // try to open file for reading
    // it does not overwrite existing file and does not create a new file if it not exists
    m_file.open(m_path, std::ios::in);


    if (!m_file.is_open()) {
        // file does not exist, partition is not formatted

        m_file.close();
        return;
    }

    // open file for reading and writing
    m_file.close();
    m_file.open(m_path, std::ios::in | std::ios::out | std::ios::binary);

    if (!m_file.is_open()) {
        throw PartitionFileNotOpenedException{"can not open file " + m_path};
    }

    // try to read boot record
    m_file.read(reinterpret_cast<char *>(&m_bootRecord), sizeof(m_bootRecord));

    if (m_file.eof()) {
        // cant read boot record
        m_file.close();
        throw PartitionCorruptedException{"can't read the partitions boot record"};
    }

    if (!ValidateBootRecord(m_bootRecord)) {
        m_file.close();
        throw PartitionCorruptedException{"the partitions boot record contains invalid data"};
    }
}

void Partition::Format(int32_t size, std::string signature, std::string description)
{
    // check arguments
    if (size > MAX_PARTITION_SIZE) {
        throw PartitionFormatException("max partition size " + std::to_string(MAX_PARTITION_SIZE) + " exceeded");
    }

    if (size < MIN_PARTITION_SIZE) {
        throw PartitionFormatException("min partition size " + std::to_string(MIN_PARTITION_SIZE) + " not reached");
    }

    if (signature.length() >= sizeof(boot_record::signature) - 1) {
        throw PartitionFormatException("max signature length is " + std::to_string(sizeof(boot_record::signature) - 1));
    }

    if (description.length() >= sizeof(boot_record::description) - 1) {
        throw PartitionFormatException(
            "max description length is " + std::to_string(sizeof(boot_record::description) - 1));
    }

    // close previously opened partition file
    m_file.close();

    // open partition file and clear its contents
    m_file.open(m_path, std::ios::binary | std::ios::trunc | std::ios::in | std::ios::out);

    // init partition info
    int32_t mftItemCount = ComputeMftItemCount(size);
    int32_t mftSize = mftItemCount * sizeof(mft_item);

    int32_t clusterCount = ComputeClusterCount(size - mftSize);
    int32_t dataSegmentSize = clusterCount * CLUSTER_SIZE;
    auto bitmapSize = static_cast<int32_t>(std::ceil(clusterCount / 8.0));

    // initialize boot record
    m_bootRecord = boot_record{};

    std::strncpy(m_bootRecord.signature, signature.c_str(), sizeof(m_bootRecord.signature));
    m_bootRecord.signature[sizeof(m_bootRecord.signature) - 1] = '\0';

    std::strncpy(m_bootRecord.description, description.c_str(), sizeof(m_bootRecord.description));
    m_bootRecord.description[sizeof(m_bootRecord.description) - 1] = '\0';

    m_bootRecord.partition_size = sizeof(m_bootRecord) + mftSize + bitmapSize + dataSegmentSize;
    m_bootRecord.cluster_size = CLUSTER_SIZE;
    m_bootRecord.cluster_count = clusterCount;
    m_bootRecord.mft_start_address = sizeof(boot_record);
    m_bootRecord.bitmap_start_address = sizeof(boot_record) + mftSize;
    m_bootRecord.data_start_address = sizeof(boot_record) + mftSize + bitmapSize;
    m_bootRecord.mft_max_fragment_count = MFT_FRAGMENTS_COUNT;

    // write boot record
    m_file.write(reinterpret_cast<const char *>(&m_bootRecord), sizeof(boot_record));

    // write mft
    for (int i = 0; i < mftItemCount; ++i) {
        mft_item mftItem{};
        mftItem.uid = UID_ITEM_FREE;

        m_file.write(reinterpret_cast<const char *>(&mftItem), sizeof(mft_item));
    }

    // write bitmap
    for (int j = 0; j < bitmapSize; ++j) {
        uint8_t byte{0};
        m_file.write(reinterpret_cast<const char *>(&byte), sizeof(byte));
    }

    // write clusters
    for (int k = 0; k < clusterCount; ++k) {
        uint8_t cluster[CLUSTER_SIZE];
        m_file.write(reinterpret_cast<const char *>(&cluster), CLUSTER_SIZE);
    }

    m_file.flush();

    // create root directory
    int32_t uid = UID_ROOT;

    MftItem rootMftItem;
    rootMftItem.index = 0;

    rootMftItem.item.uid = uid;
    rootMftItem.item.is_directory = true;
    rootMftItem.item.size = sizeof(int32_t);
    rootMftItem.item.order = 0;
    rootMftItem.item.count = 1;
    std::strncpy(rootMftItem.item.name, "/", sizeof(mft_item::name) - 1);
    rootMftItem.item.name[sizeof(mft_item::name) - 1] = '\0';

    rootMftItem.item.fragments[0].start = 0;
    rootMftItem.item.fragments[0].count = 1;

    mft_fragment unusedFragment{FRAGMENT_UNUSED_START, 0};
    for (int i = 1; i < MFT_FRAGMENTS_COUNT; i++) {
        rootMftItem.item.fragments[i] = unusedFragment;
    }

    // write root directory
    WriteMftItem(rootMftItem);
    WriteBitmapBit(0, true);
    WriteCluster(0, &uid, sizeof(int32_t));
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
void Partition::WriteMftItem(const MftItem &item)
{
    if (item.index < 0 || item.index >= GetMftItemCount()) {
        throw PartitionMftOutOfBoundsException{"mft item index " + std::to_string(item.index) + " is out of bounds"};
    }

    int32_t address = GetMftStartAddress() + item.index * sizeof(mft_item);

    Write(address, &item.item, sizeof(mft_item));
}

// done
void Partition::WriteMftItems(const std::vector<MftItem> &items)
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

    Read(address, destination, dataSize);
}

// done
void Partition::ReadClusters(const std::vector<int32_t> &indexes, void *destination, size_t dataSize)
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
void Partition::ReadClusters(const std::vector<int32_t> &indexes, std::ostream &destination, size_t dataSize)
{
    if (dataSize > GetClusterSize() * indexes.size()) {
        throw PartitionClusterOverflowException{"trying to read more data than is the clusters total size"};
    }

    int32_t clusterSize = GetClusterSize();

    std::vector<char> data;
    data.resize(static_cast<unsigned long>(clusterSize));

    for (auto &index : indexes) {
        if (dataSize <= 0) {
            break;
        }

        size_t toRead = dataSize > clusterSize ? clusterSize : dataSize;

        ReadCluster(index, data.data(), toRead);
        destination.write(data.data(), toRead);

        dataSize -= clusterSize;
    }
}

// done
void Partition::WriteCluster(int32_t index, const void *source, size_t dataSize)
{
    if (index < 0 || index >= GetClusterCount()) {
        throw PartitionDataOutOfBoundsException{"cluster index " + std::to_string(index) + " is out of bounds"};
    }

    if (dataSize > GetClusterSize()) {
        throw PartitionClusterOverflowException{"trying to write more data than fits into the cluster"};
    }

    int32_t address = GetDataStartAddress() + index * GetClusterSize();

    Write(address, source, dataSize);
}

// done
void Partition::WriteClusters(const std::vector<int32_t> &indexes, const void *source, size_t dataSize)
{
    if (dataSize > GetClusterSize() * indexes.size()) {
        throw PartitionClusterOverflowException{"trying to write more data than fits into the clusters"};
    }

    auto *src = static_cast<const char *>(source);
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
void Partition::WriteClusters(const std::vector<int32_t> &indexes, std::istream &source, size_t dataSize)
{
    if (dataSize > GetClusterSize() * indexes.size()) {
        throw PartitionClusterOverflowException{"trying to read more data than is the clusters total size"};
    }

    int32_t clusterSize = GetClusterSize();

    std::vector<char> data;
    data.resize(static_cast<unsigned long>(clusterSize));

    for (auto &index : indexes) {
        if (dataSize <= 0) {
            break;
        }

        size_t toWrite = dataSize > clusterSize ? clusterSize : dataSize;

        source.read(data.data(), toWrite);
        WriteCluster(index, data.data(), toWrite);

        dataSize -= clusterSize;
    }
}

// done
bool Partition::IsOpened() const
{
    return m_file.is_open();
}

// done
boot_record Partition::GetBootRecord() const
{
    if (!IsOpened()) {
        throw PartitionFileNotOpenedException{"partition file is not opened, probably not formatted"};
    }

    return m_bootRecord;
}

// done
std::string Partition::GetSignature() const
{
    if (!IsOpened()) {
        throw PartitionFileNotOpenedException{"partition file is not opened, probably not formatted"};
    }

    return std::string{m_bootRecord.signature};
}

// done
std::string Partition::GetDescription() const
{
    if (!IsOpened()) {
        throw PartitionFileNotOpenedException{"partition file is not opened, probably not formatted"};
    }

    return std::string{m_bootRecord.description};
}

// done
int32_t Partition::GetMftStartAddress() const
{
    if (!IsOpened()) {
        throw PartitionFileNotOpenedException{"partition file is not opened, probably not formatted"};
    }

    return m_bootRecord.mft_start_address;
}

// done
int32_t Partition::GetBitmapStartAddress() const
{
    if (!IsOpened()) {
        throw PartitionFileNotOpenedException{"partition file is not opened, probably not formatted"};
    }

    return m_bootRecord.bitmap_start_address;
}

// done
int32_t Partition::GetDataStartAddress() const
{
    if (!IsOpened()) {
        throw PartitionFileNotOpenedException{"partition file is not opened, probably not formatted"};
    }

    return m_bootRecord.data_start_address;
}

// done
int32_t Partition::GetMftItemCount() const
{
    if (!IsOpened()) {
        throw PartitionFileNotOpenedException{"partition file is not opened, probably not formatted"};
    }

    return (m_bootRecord.bitmap_start_address - m_bootRecord.mft_start_address) / sizeof(mft_item);
}

// done
int32_t Partition::GetMftMaxFragmentsCount() const
{
    if (!IsOpened()) {
        throw PartitionFileNotOpenedException{"partition file is not opened, probably not formatted"};
    }

    return m_bootRecord.mft_max_fragment_count;
}

// done
int32_t Partition::GetClusterCount() const
{
    if (!IsOpened()) {
        throw PartitionFileNotOpenedException{"partition file is not opened, probably not formatted"};
    }

    return m_bootRecord.cluster_count;
}

// done
int32_t Partition::GetClusterSize() const
{
    if (!IsOpened()) {
        throw PartitionFileNotOpenedException{"partition file is not opened, probably not formatted"};
    }

    return m_bootRecord.cluster_size;
}

// done
int32_t Partition::GetPartitionSize() const
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

    m_file.flush();
    m_file.seekg(position);
    m_file.read(static_cast<char *>(destination), size);
}

// done
void Partition::Write(int32_t position, const void *source, size_t size)
{
    if (!IsOpened()) {
        throw PartitionFileNotOpenedException{"partition file is not opened, probably not formatted"};
    }

    if (position < 0 || position + size - 1 >= GetPartitionSize()) {
        throw PartitionOutOfBoundsException{"trying to write outside of the partition"};
    }

    m_file.seekp(position);
    m_file.write(static_cast<const char *>(source), size);
    m_file.flush();
}

// done
bool Partition::ValidateBootRecord(const boot_record &bootRecord) const
{
    if (bootRecord.signature[sizeof(bootRecord.signature) - 1] != '\0') {
        return false;
    }
    if (bootRecord.description[sizeof(bootRecord.description) - 1] != '\0') {
        return false;
    }
    if (bootRecord.partition_size < MIN_PARTITION_SIZE) {
        return false;
    }
    if (bootRecord.cluster_size <= 0 || bootRecord.cluster_size % sizeof(int32_t) != 0) {
        return false;
    }
    if (bootRecord.cluster_count < 1) {
        return false;
    }
    if (bootRecord.mft_start_address <= 0) {
        return false;
    }
    if (bootRecord.bitmap_start_address <= 0) {
        return false;
    }
    if (bootRecord.data_start_address <= 0) {
        return false;
    }
    if (bootRecord.mft_max_fragment_count <= 0) {
        return false;
    };

    return true;
}

// done
int32_t Partition::ComputeMftItemCount(int32_t partitionSize) const
{
    return static_cast<int32_t>(MFT_SIZE_RELATIVE_TO_PARTITION_SIZE * partitionSize) / sizeof(mft_item);
}

// done
int32_t Partition::ComputeClusterCount(int32_t bitmapAndDataBlockSize) const
{
    int32_t clusterCount = (8 * bitmapAndDataBlockSize) / (1 + 8 * CLUSTER_SIZE);

    return static_cast<int32_t>(clusterCount);
}
