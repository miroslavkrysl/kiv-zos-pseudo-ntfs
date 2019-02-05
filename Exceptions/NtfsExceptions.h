#pragma once

#include "AppException.h"

class NtfsException : public AppException
{
    using AppException::AppException;
};

class NtfsNotADirectoryException : public NtfsException
{
    using NtfsException::NtfsException;
};

class NtfsNodeNameConflict : public NtfsException
{
    using NtfsException::NtfsException;
};
