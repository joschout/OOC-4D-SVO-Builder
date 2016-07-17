#ifndef VOXELIZATIONHANDLER_H
#define VOXELIZATIONHANDLER_H
#include <trip_tools.h>
#include "TriPartitioningInfo4D.h"
#include "Tree4DBuilder.h"
#include "BinvoxHandler.h"
#include "Tree4DBuilderDifferentSides_Time_longest.h"
#include "Tree4DBuilderDifferentSides_Space_longest.h"
#include "Tree4DBuilder_Strategy.h"

class VoxelizationHandler
{
public:
	size_t nbOfDimensions;
	float sparseness_limit;
	bool generate_levels;
	size_t input_buffersize;
	TriPartitioningInfo4D trianglePartition_info;

	bool use_data;

	float unitlength;
	float unitlength_time;

	//morton_part = number of voxels per partition
	// = (amount of voxels in the grid) / (number of partitions in the grid)
	uint64_t morton_part;

	// Array to store whether a voxel is on/off
	// Length of array = number of voxels per partition
	char* voxels;

#ifdef BINARY_VOXELIZATION
	vector<uint64_t> data; // Dynamic storage for morton codes
#else
	vector<VoxelData> data; // Dynamic storage for voxel data
#endif 

	size_t nfilled;

	// create Octreebuilder which will output our SVO
	Tree4DBuilder_Strategy builder;
	//Tree4DBuilderDifferentSides_Interface builder;
//	Tree4DBuilderDifferentSides_Time_longest builder;
//	Tree4DBuilderDifferentSides_Space_longest builder;
	//Tree4DBuilder builder;

	BinvoxHandler binvox_handler;

	VoxelizationHandler();
	VoxelizationHandler(
		TriPartitioningInfo4D& trianglePartition_info, size_t nb_of_dimensions,
		float sparseness_limit, bool generate_levels,
		size_t input_buffersize);
	
	void voxelizeAndBuildSVO4D();


private:
	//static void showProgressBar(uint64_t current_morton_code, uint64_t morton_part);
	void voxelizePartition
		(int i, uint64_t morton_startcode, uint64_t morton_endcode);
	void buildSVO_partition
		(int i, uint64_t morton_part, uint64_t morton_startcode);
};


/*void voxelizeAndBuildSVO(
	TripInfo& trianglePartition_info, float sparseness_limit,
	bool generate_levels, size_t input_buffersize);*/

#endif
