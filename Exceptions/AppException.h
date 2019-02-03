#pragma once

#include <stdexcept>

class AppException : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};
