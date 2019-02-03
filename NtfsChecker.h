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

    void PrintBootRecord(std::ostream &output);
    void PrintMft(std::ostream &output);
    void PrintBitmap(std::ostream &output);
private:
    Ntfs &ntfs;
};


