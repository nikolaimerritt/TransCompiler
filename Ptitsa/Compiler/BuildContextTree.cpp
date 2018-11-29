#include <string>
#include <vector>

#include "BuildContextTree.h"
#include "BuildAST.h"

BuildContextTree::ContextTree::ContextTree(Lexer::LexemeLine::Type type, BuildAST::PASTNode && root) :
	type(type),
	root(std::move(root)) 
{
	root = nullptr;
}

std::vector<BuildContextTree::ContextTree> BuildContextTree::generateContextTrees(std::vector<Lexer::LexemeLine> const & lexemeDoc)
{
	std::vector<ContextTree> trees;
	for (Lexer::LexemeLine const & line : lexemeDoc)
	{
		BuildAST::PASTNode node = std::make_unique<BuildAST::ASTNode>();
		BuildAST::generateAST(line, node);
		ContextTree tree(line.type, std::move(node));
		trees.push_back(std::move(tree));
	}
	return trees;
}