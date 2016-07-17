#include "TriInfo4D_multiple_files.h"

TriInfo4D_multiple_files::TriInfo4D_multiple_files():
	base_filename_without_number(""), total_n_triangles(0)
{
}

TriInfo4D_multiple_files::TriInfo4D_multiple_files(string base_filename_without_number, float start_time, float end_time): 
	base_filename_without_number(base_filename_without_number),
	total_bounding_box(AABox<vec4>(vec4(0, 0, 0, start_time), vec4(0, 0, 0, end_time))), total_n_triangles(0)
{
}

void TriInfo4D_multiple_files::addTriInfo(const TriInfo& tri_info)
{
	tri_info_vector_.push_back(tri_info);
	expandBoundingBox(tri_info.mesh_bbox);
	incrementNbOfTriangles(tri_info.n_triangles);

}

std::vector<TriInfo> TriInfo4D_multiple_files::getTriInfoVector() const
{
	return tri_info_vector_;
}

AABox<vec4> TriInfo4D_multiple_files::getTotalBoundingBox() const
{
	return total_bounding_box;
}


size_t TriInfo4D_multiple_files::getNbfOfTriangles() const
{
	return total_n_triangles;
}

void TriInfo4D_multiple_files::expandBoundingBox(const AABox<vec3>& bounding_box_to_add)
{
	if(tri_info_vector_.size() == 1)
	{
		total_bounding_box.min[0] =  bounding_box_to_add.min[0];
		total_bounding_box.min[1] =  bounding_box_to_add.min[1];
		total_bounding_box.min[2] = bounding_box_to_add.min[2];

		total_bounding_box.max[0] = bounding_box_to_add.max[0];
		total_bounding_box.max[1] = bounding_box_to_add.max[1];
		total_bounding_box.max[2] = bounding_box_to_add.max[2];
	}else
	{
		total_bounding_box.min[0] = min(total_bounding_box.min[0], bounding_box_to_add.min[0]);
		total_bounding_box.min[1] = min(total_bounding_box.min[1], bounding_box_to_add.min[1]);
		total_bounding_box.min[2] = min(total_bounding_box.min[2], bounding_box_to_add.min[2]);

		total_bounding_box.max[0] = max(total_bounding_box.max[0], bounding_box_to_add.max[0]);
		total_bounding_box.max[1] = max(total_bounding_box.max[1], bounding_box_to_add.max[1]);
		total_bounding_box.max[2] = max(total_bounding_box.max[2], bounding_box_to_add.max[2]);
	}

}

void TriInfo4D_multiple_files::incrementNbOfTriangles(size_t n_triangles)
{
	total_n_triangles += n_triangles;
}
