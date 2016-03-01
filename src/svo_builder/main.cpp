#include <TriMesh.h>
#include "globals.h"
#include "../libs/libtri/include/trip_tools.h"
#include "TriInfo4D.h"
#include "alternatePartitioner.h"
#include "ExtendedTriPartitioningInfo.h"
#include "PrintUtils.h"
#include "../../msvc/vs2015/TriHeaderFileReader.h"
#include "../../msvc/vs2015/TripFileHandler.h"
#include "../../msvc/vs2015/InputParameterParser.h"
#include "../../msvc/vs2015/VoxelizationHandler.h"
#include "TranslationHandler.h"
#include "RotationHandler.h"
using namespace std;

enum ColorType { COLOR_FROM_MODEL, COLOR_FIXED, COLOR_LINEAR, COLOR_NORMAL };

// Program version
string version = "1.5";

// number of dimensions of the grid
size_t nbOfDimensions = 4;
auto end_time = 1;

// Program parameters
string filename = "";
// vozel grid size, should be a power of 2
// amount of voxels in the grid = pow(gridsize, nbOfGridDimensions)
size_t gridsize = 16;
//the amount of system memory in MEGABYTES we maximally want to use during the voxelization proces
size_t voxel_memory_limit = 2048;
float sparseness_limit = 0.10f;
ColorType color = COLOR_FROM_MODEL;
vec3 fixed_color = vec3(1.0f, 1.0f, 1.0f); // fixed color is white
bool generate_levels = false;
bool verbose = false;

// trip header info
//TripInfo trip_info;

// buffer_size
size_t input_buffersize = 8192;


int main(int argc, char *argv[]) {


#if defined(_WIN32) || defined(_WIN64)
	_setmaxstdio(1024); // increase file descriptor limit in Windows
#endif

	// Parse program parameters
	printInfo(version);
	parseProgramParameters(argc, argv, filename, gridsize, voxel_memory_limit, sparseness_limit, verbose, generate_levels );

	// Read the .tri file containing the triangle info
	// struct to contain all info read from a .tri file header
	TriInfo triangleInfo3D = readTriHeader(filename, verbose);
	vec3 translation_direction = vec3(1, 1, 1);
	translation_direction = normalize(translation_direction);
	
	TriInfo4D triangleInfo4D = TriInfo4D(triangleInfo3D, end_time);

//	RotationHandler transformation_handler = RotationHandler(gridsize, X_AXIS);
	TranslationHandler transformation_handler = TranslationHandler(gridsize, translation_direction);
	transformation_handler.calculateTransformedBoundingBox(triangleInfo3D, triangleInfo4D.mesh_bbox_transformed, end_time);
	//tri info now contains the info from the tri header file
	//NOTE: THIS DOES NOT INCLUDE THE INFO OF EACH INDIVIDUAL TRIANGLE


	/* VOXELIZATION
	Consumes the input triangle mesh in a streaming manner.
	Produces the intermediate high-resolution voxel grid in morton order
	
	Subprocess 1: PARTITIONING
	Partitions the triangle mesh according to subgrids in a streaming matter.
	--> test each triangle against the bounding box of each subgrid
	Store each triangle mesh partition temporarily on disk
	*/
	alternatePartitioner partitioner = alternatePartitioner(gridsize, nbOfDimensions);
	TripInfo4D trianglePartition_info = partitioner.partitionTriangleModel(triangleInfo4D, voxel_memory_limit, &transformation_handler);
	

	// Parse TRIP header
	string tripheader = trianglePartition_info.base_filename + string(".trip");
	readTripHeader(tripheader, trianglePartition_info, verbose);
	

	VoxelizationHandler voxelization_handler
		= VoxelizationHandler(trianglePartition_info, nbOfDimensions, sparseness_limit, generate_levels, input_buffersize);
	voxelization_handler.voxelizeAndBuildSVO4D();

}
