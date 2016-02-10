#pragma once
#include "Buffer4D.h"
class TripInfo4D;
class ExtendedTriInfo;
struct TriInfo;

class alternatePartitioner
{
public:

	const size_t gridsize;
	const size_t nbOfDimensions;
	size_t nbOfPartitions;

	alternatePartitioner();
	alternatePartitioner(size_t gridsize, size_t nbOfDimensions);
	~alternatePartitioner();

	size_t estimateNumberOfPartitions(const size_t memory_limit);
	TripInfo4D partition(const ExtendedTriInfo& tri_info);
	void createBuffers(const ExtendedTriInfo& tri_info, vector<Buffer4D*> &buffers) const;

	static void removeTripFiles(const TripInfo4D &trip_info);
	
private:
	int input_buffersize = 8192;
	int output_buffersize = 8192;

	TripInfo4D partition_one(const ExtendedTriInfo& tri_info);
};

