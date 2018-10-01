#ifndef CONTEXT_TREE_INCLUDE
#define CONTEXT_TREE_INCLUDE

#include "Lexer.h"
#include "BuildAST.h"
#include <vector>

namespace BuildContextTree
{
	struct ContextTree
	{
		NewLexer::LexemeLine::Type type;
		BuildAST::PASTNode root;

		ContextTree(NewLexer::LexemeLine::Type type, BuildAST::PASTNode && root);
	};

	std::vector<ContextTree> generateContextTrees(std::vector<NewLexer::LexemeLine> const & lexemeDoc);
}

#endif // !CONTEXT_TREE_INCLUDE 
