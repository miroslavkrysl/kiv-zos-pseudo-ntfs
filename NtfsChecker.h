#pragma once


#include <fstream>

#include "NtfsStructs.h"
#include "Ntfs.h"

/**
 * Class NtfsChecker contains useful logic for checking
 * ntfs partition integrity and various functions for validation.
 */
class NtfsChecker
{
public:
    explicit NtfsChecker(Ntfs &ntfs);

    /**
     * Print the boot record to the given output stream.
     * @param output The output stream.
     */
    void PrintBootRecord(std::ostream &output);

    /**
     * Print the mft to the given output stream.
     * @param output The output stream.
     */
    void PrintMft(std::ostream &output);

    /**
     * Print the bitmap to the given output stream.
     * @param output The output stream.
     */
    void PrintBitmap(std::ostream &output);

private:

    Ntfs &ntfs;
};


