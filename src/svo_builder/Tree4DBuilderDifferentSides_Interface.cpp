#include "Tree4DBuilderDifferentSides_Interface.h"
#include <cassert>
#include "morton4D.h"
#include "tree4d_io.h"

#define useFastAddEmpty

// Add a datapoint to the octree: this is the main method used to push datapoints
void Tree4DBuilderDifferentSides_Interface::addVoxel(const uint64_t morton_number)
{
	// Padding for missed morton numbers, i.e. the empty voxels with morton codes between [current_morton_code, morton number)
	if (morton_number != current_morton_code) {
#ifndef useFastAddEmpty
		slowAddEmptyVoxels(morton_number - current_morton_code);
#else
		fastAddEmptyVoxels(morton_number - current_morton_code);
#endif
	}
	if(morton_number != current_morton_code)
	{
		cout << "something went wrong" << endl;
	}



	// Create a new leaf node
	Node4D node = Node4D(); // create empty node
	node.data = 1; // all nodes in binary voxelization refer to this

	//add the new leaf node to the lowest queue
	push_backNodeToLowestQueue(node);

	// flush the queues
	flushQueues(maxDepth);

	current_morton_code++;
}

void Tree4DBuilderDifferentSides_Interface::addVoxel(const VoxelData& data)
{
	// Padding for missed morton numbers
	if (data.morton != current_morton_code) {
#ifndef useFastAddEmpty
		slowAddEmptyVoxels(data.morton - current_morton_code);
#else
		fastAddEmptyVoxels(data.morton - current_morton_code);
#endif
	}

	// Create a new leaf node
	Node4D node = Node4D(); // create empty node

	// Write data point
	node.data = dataWriter->writeVoxelData(data); // store data
	node.data_cache = data; // store data as cache
	//add the new leaf node to the lowest queue
	push_backNodeToLowestQueue(node);
	// flush the queues
	flushQueues(maxDepth);

	current_morton_code++;
}

// Finalize the tree: add rest of empty nodes, make sure root node is on top
void Tree4DBuilderDifferentSides_Interface::finalizeTree()
{
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

Tree4DBuilderDifferentSides_Interface::Tree4DBuilderDifferentSides_Interface
(size_t gridsize_s, size_t gridsize_t, uint64_t current_morton_code, uint64_t max_morton_code, int maxDepth, int totalNbOfQueues, bool generate_levels, string base_filename):
	gridsize_S(gridsize_s), gridsize_T(gridsize_t),
	current_morton_code(0), max_morton_code(0),
	maxDepth(maxDepth), totalNbOfQueues(totalNbOfQueues),
	generate_levels(generate_levels), base_filename(base_filename)
{
}

void Tree4DBuilderDifferentSides_Interface::calculateMaxMortonCode()
{
	max_morton_code
		= morton4D_Encode_for<uint64_t, uint_fast32_t>(
			static_cast<uint_fast32_t>(gridsize_S) - 1, static_cast<uint_fast32_t>(gridsize_S) - 1, static_cast<uint_fast32_t>(gridsize_S) - 1, static_cast<uint_fast32_t>(gridsize_T) - 1,
			static_cast<uint_fast32_t>(gridsize_S), static_cast<uint_fast32_t>(gridsize_S), static_cast<uint_fast32_t>(gridsize_S), static_cast<uint_fast32_t>(gridsize_T));
}

// Add an empty datapoint at a certain buffer level, and refine upwards from there
void Tree4DBuilderDifferentSides_Interface::addEmptyVoxel(const int queueDepth)
{
	push_backNodeToQueueAtDepth(queueDepth, Node4D());
	//nodeQueues[buffer].push_back(Node4D());
	flushQueues(queueDepth);
	int nbOfEmptyVoxelsAdded = nbOfEmptyVoxelsAddedByAddingAnEmptyNodeAtDepth(queueDepth);

	current_morton_code += static_cast<uint64_t>(nbOfEmptyVoxelsAdded);
	



	//current_morton_code = (uint64_t)(current_morton_code + pow(16.0, maxDepth - queueDepth)); // because we're adding at a certain level
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
void Tree4DBuilderDifferentSides_Interface::slowAddEmptyVoxels(const size_t nbOfNodesToAdd)
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


//// A method to quickly add empty nodes
//inline void Tree4DBuilderDifferentSides_Space_longest::fastAddEmptyVoxels(const size_t nbOfEmptyVoxelsToAdd) {
//	size_t r_budget = nbOfEmptyVoxelsToAdd;
//	while (r_budget > 0) {
//		unsigned int queueDepth = computeBestFillQueue(r_budget);
//		addEmptyVoxel(queueDepth);
//		size_t budget_hit = (size_t)pow(16.0, maxDepth - queueDepth);
//		r_budget = r_budget - budget_hit;
//	}
//}

// A method to quickly add empty nodes
void Tree4DBuilderDifferentSides_Interface::fastAddEmptyVoxels(const size_t nbOfEmptyVoxelsToAdd)
{
	//local variable: the number of empty voxels we have yet to add
	int nbOfEmptyVoxelsYetToAdd = nbOfEmptyVoxelsToAdd;
	if(nbOfEmptyVoxelsYetToAdd < 0)
	{
		cout << "nbOfVoxelsAdded < 0" << endl;
	}


	//while we still have empty voxels to add, do:
	while (nbOfEmptyVoxelsYetToAdd > 0) {

		//calculate the depth of the best queue to add a node
		int queueDepth = computeDepthOfBestQueue(nbOfEmptyVoxelsYetToAdd);
		//add an empty node to that queue
		addEmptyVoxel(queueDepth);
		//NOTE: by adding a node to the queue at the calculated depth,
		//      we added a certain number of empty voxels
		//		This amount is SMALLER THAN OR EQUAL TO the number of empty voxels we had to add
		//		How many empty voxels did we add by pushing a node in the queue at the calculated depth?
		
		int nbOfVoxelsAdded = nbOfEmptyVoxelsAddedByAddingAnEmptyNodeAtDepth(queueDepth);
		if (nbOfVoxelsAdded < 0)
		{
			cout << "nbOfVoxelsAdded < 0" << endl;
		}
		//		update the amount of empty voxels we have still to add
		nbOfEmptyVoxelsYetToAdd = nbOfEmptyVoxelsYetToAdd - nbOfVoxelsAdded;
	}
}

/*
We want to add 'nbofEmptyVoxelsToAdd' empty voxels to the lowest queue.
Instead of adding these voxels one by one, we can speed this up by adding a lower number of nodes but in higher queues.
Taking into account the current state of the queues, 
this method calculates the highest queue in which a node can be added so that,
by adding this node to that queue, the number of empty voxels we have still to add is the smallest we can get after adding a node.

*/
int Tree4DBuilderDifferentSides_Interface::computeDepthOfBestQueue(const size_t nbofEmptyVoxelsToAdd)
{
	
	// SUPPOSE all queues are empty.
	// What is the highest queue in which we can add a node, 
	// so that the largest possible number of empty voxels is added,
	// with that amount smaller than the number of empty voxels we want to add?
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
int Tree4DBuilderDifferentSides_Interface::highestNonEmptyQueue()
{
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


/*// Compute the best fill queue given the budget
inline int Tree4DBuilderDifferentSides_Space_longest::computeBestFillQueue(const size_t budget) {
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

/*
Check if the given queue is completely filled with empty leaf nodes ( = null nodes).

IMPORTANT NOTE:
This method expects the given queue to be completely full.
It is UNSAFE to call this method when the queue is not full.
It should be checked if the queue is full before calling this method.
*/
bool Tree4DBuilderDifferentSides_Interface::doesQueueContainOnlyEmptyNodes(const QueueOfNodes &queue, int maxAmountOfElementsInQueue) {
	//NOTE: Probably not safe to use when queue not full
	for (int k = 0; k < maxAmountOfElementsInQueue; k++) { //check all places in the queue
		if (!queue[k].isNull()) { // if a node in the queue is not an empty leaf node
			return false;
		}
	}
	return true;
}

void Tree4DBuilderDifferentSides_Interface::writeNodeToDiskAndSetOffsetOfParent_Max16NodesInQueue(Node4D& parent, bool& first_stored_child, int indexOfCurrentChildNode, Node4D currentChildNode)
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
Groups the nodes in the given queue.

PRECONDITION: The queue at the given depth is filled with nodes.
When calling this method, we know that some of the nodes in this queue are NOT empty leaf nodes.

RETURNS: the parent node of the nodes in this queue.

Calling this method will write the non-empty child nodes to disk.
It sets the child pointers to the non-empty child nodes in the returned parent node.
*/
Node4D Tree4DBuilderDifferentSides_Interface::groupNodesOfMax16(const QueueOfNodes &queueOfMax16)
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
			if (!queueOfMax16[i].isNull()){
				notnull++;
			}
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

// REFINE QUEUES: check all levels from start_depth up and group 16 nodes on a higher level
void Tree4DBuilderDifferentSides_Interface::flushQueues(const int start_depth) {
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