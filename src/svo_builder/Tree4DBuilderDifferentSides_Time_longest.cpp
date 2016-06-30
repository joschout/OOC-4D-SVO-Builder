#include "Tree4DBuilderDifferentSides_Time_longest.h"
#include <cassert>
#include "svo_builder_util.h"
#include "octree_io.h"


//#define useFastAddEmpty

Tree4DBuilderDifferentSides_Time_longest::Tree4DBuilderDifferentSides_Time_longest(): 
	Tree4DBuilderDifferentSides_Interface(0, 0, 0, 0, 0, 0, false, string()),
	nbOfQueuesOf16Nodes(0), nbOfQueuesOf2Nodes(0)
{
}

Tree4DBuilderDifferentSides_Time_longest::Tree4DBuilderDifferentSides_Time_longest(
	std::string base_filename, size_t gridsize_S, size_t gridsize_T, bool generate_levels):
	Tree4DBuilderDifferentSides_Interface(gridsize_S, gridsize_T, 0, 0, 0, 0, generate_levels, base_filename),
	nbOfQueuesOf16Nodes(0), nbOfQueuesOf2Nodes(0)
{
}

void Tree4DBuilderDifferentSides_Time_longest::initializeBuilder()
{
	std::unique_ptr<TreeNodeWriterCppStyle> nWriter(new TreeNodeWriterCppStyle(base_filename));
	nodeWriter = std::move(nWriter);

	std::unique_ptr<TreeDataWriterCppStyle> dWriter(new TreeDataWriterCppStyle(base_filename));
	dataWriter = std::move(dWriter);

	/*
	gridsize_t = 2^y
	gridsize_s = 2^x  with y > x

	#vox = 2^(3x+y) ~> 2^(4 + D)    met D = y - x

	maxDepth = log2( max(gridsize_s, gridsize_t) ) ~> log2(gridsize_t) = y
	totalNbOfQueues = maxDepth + 1  ~> y + 1

	nbOfQueuesOf16Nodes = log2(gridsize_s) = x
	nbOfQueuesOf2Nodes = totalNbOfQueues - nbOfQueuesOf16Nodes ~> y + 1 - x = D + 1
	*/

	/*
	 MAAR WAT ALS y <= x?

	 gridsize_t = 2^y
	 gridsize_s = 2^x  with y <= x

	 #vox = 2^(3x+y)

	 maxDepth = log2( max(gridsize_s, gridsize_t) ) ~> log2(gridsize_S) = X
	 totalNbOfQueues = maxDepth + 1  ~> y + 1

	 nbOfQueuesOf16Nodes = log2(gridsize_s) = x
	 nbOfQueuesOf2Nodes = totalNbOfQueues - nbOfQueuesOf16Nodes ~> y + 1 - x = D + 1

	*/



	// Setup building variables
	maxDepth = log2(static_cast<int>(max(gridsize_S, gridsize_T)));
	totalNbOfQueues = maxDepth + 1;


	nbOfQueuesOf16Nodes = log2(static_cast<int>(gridsize_S));
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

	dataWriter->writeVoxelData(VoxelData());// first data point is NULL

#ifdef BINARY_VOXELIZATION
	VoxelData voxelData = VoxelData(0, vec3(), vec3(1.0, 1.0, 1.0)); // We store a simple white voxel in case of Binary voxelization
	dataWriter->writeVoxelData(voxelData);  // all leafs will refer to this
#endif
}

// Add a datapoint to the octree: this is the main method used to push datapoints
void Tree4DBuilderDifferentSides_Time_longest::addVoxel(const uint64_t morton_number) {
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
void Tree4DBuilderDifferentSides_Time_longest::addVoxel(const VoxelData& data) {
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

void Tree4DBuilderDifferentSides_Time_longest::push_backNodeToQueueAtDepth(int depth, Node4D node)
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

bool Tree4DBuilderDifferentSides_Time_longest::isQueueFilled(int depth)
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

bool Tree4DBuilderDifferentSides_Time_longest::isQueueEmpty(int depth)
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

/*
IMPORTANT NOTE:
This method expects the queue on the given depth to be completely full.
It is UNSAFE to call this method when the queue is not full.
It should be checked if the queue is full before calling this method.
*/
bool Tree4DBuilderDifferentSides_Time_longest::doesQueueContainOnlyEmptyNodes(int depth) {
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

/*
Groups the nodes in the given queue.

PRECONDITION: The queue at the given depth is filled with nodes.
When calling this method, we know that some of the nodes in this queue are NOT empty leaf nodes.

RETURNS: the parent node of the nodes in this queue.

Calling this method will write the non-empty child nodes to disk.
It sets the child pointers to the non-empty child nodes in the returned parent node.
*/
Node4D Tree4DBuilderDifferentSides_Time_longest::groupNodesAtDepth(int depth)
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

/*
Groups the nodes in the given queue.

PRECONDITION: The queue at the given depth is filled with nodes.
When calling this method, we know that some of the nodes in this queue are NOT empty leaf nodes.

RETURNS: the parent node of the nodes in this queue.

Calling this method will write the non-empty child nodes to disk.
It sets the child pointers to the non-empty child nodes in the returned parent node.
*/
Node4D Tree4DBuilderDifferentSides_Time_longest::groupNodesOfMax2(const QueueOfNodes &queueOfMax2)
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

void Tree4DBuilderDifferentSides_Time_longest::writeNodeToDiskAndSetOffsetOfParent_Max2NodesInQueue(Node4D& parent, bool& first_stored_child, int indexOfCurrentChildNode, Node4D currentChildNode)
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

/*
// Group 16 nodes, write non-empty nodes to disk and create parent node
Node4D Tree4DBuilderDifferentSides_Time_longest::groupNodes(const QueueOfNodes &queue, int maxAmountOfElementsInQueue) {
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

/*
Set in the node 'parent' the offset of its child node.
The offset is 'offset'. 
'indexInQueue' is the index of the child in the queue we are currently grouping.
*/
void Tree4DBuilderDifferentSides_Time_longest::setChildrenOffsetsForNodeWithMax2Children(Node4D &parent, int indexInQueue, char offset)
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

void Tree4DBuilderDifferentSides_Time_longest::clearQueueAtDepth(int depth)
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

Node4D Tree4DBuilderDifferentSides_Time_longest::getRootNode()
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

int Tree4DBuilderDifferentSides_Time_longest::nbOfVoxelsAddedByAddingAnEmptyVoxelAtDepth(int depth)
{
	if(depth >= nbOfQueuesOf2Nodes - 1) // depth >= y - x
	{
		return pow(16.0, maxDepth - depth);
	}else{
		//  nbOFNodesToAdd = 2^(4x + D) = pow(gridsize_S, 4) * 2^D
		size_t leftFactor = pow(gridsize_S, 4.0);
		size_t D = maxDepth - nbOfQueuesOf16Nodes - depth; //= nbOfQueuesOf2 - 1 - depth
		size_t nbOfNodesAdded = leftFactor * (2 << D);
		return nbOfNodesAdded;
	}
}

int Tree4DBuilderDifferentSides_Time_longest::calculateQueueShouldItBePossibleToAddAllVoxelsAtOnce(const size_t nbOfEmptyVoxelsToAdd)
{
	//STEL: we hebben enkel queues van 16 nodes
	if(nbOfQueuesOf2Nodes == 0)
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
	} else{ 
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
		if(nbOfEmptyVoxelsToAdd < pow(gridsize_S, 4.0) * 2 ){
			// nbOfEmptyVoxelsToAdd < 2^(4x + 1)
			size_t a = findPowerOf16(nbOfEmptyVoxelsToAdd);

			size_t suggestedDepth = maxDepth - a; //y - a
			assert(suggestedDepth >= 0);
			assert(suggestedDepth <= maxDepth);

			return suggestedDepth;
		}else{
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