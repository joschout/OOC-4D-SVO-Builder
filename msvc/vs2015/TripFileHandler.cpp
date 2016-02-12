#include "TripFileHandler.h"

// Trip header handling and error checking
void readTripHeader(string& filename, TripInfo4D& trip_info, bool verbose) {
	if (TripInfo4D::parseTrip4DHeader(filename, trip_info) != 1) {
		exit(0);
	}
	if (!trip_info.filesExist()) {
		cout << "Not all required .trip or .tripdata files exist. Please regenerate using svo_builder." << endl;
		exit(0); // not all required files exist - exiting.
	}
	if (verbose) { trip_info.print(); }
}

int TripInfo4D::parseTrip4DHeader(const std::string& filename, TripInfo4D& t)
{
	ifstream file;
	file.open(filename.c_str(), ios::in);

	t.base_filename = filename.substr(0, filename.find_last_of("."));

	string line; file >> line;  // #trip
	if (line.compare("#trip") != 0) {
		cout << "  Error: first line reads [" << line << "] instead of [#trip]" << endl; return 0;
	}
	file >> t.version;

	bool done = false;
	t.geometry_only = 0;

	while (file.good() && !done) {
		file >> line;
		if (line.compare("END") == 0) {
			done = true; // when we encounter data keyword, we're at the end of the ASCII header
		}
		else if (line.compare("gridsize") == 0) {
			file >> t.gridsize;
		}
		else if (line.compare("n_triangles") == 0) {
			file >> t.n_triangles;
		}
		else if (line.compare("geo_only") == 0) {
			file >> t.geometry_only;
		}
		else if (line.compare("bbox") == 0) {
			file >> t.mesh_bbox_transl.min[0]
				>> t.mesh_bbox_transl.min[1]
				>> t.mesh_bbox_transl.min[2]
				>> t.mesh_bbox_transl.min[3]
				>> t.mesh_bbox_transl.max[0]
				>> t.mesh_bbox_transl.max[1]
				>> t.mesh_bbox_transl.max[2]
				>> t.mesh_bbox_transl.max[3];
		}
		else if (line.compare("n_partitions") == 0) {
			file >> t.n_partitions; // read number of partitions
			t.nbOfTrianglesPerPartition.resize(t.n_partitions);
			int index;
			size_t tricount;
			for (size_t i = 0; i < t.n_partitions; i++) {
				file >> index >> tricount;
				t.nbOfTrianglesPerPartition[index] = tricount;
			}
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

void TripInfo4D::writeTrip4DHeader(const std::string& filename, const TripInfo4D& t)
{
	// open file for writing
	ofstream outfile;
	outfile.open(filename.c_str(), ios::out);
	// write ASCII header
	outfile << "#trip " << t.version << endl;
	outfile << "gridsize " << t.gridsize << endl;
	outfile << "n_triangles " << t.n_triangles << endl;
	outfile << "bbox  "
		<< t.mesh_bbox_transl.min[0] << " "
		<< t.mesh_bbox_transl.min[1] << " "
		<< t.mesh_bbox_transl.min[2] << " "
		<< t.mesh_bbox_transl.min[3] << " "
		<< t.mesh_bbox_transl.max[0] << " "
		<< t.mesh_bbox_transl.max[1] << " "
		<< t.mesh_bbox_transl.max[2] << " "
		<< t.mesh_bbox_transl.max[3] << endl;
#ifdef BINARY_VOXELIZATION
	outfile << "geo_only " << 1 << endl;
#else
	outfile << "geo_only " << 0 << endl;
#endif
	outfile << "n_partitions " << t.n_partitions << endl;

	for (size_t i = 0; i < t.n_partitions; i++) {
		outfile << i << " " << t.nbOfTrianglesPerPartition[i] << endl;
	}
	outfile << "END" << endl;
}