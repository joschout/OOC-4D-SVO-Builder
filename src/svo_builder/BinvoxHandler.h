#ifndef BINVOXHANDLER_H
#define BINVOXHANDLER_H
#include <vector>
#include "BinvoxWriter.h"


/*
Binvox uses RUN_LENGTH ENCODING
https://en.wikipedia.org/wiki/Run-length_encoding
http://rosettacode.org/wiki/Run-length_encoding#C.2B.2B
*/
class BinvoxHandler
{
public:
	size_t gridsize_T;
	std::vector<BinvoxWriter> binvox_writers;
	std::string base_filename;
	
	BinvoxHandler();
	BinvoxHandler(std::string base_filename, size_t gridsize_T);
	void initialize(vec3 translation_vec, float scale);
	void createInitialBinvoxFiles();
	void writeHeaders();
	void writeVoxel(int timepoint, int x, int y, int z);
	void sparsifyFiles();
	void closeWriters();
};


#endif