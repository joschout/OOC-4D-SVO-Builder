#pragma once
#include "Buffer4D.h"
class TripInfo4D;
class TriInfo4D;
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

	TripInfo4D partitionTriangleModel(TriInfo4D &extended_tri_info, size_t voxel_memory_limit);
	static void removeTripFiles(const TripInfo4D &trip_info);
	
private:
	int input_buffersize = 8192;
	int output_buffersize = 8192;

	size_t estimateNumberOfPartitions(const size_t memory_limit);
	TripInfo4D partition(const TriInfo4D& tri_info);
	TripInfo4D partition_one(const TriInfo4D& tri_info);
	void createBuffers(const TriInfo4D& tri_info, vector<Buffer4D*> &buffers) const;
};

