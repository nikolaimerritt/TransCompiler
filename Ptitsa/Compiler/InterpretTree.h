#ifndef INTERPRET_TREE_INCLUDE
#define INTERPRET_TREE_INCLUDE

#include <string>
#include <vector>
#include <map>

#include "BuildAST.h"
#include "BuildContextTree.h"

namespace InterpretTree
{
	std::string treesToString(std::vector<BuildContextTree::ContextTree> const & trees);
}

#endif // !INTERPRET_TREE_INCLUDE
