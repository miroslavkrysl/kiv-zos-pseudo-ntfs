cmake_minimum_required(VERSION 3.13)
project(ntfs)

find_package (Threads REQUIRED)

add_compile_options(-g)

set(CMAKE_CXX_STANDARD 14)

add_executable(ntfs
        main.cpp

        NtfsStructs.h
        Text.cpp Text.h
        Exceptions/AppException.h
        Exceptions/PartitionExceptions.h
        Exceptions/NodeExceptions.h
        Exceptions/NodeManagerExceptions.h
        Exceptions/NtfsExceptions.h
        Exceptions/ShellExceptions.h

        Ntfs.cpp Ntfs.h
        NodeManager.cpp NodeManager.h
        Node.cpp Node.h
        Partition.cpp Partition.h

        NtfsChecker.cpp NtfsChecker.h
        NodeSizeChecker.cpp NodeSizeChecker.h

        Shell.cpp Shell.h
        DirectoryTreeChecker.cpp DirectoryTreeChecker.h
        )

TARGET_LINK_LIBRARIES(ntfs pthread)


