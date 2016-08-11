#include "alternatePartitioner_Interface.h"
#include "morton4D.h"
#include "TriPartitioningInfo4D.h"

// Remove the temporary .trip files we made
void alternatePartitioner_Interface::removeTripFiles(const TriPartitioningInfo4D& trip_info)
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

alternatePartitioner_Interface::alternatePartitioner_Interface(size_t gridsize_S, size_t gridsize_T, size_t nbOfDimensions):
	gridsize_S(gridsize_S), gridsize_T(gridsize_T), nbOfDimensions(nbOfDimensions), nbOfPartitions(1)
{
}

/*
*/
size_t alternatePartitioner_Interface::estimateNumberOfPartitions(const size_t memory_limit_in_MB)
{
	std::cout << "Estimating best partition count ..." << std::endl;

	// calculate the amount of memory needed (in MB) to do the voxelization completely in memory
	/* amount of memory needed
	= amount of voxels in the high-resolution 4D grid
	* the size of a character (smallest unit we can use, used to tick of a voxel in the table)
	* 1/1024 (1kB/1024bytes)
	* 1/1024 (1MB/kB)
	*/

	/*
	NOTE
	gridsize_S  = 2^x
	gridsize_T = 2^y

	=> nbOfVoxelsInGrid = 2^(3x) * 2^y = 2^(3x+y)
	*/


	size_t nbOfVoxelsInGrid = pow(gridsize_S, 3) * gridsize_T;
	size_t requiredMemoryInBytes = (nbOfVoxelsInGrid*sizeof(char));
	uint64_t requiredMemoryInMB = requiredMemoryInBytes/ 1024 / 1024;
	std::cout << "  to do this in-core I would need " << requiredMemoryInMB << " Mb of system memory" << std::endl;
	if(data_out)
	{
		data_writer_ptr->writeToFile_endl("total nb of voxels in grid: " + to_string(nbOfVoxelsInGrid));
		data_writer_ptr->writeToFile_endl("memory necessary for in-core voxelization (grid size)(Bytes) : " + to_string(requiredMemoryInBytes));
		data_writer_ptr->writeToFile_endl("memory necessary for in-core voxelization (grid size)(MB) : " + to_string(requiredMemoryInMB));
	}

	if (requiredMemoryInMB <= memory_limit_in_MB) {
		std::cout << "  memory limit of " << memory_limit_in_MB << " Mb allows that" << std::endl;
		return 1;
	}

	size_t estimatedNbOfPartitions = 1;
	size_t sizeOfPartitionInMB = requiredMemoryInMB;

	// TODO this might not be correct when we have different grid sizes for space and time
	auto partitioning_amount = pow(2, nbOfDimensions); // 8
	while (sizeOfPartitionInMB > memory_limit_in_MB) {
		sizeOfPartitionInMB = sizeOfPartitionInMB / partitioning_amount;
		estimatedNbOfPartitions = estimatedNbOfPartitions * partitioning_amount;
	}
	std::cout << "  I am going to do it in " << estimatedNbOfPartitions << " partitions of " << sizeOfPartitionInMB << " Mb each." << std::endl;
	std::cout << "  This will take up " << estimatedNbOfPartitions *sizeOfPartitionInMB << " Mb in total on your disk." << std::endl;

	if(data_out)
	{
		data_writer_ptr->writeToFile_endl("estimated partition size (MB) (INCORRECT): " + to_string(sizeOfPartitionInMB));
		data_writer_ptr->writeToFile_endl("estimated disk space partioning (MB): " + to_string(estimatedNbOfPartitions *sizeOfPartitionInMB));
	}


	nbOfPartitions = estimatedNbOfPartitions;
	return estimatedNbOfPartitions;
}

/*
For partition i,
calculate the bounding box in world coordinates which contains morton_part voxels,
containing the voxels with  as morton numbers
		[morton_part*i, morton_part*(i+1) -1]
It first calculates the integer-based bounding box.
From this integer-based bounding box, it calculates the bounding box in world coourcinates.

IMPORTANT: NOTE that the total bounding box of the model gets translated,
so the min point is in the origin of the world coordinate system.

*/
AABox<vec4> alternatePartitioner_Interface::calculateBBoxInWorldCoordsForPartition(int i, uint64_t morton_part, float unitlength, float unitlength_time, bool verbose) const
{
	AABox<uivec4> bbox_partition_i_gridCoords;
	AABox<vec4> bbox_partition_i_worldCoords;
	//Each partition contains morton_part voxels
	// ==> partition i contains the voxels with as morton numbers
	//			[morton_part*i, morton_part*(i+1) -1]

	// compute world bounding box
	/*		morton4D_Decode_for(morton_part*i, bbox_grid.min[3], bbox_grid.min[2], bbox_grid.min[1], bbox_grid.min[0]);
	morton4D_Decode_for((morton_part*(i + 1)) - 1, bbox_grid.max[3],  bbox_grid.max[2], bbox_grid.max[1], bbox_grid.max[0]);*/
	morton4D_Decode_for(
		morton_part*i,
		bbox_partition_i_gridCoords.min[0], bbox_partition_i_gridCoords.min[1],
		bbox_partition_i_gridCoords.min[2], bbox_partition_i_gridCoords.min[3],
		gridsize_S, gridsize_S, gridsize_S, gridsize_T);
	morton4D_Decode_for(
		(morton_part*(i + 1)) - 1,
		bbox_partition_i_gridCoords.max[0], bbox_partition_i_gridCoords.max[1],
		bbox_partition_i_gridCoords.max[2], bbox_partition_i_gridCoords.max[3],
		gridsize_S, gridsize_S, gridsize_S, gridsize_T);

	// -1, because z-curve skips to first block of next partition

	bbox_partition_i_worldCoords.min[0] = bbox_partition_i_gridCoords.min[0] * unitlength;
	bbox_partition_i_worldCoords.min[1] = bbox_partition_i_gridCoords.min[1] * unitlength;
	bbox_partition_i_worldCoords.min[2] = bbox_partition_i_gridCoords.min[2] * unitlength;
	bbox_partition_i_worldCoords.min[3] = bbox_partition_i_gridCoords.min[3] * unitlength_time;

	bbox_partition_i_worldCoords.max[0] = (bbox_partition_i_gridCoords.max[0] + 1)*unitlength; // + 1, to include full last block
	bbox_partition_i_worldCoords.max[1] = (bbox_partition_i_gridCoords.max[1] + 1)*unitlength;
	bbox_partition_i_worldCoords.max[2] = (bbox_partition_i_gridCoords.max[2] + 1)*unitlength;
	bbox_partition_i_worldCoords.max[3] = (bbox_partition_i_gridCoords.max[3] + 1)*unitlength_time;


	// output partition info
	if (verbose) {
		cout << "Partitioning partition #" << i + 1 << " / " << nbOfPartitions << " id: " << i << " ..." << endl;
		cout << "  morton from " << morton_part*i << " to " << morton_part*(i + 1) << endl;
		cout << "  grid coordinates from ("
			<< bbox_partition_i_gridCoords.min[0] << ", " << bbox_partition_i_gridCoords.min[1] << ", " << bbox_partition_i_gridCoords.min[2] << ", " << bbox_partition_i_gridCoords.min[3]
			<< ") to ("
			<< bbox_partition_i_gridCoords.max[0] << ", " << bbox_partition_i_gridCoords.max[1] << ", " << bbox_partition_i_gridCoords.max[2] << ", " << bbox_partition_i_gridCoords.max[3] << ")" << endl;
		cout << "  worldspace coordinates from ("
			<< bbox_partition_i_worldCoords.min[0] << ", " << bbox_partition_i_worldCoords.min[1] << ", " << bbox_partition_i_worldCoords.min[2] << ", " << bbox_partition_i_worldCoords.min[3]
			<< ") to ("
			<< bbox_partition_i_worldCoords.max[0] << ", " << bbox_partition_i_worldCoords.max[1] << ", " << bbox_partition_i_worldCoords.max[2] << ", " << bbox_partition_i_worldCoords.max[3] << ")" << endl << endl;
	}

	return bbox_partition_i_worldCoords;
}

void alternatePartitioner_Interface::createBuffers(const AABox<vec4>& total_bounding_box, const string base_filename, vector<Buffer4D*>& buffers) const
{
	buffers.reserve(nbOfPartitions);

	// the unit length in the grid = the length of one side of the grid
	// divided by the size of the grid (i.e. the number of voxels next to each other)
	// NOTE: the bounding box in a .tri file is cubical
	float unitlength = (total_bounding_box.max[0] - total_bounding_box.min[0]) / (float)(gridsize_S);
	float unitlength_time = (total_bounding_box.max[3] - total_bounding_box.min[3]) / (float)(gridsize_T);

	float nbOfVoxelsInGrid = pow(gridsize_S, 3) * gridsize_T;
	uint64_t morton_part = nbOfVoxelsInGrid / nbOfPartitions; //amount of voxels per partition


	if (verbose)
	{
		cout << endl << "Creating " << nbOfPartitions << " buffers, one for each partition" << endl;
		cout << "-----------------------------" << endl;
	}


	// for each partition
	for (size_t i = 0; i < nbOfPartitions; i++) {

		//calculate the bounding box of the partition in world coordinates
		AABox<vec4> bbox_partition_i_worldCoords = calculateBBoxInWorldCoordsForPartition(i, morton_part, unitlength, unitlength_time, verbose);

		// create buffer for partition
		Buffer4D* buffer_partition_i = createBufferForPartition(i, bbox_partition_i_worldCoords, base_filename);
		buffers.push_back(buffer_partition_i);

		if (verbose)
		{
			cout << endl << "Creating buffer " << i + 1 << "/" << nbOfPartitions << endl;
			cout << buffer_partition_i->toString() << endl;
			cout << "-----------------------------" << endl;
		}

	}
}

Buffer4D* alternatePartitioner_Interface::createBufferForPartition(int i, AABox<vec4> &bbox_partition_i_worldCoords, const string base_filename) const
{
	std::string filename
		= base_filename
		+ string("_S") + val_to_string(gridsize_S)
		+ string("_T") + val_to_string(gridsize_T)
		+ string("_P") + val_to_string(nbOfPartitions)
		+ string("_") + val_to_string(i) + string(".tripdata");
	return new Buffer4D(filename, bbox_partition_i_worldCoords, output_buffersize);
}

void alternatePartitioner_Interface::deleteBuffers(vector<Buffer4D*> buffers) const
{
	for (size_t j = 0; j < nbOfPartitions; j++) {
		delete buffers[j];
	}
}

void alternatePartitioner_Interface::storeTriangleInPartitionBuffers(Triangle4D transformed_tri, vector<Buffer4D*>& buffers, const size_t nbOfPartitions) const
{
	AABox<vec4> bbox4D_transformed_tri = computeBoundingBox(transformed_tri);

	bool triangleIsInAPartition = false;

	for (auto j = 0; j < nbOfPartitions; j++) { // Test against all partitions
		bool triangleIsInThisPartition = buffers[j]->processTriangle(transformed_tri, bbox4D_transformed_tri);
//		if(!triangleIsInThisPartition)
//		{
//			cout << "  buffer bounding box: " << endl
//				<< "    min: " << buffers[j]->bbox_world.min << endl
//				<< "    max: " << buffers[j]->bbox_world.max << endl;
//
//		}

		triangleIsInAPartition =
			triangleIsInAPartition || triangleIsInThisPartition;
	}

	if (!triangleIsInAPartition)
	{
		cout << "ERROR: a transformed triangle is not a part of any partition!" << endl;
//		cout 
//			<< "  translated 4D tri: " << endl
//			<< "    v0: " << transformed_tri.tri.v0 << endl
//			<< "    v1: " << transformed_tri.tri.v1 << endl
//			<< "    v2: " << transformed_tri.tri.v2 << endl
//			<< "    time: " << transformed_tri.time << endl;



		std::cout << "Press ENTER to continue...";
		cin.get();
	}
}
