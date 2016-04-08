#ifndef ALTERNATEPARTITIONER_H
#define ALTERNATEPARTITIONER_H
#include "Buffer4D.h"
#include "TransformationHandler.h"
class TriPartitioningInfo4D;
class TriInfo4D;
struct TriInfo;

class alternatePartitioner
{
public:

	const size_t gridsize_S;
	const size_t gridsize_T;
	const size_t nbOfDimensions;
	size_t nbOfPartitions;

	alternatePartitioner();
	alternatePartitioner(size_t gridsize_S, size_t gridsize_T, size_t nbOfDimensions);
	~alternatePartitioner();

	TriPartitioningInfo4D partitionTriangleModel(TriInfo4D &extended_tri_info, size_t voxel_memory_limit, TransformationHandler *transformation_handler);
	static void removeTripFiles(const TriPartitioningInfo4D &trip_info);
	
private:
	int input_buffersize = 8192;
	int output_buffersize = 8192;

	size_t estimateNumberOfPartitions(const size_t memory_limit);
	TriPartitioningInfo4D createTripInfoHeader(const TriInfo4D tri_info, vector<Buffer4D*> &buffers) const;
	void deleteBuffers(vector<Buffer4D*> buffers) const;
	TriPartitioningInfo4D partition(const TriInfo4D& tri_info, TransformationHandler* transformation_handler);
	TriPartitioningInfo4D partition_one(const TriInfo4D& tri_info);
	AABox<vec4> alternatePartitioner::calculateBBoxInWorldCoordsForPartition(int i, uint64_t morton_part, float unitlength, float unitlength_time, bool verbose) const;
	Buffer4D* createBufferForPartition(int i, AABox<vec4> &bbox_partition_i_worldCoords, const string base_filename) const;
	void createBuffers(const TriInfo4D& tri_info, vector<Buffer4D*> &buffers) const;
};

#endif 