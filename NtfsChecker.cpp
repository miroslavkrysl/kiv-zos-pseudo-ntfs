#include "NtfsChecker.h"
#include "Text.h"

NtfsChecker::NtfsChecker(Ntfs &ntfs)
    : ntfs(ntfs)
{}

void NtfsChecker::PrintBootRecord(std::ostream &output)
{
    Partition &partition = ntfs.GetPartition();

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

void NtfsChecker::PrintMft(std::ostream &output)
{
    Partition &partition = ntfs.GetPartition();

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

void NtfsChecker::PrintBitmap(std::ostream &output)
{
    Partition &partition = ntfs.GetPartition();

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
