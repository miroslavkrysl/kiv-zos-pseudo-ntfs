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
     *
     * @param output The output stream.
     */
    void PrintBootRecord(std::ostream &output);

    /**
     * Print the mft to the given output stream.
     *
     * @param output The output stream.
     * @param printAll If true, prints the free mft items too.
     */
    void PrintMft(std::ostream &output, bool printAll);

    /**
     * Print the bitmap to the given output stream.
     *
     * @param output The output stream.
     */
    void PrintBitmap(std::ostream &output);

    /**
     * Check the boot record values.
     * Checks the partition size against the actual size,
     * if the mft items fits into the mft size and
     * if the cluster size and cluster count correspond
     * with the data segment size and bitmap size.
     *
     * @param output The output to print messages.
     *
     * @return True if everything is OK, false otherwise.
     */
    bool CheckBootRecord(std::ostream &output);

    /**
     * Check every node if its size corresponds
     * with the number of clusters assigned to it.
     *
     * @param output The output to print messages.
     *
     * @return True if everything is OK, false otherwise.
     */
    bool CheckNodeSizes(std::ostream &output);

    /**
     * Check every node if it is present in exactly
     * one directory reachable from the directory tree.
     *
     * @param output The output to print messages.
     *
     * @return True if everything is OK, false otherwise.
     */
    bool CheckFileDirectories(std::ostream &output);

private:

    /**
     * The ntfs which the ntfs checker operates on.
     */
    Ntfs &m_ntfs;
};


