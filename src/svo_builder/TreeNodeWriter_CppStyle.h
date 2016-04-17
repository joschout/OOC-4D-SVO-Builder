/*
#ifndef TREENODEWRITERCPPSTYLE_H
#define TREENODEWRITERCPPSTYLE_H
#include <stdio.h>
#include "Node4D.h"

class TreeNodeWriterCppStyle
{
public:
	size_t position_in_output_file; //NOTE: NOT in bytes, but in nodes
	//i.e. this is the nb of the node we will write next

	

	//TreeNodeWriterCppStyle();
	TreeNodeWriterCppStyle(std::string base_filename);
	~TreeNodeWriterCppStyle();
	void closeFile();
	size_t writeNode4D_(const Node4D &node);

private:
	std::fstream fstream_nodes;
};

#endif
*/
