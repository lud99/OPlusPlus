#pragma once

#include "macro.h"

#include <string>

#include "Parser/Lexer.h"
#include "Parser/Parser.h"
//#include "Interpreter/Bytecode/BytecodeInterpreter.h"

// Prints hello 
extern "C" EXPORT void hello_world(std::string thing = "Hello world!");