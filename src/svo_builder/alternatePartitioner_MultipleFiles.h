#ifndef ALTERNATEPARTITIONER_MULTIPLEFILES_H
#define ALTERNATEPARTITIONER_MULTIPLEFILES_H

#include "alternatePartitioner_Interface.h"

class alternatePartitioner_MultipleFiles : public alternatePartitioner_Interface
{
public: 
	alternatePartitioner_MultipleFiles();
	alternatePartitioner_MultipleFiles(size_t gridsize_S, size_t gridsize_T, size_t nbOfDimensions);

	TriPartitioningInfo4D partitionTriangleModel_multiple_files(TriInfo4D_multiple_files &extended_tri_info, size_t voxel_memory_limit);

private:
	TriPartitioningInfo4D partition_multiple_files(const TriInfo4D_multiple_files& tri_info);

	TriPartitioningInfo4D createTriPartitioningInfoHeader_multiple_files(const TriInfo4D_multiple_files& tri_info, vector<Buffer4D*> &buffers) const;
};

#endif

