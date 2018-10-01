// All we do here is create a list of lexeme lines, and identify each lexeme's type. that is all.
// Parsing the Lexemes takes place in ParseTypedLexemes.cpp

#include <vector>
#include <set>
#include "NewLexer.h"
#include "Util.h"

std::vector<std::vector<NewLexer::Function>> const NewLexer::opsBeforeCommands = {
	{ NewLexer::Function("^", "^", NewLexer::Function::INFIX) },
	{
		NewLexer::Function("*", "*", NewLexer::Function::INFIX),
		NewLexer::Function("/", "/", NewLexer::Function::INFIX)
	},
	{
		NewLexer::Function("+", "+", NewLexer::Function::INFIX),
		NewLexer::Function("-", "-", NewLexer::Function::INFIX)
	}
};

std::vector<std::vector<NewLexer::Function>> const NewLexer::opsAfterCommands({
	{
		NewLexer::Function("is", "==", NewLexer::Function::INFIX),
		NewLexer::Function("isnt", "!=", NewLexer::Function::INFIX)
	},
	{ NewLexer::Function("and", "&&", NewLexer::Function::INFIX) },
	{ NewLexer::Function("or", "||", NewLexer::Function::INFIX) },
	{ NewLexer::Function("not", "!", NewLexer::Function::PREFIX, 1) },
	{ NewLexer::Function("=", "=", NewLexer::Function::INFIX) }
	});

std::vector<NewLexer::Function> NewLexer::commands({
	NewLexer::Function("not", "!", NewLexer::Function::PREFIX, 1),
	NewLexer::Function("show", "Library::show", NewLexer::Function::PREFIX, -1),
	NewLexer::Function("exp", "Library::exp", NewLexer::Function::PREFIX, 1)
});

namespace
{
	using namespace NewLexer;
	using std::static_pointer_cast;

	std::set<Variable> definedVariables = { };

	// 'Utility ' functions
	unsigned depthOfLine(LexemeLine const & lexemeLine)
	{
		unsigned depth = 0;
		for (PLexeme const & lex : lexemeLine)
		{
			if (lex->isSymbol() && static_pointer_cast<Symbol>(lex)->type == Symbol::DEPTH) depth++;
			else break;
		}
		return depth;
	}

	unsigned lastIdxInScope(std::vector<LexemeLine> const & lexemeDoc, unsigned scopeStartIdx)
	{
		unsigned const scopeAtStart = depthOfLine(lexemeDoc[scopeStartIdx]);

		for (unsigned r = scopeStartIdx + 1; r < lexemeDoc.size(); r++)
		{
			if (depthOfLine(lexemeDoc[r]) < scopeAtStart) return r - 1;
		}
		return lexemeDoc.size() - 1;
	}

	bool indexesInsameGroup(std::vector<LexemeLine> const & lexemeDoc, unsigned firstRow, unsigned secondRow)
	{
		unsigned const depthOfFirstRow = depthOfLine(lexemeDoc[firstRow]);
		if (depthOfFirstRow == 0) return true;

		for (unsigned r = firstRow + 1; r < secondRow; r++) // group "broken" if there's a line with a lesser depth than the group depth between first and second row
		{
			if (depthOfLine(lexemeDoc[r]) < depthOfFirstRow) return false;
		}
		return true;
	}

	bool groupContainsName(std::vector<Function> const & group, std::string const & name)
	{
		for (Function const & function : group)
		{
			if (function.identifier == name) return true;
		}
		return false;
	}

	bool groupSetContainsName(std::vector<std::vector<Function>> const & groups, std::string const & identifier)
	{
		for (std::vector<Function> const & opGroup : groups)
		{
			if (groupContainsName(opGroup, identifier)) return true;
		}
		return false;
	}

	bool commandExistsWithName(std::string const & identifier)
	{
		for (Function const & command : commands)
		{
			if (command.identifier == identifier) return true;
		}
		return false;
	}

	bool functionExistsWithName(std::string const & identifier)
	{
		return groupSetContainsName(opsBeforeCommands, identifier) || groupSetContainsName(opsAfterCommands, identifier) || commandExistsWithName(identifier);
	}

	std::set<std::string> commandArgNames(LexemeLine & line)
	{
		std::set<std::string> arguments;
		for (unsigned i = 2; i < line.size(); i++)
		{
			if (line[i]->isRaw())
			{
				arguments.insert(static_pointer_cast<RawLexeme>(line[i])->value);
			}
		}
		return arguments;
	}

	// Actual CreateTypeLexemes functions
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

	std::vector<LexemeLine> docToUntypedLines(std::vector<std::vector<std::string>> const & codeDoc)
	{
		std::vector<LexemeLine> lexemeDoc;

		for (std::vector<std::string> const & codeLine : codeDoc)
		{
			std::vector<PLexeme> lexemes;
			for (std::string const & word : codeLine) lexemes.push_back(std::make_shared<RawLexeme>(word));

			if (!lexemes.empty()) lexemeDoc.emplace_back<LexemeLine>(lexemes);
		}

		return lexemeDoc;
	}

	void identifyDepthLexemes(LexemeLine & line)
	{
		for (unsigned i = 0; i < line.size(); i++)
		{
			if (line[i]->isRaw()
				&& static_pointer_cast<RawLexeme>(line[i])->value == "\t")
			{
				line[i] = std::make_shared<Symbol>(Symbol::DEPTH);
			}
		}
	}

	void identifyPhrases(LexemeLine & line)
	{
		for (unsigned i = 0; i < line.size(); i++)
		{
			if (line[i]->isRaw())
			{
				PRawLexeme rawLex = static_pointer_cast<RawLexeme>(line[i]);

				if (rawLex->value[0] == '\"') // found start of phrase
				{
					std::string const idWithoutQuote = rawLex->value.substr(1, rawLex->value.size() - 1);
					line[i] = std::make_shared<Literal>(idWithoutQuote, Literal::PHRASE);

					// appending next words to current string,  until end of string found
					while (i + 1 < line.size() 
						&& line[i + 1]->isRaw()
						&& rawLex->value.back() != '\"')
					{
						rawLex->value += " " + static_pointer_cast<RawLexeme>(line[i+1])->value;
						line.erase(i + 1);
					}
					rawLex->value.pop_back(); // deleting '\"' character
				}
			}
		}
	}

	void identifyNumbers(LexemeLine & line)
	{
		for (unsigned i = 0; i < line.size(); i++)
		{
			if (line[i]->isRaw())
			{
				std::string const value = static_pointer_cast<RawLexeme>(line[i])->value;
				if (Util::isNumber(value))
				{
					std::string const number = Util::toDecimal(value);
					line[i] =  std::make_shared<Literal>(number, Literal::NUMBER);
				}
			}
		}
	}

	void identifyBooleans(LexemeLine & line)
	{
		for (unsigned i = 0; i < line.size(); i++)
		{
			if (line[i]->isRaw())
			{
				std::string const value = static_pointer_cast<RawLexeme>(line[i])->value;
				if (value == "true" || value == "false")
				{
					line[i] = std::make_shared<Literal>(value, Literal::BOOL);
				}
			}			
		}
	}
	
	void identifySyntaxSymbols(LexemeLine & line)
	{
		for (unsigned i = 0; i < line.size(); i++)
		{
			if (line[i]->isRaw())
			{
				std::string const id = static_pointer_cast<RawLexeme>(line[i])->value;
				if (Symbol::idsToSymbols.count(id))
				{
					line[i] = std::make_shared<Symbol>(Symbol::idsToSymbols.at(id));
				}
			}
		}
	}

	void identifyKeywords(LexemeLine & line)
	{
		for (PLexeme & lex : line)
		{
			if (lex->isRaw())
			{
				std::string const value = static_pointer_cast<RawLexeme>(lex)->value;
				if (Keyword::valuesToKeywordTypes.count(value))
				{
					lex = std::make_shared<Keyword>(Keyword::valuesToKeywordTypes.at(value));
				}
			}
		}
	}

	void identifyCommandDeclarations(LexemeLine & line, unsigned row)
	{
		if (line.size() >= 2 && line[0]->isRaw() && line[1]->isSymbol())
		{
			int argCount = 0;
			for (std::string const & argName : commandArgNames(line))
			{				
				Variable arg = Variable(argName, row + 1, line.depth + 1); // let the argument be defined in function's scope
				definedVariables.insert(arg);
				argCount++;
			}

			std::string const functionName = static_pointer_cast<RawLexeme>(line[0])->value;
			Function fn = Function(functionName, functionName, Function::PREFIX, argCount);
			line[0] = std::make_shared<Function>(fn);
			commands.push_back(fn);
		}
	}

	void identifyFunctionUses(LexemeLine & line)
	{
		for (unsigned i = 0; i < line.size(); i++)
		{
			if (line[i]->isRaw())
			{
				std::string const name = static_pointer_cast<RawLexeme>(line[i])->value;
				Function fn;
				if (couldSetFunctionFromName(fn, name)) line[i] = std::make_shared<Function>(fn);
			}
		}
	}

	void encloseFunctionsWithBrackets(LexemeLine & line)
	{
		for (unsigned i = 0; i < line.size(); i++)
		{
			if (line[i]->isFunction()) // may need brackets around it if its prefix
			{
				if (static_pointer_cast<Function>(line[i])->type == Function::PREFIX) // will need brackets
				{
					if (i == 0
						|| !line[i - 1]->isSymbol()
						|| (line[i - 1]->isSymbol() && static_pointer_cast<Symbol>(line[i - 1])->type != Symbol::OPEN_BRACKET)) // doesnt have opening bracket
					{
						line.insert(i, std::make_shared<Symbol>(Symbol::OPEN_BRACKET));
					}

					if (i == line.size() - 1 || Util::bracketPolarity(line) != 0) 
					{
						line.push_back(std::make_shared<Symbol>(Symbol::CLOSE_BRACKET));
					}
				}
			}
		}
	}

	void identifyVarDefinitions(LexemeLine & line, unsigned row)
	{
		unsigned const varIdx = depthOfLine(line); // will be like `<indent> <indent> x = stuff`
		if (line.size() >= 3
			&& line[varIdx]->isRaw()
			&& line[varIdx + 1]->isFunction()
			&& static_pointer_cast<Function>(line[varIdx + 1])->identifier == "=")
		{
			std::string const varName = static_pointer_cast<RawLexeme>(line[varIdx])->value;
			Variable const var = Variable(varName, row, line.depth);
			
			line[varIdx] = std::make_shared<Variable>(var);
			definedVariables.insert(var);
		}
	}

	void setRawLexemesToVariables(LexemeLine & line, unsigned row)
	{
		for (unsigned i = 0; i < line.size(); i++)
		{
			if (line[i]->isRaw())
			{
				std::string const varName = static_pointer_cast<RawLexeme>(line[i])->value;
				line[i] = std::make_shared<Variable>(varName, row, line.depth);
			}
		}
	}

	bool couldIdentifyFunctionInGroups(Function & function, std::string const & name, std::vector<std::vector<Function>> const & groups)
	{
		for (std::vector<Function> const & group : groups)
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

bool NewLexer::couldSetFunctionFromName(NewLexer::Function & fn, std::string const & name)
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

std::vector<NewLexer::LexemeLine> NewLexer::createTypedLexemes(std::string const & code)
{
	const std::vector<std::vector<std::string>> codeDocument = codeToLines(code);
	std::vector<LexemeLine> lexemeDoc = docToUntypedLines(codeDocument);

	for (unsigned r = 0; r < lexemeDoc.size(); r++)
	{
		LexemeLine & line = lexemeDoc[r];

		identifyDepthLexemes(line);
		line.depth = depthOfLine(line);

		identifyPhrases(line);
		identifyBooleans(line);
		identifyNumbers(line);
		identifySyntaxSymbols(line);

		identifyKeywords(line);

		identifyCommandDeclarations(line, r);
		identifyFunctionUses(line);
		encloseFunctionsWithBrackets(line);

		identifyVarDefinitions(line, r);

		setRawLexemesToVariables(line, r);
	}

	return lexemeDoc;
}