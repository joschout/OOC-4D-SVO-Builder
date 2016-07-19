//#ifndef VOXELIZER_H_
//#define VOXELIZER_H_
//
//#include "../libs/libtri/include/tri_tools.h"
//#include "../libs/libtri/include/TriReader.h"
//#include "globals.h"
//#include "intersection.h"
//#include "../libs/libmorton/include/morton.h"
//#include "VoxelData.h"
//#include "Tri4DReader.h"
//
//// Voxelization-related stuff
//typedef Vec<3, unsigned int> uivec3;
//typedef Vec<4, unsigned int> uivec4;
//using namespace std;
//
//#define EMPTY_VOXEL 0
//#define FULL_VOXEL 1
//
//
//
//#ifdef BINARY_VOXELIZATION
//void voxelize_schwarz_method4D(Tri4DReader &reader, const uint64_t morton_start, const uint64_t morton_end, const float unitlength, const float unitlength_time, char* voxels, vector<uint64_t> &data, float sparseness_limit, bool &use_data, size_t &nfilled);
//#else
//void voxelize_schwarz_method4D(Tri4DReader &reader, const uint64_t morton_start, const uint64_t morton_end, const float unitlength, const float unitlength_time, char* voxels, vector<VoxelData> &data, float sparseness_limit, bool &use_data, size_t &nfilled);
//#endif
//
//
//#ifdef BINARY_VOXELIZATION
//
//void voxelizeOneTriangle(
//	const Triangle4D &tri,
//	bool &use_data, size_t data_max_items, vector<uint64_t> &data,
//	float unit_div, float unit_time_div, const vec3 &delta_p,
//	const AABox<uivec4> partition_bbox_gridCoords,
//	const float unitlength, char* voxels, const uint64_t morton_start,
//	size_t &nfilled
//	);
//
//#else
//
//void voxelizeOneTriangle(
//	const Triangle4D &tri,
//	vector<VoxelData>&data,
//	float unit_div, float unit_time_div, const vec3 &delta_p,
//	const AABox<uivec4> partition_bbox_gridCoords,
//	const float unitlength, char* voxels, const uint64_t morton_start,
//	size_t &nfilled
//	);
//
//#endif
//
//
//
//
//
//
//#endif // VOXELIZER_H_