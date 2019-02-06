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

class NtfsNotAFileException : public NtfsException
{
    using NtfsException::NtfsException;
};

class NtfsNodeNameConflict : public NtfsException
{
    using NtfsException::NtfsException;
};

class NtfsRootNotFoundException : public NtfsException
{
    using NtfsException::NtfsException;
};

class NtfsNodeNotFoundException : public NtfsException
{
    using NtfsException::NtfsException;
};

class NtfsPathNotFoundException : public NtfsException
{
    using NtfsException::NtfsException;
};

class NtfsFileNotFoundException : public NtfsException
{
    using NtfsException::NtfsException;
};

class NtfsNodeAlreadyExistsException : public NtfsException
{
    using NtfsException::NtfsException;
};

class NtfsDirectoryNotEmptyException : public NtfsException
{
    using NtfsException::NtfsException;
};
