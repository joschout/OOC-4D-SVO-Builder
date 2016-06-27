#include "Tree4DBuilder_Strategy.h"
#include "Tree4DBuilderDifferentSides_Space_longest.h"
#include "Tree4DBuilderDifferentSides_Time_longest.h"

Tree4DBuilder_Strategy::Tree4DBuilder_Strategy()
{
}

Tree4DBuilder_Strategy::Tree4DBuilder_Strategy(std::string base_filename, size_t gridsize_S, size_t gridsize_T, bool generate_levels)
{

	/*
	if(gridsize_S > gridsize_T)
	{
		longestDimension = SPACE;
	}else
	{
		longestDimension = TIME;
	}
	*/
	if(gridsize_S > gridsize_T)
	{
		std::unique_ptr<Tree4DBuilderDifferentSides_Space_longest> pointer(new Tree4DBuilderDifferentSides_Space_longest(
			base_filename,
			gridsize_S,
			gridsize_T,
			generate_levels));
		builder_ptr_ = std::move(pointer);
	}else//gridsize_S <= gridsize_T
	{
		std::unique_ptr<Tree4DBuilderDifferentSides_Time_longest> pointer(new Tree4DBuilderDifferentSides_Time_longest(
			base_filename,
			gridsize_S,
			gridsize_T,
			generate_levels));
		builder_ptr_ = std::move(pointer);
	}

}

void Tree4DBuilder_Strategy::addVoxel(const uint64_t morton_number) const
{
	builder_ptr_->addVoxel(morton_number);
}

void Tree4DBuilder_Strategy::addVoxel(const VoxelData& point) const
{
	builder_ptr_->addVoxel(point);
}

void Tree4DBuilder_Strategy::initializeBuilder() const
{
	builder_ptr_->initializeBuilder();
}

void Tree4DBuilder_Strategy::finalizeTree() const
{
	builder_ptr_->finalizeTree();
}