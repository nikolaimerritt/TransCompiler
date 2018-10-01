#ifndef NEW_LEXER_INCLUDE
#define NEW_LEXER_INCLUDE

#include <string>
#include <set>
#include <map>
#include <vector>
#include <memory>

namespace NewLexer
{
	struct Lexeme
	{	
		virtual bool isKeyword();
		virtual bool isLiteral();
		virtual bool isFunction();
		virtual bool isVariable();
		virtual bool isSymbol();
		virtual bool isRaw();
		virtual ~Lexeme();
	};

	typedef std::shared_ptr<Lexeme> PLexeme;
	
	struct RawLexeme : Lexeme
	{
		std::string value;
		bool isRaw();
		RawLexeme(std::string const &);
	};

	typedef std::shared_ptr<RawLexeme> PRawLexeme;

	struct Keyword : Lexeme
	{
		enum Type { IF, FOR_EACH, WHILE } type;
		static std::map<std::string, Type> const valuesToKeywordTypes;
		static std::map<Type, std::string> const typeToCpp;

		bool isKeyword();
		Keyword(Type);
	};

	typedef std::shared_ptr<Keyword> PKeyword;

	struct Literal : Lexeme
	{
		enum Type { PHRASE, NUMBER, BOOL } type;		
		std::string value;
		bool isLiteral();

		Literal(std::string const &, Type);
	};

	typedef std::shared_ptr<Literal> PLiteral;
	
	struct Function : Lexeme
	{
		enum Type { PREFIX, INFIX, POSTFIX, UNKNOWN } type;
		unsigned order;
		bool processed;
		int args; // value of -1 means takes any amount of args
		std::string identifier, asCpp;
		bool isFunction();

		Function();
		Function(std::string const & identifier, std::string const & asCpp, Type type);
		Function(std::string const & identifier, std::string const & asCpp, Type type, int args);
		Function(std::string const & identifier, std::string const & asCpp);
		Function(Function const & other);

		bool operator==(Function const & other) const;
		bool operator<(Function const & other) const;

	};

	typedef std::shared_ptr<Function> PFunction;

	struct Variable : Lexeme
	{
		unsigned depth, row;
		std::string identifier;
		bool isVariable();

		Variable();
		Variable(std::string const & identifier);
		Variable(std::string const &, unsigned lineNumber, unsigned depth);

		bool operator==(Variable const & other) const;
		bool operator<(Variable const & other) const;
		bool operator<=(Variable const & other) const;
	};

	typedef std::shared_ptr<Variable> PVariable;

	struct Symbol : Lexeme 
	{
		enum Type { OPEN_BRACKET, CLOSE_BRACKET, ARGS_SEP, COLON, DEPTH } type;
		bool processed;
		static std::map<std::string, Symbol::Type> const idsToSymbols;
		bool isSymbol();

		Symbol(Type);
	};

	typedef std::shared_ptr<Symbol> PSymbol;

	struct LexemeLine
	{
		enum Type { VAR_CREATION, VAR_REDEFINITION, IF, WHILE, FOR_EACH, VOID_FUNCTION_CALL, SCOPE_ENTER, SCOPE_EXIT, UNKNOWN } type;
		unsigned depth;

		LexemeLine(std::vector<PLexeme> const &);
		LexemeLine(Type);
		LexemeLine();

		PLexeme& operator[](unsigned);
		PLexeme const operator[](unsigned) const;

		std::vector<PLexeme>::iterator begin();
		std::vector<PLexeme>::const_iterator begin() const;
		std::vector<PLexeme>::iterator end();
		std::vector<PLexeme>::const_iterator end() const;

		unsigned size() const;
		void push_back(PLexeme const &);
		void erase(unsigned);
		void insert(unsigned, PLexeme const &);
		bool isEmpty() const;
		bool isNotEmpty() const;

		LexemeLine makeCopyBetween(unsigned, unsigned) const;

	private:
		std::vector<PLexeme> lexemes;
	};
	
	std::ostream & operator<<(std::ostream &, PRawLexeme const &);
	std::ostream & operator<<(std::ostream &, PKeyword const &);
	std::ostream & operator<<(std::ostream &, PLiteral const &);
	std::ostream & operator<<(std::ostream &, PFunction const &);
	std::ostream & operator<<(std::ostream &, PVariable const &);
	std::ostream & operator<<(std::ostream &, PSymbol const &);
	std::ostream & operator<<(std::ostream &, PLexeme const &);
	std::ostream & operator<<(std::ostream &, LexemeLine const &);

	extern std::vector<std::vector<Function>> const opsBeforeCommands, opsAfterCommands;
	extern std::vector<Function> commands;

	bool couldSetFunctionFromName(Function &, std::string const &);

	std::vector<LexemeLine> createTypedLexemes(std::string const &);	
	void parseTypedLexemes(std::vector<LexemeLine> &);
}

#endif // !NEW_LEXER
