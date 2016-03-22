#ifndef PARTITIONVOXELIZER_H
#define PARTITIONVOXELIZER_H


#include "../libs/libtri/include/tri_tools.h"
#include "../libs/libtri/include/TriReader.h"
#include "globals.h"
#include "intersection.h"
#include "../libs/libmorton/include/morton.h"
#include "VoxelData.h"
#include "../../msvc/vs2015/Tri4DReader.h"
#include "morton4D.h"
#include "BinvoxHandler.h"

// Voxelization-related stuff
typedef Vec<3, unsigned int> uivec3;
typedef Vec<4, unsigned int> uivec4;
using namespace std;

#define EMPTY_VOXEL 0
#define FULL_VOXEL 1


class PartitionVoxelizer
{
public:

	const float unitlength;
	char* voxels;

#ifdef BINARY_VOXELIZATION
	vector<uint64_t> *data;
#else
	vector<VoxelData> *data
#endif
	const float sparseness_limit;
	bool *use_data;
	size_t* nfilled;

	AABox<uivec4> partition_bbox_gridCoords;
	const uint64_t morton_start;
#ifdef BINARY_VOXELIZATION
	size_t data_max_items;
#endif
	float unit_div;
	float unit_time_div;
	vec3 delta_p;
	BinvoxHandler *binvox_handler;

	PartitionVoxelizer(const uint64_t morton_start, const uint64_t morton_end,
		const float unitlength, const float unitlength_time,
		char* voxels, vector<uint64_t> *data, float sparseness_limit,
		bool* use_data, size_t* nfilled);

	void voxelize_schwarz_method4D(Tri4DReader &reader);

private:
	AABox<ivec4> compute_Triangle_BoundingBox_gridCoord(const Triangle4D & tri);
	void doSlowVoxelizationIfDataArrayHasOverflowed() const;
	static void calculateTriangleProperties(const Triangle4D &tri, vec3 &e0, vec3 &e1, vec3 &e2, vec3 &n);
	void voxelizeOneTriangle(const Triangle4D &tri);

};
#endif
