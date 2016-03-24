#include "InputParameterParser.h"
#include <string>
#include <iostream>
#include "../../src/svo_builder/PrintUtils.h"
#include "../../src/svo_builder/svo_builder_util.h"
using std::string;
using std::cout;
using std::endl;

// Parse command-line params and so some basic error checking on them
void parseProgramParameters(int argc, char* argv[], std::string &filename,
	size_t &gridsize_S, size_t &gridsize_T, size_t &voxel_memory_limit,
	float &sparseness_limit, bool &verbose,
	bool &generate_levels, bool &binvox) {
	std::string color_s = "Color from model (fallback to fixed color if model has no color)";
	std::cout << "Reading program parameters ..." << std::endl;
	// Input argument validation
	if (argc < 3) {
		printInvalid();
		exit(0);
	}
	for (int i = 1; i < argc; i++) {
		// parse filename
		if (std::string(argv[i]) == "-f") {
			filename = argv[i + 1];
			size_t check_tri = filename.find(".tri");
			if (check_tri == std::string::npos) {
				cout << "Data filename does not end in .tri - I only support that file format" << endl;
				printInvalid();
				exit(0);
			}
			i++;
		}
		else if (string(argv[i]) == "-ss") {
			gridsize_S = atoi(argv[i + 1]);
			if (!isPowerOf2((unsigned int)gridsize_S)) {
				cout << "Requested gridsize is not a power of 2" << endl;
				printInvalid();
				exit(0);
			}
			i++;
		}
		else if (string(argv[i]) == "-st") {
			gridsize_T = atoi(argv[i + 1]);
			if (!isPowerOf2((unsigned int)gridsize_T)) {
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
		else if (string(argv[i]) == "-binvox") {
			binvox = true;
		}
		else {
			printInvalid(); exit(0);
		}
	}
	if (verbose) {
		cout << "  filename: " << filename << endl;
		cout << "  gridsize space: " << gridsize_S << endl;
		cout << "  gridsize time: " << gridsize_T << endl;
		cout << "  memory limit: " << voxel_memory_limit << endl;
		cout << "  sparseness optimization limit: " << sparseness_limit << " resulting in " << (sparseness_limit*voxel_memory_limit) << " memory limit." << endl;
		cout << "  color type: " << color_s << endl;
		cout << "  generate levels: " << generate_levels << endl;
		cout << "  verbosity: " << verbose << endl;
	}
}