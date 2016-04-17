#ifndef TREEDATAWRITER_H
#define TREEDATAWRITER_H

#include "VoxelData.h"

class TreeDataWriter
{
public:
	FILE* file_pointer_data;
	size_t position_in_output_file;

	TreeDataWriter();
	TreeDataWriter(std::string base_filename);
	~TreeDataWriter();
	void closeFile();
	size_t writeVoxelData(const VoxelData &voxelData);

private:
	bool hasBeenClosed;
};
#endif
