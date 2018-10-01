#include <string>
#include <stdexcept>
#include <map>

#include "NewLexer.h"

// Lexeme
bool NewLexer::Lexeme::isFunction() { return false; }
bool NewLexer::Lexeme::isKeyword() { return false; }
bool NewLexer::Lexeme::isLiteral() { return false; }
bool NewLexer::Lexeme::isRaw() { return false; }
bool NewLexer::Lexeme::isSymbol() { return false; }
bool NewLexer::Lexeme::isVariable() { return false; }

NewLexer::Lexeme::~Lexeme() { }

// RawLexeme
NewLexer::RawLexeme::RawLexeme(std::string const & value): value(value) { }

bool NewLexer::RawLexeme::isRaw() { return true; }

// Keyword
NewLexer::Keyword::Keyword(NewLexer::Keyword::Type type):
	type(type)
{ }

bool NewLexer::Keyword::isKeyword() { return true; }

std::map<std::string, NewLexer::Keyword::Type> const NewLexer::Keyword::valuesToKeywordTypes = {
	{"if", NewLexer::Keyword::IF },
	{"foreach", NewLexer::Keyword::FOR_EACH},
	{"while", NewLexer::Keyword::WHILE}
};

// Literal
NewLexer::Literal::Literal(std::string const & value, NewLexer::Literal::Type type):
	value(value),
	type(type)
{ }

 bool NewLexer::Literal::isLiteral() { return true; }

// Function
NewLexer::Function::Function() : 
	type(UNKNOWN),
	processed(false),
	args(0),
	identifier(),
	asCpp(),
	order(0)
{ }

NewLexer::Function::Function(std::string const & identifier, std::string const & asCpp, NewLexer::Function::Type type):
	identifier(identifier),
	asCpp(asCpp),
	type(type),
	processed(false),
	order(0)
{
	if (type == INFIX) args = 2;
	else throw std::invalid_argument("Cannot deduce arg amount from non-infix function " + identifier);
}

NewLexer::Function::Function(std::string const & identifier, std::string const & asCpp) :
	identifier(identifier),
	asCpp(asCpp),
	type(POSTFIX),
	processed(false),
	args(0),
	order(0)
{ }

NewLexer::Function::Function(std::string const & identifier, std::string const & asCpp, NewLexer::Function::Type type, int args) :
	identifier(identifier),
	asCpp(asCpp),
	type(type),
	args(args),
	processed(false),
	order(0)
{ }

NewLexer::Function::Function(NewLexer::Function const & copy):
	identifier(copy.identifier),
	asCpp(copy.asCpp),
	type(copy.type),
	args(copy.args),
	order(copy.order),
	processed(copy.processed)
{ }

 bool NewLexer::Function::isFunction() { return true; }

bool NewLexer::Function::operator==(NewLexer::Function const & other) const
{
	return identifier == other.identifier 
		&& asCpp == other.asCpp
		&& type == other.type
		&& args == other.args;
}
bool NewLexer::Function::operator<(Function const & other) const
{
	return identifier < other.identifier;
}

// Variable
NewLexer::Variable::Variable() {  }

NewLexer::Variable::Variable(std::string const & identifier):
	identifier(identifier),
	row(0),
	depth(0)
{ }

NewLexer::Variable::Variable(std::string const & identifier, unsigned lineNumber, unsigned depth):
	identifier(identifier),
	row(lineNumber),
	depth(depth)
{ }

 bool NewLexer::Variable::isVariable() { return true; }

bool NewLexer::Variable::operator==(Variable const & other) const
{
	return	this->identifier == other.identifier 
			&& this->depth == other.depth
			&& this->row == other.row;
}

bool NewLexer::Variable::operator<(Variable const & other) const { return this->row < other.row; }

bool NewLexer::Variable::operator<=(Variable const & other) const
{
	return identifier == other.identifier
		&& row <= other.row
		&& depth <= other.depth;
}

// SyntaxSymbol
NewLexer::Symbol::Symbol(NewLexer::Symbol::Type type):
	type(type),
	processed(false)
{ }

std::map<std::string, NewLexer::Symbol::Type> const NewLexer::Symbol::idsToSymbols = {
	{ "(", Symbol::OPEN_BRACKET },
	{ ")", Symbol::CLOSE_BRACKET },
	{ ",", Symbol::ARGS_SEP },
	{ ":", Symbol::COLON }
};

bool NewLexer::Symbol::isSymbol() { return true; }

// LexemeLine
NewLexer::LexemeLine::LexemeLine(std::vector<NewLexer::PLexeme> const & lexemes) :
	lexemes(lexemes),
	type(UNKNOWN),
	depth(0)
{ }

NewLexer::LexemeLine::LexemeLine(LexemeLine::Type type):
	lexemes(),
	type(type),
	depth(0)
{ }

NewLexer::LexemeLine::LexemeLine():
	lexemes(),
	type(UNKNOWN),
	depth(0)
{ }

NewLexer::PLexeme& NewLexer::LexemeLine::operator[](unsigned i) { return lexemes[i]; }

NewLexer::PLexeme const NewLexer::LexemeLine::operator[](unsigned i) const { return lexemes[i]; }

std::vector<NewLexer::PLexeme>::iterator NewLexer::LexemeLine::begin() { return std::begin(lexemes); }

std::vector<NewLexer::PLexeme>::const_iterator NewLexer::LexemeLine::begin() const { return std::begin(lexemes); }

std::vector<NewLexer::PLexeme>::iterator NewLexer::LexemeLine::end() { return std::end(lexemes); }

std::vector<NewLexer::PLexeme>::const_iterator NewLexer::LexemeLine::end() const { return std::end(lexemes); }

unsigned NewLexer::LexemeLine::size() const { return lexemes.size(); }

void NewLexer::LexemeLine::push_back(NewLexer::PLexeme const & lex) { lexemes.push_back(lex); }

void NewLexer::LexemeLine::erase(unsigned i) { lexemes.erase(lexemes.begin() + i); }

void NewLexer::LexemeLine::insert(unsigned i, NewLexer::PLexeme const & toInsert) { lexemes.insert(lexemes.begin() + i, toInsert); }

bool NewLexer::LexemeLine::isEmpty() const { return lexemes.empty(); }

bool NewLexer::LexemeLine::isNotEmpty() const { return !lexemes.empty(); }

NewLexer::LexemeLine NewLexer::LexemeLine::makeCopyBetween(unsigned start, unsigned end) const
{
	if (end - start >= 0)
	{
		std::vector<NewLexer::PLexeme> copy;
		copy.reserve(end - start + 1);
		copy.insert(copy.begin(), lexemes.begin() + start, lexemes.begin() + end + 1); // end usually exclusive. want to be inclusive
		return LexemeLine(copy);
	}
	else return LexemeLine(UNKNOWN);
}

std::ostream& NewLexer::operator<<(std::ostream & ostream, NewLexer::PRawLexeme const & lex) 
{ 
	ostream << "<(raw) " + lex->value + ">"; 
	return ostream;
}

std::ostream& NewLexer::operator<<(std::ostream & ostream, NewLexer::PLiteral const & lex)
{
	ostream << "<(lit) ";
	switch (lex->type)
	{
		case Literal::PHRASE:	ostream << "(\" \") ";	break;
		case Literal::NUMBER:	ostream << "(#) ";		break;
		case Literal::BOOL:		ostream << "(bool) ";	break;
		default:				ostream << "(?) ";		break;
	}
	ostream << lex->value << ">";
	return ostream;
}

std::ostream& NewLexer::operator<<(std::ostream & ostream, NewLexer::PKeyword const & lex)
{
	ostream << "<(kw) ";
	switch (lex->type)
	{
		case Keyword::FOR_EACH:		ostream << "for each";	break;
		case Keyword::IF:			ostream << "if";		break;
		case Keyword::WHILE:		ostream << "while";		break;
		default:					ostream << "unknown";	break;
	}
	ostream << ">";
	return ostream;
}
std::ostream& NewLexer::operator<<(std::ostream & ostream, NewLexer::PFunction const & lex)
{
	ostream << "<(fn) ";
	switch (lex->type)
	{
		case Function::PREFIX:	ostream << "(pre) ";	break;
		case Function::INFIX:	ostream << "(inf) ";	break;
		case Function::POSTFIX:	ostream << "(post) ";	break;
		default:				ostream << "(?) ";		break;
	}
	ostream << "(" << lex->args << ") " << lex->identifier << ">";
	return ostream;
}

std::ostream& NewLexer::operator<<(std::ostream & ostream, NewLexer::PSymbol const & lex)
{
	ostream << "<(sym) ";
	switch (lex->type)
	{
		case Symbol::ARGS_SEP:			ostream << ",";		break;
		case Symbol::OPEN_BRACKET:		ostream << "(";		break;
		case Symbol::CLOSE_BRACKET:		ostream << ")";		break;
		case Symbol::COLON:				ostream << ":";		break;
		case Symbol::DEPTH:				ostream << ">>";	break;
		default:						ostream << "?";		break;
	}
	ostream << " >";
	return ostream;
}

std::ostream& NewLexer::operator<<(std::ostream & ostream, NewLexer::PVariable const & lex)
{
	ostream << "<(var) " << lex->identifier << " d: " << lex->depth << ", r: " << lex->row << ">";
	return ostream;
}

std::ostream & NewLexer::operator<<(std::ostream & ostream, NewLexer::PLexeme const & lex)
{
	if (lex->isFunction())			ostream << std::dynamic_pointer_cast<Function>(lex);
	else if (lex->isKeyword())		ostream << std::dynamic_pointer_cast<Keyword>(lex);
	else if (lex->isLiteral())		ostream << std::dynamic_pointer_cast<Literal>(lex);
	else if (lex->isRaw())			ostream << std::dynamic_pointer_cast<RawLexeme>(lex);
	else if (lex->isSymbol())		ostream << std::dynamic_pointer_cast<Symbol>(lex);
	else if (lex->isVariable())		ostream << std::dynamic_pointer_cast<Variable>(lex);
	else							ostream << "?";

	return ostream;
}

std::ostream& NewLexer::operator<<(std::ostream & ostream, NewLexer::LexemeLine const & line)
{
	ostream << "{ ";
	switch (line.type)
	{
		case LexemeLine::Type::FOR_EACH:			ostream << "for";		break;
		case LexemeLine::Type::IF:					ostream << "if";		break;
		case LexemeLine::Type::WHILE:				ostream << "while";		break;
		case LexemeLine::Type::SCOPE_ENTER:			ostream << ">>";		break;
		case LexemeLine::Type::SCOPE_EXIT:			ostream << "<<";		break;
		case LexemeLine::Type::VAR_CREATION:		ostream << "new var";	break;
		case LexemeLine::Type::VAR_REDEFINITION:	ostream << "redef var";	break;
		case LexemeLine::Type::VOID_FUNCTION_CALL:	ostream << "fn call";	break;
		case LexemeLine::Type::UNKNOWN:				ostream << "?";			break;
	}
	ostream << " } ";

	for (PLexeme lex : line)
	{
		ostream << lex << " "; 
	}
	ostream << "\n";
	return ostream;
}