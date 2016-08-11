#ifndef ALTERNATEPARTITIONERINTERFACE_H
#define ALTERNATEPARTITIONERINTERFACE_H
#include "globals.h"
#include "Buffer4D.h"
#include "TransformationHandler.h"
#include "TriInfo4D_multiple_files.h"

class TriPartitioningInfo4D;
class TriInfo4D;
struct TriInfo;

class alternatePartitioner_Interface
{
public:

	const size_t gridsize_S;
	const size_t gridsize_T;
	const size_t nbOfDimensions;
	size_t nbOfPartitions;

	virtual ~alternatePartitioner_Interface()
	{
		
	}

	static void removeTripFiles(const TriPartitioningInfo4D &trip_info);

protected:
	int input_buffersize = 8192;
	int output_buffersize = 8192;

	alternatePartitioner_Interface(size_t gridsize_S, size_t gridsize_T, size_t nbOfDimensions);

	size_t estimateNumberOfPartitions(const size_t memory_limit);

	AABox<vec4> calculateBBoxInWorldCoordsForPartition(int i, uint64_t morton_part, float unitlength, float unitlength_time, bool verbose) const;

	void createBuffers(const AABox<vec4>& total_bounding_box, const string base_filename, vector<Buffer4D*> &buffers) const;
	Buffer4D* createBufferForPartition(int i, AABox<vec4> &bbox_partition_i_worldCoords, const string base_filename) const;
	void deleteBuffers(vector<Buffer4D*> buffers) const;
	void storeTriangleInPartitionBuffers(Triangle4D transformed_tri, vector<Buffer4D*>& buffers, const size_t nbOfPartitions) const;

};

#endif
