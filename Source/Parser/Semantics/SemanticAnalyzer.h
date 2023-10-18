#pragma once

#include "Parser.h"

namespace Ö
{
	struct TypedASTNode
	{
		ASTTypes type;
	};

	class SemanticAnalyzer
	{
		// Does typechecking between assignments and in expressions
		// Validates syntax?
		// Handles resolving implicit casts and operator overloads
		// Function overload resolution
		// Validate no multiple declarations of variables
		// Creates symbol tables for each scope used in compilation


	};

}