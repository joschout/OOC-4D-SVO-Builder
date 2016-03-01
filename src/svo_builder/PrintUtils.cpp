#include "PrintUtils.h"

#include <stdio.h>  /* defines FILENAME_MAX */
#include <cerrno>
#include <iostream>
#define WINDOWS
#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif
#include <sstream>
int printCurrentDirectory()
{
	char cCurrentPath[FILENAME_MAX];

	if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
	{
		return errno;
	}

	cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; /* not really required */

	std::cout << "The current working directory is:" << std::endl << cCurrentPath << std::endl;
	std::cout << "(You should put your .tri files here)" << std::endl;
	std::cout << "" << std::endl;
	return 0;
}

void printInfo(std::string &version) {
	std::cout << "--------------------------------------------------------------------" << std::endl;
#ifdef BINARY_VOXELIZATION
	std::cout << "Out-Of-Core Tree4D Builder " << version << " - Geometry only version" << std::endl;
#else
	std::cout << "Out-Of-Core Tree4D Builder " << version << std::endl;
#endif
#if defined(_WIN32) || defined(_WIN64)
	std::cout << "Windows " << std::endl;
#endif
#ifdef __linux__
	cout << "Linux " << endl;
#endif
#ifdef _WIN64
	std::cout << "64-bit version" << std::endl;
#endif
	std::cout << "Based on \'Out-of-Core Octree Builder\', by Jeroen Baert" << std::endl;
	std::cout << "Jeroen Baert - jeroen.baert@cs.kuleuven.be - www.forceflow.be" << std::endl;
	std::cout << "Adapted for his master thesis by Jonas Schouterden" << std::endl;
	std::cout << "--------------------------------------------------------------------" << std::endl << std::endl;
	printCurrentDirectory();
}

void printHelp() {
	std::cout << "Example: svo_builder -f /home/jeroen/bunny.tri" << std::endl;
	std::cout << "" << std::endl;
	std::cout << "All available program options:" << std::endl;
	std::cout << "" << std::endl;
	std::cout << "-f <filename.tri>     Path to a .tri input file." << std::endl;
	std::cout << "-s <gridsize>         Voxel gridsize, should be a power of 2. Default 512." << std::endl;
	std::cout << "-l <memory_limit>     Memory limit for process, in Mb. Default 1024." << std::endl;
	std::cout << "-levels               Generate intermediary voxel levels by averaging voxel data" << std::endl;
	std::cout << "-c <option>           Coloring of voxels (Options: model (default), fixed, linear, normal)" << std::endl;
	std::cout << "-d <percentage>       Percentage of memory limit to be used additionaly for sparseness optimization" << std::endl;
	std::cout << "-v                    Be very verbose." << std::endl;
	std::cout << "-h                    Print help and exit." << std::endl;
}

void printInvalid() {
	std::cout << "Not enough or invalid arguments, please try again." << std::endl;
	std::cout << "At the bare minimum, I need a path to a .TRI file" << std::endl << "" << std::endl;
	printHelp();
}