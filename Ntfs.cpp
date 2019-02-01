#include <utility>

#include "Ntfs.h"

Ntfs::Ntfs(std::string partitionPath)
    : m_partition{std::move(partitionPath)}
{}

NtfsPartition &Ntfs::GetPartition()
{
    return m_partition;
}