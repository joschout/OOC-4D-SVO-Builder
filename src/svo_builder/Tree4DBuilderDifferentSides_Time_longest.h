#ifndef TREE4DBUILDERDIFFERENTSIDES_H
#define TREE4DBUILDERDIFFERENTSIDES_H
#include <vector>
#include "Node4D.h"
#include "TreeNodeWriter.h"
#include "TreeDataWriter.h"
#include "Tree4DBuilderDifferentSides_Interface.h"
#include "TreeNodeWriter_CppStyle.h"
#include <memory>
#include "TreeDataWriter_CppStyle.h"

using std::vector;


typedef vector<Node4D> QueueOfNodes;

class Tree4DBuilderDifferentSides_Time_longest: public Tree4DBuilderDifferentSides_Interface
{
public:
	unique_ptr<TreeNodeWriterCppStyle> nodeWriter;
	unique_ptr<TreeDataWriterCppStyle> dataWriter;


	vector<QueueOfNodes> queuesOfMax2;
	vector<QueueOfNodes> queuesOfMax16;

	//vector<QueueOfNodes> nodeQueues;

	size_t gridsize_S;
	size_t gridsize_T;

	uint64_t current_morton_code; // current morton position
	uint64_t max_morton_code; // maximum morton position

	//IMPORTANT NOTE: depth of the root = 0
	int maxDepth; //max depth in tree. Also the amount of queues - 1 
	int totalNbOfQueues;

	//amount of queues = maxDepth + 1
	int nbOfQueuesOf16Nodes;
	int nbOfQueuesOf2Nodes;

	// configuration
	bool generate_levels; // switch to enable basic generation of higher octree levels

	string base_filename;

	//File IO
//	TreeNodeWriter nodeWriter;
//	TreeDataWriter dataWriter;


	//========================
	Tree4DBuilderDifferentSides_Time_longest();
	Tree4DBuilderDifferentSides_Time_longest(std::string base_filename, size_t gridsize_S, size_t gridsize_T, bool generate_levels);

	void addVoxel(const uint64_t morton_number);
	void addVoxel(const VoxelData& point);

	void initializeBuilder();
	void finalizeTree();
	
private:

	void calculateMaxMortonCode();

	// Grouping nodes
	Node4D groupNodesAtDepth(int depth);
	Node4D groupNodesOfMax2(const QueueOfNodes& queueOfMax2);
	Node4D groupNodesOfMax16(const QueueOfNodes& queueOfMax16);
	void writeNodeToDiskAndSetOffsetOfParent_Max2NodesInQueue(Node4D& parent, bool& first_stored_child, int indexOfCurrentChildNode, Node4D currentChildNode);
	void writeNodeToDiskAndSetOffsetOfParent_Max16NodesInQueue(Node4D& parent, bool& first_stored_child, int indexOfCurrentChildNode, Node4D currentChildNode);
	static void setChildrenOffsetsForNodeWithMax2Children(Node4D& parent, int indexInQueue, char offset);
	//Node4D groupNodes(const vector<Node4D>& buffer, int maxAmountOfElementsInQueue);


	void clearQueueAtDepth(int depth);
	void flushQueues(const int start_depth);
	static bool doesQueueContainOnlyEmptyNodes(const QueueOfNodes &queue, int maxAmountOfElementsInQueue);
	
	Node4D getRootNode();
	
	//void fastAddEmptyVoxels(const size_t budget);
	//int computeBestFillQueue(const size_t budget);
	void fastAddEmptyVoxels(const size_t budget);
	int nbOfVoxelsAddedByAddingAnEmptyVoxelAtDepth(int depth);
	int computeDepthOfBestQueue(const size_t nbOfEmptyVoxelsToAdd);
	int calculateQueueShouldItBePossibleToAddAllVoxelsAtOnce(const size_t nbOfEmptyVoxelsToAdd);
	int highestNonEmptyQueue();

	void slowAddEmptyVoxels(const size_t nbOfNodesToAdd);
	
	void addEmptyVoxel(const int buffer);
	
	// Interface for putting a node in the right queue
	void push_backNodeToQueueAtDepth(int depth, Node4D node);
	bool isQueueFilled(int depth);
	bool isQueueEmpty(int depth);
	bool doesQueueContainOnlyEmptyNodes(int depth);
};


#endif
