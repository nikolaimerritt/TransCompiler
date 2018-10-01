#ifndef SYNTAX_TREE_INCLUDE
#define SYNTAX_TREE_INCLUDE

#include "NewLexer.h"
#include <iostream>
#include <memory>
#include <vector>

namespace BuildAST
{
	struct ASTNode;
	typedef std::unique_ptr<ASTNode> PASTNode;

	struct ASTNode 
	{
		NewLexer::PLexeme lex;
		std::vector<PASTNode> children;

		ASTNode();
		ASTNode(NewLexer::PLexeme const & lex);
		ASTNode(ASTNode const & copy) = delete;
		ASTNode(ASTNode && temp);
		ASTNode(NewLexer::PLexeme const & lex, std::vector<PASTNode> && tempChildren); 

		~ASTNode();

		void add(PASTNode && node);
	};
		
	void generateAST(NewLexer::LexemeLine const &, PASTNode &);	
	void setASTs(std::vector<NewLexer::LexemeLine> const & lexemeDoc, std::vector<PASTNode> & nodes);
}

#endif