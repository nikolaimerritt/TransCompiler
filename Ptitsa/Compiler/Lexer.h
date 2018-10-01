#ifndef LEXER_INCLUDE
#define LEXER_INCLUDE 

#include <iostream>
#include <vector>
#include <set>
#include <map>

namespace Lexer
{	
	struct Lexeme
	{
		enum Type
		{
			UNKNOWN,
			PHRASE,
			NUMBER,
			BOOLEAN,
			OPERATOR,
			VARIABLE,
			COMMAND,
			KEYWORD,
			REG_OPENING_BRACE,
			REG_CLOSING_BRACE,
			ARGS_SEP,
			COLON,
			DEPTH
		} type;

		unsigned order;
		std::string value;
		bool processed;

		Lexeme(std::string, Type);
		bool operator==(const Lexeme&);
	};

	struct LexemeLine
	{
		enum Type 
		{
			VAR_CREATION,
			VAR_REDEFINITION,
			IF,
			WHILE,
			FOR_EACH,
			VOID_FUNCTION_CALL,
			SCOPE_ENTER,
			SCOPE_EXIT,
			UNKNOWN
		} type;

		LexemeLine(const std::vector<Lexeme>&);
		LexemeLine(Type);
		
		Lexeme& operator[](int);
		const Lexeme& operator[](int) const;

		std::vector<Lexeme>::iterator begin();
		std::vector<Lexer::Lexeme>::const_iterator begin() const;
		std::vector<Lexeme>::iterator end();
		std::vector<Lexer::Lexeme>::const_iterator end() const;

		unsigned size() const;
		void push_back(const Lexeme&);
		void erase(int);
		void insert(int, const Lexeme&);
		bool isNotEmpty();

		LexemeLine copyBetween(unsigned, unsigned) const;

	private:
		std::vector<Lexeme> lexemes;
	};

	struct Variable
	{
		std::string identifier;
		unsigned depth;
		unsigned lineNumber;
		
		Variable();
		Variable(std::string, unsigned, unsigned);

		bool operator==(Variable) const;
		bool operator<(Variable) const;
	};

	struct Function
	{
		std::string identifier;
		std::string asCpp;
		int args;
		enum Type { PREFIX, INFIX, POSTFIX } type;

		Function();
		Function(std::string const &, std::string const &, Type);
		Function(std::string const &, std::string const &, Type, int);
		Function(std::string const &, std::string const &);

		bool operator==(Function const &) const;
		bool operator<(Function const &) const;
	};

	extern std::vector<std::set<Function>> const opsBeforeCommands;
	extern std::set<Function> commands;
	extern std::vector<std::set<Function>> const opsAfterCommands;

	bool couldSetFunctionFromName(Lexer::Function &, std::string const &);

	std::vector<LexemeLine> codeToLexemeLines(std::string const &);
	std::string lineTypeAsString(LexemeLine::Type);
}
#endif