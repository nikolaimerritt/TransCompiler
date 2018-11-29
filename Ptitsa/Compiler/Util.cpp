#include "Util.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <queue>

using namespace Util;
using std::static_pointer_cast;

std::vector<std::string> Util::splitStringBy(std::string const & toSplit, char splitBy)
{
	std::stringstream toSplitStream(toSplit);
	std::string segment;
	std::vector<std::string> segments;

	while (std::getline(toSplitStream, segment, splitBy)) segments.push_back(segment);

	return segments;
}

void Util::replaceAll(std::string & parent, std::string const & from, std::string const & to)
{
	if (from.empty()) return;

	size_t startPosition = 0;
	while ((startPosition = parent.find(from, startPosition)) != std::string::npos)
	{
		parent.replace(startPosition, from.length(), to);
		startPosition += to.length();
	}
}

char Util::lastNonWhitespace(std::string const & string)
{
	for (unsigned i = string.size() - 1; i >= 0; --i)
	{
		if (string[i] != ' ' && string[i] != '\t' && string[i] != '\n') return string[i];
	}
	return ' ';
}

void Util::removeLast(std::string & string, char toRemove)
{
	for (unsigned i = string.size() - 1; i >= 0; --i)
	{
		if (string[i] == toRemove)
		{
			string.erase(i, 1);
			return;
		}
	}
}

bool Util::isNumber(std::string const & string)
{
	if (!(isDigit(string[0]) || string[0] == '.' || string[0] == '-')) return false;
	if (string == "-.") return false;
	if (string == "-") return false;

	bool decimalPointSeen = string[0] == '.';

	for (unsigned i = 1; i < string.size(); ++i)
	{
		if (string[i] == '.')
		{
			if (decimalPointSeen) return false;
			decimalPointSeen = true;
		}
		else if (!isDigit(string[i])) return false;
	}
	return true;
}

bool Util::isDigit(char c) { return c >= '0' && c <= '9'; }

std::string Util::toDecimal(std::string const & number)
{
	for (unsigned i = 0; i < number.size(); i++)
	{
		if (number[i] == '.') return number;
	}
	return number + ".0";
}

void Util::mollysPrintAST(BuildAST::PASTNode const & node, unsigned level)
{
	for (unsigned i = 0; i <= level; ++i) std::cout << "   ";
	std::cout << " " << node->lex << std::endl;

	for (BuildAST::PASTNode const & child : node->children)
	{
		mollysPrintAST(child, level + 1);
	}
}

void Util::mollysPrintAST(BuildAST::PASTNode const & root) { mollysPrintAST(root, 0); }

int Util::bracketPolarity(Lexer::LexemeLine const & line, unsigned start, unsigned end)
{
	int polarity = 0;
	for (unsigned i = start; i <= end; i++)
	{
		if (line[i]->isSymbol())
		{
			Lexer::Symbol::Type const symbolType = std::static_pointer_cast<Lexer::Symbol>(line[i])->type;

			switch (symbolType)
			{
				case Lexer::Symbol::OPEN_BRACKET:	polarity++;		break;
				case Lexer::Symbol::CLOSE_BRACKET:	polarity--;		break;
			}
		}
	}
	return polarity;
}

int Util::bracketPolarity(Lexer::LexemeLine const & line, unsigned end) { return bracketPolarity(line, 0, end); }
int Util::bracketPolarity(Lexer::LexemeLine const & line) { return bracketPolarity(line, 0, line.size() - 1); }

int Util::closingBracketIndex(Lexer::LexemeLine const & line, unsigned start = 0)
{
	for (unsigned i = start; i < line.size(); ++i)
	{
		if (bracketPolarity(line, start, i) == 0) return i;
	}
	return -1;
}

std::vector<std::vector<Lexer::PLexeme>> Util::commandArguments(Lexer::LexemeLine const & line, unsigned openingBracketIdx)
{
	// Command call enclosed by brackets, so argsEndIdx is closing bracket idx - 1
	unsigned const argsEndIdx = closingBracketIndex(line, openingBracketIdx) - 1;

	// args separated by ','
	std::vector<std::vector<Lexer::PLexeme>> arguments;
	std::vector<Lexer::PLexeme> arg;

	for (unsigned i = openingBracketIdx + 2; i <= argsEndIdx; ++i)
	{
		if (line[i]->isSymbol())
		{
			Lexer::Symbol::Type const type = static_pointer_cast<Lexer::Symbol>(line[i])->type;

			if (type == Lexer::Symbol::OPEN_BRACKET)
			{
				unsigned closeIdx = closingBracketIndex(line, i);
				arg.insert(arg.end(), line.begin() + i, line.begin() + closeIdx + 1);
				arguments.push_back(arg);
				arg.clear();
				i = closeIdx;
			}
			else if (type == Lexer::Symbol::ARGS_SEP && !arg.empty())
			{
				arguments.push_back(arg);
				arg.clear();
			}
		}
		else arg.push_back(line[i]);
	}

	if (!arg.empty()) arguments.push_back(arg);

	return arguments;
}