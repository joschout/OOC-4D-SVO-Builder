#ifndef TRIPFILEHANDLER_H
#define TRIPFILEHANDLER_H
#include "../../src/svo_builder/ExtendedTriPartitioningInfo.h"

// Trip header handling and error checking
void readTripHeader(string& filename, TripInfo4D& trip_info, bool verbose);

int parseTrip4DHeader(const std::string &filename, TripInfo4D &t);

 void writeTrip4DHeader(const std::string &filename, const TripInfo4D &t);
#endif

