#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
/*
#include "Language\Object.h"
#include "Language\Core.h"

int main()
{
Library::show(operator+(BuiltinType::Object(std::string("hi")), std::string("there")));
return 0;
} */

#include "Compiler\Lexer.h"
#include "Compiler\BuildContextTree.h"
#include "Compiler\BuildContextTree.h"
#include "Compiler\InterpretTree.h"

std::string getCode()
{
	std::string const INPUT_FILE = "D:/Code/C++/Ptitsa/Ptitsa/program.pti";

	std::ifstream file(INPUT_FILE);
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

void writeCode(const std::string& cppCode)
{
	std::string const OUTPUT_FILE = "D:/Code/C++/Ptitsa/Ptitsa/program.cpp";
	std::ofstream output(OUTPUT_FILE);
	output << cppCode;
	output.close();
}

int main()
{
	std::vector<Lexer::LexemeLine> const lexemeLines = Lexer::codeToLexemeLines(getCode());
	std::vector<BuildContextTree::ContextTree*> trees = BuildContextTree::generateContextTrees(lexemeLines);
	std::string const cppCode = InterpretTree::treesToString(trees);

	BuildContextTree::deleteContextTrees(trees);

	writeCode(cppCode);
}
// */

