#include "Tree4DBuilderDifferentSides.h"
#include "morton4D.h"
#include <cassert>
#include "tree4d_io.h"
#include "svo_builder_util.h"
#include <queue>

Tree4DBuilderDifferentSides::Tree4DBuilderDifferentSides(): 
	gridsize_S(0), gridsize_T(0),
	current_morton_code(0), max_morton_code(0),
	maxDepth(0), totalNbOfQueues(0), nbOfQueuesOf16Nodes(0), nbOfQueuesOf2Nodes(0),
	generate_levels(false),
	nodeWriter(nullptr), dataWriter(nullptr)
{
}

Tree4DBuilderDifferentSides::Tree4DBuilderDifferentSides(
	std::string base_filename, size_t gridsize_S, size_t gridsize_T):
	base_filename(base_filename), gridsize_S(gridsize_S), gridsize_T(gridsize_T)
{
	nodeWriter = TreeNodeWriter(base_filename);
	dataWriter = TreeDataWriter(base_filename);

	initializeBuilder();
}

void Tree4DBuilderDifferentSides::calculateMaxMortonCode()
{
	max_morton_code
		= morton4D_Encode_for<uint64_t, uint_fast32_t>(
			static_cast<uint_fast32_t>(gridsize_S) - 1, static_cast<uint_fast32_t>(gridsize_S) - 1, static_cast<uint_fast32_t>(gridsize_S) - 1, static_cast<uint_fast32_t>(gridsize_T) - 1,
			static_cast<uint_fast32_t>(gridsize_S), static_cast<uint_fast32_t>(gridsize_S), static_cast<uint_fast32_t>(gridsize_S), static_cast<uint_fast32_t>(gridsize_T));
}

void Tree4DBuilderDifferentSides::initializeBuilder()
{
	/*
	gridsize_t = 2^y
	gridsize_s = 2^x  with y > x

	#vox = 2^(3x+y)

	#queuesOf16Nodes = log2(gridsize_s) = y	
	#
	*/


	// Setup building variables
	maxDepth = log2(static_cast<unsigned int>(max(gridsize_S, gridsize_T)));
	totalNbOfQueues = maxDepth + 1;


	nbOfQueuesOf16Nodes = log2(static_cast<unsigned int>(gridsize_S));
	nbOfQueuesOf2Nodes = totalNbOfQueues - nbOfQueuesOf16Nodes;

	queuesOfMax2.resize(nbOfQueuesOf2Nodes);
	queuesOfMax16.resize(nbOfQueuesOf16Nodes);

	//nodeQueues.resize(maxDepth + 1);
	for (int i = 0; i < nbOfQueuesOf2Nodes; i++)
	{
		queuesOfMax2[i].reserve(2);
	}
	for (int i = 0; i < nbOfQueuesOf16Nodes; i++)
	{
		queuesOfMax16[i].reserve(16);
	}

	// Fill data arrays
	calculateMaxMortonCode();

	dataWriter.writeVoxelData(VoxelData());// first data point is NULL

#ifdef BINARY_VOXELIZATION
	VoxelData voxelData = VoxelData(0, vec3(), vec3(1.0, 1.0, 1.0)); // We store a simple white voxel in case of Binary voxelization
	dataWriter.writeVoxelData(voxelData);  // all leafs will refer to this
#endif
}

// Add a datapoint to the octree: this is the main method used to push datapoints
void Tree4DBuilderDifferentSides::addVoxel(const uint64_t morton_number) {
	// Padding for missed morton numbers, i.e. the empty voxels with morton codes between [current_morton_code, morton number)
	if (morton_number != current_morton_code) {
		fastAddEmpty(morton_number - current_morton_code);
	}

	// Create a new leaf node
	Node4D node = Node4D(); // create empty node
	node.data = 1; // all nodes in binary voxelization refer to this
				   

	//add the new leaf node to the lowest queue
	queuesOfMax16.at(nbOfQueuesOf16Nodes - 1).push_back(node);
	//nodeQueues.at(maxDepth).push_back(node);
	// flush the queues
	flushQueues(maxDepth);

	current_morton_code++;
}

// Add a datapoint to the octree: this is the main method used to push datapoints
void Tree4DBuilderDifferentSides::addVoxel(const VoxelData& data) {
	// Padding for missed morton numbers
	if (data.morton != current_morton_code) {
		fastAddEmpty(data.morton - current_morton_code);
	}

	// Create node
	Node4D node = Node4D(); // create empty node
		
	// Write data point
	node.data = dataWriter.writeVoxelData(data); // store data

	node.data_cache = data; // store data as cache
							// Add to buffers
	queuesOfMax16.at(nbOfQueuesOf16Nodes - 1).push_back(node);;
	// flush the queues
	flushQueues(maxDepth);

	current_morton_code++;
}

void Tree4DBuilderDifferentSides::push_backNodeToQueueAtDepth(int depth, Node4D node)
{
	assert(depth >= 0);
	assert(depth <= maxDepth);

	if(depth <= nbOfQueuesOf2Nodes - 1) 
		//depth in [0, nbOfQueuesOf2nodes - 1]
		//index in queuesOfMax2 : [0, nbOfQueuesOf2nodes - 1]
	{
		queuesOfMax2[depth].push_back(node);
	}else 
		//depth in [nbOfQueuesOf2nodes, totalNbOfQueues - 1]    (remember: maxDepth = totalNbOfQueues - 1
		//index in queuesOfMax16 : [0, nbOfQueuesOf16nodes - 1]
	{
		queuesOfMax16[depth - nbOfQueuesOf2Nodes].push_back(node);
	}
}

bool Tree4DBuilderDifferentSides::isQueueFilled(int depth)
{
	assert(depth >= 0);
	assert(depth <= maxDepth);

	if (depth <= nbOfQueuesOf2Nodes - 1) {
		//depth in [0, nbOfQueuesOf2nodes - 1]
		//index in queuesOfMax2 : [0, nbOfQueuesOf2nodes - 1]
		if(queuesOfMax2[depth].size() == 2)
		{
			return true;
		} else {
			return false;
		}
	} else {
		//depth in [nbOfQueuesOf2nodes, totalNbOfQueues - 1]    (remember: maxDepth = totalNbOfQueues - 1
		//index in queuesOfMax16 : [0, nbOfQueuesOf16nodes - 1]

		if(queuesOfMax16[depth - nbOfQueuesOf2Nodes].size() == 16)
		{
			return true;
		} else {
			return false;
		}	
	}
}

bool Tree4DBuilderDifferentSides::isQueueEmpty(int depth)
{
	assert(depth >= 0);
	assert(depth <= maxDepth);

	if (depth <= nbOfQueuesOf2Nodes - 1) {
		//depth in [0, nbOfQueuesOf2nodes - 1]
		//index in queuesOfMax2 : [0, nbOfQueuesOf2nodes - 1]
		if (queuesOfMax2[depth].size() == 0)
		{
			return true;
		}
		else {
			return false;
		}
	}
	else {
		//depth in [nbOfQueuesOf2nodes, totalNbOfQueues - 1]    (remember: maxDepth = totalNbOfQueues - 1
		//index in queuesOfMax16 : [0, nbOfQueuesOf16nodes - 1]

		if (queuesOfMax16[depth - nbOfQueuesOf2Nodes].size() == 0)
		{
			return true;
		}
		else {
			return false;
		}
	}
}

bool Tree4DBuilderDifferentSides::doesQueueContainOnlyEmptyNodes(int depth) {
	assert(depth >= 0);
	assert(depth <= maxDepth);

	if (depth <= nbOfQueuesOf2Nodes - 1)
		//depth in [0, nbOfQueuesOf2nodes - 1]
		//index in queuesOfMax2 : [0, nbOfQueuesOf2nodes - 1]
	{
		return doesQueueContainOnlyEmptyNodes(queuesOfMax2[depth], 2);
	}
	else
		//depth in [nbOfQueuesOf2nodes, totalNbOfQueues - 1]    (remember: maxDepth = totalNbOfQueues - 1
		//index in queuesOfMax16 : [0, nbOfQueuesOf16nodes - 1]
	{
		return doesQueueContainOnlyEmptyNodes(queuesOfMax16[depth - nbOfQueuesOf2Nodes], 16);
	}
}

// REFINE QUEUES: check all levels from start_depth up and group 16 nodes on a higher level
void Tree4DBuilderDifferentSides::flushQueues(const int start_depth) {
	for (int depth = start_depth; depth >= 0; depth--) {
		if(isQueueFilled(depth)) { // we have the max amount of nodes in the queue at depth depth
			assert(depth - 1 >= 0);
			if(doesQueueContainOnlyEmptyNodes(depth)){
				push_backNodeToQueueAtDepth(depth - 1, Node4D()); // push back NULL node to represent the 16 (or 2) empty nodes
			}
			else{
				push_backNodeToQueueAtDepth(depth - 1, groupNodesAtDepth(depth)); // push back parent node
			}
			clearQueueAtDepth(depth); // clear the 16 nodes on this level
		}
		else {
			break; // break the for loop: no upper levels will need changing
		}
	}
}

// Check if a buffer contains non-empty nodes
bool Tree4DBuilderDifferentSides::doesQueueContainOnlyEmptyNodes(const QueueOfNodes &queue, int maxAmountOfElementsInQueue) {
	for (int k = 0; k < maxAmountOfElementsInQueue; k++) {
		if (!queue[k].isNull()) {
			return false;
		}
	}
	return true;
}

Node4D Tree4DBuilderDifferentSides::groupNodesAtDepth(int depth)
{
	assert(depth >= 0);
	assert(depth <= maxDepth);

	if (depth <= nbOfQueuesOf2Nodes - 1)
		//depth in [0, nbOfQueuesOf2nodes - 1]
		//index in queuesOfMax2 : [0, nbOfQueuesOf2nodes - 1]
	{
		return groupNodesOfMax2(queuesOfMax2[depth]);
	}
	else
		//depth in [nbOfQueuesOf2nodes, totalNbOfQueues - 1]    (remember: maxDepth = totalNbOfQueues - 1
		//index in queuesOfMax16 : [0, nbOfQueuesOf16nodes - 1]
	{
		return groupNodesOfMax16(queuesOfMax16[depth - nbOfQueuesOf2Nodes]);
	}
}

Node4D Tree4DBuilderDifferentSides::groupNodesOfMax2(const QueueOfNodes &queueOfMax2)
{
	Node4D parent = Node4D();
	bool first_stored_child = true;

	for (int indexOfCurrentChildNode = 0; indexOfCurrentChildNode < 2; indexOfCurrentChildNode++) {
		Node4D currentChildNode = queueOfMax2[indexOfCurrentChildNode];
		if (!currentChildNode.isNull()) {
			writeNodeToDiskAndSetOffsetOfParent_Max2NodesInQueue(parent, first_stored_child, indexOfCurrentChildNode, currentChildNode);
		}
		else {
			setChildrenOffsetsForNodeWithMax2Children(parent, indexOfCurrentChildNode, NOCHILD);
		}
	}

	// SIMPLE LEVEL CONSTRUCTION
	if (generate_levels) {
		VoxelData voxelData = VoxelData();
		float notnull = 0.0f;
		for (int i = 0; i < 2; i++) { // this node has no data: need to refine
			if (!queueOfMax2[i].isNull())
				notnull++;
			voxelData.color += queueOfMax2[i].data_cache.color;
			voxelData.normal += queueOfMax2[i].data_cache.normal;
		}
		voxelData.color = voxelData.color / notnull;
		vec3 tonormalize = (vec3)(voxelData.normal / notnull);
		voxelData.normal = normalize(tonormalize);
		// set it in the parent node
		parent.data = dataWriter.writeVoxelData(voxelData);
		parent.data_cache = voxelData;
	}
	return parent;
}

Node4D Tree4DBuilderDifferentSides::groupNodesOfMax16(const QueueOfNodes &queueOfMax16)
{
	Node4D parent = Node4D();
	bool first_stored_child = true;

	for (int indexOfCurrentChildNode = 0; indexOfCurrentChildNode < 16; indexOfCurrentChildNode++) {
		Node4D currentChildNode = queueOfMax16[indexOfCurrentChildNode];
		if (!currentChildNode.isNull()) {
			writeNodeToDiskAndSetOffsetOfParent_Max16NodesInQueue(parent, first_stored_child, indexOfCurrentChildNode, currentChildNode);
		}
		else {
			parent.children_offset[indexOfCurrentChildNode] = NOCHILD;
		}
	}

	// SIMPLE LEVEL CONSTRUCTION
	if (generate_levels) {
		VoxelData voxelData = VoxelData();
		float notnull = 0.0f;
		for (int i = 0; i < 16; i++) { // this node has no data: need to refine
			if (!queueOfMax16[i].isNull())
				notnull++;
			voxelData.color += queueOfMax16[i].data_cache.color;
			voxelData.normal += queueOfMax16[i].data_cache.normal;
		}
		voxelData.color = voxelData.color / notnull;
		vec3 tonormalize = (vec3)(voxelData.normal / notnull);
		voxelData.normal = normalize(tonormalize);
		// set it in the parent node
		parent.data = dataWriter.writeVoxelData(voxelData);
		parent.data_cache = voxelData;
	}
	return parent;
}

void Tree4DBuilderDifferentSides::writeNodeToDiskAndSetOffsetOfParent_Max2NodesInQueue(Node4D& parent, bool& first_stored_child, int indexOfCurrentChildNode, Node4D currentChildNode)
{
	//store the node on disk
	size_t positionOfChildOnDisk = nodeWriter.writeNode4D_(currentChildNode);

	if (first_stored_child) {
		parent.children_base = positionOfChildOnDisk;
		setChildrenOffsetsForNodeWithMax2Children(parent, indexOfCurrentChildNode, 0);
		first_stored_child = false;	
	}
	else { //NOTE: we can have only two nodes, we know this is the second node
		char offset = (char)(positionOfChildOnDisk - parent.children_base);
		setChildrenOffsetsForNodeWithMax2Children(parent, indexOfCurrentChildNode, offset);	
	}
}

void Tree4DBuilderDifferentSides::writeNodeToDiskAndSetOffsetOfParent_Max16NodesInQueue(Node4D& parent, bool& first_stored_child, int indexOfCurrentChildNode, Node4D currentChildNode)
{
	//store the node on disk
	size_t positionOfChildOnDisk = nodeWriter.writeNode4D_(currentChildNode);
	if (first_stored_child) {
		parent.children_base = positionOfChildOnDisk;
		parent.children_offset[indexOfCurrentChildNode] = 0;
		first_stored_child = false;	
	}
	else { //NOTE: when we can have only two nodes, we know this is the second node
		//store the current child node
		char offset = (char)(positionOfChildOnDisk - parent.children_base);
		parent.children_offset[indexOfCurrentChildNode] = offset;
	}
}

/*
// Group 16 nodes, write non-empty nodes to disk and create parent node
Node4D Tree4DBuilderDifferentSides::groupNodes(const QueueOfNodes &queue, int maxAmountOfElementsInQueue) {
	Node4D parent = Node4D();
	bool first_stored_child = true;

	for (int k = 0; k < maxAmountOfElementsInQueue; k++) {
		if (!queue[k].isNull()) {
			if (first_stored_child) {

				//store the first child on disk
				size_t positionOfFirstStoredChildInFile = nodeWriter.writeNode4D_(queue[k]);

				parent.children_base = positionOfFirstStoredChildInFile;
				
				first_stored_child = false;

				//TODO remove ugly hardcoding
				if(maxAmountOfElementsInQueue == 2)
				{
					setChildrenOffsetsForNodeWithMax2Children(parent, k, 0);
				}else// 16 elements in the queue
				{
					parent.children_offset[k] = 0;
				}
			}
			else { //NOTE: when we can have only two nodes, we know this is the second node
				//store the current child node
				size_t positionOfChildOnDisk = nodeWriter.writeNode4D_(queue[k]);

				char offset = (char)(positionOfChildOnDisk - parent.children_base);
			
				if (maxAmountOfElementsInQueue == 2)
				{
					setChildrenOffsetsForNodeWithMax2Children(parent, k, offset);
				}else // 16 elements in the queue
				{
					parent.children_offset[k] = offset;
				}
			}
		}
		else {
			if (maxAmountOfElementsInQueue == 2)
			{
				setChildrenOffsetsForNodeWithMax2Children(parent, k, NOCHILD);
			}
			else // 16 elements in the queue
			{
				parent.children_offset[k] = NOCHILD;
			}
		}
	}

	// SIMPLE LEVEL CONSTRUCTION
	if (generate_levels) {
		VoxelData voxelData = VoxelData();
		float notnull = 0.0f;
		for (int i = 0; i < maxAmountOfElementsInQueue; i++) { // this node has no data: need to refine
			if (!queue[i].isNull())
				notnull++;
			voxelData.color += queue[i].data_cache.color;
			voxelData.normal += queue[i].data_cache.normal;
		}
		voxelData.color = voxelData.color / notnull;
		vec3 tonormalize = (vec3)(voxelData.normal / notnull);
		voxelData.normal = normalize(tonormalize);
		// set it in the parent node
		parent.data = dataWriter.writeVoxelData(voxelData);
		parent.data_cache = voxelData;
	}

	return parent;
}
*/

void Tree4DBuilderDifferentSides::setChildrenOffsetsForNodeWithMax2Children(Node4D &parent, int indexInQueue, char offset)
{
	if (indexInQueue == 0)
	{
		for (int i = 0; i < 8; i++) {
			parent.children_offset[i] = offset;
		}
	}
	else { // indexInQueue == 1
		for (int i = 8; i < 16; i++) {
			parent.children_offset[i] = offset;
		}
	}
}

void Tree4DBuilderDifferentSides::clearQueueAtDepth(int depth)
{
	assert(depth >= 0);
	assert(depth <= maxDepth);

	if (depth <= nbOfQueuesOf2Nodes - 1)
		//depth in [0, nbOfQueuesOf2nodes - 1]
		//index in queuesOfMax2 : [0, nbOfQueuesOf2nodes - 1]
	{
		queuesOfMax2[depth].clear();
	}
	else
		//depth in [nbOfQueuesOf2nodes, totalNbOfQueues - 1]    (remember: maxDepth = totalNbOfQueues - 1
		//index in queuesOfMax16 : [0, nbOfQueuesOf16nodes - 1]
	{
		queuesOfMax16[depth - nbOfQueuesOf2Nodes].clear();
	}
}

// Finalize the tree: add rest of empty nodes, make sure root node is on top
void Tree4DBuilderDifferentSides::finalizeTree() {
	// fill octree
	if (current_morton_code < max_morton_code) {
		fastAddEmpty((max_morton_code - current_morton_code) + 1);
	}

	// write root node
	nodeWriter.writeNode4D_(getRootNode());
	
	// write header
	Tree4DInfo tree4D_info(1, base_filename, gridsize_S, gridsize_T, nodeWriter.position_in_output_file, dataWriter.position_in_output_file);
	writeOctreeHeader(base_filename + string(".tree4d"), tree4D_info);

	// close files
	dataWriter.closeFile();
	nodeWriter.closeFile();
}

Node4D Tree4DBuilderDifferentSides::getRootNode()
{
	if ( nbOfQueuesOf2Nodes > 0)
	{
		return queuesOfMax2[0][0];
	}
	else
	{
		return queuesOfMax16[0][0];
	}
}

// A method to quickly add empty nodes
inline void Tree4DBuilderDifferentSides::fastAddEmpty(const size_t budget) {
	size_t r_budget = budget;
	while (r_budget > 0) {
		unsigned int buffer = computeBestFillQueue(r_budget);
		addEmptyVoxel(buffer);
		size_t budget_hit = (size_t)pow(16.0, maxDepth - buffer);
		r_budget = r_budget - budget_hit;
	}
}

// Compute the best fill buffer given the budget
inline int Tree4DBuilderDifferentSides::computeBestFillQueue(const size_t budget) {
	// which power of 16 fits in budget?
	int budget_queue_suggestion = maxDepth - findPowerOf16(budget);
	// if our current guess is already b_maxdepth, return that, no need to test further
	if (budget_queue_suggestion == maxDepth)
	{
		return maxDepth;
	}
	// best fill buffer is maximum of suggestion and highest non_empty buffer
	return max(budget_queue_suggestion, highestNonEmptyQueue());
}

// Find the highest non empty buffer, return its index
inline int Tree4DBuilderDifferentSides::highestNonEmptyQueue() {
	int highest_found = maxDepth; // highest means "lower in buffer id" here.
	for (int currentDepth = maxDepth; currentDepth >= 0; currentDepth--) {
		if (isQueueEmpty(currentDepth)) { // this buffer level is empty
			highest_found--;
		}
		else { // this buffer level is nonempty: break
			return highest_found;
		}
	}
	return highest_found;
}

// Add an empty datapoint at a certain buffer level, and refine upwards from there
void Tree4DBuilderDifferentSides::addEmptyVoxel(const int queueDepth) {
	push_backNodeToQueueAtDepth(queueDepth, Node4D());
	//nodeQueues[buffer].push_back(Node4D());
	flushQueues(queueDepth);
	current_morton_code = (uint64_t)(current_morton_code + pow(16.0, maxDepth - queueDepth)); // because we're adding at a certain level
}