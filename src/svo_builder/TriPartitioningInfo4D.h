#ifndef TRIPARTITIONINGINFO4D_H
#define TRIPARTITIONINGINFO4D_H

#include <trip_tools.h>

class TriInfo4D;

class TriPartitioningInfo4D
{
public:
	//TripInfo tripInfo3D;
	AABox<vec4> mesh_bbox_transl;
	//vec3 translation_direction;
	float end_time;

	string base_filename;
	int version;
	int geometry_only;
	size_t gridsize_S;
	size_t gridsize_T;
	//AABox<vec3> mesh_bbox;
	size_t n_triangles;
	size_t n_partitions;
	vector<size_t> nbOfTrianglesPerPartition;


	TriPartitioningInfo4D();
	TriPartitioningInfo4D(const TriInfo4D &t);
	~TriPartitioningInfo4D();

	void print() const;
	bool filesExist() const;
	static int parseTrip4DHeader(const std::string &filename, TriPartitioningInfo4D &t);
	static void writeTrip4DHeader(const std::string &filename, const TriPartitioningInfo4D &t);
};

#endif

