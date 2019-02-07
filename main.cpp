#include <iostream>
#include <sstream>

#include "Ntfs.h"
#include "NtfsChecker.h"
#include "NodeManager.h"
#include "Shell.h"

void print_usage() {
    std::cout << "Usage: ntfs <partition_file_name>" << std::endl;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        print_usage();
        return 0;
    }

    try {

        Ntfs ntfs{argv[1]};

        Shell shell{ntfs};
        shell.Run();
    }
    catch (std::exception &exception){
        std::cout << exception.what() << std::endl;
    }

    return 0;
}