#ifndef SYNTAX_TREE_INCLUDE
#define SYNTAX_TREE_INCLUDE

#include "Lexer.h"
#include <iostream>
#include <memory>
#include <vector>

namespace BuildAST
{
	struct ASTNode;
	typedef std::unique_ptr<ASTNode> PASTNode;

	struct ASTNode 
	{
		Lexer::PLexeme lex;
		std::vector<PASTNode> children;

		ASTNode();
		ASTNode(Lexer::PLexeme const & lex);
		ASTNode(ASTNode const & copy) = delete;
		ASTNode(ASTNode && temp);
		ASTNode(Lexer::PLexeme const & lex, std::vector<PASTNode> && tempChildren);

		~ASTNode();

		void add(PASTNode && node);
	};
		
	void generateAST(Lexer::LexemeLine const &, PASTNode &);
	void setASTs(std::vector<Lexer::LexemeLine> const & lexemeDoc, std::vector<PASTNode> & nodes);
}

#endif