#include "Lexer.h"
#include "Util.h"
#include "Mistake.h"
#include <stdexcept>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <set>
#include <map>
#include <algorithm>


std::vector<std::set<Lexer::Function>> const Lexer::opsBeforeCommands = {
	{ Lexer::Function("^", "^", Lexer::Function::INFIX) },
	{
		Lexer::Function("*", "*", Lexer::Function::INFIX),
		Lexer::Function("/", "/", Lexer::Function::INFIX)
	}, 
	{
		Lexer::Function("+", "+", Lexer::Function::INFIX),
		Lexer::Function("-", "-", Lexer::Function::INFIX)
	}
};

std::vector<std::set<Lexer::Function>> const Lexer::opsAfterCommands = {
	{ 
		Lexer::Function("is", "==", Lexer::Function::INFIX),
		Lexer::Function("isnt", "!=", Lexer::Function::INFIX)
	},
	{ Lexer::Function("and", "&&", Lexer::Function::INFIX) },
	{ Lexer::Function("or", "||", Lexer::Function::INFIX) },
	{ Lexer::Function("not", "!", Lexer::Function::PREFIX, 1) },
	{ Lexer::Function("=", "=", Lexer::Function::INFIX) }
};

std::set<Lexer::Function> Lexer::commands = {
	Lexer::Function("show", "Library::show", Lexer::Function::PREFIX, -1),
	Lexer::Function("exp", "Library::exp", Lexer::Function::PREFIX, 1)
};

Lexer::Lexeme::Lexeme(std::string value, Type type):	
	processed(false),
	order(0),
	value(value),
	type(type) 
{ }

bool Lexer::Lexeme::operator==(const Lexer::Lexeme& other) { return type == other.type && value == other.value; }

Lexer::Variable::Variable() {  }

Lexer::Variable::Variable(std::string identifier, unsigned lineNumber, unsigned depth) :	
	identifier(identifier),
	lineNumber(lineNumber),
	depth(depth) 
{  }
																					
bool Lexer::Variable::operator==(Variable other) const 
{ 
	return	this->identifier == other.identifier && 
			this->depth == other.depth && 
			this->lineNumber == other.lineNumber; 
}

bool Lexer::Variable::operator<(Variable other) const { return this->lineNumber < other.lineNumber; }

Lexer::LexemeLine::LexemeLine(const std::vector<Lexeme>& lexemes):	
	type(UNKNOWN),
	lexemes(lexemes) 
{ }

Lexer::LexemeLine::LexemeLine(Type type) :
	type(type),
	lexemes()
{ }

Lexer::Lexeme& Lexer::LexemeLine::operator[](int i) 
{
	return lexemes[i]; 
}
const Lexer::Lexeme& Lexer::LexemeLine::operator[](int i) const 
{
	return lexemes[i]; 
}

std::vector<Lexer::Lexeme>::iterator Lexer::LexemeLine::begin() 
{ 
	return lexemes.begin(); 
}
std::vector<Lexer::Lexeme>::const_iterator Lexer::LexemeLine::begin() const 
{ 
	return lexemes.begin(); 
}

std::vector<Lexer::Lexeme>::iterator Lexer::LexemeLine::end() 
{ 
	return lexemes.end(); 
}
std::vector<Lexer::Lexeme>::const_iterator Lexer::LexemeLine::end() const 
{ 
	return lexemes.end(); 
}

unsigned Lexer::LexemeLine::size() const 
{ 
	return lexemes.size(); 
}
void Lexer::LexemeLine::push_back(const Lexeme& toPushBack) 
{ 
	lexemes.push_back(toPushBack); 
}
void Lexer::LexemeLine::erase(int i) 
{ 
	lexemes.erase(lexemes.begin() + i); 
}
void Lexer::LexemeLine::insert(int i, const Lexeme& toInsert) 
{ 
	lexemes.insert(lexemes.begin() + i, toInsert); 
}
bool Lexer::LexemeLine::isNotEmpty()
{
	return !lexemes.empty();
}
Lexer::LexemeLine Lexer::LexemeLine::copyBetween(unsigned start, unsigned end) const
{
	if (end - start >= 0)
	{
		std::vector<Lexeme> copy;
		copy.reserve(end - start + 1);
		copy.insert(copy.begin(), lexemes.begin() + start, lexemes.begin() + end + 1); // end usually exclusive. want to be inclusive
		return LexemeLine(copy);
	}
	else return LexemeLine(UNKNOWN);
}

Lexer::Function::Function() { }

Lexer::Function::Function(std::string const & identifier, std::string const & asCpp, Type type) :
	identifier(identifier),
	asCpp(asCpp),
	type(type)
{
	if (type == INFIX) args = 2;
	else throw std::invalid_argument("Cannot deduce arg amount from non-infix function " + identifier);
}

Lexer::Function::Function(std::string const & identifier, std::string const & asCpp, Type type, int args) :
	identifier(identifier),
	asCpp(asCpp),
	type(type),
	args(args)
{
	if (type == INFIX && args != 2) throw std::invalid_argument("Cannot have an infix function which doesn't take two arguments.");
}

Lexer::Function::Function(std::string const & identifier, std::string const & asCpp):
	identifier(identifier),
	asCpp(asCpp),
	type(POSTFIX),
	args(0)
{ }

bool Lexer::Function::operator==(Function const & other) const 
{ 
	return identifier == other.identifier; 
}
bool Lexer::Function::operator<(Function const & other) const 
{ 
	return identifier < other.identifier; 
}

namespace
{
	using namespace Lexer;

	std::set<std::string> const keywords = { "if", "while" };
	std::set<Variable> definedVariables = { };

	unsigned depthOfLine(const LexemeLine& lexemeLine)
	{
		int depth = 0;
		for (unsigned i = 0; i < lexemeLine.size() && lexemeLine[i].type == Lexeme::DEPTH; i++) depth++;
		return depth;
	}

	unsigned lastIdxInScope(const std::vector<LexemeLine>& lexemeDoc, unsigned startIdx)
	{
		unsigned scopeAtStart = depthOfLine(lexemeDoc[startIdx]);
		for (unsigned r = startIdx + 1; r < lexemeDoc.size(); r++)
		{
			if (depthOfLine(lexemeDoc[r]) < scopeAtStart) return r - 1;
		}
		return lexemeDoc.size() - 1;
	}

	bool groupUnbroken(const std::vector<LexemeLine>& lexemeDoc, unsigned definitionRow, unsigned usageRow)
	{
		unsigned const depthOfDefinition = depthOfLine(lexemeDoc[definitionRow]);
		if (depthOfDefinition == 0) return true;

		for (unsigned r = definitionRow + 1; r < usageRow; r++)
		{
			if (depthOfLine(lexemeDoc[r]) < depthOfDefinition) return false;
		}
		return true;
	}

	std::vector<std::vector<std::string>> codeToLines(std::string code)
	{
		std::vector<std::vector<std::string>> allLines;
		std::vector<std::string> rows = Util::splitStringBy(code, '\n');

		for (std::string& line : rows)
		{
			std::vector<std::string> lineSplit;

			while (line[0] == '\t')
			{
				lineSplit.push_back("\t");
				line.erase(0, 1); // tab dealt with. not needed anymore
			}

			const std::vector<std::string> words = Util::splitStringBy(line, ' ');
			lineSplit.insert(lineSplit.end(), words.begin(), words.end());
			allLines.push_back(lineSplit);
		}

		return allLines;
	}

	std::vector<LexemeLine> docToUnidentifiedLexemeLines(const std::vector<std::vector<std::string>>& codeDocument)
	{
		std::vector<LexemeLine> lexemeLines;
		for (const std::vector<std::string>& codeLine : codeDocument)
		{
			std::vector<Lexeme> rowOfLexemes;
			for (const std::string& word : codeLine)
			{
				rowOfLexemes.push_back(Lexeme(word, Lexeme::UNKNOWN));
			}			
			if (!rowOfLexemes.empty())
			{
				lexemeLines.push_back(LexemeLine(rowOfLexemes));
			}
		}
		return lexemeLines;
	}

	void identifyDepthLexemes(std::vector<LexemeLine>& lexemeDoc)
	{
		for (LexemeLine& lexemeLine : lexemeDoc)
		{
			for (Lexeme& lex : lexemeLine)
			{
				if (lex.type == Lexeme::UNKNOWN && lex.value == "\t") lex.type = Lexeme::DEPTH;
				else break;
			}
		}
	}

	void identifyPhrases(LexemeLine& lexemeLine)
	{
		for (unsigned i = 0; i < lexemeLine.size(); i++)
		{
			if (lexemeLine[i].value[0] == '\"') // found start of phrase
			{
				lexemeLine[i].value.erase(0, 1); // dont want that quote anymore
				lexemeLine[i].type = Lexeme::PHRASE;

				// appending next words to current string,  until end of string found
				while (i + 1 < lexemeLine.size() && lexemeLine[i].value.back() != '\"')
				{
					lexemeLine[i].value += " " + lexemeLine[i + 1].value;
					lexemeLine.erase(i + 1); 
				}
				lexemeLine[i].value.pop_back(); // deleting '\"' character
			}
		}
	}

	void identifyNumbers(LexemeLine& lexemeLine)
	{
		for (Lexeme& lex : lexemeLine)
		{
			if (lex.type == Lexeme::UNKNOWN && Util::isNumber(lex.value)) lex.type = Lexeme::NUMBER;
		}
	}

	void identifyBooleans(LexemeLine& lexemeLine)
	{
		for (Lexeme& lex : lexemeLine)
		{
			if (lex.type == Lexeme::UNKNOWN &&
				(lex.value == "true" || lex.value == "false")) lex.type = Lexeme::BOOLEAN;
		}
	}

	bool groupContainsName(std::set<Function> const & group, std::string const & name)
	{
		for (Function const & function : group)
		{
			if (function.identifier == name) return true;
		}
		return false;
	}

	bool groupSetContainsName(std::vector<std::set<Function>> const & groups, std::string const & name)
	{
		for (std::set<Function> const & opGroup : groups)
		{
			if (groupContainsName(opGroup, name)) return true;
		}

		return false;
	}
	
	bool commandExistsWithName(std::string const & name)
	{
		for (Function const & command : commands)
		{
			if (command.identifier == name) return true;
		}
		return false;
	}

	void identifyOperators(LexemeLine & lexemeLine)
	{
		for (Lexeme & lex : lexemeLine)
		{
			if (lex.type == Lexeme::UNKNOWN)
			{
				if (lex.value == "(")		lex.type = Lexeme::REG_OPENING_BRACE;
				else if (lex.value == ")")	lex.type = Lexeme::REG_CLOSING_BRACE;
				else if (lex.value == ",")	lex.type = Lexeme::ARGS_SEP;
				else if (lex.value == ":")	lex.type = Lexeme::COLON;
				else
				{
					if (groupSetContainsName(opsBeforeCommands, lex.value) || groupSetContainsName(opsAfterCommands, lex.value))
					{
						lex.type = Lexeme::OPERATOR;
					}						
				}
			}		
		}
	}

	void identifyKeywords(LexemeLine& lexemeLine)
	{
		for (Lexeme& lex : lexemeLine)
		{
			if (lex.type == Lexeme::UNKNOWN && keywords.count(lex.value)) lex.type = Lexeme::KEYWORD;
		}
	}

	void identifyCommandDefinitions(LexemeLine& lexemeLine)
	{
		// command definitions of the form `commandName : arg1, arg2, ..., argN`
		if (lexemeLine.size() >= 2 &&
			lexemeLine[0].type == Lexeme::UNKNOWN &&
			lexemeLine[1].type == Lexeme::COLON)
		{
			lexemeLine[0].type = Lexeme::COMMAND;
			unsigned const args = std::count(lexemeLine.begin(), lexemeLine.end(), Lexeme(",", Lexeme::COLON)) + 1;
			Function const command = Function(lexemeLine[0].value, lexemeLine[0].value, Function::PREFIX, args);
			commands.insert(command);
		}
	}

	void identifyCommandUses(LexemeLine& lexemeLine)
	{
		for (Lexeme& lex : lexemeLine)
		{
			if (lex.type == Lexeme::UNKNOWN && commandExistsWithName(lex.value)) lex.type = Lexeme::COMMAND;
		}
	}

	void encloseCommandsWithBrackets(LexemeLine& lexemeLine)
	{
		for (unsigned i = 0; i < lexemeLine.size(); ++i)
		{
			if (lexemeLine[i].type == Lexeme::COMMAND || lexemeLine[i].type == Lexeme::OPERATOR) // might need brackets around it cos its prefix
			{
				Function lexAsFn;
				if (couldSetFunctionFromName(lexAsFn, lexemeLine[i].value) && lexAsFn.type == Function::PREFIX) // does need brackets round it cos its prefix
				{
					if (i == 0 || lexemeLine[i - 1].type != Lexeme::REG_OPENING_BRACE) // if doesnt have opening bracket
					{
						lexemeLine.insert(i, Lexeme("(", Lexeme::REG_OPENING_BRACE));
					}

					if (i == lexemeLine.size() - 1 || Util::bracketPolarity(lexemeLine) != 0) // if doesnt have closing bracket
					{
						lexemeLine.insert(lexemeLine.size(), Lexeme(")", Lexeme::REG_CLOSING_BRACE));
					}
				}
				
			}
		}
	}

	bool varAlreadyDefined(std::vector<LexemeLine> const & lexemeDoc, Variable var)
	{
		for (Lexer::Variable const & definedVar : definedVariables)
		{
			if (definedVar.identifier == var.identifier &&
				definedVar.lineNumber <= var.lineNumber &&
				definedVar.depth <= var.depth &&
				groupUnbroken(lexemeDoc, definedVar.lineNumber - 1, var.lineNumber - 1))
				return true;
		}
		return false;
	}

	void identifyVarDefinitions(std::vector<LexemeLine>& lexemeDoc, unsigned row, unsigned depth)
	{
		unsigned varDeclIdx;
		const Lexeme equalsOp("=", Lexeme::OPERATOR);
		LexemeLine& lexemeLine = lexemeDoc[row];

		if (lexemeDoc.size() >= 2 &&
			Util::couldSetFirstIndexAfterType(lexemeLine, Lexeme::DEPTH, varDeclIdx) &&
			lexemeLine[varDeclIdx].type == Lexeme::UNKNOWN &&
			lexemeLine[varDeclIdx + 1] == equalsOp)
		{
			lexemeLine[varDeclIdx].type = Lexeme::VARIABLE;
			Variable const varDetails = Variable(lexemeLine[varDeclIdx].value, row + 1, depth);

			if (!varAlreadyDefined(lexemeDoc, varDetails))
			{
				definedVariables.insert(varDetails);
				lexemeLine.type = LexemeLine::VAR_CREATION;
			}
			else lexemeLine.type = LexemeLine::VAR_REDEFINITION;

		}
	}

	void identifyVarUses(std::vector<LexemeLine>& lexemeDoc, unsigned row, unsigned depth)
	{
		for (Lexeme& lex : lexemeDoc[row])
		{
			if (lex.type == Lexeme::UNKNOWN)
			{
				Variable const lexAsVar = Variable(lex.value, row, depth);

				bool varDefinedAfterReference = false;
				bool varDefinedInDeeperScope = false;

				for (const Variable& definedVar : definedVariables)
				{
					if (definedVar.identifier == lexAsVar.identifier)
					{
						if (definedVar.lineNumber <= row)
						{
							varDefinedAfterReference = true;
							if (definedVar.depth <= depth && groupUnbroken(lexemeDoc, definedVar.lineNumber - 1, row))
							{
								lex.type = Lexeme::VARIABLE;
								return;
							}
							else varDefinedInDeeperScope = true;
						}
						else varDefinedAfterReference = true;
					}
				}

				std::string const lineNumberStr = std::to_string(row + 1); // line number starts counting from 1, not 0
				if (varDefinedInDeeperScope)
					throw Mistake::Variable_Does_Not_Exist("The variable " + lex.value + " has been made outside of line " + lineNumberStr + "'s scope.");
				else if (varDefinedAfterReference)
					throw Mistake::Variable_Does_Not_Exist("The variable " + lex.value + " has been declared after it has been referred to on line " + lineNumberStr + ". This means it does not exist for that line.", row);
				else
					throw Mistake::Variable_Does_Not_Exist("The variable " + lex.value + " does not exist. It may have been misspelled.", row);
			}
		}
	}

	void removeDepthLexemes(std::vector<LexemeLine>& lexemeDoc)
	{
		for (LexemeLine& lexemeLine : lexemeDoc)
		{
			while (lexemeLine.isNotEmpty() && lexemeLine[0].type == Lexeme::DEPTH)
			{
				lexemeLine.erase(0);
			}
		}
	}

	void addScopeLines(std::vector<LexemeLine>& lexemeDoc)
	{
		LexemeLine const scopeEnter(LexemeLine::SCOPE_ENTER);
		LexemeLine const scopeExit(LexemeLine::SCOPE_EXIT);

		for (unsigned r = 1; r < lexemeDoc.size(); r++)
		{
			if (depthOfLine(lexemeDoc[r]) > depthOfLine(lexemeDoc[r - 1]))
			{
				lexemeDoc.insert(lexemeDoc.begin() + r, scopeEnter);
				r++;
			}
			else if (depthOfLine(lexemeDoc[r]) < depthOfLine(lexemeDoc[r - 1]))
			{
				lexemeDoc.insert(lexemeDoc.begin() + r, scopeExit);
				r++;
			}
		}

		if (depthOfLine(lexemeDoc[lexemeDoc.size() - 1]) > 0)
		{
			lexemeDoc.push_back(scopeExit);
		}
	}

	bool setInnermostBracketIndexes(LexemeLine& lexemeLine, unsigned start, unsigned end, unsigned& openingIdx, unsigned& closingIdx)
	{
		// innermost '(' at point with highest bracket polarity	
		int highestPolarity = 0;

		for (unsigned i = start; i <= end; ++i)
		{
			if (lexemeLine[i].type == Lexeme::REG_OPENING_BRACE && !lexemeLine[i].processed)
			{
				int const bracketPolarityHere = Util::bracketPolarity(lexemeLine, i);
				if (bracketPolarityHere > highestPolarity)
				{
					highestPolarity = bracketPolarityHere;
					openingIdx = i;
				}
			}
		}

		if (highestPolarity > 0) // innermost opening bracket index actually found
		{
			lexemeLine[openingIdx].processed = true;

			for (unsigned j = openingIdx + 1; j <= end; ++j)
			{
				if (lexemeLine[j].type == Lexeme::REG_CLOSING_BRACE && 
					!lexemeLine[j].processed &&
					Util::bracketPolarity(lexemeLine, openingIdx, j) == 0)
				{
					closingIdx = j;
					lexemeLine[j].processed = true;
					return true;
				}
			}
		}
		return false;
	}

	void setOrderLookingAtGroup(LexemeLine& lexemeLine, unsigned start, unsigned end, unsigned& highestOrder, std::vector<std::set<Function>> const & groups)
	{
		for (std::set<Function> const & precedenceGroup : groups)
		{
			for (unsigned i = start; i <= end; ++i)
			{
				if (lexemeLine[i].type == Lexer::Lexeme::OPERATOR 
					&& groupContainsName(precedenceGroup, lexemeLine[i].value)
					&& !lexemeLine[i].processed)
				{
					lexemeLine[i].order = highestOrder++;
					lexemeLine[i].processed = true;
				}
			}
		}
	}

	void setOrderLookingACommands(LexemeLine& lexemeLine, unsigned start, unsigned end, unsigned& highestOrder)
	{
		for (unsigned i = start; i <= end; ++i)
		{
			if (lexemeLine[i].type == Lexeme::COMMAND && !lexemeLine[i].processed)
			{
				lexemeLine[i].order = highestOrder++;
				lexemeLine[i].processed = true;
			}
		}
	}

	void setOrderForLexemeLine(LexemeLine& lexemeLine, unsigned start, unsigned end, unsigned& highestOrder) // start, end inclusive
	{
		if (start == end) return;

		unsigned openingIdx, closingIdx = 0;
		while (setInnermostBracketIndexes(lexemeLine, start, end, openingIdx, closingIdx)) // while there are brackets in the first place
		{
			setOrderForLexemeLine(lexemeLine, openingIdx + 1, closingIdx - 1, highestOrder); // set order on innermost brackets
		}
		
		setOrderLookingAtGroup(lexemeLine, start, end, highestOrder, opsBeforeCommands);
		setOrderLookingACommands(lexemeLine, start, end, highestOrder);
		setOrderLookingAtGroup(lexemeLine, start, end, highestOrder, opsAfterCommands);
	}

	void interpretKeywords(std::vector<LexemeLine>& lexemeDoc) 
	{
		Lexeme const ifLexeme = Lexeme("if", Lexeme::KEYWORD);
		Lexeme const whileLexeme = Lexeme("while", Lexeme::KEYWORD);

		for (LexemeLine& lexemeLine : lexemeDoc)
		{
			if (lexemeLine.isNotEmpty())
			{
				if (lexemeLine[0] == ifLexeme)
				{
					lexemeLine.erase(0); // dont need that
					lexemeLine.type = LexemeLine::IF;
				}
				else if (lexemeLine[0] == whileLexeme)
				{
					lexemeLine.erase(0);
					lexemeLine.type = LexemeLine::WHILE;
				}
			}
		}
	}

	void setOrderForLexemeLine(LexemeLine& lexemeLine) 
	{
		unsigned highestOrder = 1;
		return setOrderForLexemeLine(lexemeLine, 0, lexemeLine.size() - 1, highestOrder); 
	}

	bool couldIdentifyFunctionInGroups(Function & function, std::string const & name, std::vector<std::set<Function>> const & groups)
	{
		for (std::set<Function> const & group : groups)
		{
			for (Function const & fn : group)
			{
				if (fn.identifier == name)
				{
					function = fn;
					return true;
				}
			}
		}

		return false;
	}
}

std::vector<Lexer::LexemeLine> Lexer::codeToLexemeLines(const std::string& code)
{
	const std::vector<std::vector<std::string>> codeDocument = codeToLines(code);
	std::vector<LexemeLine> lexemeLines = docToUnidentifiedLexemeLines(codeDocument);

	identifyDepthLexemes(lexemeLines);

	for (unsigned r = 0; r < lexemeLines.size(); r++)
	{
		LexemeLine& lexemeLine = lexemeLines[r];
		unsigned const depth = depthOfLine(lexemeLine);

		identifyPhrases(lexemeLine);
		identifyNumbers(lexemeLine);
		identifyBooleans(lexemeLine);

		identifyKeywords(lexemeLine);

		identifyOperators(lexemeLine);
		
		identifyCommandDefinitions(lexemeLine);
		identifyCommandUses(lexemeLine);

		encloseCommandsWithBrackets(lexemeLine);

		identifyVarDefinitions(lexemeLines, r, depth);
		identifyVarUses(lexemeLines, r, depth);

		setOrderForLexemeLine(lexemeLine);
	}

	addScopeLines(lexemeLines);
	removeDepthLexemes(lexemeLines);

	interpretKeywords(lexemeLines);

	return lexemeLines;
}

std::string Lexer::lineTypeAsString(Lexer::LexemeLine::Type type)
{
	switch (type)
	{
		case LexemeLine::VAR_CREATION:	
			return "Variable Creation";
		case LexemeLine::VAR_REDEFINITION:
			return "Redefining variable";
		case LexemeLine::IF:
			return "If statement condition";
		case LexemeLine::FOR_EACH:
			return "For each loop header";
		case LexemeLine::WHILE:
			return "While loop header";
		case LexemeLine::UNKNOWN:
			return "Unknown";
		case LexemeLine::SCOPE_ENTER:
			return "Enter scope";
		case LexemeLine::SCOPE_EXIT:
			return "Exit scope";
		default:
			return "No recognised type assigned";
	}
}

bool Lexer::couldSetFunctionFromName(Lexer::Function & fn, std::string const & name)
{
	if (couldIdentifyFunctionInGroups(fn, name, opsBeforeCommands)) return true;
	if (couldIdentifyFunctionInGroups(fn, name, opsAfterCommands)) return true;

	for (Function const & command : commands)
	{
		if (command.identifier == name)
		{
			fn = command;
			return true;
		}
	}

	return false;
}
