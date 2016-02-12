#ifndef INPUTPARAMETERPARSER_H
#define INPUTPARAMETERPARSER_H
#include <string>

// Parse command-line params and so some basic error checking on them
void parseProgramParameters(int argc, char* argv[], std::string &filename,
	size_t &gridsize, size_t &voxel_memory_limit,
	float &sparseness_limit, bool &verbose,
	bool &generate_levels);
#endif

