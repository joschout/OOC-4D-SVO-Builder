#ifndef TREE4D_BUILDER_H_
#define TREE4D_BUILDER_H_

#include <stdio.h>
#include <fstream>
#include "../libs/libtri/include/tri_util.h"
#include "svo_builder_util.h"
#include "Node4D.h"

using namespace std;
using namespace trimesh;

// Octreebuilder class. You pass this class DataPoints, it builds an octree from them.
class Tree4DBuilder {
public:
	vector< vector< Node4D > > b_buffers;
	size_t gridsize_S;
	size_t gridsize_T;
	int b_maxdepth; // maximum octree depth
	uint64_t b_current_morton; // current morton position
	uint64_t b_max_morton; // maximum morton position
	size_t b_data_pos; // current output data position (array index)
	size_t b_node_pos; // current output node position (array index)

					   // configuration
	bool generate_levels; // switch to enable basic generation of higher octree levels

	FILE* node_out; // pointer to the node file
	FILE* data_out; // pointer to the payload data file
	string base_filename;

	Tree4DBuilder();
	Tree4DBuilder(std::string base_filename, size_t gridsize_S, size_t gridsize_T, bool generate_levels);
	void finalizeTree();
	void addVoxel(const uint64_t morton_number);
	void addVoxel(const VoxelData& point);

private:
	// helper methods for octree building
	void fastAddEmpty(const size_t budget);
	void addEmptyVoxel(const int buffer);
	bool isBufferEmpty(const vector<Node4D> &buffer);
	void refineBuffers(const int start_depth);
	Node4D groupNodes(const vector<Node4D> &buffer);
	int highestNonEmptyBuffer();
	int computeBestFillBuffer(const size_t budget);
};

// Check if a buffer contains non-empty nodes
inline bool Tree4DBuilder::isBufferEmpty(const vector<Node4D> &buffer) {
	for (int k = 0; k<16; k++) {
		if (!buffer[k].isNull()) {
			return false;
		}
	}
	return true;
}

// Find the highest non empty buffer, return its index
inline int Tree4DBuilder::highestNonEmptyBuffer() {
	int highest_found = b_maxdepth; // highest means "lower in buffer id" here.
	for (int k = b_maxdepth; k >= 0; k--) {
		if (b_buffers[k].size() == 0) { // this buffer level is empty
			highest_found--;
		}
		else { // this buffer level is nonempty: break
			return highest_found;
		}
	}
	return highest_found;
}

// Compute the best fill buffer given the budget
inline int Tree4DBuilder::computeBestFillBuffer(const size_t budget) {
	// which power of 16 fits in budget?
	int budget_buffer_suggestion = b_maxdepth - findPowerOf16(budget);
	// if our current guess is already b_maxdepth, return that, no need to test further
	if (budget_buffer_suggestion == b_maxdepth) { return b_maxdepth; }
	// best fill buffer is maximum of suggestion and highest non_empty buffer
	return max(budget_buffer_suggestion, highestNonEmptyBuffer());
}

// A method to quickly add empty nodes
inline void Tree4DBuilder::fastAddEmpty(const size_t budget) {
	size_t r_budget = budget;
	while (r_budget > 0) {
		unsigned int buffer = computeBestFillBuffer(r_budget);
		addEmptyVoxel(buffer);
		size_t budget_hit = (size_t)pow(16.0, b_maxdepth - buffer);
		r_budget = r_budget - budget_hit;
	}
}

#endif // OCTREE_BUILDER_H_