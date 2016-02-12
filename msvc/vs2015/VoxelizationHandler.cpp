#include "VoxelizationHandler.h"
#include "../../src/svo_builder/partitioner.h"
#include "../../src/svo_builder/OctreeBuilder.h"
#include <algorithm>
#include "../../src/svo_builder/alternatePartitioner.h"

void voxelizeAndBuildSVO(
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
	*====================*/

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

}

void voxelizeAndBuildSVO4D(
	TripInfo4D& trianglePartition_info, size_t nbOfDimensions,
	float sparseness_limit, bool generate_levels,
	size_t input_buffersize)
{
	// General voxelization calculations (stuff we need throughout voxelization process)
	float unitlength
		= (trianglePartition_info.mesh_bbox_transl.max[0] - trianglePartition_info.mesh_bbox_transl.min[0]) / (float)trianglePartition_info.gridsize;

	//morton_part = amount of voxels per partion
	// = amount of voxels in the grid/number of partitions in the grid
	uint64_t morton_part
		= pow(trianglePartition_info.gridsize, nbOfDimensions) / trianglePartition_info.n_partitions;

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
	*====================*/

	// Start voxelisation and SVO building per partition
	for (size_t i = 0; i < trianglePartition_info.n_partitions; i++) {
		if (trianglePartition_info.nbOfTrianglesPerPartition[i] == 0) { continue; } // skip partition if it contains no triangles

																					// VOXELIZATION

		cout << "Voxelizing partition " << i << " ..." << endl;
		// morton codes for this partition
		uint64_t morton_startcode = i * morton_part;
		uint64_t morton_endcode = (i + 1) * morton_part;
		// open file to read triangles

		string part_data_filename = trianglePartition_info.base_filename + string("_") + val_to_string(i) + string(".tripdata");
		TriReader reader = TriReader(part_data_filename, trianglePartition_info.nbOfTrianglesPerPartition[i], min(trianglePartition_info.nbOfTrianglesPerPartition[i], input_buffersize));
		if (verbose) { cout << "  reading " << trianglePartition_info.nbOfTrianglesPerPartition[i] << " triangles from " << part_data_filename << endl; }

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
	alternatePartitioner::removeTripFiles(trianglePartition_info);

}
