#include "alternatePartitioner.h"
#include <iostream>
#include <TriReader.h>
#include "ExtendedTriInfo.h"
#include "Buffer4D.h"
#include "voxelizer.h"
#include "morton4D.h"
#include "TranslationHandler.h"
#include <trip_tools.h>
#include "ExtendedTriPartitioningInfo.h"

alternatePartitioner::alternatePartitioner():
	gridsize(0), nbOfDimensions(0), nbOfPartitions(0)
{
}

alternatePartitioner::alternatePartitioner(size_t gridsize, size_t nbOfDimensions):
	gridsize(gridsize), nbOfDimensions(nbOfDimensions), nbOfPartitions(0)
{
}

alternatePartitioner::~alternatePartitioner()
{
}

/*
*/
size_t alternatePartitioner::estimateNumberOfPartitions(const size_t memory_limit) 
{
	std::cout << "Estimating best partition count ..." << std::endl;

	// calculate the amount of memory needed (in MB) to do the partitioning completely in memory
	/* amount of memory needed
	= amount of voxels in the high-resolution 3D grid
	* the size of a character
	* 1/1024 (1kB/1024bytes)
	* 1/1024 (1MB/kB)
	*/

	size_t nbOfVoxelsInGrid = pow(gridsize, nbOfDimensions);
	uint64_t requiredMemory = (nbOfVoxelsInGrid*sizeof(char)) / 1024 / 1024;
	std::cout << "  to do this in-core I would need " << requiredMemory << " Mb of system memory" << std::endl;
	if (requiredMemory <= memory_limit) {
		std::cout << "  memory limit of " << memory_limit << " Mb allows that" << std::endl;
		return 1;
	}
	size_t estimatedNbOfPartitions = 1;
	size_t partition_size = requiredMemory;
	auto partitioning_amount = pow(2, nbOfDimensions);
	while (partition_size > memory_limit) {
		partition_size = partition_size / partitioning_amount;
		estimatedNbOfPartitions = estimatedNbOfPartitions * partitioning_amount;
	}
	std::cout << "  going to do it in " << estimatedNbOfPartitions << " partitions of " << partition_size << " Mb each." << std::endl;
	
	nbOfPartitions = estimatedNbOfPartitions;
	return estimatedNbOfPartitions;
}


// Partition the mesh referenced by tri_info into n triangle partitions for gridsize,
// and store information about the partitioning in trip_info
TripInfo4D alternatePartitioner::partition(const ExtendedTriInfo& tri_info) {
	// Special case: just one partition
	//if (nbOfPartitions == 1) {
	//	return partition_one(tri_info, gridsize);
	//}

	// Open tri_data stream
	
	//the reader knows how many triangles there are in the model,
	// is given input_buffersize as buffersize
	const std::string tridata_filename = tri_info.triInfo3D.base_filename + string(".tridata");
	TriReader reader = TriReader(tridata_filename, tri_info.triInfo3D.n_triangles, input_buffersize);
	

	// Create Mortonbuffers: we will have one buffer per partition
	vector<Buffer4D*> buffers;
	createBuffers(tri_info, buffers);

	float unitlength_time = (tri_info.mesh_bbox_transl.max[3] - tri_info.mesh_bbox_transl.min[3]) / (float)gridsize;


	while (reader.hasNext()) {
		Triangle t;
		
		reader.getTriangle(t);
		
		vec3 tv0_transl = t.v0;
		vec3 tv1_transl = t.v1;
		vec3 tv2_transl = t.v2;
		
		for (float time = 0; time <= tri_info.end_time; time = time + unitlength_time)
		{
			tv0_transl = translate(tv0_transl, tri_info.translation_direction, unitlength_time);
			tv1_transl = translate(tv1_transl, tri_info.translation_direction, unitlength_time);
			tv2_transl = translate(tv2_transl, tri_info.translation_direction, unitlength_time);
			AABox<vec4> bbox4D = computeBoundingBoxOneTimePoint(tv0_transl, tv1_transl, tv2_transl, time);
			for (int j = 0; j < nbOfPartitions; j++) { // Test against all partitions
				buffers[j]->processTriangle(t, bbox4D);
			}

		}
	}


	 // create TripInfo object to hold header info
	TripInfo4D trip_info = TripInfo4D(tri_info);

	auto nbOfTriangles_inlc_transl = gridsize * tri_info.triInfo3D.n_triangles;
	trip_info.n_triangles = nbOfTriangles_inlc_transl;

	// Collect ntriangles and close buffers
	trip_info.nbOfTrianglesPerPartition.resize(nbOfPartitions);
	for (size_t j = 0; j < nbOfPartitions; j++) {
		trip_info.nbOfTrianglesPerPartition[j] = buffers[j]->n_triangles;
		delete buffers[j];
	}

	
	// Write trip header
	trip_info.base_filename = tri_info.triInfo3D.base_filename + val_to_string(gridsize) + string("_") + val_to_string(nbOfPartitions);
	std::string header = trip_info.base_filename + string(".trip");
	trip_info.gridsize = gridsize;
	trip_info.n_partitions = nbOfPartitions;
	TripInfo4D::writeTrip4DHeader(header, trip_info);

	return trip_info;
}


// Create as many Buffers as there are partitions for a total gridsize,
// store them in the given vector,
// use tri_info for filename information
void alternatePartitioner::createBuffers(const ExtendedTriInfo& tri_info, vector<Buffer4D*> &buffers) const
{
	buffers.reserve(nbOfPartitions);

	//the unit length in the grid = the length of one side of the grid
	// divided by the size of the grid (i.e. the number of voxels next to each other)
	//NOTE: the bounding box in a .tri file is cubical
	float unitlength = (tri_info.mesh_bbox_transl.max[0] - tri_info.mesh_bbox_transl.min[0]) / (float)gridsize;
	float unitlength_time = (tri_info.mesh_bbox_transl.max[3] - tri_info.mesh_bbox_transl.min[3]) / (float)gridsize;

	uint64_t morton_part = pow(gridsize, nbOfDimensions) / nbOfPartitions;

	AABox<uivec4> bbox_grid;
	AABox<vec4> bbox_world;
	std::string filename;

	for (size_t i = 0; i < nbOfPartitions; i++) {
		// compute world bounding box
		morton4D_Decode_for(morton_part*i, bbox_grid.min[3], bbox_grid.min[2], bbox_grid.min[1], bbox_grid.min[0]);
		morton4D_Decode_for((morton_part*(i + 1)) - 1, bbox_grid.min[3],  bbox_grid.max[2], bbox_grid.max[1], bbox_grid.max[0]);
		// -1, because z-curve skips to first block of next partition
		
		bbox_world.min[0] = bbox_grid.min[0] * unitlength;
		bbox_world.min[1] = bbox_grid.min[1] * unitlength;
		bbox_world.min[2] = bbox_grid.min[2] * unitlength;
		bbox_world.min[3] = bbox_grid.min[3] * unitlength_time;

		bbox_world.max[0] = (bbox_grid.max[0] + 1)*unitlength; // + 1, to include full last block
		bbox_world.max[1] = (bbox_grid.max[1] + 1)*unitlength;
		bbox_world.max[2] = (bbox_grid.max[2] + 1)*unitlength;
		bbox_world.max[3] = (bbox_grid.max[2] + 1)*unitlength_time;


		// output partition info
		if (verbose) {
			cout << "Partitioning partition #" << i + 1 << " / " << nbOfPartitions << " id: " << i << " ..." << endl;
			cout << "  morton from " << morton_part*i << " to " << morton_part*(i + 1) << endl;
			cout << "  grid coordinates from (" << bbox_grid.min[0] << "," << bbox_grid.min[1] << "," << bbox_grid.min[2] << ") to ("
				<< bbox_grid.max[0] << "," << bbox_grid.max[1] << "," << bbox_grid.max[2] << ")" << endl;
			cout << "  worldspace coordinates from (" << bbox_world.min[0] << "," << bbox_world.min[1] << "," << bbox_world.min[2] << ") to ("
				<< bbox_world.max[0] << "," << bbox_world.max[1] << "," << bbox_world.max[2] << ")" << endl;
		}

		// create buffer for partition
		filename = tri_info.triInfo3D.base_filename + val_to_string(gridsize) + string("_") + val_to_string(nbOfPartitions) + string("_") + val_to_string(i) + string(".tripdata");
		buffers[i] = new Buffer4D(filename, bbox_world, output_buffersize);
	}
}

// Handle the special case of just needing one partition
TripInfo4D alternatePartitioner::partition_one(const ExtendedTriInfo& tri_info)
{
	// Just copy files
	string src = tri_info.triInfo3D.base_filename + string(".tridata");
	string dst
		= tri_info.triInfo3D.base_filename + val_to_string(gridsize)
		+ string("_") + val_to_string(1)
		+ string("_") + val_to_string(0) + string(".tripdata");
	copy_file(src, dst);

	// Write header
	TripInfo4D trip_info = TripInfo4D(tri_info);
	trip_info.nbOfTrianglesPerPartition.resize(1);
	trip_info.nbOfTrianglesPerPartition[0] = tri_info.triInfo3D.n_triangles;
	trip_info.base_filename = tri_info.triInfo3D.base_filename + val_to_string(gridsize) + string("_") + val_to_string(1);
	std::string header = trip_info.base_filename + string(".trip");
	trip_info.gridsize = gridsize;
	trip_info.n_partitions = 1;
	
	TripInfo4D::writeTrip4DHeader(header, trip_info);
	
	return trip_info;

}
