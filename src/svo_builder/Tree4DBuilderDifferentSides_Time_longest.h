#ifndef TREE4DBUILDERDIFFERENTSIDES_H
#define TREE4DBUILDERDIFFERENTSIDES_H
#include <vector>
#include "Node4D.h"
#include "TreeNodeWriter.h"
#include "TreeDataWriter.h"
#include "Tree4DBuilderDifferentSides_Interface.h"

using std::vector;

typedef vector<Node4D> QueueOfNodes;

class Tree4DBuilderDifferentSides_Time_longest: public Tree4DBuilderDifferentSides_Interface
{
public:
	vector<QueueOfNodes> queuesOfMax2;
	vector<QueueOfNodes> queuesOfMax16;

	//amount of queues = maxDepth + 1
	int nbOfQueuesOf16Nodes;
	int nbOfQueuesOf2Nodes;

	//========================
	Tree4DBuilderDifferentSides_Time_longest();
	Tree4DBuilderDifferentSides_Time_longest(std::string base_filename, size_t gridsize_S, size_t gridsize_T, bool generate_levels);

	void initializeBuilder();
	
private:

	// Grouping nodes
	Node4D groupNodesAtDepth(int depth) override;
	Node4D groupNodesOfMax2(const QueueOfNodes& queueOfMax2);
	void writeNodeToDiskAndSetOffsetOfParent_Max2NodesInQueue(Node4D& parent, bool& first_stored_child, int indexOfCurrentChildNode, Node4D currentChildNode);
	static void setChildrenOffsetsForNodeWithMax2Children(Node4D& parent, int indexInQueue, char offset);
	//Node4D groupNodes(const vector<Node4D>& buffer, int maxAmountOfElementsInQueue);
	
	void clearQueueAtDepth(int depth) override;
	
	Node4D getRootNode() override;
	
	int nbOfEmptyVoxelsAddedByAddingAnEmptyNodeAtDepth(int depth) override;

	int calculateQueueShouldItBePossibleToAddAllVoxelsAtOnce(const size_t nbOfEmptyVoxelsToAdd) override;

	// Interface for putting a node in the right queue
	void push_backNodeToLowestQueue(Node4D& node) override;
	void push_backNodeToQueueAtDepth(int depth, Node4D& node) override;
	bool isQueueFilled(int depth) override;
	bool isQueueEmpty(int depth) override;
	bool doesQueueContainOnlyEmptyNodes(int depth) override;
	using Tree4DBuilderDifferentSides_Interface::doesQueueContainOnlyEmptyNodes;
};


#endif