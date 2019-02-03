#pragma once

#include "AppException.h"

class NodeManagerException : public AppException
{
    using AppException::AppException;
};

class NodeManagerNotEnoughFreeClustersException : public NodeManagerException
{
    using NodeManagerException::NodeManagerException;
};

class NodeManagerNotEnoughFreeMftItemsException : public NodeManagerException
{
    using NodeManagerException::NodeManagerException;
};

class NodeManagerNodeNotFoundException : public NodeManagerException
{
    using NodeManagerException::NodeManagerException;
};
