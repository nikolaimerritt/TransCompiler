#ifndef UTIL_INCLUDE
#define UTIL_INCLUDE

#include <algorithm>
#include <iostream>
#include <vector>

#include "Lexer.h"
#include "NewLexer.h"
#include "BuildAST.h"
#include "BuildContextTree.h"

namespace Util
{
	std::vector<std::string> splitStringBy(std::string const & toSplit, char splitBy);
	//void printLexemeRow(const std::vector<Lexer::Lexeme>&);
	//void printLexemeDocument(const std::vector<Lexer::LexemeLine>&);
	
	template <typename T> bool couldSetIndex(std::vector<T> vector, T item, unsigned& index) 
	{ 
		unsigned position = std::distance(vector.begin(), std::find(vector.begin(), vector.end(), item));
		if (position < vector.size())
		{
			index = position;
			return true;
		}
		return false;
	}

	void removeLast(std::string&, char);
	void replaceAll(std::string & parent, std::string const & from, std::string const & to);
	char lastNonWhitespace(std::string const & string);
		
	bool isDigit(char);
	bool isNumber(std::string const & string);
	std::string toDecimal(std::string const & number);
		
	void mollysPrintAST(BuildAST::PASTNode const & node, unsigned level);
	void mollysPrintAST(BuildAST::PASTNode const & root);
	//void printContextTrees(const std::vector<BuildContextTree::ContextTree*>&);
	//void deleteContextTrees(std::vector<BuildContextTree::ContextTree*>&);
	
	int bracketPolarity(NewLexer::LexemeLine const & line);
	int bracketPolarity(NewLexer::LexemeLine const & line, unsigned end);
	int bracketPolarity(NewLexer::LexemeLine const & line, unsigned start, unsigned end);
	
	int closingBracketIndex(NewLexer::LexemeLine const & line, unsigned start);
	bool varAlreadyDefined(Lexer::Variable);

	std::vector<std::vector<NewLexer::PLexeme>> commandArguments(NewLexer::LexemeLine const & line, unsigned openingBracketIdx);

	// NewLexer
	void printNewLexemeLine(NewLexer::LexemeLine const &);
}

#endif