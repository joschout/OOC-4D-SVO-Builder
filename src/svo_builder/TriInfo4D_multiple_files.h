#ifndef TRIINFO4D_MULTIPLEFILES_H
#define TRIINFO4D_MULTIPLEFILES_H
#include <vector>
#include <tri_tools.h>

class TriInfo4D_multiple_files
{
public:
	

	string base_filename_without_number;

	TriInfo4D_multiple_files();
	TriInfo4D_multiple_files(string base_filename_without_number, float start_time, float end_time);
	void addTriInfo(const TriInfo& tri_info);
	std::vector<TriInfo> getTriInfoVector() const; 
	AABox<vec4> getTotalBoundingBox() const;
	size_t getNbfOfTriangles() const;
	
private:
	std::vector<TriInfo> tri_info_vector_;

	//bounding box around all added TriInfo instances
	AABox<vec4> total_bounding_box;

	size_t total_n_triangles;

	void expandBoundingBox(const AABox<vec3>& bounding_box_to_add);
	void incrementNbOfTriangles(size_t n_triangles);
};


#endif

