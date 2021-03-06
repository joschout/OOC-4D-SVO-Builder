#ifndef PARTITIONVOXELIZER_H
#define PARTITIONVOXELIZER_H


#include "../libs/libtri/include/tri_tools.h"
#include "../libs/libtri/include/TriReader.h"
#include "globals.h"
#include "intersection.h"
#include "../libs/libmorton/include/morton.h"
#include "VoxelData.h"
#include "Tri4DReader.h"
#include "morton4D.h"
#include "BinvoxHandler.h"


using namespace std;

#define EMPTY_VOXEL 0
#define FULL_VOXEL 1


class PartitionVoxelizer
{
public:
	size_t gridsize_S;
	size_t gridsize_T;

	const float unitlength;
	char* voxels;

#ifdef BINARY_VOXELIZATION
	vector<uint64_t> *data;
	size_t data_max_items;
#else
	vector<VoxelData> *data;
#endif
	const float sparseness_limit;
	bool *use_data;
	size_t* nfilled;

	AABox<uivec4> partition_bbox_gridCoords;
	const uint64_t morton_start;
	float unit_div;
	float unit_time_div;
	vec3 delta_p;
	BinvoxHandler *binvox_handler;


#ifdef BINARY_VOXELIZATION
	PartitionVoxelizer(const uint64_t morton_start, const uint64_t morton_end,
		size_t gridsize_S, size_t gridsize_T,
		const float unitlength, const float unitlength_time,
		char* voxels, vector<uint64_t> *data, float sparseness_limit,
		bool* use_data, size_t* nfilled);
#else
	PartitionVoxelizer(const uint64_t morton_start, const uint64_t morton_end,
		size_t gridsize_S, size_t gridsize_T,
		const float unitlength, const float unitlength_time,
		char* voxels, vector<VoxelData> *data, float sparseness_limit,
		bool* use_data, size_t* nfilled);
#endif

	void voxelize_schwarz_method4D(Tri4DReader &reader);

private:
	AABox<ivec4> compute_Triangle_BoundingBox_gridCoord(const Triangle4D & tri);
	static void calculateTriangleProperties(const Triangle4D &tri, vec3 &e0, vec3 &e1, vec3 &e2, vec3 &n);
	void voxelizeOneTriangle(const Triangle4D &tri);
#ifdef BINARY_VOXELIZATION
	void doSlowVoxelizationIfDataArrayHasOverflowed() const;
#endif

};
#endif
