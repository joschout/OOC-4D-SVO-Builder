#include "ExtendedTriPartitioningInfo.h"
#include "TriInfo4D.h"

TripInfo4D::TripInfo4D():
	mesh_bbox_transl(AABox<vec4>()),
	end_time(0), base_filename(""), version(1),
	geometry_only(0), gridsize(0), n_triangles(0),
	n_partitions(0)
{	
}

TripInfo4D::TripInfo4D(const TriInfo4D  &t):
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

