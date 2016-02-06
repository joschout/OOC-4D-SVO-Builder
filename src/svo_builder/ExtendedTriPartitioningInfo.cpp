#include "ExtendedTriPartitioningInfo.h"
#include "ExtendedTriInfo.h"

TripInfo4D::TripInfo4D():
	mesh_bbox_transl(AABox<vec4>()),
	end_time(0), base_filename(""), version(1),
	geometry_only(0), gridsize(0), n_triangles(0),
	n_partitions(0)
{	
}

TripInfo4D::TripInfo4D(const ExtendedTriInfo  &t):
	mesh_bbox_transl(t.mesh_bbox_transl),
	end_time(t.end_time), base_filename(t.triInfo3D.base_filename),
	version(t.triInfo3D.version),
	geometry_only(t.triInfo3D.geometry_only), gridsize(0), n_triangles(0),
	n_partitions(0)
{
}

TripInfo4D::~TripInfo4D()
{
}

void TripInfo4D::print() const
{
	cout << "  base_filename: " << base_filename << endl;
	cout << "  trip version: " << version << endl;
	cout << "  geometry only: " << geometry_only << endl;
	cout << "  gridsize: " << gridsize << endl;
	cout << "  bbox min: " 
		<< mesh_bbox_transl.min[0] << " " 
		<< mesh_bbox_transl.min[1] << " " 
		<< mesh_bbox_transl.min[2] << " "
		<< mesh_bbox_transl.min[3] << endl;
	cout << "  bbox max: " 
		<< mesh_bbox_transl.max[0] << " "
		<< mesh_bbox_transl.max[1] << " "
		<< mesh_bbox_transl.max[2] << " "
		<< mesh_bbox_transl.max[3] << endl;
	cout << "  n_triangles: " << n_triangles << endl;
	cout << "  n_partitions: " << n_partitions << endl;
	for (size_t i = 0; i< n_partitions; i++) {
		cout << "  partition " << i << " - tri_count: " << nbOfTrianglesPerPartition[i] << endl;
	}

}

bool TripInfo4D::filesExist() const
{
	string header = base_filename + string(".trip");
	for (size_t i = 0; i< n_partitions; i++) {
		if (nbOfTrianglesPerPartition[i] > 0) { // we only require the file to be there if it contains any triangles.
			string part_data_filename 
				= base_filename + string("_") + val_to_string(i) + string(".tripdata");
			if (!file_exists(part_data_filename)) {
				return false;
			}
		}
	}
	return (file_exists(header));
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
