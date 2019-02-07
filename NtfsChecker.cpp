#include <limits>
#include <cmath>
#include "NtfsChecker.h"
#include "Text.h"
#include "Exceptions/PartitionExceptions.h"
#include "NodeSizeChecker.h"

// done
NtfsChecker::NtfsChecker(Ntfs &ntfs)
    : m_ntfs(ntfs)
{}

// done
void NtfsChecker::PrintBootRecord(std::ostream &output)
{
    Partition &partition = m_ntfs.m_partition;

    if (!partition.IsOpened()) {
        throw PartitionFileNotOpenedException{"partition file is not opened"};
    }

    output << Text::hline(61) << std::endl;
    output << "           Signature: " << partition.GetSignature() << std::endl;
    output << "         Description: " << partition.GetDescription() << std::endl;
    output << "      Partition size: " << partition.GetPartitionSize() << std::endl;
    output << "        Cluster size: " << partition.GetClusterSize() << std::endl;
    output << "       Cluster count: " << partition.GetClusterCount() << std::endl;
    output << "      Mft item count: " << partition.GetMftItemCount() << std::endl;
    output << "   Mft start address: " << partition.GetMftStartAddress() << std::endl;
    output << "Bitmap start address: " << partition.GetBitmapStartAddress() << std::endl;
    output << "  Data start address: " << partition.GetDataStartAddress() << std::endl;
    output << "   Mft max fragments: " << partition.GetMftMaxFragmentsCount() << std::endl;
    output << Text::hline(61) << std::endl;
}

// done
void NtfsChecker::PrintMft(std::ostream &output, bool printAll)
{
    Partition &partition = m_ntfs.m_partition;

    if (!partition.IsOpened()) {
        throw PartitionFileNotOpenedException{"partition file is not opened"};
    }

    output << Text::hline(61) << std::endl;

    output
        << Text::justifyR("index", 10) << "|"
        << Text::justifyR("uid", 10) << "|"
        << Text::justifyR("d/f", 3) << "|"
        << Text::justifyR("name", 12) << "|"
        << Text::justifyR("size", 10) << "|"
        << Text::justifyR("order", 5) << "|"
        << Text::justifyR("count", 5) << std::endl;

    output << Text::hline(61) << std::endl;

    for (int i = 0; i < partition.GetMftItemCount(); i++) {
        MftItem mftItem = partition.ReadMftItem(i);
        mft_item &item = mftItem.item;

        if (!printAll && item.uid == UID_ITEM_FREE) {
            continue;
        }

        output
            << Text::justifyR(std::to_string(i), 10) << "|"
            << Text::justifyR(std::to_string(item.uid), 10) << "|"
            << Text::justifyR(item.is_directory ? "D" : "F", 3) << "|"
            << Text::justifyR(std::string{item.name}, 12) << "|"
            << Text::justifyR(std::to_string(item.size), 10) << "|"
            << Text::justifyR(std::to_string(item.order), 5) << "|"
            << Text::justifyR(std::to_string(item.count), 5) << std::endl;

        output << Text::hline(61) << std::endl;
    }
}

// done
void NtfsChecker::PrintBitmap(std::ostream &output)
{
    Partition &partition = m_ntfs.m_partition;

    if (!partition.IsOpened()) {
        throw PartitionFileNotOpenedException{"partition file is not opened"};
    }

    output << Text::hline(61) << std::endl;

    output << "    _|";
    for (int i = 0; i < 10; i++) {
        output << Text::justifyR(std::to_string(i), 1) << " ";
    }
    output << std::endl;

    for (int i = 0; i < partition.GetClusterCount();) {
        output << Text::justifyR(std::to_string(i), 5) << " ";

        for (int j = 0; j < 10 && i < partition.GetClusterCount(); j++, i++) {
            output << partition.ReadBitmapBit(i) << " ";
        }
        output << std::endl;
    }

    output << Text::hline(61) << std::endl;
}

// done
bool NtfsChecker::CheckBootRecord(std::ostream &output)
{
    boot_record bootRecord = m_ntfs.m_partition.GetBootRecord();

    // ---- check partition size ----

    auto &file = m_ntfs.m_partition.m_file;

    // compute the partition file size
    file.seekg(0, std::ios::beg);
    file.ignore(std::numeric_limits<std::streamsize>::max());
    std::streamsize size = file.gcount();
    file.clear();
    file.seekg(0, std::ios_base::beg);

    if (size != bootRecord.partition_size) {
        output <<
            "WARNING: the size stated in the boot record doesn't correspond with the actual partition size"
            << std::endl;
        return false;
    }

    // ---- check mft size to fit mft items ----
    int32_t mftSize = bootRecord.bitmap_start_address - bootRecord.mft_start_address;
    if (mftSize % sizeof(mft_item) != 0) {
        output <<
               "WARNING: the mft size isn't divisible by the mft item size"
               << std::endl;
        return false;
    }


    // ---- check cluster size and cluster count against the data segment size and bitmap ----
    int32_t bitmapSize = bootRecord.data_start_address - bootRecord.bitmap_start_address;
    int32_t dataSegmentSize = bootRecord.partition_size - bootRecord.data_start_address;

    auto expectedBytes = static_cast<int32_t>(std::ceil(bootRecord.cluster_count / 8.0));
    if (expectedBytes != bitmapSize) {
        output <<
               "WARNING: the bitmap size doesn't correspond with the cluster count"
               << std::endl;
        return false;
    }

    auto expectedSize = bootRecord.cluster_count * bootRecord.cluster_size;
    if (expectedSize != dataSegmentSize) {
        output <<
               "WARNING: the data segment size doesn't correspond with the cluster count"
               << std::endl;
        return false;
    }

    return true;
}

// done
bool NtfsChecker::CheckNodeSizes(std::ostream &output)
{
    NodeSizeChecker checker{m_ntfs, output};

    return checker.Run(4);
}
