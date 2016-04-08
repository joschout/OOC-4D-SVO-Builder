#ifndef TRIPFILEHANDLER_H
#define TRIPFILEHANDLER_H
#include "TriPartitioningInfo4D.h"

// Trip header handling and error checking
void readTripHeader(string& filename, TriPartitioningInfo4D& trip_info, bool verbose);

int parseTrip4DHeader(const std::string &filename, TriPartitioningInfo4D &t);

 void writeTrip4DHeader(const std::string &filename, const TriPartitioningInfo4D &t);
#endif

