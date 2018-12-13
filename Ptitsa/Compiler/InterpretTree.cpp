#include "InterpretTree.h"
#include "BuildContextTree.h"
#include "BuildAST.h"

#include <string>

namespace 
{
	std::string lexemeToCpp(Lexer::PLexeme const & lex)
	{
		using namespace Lexer;
		using std::static_pointer_cast;

		if (lex->isFunction())
		{
			return static_pointer_cast<Function>(lex)->asCpp;
		}

		else if (lex->isLiteral())
		{
			Literal const lit = * static_pointer_cast<Literal>(lex);

			if (lit.type == Literal::PHRASE)	return "std::string(\"" + lit.value + "\")";
			else return lit.value;
		}

		else if (lex->isSymbol())
		{
			switch (static_pointer_cast<Symbol>(lex)->type)
			{
				case Symbol::Type::ARGS_SEP:	
					return ",";
				case Symbol::Type::OPEN_BRACKET:
					return "(";
				case Symbol::Type::CLOSE_BRACKET:
					return ")";
			}
		}
		else if (lex->isVariable())
		{
			return static_pointer_cast<Variable>(lex)->identifier;
		}

	}

	std::string functionCallsToString(BuildAST::PASTNode const & node)
	{
		using namespace Lexer;
		std::string const nodeAsString = lexemeToCpp(node->lex);

		if (node->children.empty()) return nodeAsString;
		else if (node->lex->isFunction())
		{
			std::string fnCallAsString;
			std::vector<std::string> argNames;

			for (BuildAST::PASTNode const & arg : node->children)
			{
				argNames.push_back(functionCallsToString(arg));
			}

			Function const fn = *std::static_pointer_cast<Function>(node->lex);

			if (fn.type == Function::PREFIX)
			{
				fnCallAsString = fn.asCpp + "(";
				for (unsigned i = 0; i < argNames.size(); i++)
				{
					fnCallAsString += argNames[i];
					if (i + 1 < argNames.size()) fnCallAsString += ", ";
					else fnCallAsString += ")";
				}
			}
			else if (fn.type == Function::INFIX)
			{
				fnCallAsString = argNames[0] + " " + fn.asCpp + " " + argNames[1];
			}
			else if (fn.type == Function::POSTFIX)
			{
				fnCallAsString = argNames[0] + " " + fn.asCpp;
			}

			return fnCallAsString;
		}

	}

	std::string treeToString(BuildContextTree::ContextTree const & tree)
	{
		using Lexer::LexemeLine;
		std::string const varType = "BuiltinType::Object";

		switch (tree.type)
		{
		case LexemeLine::VAR_CREATION:
			return "BuiltinType::Object " + functionCallsToString(tree.root) + ";";

		case LexemeLine::VAR_REDEFINITION:
			return functionCallsToString(tree.root) + ";";

		case LexemeLine::IF:
			return "if (Library::isTrue(" + functionCallsToString(tree.root) + "))";

		case LexemeLine::WHILE:
			return "while (Library::isTrue(" + functionCallsToString(tree.root) + "))";

		case LexemeLine::SCOPE_ENTER:
			return "{";

		case LexemeLine::SCOPE_EXIT:
			return "}";

		default:
			return functionCallsToString(tree.root) + ";";
		}
	}
}

std::string InterpretTree::treesToString(std::vector<BuildContextTree::ContextTree> const & trees)
{
	std::string cppCode = R"(
#include "Language\Object.h"
#include "Language\Core.h"

int main()
{

)";

	for (BuildContextTree::ContextTree const & tree : trees)
	{
		cppCode += treeToString(tree) + "\n";
	}
	cppCode += R"(
}
return 0;
)";
	return cppCode;
}