#include "BuildAST.h"
#include "Util.h"
#include "Lexer.h"
#include <iostream>
#include <vector>
#include <algorithm>

BuildAST::ASTNode::ASTNode():
	lex(nullptr),
	children()
{ }

BuildAST::ASTNode::ASTNode(Lexer::PLexeme const & lex):
	lex(lex),
	children()
{ }


BuildAST::ASTNode::ASTNode(BuildAST::ASTNode && temp) :
	lex(temp.lex),
	children(std::move(temp.children))
{
	temp.children.clear();
	temp.lex = nullptr;
}

BuildAST::ASTNode::ASTNode(Lexer::PLexeme const & lex, std::vector<BuildAST::PASTNode> && tempChildren) :
	lex(lex),
	children(std::move(tempChildren))
{
	tempChildren.clear();
}

BuildAST::ASTNode::~ASTNode() { }

void BuildAST::ASTNode::add(BuildAST::PASTNode && node) 
{ 
	children.push_back(std::move(node));
	node = nullptr;
}


void BuildAST::generateAST(Lexer::LexemeLine const & line, BuildAST::PASTNode & root)
{
	using namespace Lexer;
	using std::static_pointer_cast;

	if (line.size() < 1) return;

	unsigned maxOrder = 0;
	unsigned maxOrderIdx = 0;
	bool functionFound = false;

	for (unsigned i = 0; i < line.size(); ++i)
	{
		if (line[i]->isFunction())
		{
			unsigned const order = static_pointer_cast<Function>(line[i])->order;
			if (order > maxOrder)
			{
				functionFound = true;
				maxOrder = order;
				maxOrderIdx = i;
			}
		}
	}

	if (functionFound)
	{
		root->lex = line[maxOrderIdx];
		Function const fn = * static_pointer_cast<Function>(root->lex);

		if (fn.type == Lexer::Function::PREFIX) // is prefix function
		{
			const std::vector<std::vector<Lexer::PLexeme>> args = Util::commandArguments(line, maxOrderIdx - 1);
			for (const std::vector<Lexer::PLexeme>& argument : args)
			{
				PASTNode node = std::make_unique<ASTNode>();
				const Lexer::LexemeLine argAsLexemeLine(argument);
				generateAST(argAsLexemeLine, node);
				root->add(std::move(node));
			}
		}
		else if (fn.type == Lexer::Function::INFIX) // is two-arg operator
		{
			Lexer::LexemeLine const leftArgs = line.makeCopyBetween(0, maxOrderIdx - 1); // End NOT inclusive, hence no -1
			PASTNode leftArgsNodes = std::make_unique<ASTNode>();
			generateAST(leftArgs, leftArgsNodes);
			root->add(std::move(leftArgsNodes));

			Lexer::LexemeLine const rightArgs = line.makeCopyBetween(maxOrderIdx + 1, line.size() - 1);
			PASTNode rightArgsNodes = std::make_unique<ASTNode>();
			generateAST(rightArgs, rightArgsNodes);
			root->add(std::move(rightArgsNodes));
		}
	}

	else
	{
		for (Lexer::PLexeme const & lex : line)
		{
			if (!lex->isSymbol()) root->lex = lex;
		}
	}
}

void BuildAST::setASTs(std::vector<Lexer::LexemeLine> const & lexemeDoc, std::vector<PASTNode> & nodes)
{
	for (Lexer::LexemeLine const & line : lexemeDoc)
	{
		if (line.isNotEmpty())
		{
			PASTNode node = std::make_unique<ASTNode>();
			generateAST(line, node);
			nodes.push_back(std::move(node));
		}
	}
}
