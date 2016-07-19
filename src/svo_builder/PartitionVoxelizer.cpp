#include "PartitionVoxelizer.h"
#include "morton4D.h"

using namespace std;
using namespace trimesh;

#define X 0
#define Y 1
#define Z 2
#define T 3


/*
Voxelize 1 partition using the Schwarz method.
*/

void PartitionVoxelizer::voxelize_schwarz_method4D(Tri4DReader &reader) {
	// voxelize every triangle
	cout << "  voxelizing triangles using Schwarz..." << endl;
	while (reader.hasNext()) {
		if(reader.n_served % (reader.n_triangles / 100) == 0)
		{
			float progress = (float)reader.n_served / (float)reader.n_triangles;
			int barWidth = 70;

			std::cout << '\r' << "[";
			int pos = barWidth * progress;
			for (int i = 0; i < barWidth; ++i) {
				if (i < pos) std::cout << "=";
				else if (i == pos) std::cout << ">";
				else std::cout << " ";
			}
					std::cout << "] " << int(progress * 100.0) << " %\r";
			std::cout.flush();
		}



		// read triangle
		Triangle4D tri;
		reader.getTriangle(tri);
		voxelizeOneTriangle(tri);
	}
	cout << endl;
}

AABox<ivec4> PartitionVoxelizer::compute_Triangle_BoundingBox_gridCoord(const Triangle4D& tri)
{
	AABox<vec4> triangle_bbox_worldCoord = computeBoundingBox(tri);
	AABox<ivec4> triangle_bbox_gridCoord;

	triangle_bbox_gridCoord.min[0] = static_cast<int>(triangle_bbox_worldCoord.min[0] * unit_div);
	triangle_bbox_gridCoord.min[1] = static_cast<int>(triangle_bbox_worldCoord.min[1] * unit_div);
	triangle_bbox_gridCoord.min[2] = static_cast<int>(triangle_bbox_worldCoord.min[2] * unit_div);
	triangle_bbox_gridCoord.min[3] = static_cast<int>(triangle_bbox_worldCoord.min[3] * unit_time_div);
	triangle_bbox_gridCoord.max[0] = static_cast<int>(triangle_bbox_worldCoord.max[0] * unit_div);
	triangle_bbox_gridCoord.max[1] = static_cast<int>(triangle_bbox_worldCoord.max[1] * unit_div);
	triangle_bbox_gridCoord.max[2] = static_cast<int>(triangle_bbox_worldCoord.max[2] * unit_div);
	triangle_bbox_gridCoord.max[3] = static_cast<int>(triangle_bbox_worldCoord.max[3] * unit_time_div);

	// clamp
	triangle_bbox_gridCoord.min[0] = clampval<int>(triangle_bbox_gridCoord.min[0], partition_bbox_gridCoords.min[0], partition_bbox_gridCoords.max[0]);
	triangle_bbox_gridCoord.min[1] = clampval<int>(triangle_bbox_gridCoord.min[1], partition_bbox_gridCoords.min[1], partition_bbox_gridCoords.max[1]);
	triangle_bbox_gridCoord.min[2] = clampval<int>(triangle_bbox_gridCoord.min[2], partition_bbox_gridCoords.min[2], partition_bbox_gridCoords.max[2]);
	triangle_bbox_gridCoord.min[3] = clampval<int>(triangle_bbox_gridCoord.min[3], partition_bbox_gridCoords.min[3], partition_bbox_gridCoords.max[3]);
	triangle_bbox_gridCoord.max[0] = clampval<int>(triangle_bbox_gridCoord.max[0], partition_bbox_gridCoords.min[0], partition_bbox_gridCoords.max[0]);
	triangle_bbox_gridCoord.max[1] = clampval<int>(triangle_bbox_gridCoord.max[1], partition_bbox_gridCoords.min[1], partition_bbox_gridCoords.max[1]);
	triangle_bbox_gridCoord.max[2] = clampval<int>(triangle_bbox_gridCoord.max[2], partition_bbox_gridCoords.min[2], partition_bbox_gridCoords.max[2]);
	triangle_bbox_gridCoord.max[3] = clampval<int>(triangle_bbox_gridCoord.max[3], partition_bbox_gridCoords.min[3], partition_bbox_gridCoords.max[3]);

	return triangle_bbox_gridCoord;
}


#ifdef BINARY_VOXELIZATION
void PartitionVoxelizer::doSlowVoxelizationIfDataArrayHasOverflowed() const
{
	if (*use_data) {
		if (data->size() > data_max_items) {
			if (verbose) {
				cout << "Sparseness optimization side-array overflowed, reverting to slower voxelization." << endl;
				cout << data->size() << " > " << data_max_items << endl;
			}
			*use_data = false;
		}
	}
}
#endif

void PartitionVoxelizer::calculateTriangleProperties(const Triangle4D &tri, vec3 &e0, vec3 &e1, vec3 &e2, vec3 &n)
{
	e0 = tri.tri.v1 - tri.tri.v0;
	e1 = tri.tri.v2 - tri.tri.v1;
	e2 = tri.tri.v0 - tri.tri.v2;
	vec3 to_normalize = (e0)CROSS(e1);
	n = normalize(to_normalize); // triangle normal

}

void PartitionVoxelizer::voxelizeOneTriangle(const Triangle4D &tri)
{
#ifdef BINARY_VOXELIZATION
	doSlowVoxelizationIfDataArrayHasOverflowed();
#endif

	// compute triangle bbox in grid coordinates
	AABox<ivec4> triangle_bbox_gridCoord = compute_Triangle_BoundingBox_gridCoord(tri);

	// COMMON PROPERTIES FOR THE TRIANGLE
	vec3 e0, e1, e2, n; //edges and normal of triangle
	calculateTriangleProperties(tri, e0, e1, e2, n);


	// PLANE TEST PROPERTIES
	vec3 c = vec3(0.0f, 0.0f, 0.0f); // critical point
	if (n[X] > 0) { c[X] = unitlength; }
	if (n[Y] > 0) { c[Y] = unitlength; }
	if (n[Z] > 0) { c[Z] = unitlength; }
	float d1 = n DOT(c - tri.tri.v0);
	float d2 = n DOT((delta_p - c) - tri.tri.v0);

	// PROJECTION TEST PROPERTIES
	// XY plane
	vec2 n_xy_e0 = vec2(-1.0f*e0[Y], e0[X]);
	vec2 n_xy_e1 = vec2(-1.0f*e1[Y], e1[X]);
	vec2 n_xy_e2 = vec2(-1.0f*e2[Y], e2[X]);
	if (n[Z] < 0.0f) {
		n_xy_e0 = -1.0f * n_xy_e0;
		n_xy_e1 = -1.0f * n_xy_e1;
		n_xy_e2 = -1.0f * n_xy_e2;
	}
	float d_xy_e0 = (-1.0f * (n_xy_e0 DOT vec2(tri.tri.v0[X], tri.tri.v0[Y]))) + max(0.0f, unitlength*n_xy_e0[0]) + max(0.0f, unitlength*n_xy_e0[1]);
	float d_xy_e1 = (-1.0f * (n_xy_e1 DOT vec2(tri.tri.v1[X], tri.tri.v1[Y]))) + max(0.0f, unitlength*n_xy_e1[0]) + max(0.0f, unitlength*n_xy_e1[1]);
	float d_xy_e2 = (-1.0f * (n_xy_e2 DOT vec2(tri.tri.v2[X], tri.tri.v2[Y]))) + max(0.0f, unitlength*n_xy_e2[0]) + max(0.0f, unitlength*n_xy_e2[1]);
	// YZ plane
	vec2 n_yz_e0 = vec2(-1.0f*e0[Z], e0[Y]);
	vec2 n_yz_e1 = vec2(-1.0f*e1[Z], e1[Y]);
	vec2 n_yz_e2 = vec2(-1.0f*e2[Z], e2[Y]);
	if (n[X] < 0.0f) {
		n_yz_e0 = -1.0f * n_yz_e0;
		n_yz_e1 = -1.0f * n_yz_e1;
		n_yz_e2 = -1.0f * n_yz_e2;
	}
	float d_yz_e0 = (-1.0f * (n_yz_e0 DOT vec2(tri.tri.v0[Y], tri.tri.v0[Z]))) + max(0.0f, unitlength*n_yz_e0[0]) + max(0.0f, unitlength*n_yz_e0[1]);
	float d_yz_e1 = (-1.0f * (n_yz_e1 DOT vec2(tri.tri.v1[Y], tri.tri.v1[Z]))) + max(0.0f, unitlength*n_yz_e1[0]) + max(0.0f, unitlength*n_yz_e1[1]);
	float d_yz_e2 = (-1.0f * (n_yz_e2 DOT vec2(tri.tri.v2[Y], tri.tri.v2[Z]))) + max(0.0f, unitlength*n_yz_e2[0]) + max(0.0f, unitlength*n_yz_e2[1]);
	// ZX plane
	vec2 n_zx_e0 = vec2(-1.0f*e0[X], e0[Z]);
	vec2 n_zx_e1 = vec2(-1.0f*e1[X], e1[Z]);
	vec2 n_zx_e2 = vec2(-1.0f*e2[X], e2[Z]);
	if (n[Y] < 0.0f) {
		n_zx_e0 = -1.0f * n_zx_e0;
		n_zx_e1 = -1.0f * n_zx_e1;
		n_zx_e2 = -1.0f * n_zx_e2;
	}
	float d_xz_e0 = (-1.0f * (n_zx_e0 DOT vec2(tri.tri.v0[Z], tri.tri.v0[X]))) + max(0.0f, unitlength*n_zx_e0[0]) + max(0.0f, unitlength*n_zx_e0[1]);
	float d_xz_e1 = (-1.0f * (n_zx_e1 DOT vec2(tri.tri.v1[Z], tri.tri.v1[X]))) + max(0.0f, unitlength*n_zx_e1[0]) + max(0.0f, unitlength*n_zx_e1[1]);
	float d_xz_e2 = (-1.0f * (n_zx_e2 DOT vec2(tri.tri.v2[Z], tri.tri.v2[X]))) + max(0.0f, unitlength*n_zx_e2[0]) + max(0.0f, unitlength*n_zx_e2[1]);

	/*
	test possible grid boxes for overlap
	Check the voxels in for each interval
	X: [ triangle_bbox_gridCoord.min[0], triangle_bbox_gridCoord.max[0] ]
	Y: [ triangle_bbox_gridCoord.min[1], triangle_bbox_gridCoord.max[1] ]
	Z: [ triangle_bbox_gridCoord.min[2], triangle_bbox_gridCoord.max[2] ]
	T: [ triangle_bbox_gridCoord.min[3], triangle_bbox_gridCoord.max[3] ] -> normally only one value
	*/
	for (int x = triangle_bbox_gridCoord.min[0]; x <= triangle_bbox_gridCoord.max[0]; x++) {
		for (int y = triangle_bbox_gridCoord.min[1]; y <= triangle_bbox_gridCoord.max[1]; y++) {
			for (int z = triangle_bbox_gridCoord.min[2]; z <= triangle_bbox_gridCoord.max[2]; z++) {
				for (int t = triangle_bbox_gridCoord.min[3]; t <= triangle_bbox_gridCoord.max[3]; t++) {


/*					if(verbose)
					{
						cout << "x: " << x << ", y:" << y << ", z:" << z << ", t:" << t << endl;
					}*/
					


					uint64_t index = morton4D_Encode_for<uint64_t>(x, y, z, t, gridsize_S, gridsize_S, gridsize_S, gridsize_T);
					if (voxels[index - morton_start] == FULL_VOXEL) { continue; } // already marked, continue

					 // TRIANGLE PLANE THROUGH BOX TEST
					vec3 p = vec3(x*unitlength, y*unitlength, z*unitlength);
					float nDOTp = n DOT p;
					if ((nDOTp + d1) * (nDOTp + d2) > 0.0f) { continue; }

					// PROJECTION TESTS
					// XY
					vec2 p_xy = vec2(p[X], p[Y]);
					if (((n_xy_e0 DOT p_xy) + d_xy_e0) < 0.0f) { continue; }
					if (((n_xy_e1 DOT p_xy) + d_xy_e1) < 0.0f) { continue; }
					if (((n_xy_e2 DOT p_xy) + d_xy_e2) < 0.0f) { continue; }

					// YZ
					vec2 p_yz = vec2(p[Y], p[Z]);
					if (((n_yz_e0 DOT p_yz) + d_yz_e0) < 0.0f) { continue; }
					if (((n_yz_e1 DOT p_yz) + d_yz_e1) < 0.0f) { continue; }
					if (((n_yz_e2 DOT p_yz) + d_yz_e2) < 0.0f) { continue; }

					// XZ	
					vec2 p_zx = vec2(p[Z], p[X]);
					if  (((n_zx_e0 DOT p_zx) + d_xz_e0) < 0.0f) { continue; }
					if (((n_zx_e1 DOT p_zx) + d_xz_e1) < 0.0f) { continue; }
					if (((n_zx_e2 DOT p_zx) + d_xz_e2) < 0.0f) { continue; }

#ifdef BINARY_VOXELIZATION
					voxels[index - morton_start] = FULL_VOXEL;
					if (*use_data) { data->push_back(index); }

					if(binvox)
					{
						binvox_handler->writeVoxel(t, x, y, z);
					}

#else
					voxels[index - morton_start] = FULL_VOXEL;
					data->push_back(VoxelData(index, tri.tri.normal, average3Vec(tri.tri.v0_color, tri.tri.v1_color, tri.tri.v2_color))); // we ignore data limits for colored voxelization
#endif
					(*nfilled)++;
					continue;
				}
			}
		}
	}

}

#ifdef BINARY_VOXELIZATION
PartitionVoxelizer::PartitionVoxelizer(
	const uint64_t morton_start, const uint64_t morton_end,
	size_t gridsize_S, size_t gridsize_T,
	const float unitlength, const float unitlength_time,
	char* voxels, vector<uint64_t> *data, float sparseness_limit,
	bool* use_data, size_t* nfilled):
#else
PartitionVoxelizer::PartitionVoxelizer(
	const uint64_t morton_start, const uint64_t morton_end,
	size_t gridsize_S, size_t gridsize_T,
	const float unitlength, const float unitlength_time,
	char* voxels, vector<VoxelData> *data, float sparseness_limit,
	bool* use_data, size_t* nfilled) :
#endif
	gridsize_S(gridsize_S), gridsize_T(gridsize_T),
	unitlength(unitlength),
    voxels(voxels),
    data(data),
	sparseness_limit(sparseness_limit),
	use_data(use_data), nfilled(nfilled),
	morton_start(morton_start), binvox_handler(nullptr)
{
	memset(voxels, EMPTY_VOXEL, (morton_end - morton_start)*sizeof(char));
	data->clear();

	// compute partition min and max in grid coords
	partition_bbox_gridCoords = AABox<uivec4>();
	morton4D_Decode_for(
		morton_start,
		partition_bbox_gridCoords.min[0], partition_bbox_gridCoords.min[1], partition_bbox_gridCoords.min[2], partition_bbox_gridCoords.min[3],
		gridsize_S, gridsize_S, gridsize_S, gridsize_T);
	morton4D_Decode_for(
		morton_end - 1,
		partition_bbox_gridCoords.max[0], partition_bbox_gridCoords.max[1], partition_bbox_gridCoords.max[2], partition_bbox_gridCoords.max[3],
		gridsize_S, gridsize_S, gridsize_S, gridsize_T);

	if (verbose) {
		cout << "  grid coordinates for bbox of this partition: " << endl
			<< "      from ("
			<< partition_bbox_gridCoords.min[0] << ", " << partition_bbox_gridCoords.min[1] << ", " << partition_bbox_gridCoords.min[2] << ", " << partition_bbox_gridCoords.min[3]
			<< ") to ("
			<< partition_bbox_gridCoords.max[0] << ", " << partition_bbox_gridCoords.max[1] << ", " << partition_bbox_gridCoords.max[2] << ", " << partition_bbox_gridCoords.max[3] << ")" << endl;
	}


	// compute maximum grow size for data array
#ifdef BINARY_VOXELIZATION
	uint64_t max_bytes_data = static_cast<uint64_t>(((morton_end - morton_start)*sizeof(char)) * sparseness_limit);
	if (*use_data) {
		data_max_items = max_bytes_data / sizeof(uint64_t);
	}
	else {
		data_max_items = max_bytes_data / sizeof(VoxelData);
	}
#endif

	// COMMON PROPERTIES FOR ALL TRIANGLES
	unit_div = 1.0f / unitlength;
	unit_time_div = 1.0f / unitlength_time;
	//vec4 delta_p = vec4(unitlength, unitlength, unitlength, unitlength_time);
	delta_p = vec3(unitlength, unitlength, unitlength);
}
