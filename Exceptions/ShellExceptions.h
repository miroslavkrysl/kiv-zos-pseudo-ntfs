#pragma once

#include "AppException.h"

class ShellException : public AppException
{
    using AppException::AppException;
};

class ShellWrongArgumentsException : public ShellException
{
    using ShellException::ShellException;
};


