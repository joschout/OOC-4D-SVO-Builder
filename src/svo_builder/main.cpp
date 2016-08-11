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
#include "alternatePartitioner_SingleFile.h"
#include "alternatePartitioner_MultipleFiles.h"
#include <memory>
//#define logVerboseToFile
//#define NO_CONSOLE_OUTPUT
using namespace std;

// Program version
string version = "1.5";

// number of dimensions of the grid
size_t nbOfDimensions = 4;
auto end_time = 16; //1;

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
bool data_out = false;
bool binvox = false;
bool multiple_input_files;


// timers
Timer main_timer;
Timer partitioning_total_timer;
Timer partitioning_io_input_timer;
Timer partitioning_io_output_timer;
Timer partitioning_algorithm_timer;
Timer voxelization_total_timer;
Timer voxelization_io_input_timer;
Timer voxelization_algorithm_timer;
Timer svo_total_timer;
Timer svo_io_output_timer;
Timer svo_algorithm_timer;

unique_ptr<DataWriter> data_writer_ptr;

// buffer_size
size_t input_buffersize = 8192;


// Initialize all performance timers
void setupTimers() {
	main_timer = Timer();

	// Partitioning timers
	partitioning_total_timer = Timer();
	partitioning_io_input_timer = Timer();
	partitioning_io_output_timer = Timer();
	partitioning_algorithm_timer = Timer();

	// Voxelization timers
	voxelization_total_timer = Timer();
	voxelization_io_input_timer = Timer();
	voxelization_algorithm_timer = Timer();

	svo_total_timer = Timer();
	svo_io_output_timer = Timer();
	svo_algorithm_timer = Timer();
}

// Printout total time of running Timers (for debugging purposes)
void printTimerInfo() {
	//double diff = main_timer.elapsed_time_milliseconds - (algo_timer.elapsed_time_milliseconds + io_timer_in.elapsed_time_milliseconds + io_timer_out.elapsed_time_milliseconds);

	stringstream output;
	output << "Total MAIN time      : " << main_timer.elapsed_time_milliseconds << " ms." << endl;
	double total_misc = main_timer.elapsed_time_milliseconds
		- partitioning_total_timer.elapsed_time_milliseconds
		- voxelization_total_timer.elapsed_time_milliseconds
		- svo_total_timer.elapsed_time_milliseconds;
	
	output << "  misc total time (MAIN - PART - VOX - SVO) : " << total_misc << " ms." << endl;
	output << "PARTITIONING" << endl;
	output << "  Total time		: " << partitioning_total_timer.elapsed_time_milliseconds << " ms." << endl;
	output << "  IO IN time		: " << partitioning_io_input_timer.elapsed_time_milliseconds << " ms." << endl;
	output << "  algorithm time	: " << partitioning_algorithm_timer.elapsed_time_milliseconds << " ms." << endl;
	output << "  IO OUT time		: " << partitioning_io_output_timer.elapsed_time_milliseconds << " ms." << endl;
	double part_diff = partitioning_total_timer.elapsed_time_milliseconds - partitioning_io_input_timer.elapsed_time_milliseconds - partitioning_algorithm_timer.elapsed_time_milliseconds - partitioning_io_output_timer.elapsed_time_milliseconds;
	output << "  misc time		: " << part_diff << " s." << endl;
	output << "VOXELIZING" << endl;
	output << "  Total time		: " << voxelization_total_timer.elapsed_time_milliseconds << " ms." << endl;
	output << "  IO IN time		: " << voxelization_io_input_timer.elapsed_time_milliseconds << " ms." << endl;
	output << "  algorithm time	: " << voxelization_algorithm_timer.elapsed_time_milliseconds << " ms." << endl;
	double vox_diff = voxelization_total_timer.elapsed_time_milliseconds - voxelization_io_input_timer.elapsed_time_milliseconds - voxelization_algorithm_timer.elapsed_time_milliseconds;
	output << "  misc time		: " << vox_diff << " s." << endl;
	output << "SVO BUILDING" << endl;
	output << "  Total time		: " << svo_total_timer.elapsed_time_milliseconds << " ms." << endl;
	output << "  IO OUT time		: " << svo_io_output_timer.elapsed_time_milliseconds << " ms." << endl;
	output << "  algorithm time	: " << svo_algorithm_timer.elapsed_time_milliseconds << " ms." << endl;
	double svo_misc = svo_total_timer.elapsed_time_milliseconds - svo_io_output_timer.elapsed_time_milliseconds - svo_algorithm_timer.elapsed_time_milliseconds;
	output << "  misc time		: " << svo_misc << " s." << endl;

	string output_string = output.str();
	cout << output_string;
	if(data_out)
	{
		data_writer_ptr->writeToFile(output_string);
	}	
}

std::string getTimeString() {
	std::stringstream o;
	o << time(NULL);
	return o.str();
}

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


#ifdef NO_CONSOLE_OUTPUT
	cout.setstate(std::ios_base::failbit);
#endif



	//TIMING
	setupTimers();
	main_timer.start(); // start timer of the total program

	// Parse program parameters
	printInfo(version);
	parseProgramParameters(
		argc, argv, filename, 
		gridsize_S, gridsize_T,
		voxel_memory_limit, sparseness_limit, verbose, data_out, generate_levels, 
		binvox, multiple_input_files, color_type);


	if(data_out)
	{

		string filename_without_numbers_and_extension;
		if(!multiple_input_files)
		{
			filename_without_numbers_and_extension = filename.substr(0, filename.find_last_of("."));
		}else
		{
			TriHeaderHandler::getBaseFileNameWithoutNumbersAtTheEnd(filename, filename_without_numbers_and_extension);
		}
		string data_output_filename_without_extension = filename_without_numbers_and_extension + "_S" + to_string(gridsize_S) + "_T" + to_string(gridsize_T) + "_time" + getTimeString();

		std::unique_ptr<DataWriter> dWriter(new DataWriter(data_output_filename_without_extension));
		data_writer_ptr = std::move(dWriter);

		stringstream output;
		output << "filename: " << filename << endl;
		output << "gridsize space: " << gridsize_S << endl;
		output << "gridsize time: " << gridsize_T << endl;
		output << "memory limit (MB): " << voxel_memory_limit << endl;
		output << "sparseness optimization limit: " << sparseness_limit << endl;
		output << "   resulting in memory limit: " << (sparseness_limit*voxel_memory_limit) << endl;
		output << "color type: " << color_type_to_string(color_type) << endl;
		output << "generate levels: " << generate_levels << endl;
		output << "verbosity: " << verbose << endl;
		string output_string = output.str();

		data_writer_ptr->writeToFile(output_string);
	}
	

	if(!multiple_input_files){


		// === PARTITIONING === //
		partitioning_total_timer.start();		// TIMING

		partitioning_io_input_timer.start();	// TIMING
		// Read the .tri file containing the triangle info
		// struct to contain all info read from a .tri file header
		TriInfo triangleInfo3D = readTriHeader(filename, verbose);
		partitioning_io_input_timer.stop();		// TIMING

		TriInfo4D triangleInfo4D = TriInfo4D(triangleInfo3D, end_time);

		partitioning_total_timer.stop();;	// TIMING
		vec3 translation_direction = vec3(1.0, 1.0, 1.0);
		translation_direction = normalize(translation_direction);
		TranslationHandler transformation_handler = TranslationHandler(gridsize_T, translation_direction);
		transformation_handler.speed_factor = 1.0f;
		transformation_handler.calculateTransformedBoundingBox(triangleInfo4D, end_time);
		partitioning_total_timer.start();	// TIMING

//		triangleInfo4D.mesh_bbox_transformed
//			= AABox<vec4>(
//				triangleInfo4D.mesh_bbox_transformed.min + vec4(1.0, 1.0, 1.0, 0) - vec4(0.1, 0.1, 0.1, 0),
//				triangleInfo4D.mesh_bbox_transformed.max + vec4(1.0, 1.0, 1.0, 0) + vec4(0.1, 0.1, 0.1, 0));

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

		alternatePartitioner_SingleFile partitioner = alternatePartitioner_SingleFile(gridsize_S, gridsize_T, nbOfDimensions);
		//alternatePartitioner partitioner = alternatePartitioner(gridsize_S, gridsize_T, nbOfDimensions);
		TriPartitioningInfo4D trianglePartition_info = partitioner.partitionTriangleModel(triangleInfo4D, voxel_memory_limit, &transformation_handler);

		partitioning_total_timer.stop(); // TIMING
		// === END OF PARTITIONING === //


		//start of voxelization and tree building


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
		// === PARTITIONING === //
		partitioning_total_timer.start();		// TIMING

		partitioning_io_input_timer.start();	// TIMING
		TriHeaderHandler tri_header_handler = TriHeaderHandler(filename, 0, end_time, multiple_input_files, gridsize_T, verbose);		
		TriInfo4D_multiple_files triangleInfo4D = tri_header_handler.readHeaders();
		partitioning_io_input_timer.stop();		// TIMING


//		triangleInfo4D.setBoundingBox(
//			AABox<vec4>(
//				triangleInfo4D.getTotalBoundingBox().min + vec4(-1.0, -1.0, -1.0, -1.0),
//				triangleInfo4D.getTotalBoundingBox().max + vec4(1.0, 1.0, 1.0, 1.0)));
		
		alternatePartitioner_MultipleFiles partitioner = alternatePartitioner_MultipleFiles(gridsize_S, gridsize_T, nbOfDimensions);
		TriPartitioningInfo4D trianglePartition_info = partitioner.partitionTriangleModel_multiple_files(triangleInfo4D, voxel_memory_limit);

		partitioning_total_timer.stop();		// TIMING

		// === END OF PARTITIONING === //



		// === VOXELIZATION + TREE BUILDING ===//

		voxelization_total_timer.start();	// TIMING
		voxelization_io_input_timer.start();	//TIMING

		// Parse TRIP header
		string tripheader = trianglePartition_info.base_filename + string(".trip");
		readTripHeader(tripheader, trianglePartition_info, verbose);

		voxelization_io_input_timer.stop();	// TIMING

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
		// nu zijn zowel 


		voxelization_handler.voxelizeAndBuildSVO4D();
#ifdef logVerboseToFile
		std::fclose(stdout);
#endif

	}

//TIMING
	main_timer.stop();
	printTimerInfo();
}
