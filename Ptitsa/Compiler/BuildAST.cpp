#include "BuildAST.h"
#include "Util.h"
#include "NewLexer.h"
#include <iostream>
#include <vector>
#include <algorithm>

BuildAST::ASTNode::ASTNode():
	lex(nullptr),
	children()
{ }

BuildAST::ASTNode::ASTNode(NewLexer::PLexeme const & lex):
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

BuildAST::ASTNode::ASTNode(NewLexer::PLexeme const & lex, std::vector<BuildAST::PASTNode> && tempChildren) :
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


void BuildAST::generateAST(NewLexer::LexemeLine const & line, BuildAST::PASTNode & root)
{
	using namespace NewLexer;
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

		if (fn.type == NewLexer::Function::PREFIX) // is prefix function
		{
			const std::vector<std::vector<NewLexer::PLexeme>> args = Util::commandArguments(line, maxOrderIdx - 1);
			for (const std::vector<NewLexer::PLexeme>& argument : args)
			{
				PASTNode node = std::make_unique<ASTNode>();
				const NewLexer::LexemeLine argAsLexemeLine(argument);
				generateAST(argAsLexemeLine, node);
				root->add(std::move(node));
			}
		}
		else if (fn.type == NewLexer::Function::INFIX) // is two-arg operator
		{
			NewLexer::LexemeLine const leftArgs = line.makeCopyBetween(0, maxOrderIdx - 1); // End NOT inclusive, hence no -1
			PASTNode leftArgsNodes = std::make_unique<ASTNode>();
			generateAST(leftArgs, leftArgsNodes);
			root->add(std::move(leftArgsNodes));

			NewLexer::LexemeLine const rightArgs = line.makeCopyBetween(maxOrderIdx + 1, line.size() - 1);
			PASTNode rightArgsNodes = std::make_unique<ASTNode>();
			generateAST(rightArgs, rightArgsNodes);
			root->add(std::move(rightArgsNodes));
		}
	}

	else
	{
		for (NewLexer::PLexeme const & lex : line)
		{
			if (!lex->isSymbol()) root->lex = lex;
		}
	}
}

void BuildAST::setASTs(std::vector<NewLexer::LexemeLine> const & lexemeDoc, std::vector<PASTNode> & nodes)
{
	for (NewLexer::LexemeLine const & line : lexemeDoc)
	{
		if (line.isNotEmpty())
		{
			PASTNode node = std::make_unique<ASTNode>();
			generateAST(line, node);
			nodes.push_back(std::move(node));
		}
	}
}
