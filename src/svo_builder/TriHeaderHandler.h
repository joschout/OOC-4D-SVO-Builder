#ifndef TRIHEADERHANDLER_H
#define TRIHEADERHANDLER_H
#include <string>
#include "TriInfo4D_multiple_files.h"

class TriHeaderHandler
{
public:
	bool verbose = false;
	bool multiple_input_files = false;
	std::string base_filename;
	size_t gridsize_T;

	float start_time;
	float end_time;

	TriHeaderHandler(std::string base_filename, float start_time, float end_time, bool multiple_input_files, size_t gridsize_T, bool verbose);

	TriInfo4D_multiple_files readHeaders();


private:
	TriInfo4D_multiple_files readHeaders_multiple_input_files();
	TriInfo readHeaders_one_input_file(string &filename);

	int parseTri3DHeader(std::string filename, TriInfo& t) const;
};


#endif
