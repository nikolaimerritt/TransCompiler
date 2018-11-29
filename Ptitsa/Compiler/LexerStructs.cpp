#include <string>
#include <stdexcept>
#include <map>
#include <iostream>

#include "Lexer.h"

// Lexeme
bool Lexer::Lexeme::isFunction() { return false; }
bool Lexer::Lexeme::isKeyword() { return false; }
bool Lexer::Lexeme::isLiteral() { return false; }
bool Lexer::Lexeme::isRaw() { return false; }
bool Lexer::Lexeme::isSymbol() { return false; }
bool Lexer::Lexeme::isVariable() { return false; }

Lexer::Lexeme::~Lexeme() = default;

// RawLexeme
Lexer::RawLexeme::RawLexeme(std::string const & value): value(value) { }

bool Lexer::RawLexeme::isRaw() { return true; }

// Keyword
Lexer::Keyword::Keyword(Lexer::Keyword::Type type):
	type(type)
{ }

bool Lexer::Keyword::isKeyword() { return true; }

std::map<std::string, Lexer::Keyword::Type> const Lexer::Keyword::valuesToKeywordTypes = {
	{"if", Lexer::Keyword::IF },
	{"foreach", Lexer::Keyword::FOR_EACH},
	{"while", Lexer::Keyword::WHILE}
};

// Literal
Lexer::Literal::Literal(std::string const & value, Lexer::Literal::Type type):
	value(value),
	type(type)
{ }

 bool Lexer::Literal::isLiteral() { return true; }

// Function
Lexer::Function::Function() :
	type(UNKNOWN),
	processed(false),
	args(0),
	identifier(),
	asCpp(),
	order(0)
{ }

Lexer::Function::Function(std::string const & identifier, std::string const & asCpp, Lexer::Function::Type type):
	identifier(identifier),
	asCpp(asCpp),
	type(type),
	processed(false),
	order(0)
{
	if (type == INFIX) args = 2;
	else throw std::invalid_argument("Cannot deduce arg amount from non-infix function " + identifier);
}

Lexer::Function::Function(std::string const & identifier, std::string const & asCpp) :
	identifier(identifier),
	asCpp(asCpp),
	type(POSTFIX),
	processed(false),
	args(0),
	order(0)
{ }

Lexer::Function::Function(std::string const & identifier, std::string const & asCpp, Lexer::Function::Type type, int args) :
	identifier(identifier),
	asCpp(asCpp),
	type(type),
	args(args),
	processed(false),
	order(0)
{ }

Lexer::Function::Function(Lexer::Function const & copy):
	identifier(copy.identifier),
	asCpp(copy.asCpp),
	type(copy.type),
	args(copy.args),
	order(copy.order),
	processed(copy.processed)
{ }

 bool Lexer::Function::isFunction() { return true; }

bool Lexer::Function::operator==(Lexer::Function const & other) const
{
	return identifier == other.identifier 
		&& asCpp == other.asCpp
		&& type == other.type
		&& args == other.args;
}
bool Lexer::Function::operator<(Function const & other) const
{
	return identifier < other.identifier;
}

// Variable
Lexer::Variable::Variable() = default;

Lexer::Variable::Variable(std::string const & identifier):
	identifier(identifier),
	row(0),
	depth(0)
{ }

Lexer::Variable::Variable(std::string const & identifier, unsigned lineNumber, unsigned depth):
	identifier(identifier),
	row(lineNumber),
	depth(depth)
{ }

 bool Lexer::Variable::isVariable() { return true; }

bool Lexer::Variable::operator==(Variable const & other) const
{
	return	this->identifier == other.identifier 
			&& this->depth == other.depth
			&& this->row == other.row;
}

bool Lexer::Variable::operator<(Variable const & other) const { return this->row < other.row; }

bool Lexer::Variable::operator<=(Variable const & other) const
{
	return identifier == other.identifier
		&& row <= other.row
		&& depth <= other.depth;
}

// SyntaxSymbol
Lexer::Symbol::Symbol(Lexer::Symbol::Type type):
	type(type),
	processed(false)
{ }

std::map<std::string, Lexer::Symbol::Type> const Lexer::Symbol::idsToSymbols = {
	{ "(", Symbol::OPEN_BRACKET },
	{ ")", Symbol::CLOSE_BRACKET },
	{ ",", Symbol::ARGS_SEP },
	{ ":", Symbol::COLON }
};

bool Lexer::Symbol::isSymbol() { return true; }

// LexemeLine
Lexer::LexemeLine::LexemeLine(std::vector<Lexer::PLexeme> const & lexemes) :
	lexemes(lexemes),
	type(UNKNOWN),
	depth(0)
{ }

Lexer::LexemeLine::LexemeLine(LexemeLine::Type type):
	lexemes(),
	type(type),
	depth(0)
{ }

Lexer::LexemeLine::LexemeLine():
	lexemes(),
	type(UNKNOWN),
	depth(0)
{ }

Lexer::PLexeme& Lexer::LexemeLine::operator[](unsigned i) { return lexemes[i]; }

Lexer::PLexeme const Lexer::LexemeLine::operator[](unsigned i) const { return lexemes[i]; }

std::vector<Lexer::PLexeme>::iterator Lexer::LexemeLine::begin() { return std::begin(lexemes); }

std::vector<Lexer::PLexeme>::const_iterator Lexer::LexemeLine::begin() const { return std::begin(lexemes); }

std::vector<Lexer::PLexeme>::iterator Lexer::LexemeLine::end() { return std::end(lexemes); }

std::vector<Lexer::PLexeme>::const_iterator Lexer::LexemeLine::end() const { return std::end(lexemes); }

unsigned Lexer::LexemeLine::size() const { return lexemes.size(); }

void Lexer::LexemeLine::push_back(Lexer::PLexeme const & lex) { lexemes.push_back(lex); }

void Lexer::LexemeLine::erase(unsigned i) { lexemes.erase(lexemes.begin() + i); }

void Lexer::LexemeLine::insert(unsigned i, Lexer::PLexeme const & toInsert) { lexemes.insert(lexemes.begin() + i, toInsert); }

bool Lexer::LexemeLine::isEmpty() const { return lexemes.empty(); }

bool Lexer::LexemeLine::isNotEmpty() const { return !lexemes.empty(); }

Lexer::LexemeLine Lexer::LexemeLine::makeCopyBetween(unsigned start, unsigned end) const
{
	if (end - start >= 0)
	{
		std::vector<Lexer::PLexeme> copy;
		copy.reserve(end - start + 1);
		copy.insert(copy.begin(), lexemes.begin() + start, lexemes.begin() + end + 1); // end usually exclusive. want to be inclusive
		return LexemeLine(copy);
	}
	else return LexemeLine(UNKNOWN);
}

std::ostream& Lexer::operator<<(std::ostream & ostream, Lexer::PRawLexeme const & lex)
{ 
	ostream << "<(raw) " + lex->value + ">"; 
	return ostream;
}

std::ostream& Lexer::operator<<(std::ostream & ostream, Lexer::PLiteral const & lex)
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

std::ostream& Lexer::operator<<(std::ostream & ostream, Lexer::PKeyword const & lex)
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
std::ostream& Lexer::operator<<(std::ostream & ostream, Lexer::PFunction const & lex)
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

std::ostream& Lexer::operator<<(std::ostream & ostream, Lexer::PSymbol const & lex)
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

std::ostream& Lexer::operator<<(std::ostream & ostream, Lexer::PVariable const & lex)
{
	ostream << "<(var) " << lex->identifier << " d: " << lex->depth << ", r: " << lex->row << ">";
	return ostream;
}

std::ostream & Lexer::operator<<(std::ostream & ostream, Lexer::PLexeme const & lex)
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

std::ostream& Lexer::operator<<(std::ostream & ostream, Lexer::LexemeLine const & line)
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

	for (PLexeme const & lex : line)
	{
		ostream << lex << " "; 
	}
	ostream << "\n";
	return ostream;
}