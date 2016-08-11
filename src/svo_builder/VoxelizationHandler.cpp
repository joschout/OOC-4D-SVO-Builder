#include "VoxelizationHandler.h"
#include <algorithm>
#include "alternatePartitioner.h"
#include "globals.h"
#include "Tri4DReader.h"
#include "voxelizer.h"
#include "PartitionVoxelizer.h"

#include "PrintStatusBar.h"
using std::cout;


/*
RETURNS the number of voxels found for this partition
*/
size_t VoxelizationHandler::voxelizePartition(
	int i, uint64_t morton_startcode, uint64_t morton_endcode)
{
	cout << "Voxelizing partition " << i << " ..." << endl;

	voxelization_io_input_timer.start();	 // TIMING
	// open file to read triangles
	string part_data_filename
		= trianglePartition_info.base_filename + string("_")
		+ val_to_string(i) + string(".tripdata");
	Tri4DReader reader = Tri4DReader(part_data_filename, trianglePartition_info.nbOfTrianglesPerPartition[i], min(trianglePartition_info.nbOfTrianglesPerPartition[i], input_buffersize));
	voxelization_io_input_timer.stop();	 // TIMING
	if (verbose)
	{
		cout << "  morton start_code: " << morton_startcode << endl;
		cout << "  morton end_code: " << morton_endcode << endl;
		cout << "  reading " << trianglePartition_info.nbOfTrianglesPerPartition[i] << " triangles from " << part_data_filename << endl;
	}

	// voxelize partition
	size_t nfilled_before = nfilled;

	voxelization_algorithm_timer.start();	// TIMING
	PartitionVoxelizer partition_voxelizer 
		= PartitionVoxelizer(
			morton_startcode, morton_endcode,
			trianglePartition_info.gridsize_S, trianglePartition_info.gridsize_T,
			unitlength, unitlength_time,
			voxels, &data, sparseness_limit, &use_data, &nfilled);
	voxelization_algorithm_timer.stop();
	if(binvox)
	{
		partition_voxelizer.binvox_handler = &binvox_handler;
	}
	
	partition_voxelizer.voxelize_schwarz_method4D(reader);
	
	size_t nbOfNewNonEmptyVoxelsFound = nfilled - nfilled_before;
	return nbOfNewNonEmptyVoxelsFound;
}

/*
reminder: morton_part = #voxels/partition
*/
void VoxelizationHandler::buildSVO_partition(int i, uint64_t morton_part, uint64_t morton_startcode)
{
	cout << "Building Sparse Voxel Tree for partition " << i << " ..." << endl;
	svo_algorithm_timer.start();	// TIMING
#ifdef BINARY_VOXELIZATION
	if (use_data) { // use array of morton codes to build the SVO
		cout << "  using array of morton codes to build the svo" << endl
			<< "  sorting the data..." << endl;
		sort(data.begin(), data.end()); // sort morton codes
		cout << "  adding voxels..." << endl;
		for (vector<uint64_t>::const_iterator it = data.begin(); it != data.end(); ++it) {
			if(verbose){
				showProgressBar((it - data.begin()), (data.size() - 1));
			}

			builder.addVoxel(*it);
		}
	}
	else { // morton array overflowed : using slower way to build SVO
		cout << "  morton array overflowed : using slower way to build SVO" << endl;
		uint64_t morton_number;
		for (size_t j = 0; j < morton_part; j++) {//for each voxel in this partition
			if (!voxels[j] == EMPTY_VOXEL) {
				morton_number = morton_startcode + j;
				if(verbose)	{
					showProgressBar(j, morton_part);
				}
				
				builder.addVoxel(morton_number);
			}
		}
	}
#else
	cout << "  sorting the data..." << endl;
	sort(data.begin(), data.end()); // sort
	cout << "  adding voxels..." << endl;
	for (std::vector<VoxelData>::iterator it = data.begin(); it != data.end(); ++it) {
		if(verbose){
			showProgressBar((it - data.begin()), (data.size() - 1));
		}
		
		if (color_type == COLOR_FIXED) {
			it->color = fixed_color;
		}
		else if (color_type == COLOR_LINEAR) { // linear color scale
			it->color = mortonToRGB(it->morton, gridsize_S);
		}
		else if (color_type == COLOR_NORMAL) { // color models using their normals
			vec3 normal = normalize(it->normal);
			it->color = vec3((normal[0] + 1.0f) / 2.0f, (normal[1] + 1.0f) / 2.0f, (normal[2] + 1.0f) / 2.0f);
		}
		builder.addVoxel(*it);
	}
#endif

	svo_algorithm_timer.stop();		// TIMING
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
		binvox_handler.initialize(vec3(0.0f, 0.0f, 0.0f), 1.0, trianglePartition_info.gridsize_S);
		binvox_handler.createInitialBinvoxFiles();	
	}
	

	//ONLY FOR DATA OUTPUT
	size_t total_nb_of_non_empty_voxels_found = 0;

	for (size_t i = 0; i < trianglePartition_info.n_partitions; i++) {

		if(data_out){
			data_writer_ptr->writeToFile_endl("partition " + to_string(i) + " - nb of triangles to voxelize: " + to_string(trianglePartition_info.nbOfTrianglesPerPartition[i]));
		}


		//IF the partition contains no triangles THEN skip it
		if (trianglePartition_info.nbOfTrianglesPerPartition[i] == 0)
		{
			continue;
		} 

		//  === VOXELIZATION for partition i=== //
		voxelization_total_timer.start();

		// morton codes for this partition
		uint64_t morton_startcode = i * morton_part; //reminder: morton_part = #voxels/partition
		uint64_t morton_endcode = (i + 1) * morton_part;
		size_t nbOfNewVoxelsfound = voxelizePartition(i, morton_startcode, morton_endcode);
		
		voxelization_total_timer.stop();
		// === END VOXELIZATION for partition i ===//


		total_nb_of_non_empty_voxels_found += nbOfNewVoxelsfound;

		if (verbose){
			cout << "  found " << nbOfNewVoxelsfound<< " new voxels." << endl;
			cout << "---------------------" << endl;
		}
		if (data_out){
			data_writer_ptr->writeToFile_endl("partition " + to_string(i) + " - nb of non-empty voxels found: " + to_string(nbOfNewVoxelsfound));
		}


		// === BUILDING SVO for partition i === //
		svo_total_timer.start();	// TIMING
		buildSVO_partition(i, morton_part, morton_startcode);
		svo_total_timer.stop();		// TIMING
		// === END BUILDING SVO for partition i === //

		if (verbose){
			cout << "=====================" << endl;
		}

//		data.clear(); 
//not wrong and should happen, but already happens in the constructor of PartitionVoxelizer,
//		of which an instance is created in size_t nbOfNewVoxelsfound = voxelizePartition(i, morton_startcode, morton_endcode);
	}



	if (data_out) {
		data_writer_ptr->writeToFile_endl("total number of non-empty voxels found: " + to_string(total_nb_of_non_empty_voxels_found));

		size_t nbOfVoxelsInGrid = pow(trianglePartition_info.gridsize_S, 3) * trianglePartition_info.gridsize_T;
		float sparseness = static_cast<float>(nbOfVoxelsInGrid - total_nb_of_non_empty_voxels_found) / static_cast<float>(nbOfVoxelsInGrid);
		data_writer_ptr->writeToFile_endl("sparseness (%): " + to_string(sparseness));
	}

	if(binvox)
	{
		binvox_handler.sparsifyFiles();
		binvox_handler.closeWriters();
	}

	svo_total_timer.start(); svo_algorithm_timer.start(); // TIMING
	builder.finalizeTree(); // finalize SVO so it gets written to disk
	svo_total_timer.stop(); svo_algorithm_timer.stop(); // TIMING

	cout << "done" << endl;
	cout << "Total amount of voxels: " << nfilled << endl;

	// Removing .trip files which are left by partitioner
	alternatePartitioner::removeTripFiles(trianglePartition_info);

}

#ifdef BINARY_VOXELIZATION
VoxelizationHandler::VoxelizationHandler(): nbOfDimensions(4), sparseness_limit(0.10f), generate_levels(false), input_buffersize(8192), use_data(false), unitlength(1), unitlength_time(1), morton_part(8), voxels(nullptr), nfilled(0), builder(Tree4DBuilder_Strategy())
#else
VoxelizationHandler::VoxelizationHandler() : nbOfDimensions(4), sparseness_limit(0.10f), generate_levels(false), input_buffersize(8192), use_data(false), unitlength(1), unitlength_time(1), morton_part(8), voxels(nullptr), color_type(COLOR_FROM_MODEL), gridsize_S(0), nfilled(0), builder(Tree4DBuilder_Strategy())
#endif
{
}

#ifdef BINARY_VOXELIZATION
VoxelizationHandler::VoxelizationHandler(TriPartitioningInfo4D& trianglePartition_info, size_t nb_of_dimensions, float sparseness_limit, bool generate_levels, size_t input_buffersize)
	: nbOfDimensions(nb_of_dimensions),
    sparseness_limit(sparseness_limit),
    generate_levels(generate_levels),
    input_buffersize(input_buffersize), trianglePartition_info(trianglePartition_info)
{
#else
VoxelizationHandler::VoxelizationHandler(
	TriPartitioningInfo4D& trianglePartition_info, size_t nb_of_dimensions,
	float sparseness_limit, bool generate_levels,
	size_t input_buffersize,
	ColorType color_type, size_t gridsize_S):
	nbOfDimensions(nb_of_dimensions),
	sparseness_limit(sparseness_limit),
	generate_levels(generate_levels),
	input_buffersize(input_buffersize), trianglePartition_info(trianglePartition_info),
	color_type(color_type), gridsize_S(gridsize_S)
{
#endif
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


	 voxelization_total_timer.stop();	// TIMING
	 svo_total_timer.start();			// TIMING

	 builder
		 = Tree4DBuilder_Strategy(
			 trianglePartition_info.base_filename,
			 trianglePartition_info.gridsize_S,
			 trianglePartition_info.gridsize_T,
			 generate_levels);
	builder.initializeBuilder();

	svo_total_timer.stop();				// TIMING
	
	use_data = true;
}
