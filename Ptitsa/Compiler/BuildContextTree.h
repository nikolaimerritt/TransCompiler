#ifndef CONTEXT_TREE_INCLUDE
#define CONTEXT_TREE_INCLUDE

#include "Lexer.h"
#include "BuildAST.h"
#include <vector>

namespace BuildContextTree
{
	struct ContextTree
	{
		Lexer::LexemeLine::Type type;
		BuildAST::PASTNode root;

		ContextTree(Lexer::LexemeLine::Type type, BuildAST::PASTNode && root);
	};

	std::vector<ContextTree> generateContextTrees(std::vector<Lexer::LexemeLine> const & lexemeDoc);
}

#endif // !CONTEXT_TREE_INCLUDE 
