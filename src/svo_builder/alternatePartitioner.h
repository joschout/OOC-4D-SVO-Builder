#ifndef ALTERNATEPARTITIONER_H
#define ALTERNATEPARTITIONER_H
#include "Buffer4D.h"
#include "TransformationHandler.h"
#include "TriInfo4D_multiple_files.h"

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
	TriPartitioningInfo4D partitionTriangleModel_multiple_files(TriInfo4D_multiple_files &extended_tri_info, size_t voxel_memory_limit);

	static void removeTripFiles(const TriPartitioningInfo4D &trip_info);
	
private:
	int input_buffersize = 8192;
	int output_buffersize = 8192;

	size_t estimateNumberOfPartitions(const size_t memory_limit);

	TriPartitioningInfo4D createTriPartitioningInfoHeader(const TriInfo4D tri_info, vector<Buffer4D*> &buffers) const;
	TriPartitioningInfo4D createTriPartitioningInfoHeader_multiple_files(const TriInfo4D_multiple_files& tri_info, vector<Buffer4D*> &buffers) const;

	TriPartitioningInfo4D partition(const TriInfo4D& tri_info, TransformationHandler* transformation_handler);
	TriPartitioningInfo4D partition_multiple_files(const TriInfo4D_multiple_files& tri_info);
	TriPartitioningInfo4D partition_one(const TriInfo4D& tri_info);

	AABox<vec4> calculateBBoxInWorldCoordsForPartition(int i, uint64_t morton_part, float unitlength, float unitlength_time, bool verbose) const;
	
	void createBuffers(const TriInfo4D& tri_info, vector<Buffer4D*> &buffers) const;
	void createBuffers_multiple_files(const TriInfo4D_multiple_files& tri_info, vector<Buffer4D*> &buffers) const;
	Buffer4D* createBufferForPartition(int i, AABox<vec4> &bbox_partition_i_worldCoords, const string base_filename) const;
	void deleteBuffers(vector<Buffer4D*> buffers) const;

	void storeTriangleInPartitionBuffers(Triangle4D transformed_tri, vector<Buffer4D*>& buffers, const size_t nbOfPartitions) const;

};

#endif 