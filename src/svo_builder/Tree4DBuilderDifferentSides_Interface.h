#ifndef TREE4DBUILDERDIFFERENTSIDES_INTERFACE_H
#define TREE4DBUILDERDIFFERENTSIDES_INTERFACE_H

#include "VoxelData.h"

class Tree4DBuilderDifferentSides_Interface
{



public:
	virtual ~Tree4DBuilderDifferentSides_Interface()
	{
	}

	virtual void addVoxel(const uint64_t morton_number) = 0;
	virtual void addVoxel(const VoxelData& point) = 0;

	virtual void initializeBuilder() = 0;
	virtual void finalizeTree() = 0;

};


#endif