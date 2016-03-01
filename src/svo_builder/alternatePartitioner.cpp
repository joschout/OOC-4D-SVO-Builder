#include "alternatePartitioner.h"
#include <iostream>
#include <TriReader.h>
#include "TriInfo4D.h"
#include "Buffer4D.h"
#include "voxelizer.h"
#include "morton4D.h"
#include "ExtendedTriPartitioningInfo.h"


alternatePartitioner::alternatePartitioner():
	gridsize(1), nbOfDimensions(4), nbOfPartitions(1)
{
}

alternatePartitioner::alternatePartitioner(size_t gridsize, size_t nbOfDimensions):
	gridsize(gridsize), nbOfDimensions(nbOfDimensions), nbOfPartitions(1)
{
}

alternatePartitioner::~alternatePartitioner()
{
}

TripInfo4D alternatePartitioner::partitionTriangleModel(TriInfo4D& extended_tri_info, size_t voxel_memory_limit, TransformationHandler *transformation_handler)
{
	//estimate the amount of triangle partitions needed for voxelization
	//NOTE: the estimation is hardcoded for 3D trees
	size_t nbOfTrianglePartitions = estimateNumberOfPartitions(voxel_memory_limit);
	cout << "Partitioning data into " << nbOfTrianglePartitions << " partitions ... "; cout.flush();

	//partition the  triangle mesh
	TripInfo4D trianglePartition_info = partition(extended_tri_info, transformation_handler);
	cout << "done." << endl;

	return trianglePartition_info;
}

/*
*/
size_t alternatePartitioner::estimateNumberOfPartitions(const size_t memory_limit_in_MB) 
{
	std::cout << "Estimating best partition count ..." << std::endl;

	// calculate the amount of memory needed (in MB) to do the voxelization completely in memory
	/* amount of memory needed
	= amount of voxels in the high-resolution 4D grid
	* the size of a character (smallest unit we can use, used to tick of a voxel in the table)
	* 1/1024 (1kB/1024bytes)
	* 1/1024 (1MB/kB)
	*/

	size_t nbOfVoxelsInGrid = pow(gridsize, nbOfDimensions);
	uint64_t requiredMemoryInMB = (nbOfVoxelsInGrid*sizeof(char)) / 1024 / 1024;
	std::cout << "  to do this in-core I would need " << requiredMemoryInMB << " Mb of system memory" << std::endl;
	if (requiredMemoryInMB <= memory_limit_in_MB) {
		std::cout << "  memory limit of " << memory_limit_in_MB << " Mb allows that" << std::endl;
		return 1;
	}

	size_t estimatedNbOfPartitions = 1;
	size_t sizeOfPartitionInMB = requiredMemoryInMB;
	auto partitioning_amount = pow(2, nbOfDimensions); // 8
	while (sizeOfPartitionInMB > memory_limit_in_MB) {
		sizeOfPartitionInMB = sizeOfPartitionInMB / partitioning_amount;
		estimatedNbOfPartitions = estimatedNbOfPartitions * partitioning_amount;
	}
	std::cout << "  going to do it in " << estimatedNbOfPartitions << " partitions of " << sizeOfPartitionInMB << " Mb each." << std::endl;
	
	nbOfPartitions = estimatedNbOfPartitions;
	return estimatedNbOfPartitions;
}

TripInfo4D alternatePartitioner::createTripInfoHeader(const TriInfo4D tri_info, vector<Buffer4D*> &buffers) const
{
	TripInfo4D trip_info = TripInfo4D(tri_info);

	auto nbOfTriangles_incl_transf = gridsize * tri_info.triInfo3D.n_triangles;
	trip_info.n_triangles = nbOfTriangles_incl_transf;

	// Collect nbOfTriangles of each partition
	trip_info.nbOfTrianglesPerPartition.resize(nbOfPartitions);
	for (size_t j = 0; j < nbOfPartitions; j++) {
		trip_info.nbOfTrianglesPerPartition[j] = buffers[j]->n_triangles;
	}

	// Write trip header
	trip_info.base_filename = tri_info.triInfo3D.base_filename + val_to_string(gridsize) + string("_") + val_to_string(nbOfPartitions);
	std::string header = trip_info.base_filename + string(".trip");
	trip_info.gridsize = gridsize;
	trip_info.n_partitions = nbOfPartitions;
	TripInfo4D::writeTrip4DHeader(header, trip_info);

	return trip_info;
}

void alternatePartitioner::deleteBuffers(vector<Buffer4D*> buffers) const
{
	for (size_t j = 0; j < nbOfPartitions; j++) {
		delete buffers[j];
	}
}

// Partition the mesh referenced by tri_info into n triangle partitions for gridsize,
// and store information about the partitioning in trip_info
TripInfo4D alternatePartitioner::partition(const TriInfo4D& tri_info, TransformationHandler *transformation_handler) {
	// Special case: just one partition
	//if (nbOfPartitions == 1) {
	//	return partition_one(tri_info, gridsize);
	//}

	// Open tri_data stream
	// the reader knows how many triangles there are in the model,
	// the reader is given input_buffersize as buffersize for buffering read triangles
	const std::string tridata_filename = tri_info.triInfo3D.base_filename + string(".tridata");
	TriReader tridataReader = TriReader(tridata_filename, tri_info.triInfo3D.n_triangles, input_buffersize);
	
	// Create Mortonbuffers: we will have one buffer per partition
	vector<Buffer4D*> buffers;
	createBuffers(tri_info, buffers);

	//for each triangle
	while (tridataReader.hasNext()) {
		Triangle tri;
		tridataReader.getTriangle(tri);
		//transform the triangle to the different points in time and store those triangles in the buffers
		transformation_handler->transformAndStore(tri_info, tri, buffers, nbOfPartitions);
	}

	TripInfo4D trip_info = createTripInfoHeader(tri_info, buffers);
	deleteBuffers(buffers);

	return trip_info;
}


// Create as many Buffers as there are partitions for a total gridsize,
// store them in the given vector,
// use tri_info for filename information
void alternatePartitioner::createBuffers(const TriInfo4D& tri_info, vector<Buffer4D*> &buffers) const
{
	buffers.reserve(nbOfPartitions);

	// the unit length in the grid = the length of one side of the grid
	// divided by the size of the grid (i.e. the number of voxels next to each other)
	// NOTE: the bounding box in a .tri file is cubical
	float unitlength = (tri_info.mesh_bbox_transformed.max[0] - tri_info.mesh_bbox_transformed.min[0]) / (float)(gridsize );
	float unitlength_time = (tri_info.mesh_bbox_transformed.max[3] - tri_info.mesh_bbox_transformed.min[3]) / (float)(gridsize);

	float nbOfVoxelsInGrid = pow(gridsize, nbOfDimensions);
	uint64_t morton_part = nbOfVoxelsInGrid / nbOfPartitions; //amount of voxels per partition

	cout << endl << "creating buffers" << endl;
	// for each partition
	for (size_t i = 0; i < nbOfPartitions; i++) {

		//calculate the bounding box of the partition in world coordinates
		AABox<vec4> bbox_partition_i_worldCoords = calculateBBoxInWorldCoordsForPartition(i, morton_part, unitlength, unitlength_time, verbose);
		
		// create buffer for partition
		Buffer4D* buffer_partition_i = createBufferForPartition(i, bbox_partition_i_worldCoords, tri_info.triInfo3D.base_filename);
		buffers.push_back(buffer_partition_i);
	}
}

// Remove the temporary .trip files we made
void alternatePartitioner::removeTripFiles(const TripInfo4D& trip_info)
{
	// remove header file
	string filename = trip_info.base_filename + string(".trip");
	remove(filename.c_str());
	// remove tripdata files
	for (size_t i = 0; i < trip_info.n_partitions; i++) {
		filename = trip_info.base_filename + string("_") + val_to_string(i) + string(".tripdata");
		remove(filename.c_str());
	}
}

AABox<vec4> alternatePartitioner::calculateBBoxInWorldCoordsForPartition(int i, uint64_t morton_part, float unitlength, float unitlength_time, bool verbose) const
{
	AABox<uivec4> bbox_partition_i_gridCoords;
	AABox<vec4> bbox_partition_i_worldCoords;
	//Each partition contains morton_part voxels
	// ==> partition i contains the voxels with as morton numbers
	//			[morton_part*i, morton_part*(i+1) -1]

	// compute world bounding box
	/*		morton4D_Decode_for(morton_part*i, bbox_grid.min[3], bbox_grid.min[2], bbox_grid.min[1], bbox_grid.min[0]);
	morton4D_Decode_for((morton_part*(i + 1)) - 1, bbox_grid.max[3],  bbox_grid.max[2], bbox_grid.max[1], bbox_grid.max[0]);*/
	morton4D_Decode_for(morton_part*i, bbox_partition_i_gridCoords.min[0], bbox_partition_i_gridCoords.min[1], bbox_partition_i_gridCoords.min[2], bbox_partition_i_gridCoords.min[3]);
	morton4D_Decode_for((morton_part*(i + 1)) - 1, bbox_partition_i_gridCoords.max[0], bbox_partition_i_gridCoords.max[1], bbox_partition_i_gridCoords.max[2], bbox_partition_i_gridCoords.max[3]);

	// -1, because z-curve skips to first block of next partition

	bbox_partition_i_worldCoords.min[0] = bbox_partition_i_gridCoords.min[0] * unitlength;
	bbox_partition_i_worldCoords.min[1] = bbox_partition_i_gridCoords.min[1] * unitlength;
	bbox_partition_i_worldCoords.min[2] = bbox_partition_i_gridCoords.min[2] * unitlength;
	bbox_partition_i_worldCoords.min[3] = bbox_partition_i_gridCoords.min[3] * unitlength_time;

	bbox_partition_i_worldCoords.max[0] = (bbox_partition_i_gridCoords.max[0] + 1)*unitlength; // + 1, to include full last block
	bbox_partition_i_worldCoords.max[1] = (bbox_partition_i_gridCoords.max[1] + 1)*unitlength;
	bbox_partition_i_worldCoords.max[2] = (bbox_partition_i_gridCoords.max[2] + 1)*unitlength;
	bbox_partition_i_worldCoords.max[3] = (bbox_partition_i_gridCoords.max[2] + 1)*unitlength_time;


	// output partition info
	if (verbose) {
		cout << "Partitioning partition #" << i + 1 << " / " << nbOfPartitions << " id: " << i << " ..." << endl;
		cout << "  morton from " << morton_part*i << " to " << morton_part*(i + 1) << endl;
		cout << "  grid coordinates from ("
			<< bbox_partition_i_gridCoords.min[0] << "," << bbox_partition_i_gridCoords.min[1] << "," << bbox_partition_i_gridCoords.min[2] << "," << bbox_partition_i_gridCoords.min[3]
			<< ") to ("
			<< bbox_partition_i_gridCoords.max[0] << "," << bbox_partition_i_gridCoords.max[1] << "," << bbox_partition_i_gridCoords.max[2] << "," << bbox_partition_i_gridCoords.max[3] << ")" << endl;
		cout << "  worldspace coordinates from ("
			<< bbox_partition_i_worldCoords.min[0] << "," << bbox_partition_i_worldCoords.min[1] << "," << bbox_partition_i_worldCoords.min[2] << "," << bbox_partition_i_worldCoords.min[3]
			<< ") to ("
			<< bbox_partition_i_worldCoords.max[0] << "," << bbox_partition_i_worldCoords.max[1] << "," << bbox_partition_i_worldCoords.max[2] << "," << bbox_partition_i_worldCoords.max[3] << ")" << endl << endl;
	}

	return bbox_partition_i_worldCoords;

}

Buffer4D* alternatePartitioner::createBufferForPartition(int i, AABox<vec4> &bbox_partition_i_worldCoords, const string base_filename) const
{
	std::string filename 
		= base_filename + val_to_string(gridsize) 
		+ string("_") + val_to_string(nbOfPartitions)
		+ string("_") + val_to_string(i) + string(".tripdata");
	return new Buffer4D(filename, bbox_partition_i_worldCoords, output_buffersize);

}

// Handle the special case of just needing one partition
TripInfo4D alternatePartitioner::partition_one(const TriInfo4D& tri_info)
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
