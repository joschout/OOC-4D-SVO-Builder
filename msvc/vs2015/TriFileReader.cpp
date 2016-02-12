#include "TriFileReader.h"
#include <string>

// Tri header handling and error checking
TriInfo readTriHeader(std::string& filename, bool verbose) {
	TriInfo tri_info = TriInfo();

	std::cout << "Parsing tri header " << filename << " ..." << std::endl;
	if (parseTri3DHeader(filename, tri_info) != 1) {
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

int parseTri3DHeader(std::string filename, TriInfo& t)
{
	ifstream file;
	file.open(filename.c_str(), ios::in);

	if (file.is_open()) {
		cout << filename + " should be correctly opened" << std::endl;

		string temp = filename.substr(0, filename.find_last_of("."));
		t.base_filename = temp;
		string line = "";


		file >> line;  // #tri
		if (line.compare("#tri") != 0) {
			cout << "  Error: first line reads [" << line << "] instead of [#tri]" << endl; return 0;
		}
		file >> t.version;

		bool done = false;
		t.geometry_only = 0;

		while (file.good() && !done) {
			file >> line;
			if (line.compare("END") == 0) {
				done = true; // when we encounter data keyword, we're at the end of the ASCII header
			}
			else if (line.compare("ntriangles") == 0) {
				file >> t.n_triangles;
			}
			else if (line.compare("geo_only") == 0) {
				file >> t.geometry_only;
			}
			else if (line.compare("bbox") == 0) {
				file >> t.mesh_bbox.min[0] >> t.mesh_bbox.min[1] >> t.mesh_bbox.min[2] >> t.mesh_bbox.max[0] >> t.mesh_bbox.max[1] >> t.mesh_bbox.max[2];
			}
			else {
				cout << "  unrecognized keyword [" << line << "], skipping" << endl;
				char c; do { c = file.get(); } while (file.good() && (c != '\n'));
			}
		}
		if (!done) {
			cout << "  error reading header" << endl; return 0;
		}
		file.close();
		return 1;
	}

	cout << "  Error: file " << filename << " does not exist." << endl;
	return 0;

}
