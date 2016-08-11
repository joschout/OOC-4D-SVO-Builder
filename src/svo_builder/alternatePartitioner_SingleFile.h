#ifndef ALTERNATEPARTITIONER_SINGLEFILE_H
#define ALTERNATEPARTITIONER_SINGLEFILE_H

#include "alternatePartitioner_Interface.h"

class alternatePartitioner_SingleFile : public alternatePartitioner_Interface
{
public:
	alternatePartitioner_SingleFile();
	alternatePartitioner_SingleFile(size_t gridsize_S, size_t gridsize_T, size_t nbOfDimensions);

	TriPartitioningInfo4D partitionTriangleModel(TriInfo4D &extended_tri_info, size_t voxel_memory_limit, TransformationHandler *transformation_handler);

private:
	TriPartitioningInfo4D partition(const TriInfo4D& tri_info, TransformationHandler* transformation_handler);

	TriPartitioningInfo4D createTriPartitioningInfoHeader(const TriInfo4D tri_info, vector<Buffer4D*> &buffers) const;
};
#endif
