#include "Tree4DBuilderDifferentSides_Space_longest.h"
#include <cassert>
#include "svo_builder_util.h"
#include "octree_io.h"



Tree4DBuilderDifferentSides_Space_longest::Tree4DBuilderDifferentSides_Space_longest() :
	Tree4DBuilderDifferentSides_Interface( 0, 0, 0, 0, 0, 0, false, string()),
	nbOfQueuesOf16Nodes(0), nbOfQueuesOf8Nodes(0)
{
}

Tree4DBuilderDifferentSides_Space_longest::Tree4DBuilderDifferentSides_Space_longest(
	std::string base_filename, size_t gridsize_S, size_t gridsize_T, bool generate_levels) :
	Tree4DBuilderDifferentSides_Interface(gridsize_S, gridsize_T, 0, 0, 0, 0, generate_levels, base_filename),
	nbOfQueuesOf16Nodes(0), nbOfQueuesOf8Nodes(0)
{
}

void Tree4DBuilderDifferentSides_Space_longest::initializeBuilder()
{
	svo_io_output_timer.start();	// TIMING
	std::unique_ptr<TreeNodeWriterCppStyle> nWriter(new TreeNodeWriterCppStyle(base_filename));
	nodeWriter = std::move(nWriter);

	std::unique_ptr<TreeDataWriterCppStyle> dWriter(new TreeDataWriterCppStyle(base_filename));
	dataWriter = std::move(dWriter);
	svo_io_output_timer.stop();		// TIMING
	svo_algorithm_timer.start();	// TIMING


	/*
	OLD CASE, NOT THE ONE I'M INTERESTED IN IN THIS TEST CASE

	CASE: y >= x
		gridsize_t = 2^y
		gridsize_s = 2^x  with y > x

		#vox = 2^(3x+y) ~> 2^(4x + D)    met D = y - x

		maxDepth = log2( max(gridsize_s, gridsize_t) ) ~> log2(gridsize_t) = y
		totalNbOfQueues = maxDepth + 1  ~> y + 1

		nbOfQueuesOf16Nodes = log2(gridsize_s) = x
		nbOfQueuesOf2Nodes = totalNbOfQueues - nbOfQueuesOf16Nodes ~> y + 1 - x = D + 1
	*/

	/*
	NEW CASE:
	CASE x < y

	gridsize_t = 2^y
	gridsize_s = 2^x  with y < x

	#vox = 2^(3x+y)

	maxDepth = log2( max(gridsize_s, gridsize_t) ) ~> log2(gridsize_S) = x
	totalNbOfQueues = maxDepth + 1  ~> x + 1

	nbOfQueuesOf16Nodes = log2(gridsize_t) = y
	nbOfQueuesOf2Nodes = totalNbOfQueues - nbOfQueuesOf16Nodes 

	*/



	// Setup building variables
	maxDepth = log2(static_cast<int>(max(gridsize_S, gridsize_T)));
	totalNbOfQueues = maxDepth + 1;


	nbOfQueuesOf16Nodes = log2(static_cast<int>(gridsize_T));
	nbOfQueuesOf8Nodes = totalNbOfQueues - nbOfQueuesOf16Nodes;

	queuesOfMax8.resize(nbOfQueuesOf8Nodes);
	queuesOfMax16.resize(nbOfQueuesOf16Nodes);

	//nodeQueues.resize(maxDepth + 1);
	for (int i = 0; i < nbOfQueuesOf8Nodes; i++)
	{
		queuesOfMax8[i].reserve(8);
	}
	for (int i = 0; i < nbOfQueuesOf16Nodes; i++)
	{
		queuesOfMax16[i].reserve(16);
	}


	svo_algorithm_timer.stop();		// TIMING
	svo_total_timer.stop();			// TIMING
	if(data_out)
	{
		stringstream out;
		out
			<< "tree building - number of 8-element queues: " << nbOfQueuesOf8Nodes << endl
			<< "tree building - number of 16-element queues: " << nbOfQueuesOf16Nodes << endl
			<< "tree building - total number of queues: " << totalNbOfQueues << endl;
		data_writer_ptr->writeToFile(out.str());
	}
	svo_total_timer.start();		// TIMING
	svo_algorithm_timer.start();	// TIMING

	// Fill data arrays
	calculateMaxMortonCode();
	
	svo_algorithm_timer.stop();		// TIMING
	svo_io_output_timer.start();	// TIMING
	dataWriter->writeVoxelData(VoxelData());// first data point is NULL

#ifdef BINARY_VOXELIZATION
	VoxelData voxelData = VoxelData(0, vec3(), vec3(1.0, 1.0, 1.0)); // We store a simple white voxel in case of Binary voxelization
	dataWriter->writeVoxelData(voxelData);  // all leafs will refer to this
#endif
	svo_io_output_timer.stop();		// TIMING
	svo_algorithm_timer.stop();		// TIMING
}

void Tree4DBuilderDifferentSides_Space_longest::push_backNodeToQueueAtDepth(int depth, Node4D& node)
{
	assert(depth >= 0);
	assert(depth <= maxDepth);

	if (depth <= nbOfQueuesOf8Nodes - 1)
		//depth in [0, nbOfQueuesOf2nodes - 1]
		//index in queuesOfMax2 : [0, nbOfQueuesOf2nodes - 1]
	{
		queuesOfMax8[depth].push_back(node);
	}
	else
		//depth in [nbOfQueuesOf2nodes, totalNbOfQueues - 1]    (remember: maxDepth = totalNbOfQueues - 1
		//index in queuesOfMax16 : [0, nbOfQueuesOf16nodes - 1]
	{
		queuesOfMax16[depth - nbOfQueuesOf8Nodes].push_back(node);
	}
}

bool Tree4DBuilderDifferentSides_Space_longest::isQueueFilled(int depth)
{
	assert(depth >= 0);
	assert(depth <= maxDepth);

	if (depth <= nbOfQueuesOf8Nodes - 1) {
		//depth in [0, nbOfQueuesOf2nodes - 1]
		//index in queuesOfMax2 : [0, nbOfQueuesOf2nodes - 1]
		if (queuesOfMax8[depth].size() == 8)
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

		if (queuesOfMax16[depth - nbOfQueuesOf8Nodes].size() == 16)
		{
			return true;
		}
		else {
			return false;
		}
	}
}

bool Tree4DBuilderDifferentSides_Space_longest::isQueueEmpty(int depth)
{
	assert(depth >= 0);
	assert(depth <= maxDepth);

	if (depth <= nbOfQueuesOf8Nodes - 1) {
		//depth in [0, nbOfQueuesOf2nodes - 1]
		//index in queuesOfMax2 : [0, nbOfQueuesOf2nodes - 1]
		if (queuesOfMax8[depth].size() == 0)
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

		if (queuesOfMax16[depth - nbOfQueuesOf8Nodes].size() == 0)
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
bool Tree4DBuilderDifferentSides_Space_longest::doesQueueContainOnlyEmptyNodes(int depth) {
	assert(depth >= 0);
	assert(depth <= maxDepth);

	if (depth <= nbOfQueuesOf8Nodes - 1)
		//depth in [0, nbOfQueuesOf2nodes - 1]
		//index in queuesOfMax2 : [0, nbOfQueuesOf2nodes - 1]
	{
		return doesQueueContainOnlyEmptyNodes(queuesOfMax8[depth], 8);
	}
	else
		//depth in [nbOfQueuesOf2nodes, totalNbOfQueues - 1]    (remember: maxDepth = totalNbOfQueues - 1
		//index in queuesOfMax16 : [0, nbOfQueuesOf16nodes - 1]
	{
		return doesQueueContainOnlyEmptyNodes(queuesOfMax16[depth - nbOfQueuesOf8Nodes], 16);
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
Node4D Tree4DBuilderDifferentSides_Space_longest::groupNodesAtDepth(int depth)
{
	assert(depth >= 0);
	assert(depth <= maxDepth);

	if (depth <= nbOfQueuesOf8Nodes - 1)
		//depth in [0, nbOfQueuesOf2nodes - 1]
		//index in queuesOfMax2 : [0, nbOfQueuesOf2nodes - 1]
	{
		return groupNodesOfMax8(queuesOfMax8[depth]);
	}
	else
		//depth in [nbOfQueuesOf2nodes, totalNbOfQueues - 1]    (remember: maxDepth = totalNbOfQueues - 1
		//index in queuesOfMax16 : [0, nbOfQueuesOf16nodes - 1]
	{
		return groupNodesOfMax16(queuesOfMax16[depth - nbOfQueuesOf8Nodes]);
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
Node4D Tree4DBuilderDifferentSides_Space_longest::groupNodesOfMax8(const QueueOfNodes &queueOfMax8)
{
	Node4D parent = Node4D();
	bool first_stored_child = true;

	//for each of the 8 child nodes in the full queue
	for (int indexOfCurrentChildNode = 0; indexOfCurrentChildNode < 8; indexOfCurrentChildNode++) {
		Node4D currentChildNode = queueOfMax8[indexOfCurrentChildNode];
		if (!currentChildNode.isNull()) {
			writeNodeToDiskAndSetOffsetOfParent_Max8NodesInQueue(parent, first_stored_child, indexOfCurrentChildNode, currentChildNode);
		}
		else {
			setChildrenOffsetsForNodeWithMax8Children(parent, indexOfCurrentChildNode, NOCHILD);
		}
	}

	// SIMPLE LEVEL CONSTRUCTION
	if (generate_levels) {
		VoxelData voxelData = VoxelData();
		float notnull = 0.0f;
		for (int i = 0; i < 8; i++) { // this node has no data: need to refine
			if (!queueOfMax8[i].isNull()){
				notnull++;
			}
			voxelData.color += queueOfMax8[i].data_cache.color;
			voxelData.normal += queueOfMax8[i].data_cache.normal;
		}
		voxelData.color = voxelData.color / notnull;
		vec3 tonormalize = (vec3)(voxelData.normal / notnull);
		voxelData.normal = normalize(tonormalize);
		// set it in the parent node
		svo_algorithm_timer.stop();		// TIMING
		svo_io_output_timer.start();	// TIMING
		parent.data = dataWriter->writeVoxelData(voxelData);
		svo_io_output_timer.stop();		// TIMING
		svo_algorithm_timer.start();	// TIMING
		parent.data_cache = voxelData;
	}
	return parent;
}

void Tree4DBuilderDifferentSides_Space_longest::writeNodeToDiskAndSetOffsetOfParent_Max8NodesInQueue(Node4D& parent, bool& first_stored_child, int indexOfCurrentChildNode, Node4D currentChildNode)
{
	//store the node on disk
	svo_algorithm_timer.stop();		// TIMING
	svo_io_output_timer.start();	// TIMING
	size_t positionOfChildOnDisk = nodeWriter->writeNode4D_(currentChildNode);
	svo_io_output_timer.stop();		// TIMING
	svo_algorithm_timer.start();	// TIMING

	if (first_stored_child) {
		parent.children_base = positionOfChildOnDisk;
		setChildrenOffsetsForNodeWithMax8Children(parent, indexOfCurrentChildNode, 0);
		first_stored_child = false;
	}
	else { //NOTE: we can have only two nodes, we know this is the second node
		char offset = (char)(positionOfChildOnDisk - parent.children_base);
		setChildrenOffsetsForNodeWithMax8Children(parent, indexOfCurrentChildNode, offset);
	}
}


/*
Set in the node 'parent' the offset of its child node.
The offset is 'offset'.
'indexInQueue' is the index of the child in the queue we are currently grouping.
*/
void Tree4DBuilderDifferentSides_Space_longest::setChildrenOffsetsForNodeWithMax8Children(Node4D &parent, int indexInQueue, char offset)
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


//	if( offset == -1)
//	{
//		std::cout << "Why is offset -1? "  << endl;
//	}
//
//	std::cout << "offset: " << (int) offset << endl;
//	std::cout << "parent.children_offset[indexInQueue] : " << (int)parent.children_offset[indexInQueue] << endl;
//	std::cout << "parent.children_offset[8 + indexInQueue] : " << (int)parent.children_offset[8 + indexInQueue] << endl;
}

void Tree4DBuilderDifferentSides_Space_longest::clearQueueAtDepth(int depth)
{
	assert(depth >= 0);
	assert(depth <= maxDepth);

	if (depth <= nbOfQueuesOf8Nodes - 1)
		//depth in [0, nbOfQueuesOf2nodes - 1]
		//index in queuesOfMax2 : [0, nbOfQueuesOf2nodes - 1]
	{
		queuesOfMax8[depth].clear();
	}
	else
		//depth in [nbOfQueuesOf2nodes, totalNbOfQueues - 1]    (remember: maxDepth = totalNbOfQueues - 1
		//index in queuesOfMax16 : [0, nbOfQueuesOf16nodes - 1]
	{
		queuesOfMax16[depth - nbOfQueuesOf8Nodes].clear();
	}
}

Node4D Tree4DBuilderDifferentSides_Space_longest::getRootNode()
{
	if (nbOfQueuesOf8Nodes > 0)
	{
		return queuesOfMax8[0][0];
	}
	else
	{
		return queuesOfMax16[0][0];
	}
}

size_t Tree4DBuilderDifferentSides_Space_longest::nbOfEmptyVoxelsAddedByAddingAnEmptyNodeAtDepth(int depth)
{
//	if (depth >= nbOfQueuesOf8Nodes - 1) // depth >= y - x
//	{
//		int nbOfNodesAdded = pow(16.0, maxDepth - depth);
//		return nbOfNodesAdded;
//	}
//	else {
//		// OLD:  nbOFNodesToAdd = 2^(4x + D) = pow(gridsize_S, 4) * 2^D
//		//NEW :  nbOFNodesToAdd = 2^(3x + y) = pow(gridsize_S, 3) * 2^D met D = x - y
//
//
//		size_t leftFactor = pow(gridsize_T, 4.0);
//		// SPECIAL: 8 = 2^3
//		size_t D = 3 * (maxDepth - nbOfQueuesOf16Nodes - depth);
//		size_t nbOfNodesAdded = leftFactor * (2 << D); 
//		return nbOfNodesAdded;
//	}

	//POGING 2
	size_t factor_8 = 1;
	size_t factor_16 = 1;
	size_t nbOfNodesAdded = 1;
	if(depth >= nbOfQueuesOf8Nodes - 1)
	{
		factor_16 = static_cast<size_t>(pow(16.0, maxDepth - depth));
	} else
	{
		factor_16 = static_cast<size_t>(pow(16.0, nbOfQueuesOf16Nodes));
		factor_8 = static_cast<size_t>(pow(8.0, nbOfQueuesOf8Nodes - 1 - depth));
	}

	nbOfNodesAdded = factor_8 * factor_16;
	return nbOfNodesAdded;

}

int Tree4DBuilderDifferentSides_Space_longest::calculateQueueShouldItBePossibleToAddAllVoxelsAtOnce(const size_t nbOfEmptyNodesToAdd)
{
	//STEL: we hebben enkel queues van 16 nodes
	if (nbOfQueuesOf8Nodes == 0)
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

		size_t a = findPowerOf16(nbOfEmptyNodesToAdd);

		size_t suggestedDepth = maxDepth - a;
		assert(suggestedDepth >= 0);
		assert(suggestedDepth <= maxDepth);

		return suggestedDepth;
	}
	else {
		/*
		gridsize_t = 2^y
		gridsize_s = 2^x  with y < x

		#vox = 2^(3x+y)  met D = x - y >= 0

		maxDepth = log2( max(gridsize_s, gridsize_t) ) ~> log2(gridsize_s) = x
		totalNbOfQueues = maxDepth + 1  ~> x + 1

		nbOfQueuesOf16Nodes = log2(gridsize_t) = y
		nbOfQueuesOf8Nodes = totalNbOfQueues - nbOfQueuesOf16Nodes ~> x + 1 - y = D + 1
		*/

		// we hebben y     queues van 16 nodes
		//			 D + 1 queues van  8 nodes
		// 
		// 16^y nodes toevoegen  
		//		= 2^4y nodes toevoegen
		//			--> 1 node toevoegen op maxDepth - y = x - y (onderste queue met 2 nodes)
		// ...
		// 2 * 16^y nodes toevoegen
		//		= 2^(4y + 1) nodes toevoegen
		//			--> 2 nodes toevoegen op maxDepth - y 
		//									= x- y (onderste queue met 2 nodes)
		//			--> 1 node toevoegen op maxDepth - y - 1 
		//									= x - y - 1
		// 4 * 16^y nodes toevoegen
		//		= 2^(4y + 2) nodes toevoegen
		//			-->  2 nodes toevoegen op maxDepth - y - 1
		//									= x - y - 1
		//			-->  1 node toevoegen op maxDepth - y - 2
		//									= x - y - 2
		// ... 
		// D * 16^x nodes toevoegen  = #vox
		//		= 2^(4y + D) nodes toevoegen
		//			--> 1 node toevoegen op maxDepth - y - D
		//									= x - y - D
		//									= 0   (bovenste queue)
		if (nbOfEmptyNodesToAdd < pow(gridsize_T, 4.0) * 8) {
			// nbOfEmptyNodesToAdd < 2^(4y + 1)
			int a = findPowerOf16(nbOfEmptyNodesToAdd);

			int suggestedDepth = maxDepth - a; //y - a
			assert(suggestedDepth >= maxDepth - nbOfQueuesOf16Nodes);
			assert(suggestedDepth <= maxDepth);

			return suggestedDepth;
		}
		else {
			// nbOfEmptyNodesToAdd >= 2^(4y + 1) ( = 2 * 16^y)
			size_t factor = nbOfEmptyNodesToAdd / pow(gridsize_T, 4.0);
			// SPECIAL: 
			int a = static_cast<int>(log(factor) / log(8));
			//int a = log2(factor); //a in {1, 2, ..., D}
			int suggestedDepth = maxDepth - nbOfQueuesOf16Nodes - a; //x - y - a
			assert(suggestedDepth >= 0);
			assert(suggestedDepth <= maxDepth - nbOfQueuesOf16Nodes - 1);

			return suggestedDepth;
		}
	}
}

/*
Use this method to push back a node to the lowest queue
instead of pushing it to the last queue in queuesOfMax16

REASON: handling the special case where gridsize_t = 1
==> nbOfQueuesOf16Nodes == 0
==> queuesOfMax16.size() == 0
==> 
	add the new leaf node to the lowest queue
	queuesOfMax16.at(nbOfQueuesOf16Nodes - 1).push_back(node); --> crash
*/
void Tree4DBuilderDifferentSides_Space_longest::push_backNodeToLowestQueue(Node4D& node)
{
	if(gridsize_T> 1)
	{
		queuesOfMax16.at(nbOfQueuesOf16Nodes - 1).push_back(node);
	}
	else //gridsize_T ==1
	{
		queuesOfMax8.at(nbOfQueuesOf8Nodes - 1).push_back(node);
	}
}
