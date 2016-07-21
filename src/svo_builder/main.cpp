#include <TriMesh.h>
#include <cstdio>
#include "globals.h"
#include "../libs/libtri/include/trip_tools.h"
#include "TriInfo4D.h"
#include "alternatePartitioner.h"
#include "TriPartitioningInfo4D.h"
#include "PrintUtils.h"
#include "TriHeaderFileReader.h"
#include "TripFileHandler.h"
#include "InputParameterParser.h"
#include "VoxelizationHandler.h"
#include "TranslationHandler.h"
#include "RotationHandler.h"
#include "TriHeaderHandler.h"
#include "ColorType.h"

//#define logVerboseToFile

using namespace std;

// Program version
string version = "1.5";

// number of dimensions of the grid
size_t nbOfDimensions = 4;
auto end_time = 1;

// Program parameters
string filename = "";
// vozel grid size, should be a power of 2
// amount of voxels in the grid = pow(gridsize, nbOfGridDimensions)
size_t gridsize_S = 16;
size_t gridsize_T = 16;
//the amount of system memory in MEGABYTES we maximally want to use during the voxelization proces
size_t voxel_memory_limit = 2048;
float sparseness_limit = 0.10f;

ColorType color_type = COLOR_FROM_MODEL;

bool generate_levels = false;
bool verbose = false;
bool binvox = false;
bool multiple_input_files;

// trip header info
//TripInfo trip_info;

// buffer_size
size_t input_buffersize = 8192;


int main(int argc, char *argv[]) {


#if defined(_WIN32) || defined(_WIN64)
	_setmaxstdio(1024); // increase file descriptor limit in Windows
#endif

#ifdef logVerboseToFile
	std::printf("stdout is printed to console\n");
	if (std::freopen("log_ooc_svo_builder.txt", "w", stdout)) {
		std::printf("stdout is redirected to a file: log_ooc_svo_builder.txt\n"); // this is written to redir.txt
		//std::fclose(stdout);
	}

#endif

	// Parse program parameters
	printInfo(version);
	parseProgramParameters(
		argc, argv, filename, 
		gridsize_S, gridsize_T,
		voxel_memory_limit, sparseness_limit, verbose, generate_levels, 
		binvox, multiple_input_files, color_type);




	if(!multiple_input_files){
		// Read the .tri file containing the triangle info
		// struct to contain all info read from a .tri file header
		TriInfo triangleInfo3D = readTriHeader(filename, verbose);
		vec3 translation_direction = vec3(1, 1, 1);
		translation_direction = normalize(translation_direction);
	
		TriInfo4D triangleInfo4D = TriInfo4D(triangleInfo3D, end_time);
	//	RotationHandler transformation_handler = RotationHandler(gridsize, X_AXIS);
		TranslationHandler transformation_handler = TranslationHandler(gridsize_T, translation_direction);
		transformation_handler.speed_factor = 1.0f;
		transformation_handler.calculateTransformedBoundingBox(triangleInfo4D, end_time);
		//tri info now contains the info from the tri header file
		//NOTE: THIS DOES NOT INCLUDE THE INFO OF EACH INDIVIDUAL TRIANGLE


		//===============================================//

		/* VOXELIZATION
		Consumes the input triangle mesh in a streaming manner.
		Produces the intermediate high-resolution voxel grid in morton order

		Subprocess 1: PARTITIONING
		Partitions the triangle mesh according to subgrids in a streaming matter.
		--> test each triangle against the bounding box of each subgrid
		Store each triangle mesh partition temporarily on disk
		*/
		alternatePartitioner partitioner = alternatePartitioner(gridsize_S, gridsize_T, nbOfDimensions);
		TriPartitioningInfo4D trianglePartition_info = partitioner.partitionTriangleModel(triangleInfo4D, voxel_memory_limit, &transformation_handler);

		// Parse TRIP header
		string tripheader = trianglePartition_info.base_filename + string(".trip");
		readTripHeader(tripheader, trianglePartition_info, verbose);

#ifdef BINARY_VOXELIZATION
		VoxelizationHandler voxelization_handler
			= VoxelizationHandler(
				trianglePartition_info, nbOfDimensions, sparseness_limit, generate_levels, input_buffersize);		
#else
		VoxelizationHandler voxelization_handler
			= VoxelizationHandler(
				trianglePartition_info, nbOfDimensions, sparseness_limit, generate_levels, input_buffersize,
				color_type, gridsize_S);
#endif
		voxelization_handler.voxelizeAndBuildSVO4D();

#ifdef logVerboseToFile
		std::fclose(stdout);
#endif


	}else
	{
		TriHeaderHandler tri_header_handler = TriHeaderHandler(filename, 0, end_time, multiple_input_files, gridsize_T, verbose);
		TriInfo4D_multiple_files triangleInfo4D = tri_header_handler.readHeaders();

//		for (const TriInfo& tri_info_3d : triangleInfo4D.getTriInfoVector())
//		{
//			tri_info_3d.print();
//		}

		alternatePartitioner partitioner = alternatePartitioner(gridsize_S, gridsize_T, nbOfDimensions);
	
		TriPartitioningInfo4D trianglePartition_info = partitioner.partitionTriangleModel_multiple_files(triangleInfo4D, voxel_memory_limit);

		// Parse TRIP header
		string tripheader = trianglePartition_info.base_filename + string(".trip");
		readTripHeader(tripheader, trianglePartition_info, verbose);

#ifdef BINARY_VOXELIZATION
		VoxelizationHandler voxelization_handler
			= VoxelizationHandler(
				trianglePartition_info, nbOfDimensions, sparseness_limit, generate_levels, input_buffersize);	
#else
		VoxelizationHandler voxelization_handler
			= VoxelizationHandler(
				trianglePartition_info, nbOfDimensions, sparseness_limit, generate_levels, input_buffersize,
				color_type, gridsize_S);
#endif
		voxelization_handler.voxelizeAndBuildSVO4D();
#ifdef logVerboseToFile
		std::fclose(stdout);
#endif

	}


}
