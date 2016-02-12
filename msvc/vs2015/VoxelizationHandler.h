#ifndef VOXELIZATIONHANDLER_H
#define VOXELIZATIONHANDLER_H
#include <trip_tools.h>
#include "../../src/svo_builder/ExtendedTriPartitioningInfo.h"
void voxelizeAndBuildSVO(
	TripInfo& trianglePartition_info, float sparseness_limit,
	bool generate_levels, size_t input_buffersize);

void voxelizeAndBuildSVO4D(
	TripInfo4D& trianglePartition_info, size_t nbOfDimensions,
	float sparseness_limit, bool generate_levels,
	size_t input_buffersize);
#endif
