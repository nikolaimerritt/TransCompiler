#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <set>

#include "Compiler/Lexer.h"
#include "Compiler/BuildContextTree.h"
#include "Compiler/Util.h"
#include "Compiler/InterpretTree.h"

std::string getCode()
{
    std::string const inputFile = "program.pti";
    std::ifstream file(inputFile);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void writeCode(std::string const & cppCode)
{
    std::string const outputFile = "program.cpp";
    std::ofstream output(outputFile);
    output << cppCode;
    output.close();
}

int main()
{
    std::vector<Lexer::LexemeLine> lexemeDoc = Lexer::createTypedLexemes(getCode());
    Lexer::parseTypedLexemes(lexemeDoc);
    std::vector<BuildContextTree::ContextTree> trees = BuildContextTree::generateContextTrees(lexemeDoc);
    std::cout << InterpretTree::treesToString(trees) << std::endl;

    return 0;
}