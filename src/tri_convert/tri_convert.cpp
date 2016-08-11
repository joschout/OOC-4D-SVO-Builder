#include <TriMesh.h>
#include <vector>
#include <string>
#include <sstream>
#include "tri_convert_util.h"
#include "../svo_builder/svo_builder_util.h"
#include "FileNameHandler.h"
#include <iomanip>

using namespace std;
using namespace trimesh;

// Program version
string version = "1.5";

// Program parameters
string filename = "";
bool recompute_normals = false;
vec3 fixed_color = vec3(1.0f, 1.0f, 1.0f);

size_t gridsize_T = 1;
bool multiple_input_files = false;


#define WINDOWS
#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif
int printCurrentDirectory()
{
	char cCurrentPath[FILENAME_MAX];

	if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
	{
		return errno;
	}

	cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; /* not really required */

	std::cout << "The current working directory is:" << std::endl << cCurrentPath << std::endl;
	std::cout << "(You should put your octree files here)" << std::endl;
	std::cout << "" << std::endl;
	return 0;
}

void printInfo(){
	
	
	cout << "-------------------------------------------------------------" << endl;
#ifdef BINARY_VOXELIZATION
	cout << "Tri Converter " << version << " - BINARY VOXELIZATION"<< endl;
#else
	cout << "Tri Converter " << version << " - GEOMETRY+NORMALS"<< endl;
#endif
#if defined(_WIN32) || defined(_WIN64)
	cout << "Windows ";
#endif
#ifdef __linux__
	cout << "Linux ";
#endif
#ifdef _WIN64
	cout << "64-bit version" << endl;
#endif
	cout << "Jeroen Baert - jeroen.baert@cs.kuleuven.be - www.forceflow.be" << endl;
	cout << "-------------------------------------------------------------" << endl << endl;
}

void printHelp(){
	std::cout << "Example: tri_convert -f /home/jeroen/bunny.ply" << endl;
	std::cout << "" << endl;
	std::cout << "All available program options:" << endl;
	std::cout << "" << endl;
	std::cout << "-f <filename>         Path to a model input file (.ply, .obj, .3ds, .sm, .ray or .off)." << endl;
	std::cout << "-r                    Recompute face normals." << endl;
	std::cout << "-h                    Print help and exit." << endl;
}

void printInvalid(){
	std::cout << "Not enough or invalid arguments, please try again.\n" << endl; 
	printHelp();
}

void parseProgramParameters(int argc, char* argv[]){
	// Input argument validation
	if(argc<3){ // not enough arguments
		printInvalid(); exit(0);
	} 
	for (int i = 1; i < argc; i++) {
			if (string(argv[i]) == "-f") {
				multiple_input_files = false;
				filename = argv[i + 1]; 
				i++;
			} 
			else if (string(argv[i]) == "-mf") {
				multiple_input_files = true;
				filename = argv[i + 1];
				i++;
			}
			else if (string(argv[i]) == "-st") {
				gridsize_T = atoi(argv[i + 1]);
				i++;
			}
			else if (string(argv[i]) == "-r") {
				recompute_normals = true;
			} else if(string(argv[i]) == "-h") {
				printHelp(); exit(0);
			} else {
				printInvalid(); exit(0);
			}
	}
	cout << "  filename: " << filename << endl;
	cout << "  recompute normals: " << recompute_normals << endl;
}


int main(int argc, char *argv[]){
	printInfo();
	printCurrentDirectory();

	// Parse parameters
	parseProgramParameters(argc,argv);

	if(!multiple_input_files)
	{
		// Read mesh
		TriMesh *themesh = TriMesh::read(filename.c_str());
		themesh->need_faces(); // unpack triangle strips so we have faces
		themesh->need_bbox(); // compute the bounding box
#ifndef BINARY_VOXELIZATION
		themesh->need_normals(); // check if there are normals, and if not, recompute them
								 // TODO: Check for colors here, inform user about decision
#endif


		AABox<vec3> mesh_bbox = createMeshBBCube(themesh); // pad the mesh BBOX out to be a cube

														   // Moving mesh to origin
		cout << "Moving mesh to origin ... ";
		Timer timer = Timer();
		for (size_t i = 0; i < themesh->vertices.size(); i++) {
			themesh->vertices[i] = themesh->vertices[i] - mesh_bbox.min;
		}
		cout << "done in " << timer.elapsed_time_milliseconds << " s." << endl;

		// Write mesh to format we can stream in
		string base = filename.substr(0, filename.find_last_of("."));
		std::string tri_header_out_name = base + string(".tri");
		std::string tri_out_name = base + string(".tridata");

		FILE* tri_out = fopen(tri_out_name.c_str(), "wb");

		cout << "Writing mesh triangles ... "; timer.reset();
		Triangle t = Triangle();
		// Write all triangles to data file
		for (size_t i = 0; i < themesh->faces.size(); i++) {
			t.v0 = themesh->vertices[themesh->faces[i][0]];
			t.v1 = themesh->vertices[themesh->faces[i][1]];
			t.v2 = themesh->vertices[themesh->faces[i][2]];
#ifndef BINARY_VOXELIZATION
			// COLLECT VERTEX COLORS
			if (!themesh->colors.empty()) { // if this mesh has colors, we're going to use them
				t.v0_color = themesh->colors[themesh->faces[i][0]];
				t.v1_color = themesh->colors[themesh->faces[i][1]];
				t.v2_color = themesh->colors[themesh->faces[i][2]];
			}
			// COLLECT NORMALS
			if (recompute_normals) {
				t.normal = computeFaceNormal(themesh, i); // recompute normals
			}
			else {
				t.normal = getShadingFaceNormal(themesh, i); // use mesh provided normals
			}
#endif
			writeTriangle(tri_out, t);
		}
		cout << "done in " << timer.elapsed_time_milliseconds << " ms." << endl;

		// Prepare tri_info and write header
		cout << "Writing header to " << tri_header_out_name << " ... " << endl;
		TriInfo tri_info;
		tri_info.version = 1;
		tri_info.mesh_bbox = mesh_bbox;
		tri_info.n_triangles = themesh->faces.size();
#ifdef BINARY_VOXELIZATION
		tri_info.geometry_only = 1;
#else
		tri_info.geometry_only = 0;
#endif
		writeTriHeader(tri_header_out_name, tri_info);
		tri_info.print();
		cout << "Done." << endl;
		
	}else
	{

		AABox<vec3> global_box = AABox<vec3>();

		string filename_without_numbers_and_extension;
		int nbOfCharsInNumberAtTheEnd = getBaseFileNameWithoutNumbersAtTheEnd(filename, filename_without_numbers_and_extension);
		std::string file_extension = filename.substr(filename.find_last_of("."));


		int nbOfFiles = gridsize_T;

		for (int fileNumber = 0; fileNumber < nbOfFiles; fileNumber++)
		{
			stringstream ss;
			ss << std::setw(nbOfCharsInNumberAtTheEnd) << std::setfill('0') << fileNumber + 1;
			string current_ending_number = ss.str();
			string file_name_current_geometry_file = filename_without_numbers_and_extension + current_ending_number + file_extension;
		
			TriMesh *themesh = TriMesh::read(file_name_current_geometry_file.c_str());
			themesh->need_faces(); // unpack triangle strips so we have faces
			themesh->need_bbox(); // compute the bounding box
			//AABox<vec3> mesh_bbox = createMeshBBCube(themesh);
			
			//update global bounding box
			if (fileNumber == 0)
			{
				global_box.min[0] = themesh->bbox.min[0];
				global_box.min[1] = themesh->bbox.min[1];
				global_box.min[2] = themesh->bbox.min[2];

				global_box.max[0] = themesh->bbox.max[0];
				global_box.max[1] = themesh->bbox.max[1];
				global_box.max[2] = themesh->bbox.max[2];
			}
			else
			{
				global_box.min[0] = min(global_box.min[0], themesh->bbox.min[0]);
				global_box.min[1] = min(global_box.min[1], themesh->bbox.min[1]);
				global_box.min[2] = min(global_box.min[2], themesh->bbox.min[2]);

				global_box.max[0] = max(global_box.max[0], themesh->bbox.max[0]);
				global_box.max[1] = max(global_box.max[1], themesh->bbox.max[1]);
				global_box.max[2] = max(global_box.max[2], themesh->bbox.max[2]);
			}		
		}


		//create cube
		vec3 mesh_min = global_box.min;
		vec3 mesh_max = global_box.max;
		vec3 lengths = mesh_max - mesh_min;
		for (int i = 0; i<3;i++) {
			float delta = lengths.max() - lengths[i];
			if (delta != 0) {
				mesh_min[i] = mesh_min[i] - (delta / 2.0f);
				mesh_max[i] = mesh_max[i] + (delta / 2.0f);
			}
		}
		global_box = AABox<vec3>(mesh_min, mesh_max);




		for (int fileNumber = 0; fileNumber < nbOfFiles; fileNumber++)
		{
			stringstream ss;
			ss << std::setw(nbOfCharsInNumberAtTheEnd) << std::setfill('0') << fileNumber + 1;
			string current_ending_number = ss.str();
			string file_name_current_geometry_file = filename_without_numbers_and_extension + current_ending_number + file_extension;

			// Read mesh
			TriMesh *themesh = TriMesh::read(file_name_current_geometry_file.c_str());
			themesh->need_faces(); // unpack triangle strips so we have faces
			themesh->need_bbox(); // compute the bounding box
#ifndef BINARY_VOXELIZATION
			themesh->need_normals(); // check if there are normals, and if not, recompute them
									 // TODO: Check for colors here, inform user about decision
#endif


			//AABox<vec3> mesh_bbox = createMeshBBCube(themesh); // pad the mesh BBOX out to be a cube

															   // Moving mesh to origin
			cout << "Moving mesh to origin ... ";
			Timer timer = Timer();
			for (size_t i = 0; i < themesh->vertices.size(); i++) {
				themesh->vertices[i] = themesh->vertices[i] - global_box.min;
			}
			cout << "done in " << timer.elapsed_time_milliseconds << " s." << endl;

			// Write mesh to format we can stream in
			string base = file_name_current_geometry_file.substr(0, file_name_current_geometry_file.find_last_of("."));
			std::string tri_header_out_name = base + string(".tri");
			std::string tri_out_name = base + string(".tridata");

			FILE* tri_out = fopen(tri_out_name.c_str(), "wb");

			cout << "Writing mesh triangles ... "; timer.reset();
			Triangle t = Triangle();
			// Write all triangles to data file
			for (size_t i = 0; i < themesh->faces.size(); i++) {
				t.v0 = themesh->vertices[themesh->faces[i][0]];
				t.v1 = themesh->vertices[themesh->faces[i][1]];
				t.v2 = themesh->vertices[themesh->faces[i][2]];
#ifndef BINARY_VOXELIZATION
				// COLLECT VERTEX COLORS
				if (!themesh->colors.empty()) { // if this mesh has colors, we're going to use them
					t.v0_color = themesh->colors[themesh->faces[i][0]];
					t.v1_color = themesh->colors[themesh->faces[i][1]];
					t.v2_color = themesh->colors[themesh->faces[i][2]];
				}
				// COLLECT NORMALS
				if (recompute_normals) {
					t.normal = computeFaceNormal(themesh, i); // recompute normals
				}
				else {
					t.normal = getShadingFaceNormal(themesh, i); // use mesh provided normals
				}
#endif
				writeTriangle(tri_out, t);
			}
			cout << "done in " << timer.elapsed_time_milliseconds << " ms." << endl;

			// Prepare tri_info and write header
			cout << "Writing header to " << tri_header_out_name << " ... " << endl;
			TriInfo tri_info;
			tri_info.version = 1;
			tri_info.mesh_bbox = global_box;
			tri_info.n_triangles = themesh->faces.size();
#ifdef BINARY_VOXELIZATION
			tri_info.geometry_only = 1;
#else
			tri_info.geometry_only = 0;
#endif
			writeTriHeader(tri_header_out_name, tri_info);
			tri_info.print();
			cout << "Done." << endl;
		}
	}
}