cmake_minimum_required(VERSION 3.12)
project(C_TransCompiler)

set(CMAKE_CXX_STANDARD 17)

add_executable(C_TransCompiler Ptitsa/Compiler/BuildAST.cpp Ptitsa/Compiler/BuildAST.h Ptitsa/Compiler/BuildContextTree.cpp Ptitsa/Compiler/BuildContextTree.h Ptitsa/Compiler/InterpretTree.cpp Ptitsa/Compiler/InterpretTree.h Ptitsa/Compiler/Lexer.h Ptitsa/Compiler/LexerStructs.cpp Ptitsa/Compiler/Mistake.cpp Ptitsa/Compiler/Mistake.h Ptitsa/Compiler/ParseTypedLexemes.cpp Ptitsa/Compiler/Util.cpp Ptitsa/Compiler/Util.h Ptitsa/Compiler/CreateTypedLexemes.cpp Ptitsa/Language/Core.cpp Ptitsa/Language/Core.h Ptitsa/Language/Object.cpp Ptitsa/Language/Object.h Ptitsa/Language/ObjectOperators.cpp Ptitsa/Ptitsa.cpp)