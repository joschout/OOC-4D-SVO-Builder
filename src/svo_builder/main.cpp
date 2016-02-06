#include <TriMesh.h>
#include <vector>
#include <string>
#include <sstream>
#include "globals.h"
#include "../libs/libtri/include/trip_tools.h"
#include "../libs/libtri/include/TriReader.h"
#include <algorithm>

#include "voxelizer.h"
#include "OctreeBuilder.h"
#include "partitioner.h"
#include "ExtendedTriInfo.h"
#include "alternatePartitioner.h"
#include "ExtendedTriPartitioningInfo.h"
#include "PrintUtils.h"
using namespace std;

enum ColorType { COLOR_FROM_MODEL, COLOR_FIXED, COLOR_LINEAR, COLOR_NORMAL };

// Program version
string version = "1.5";

// number of dimensions of the grid
size_t nbOfDimensions = 3;

// Program parameters
string filename = "";
// vozel grid size, should be a power of 2
// amount of voxels in the grid = pow(gridsize, nbOfGridDimensions)
size_t gridsize = 1024;
//the amount of system memory in MEGABYTES we maximally want to use during the voxelization proces
size_t voxel_memory_limit = 2048;
float sparseness_limit = 0.10f;
ColorType color = COLOR_FROM_MODEL;
vec3 fixed_color = vec3(1.0f, 1.0f, 1.0f); // fixed color is white
bool generate_levels = false;
bool verbose = false;

// trip header info
TriInfo tri_info; //struct to contain all info read from a .tri file header
TripInfo trip_info;

// buffer_size
size_t input_buffersize = 8192;


// Parse command-line params and so some basic error checking on them
void parseProgramParameters(int argc, char* argv[]) {
	string color_s = "Color from model (fallback to fixed color if model has no color)";
	cout << "Reading program parameters ..." << endl;
	// Input argument validation
	if (argc < 3) {
		printInvalid();
		exit(0);
	}
	for (int i = 1; i < argc; i++) {
		// parse filename
		if (string(argv[i]) == "-f") {
			filename = argv[i + 1];
			size_t check_tri = filename.find(".tri");
			if (check_tri == string::npos) {
				cout << "Data filename does not end in .tri - I only support that file format" << endl;
				printInvalid();
				exit(0);
			}
			i++;
		}
		else if (string(argv[i]) == "-s") {
			gridsize = atoi(argv[i + 1]);
			if (!isPowerOf2((unsigned int)gridsize)) {
				cout << "Requested gridsize is not a power of 2" << endl;
				printInvalid();
				exit(0);
			}
			i++;
		}
		else if (string(argv[i]) == "-l") {
			voxel_memory_limit = atoi(argv[i + 1]);
			if (voxel_memory_limit <= 1) {
				cout << "Requested memory limit is nonsensical. Use a value >= 1" << endl;
				printInvalid();
				exit(0);
			}
			i++;
		}
		else if (string(argv[i]) == "-d") {
			int percent = atoi(argv[i + 1]);
			sparseness_limit = percent / 100.0f;
			if (sparseness_limit < 0) {
				cout << "Requested data memory limit is nonsensical. Use a value > 0" << endl;
				printInvalid();
				exit(0);
			}
			i++;
		}
		else if (string(argv[i]) == "-v") {
			verbose = true;
		}
		else if (string(argv[i]) == "-levels") {
			generate_levels = true;
		}
		else if (string(argv[i]) == "-c") {
			string color_input = string(argv[i + 1]);
#ifdef BINARY_VOXELIZATION
			cout << "You asked to generate colors, but we're only doing binary voxelisation." << endl;
#else
			if (color_input == "model") {
				color = COLOR_FROM_MODEL;
			}
			else if (color_input == "linear") {
				color = COLOR_LINEAR;
				color_s = "Linear";
			}
			else if (color_input == "normal") {
				color = COLOR_NORMAL;
				color_s = "Normal";
			}
			else if (color_input == "fixed") {
				color = COLOR_FIXED;
				color_s = "Fixed";
			}
			else {
				cout << "Unrecognized color switch: " << color_input << ", so reverting to colors from model." << endl;
			}
#endif
			i++;
		}
		else if (string(argv[i]) == "-h") {
			printHelp(); exit(0);
		}
		else {
			printInvalid(); exit(0);
		}
	}
	if (verbose) {
		cout << "  filename: " << filename << endl;
		cout << "  gridsize: " << gridsize << endl;
		cout << "  memory limit: " << voxel_memory_limit << endl;
		cout << "  sparseness optimization limit: " << sparseness_limit << " resulting in " << (sparseness_limit*voxel_memory_limit) << " memory limit." << endl;
		cout << "  color type: " << color_s << endl;
		cout << "  generate levels: " << generate_levels << endl;
		cout << "  verbosity: " << verbose << endl;
	}
}

// Tri header handling and error checking
void readTriHeader(string& filename, TriInfo& tri_info) {
	cout << "Parsing tri header " << filename << " ..." << endl;
	if (ExtendedTriInfo::parseTri3DHeader(filename, tri_info) != 1) {
		exit(0); // something went wrong in parsing the header - exiting.
	}
	// disabled for benchmarking
	if (!tri_info.filesExist()) {
		cout << "Not all required .tri or .tridata files exist. Please regenerate using tri_convert." << endl;
		exit(0); // not all required files exist - exiting.
	}
	if (verbose) { tri_info.print(); }
	// Check if the user is using the correct executable for type of tri file
#ifdef BINARY_VOXELIZATION
	if (!tri_info.geometry_only) {
		cout << "You're using a .tri file which contains more than just geometry with a geometry-only SVO Builder! Regenerate that .tri file using tri_convert_binary." << endl;
		exit(0);
	}
#else
	if (tri_info.geometry_only) {
		cout << "You're using a .tri file which contains only geometry with the regular SVO Builder! Regenerate that .tri file using tri_convert." << endl;
		exit(0);
	}
#endif
}

// Trip header handling and error checking
void readTripHeader(string& filename, TripInfo4D& trip_info) {
	if (TripInfo4D::parseTrip4DHeader(filename, trip_info) != 1) {
		exit(0);
	}
	if (!trip_info.filesExist()) {
		cout << "Not all required .trip or .tripdata files exist. Please regenerate using svo_builder." << endl;
		exit(0); // not all required files exist - exiting.
	}
	if (verbose) { trip_info.print(); }
}

TripInfo4D partitionTriangleModel(ExtendedTriInfo &extended_tri_info){
	

/*	//estimate the amount of triangle partitions needed for voxelization
	//NOTE: the estimation is hardcoded for 3D trees
	size_t nbOfTrianglePartitions = estimate_partitions(gridsize, voxel_memory_limit, nbOfDimensions);
	cout << "Partitioning data into " << nbOfTrianglePartitions << " partitions ... "; cout.flush();

	//partition the  triangle mesh
	TripInfo trianglePartition_info = partition(tri_info, nbOfTrianglePartitions, gridsize);
	cout << "done." << endl;

	return trianglePartition_info;*/

	alternatePartitioner partitioner = alternatePartitioner(gridsize, nbOfDimensions);
	

	//estimate the amount of triangle partitions needed for voxelization
	//NOTE: the estimation is hardcoded for 3D trees
	size_t nbOfTrianglePartitions = partitioner.estimateNumberOfPartitions(voxel_memory_limit);
	cout << "Partitioning data into " << nbOfTrianglePartitions << " partitions ... "; cout.flush();

	//partition the  triangle mesh
	TripInfo4D trianglePartition_info = partitioner.partition(extended_tri_info);
	cout << "done." << endl;

	return trianglePartition_info;
}

 void voxelizeAndBuildSVO(TripInfo& trianglePartition_info	 )
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

int main(int argc, char *argv[]) {


#if defined(_WIN32) || defined(_WIN64)
	_setmaxstdio(1024); // increase file descriptor limit in Windows
#endif

	vec3 translation_direction = vec3(1, 1, 1);
	translation_direction = normalize(translation_direction);
	
	auto end_time = 10;


	// Parse program parameters
	printInfo(version);
	parseProgramParameters(argc, argv);

/*	FILE * pFile;

	errno_t errorCode = fopen_s(&pFile, "sphere.tri", "w");
	if (errorCode == 0)
	{
		std::cout << "file can be opened correctly" << std::endl;
		fclose(pFile);
	}
	else {
		std::cout << "Opening your file went wrong inside your main" << std::endl;
	}*/


	// Read the .tri file containing the triangle info
	readTriHeader(filename, tri_info);

	ExtendedTriInfo extended_tri_info = ExtendedTriInfo(tri_info, translation_direction, end_time);
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
	TripInfo4D trianglePartition_info = partitionTriangleModel(extended_tri_info);
	
	// Parse TRIP header
	string tripheader = trianglePartition_info.base_filename + string(".trip");
	readTripHeader(tripheader, trianglePartition_info);
	
	//voxelizeAndBuildSVO(trianglePartition_info);

}
