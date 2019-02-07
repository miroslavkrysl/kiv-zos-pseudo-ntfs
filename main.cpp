#include <iostream>
#include <sstream>

#include "Ntfs.h"
#include "NtfsChecker.h"
#include "NodeManager.h"
#include "Shell.h"

/**
 * Prints the usage.
 */
void print_usage() {
    std::cout << "Usage: ntfs <partition_file_name>" << std::endl;
}

/**
 * The main function of the program.
 * Initializes the Ntfs and the Shell and runs it.
 *
 * @param argc The number of program arguments.
 * @param argv The array of program arguments.
 *
 * @return 0 on success, non 0 on fail.
 */
int main(int argc, char **argv)
{
    if (argc < 2) {
        print_usage();
        return 0;
    }

    try {

        Ntfs ntfs{argv[1]};

        Shell shell{ntfs, std::cin, std::cout};
        shell.Run();
    }
    catch (std::exception &exception){
        std::cout << exception.what() << std::endl;
    }

    return 0;
}