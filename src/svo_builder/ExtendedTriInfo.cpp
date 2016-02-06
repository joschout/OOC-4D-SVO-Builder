#include "ExtendedTriInfo.h"
#include "TranslationHandler.h"


#include <stdio.h>  /* defines FILENAME_MAX */

#include <direct.h>
#define GetCurrentDir _getcwd

ExtendedTriInfo::ExtendedTriInfo(): end_time(0)
{
}

ExtendedTriInfo::ExtendedTriInfo(TriInfo triInfo, vec3 translation_direction, float end_time):
	triInfo3D(triInfo), mesh_bbox_transl(AABox<vec4>(vec4(), vec4())),
	translation_direction(translation_direction), end_time(end_time)
{

	AABox<vec3> start_box = triInfo3D.mesh_bbox;
	
	vec3 translated_min = translate(start_box.min, translation_direction, end_time);
	vec3 translated_max = translate(start_box.max, translation_direction, end_time);

	mesh_bbox_transl.min[0] = min(start_box.min[0], min(translated_min[0], translated_max[0]));
	mesh_bbox_transl.min[1] = min(start_box.min[1], min(translated_min[1], translated_max[1]));
	mesh_bbox_transl.min[2] = min(start_box.min[2], min(translated_min[2], translated_max[2]));

	mesh_bbox_transl.max[0] = max(start_box.max[0], max(translated_min[0], translated_max[0]));
	mesh_bbox_transl.max[1] = max(start_box.max[1], max(translated_min[1], translated_max[1]));
	mesh_bbox_transl.max[2] = max(start_box.max[2], max(translated_min[2], translated_max[2]));

	mesh_bbox_transl.min[3] = 0;
	mesh_bbox_transl.max[3] = end_time;

}

ExtendedTriInfo::~ExtendedTriInfo()
{
}

int ExtendedTriInfo::parseTri3DHeader(std::string filename, TriInfo& t)
{
	ifstream file;
	//file.open(filename.c_str(), ios::in);
	file.open("sphere.tri", ios::in);

	if (file.is_open()) {
		cout << filename + " should be correctly opened" << std::endl;

		string temp = filename.substr(0, filename.find_last_of("."));
		t.base_filename = temp;
		string line = "";
	

		file >> line;  // #tri
		if (line.compare("#tri") != 0) {
			cout << "  Error: first line reads [" << line << "] instead of [#tri]" << endl; return 0;
		}
		file >> t.version;

		bool done = false;
		t.geometry_only = 0;

		while (file.good() && !done) {
			file >> line;
			if (line.compare("END") == 0) {
				done = true; // when we encounter data keyword, we're at the end of the ASCII header
			}
			else if (line.compare("ntriangles") == 0) {
				file >> t.n_triangles;
			}
			else if (line.compare("geo_only") == 0) {
				file >> t.geometry_only;
			}
			else if (line.compare("bbox") == 0) {
				file >> t.mesh_bbox.min[0] >> t.mesh_bbox.min[1] >> t.mesh_bbox.min[2] >> t.mesh_bbox.max[0] >> t.mesh_bbox.max[1] >> t.mesh_bbox.max[2];
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

	cout << "  Error: file " << filename << " does not exist." << endl;
	return 0;
	
}
