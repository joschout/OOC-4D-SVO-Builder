
#ifndef TREEDATAWRITERCPPSTYLE_H
#define TREEDATAWRITERCPPSTYLE_H

#include "VoxelData.h"

class TreeDataWriterCppStyle
{
public:
	size_t position_in_output_file; //NOTE: NOT in bytes, but in nodes
									//i.e. this is the nb of the node we will write next

									//TreeNodeWriterCppStyle();
	TreeDataWriterCppStyle(std::string base_filename);
	~TreeDataWriterCppStyle();
	void closeFile();
	size_t writeVoxelData(const VoxelData &voxelData);

private:
	std::fstream fstream_data;

};

#endif