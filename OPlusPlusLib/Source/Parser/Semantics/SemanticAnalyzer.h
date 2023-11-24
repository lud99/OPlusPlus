#pragma once

#include "../Parser.h"
#include "../Nodes.h"

#include "SymbolTable.h"
#include "TypeTable.h"
#include "../CompileTimeErrorList.h"

namespace O
{
	class SemanticAnalyzer : public CompileTimeErrorList
	{
		// Does typechecking between assignments and in expressions
		// Validates syntax?
		// Handles resolving implicit casts and operator overloads
		// Function overload resolution
		// Validate no multiple declarations of variables
		// Creates symbol tables for each scope used in compilation

	public:
		EXPORT SemanticAnalyzer(AST::Node* program);

		EXPORT void AnalyzeProgram();


		TypeTableEntry& GetTypeOfNode(AST::Node* node, SymbolTable& localSymbolTable, TypeTable& localTypeTable);
		bool Typecheck(TypeTableEntry& lhs, TypeTableEntry& rhs);

		EXPORT auto& GetGlobalTypeTable() { return m_GlobalTypeTable; };


		EXPORT ~SemanticAnalyzer();

	private:
		void Analyze(AST::Node* node, SymbolTable& localSymbolTable, TypeTable& localTypeTable);
		void AnalyzeScope(AST::Scope* scope);

		void CreateTablesForScope(AST::Scope* node, SymbolTable& localSymbolTable, TypeTable& localTypeTable);
		
		VariableSymbol* CreateSymbolForVariableDeclaration(AST::VariableDeclaration* node, SymbolTable& localSymbolTable, TypeTable& localTypeTable);
		CallableSymbol* CreateSymbolForFunctionDeclaration(AST::FunctionDefinitionStatement* node, SymbolTable& localSymbolTable, TypeTable& localTypeTable);
		
		bool DoesTypesMatch(TypeTable& localTypeTable, TypeTableEntry& expectedType, TypeTableEntry& otherType);

		void MakeError(const std::string& message, CompileTimeError::Severity severity = CompileTimeError::Error);

		void MakeErrorAlreadyDefined(const std::string symbolName, SymbolType symbolType);
		void MakeErrorNotDefined(const std::string symbolName);

	private:
		AST::Node* m_Program;

		SymbolTable* m_GlobalSymbolTable;
		TypeTable* m_GlobalTypeTable;
	};

}