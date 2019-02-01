#pragma once


#include "NtfsPartition.h"

class Ntfs
{
public:
    explicit Ntfs(std::string partitionPath);

    NtfsPartition &GetPartition();
private:
    NtfsPartition m_partition;
};


