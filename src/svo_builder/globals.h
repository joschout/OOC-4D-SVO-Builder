#ifndef GLOBALS_H_
#define GLOBALS_H_
#include <memory>
#include "svo_builder_util.h"
#include "DataWriter.h"

using namespace std;

// global flag: be verbose about what we do?
extern bool verbose;
extern bool data_out;

//global flag: should we create separate binvox files for each timepoint?
extern bool binvox;
// Timers (for debugging purposes)
// (This is a bit ugly, but it's a quick and surefire way to measure performance)

extern unique_ptr<DataWriter> data_writer_ptr;


// Main program timer
extern Timer main_timer;

// Timers for partitioning step
extern Timer partitioning_total_timer;
extern Timer partitioning_io_input_timer;
extern Timer partitioning_io_output_timer;
extern Timer partitioning_algorithm_timer;

// Timers for voxelizing step
extern Timer voxelization_total_timer;
extern Timer voxelization_io_input_timer;
extern Timer voxelization_algorithm_timer;

// Timers for SVO building step
extern Timer svo_total_timer;
extern Timer svo_io_output_timer;
extern Timer svo_algorithm_timer;

// Voxelization-related stuff
typedef Vec<3, unsigned int> uivec3;
typedef Vec<4, unsigned int> uivec4;

#endif // GLOBALS_H_