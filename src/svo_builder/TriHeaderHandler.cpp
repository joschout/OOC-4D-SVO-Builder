 #include "TriHeaderHandler.h"
#include <ctype.h>
#include <iomanip>
#include "globals.h"

TriHeaderHandler::TriHeaderHandler(std::string base_filename, float start_time, float end_time, bool multiple_input_files, size_t gridsize_T, bool verbose):
	verbose(verbose), multiple_input_files(multiple_input_files), base_filename(base_filename), gridsize_T(gridsize_T), start_time(start_time), end_time(end_time)
{
}

TriInfo4D_multiple_files TriHeaderHandler::readHeaders()
{
	//check how many tri files we need to parse
	if(multiple_input_files)
	{
		return readHeaders_multiple_input_files();
	}else //only one input file
	{
		TriInfo tri_info = readHeaders_one_input_file(base_filename);

		if (data_out) {
			partitioning_io_input_timer.stop();		// TIMING
			partitioning_total_timer.stop();		// TIMING
			data_writer_ptr->writeToFile_endl("total number of triangles: " + to_string(tri_info.n_triangles));
			partitioning_total_timer.start();		// TIMING
			partitioning_io_input_timer.start();	// TIMING	
		}

		TriInfo4D_multiple_files tri_info4_d_multiple_files = TriInfo4D_multiple_files();
		tri_info4_d_multiple_files.addTriInfo(tri_info);
		return tri_info4_d_multiple_files;
	}	
}

int TriHeaderHandler::getBaseFileNameWithoutNumbersAtTheEnd(string original_filename, string& filename_without_numbers_and_extension)
{
	// e.g. translating_box_000001.tri ==> translating_box_000001
	string base_filename_with_number = original_filename.substr(0, original_filename.find_last_of("."));

	//count the number of digits at the end of the string
	int numberOfDigitsAtTheEnd = 0;
	while (numberOfDigitsAtTheEnd < base_filename_with_number.length())
	{
		char s_char = base_filename_with_number[base_filename_with_number.length() - 1 - numberOfDigitsAtTheEnd];
		if (!isdigit(s_char)) //if the char is not a number, break
		{
			break;
		}
		numberOfDigitsAtTheEnd++;
	}


	string base_filename_without_number = string(base_filename_with_number);
	int startingPositionOfNumber = base_filename_with_number.length() - numberOfDigitsAtTheEnd;
	base_filename_without_number.erase(startingPositionOfNumber, numberOfDigitsAtTheEnd);

	filename_without_numbers_and_extension = base_filename_without_number;

	string numberAtTheEnd = base_filename_with_number.substr(startingPositionOfNumber, numberOfDigitsAtTheEnd);
	int nbOfCharsInNumberAtTheEnd = numberAtTheEnd.length();

	return nbOfCharsInNumberAtTheEnd;
}

TriInfo4D_multiple_files TriHeaderHandler::readHeaders_multiple_input_files()
{
	/*
	for each tri file
	1. create a TriInfo object;
	2. parse the tri header file
	*/

	string base_filename_without_number;
	int nbOfCharsInNumberAtTheEnd = getBaseFileNameWithoutNumbersAtTheEnd(base_filename, base_filename_without_number);


//	// e.g. translating_box_000001.tri ==> translating_box_000001
//	string base_filename_with_number = base_filename.substr(0, base_filename.find_last_of("."));
//
//	//count the number of digits at the end of the string
//	int numberOfDigitsAtTheEnd = 0;
//	while (numberOfDigitsAtTheEnd < base_filename_with_number.length())
//	{
//		char s_char = base_filename_with_number[base_filename_with_number.length() - 1 - numberOfDigitsAtTheEnd];
//		if(!isdigit(s_char)) //if the char is not a number, break
//		{
//			break;
//		}
//		numberOfDigitsAtTheEnd++;
//	}
//
//
//	string base_filename_without_number = string(base_filename_with_number);
//	int startingPositionOfNumber = base_filename_with_number.length() - numberOfDigitsAtTheEnd;
//	base_filename_without_number.erase(startingPositionOfNumber, numberOfDigitsAtTheEnd);
//
//	string numberAtTheEnd = base_filename_with_number.substr(startingPositionOfNumber, numberOfDigitsAtTheEnd);
//	int nbOfCharsInNumberAtTheEnd = numberAtTheEnd.length();

	int nbOfTriFiles = gridsize_T;
	
	TriInfo4D_multiple_files tri_info4_d_multiple_files = TriInfo4D_multiple_files(base_filename_without_number, start_time, end_time);

	for (int i = 0; i < nbOfTriFiles; i++)
	{
		//create the file name
		stringstream ss;
		ss << std::setw(nbOfCharsInNumberAtTheEnd) << std::setfill('0') << i + 1;
		string current_ending_number = ss.str();
		string file_name_current_tri_file = base_filename_without_number + current_ending_number + ".tri" ;

		TriInfo tri_info = readHeaders_one_input_file(file_name_current_tri_file);
		if(data_out)
		{
			partitioning_io_input_timer.stop();		// TIMING
			partitioning_total_timer.stop();		// TIMING
			string output = "tri file " + to_string(i + 1) + " - number of triangles: " + to_string(tri_info.n_triangles);
			data_writer_ptr->writeToFile_endl(output);
			partitioning_total_timer.start();		// TIMING
			partitioning_io_input_timer.start();	// TIMING			
		}

		tri_info4_d_multiple_files.addTriInfo(tri_info);
	}

	if(data_out){
		partitioning_io_input_timer.stop();		// TIMING
		partitioning_total_timer.stop();		// TIMING
		data_writer_ptr->writeToFile_endl("total number of triangles: " + to_string(tri_info4_d_multiple_files.getNbfOfTriangles()));
		partitioning_total_timer.start();		// TIMING
		partitioning_io_input_timer.start();	// TIMING	
	}

	return tri_info4_d_multiple_files;

}

TriInfo TriHeaderHandler::readHeaders_one_input_file(string &filename)
{
	TriInfo tri_info = TriInfo();

	if(verbose)
	{
		std::cout << "Parsing tri header " << filename << " ..." << std::endl;
	}
	if (parseTri3DHeader(filename, tri_info) != 1) {
		cout << "Something went wrong when parsing the header" << endl;
		std::cout << "Press ENTER to exit...";
		cin.get();
		exit(0); // something went wrong in parsing the header - exiting.
	}
	// disabled for benchmarking
	if (!tri_info.filesExist()) {
		cout << "Not all required .tri or .tridata files exist. Please regenerate using tri_convert." << endl;
		std::cout << "Press ENTER to exit...";
		cin.get();
		exit(0); // not all required files exist - exiting.
	}
	if (verbose) { tri_info.print(); }
	// Check if the user is using the correct executable for type of tri file
#ifdef BINARY_VOXELIZATION
	if (!tri_info.geometry_only) {
		cout << "You're using a .tri file which contains more than just geometry with a geometry-only SVO Builder! Regenerate that .tri file using tri_convert_binary." << endl;
		std::cout << "Press ENTER to exit...";
		cin.get();
		exit(0);
	}
#else
	if (tri_info.geometry_only) {
		cout << "You're using a .tri file which contains only geometry with the regular SVO Builder! Regenerate that .tri file using tri_convert." << endl;
		std::cout << "Press ENTER to exit...";
		cin.get();
		exit(0);
	}
#endif
	return tri_info;
}

int TriHeaderHandler::parseTri3DHeader(std::string filename, TriInfo& t) const
{
	ifstream file;
	file.open(filename.c_str(), ios::in);

	if (file.is_open()) {
		if(verbose)
		{
		cout << filename + " should be correctly opened" << std::endl;			
		}

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

