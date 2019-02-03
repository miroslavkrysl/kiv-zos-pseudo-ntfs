#pragma once

#include <stdexcept>

#include "AppException.h"

class PartitionException : public AppException
{
    using AppException::AppException;
};

class PartitionFileNotOpenedException : public PartitionException
{
    using PartitionException::PartitionException;
};

class PartitionCorruptedException : public PartitionException
{
    using PartitionException::PartitionException;
};

class PartitionFormatException : public PartitionException
{
    using PartitionException::PartitionException;
};

class PartitionOutOfBoundsException : public PartitionException
{
    using PartitionException::PartitionException;
};

class PartitionMftOutOfBoundsException : public PartitionException
{
    using PartitionException::PartitionException;
};

class PartitionBitmapOutOfBoundsException : public PartitionException
{
    using PartitionException::PartitionException;
};

class PartitionClusterOverflowException : public PartitionException
{
    using PartitionException::PartitionException;
};

class PartitionDataOutOfBoundsException : public PartitionException
{
    using PartitionException::PartitionException;
};
