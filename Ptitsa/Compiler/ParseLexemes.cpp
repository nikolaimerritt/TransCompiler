#include "Lexer.h"
#include "Util.h"
#include <vector>
#include <algorithm>

namespace
{
	using namespace Lexer;
	using std::static_pointer_cast;

	static std::vector<Variable> definedVars =  { 
		Variable("pi", 0, 0) 
	};

	// Helper functions
	bool varAlreadyDefined(Variable const & var)
	{
		for (Variable const & definedVar : definedVars)
		{
			if (definedVar <= var) return true;
		}
		return false;
	}

	void removeIndentLexemes(LexemeLine & line)
	{
		while (line.isNotEmpty()
			&& line[0]->isSymbol()
			&& static_pointer_cast<Symbol>(line[0])->type == Symbol::DEPTH)
		{
			line.erase(0);
		}
	}
	bool couldSetInnerBracketIndexes(LexemeLine & line, unsigned start, unsigned end, unsigned & openIdx, unsigned & closeIdx)
	{
		// inner opening bracket at point with highest bracket polarity
		int maxPolarity = 0;

		for (unsigned i = start; i <= end; i++)
		{
			if (line[i]->isSymbol())
			{
				Symbol const symbol = *static_pointer_cast<Symbol>(line[i]);
				if (symbol.type == Symbol::OPEN_BRACKET && !symbol.processed)
				{
					int const polarityHere = Util::bracketPolarity(line, i);
					if (polarityHere > maxPolarity)
					{
						maxPolarity = polarityHere;
						openIdx = i;
					}
				}
			}
		}

		if (maxPolarity > 0) // found inner opening bracket
		{
			static_pointer_cast<Symbol>(line[openIdx])->processed = true;

			for (unsigned i = openIdx + 1; i <= end; i++) // now finding closing bracket
			{
				if (line[i]->isSymbol())
				{
					PSymbol symbol = static_pointer_cast<Symbol>(line[i]);

					if (symbol->type == Symbol::CLOSE_BRACKET
						&& !symbol->processed
						&& Util::bracketPolarity(line, openIdx, i) == 0)
					{
						closeIdx = i;
						symbol->processed = true;
						return true;
					}
				}
			}
		}

		return false;
	}

	bool groupsContainFunction(std::vector<std::vector<Function>> const & groups, Function const & fn)
	{
		for (std::vector<Function> const & group : groups)
		{
			if (std::find(group.begin(), group.end(), fn) != group.end()) return true;
		}
		return false;
	}

	// Actual ParseLexeme functions

	void identifyVarCreationsAndRedefinitions(LexemeLine & line)
	{
		if (line.size() >= 2
			&& line[0]->isVariable()
			&& line[1]->isFunction())
		{
			Variable const var = * static_pointer_cast<Variable>(line[0]);
			Function const fn = * static_pointer_cast<Function>(line[1]);

			if (fn.type == Function::INFIX && fn.identifier == "=") 
			{
				if (varAlreadyDefined(var))
				{
					line.type = LexemeLine::VAR_REDEFINITION;
				}
				else
				{
					definedVars.push_back(var);
					line.type = LexemeLine::VAR_CREATION;
				}				
			}			
		}
	}

	void identifyStatements(LexemeLine & line)
	{
		if (line.isEmpty()) return;

		if (line[0]->isKeyword())
		{
			switch (static_pointer_cast<Keyword>(line[0])->type)
			{
				case Keyword::IF:		line.type = LexemeLine::IF;			break;
				case Keyword::WHILE:	line.type = LexemeLine::WHILE;		break;
			}
			line.erase(0);	// dont need keyword anymore, type already known
		}
	}

	void identifyVoidFunctionCalls(LexemeLine & line)
	{
		if (line.isEmpty()) return;

		if (line.type == LexemeLine::UNKNOWN && line[0]->isFunction())
		{
			line.type = LexemeLine::VOID_FUNCTION_CALL;
		}
	}

	void generateScopeLines(std::vector<LexemeLine> & lexemeDoc)
	{
		LexemeLine const scopeEneterLine = LexemeLine(LexemeLine::SCOPE_ENTER);
		LexemeLine const scopeExitLine = LexemeLine(LexemeLine::SCOPE_EXIT);

		unsigned r = 1;

		while (r < lexemeDoc.size())
		{
			if (lexemeDoc[r].depth > lexemeDoc[r - 1].depth)
			{
				removeIndentLexemes(lexemeDoc[r]);
				lexemeDoc.insert(lexemeDoc.begin() + r, scopeEneterLine);
				r += 2;
			}
			else if (lexemeDoc[r].depth < lexemeDoc[r - 1].depth)
			{
				removeIndentLexemes(lexemeDoc[r]);				
				lexemeDoc.insert(lexemeDoc.begin() + r, scopeExitLine);
				r += 2;
			}
			else r++;
		}
		if (lexemeDoc.back().depth > lexemeDoc.front().depth)
		{
			removeIndentLexemes(lexemeDoc.back());
			lexemeDoc.push_back(scopeExitLine);
		}
	}
	
	void setOrderLookingAtGroup(LexemeLine & line, unsigned start, unsigned end, unsigned & highestOrder, std::vector<Function> const & group)
	{
		for (unsigned i = start; i <= end; i++)
		{
			if (line[i]->isFunction())
			{
				PFunction const fn = static_pointer_cast<Function>(line[i]);

				if (!fn->processed && 
					std::find(group.begin(), group.end(), *fn) != group.end())
				{
					fn->order = highestOrder++;
					fn->processed = true;
				}
			}
		}
	}


	void setOrder(LexemeLine & line, unsigned start, unsigned end, unsigned & highestOrder)
	{
		if (start == end) return;

		unsigned openIdx, closeIdx = 0;
		while (couldSetInnerBracketIndexes(line, start, end, openIdx, closeIdx))
		{
			setOrder(line, openIdx + 1, closeIdx - 1, highestOrder);
		}

		for (std::vector<Function> const & precedenceGroup : opsBeforeCommands)
		{
			setOrderLookingAtGroup(line, start, end, highestOrder, precedenceGroup);
		}
			
		setOrderLookingAtGroup(line, start, end, highestOrder, commands);

		for (std::vector<Function> const & precedenceGroup : opsAfterCommands)
		{
			setOrderLookingAtGroup(line, start, end, highestOrder, precedenceGroup);
		}
	}

	void setOrder(LexemeLine & line) 
	{
		if (line.isNotEmpty())
		{
			unsigned highestOrder = 1;
			setOrder(line, 0, line.size() - 1, highestOrder);
		}		
	}
}

void Lexer::parseTypedLexemes(std::vector<Lexer::LexemeLine> & lexemeDoc)
{
	generateScopeLines(lexemeDoc);

	for (LexemeLine & line : lexemeDoc)
	{
		identifyVarCreationsAndRedefinitions(line);
		identifyStatements(line);
		identifyVoidFunctionCalls(line);

		setOrder(line);
	}
}