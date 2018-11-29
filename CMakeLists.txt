cmake_minimum_required(VERSION 3.12)
project(C_TransCompiler)

set(CMAKE_CXX_STANDARD 17)

add_executable(C_TransCompiler main.cpp Compiler/BuildAST.cpp Compiler/BuildAST.h Compiler/BuildContextTree.cpp Compiler/BuildContextTree.h Compiler/CreateTypedLexemes.h Compiler/InterpretTree.cpp Compiler/InterpretTree.h Compiler/NewLexer.h Compiler/LexerStructs.cpp Compiler/Mistake.cpp Compiler/Mistake.h Compiler/ParseLexemes.cpp Compiler/Util.cpp Compiler/Util.h Language/Core.cpp Language/Core.h Language/Object.cpp Language/Object.h Language/ObjectOperators.cpp)