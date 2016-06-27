#ifndef TREE4DBUILDER_STRATEGY
#define TREE4DBUILDER_STRATEGY
#include <memory>
#include "Tree4DBuilderDifferentSides_Interface.h"

class Tree4DBuilder_Strategy
{
	std::unique_ptr<Tree4DBuilderDifferentSides_Interface> builder_ptr_;


public:
	Tree4DBuilder_Strategy();
	Tree4DBuilder_Strategy(std::string base_filename, size_t gridsize_S, size_t gridsize_T, bool generate_levels);


	void addVoxel(const uint64_t morton_number) const;

	void addVoxel(const VoxelData& point) const;

	void initializeBuilder() const;

	void finalizeTree() const;
};
#endif
