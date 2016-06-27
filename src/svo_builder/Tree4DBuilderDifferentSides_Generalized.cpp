#include "Tree4DBuilderDifferentSides_Generalized.h"
#include "morton4D.h"
#include <cassert>
#include "tree4d_io.h"
#include "svo_builder_util.h"
#include "octree_io.h"


//#define useFastAddEmpty

Tree4DBuilderDifferentSides_Generalized::Tree4DBuilderDifferentSides_Generalized() :
	gridsize_S(0), gridsize_T(0),
	current_morton_code(0), max_morton_code(0),
	maxDepth(0), totalNbOfQueues(0), nbOfQueuesOf16Nodes(0), nbOfSmallerQueues(0),
	generate_levels(false)
{
}

Tree4DBuilderDifferentSides_Generalized::Tree4DBuilderDifferentSides_Generalized(
	std::string base_filename, size_t gridsize_S, size_t gridsize_T, bool generate_levels) :
	base_filename(base_filename), current_morton_code(0),
	max_morton_code(0), maxDepth(0), totalNbOfQueues(0), nbOfQueuesOf16Nodes(0), nbOfSmallerQueues(0),
	gridsize_S(gridsize_S), gridsize_T(gridsize_T),
	generate_levels(generate_levels)
{
}

void Tree4DBuilderDifferentSides_Generalized::calculateMaxMortonCode()
{
	max_morton_code
		= morton4D_Encode_for<uint64_t, uint_fast32_t>(
			static_cast<uint_fast32_t>(gridsize_S) - 1, static_cast<uint_fast32_t>(gridsize_S) - 1, static_cast<uint_fast32_t>(gridsize_S) - 1, static_cast<uint_fast32_t>(gridsize_T) - 1,
			static_cast<uint_fast32_t>(gridsize_S), static_cast<uint_fast32_t>(gridsize_S), static_cast<uint_fast32_t>(gridsize_S), static_cast<uint_fast32_t>(gridsize_T));
}
void Tree4DBuilderDifferentSides_Generalized::setupBuildingVariables()
{
	/*
	CASE: y >= x  ==>  gridsize_t >= gridsize_s
	gridsize_t = 2^y
	gridsize_s = 2^x  with y > x

	#vox = 2^(3x+y) ~> 2^(4x + D)    met D = y - x

	maxDepth = log2( max(gridsize_s, gridsize_t) ) ~> log2(gridsize_t) = y
	totalNbOfQueues = maxDepth + 1  ~> y + 1

	nbOfQueuesOf16Nodes = log2(gridsize_s) = x
	nbOfQueuesOf2Nodes = totalNbOfQueues - nbOfQueuesOf16Nodes ~> y + 1 - x = D + 1
	*/

	/*
	CASE y < x  ==>  gridsize_t < gridsize_s

	gridsize_t = 2^y
	gridsize_s = 2^x  with y < x

	#vox = 2^(3x+y)

	maxDepth = log2( max(gridsize_s, gridsize_t) ) ~> log2(gridsize_S) = x
	totalNbOfQueues = maxDepth + 1  ~> x + 1

	nbOfQueuesOf16Nodes = log2(gridsize_t) = y
	nbOfQueuesOf8Nodes = totalNbOfQueues - nbOfQueuesOf16Nodes
	*/

	size_t sizeOfLongestDimension = (longestDimension == TIME ? gridsize_T : gridsize_S);
	size_t sizeOfShortestDimension = (longestDimension == TIME ? gridsize_S : gridsize_T);
	maxSizeOfSmallestQueue = (longestDimension == TIME ? 2 : 8);


	// Setup building variables
	maxDepth = log2(static_cast<int>(sizeOfLongestDimension));
	totalNbOfQueues = maxDepth + 1;

	nbOfQueuesOf16Nodes = log2(static_cast<int>(sizeOfShortestDimension));
	nbOfSmallerQueues = totalNbOfQueues - nbOfQueuesOf16Nodes;

	queuesOfSmallestAmountOfNodes.resize(nbOfSmallerQueues);
	queuesOfMax16.resize(nbOfQueuesOf16Nodes);

	//nodeQueues.resize(maxDepth + 1);
	for (int i = 0; i < nbOfSmallerQueues; i++)
	{
		queuesOfSmallestAmountOfNodes[i].reserve(maxSizeOfSmallestQueue);
	}
	for (int i = 0; i < nbOfQueuesOf16Nodes; i++)
	{
		queuesOfMax16[i].reserve(16);
	}

}

void Tree4DBuilderDifferentSides_Generalized::initializeBuilder()
{
	std::unique_ptr<TreeNodeWriterCppStyle> nWriter(new TreeNodeWriterCppStyle(base_filename));
	nodeWriter = std::move(nWriter);

	std::unique_ptr<TreeDataWriterCppStyle> dWriter(new TreeDataWriterCppStyle(base_filename));
	dataWriter = std::move(dWriter);

	if(gridsize_S > gridsize_T)
	{
		longestDimension = SPACE;
	}else
	{
		longestDimension = TIME;
	}
	setupBuildingVariables();
	// Fill data arrays
	calculateMaxMortonCode();

	dataWriter->writeVoxelData(VoxelData());// first data point is NULL

#ifdef BINARY_VOXELIZATION
	VoxelData voxelData = VoxelData(0, vec3(), vec3(1.0, 1.0, 1.0)); // We store a simple white voxel in case of Binary voxelization
	dataWriter->writeVoxelData(voxelData);  // all leafs will refer to this
#endif
}

// Add a datapoint to the octree: this is the main method used to push datapoints
void Tree4DBuilderDifferentSides_Generalized::addVoxel(const uint64_t morton_number) {
	// Padding for missed morton numbers, i.e. the empty voxels with morton codes between [current_morton_code, morton number)
	/*	if (morton_number != current_morton_code) {
	fastAddEmptyVoxels(morton_number - current_morton_code);
	}*/

	if (morton_number != current_morton_code) {
#ifndef useFastAddEmpty
		slowAddEmptyVoxels(morton_number - current_morton_code);
#else
		fastAddEmptyVoxels(morton_number - current_morton_code);
#endif
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
void Tree4DBuilderDifferentSides_Generalized::addVoxel(const VoxelData& data) {
	// Padding for missed morton numbers
	/*	if (data.morton != current_morton_code) {
	fastAddEmptyVoxels(data.morton - current_morton_code);
	}*/

	if (data.morton != current_morton_code) {
#ifndef useFastAddEmpty
		slowAddEmptyVoxels(data.morton - current_morton_code);
#else
		fastAddEmptyVoxels(data.morton - current_morton_code);
#endif
	}

	// Create node
	Node4D node = Node4D(); // create empty node

	// Write data point
	node.data = dataWriter->writeVoxelData(data); // store data
	node.data_cache = data; // store data as cache
	// Add to buffers
	queuesOfMax16.at(nbOfQueuesOf16Nodes - 1).push_back(node);;
	// flush the queues
	flushQueues(maxDepth);

	current_morton_code++;
}

void Tree4DBuilderDifferentSides_Generalized::push_backNodeToQueueAtDepth(int depth, Node4D node)
{
	assert(depth >= 0);
	assert(depth <= maxDepth);

	if (depth <= nbOfSmallerQueues - 1)
		//depth in [0, nbOfSmallerQueues - 1]
		//index in queuesOfSmallestAmountOfNodes : [0, nbOfSmallerQueues - 1]
	{
		queuesOfSmallestAmountOfNodes[depth].push_back(node);
	}
	else
		//depth in [nbOfQueuesOf2nodes, totalNbOfQueues - 1]    (remember: maxDepth = totalNbOfQueues - 1
		//index in queuesOfMax16 : [0, nbOfQueuesOf16nodes - 1]
	{
		queuesOfMax16[depth - nbOfSmallerQueues].push_back(node);
	}
}

bool Tree4DBuilderDifferentSides_Generalized::isQueueFilled(int depth)
{
	assert(depth >= 0);
	assert(depth <= maxDepth);

	if (depth <= nbOfSmallerQueues - 1) {
		//depth in [0, nbOfQueuesOf2nodes - 1]
		//index in queuesOfSmallestAmountOfNodes : [0, nbOfQueuesOf2nodes - 1]
		if (queuesOfSmallestAmountOfNodes[depth].size() == maxSizeOfSmallestQueue)
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

		if (queuesOfMax16[depth - nbOfSmallerQueues].size() == 16)
		{
			return true;
		}
		else {
			return false;
		}
	}
}

bool Tree4DBuilderDifferentSides_Generalized::isQueueEmpty(int depth)
{
	assert(depth >= 0);
	assert(depth <= maxDepth);

	if (depth <= nbOfSmallerQueues - 1) {
		//depth in [0, nbOfQueuesOf2nodes - 1]
		//index in queuesOfSmallestAmountOfNodes : [0, nbOfQueuesOf2nodes - 1]
		if (queuesOfSmallestAmountOfNodes[depth].size() == 0)
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

		if (queuesOfMax16[depth - nbOfSmallerQueues].size() == 0)
		{
			return true;
		}
		else {
			return false;
		}
	}
}

bool Tree4DBuilderDifferentSides_Generalized::doesQueueContainOnlyEmptyNodes(int depth) {
	assert(depth >= 0);
	assert(depth <= maxDepth);

	if (depth <= nbOfSmallerQueues - 1)
		//depth in [0, nbOfQueuesOf2nodes - 1]
		//index in queuesOfSmallestAmountOfNodes : [0, nbOfQueuesOf2nodes - 1]
	{
		return doesQueueContainOnlyEmptyNodes(queuesOfSmallestAmountOfNodes[depth], maxSizeOfSmallestQueue);
	}
	else
		//depth in [nbOfQueuesOf2nodes, totalNbOfQueues - 1]    (remember: maxDepth = totalNbOfQueues - 1
		//index in queuesOfMax16 : [0, nbOfQueuesOf16nodes - 1]
	{
		return doesQueueContainOnlyEmptyNodes(queuesOfMax16[depth - nbOfSmallerQueues], 16);
	}
}

// REFINE QUEUES: check all levels from start_depth up and group 16 nodes on a higher level
void Tree4DBuilderDifferentSides_Generalized::flushQueues(const int start_depth) {
	for (int depth = start_depth; depth >= 0; depth--) {
		if (isQueueFilled(depth)) { // we have the max amount of nodes in the queue at depth depth
			assert(depth - 1 >= 0);
			if (doesQueueContainOnlyEmptyNodes(depth)) {
				push_backNodeToQueueAtDepth(depth - 1, Node4D()); // push back NULL node to represent the 16 (or 2) empty nodes
			}
			else {
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
bool Tree4DBuilderDifferentSides_Generalized::doesQueueContainOnlyEmptyNodes(const QueueOfNodes &queue, int maxAmountOfElementsInQueue) {
	for (int k = 0; k < maxAmountOfElementsInQueue; k++) {
		if (!queue[k].isNull()) {
			return false;
		}
	}
	return true;
}

Node4D Tree4DBuilderDifferentSides_Generalized::groupNodesAtDepth(int depth)
{
	assert(depth >= 0);
	assert(depth <= maxDepth);

	if (depth <= nbOfSmallerQueues - 1)
		//depth in [0, nbOfQueuesOf2nodes - 1]
		//index in queuesOfSmallestAmountOfNodes : [0, nbOfQueuesOf2nodes - 1]
	{
		return groupNodesOfMax2(queuesOfSmallestAmountOfNodes[depth]);
	}
	else
		//depth in [nbOfQueuesOf2nodes, totalNbOfQueues - 1]    (remember: maxDepth = totalNbOfQueues - 1
		//index in queuesOfMax16 : [0, nbOfQueuesOf16nodes - 1]
	{
		return groupNodesOfMax16(queuesOfMax16[depth - nbOfSmallerQueues]);
	}
}

Node4D Tree4DBuilderDifferentSides_Generalized::groupNodesOfMax2(const QueueOfNodes &queueOfMax2)
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
		parent.data = dataWriter->writeVoxelData(voxelData);
		parent.data_cache = voxelData;
	}
	return parent;
}

Node4D Tree4DBuilderDifferentSides_Generalized::groupNodesOfMax16(const QueueOfNodes &queueOfMax16)
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
		parent.data = dataWriter->writeVoxelData(voxelData);
		parent.data_cache = voxelData;
	}
	return parent;
}

void Tree4DBuilderDifferentSides_Generalized::writeNodeToDiskAndSetOffsetOfParent_Max2NodesInQueue(Node4D& parent, bool& first_stored_child, int indexOfCurrentChildNode, Node4D currentChildNode)
{
	//store the node on disk
	size_t positionOfChildOnDisk = nodeWriter->writeNode4D_(currentChildNode);
	
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

void Tree4DBuilderDifferentSides_Generalized::writeNodeToDiskAndSetOffsetOfParent_Max16NodesInQueue(Node4D& parent, bool& first_stored_child, int indexOfCurrentChildNode, Node4D currentChildNode)
{
	//store the node on disk
	size_t positionOfChildOnDisk = nodeWriter->writeNode4D_(currentChildNode);

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
Node4D Tree4DBuilderDifferentSides_Generalized::groupNodes(const QueueOfNodes &queue, int maxAmountOfElementsInQueue) {
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

void Tree4DBuilderDifferentSides_Generalized::setChildrenOffsetsForNodeWithMax2Children(Node4D &parent, int indexInQueue, char offset)
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

/*
Set in the node 'parent' the offset of its child node.
The offset is 'offset'.
'indexInQueue' is the index of the child in the queue we are currently grouping.
*/
void Tree4DBuilderDifferentSides_Generalized::setChildrenOffsetsForNodeWithMax8Children(Node4D &parent, int indexInQueue, char offset)
{
	//	/*
	//	total number of chilren in Node : 16
	//
	//	index i in queue of 16  --> child      i											e.g. 0 -> 0
	//	index i in queue of 8   --> children  2i, 2i+1										e.g. 0 -> 0,1
	//	index i in queue of 4   --> children  3i, 3i+1, 3i+2, 3i+3							e.g. 0 -> 0,1,2,3
	//	index i in queue of 2   --> children  4i, 4i+2, 4i+2, 4i+3, 4i+4, 4i+5, 4i+6, 4i+7	e.g. 0 -> 0,1,2,3,4,5,6,7
	//	*/
	//
	//	for (int i = 0; i < 2; i++)
	//	{
	//		parent.children_offset[2*indexInQueue + i] = offset;
	//	}

	parent.children_offset[indexInQueue] = offset;
	parent.children_offset[8 + indexInQueue] = offset;
}


void Tree4DBuilderDifferentSides_Generalized::clearQueueAtDepth(int depth)
{
	assert(depth >= 0);
	assert(depth <= maxDepth);

	if (depth <= nbOfSmallerQueues - 1)
		//depth in [0, nbOfQueuesOf2nodes - 1]
		//index in queuesOfSmallestAmountOfNodes : [0, nbOfQueuesOf2nodes - 1]
	{
		queuesOfSmallestAmountOfNodes[depth].clear();
	}
	else
		//depth in [nbOfQueuesOf2nodes, totalNbOfQueues - 1]    (remember: maxDepth = totalNbOfQueues - 1
		//index in queuesOfMax16 : [0, nbOfQueuesOf16nodes - 1]
	{
		queuesOfMax16[depth - nbOfSmallerQueues].clear();
	}
}

// Finalize the tree: add rest of empty nodes, make sure root node is on top
void Tree4DBuilderDifferentSides_Generalized::finalizeTree() {
	// fill octree
	/*	if (current_morton_code < max_morton_code) {
	fastAddEmptyVoxels((max_morton_code - current_morton_code) + 1);
	}*/
	if (current_morton_code < max_morton_code)
	{
#ifndef useFastAddEmpty
		slowAddEmptyVoxels((max_morton_code - current_morton_code) + 1);
#else
		fastAddEmptyVoxels((max_morton_code - current_morton_code) + 1);
#endif
	}

	// write root node
	nodeWriter->writeNode4D_(getRootNode());

	// write header
	Tree4DInfo tree4D_info(1, base_filename, gridsize_S, gridsize_T, nodeWriter->position_in_output_file, dataWriter->position_in_output_file);
		writeOctreeHeader(base_filename + string(".tree4d"), tree4D_info);
}

Node4D Tree4DBuilderDifferentSides_Generalized::getRootNode()
{
	if (nbOfSmallerQueues > 0)
	{
		return queuesOfSmallestAmountOfNodes[0][0];
	}
	else
	{
		return queuesOfMax16[0][0];
	}
}


/*
Intuitief:
in 4D:
16 empty nodes toevoegen in queue op level depth
==
1 empty node toevoegen in queue op level depth - 1
in 2D:
2 empty nodes toevoegen in queue op level depth
==
1 empty node toevoegen in queue op level depth - 1
==>
1 empty parent node toeveogen is veel efficienter dan
2^dimension empty child nodes toevoegen en die dan groeperen

Hoe over verschillende levels toepassen?
Een empty node kan NIET toegevoegd worden aan een queue
ALS er is een queue op een lagere diepte die nog nodes bevat.

Aan welke queue voegen we de node toe?
--> afhankelijk van
1) a = het aantal lege nodes dat we moeten toevoegen,
afgerond naar een macht van 2^dimension
2) b = de diepte van de laagste niet-lege queue

*/
void Tree4DBuilderDifferentSides_Generalized::slowAddEmptyVoxels(const size_t nbOfNodesToAdd)
{
	int nbOfEmptyNodeYetToAdd = nbOfNodesToAdd;

	while (nbOfEmptyNodeYetToAdd > 0)
	{
		push_backNodeToQueueAtDepth(maxDepth, Node4D());
		flushQueues(maxDepth);
		current_morton_code = (uint64_t)(current_morton_code + 1); // because we're adding at a certain level
		nbOfEmptyNodeYetToAdd--;
	}
}

/*
// A method to quickly add empty nodes
inline void Tree4DBuilderDifferentSides_Generalized::fastAddEmptyVoxels(const size_t nbOfEmptyVoxelsToAdd) {
size_t r_budget = nbOfEmptyVoxelsToAdd;
while (r_budget > 0) {
unsigned int queueDepth = computeBestFillQueue(r_budget);
addEmptyVoxel(queueDepth);
size_t budget_hit = (size_t)pow(16.0, maxDepth - queueDepth);
r_budget = r_budget - budget_hit;
}
}
*/

/*// Compute the best fill queue given the budget
inline int Tree4DBuilderDifferentSides_Generalized::computeBestFillQueue(const size_t budget) {
// which power of 16 fits in budget?
int budget_queue_suggestion = maxDepth - findPowerOf16(budget);
// if our current guess is already b_maxdepth, return that, no need to test further
if (budget_queue_suggestion == maxDepth)
{
return maxDepth;
}
// best fill buffer is maximum of suggestion and highest non_empty buffer
return max(budget_queue_suggestion, highestNonEmptyQueue());
}*/

// A method to quickly add empty nodes
inline void Tree4DBuilderDifferentSides_Generalized::fastAddEmptyVoxels(const size_t nbOfEmptyVoxelsToAdd) {
	int nbOfEmptyVoxelsYetToAdd = nbOfEmptyVoxelsToAdd;

	while (nbOfEmptyVoxelsYetToAdd > 0) {
		int queueDepth = computeDepthOfBestQueue(nbOfEmptyVoxelsYetToAdd);
		addEmptyVoxel(queueDepth);
		int nbOfVoxelsAdded = nbOfVoxelsAddedByAddingAnEmptyVoxelAtDepth(queueDepth);
		nbOfEmptyVoxelsYetToAdd -= nbOfVoxelsAdded;
	}
}

int Tree4DBuilderDifferentSides_Generalized::nbOfVoxelsAddedByAddingAnEmptyVoxelAtDepth(int depth)
{
	if (depth >= nbOfSmallerQueues - 1) // depth >= y - x
	{
		return pow(16.0, maxDepth - depth);
	}
	else {
		//  nbOFNodesToAdd = 2^(4x + D) = pow(gridsize_S, 4) * 2^D
		size_t leftFactor = pow(gridsize_S, 4.0);
		size_t D = maxDepth - nbOfQueuesOf16Nodes - depth; //= nbOfQueuesOf2 - 1 - depth
		size_t nbOfNodesAdded = leftFactor * (2 << D);
		return nbOfNodesAdded;
	}
}

int Tree4DBuilderDifferentSides_Generalized::calculateQueueShouldItBePossibleToAddAllVoxelsAtOnce(const size_t nbOfEmptyVoxelsToAdd)
{
	//STEL: we hebben enkel queues van 16 nodes
	if (nbOfSmallerQueues == 0)
	{
		// In elke queue passen 16 nodes.
		// 1 node toevoegen --> 1 node toevoegen op maxDepth
		//  ...
		// 1 keer 16 nodes toevoegen  = 16^1 * 1 nodes toevoegen --> 1 node toevoegen op maxDepth - 1
		// 2 keer 16 nodes toevoegen  = 16^1 * 2 nodes toevoegen --> 2 nodes toevoegen op maxDepth - 1
		// ...
		// 16 keer 16 nodes toevoegen = 16^2 * 1 nodes toevoegen --> 1 node toevoegen op maxDepth - 2
		// 16^2 *1  + 1 nodes toevoegen --> 1 node toevoegen op maxDepths-2 , 1 node op maxDepth
		// ...
		// 16^a nodes toevoegen --> 1 node toevoegen op maxDepth - a

		size_t a = findPowerOf16(nbOfEmptyVoxelsToAdd);

		size_t suggestedDepth = maxDepth - a;
		assert(suggestedDepth >= 0);
		assert(suggestedDepth <= maxDepth);

		return suggestedDepth;
	}
	else {
		/*
		gridsize_t = 2^y
		gridsize_s = 2^x  with y > x

		#vox = 2^(3x+y) ~> 2^(4x + D)    met D = y - x >= 0

		maxDepth = log2( max(gridsize_s, gridsize_t) ) ~> log2(gridsize_t) = y
		totalNbOfQueues = maxDepth + 1  ~> y + 1

		nbOfQueuesOf16Nodes = log2(gridsize_s) = x
		nbOfQueuesOf2Nodes = totalNbOfQueues - nbOfQueuesOf16Nodes ~> y + 1 - x = D + 1
		*/

		// we hebben x     queues van 16 nodes
		//			 D + 1 queues van  2 nodes
		// 
		// 16^x nodes toevoegen  
		//		= 2^4x nodes toevoegen
		//			--> 1 node toevoegen op maxDepth - x = y - x (onderste queue met 2 nodes)
		// ...
		// 2 * 16^x nodes toevoegen
		//		= 2^(4x + 1) nodes toevoegen
		//			--> 2 nodes toevoegen op maxDepth - x 
		//									= y- x (onderste queue met 2 nodes)
		//			--> 1 node toevoegen op maxDepth - x - 1 
		//									= y - x - 1
		// 4 * 16^x nodes toevoegen
		//		= 2^(4x + 2) nodes toevoegen
		//			-->  2 nodes toevoegen op maxDepth - x - 1
		//									= y - x - 1
		//			-->  1 node toevoegen op maxDepth - x - 2
		//									= y - x - 2
		// ... 
		// D * 16^x nodes toevoegen  = #vox
		//		= 2^(4x + D) nodes toevoegen
		//			--> 1 node toevoegen op maxDepth - x - D
		//									= y - x - D
		//									= 0   (bovenste queue)
		if (nbOfEmptyVoxelsToAdd < pow(gridsize_S, 4.0) * 2) {
			// nbOfEmptyVoxelsToAdd < 2^(4x + 1)
			size_t a = findPowerOf16(nbOfEmptyVoxelsToAdd);

			size_t suggestedDepth = maxDepth - a; //y - a
			assert(suggestedDepth >= 0);
			assert(suggestedDepth <= maxDepth);

			return suggestedDepth;
		}
		else {
			// nbOfEmptyVoxelsToAdd >= 2^(4x + 1) ( = 2 * 16^x)
			int factor = nbOfEmptyVoxelsToAdd / pow(gridsize_S, 4.0);
			int a = log2(factor); //a in {1, 2, ..., D}
			int suggestedDepth = maxDepth - nbOfQueuesOf16Nodes - a; //y - x - a
			assert(suggestedDepth >= 0);
			assert(suggestedDepth <= maxDepth);

			return suggestedDepth;
		}
	}
}

// Compute the best fill queue given the budget
inline int Tree4DBuilderDifferentSides_Generalized::computeDepthOfBestQueue(const size_t nbofEmptyVoxelsToAdd) {
	// which power of 16 fits in budget?
	int depthA = calculateQueueShouldItBePossibleToAddAllVoxelsAtOnce(nbofEmptyVoxelsToAdd);

	// if our current guess is already b_maxdepth, return that, no need to test further
	if (depthA == maxDepth)
	{
		return maxDepth;
	}

	int depthB = highestNonEmptyQueue();

	//choose the LOWEST QUEUE  ==> the highest depth
	int suggestedDepth = max(depthA, depthB);
	assert(suggestedDepth >= 0);
	assert(suggestedDepth <= maxDepth);
	return suggestedDepth;
}

// Find the highest non empty buffer, return its index
inline int Tree4DBuilderDifferentSides_Generalized::highestNonEmptyQueue() {
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
void Tree4DBuilderDifferentSides_Generalized::addEmptyVoxel(const int queueDepth) {
	push_backNodeToQueueAtDepth(queueDepth, Node4D());
	//nodeQueues[buffer].push_back(Node4D());
	flushQueues(queueDepth);
	current_morton_code += static_cast<uint64_t>(nbOfVoxelsAddedByAddingAnEmptyVoxelAtDepth(queueDepth));
	//current_morton_code = (uint64_t)(current_morton_code + pow(16.0, maxDepth - queueDepth)); // because we're adding at a certain level
}