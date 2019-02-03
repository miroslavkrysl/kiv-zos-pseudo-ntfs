#pragma once


#include "Partition.h"

class Ntfs
{
public:
    explicit Ntfs(std::string partitionPath);

    Partition &GetPartition();
private:
    Partition m_partition;
};


