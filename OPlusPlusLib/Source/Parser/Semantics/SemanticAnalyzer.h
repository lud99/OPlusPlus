#pragma once

#include "../Parser.h"
#include "../Nodes.h"

#include "SymbolTypeTable.h"
#include "../CompileTimeErrorList.h"

namespace O
{
	using namespace AST;

	struct ResolvedMemberAccess
	{
		ClassSymbol* lhs = nullptr;
		std::vector<Symbol*> rhs;
	};

	enum ExpressionType {
		PlaceExpression,
		ValueExpression
	};

	struct CallableSignature
	{
		std::vector<TypeId> parameterTypes;
		TypeId returnType;
	};

	struct DetailedCallableSignature
	{
		std::vector<O::Type> parameterTypes;
		O::Type returnType;

		std::string name;
		CallableSymbolType callableKind;
	};

	class OperatorDefinitions
	{
	public:
		OperatorDefinitions();

	private:
		void GeneratePrimitiveOperators(TypeId type);
		void GenerateAssignmentOperators(TypeId type);

	public:
		//std::unordered_map<, std::vector<CallableSignature>> m_OperatorSignatures;

		std::unordered_map<Operators::Name, std::vector<CallableSignature>> m_OperatorSignatures;

		std::unordered_map<Operators::Name, std::vector<CallableSignature>> m_BuiltInOperatorDefinitions;
		std::unordered_map<Operators::Name, std::vector<Nodes::FunctionDefinitionStatement>> m_OverloadedOperatorDefinitions;
	};

	class SemanticAnalyzer : public CompileTimeErrorList
	{
	public:
		EXPORT SemanticAnalyzer(AST::Node* program);

		EXPORT void AnalyzeProgram();

		static int c;


		Type& GetTypeOfExpression(AST::Node* node, SymbolTypeTable& table);
		Type& ResolveTypeNode(AST::Nodes::Type* node, SymbolTypeTable& table);

		EXPORT auto& GetGlobalTypeTable() { return m_GlobalSymbolTypeTable; };
		EXPORT auto& GetCachedTypes() { return m_ResolvedOverloadCache; };


		EXPORT ~SemanticAnalyzer();

	private:
		void Analyze(AST::Node* node, SymbolTypeTable& table, std::optional<O::Type> expectedType = {});
		void AnalyzeScope(Nodes::Scope* scope);

		// Returns all matching properties on the object
		ResolvedMemberAccess AnalyzeMemberAccess(AST::Node* node, SymbolTypeTable& table, std::optional<O::Type> expectedType = {});
		std::optional<Symbol*> AnalyzeScopeResolution(AST::Node* node, SymbolTypeTable& table, std::optional<O::Type> expectedType = {});

		void GetReturnTypes(AST::Node* node, std::vector<Type>& returnTypes, SymbolTypeTable& table, std::optional<O::Type> expectedType = {});

		void CreateTablesForScope(Nodes::Scope* node, SymbolTypeTable& table);

		VariableSymbol* CreateSymbolForVariableDeclaration(Nodes::VariableDeclaration* node, SymbolTypeTable& table, VariableSymbolType variableType);
		CallableSymbol* CreateSymbolForFunctionDeclaration(Nodes::FunctionDefinitionStatement* node, SymbolTypeTable& table, bool isMethod = false);

		VariableSymbol* CreateSymbolForClassMemberDeclaration(Nodes::VariableDeclaration* node, ClassSymbol& classSymbol);
		CallableSymbol* CreateSymbolForMethodDeclaration(Nodes::FunctionDefinitionStatement* node, ClassSymbol& classSymbol);

		std::vector<TypeId> CreateSymbolsForCallableDefinition(Nodes::FunctionDefinitionStatement* node);
		std::optional<Type> AnalyzeCallableDefinition(Nodes::FunctionDefinitionStatement* node, SymbolTypeTable& table, std::optional<Type> declaredReturnType);

		Symbol* GetSymbolForNode(AST::Node* node, SymbolTypeTable& table);

		bool DoesTypesMatchThrowing(TypeTable& localTypeTable, Type& otherType, Type& expectedType);
		bool DoesTypesMatch(TypeTable& localTypeTable, Type& otherType, Type& expectedType);

		std::optional<CallableSignature> ResolveOverload(TypeTable& localTypeTable, std::vector<CallableSignature> overloads, DetailedCallableSignature calle, std::optional<O::Type> expectedReturnType = {});
		
		O::Type& InsertArray(O::Type& underlyingType, TypeTable& localTypeTable);
		//O::Type& InsertTuple(std::vector<O::Type> underlyingTypes, TypeTable& localTypeTable);
		//O::Type& InsertFunction(std::vector<O::Type> argumentTypes, O::Type returnType);

		void MakeError(const std::string& message, CompileTimeError::Severity severity = CompileTimeError::Error);

		void MakeErrorAlreadyDefined(const std::string symbolName, SymbolType symbolType);
		void MakeErrorCallableAlreadyDefined(const std::string symbolName, SymbolType symbolType, CallableSignature signature, TypeTable& types);
		void MakeErrorNotDefined(const std::string symbolName);
		void MakeErrorInvalidCallableName(const std::string symbolName, SymbolType symbol);
		void MakeErrorInvalidDeclaredType(const std::string symbolName, const std::string declaredType, const std::string expetedType);
		void MakeErrorTypeInvalidProperty(O::Type& type, const std::string property);
		void MakeErrorTypeCallableNotDefined(const std::string typeName, DetailedCallableSignature signature);
		void MakeErrorTypeCallableNotDefined(const std::string typeName, const std::string name);



	private:
		AST::Node* m_Program;

		OperatorDefinitions m_OperatorDefinitions;

		std::unordered_map<AST::Node*, CallableSignature> m_ResolvedOverloadCache;
		std::unordered_map<AST::Node*, ExpressionType> m_CachedExpressionTypes;
		std::unordered_map<AST::Node*, Symbol*> m_CachedSymbolsForNodes;

		SymbolTypeTable* m_GlobalSymbolTypeTable;
	};

}