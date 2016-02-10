#ifndef VOXELIZATIONHANDLER_H
#define VOXELIZATIONHANDLER_H

#include "ExtendedTriPartitioningInfo.h"

void voxelizeAndBuildSVO4D(TripInfo4D& trianglePartition_info,
	size_t nbOfDimensions, bool generate_levels,
	size_t input_buffersize, float sparseness_limit);

#endif
