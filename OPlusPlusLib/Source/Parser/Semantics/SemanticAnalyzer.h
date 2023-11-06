#pragma once

#include "../Parser.h"
#include "../Nodes.h"

#include "SymbolTable.h"
#include "TypeTable.h"

namespace O
{
	class SemanticAnalyzer
	{
		// Does typechecking between assignments and in expressions
		// Validates syntax?
		// Handles resolving implicit casts and operator overloads
		// Function overload resolution
		// Validate no multiple declarations of variables
		// Creates symbol tables for each scope used in compilation

	public:
		SemanticAnalyzer(AST::Node* program);


		void Analyze(AST::Node* node, SymbolTable* upwardSymbolTable, SymbolTable* localSymbolTable);

	public:

	private:
		AST::Node* m_Program;

		SymbolTable m_SymbolTable;
		TypeTable m_TypeTable;
	};

}