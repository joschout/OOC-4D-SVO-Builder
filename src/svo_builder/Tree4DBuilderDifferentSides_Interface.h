#ifndef TREE4DBUILDERDIFFERENTSIDES_INTERFACE_H
#define TREE4DBUILDERDIFFERENTSIDES_INTERFACE_H

#include "VoxelData.h"
#include "Node4D.h"
#include "TreeDataWriter_CppStyle.h"
#include "TreeNodeWriter_CppStyle.h"
#include <memory>

typedef vector<Node4D> QueueOfNodes;

/*
1) Note: This is an ABSTRACT BASE CLASS.
It thus can only be used as a base class.
It thus is allowed to have virtual function members without definition ( = pure virtual functions ).
The syntax is to replace their definition by =0 (an equal sign and a zero).
Classes that contain at least one pure virtual function are known as abstract base classes.
Abstract base classes cannot be used to instantiate objects

2) Note: virtual methods are used in dynamic dispatch.
I.e. when using polymorphism, when using a pointer to this base class,
a function call to a virtual function will use the appropriate function in the derived class.

A virtual member is a member function that can be redefined in a derived class,
while preserving its calling properties through references.

*/
class Tree4DBuilderDifferentSides_Interface //ABSTRACT BASE CLASS
{
public:
	virtual ~Tree4DBuilderDifferentSides_Interface()
	{
	}

	void addVoxel(const uint64_t morton_number);
	void addVoxel(const VoxelData& point);

	virtual void initializeBuilder() = 0;
	void finalizeTree();

protected:
	// === protected constructor: reminder this is an abstract base class === //
	Tree4DBuilderDifferentSides_Interface(size_t gridsize_s, size_t gridsize_t,
		uint64_t current_morton_code, uint64_t max_morton_code,
		int maxDepth, int totalNbOfQueues,
		bool generate_levels, string base_filename);


	//================ VARIABLES==================//
	size_t gridsize_S;
	size_t gridsize_T;

	uint64_t current_morton_code; // current morton position
	uint64_t max_morton_code; // maximum morton position

	unique_ptr<TreeNodeWriterCppStyle> nodeWriter;
	unique_ptr<TreeDataWriterCppStyle> dataWriter;
	
	//IMPORTANT NOTE: depth of the root = 0
	int maxDepth; //max depth in tree. Also the amount of queues - 1 
	int totalNbOfQueues; //nb of queues = maxDepth + 1

	// configuration
	bool generate_levels; // switch to enable basic generation of higher octree levels

	string base_filename;

	//================ FUNCTIONS =================//

	void calculateMaxMortonCode();
	
	void addEmptyVoxel(const int queueDepth);
	void slowAddEmptyVoxels(const size_t nbOfNodesToAdd);
	virtual int nbOfEmptyVoxelsAddedByAddingAnEmptyNodeAtDepth(int depth) = 0;
	
	// --- fast adding empty voxels --- //
	void fastAddEmptyVoxels(const size_t nbOfEmptyVoxelsToAdd);
	int computeDepthOfBestQueue(const size_t nbofEmptyVoxelsToAdd);
	virtual int calculateQueueShouldItBePossibleToAddAllVoxelsAtOnce(const size_t nbOfEmptyNodesToAdd) = 0;
	int highestNonEmptyQueue();
	//void fastAddEmptyVoxels(const size_t budget);
	//int computeBestFillQueue(const size_t budget);


	// Interface for putting a node in the right queue
	virtual void push_backNodeToLowestQueue(Node4D& node) = 0;
	virtual void push_backNodeToQueueAtDepth(int depth, Node4D& node) = 0;
	virtual bool isQueueFilled(int depth) = 0;
	virtual bool isQueueEmpty(int depth) = 0;
	virtual bool doesQueueContainOnlyEmptyNodes(int depth) = 0;
	static bool doesQueueContainOnlyEmptyNodes(const QueueOfNodes &queue, int maxAmountOfElementsInQueue);
	
	virtual Node4D groupNodesAtDepth(int depth) = 0;
	Node4D groupNodesOfMax16(const QueueOfNodes& queueOfMax16);

	virtual void clearQueueAtDepth(int depth) = 0;
	void flushQueues(const int start_depth);

	void writeNodeToDiskAndSetOffsetOfParent_Max16NodesInQueue(Node4D& parent, bool& first_stored_child, int indexOfCurrentChildNode, Node4D currentChildNode);
	
	virtual Node4D getRootNode() = 0;

};


#endif