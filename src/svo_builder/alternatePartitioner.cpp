#include "alternatePartitioner.h"
#include <iostream>
#include <TriReader.h>
#include "TriInfo4D.h"
#include "Buffer4D.h"
#include "globals.h"
#include "morton4D.h"
#include "TriPartitioningInfo4D.h"
#include <memory>
#include "PrintUtils.h"
#include "PrintStatusBar.h"

using namespace trimesh;
alternatePartitioner::alternatePartitioner():
	gridsize_S(1), gridsize_T(1), nbOfDimensions(4), nbOfPartitions(1)
{
}

alternatePartitioner::alternatePartitioner(size_t gridsize_S, size_t gridsize_T, size_t nbOfDimensions):
	gridsize_S(gridsize_S), gridsize_T(gridsize_T), nbOfDimensions(nbOfDimensions), nbOfPartitions(1)
{
}

alternatePartitioner::~alternatePartitioner()
{
}

TriPartitioningInfo4D alternatePartitioner::partitionTriangleModel(TriInfo4D& extended_tri_info, size_t voxel_memory_limit, TransformationHandler *transformation_handler)
{
	//estimate the amount of triangle partitions needed for voxelization
	//NOTE: the estimation is hardcoded for 3D trees
	size_t nbOfTrianglePartitions = estimateNumberOfPartitions(voxel_memory_limit);
	cout << "Partitioning data into " << nbOfTrianglePartitions << " partitions ... "; cout.flush();

	//partition the  triangle mesh
	TriPartitioningInfo4D trianglePartition_info = partition(extended_tri_info, transformation_handler);
	cout << "done." << endl;

	return trianglePartition_info;
}

TriPartitioningInfo4D alternatePartitioner::partitionTriangleModel_multiple_files(TriInfo4D_multiple_files& extended_tri_info, size_t voxel_memory_limit)
{
	//estimate the amount of triangle partitions needed for voxelization
	//NOTE: the estimation is hardcoded for 3D trees
	size_t nbOfTrianglePartitions = estimateNumberOfPartitions(voxel_memory_limit);
	cout << "Partitioning data into " << nbOfTrianglePartitions << " partitions ... "; cout.flush();

	//partition the  triangle mesh
	TriPartitioningInfo4D trianglePartition_info = partition_multiple_files(extended_tri_info);
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

	/*
	NOTE 
		gridsize_S  = 2^x
		gridsize_T = 2^y

	=> nbOfVoxelsInGrid = 2^(3x) * 2^y = 2^(3x+y)
	*/


	size_t nbOfVoxelsInGrid = pow(gridsize_S, 3) * gridsize_T;
	uint64_t requiredMemoryInMB = (nbOfVoxelsInGrid*sizeof(char)) / 1024 / 1024;
	std::cout << "  to do this in-core I would need " << requiredMemoryInMB << " Mb of system memory" << std::endl;
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
	
	nbOfPartitions = estimatedNbOfPartitions;
	return estimatedNbOfPartitions;
}

TriPartitioningInfo4D alternatePartitioner::createTriPartitioningInfoHeader(const TriInfo4D tri_info, vector<Buffer4D*> &buffers) const
{
	TriPartitioningInfo4D trip_info = TriPartitioningInfo4D(tri_info);

	auto nbOfTriangles_incl_transf = gridsize_T * tri_info.triInfo3D.n_triangles;
	trip_info.n_triangles = nbOfTriangles_incl_transf;

	// Collect nbOfTriangles of each partition
	trip_info.nbOfTrianglesPerPartition.resize(nbOfPartitions);
	for (size_t j = 0; j < nbOfPartitions; j++) {
		trip_info.nbOfTrianglesPerPartition[j] = buffers[j]->n_triangles;
	}

	// Write trip header
	trip_info.base_filename 
		= tri_info.triInfo3D.base_filename 
		+ string("_S") + val_to_string(gridsize_S)
		+ string("_T") + val_to_string(gridsize_T)
		+ string("_P") + val_to_string(nbOfPartitions);
	std::string header = trip_info.base_filename + string(".trip");
	trip_info.gridsize_S = gridsize_S;
	trip_info.gridsize_T = gridsize_T;
	trip_info.n_partitions = nbOfPartitions;
	TriPartitioningInfo4D::writeTrip4DHeader(header, trip_info);

	return trip_info;
}

TriPartitioningInfo4D alternatePartitioner::createTriPartitioningInfoHeader_multiple_files(const TriInfo4D_multiple_files& tri_info, vector<Buffer4D*>& buffers) const
{
	TriPartitioningInfo4D trip_info = TriPartitioningInfo4D(tri_info);

	// Collect nbOfTriangles of each partition
	trip_info.nbOfTrianglesPerPartition.resize(nbOfPartitions);
	for (size_t j = 0; j < nbOfPartitions; j++) {
		trip_info.nbOfTrianglesPerPartition[j] = buffers[j]->n_triangles;
	}

	// Write trip header
	trip_info.base_filename
		= tri_info.base_filename_without_number
		+ string("_S") + val_to_string(gridsize_S)
		+ string("_T") + val_to_string(gridsize_T)
		+ string("_P") + val_to_string(nbOfPartitions);
	std::string header = trip_info.base_filename + string(".trip");
	trip_info.gridsize_S = gridsize_S;
	trip_info.gridsize_T = gridsize_T;
	trip_info.n_partitions = nbOfPartitions;
	TriPartitioningInfo4D::writeTrip4DHeader(header, trip_info);

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
TriPartitioningInfo4D alternatePartitioner::partition(const TriInfo4D& tri_info, TransformationHandler* transformation_handler) {
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

	TriPartitioningInfo4D trip_info = createTriPartitioningInfoHeader(tri_info, buffers);
	deleteBuffers(buffers);

	return trip_info;
}

// Partition the mesh referenced by tri_info into n triangle partitions for gridsize,
// and store information about the partitioning in trip_info
TriPartitioningInfo4D alternatePartitioner::partition_multiple_files(const TriInfo4D_multiple_files& tri_info) {


	// MAKE ONE TRIDATAREADER PER TRIDATA FILE --> one triReader per moment in time;

	vector<unique_ptr<TriReader>> tridataReaders_;


//	vector<TriReader> tridataReaders;
//	tridataReaders.reserve(tri_info.getTriInfoVector().size());


	for(const TriInfo& tri_info_3d: tri_info.getTriInfoVector())
	{
//		cout << "tri_info_3d.base_filename: " << tri_info_3d.base_filename << endl;

		// Open tri_data stream
		// the reader knows how many triangles there are in the model,
		// the reader is given input_buffersize as buffersize for buffering read triangles
		const std::string tridata_filename = tri_info_3d.base_filename + string(".tridata");
		
		std::unique_ptr<TriReader>  tridataReader_prt(new TriReader(tridata_filename, tri_info_3d.n_triangles, input_buffersize));
		tridataReaders_.push_back(std::move(tridataReader_prt));
//		TriReader tridataReader = TriReader(tridata_filename, tri_info_3d.n_triangles, input_buffersize);
//		tridataReaders.push_back(tridataReader);

//		cout << "size of tridataReaders vector" << tridataReaders_.size() << endl;
	}

//	if(tridataReaders_.size() == tri_info.getTriInfoVector().size())
//	{
//		cout << "size of tridataReaders vector is correct " << endl;
//	}else
//	{
//		cout << "size of tridataReaders vector is not correct " << endl;
//	}

	// Create Mortonbuffers: we will have one buffer per partition
	vector<Buffer4D*> buffers;
	createBuffers_multiple_files(tri_info, buffers);

	//for each moment in time = for each triReader
	float& start_time = tri_info.getTotalBoundingBox().max[3];
	float& end_time = tri_info.getTotalBoundingBox().min[3];
	float unitlength_time = (start_time - end_time) / (float)gridsize_T;
	float time = 0;
	size_t max_amount = tri_info.getNbfOfTriangles();
	size_t current_amount_of_triangles_partitioned = 0;

	cout << "Partitioning triangles: " << endl;
	for (int reader_index = 0; reader_index < tridataReaders_.size(); reader_index++)
	{
		


		

		//TriReader& tridataReader = tridataReaders_.at(reader_index);
		//for each triangle
		while (tridataReaders_.at(reader_index)->hasNext()) {
			Triangle tri;
			tridataReaders_.at(reader_index)->getTriangle(tri);
			Triangle4D tri_4d = Triangle4D(tri, time);
			storeTriangleInPartitionBuffers(tri_4d, buffers, nbOfPartitions);
			current_amount_of_triangles_partitioned++;
			if(verbose)
			{
				showProgressBar(current_amount_of_triangles_partitioned, max_amount);
			}
		}

		time += unitlength_time;
		

//		cout << "reader_index: " << reader_index << endl;
	}

	TriPartitioningInfo4D trip_info = createTriPartitioningInfoHeader_multiple_files(tri_info, buffers);
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
	float unitlength = (tri_info.mesh_bbox_transformed.max[0] - tri_info.mesh_bbox_transformed.min[0]) / (float)(gridsize_S );
	float unitlength_time = (tri_info.mesh_bbox_transformed.max[3] - tri_info.mesh_bbox_transformed.min[3]) / (float)(gridsize_T);

	float nbOfVoxelsInGrid = pow(gridsize_S, 3) * gridsize_T;
	uint64_t morton_part = nbOfVoxelsInGrid / nbOfPartitions; //amount of voxels per partition


	if(verbose)
	{
		cout << endl << "Creating " << nbOfPartitions << " buffers, one for each partition"<< endl;
		cout << "-----------------------------" << endl;
	}


	// for each partition
	for (size_t i = 0; i < nbOfPartitions; i++) {
	
		//calculate the bounding box of the partition in world coordinates
		AABox<vec4> bbox_partition_i_worldCoords = calculateBBoxInWorldCoordsForPartition(i, morton_part, unitlength, unitlength_time, verbose);
		
		// create buffer for partition
		Buffer4D* buffer_partition_i = createBufferForPartition(i, bbox_partition_i_worldCoords, tri_info.triInfo3D.base_filename);
		buffers.push_back(buffer_partition_i);

		if (verbose)
		{
			cout << endl << "Creating buffer " << i+1 << "/" << nbOfPartitions << endl;
			cout << buffer_partition_i->toString() << endl;
			cout << "-----------------------------" << endl;
		}

	}
}

void alternatePartitioner::createBuffers_multiple_files(const TriInfo4D_multiple_files& tri_info, vector<Buffer4D*>& buffers) const
{
	buffers.reserve(nbOfPartitions);

	// the unit length in the grid = the length of one side of the grid
	// divided by the size of the grid (i.e. the number of voxels next to each other)
	// NOTE: the bounding box in a .tri file is cubical
	float unitlength = (tri_info.getTotalBoundingBox().max[0] - tri_info.getTotalBoundingBox().min[0]) / (float)(gridsize_S);
	float unitlength_time = (tri_info.getTotalBoundingBox().max[3] - tri_info.getTotalBoundingBox().min[3]) / (float)(gridsize_T);

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
		Buffer4D* buffer_partition_i = createBufferForPartition(i, bbox_partition_i_worldCoords, tri_info.base_filename_without_number);
		buffers.push_back(buffer_partition_i);

		if (verbose)
		{
			cout << endl << "Creating buffer " << i + 1 << "/" << nbOfPartitions << endl;
			cout << buffer_partition_i->toString() << endl;
			cout << "-----------------------------" << endl;
		}

	}
}

void alternatePartitioner::storeTriangleInPartitionBuffers(Triangle4D transformed_tri, vector<Buffer4D*>& buffers, const size_t nbOfPartitions) const
{
	AABox<vec4> bbox4D_transformed_tri = computeBoundingBox(transformed_tri);

	bool triangleIsInAPartition = false;

	for (auto j = 0; j < nbOfPartitions; j++) { // Test against all partitions
		bool triangleIsInThisPartition = buffers[j]->processTriangle(transformed_tri, bbox4D_transformed_tri);
		triangleIsInAPartition =
			triangleIsInAPartition || triangleIsInThisPartition;
	}

	if (!triangleIsInAPartition)
	{
		cout << "ERROR: a transformed triangle is not a part of any partition!" << endl;
		std::cout << "Press ENTER to continue...";
		cin.get();
	}
}

// Remove the temporary .trip files we made
void alternatePartitioner::removeTripFiles(const TriPartitioningInfo4D& trip_info)
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

Buffer4D* alternatePartitioner::createBufferForPartition(int i, AABox<vec4> &bbox_partition_i_worldCoords, const string base_filename) const
{
	std::string filename 
		= base_filename 
		+ string("_S") + val_to_string(gridsize_S) 
		+ string("_T") + val_to_string(gridsize_T)
		+ string("_P") + val_to_string(nbOfPartitions)
		+ string("_") + val_to_string(i) + string(".tripdata");
	return new Buffer4D(filename, bbox_partition_i_worldCoords, output_buffersize);

}

// Handle the special case of just needing one partition
TriPartitioningInfo4D alternatePartitioner::partition_one(const TriInfo4D& tri_info)
{
	// Just copy files
	string src = tri_info.triInfo3D.base_filename + string(".tridata");
	string dst
		= tri_info.triInfo3D.base_filename + val_to_string(gridsize_S)
		+ string("_") + val_to_string(1)
		+ string("_") + val_to_string(0) + string(".tripdata");
	copy_file(src, dst);

	// Write header
	TriPartitioningInfo4D trip_info = TriPartitioningInfo4D(tri_info);
	trip_info.nbOfTrianglesPerPartition.resize(1);
	trip_info.nbOfTrianglesPerPartition[0] = tri_info.triInfo3D.n_triangles;
	trip_info.base_filename = tri_info.triInfo3D.base_filename + val_to_string(gridsize_S) + string("_") + val_to_string(1);
	std::string header = trip_info.base_filename + string(".trip");
	trip_info.gridsize_S = gridsize_S;
	trip_info.gridsize_T = gridsize_T;
	trip_info.n_partitions = 1;
	
	TriPartitioningInfo4D::writeTrip4DHeader(header, trip_info);
	
	return trip_info;

}
