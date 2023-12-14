#pragma once

#include "../Parser.h"
#include "../Nodes.h"

#include "SymbolTable.h"
#include "TypeTable.h"
#include "../CompileTimeErrorList.h"

namespace O
{
	using namespace AST;

	struct CallableSignature
	{
		std::vector<TypeId> parameterTypes;
		TypeId returnType;
	};

	class OperatorDefinitions
	{
	public:
		OperatorDefinitions();

	private:
		void GeneratePrimitiveOperators(TypeId type);
		void GenerateAssignmentOperators(TypeId type);

	public:
		std::unordered_map<Operators::Name, std::vector<CallableSignature>> m_OperatorSignatures;

		std::unordered_map<Operators::Name, std::vector<CallableSignature>> m_BuiltInOperatorDefinitions;
		std::unordered_map<Operators::Name, std::vector<Nodes::FunctionDefinitionStatement>> m_OverloadedOperatorDefinitions;
	};

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


		Type& GetTypeOfNode(AST::Node* node, SymbolTable& localSymbolTable, TypeTable& localTypeTable);

		bool Typecheck(Type& lhs, Type& rhs);

		EXPORT auto& GetGlobalTypeTable() { return m_GlobalTypeTable; };


		EXPORT ~SemanticAnalyzer();

	private:
		void Analyze(AST::Node* node, SymbolTable& localSymbolTable, TypeTable& localTypeTable);
		void AnalyzeScope(Nodes::Scope* scope);

		void GetReturnTypes(AST::Node* node, std::vector<Type>& returnTypes, SymbolTable& localSymbolTable, TypeTable& localTypeTable);

		void CreateTablesForScope(Nodes::Scope* node, SymbolTable& localSymbolTable, TypeTable& localTypeTable);
		
		VariableSymbol* CreateSymbolForVariableDeclaration(Nodes::VariableDeclaration* node, SymbolTable& localSymbolTable, TypeTable& localTypeTable, VariableSymbolType variableType);
		CallableSymbol* CreateSymbolForFunctionDeclaration(Nodes::FunctionDefinitionStatement* node, SymbolTable& localSymbolTable, TypeTable& localTypeTable, bool isMethod = false);

		VariableSymbol* CreateSymbolForClassMemberDeclaration(Nodes::VariableDeclaration* node, ClassSymbol& classSymbol);
		CallableSymbol* CreateSymbolForMethodDeclaration(Nodes::FunctionDefinitionStatement* node, ClassSymbol& classSymbol);

		std::vector<TypeId> CreateSymbolsForCallableDefinition(Nodes::FunctionDefinitionStatement* node);
		std::optional<Type> AnalyzeCallableDefinition(Nodes::FunctionDefinitionStatement* node, SymbolTable& localSymbolTable, TypeTable& localTypeTable, std::optional<Type> returnType);

		bool DoesTypesMatchThrowing(TypeTable& localTypeTable, Type& otherType, Type& expectedType);
		bool DoesTypesMatch(TypeTable& localTypeTable, Type& otherType, Type& expectedType);

		std::optional<CallableSignature> ResolveOverload(TypeTable& localTypeTable, std::vector<CallableSignature> overloads, std::vector<Type> arguments);



		void MakeError(const std::string& message, CompileTimeError::Severity severity = CompileTimeError::Error);

		void MakeErrorAlreadyDefined(const std::string symbolName, SymbolType symbolType);
		void MakeErrorNotDefined(const std::string symbolName);
		void MakeErrorInvalidCallableName(const std::string symbolName, SymbolType symbol);
		void MakeErrorInvalidDeclaredType(const std::string symbolName, const std::string declaredType, const std::string expetedType);

	private:
		AST::Node* m_Program;

		OperatorDefinitions m_OperatorDefinitions;

		std::unordered_map<AST::Node*, CallableSignature> m_ResolvedOverloadCache;

		SymbolTable* m_GlobalSymbolTable;
		TypeTable* m_GlobalTypeTable;
	};

}