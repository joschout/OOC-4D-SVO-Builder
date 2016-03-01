#ifndef ALTERNATEPARTITIONER_H
#define ALTERNATEPARTITIONER_H
#include "Buffer4D.h"
#include "TransformationHandler.h"
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

	TripInfo4D partitionTriangleModel(TriInfo4D &extended_tri_info, size_t voxel_memory_limit, TransformationHandler *transformation_handler);
	static void removeTripFiles(const TripInfo4D &trip_info);
	
private:
	int input_buffersize = 8192;
	int output_buffersize = 8192;

	size_t estimateNumberOfPartitions(const size_t memory_limit);
	TripInfo4D createTripInfoHeader(const TriInfo4D tri_info, vector<Buffer4D*> &buffers) const;
	void deleteBuffers(vector<Buffer4D*> buffers) const;
	TripInfo4D partition(const TriInfo4D& tri_info, TransformationHandler* transformation_handler);
	TripInfo4D partition_one(const TriInfo4D& tri_info);
	AABox<vec4> alternatePartitioner::calculateBBoxInWorldCoordsForPartition(int i, uint64_t morton_part, float unitlength, float unitlength_time, bool verbose) const;
	Buffer4D* createBufferForPartition(int i, AABox<vec4> &bbox_partition_i_worldCoords, const string base_filename) const;
	void createBuffers(const TriInfo4D& tri_info, vector<Buffer4D*> &buffers) const;
};

#endif 