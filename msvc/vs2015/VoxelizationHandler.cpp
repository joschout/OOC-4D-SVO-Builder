#include "VoxelizationHandler.h"
#include <algorithm>
#include "../../src/svo_builder/alternatePartitioner.h"
#include "../../src/svo_builder/globals.h"
#include "Tri4DReader.h"
#include "../../src/svo_builder/voxelizer.h"
#include "../../src/svo_builder/PartitionVoxelizer.h"

using std::cout;

/*void voxelizeAndBuildSVO(
	TripInfo& trianglePartition_info, float sparseness_limit,
	bool generate_levels, size_t input_buffersize)
{
	// General voxelization calculations (stuff we need throughout voxelization process)
	float unitlength
		= (trianglePartition_info.mesh_bbox.max[0] - trianglePartition_info.mesh_bbox.min[0]) / (float)trianglePartition_info.gridsize;

	//morton_part = amount of voxels per partion
	// = amount of voxels in the grid/number of partitions in the grid
	uint64_t morton_part
		= pow(trianglePartition_info.gridsize, 3) / trianglePartition_info.n_partitions;

	char* voxels = new char[(size_t)morton_part]; // Storage for voxel on/off
#ifdef BINARY_VOXELIZATION
	vector<uint64_t> data; // Dynamic storage for morton codes
#else
	vector<VoxelData> data; // Dynamic storage for voxel data
#endif 
	size_t nfilled = 0;

	// create Octreebuilder which will output our SVO
	OctreeBuilder builder = OctreeBuilder(trianglePartition_info.base_filename, trianglePartition_info.gridsize, generate_levels);


	/*====================
	*= SVO CONSTRUCTION =
	*====================#1#

	// Start voxelisation and SVO building per partition
	for (size_t i = 0; i < trianglePartition_info.n_partitions; i++) {
		if (trianglePartition_info.part_tricounts[i] == 0) { continue; } // skip partition if it contains no triangles

																		 // VOXELIZATION

		cout << "Voxelizing partition " << i << " ..." << endl;
		// morton codes for this partition
		uint64_t morton_startcode = i * morton_part;
		uint64_t morton_endcode = (i + 1) * morton_part;
		// open file to read triangles

		string part_data_filename = trianglePartition_info.base_filename + string("_") + val_to_string(i) + string(".tripdata");
		TriReader reader = TriReader(part_data_filename, trianglePartition_info.part_tricounts[i], min(trianglePartition_info.part_tricounts[i], input_buffersize));
		if (verbose) { cout << "  reading " << trianglePartition_info.part_tricounts[i] << " triangles from " << part_data_filename << endl; }

		// voxelize partition
		size_t nfilled_before = nfilled;
		bool use_data = true;
		voxelize_schwarz_method(reader, morton_startcode, morton_endcode, unitlength, voxels, data, sparseness_limit, use_data, nfilled);
		if (verbose) { cout << "  found " << nfilled - nfilled_before << " new voxels." << endl; }

		// build SVO
		cout << "Building SVO for partition " << i << " ..." << endl;

#ifdef BINARY_VOXELIZATION
		if (use_data) { // use array of morton codes to build the SVO
			sort(data.begin(), data.end()); // sort morton codes
			for (vector<uint64_t>::iterator it = data.begin(); it != data.end(); ++it) {
				builder.addVoxel(*it);
			}
		}
		else { // morton array overflowed : using slower way to build SVO
			uint64_t morton_number;
			for (size_t j = 0; j < morton_part; j++) {
				if (!voxels[j] == EMPTY_VOXEL) {
					morton_number = morton_startcode + j;
					builder.addVoxel(morton_number);
				}
			}
		}
#else
		sort(data.begin(), data.end()); // sort
		for (std::vector<VoxelData>::iterator it = data.begin(); it != data.end(); ++it) {
			if (color == COLOR_FIXED) {
				it->color = fixed_color;
			}
			else if (color == COLOR_LINEAR) { // linear color scale
				it->color = mortonToRGB(it->morton, gridsize);
			}
			else if (color == COLOR_NORMAL) { // color models using their normals
				vec3 normal = normalize(it->normal);
				it->color = vec3((normal[0] + 1.0f) / 2.0f, (normal[1] + 1.0f) / 2.0f, (normal[2] + 1.0f) / 2.0f);
			}
			builder.addVoxel(*it);
		}
#endif

	}

	builder.finalizeTree(); // finalize SVO so it gets written to disk
	cout << "done" << endl;
	cout << "Total amount of voxels: " << nfilled << endl;

	// Removing .trip files which are left by partitioner
	removeTripFiles(trianglePartition_info);

}*/

void VoxelizationHandler::voxelizePartition(
	int i, uint64_t morton_startcode, uint64_t morton_endcode)
{
	cout << "Voxelizing partition " << i << " ..." << endl;

	// open file to read triangles
	string part_data_filename
		= trianglePartition_info.base_filename + string("_")
		+ val_to_string(i) + string(".tripdata");
	Tri4DReader reader = Tri4DReader(part_data_filename, trianglePartition_info.nbOfTrianglesPerPartition[i], min(trianglePartition_info.nbOfTrianglesPerPartition[i], input_buffersize));
	if (verbose)
	{
		cout << "  morton start_code: " << morton_startcode << endl;
		cout << "  morton end_code: " << morton_endcode << endl;
		cout << "  reading " << trianglePartition_info.nbOfTrianglesPerPartition[i] << " triangles from " << part_data_filename << endl;
	}

	// voxelize partition
	size_t nfilled_before = nfilled;

	PartitionVoxelizer partition_voxelizer = PartitionVoxelizer(morton_startcode, morton_endcode, unitlength, unitlength_time, voxels, &data, sparseness_limit, &use_data, &nfilled);
	if(binvox)
	{
		partition_voxelizer.binvox_handler = &binvox_handler;
	}
	
	partition_voxelizer.voxelize_schwarz_method4D(reader);
	//voxelize_schwarz_method4D(reader, morton_startcode, morton_endcode, unitlength, unitlength_time, voxels, data, sparseness_limit, use_data, nfilled);
	if (verbose)
	{
		cout << "  found " << nfilled - nfilled_before << " new voxels." << endl;
	}

}

void VoxelizationHandler::buildSVO_partition(int i, uint64_t morton_part, uint64_t morton_startcode)
{
	cout << "Building SVO for partition " << i << " ..." << endl;

#ifdef BINARY_VOXELIZATION
	if (use_data) { // use array of morton codes to build the SVO
		sort(data.begin(), data.end()); // sort morton codes
		for (vector<uint64_t>::const_iterator it = data.begin(); it != data.end(); ++it) {
			builder.addVoxel(*it);
		}
	}
	else { // morton array overflowed : using slower way to build SVO
		uint64_t morton_number;
		for (size_t j = 0; j < morton_part; j++) {
			if (!voxels[j] == EMPTY_VOXEL) {
				morton_number = morton_startcode + j;
				builder.addVoxel(morton_number);
			/*	if (binvox)
				{
					int x, y, z, t;
					morton4D_Decode_for(morton_number, x, y, z, t);
					binvox_handler.writeVoxel(t, x, y, z);
				}*/
			}
		}
	}
#else
	sort(data.begin(), data.end()); // sort
	for (std::vector<VoxelData>::iterator it = data.begin(); it != data.end(); ++it) {
		if (color == COLOR_FIXED) {
			it->color = fixed_color;
		}
		else if (color == COLOR_LINEAR) { // linear color scale
			it->color = mortonToRGB(it->morton, gridsize);
		}
		else if (color == COLOR_NORMAL) { // color models using their normals
			vec3 normal = normalize(it->normal);
			it->color = vec3((normal[0] + 1.0f) / 2.0f, (normal[1] + 1.0f) / 2.0f, (normal[2] + 1.0f) / 2.0f);
		}
		builder.addVoxel(*it);
	}
#endif

}

void VoxelizationHandler::voxelizeAndBuildSVO4D()
{
	if (verbose) {
		cout << "=====================" << endl;
	}
	// For each partition: Voxelize and build SVO

	if(binvox)
	{
		binvox_handler = BinvoxHandler(trianglePartition_info.base_filename, trianglePartition_info.gridsize_T);
		binvox_handler.initialize(vec3(0.0f, 0.0f, 0.0f), 1.0);
		binvox_handler.createInitialBinvoxFiles();	
	}
	
	for (size_t i = 0; i < trianglePartition_info.n_partitions; i++) {

		//IF the partition contains no triangles THEN skip it
		if (trianglePartition_info.nbOfTrianglesPerPartition[i] == 0)
		{
			continue;
		} 

		// VOXELIZATION
		// morton codes for this partition
		uint64_t morton_startcode = i * morton_part; //reminder: morton_part = #voxels/partition
		uint64_t morton_endcode = (i + 1) * morton_part;
		voxelizePartition(i, morton_startcode, morton_endcode);
		
		if(verbose){
			cout << "---------------------" << endl;
		}
		// build SVO
		buildSVO_partition(i, morton_part, morton_startcode);
		if (verbose){
			cout << "=====================" << endl;
		}
	}

	if(binvox)
	{
		binvox_handler.sparsifyFiles();
		binvox_handler.closeWriters();
	}

	builder.finalizeTree(); // finalize SVO so it gets written to disk
	cout << "done" << endl;
	cout << "Total amount of voxels: " << nfilled << endl;

	// Removing .trip files which are left by partitioner
	alternatePartitioner::removeTripFiles(trianglePartition_info);

}

VoxelizationHandler::VoxelizationHandler(): nbOfDimensions(4), sparseness_limit(0.10f), generate_levels(false), input_buffersize(8192), use_data(false), unitlength(1), unitlength_time(1), morton_part(8), voxels(nullptr), nfilled(0), builder(Tree4DBuilder())
{
}

VoxelizationHandler::VoxelizationHandler(TripInfo4D& trianglePartition_info, size_t nb_of_dimensions, float sparseness_limit, bool generate_levels, size_t input_buffersize)
	: nbOfDimensions(nb_of_dimensions),
    sparseness_limit(sparseness_limit),
    generate_levels(generate_levels),
    input_buffersize(input_buffersize), trianglePartition_info(trianglePartition_info)
{
	unitlength
		= (trianglePartition_info.mesh_bbox_transl.max[0] - trianglePartition_info.mesh_bbox_transl.min[0]) / (float)trianglePartition_info.gridsize_S;

	unitlength_time
		= (trianglePartition_info.mesh_bbox_transl.max[3] - trianglePartition_info.mesh_bbox_transl.min[3]) / (float)trianglePartition_info.gridsize_T;

	//morton_part = number of voxels per partition
	// = (amount of voxels in the grid) / (number of partitions in the grid)
	morton_part
		= (pow(trianglePartition_info.gridsize_S, 3) * trianglePartition_info.gridsize_T) / trianglePartition_info.n_partitions;

	voxels = new char[static_cast<size_t>(morton_part)];

	 nfilled = 0;

	// create Octreebuilder which will output our SVO
	builder
		= Tree4DBuilder(
			trianglePartition_info.base_filename,
			trianglePartition_info.gridsize_S,
			trianglePartition_info.gridsize_T,
			generate_levels);

	use_data = true;
}
