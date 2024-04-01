#pragma once

#include "../Parser.h"
#include "../Nodes.h"

#include "SymbolTypeTable.h"
#include "../CompileTimeErrorList.h"

namespace O
{
	using namespace AST;

	typedef std::optional<const Type*> OptType;

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
		std::vector<const O::Type*> parameterTypes;
		const O::Type* returnType;

		std::string name;
		CallableSymbolType callableKind;
	};

	class OperatorDefinitions
	{
	public:
		OperatorDefinitions() {};
		
		void Create(SymbolTypeTable& table);

	private:
		void CreateArithmeticValueExprOperators(TypeId type);
		void CreateBooleanValueExprOperators(TypeId type);

		// Also generates assignent operators
		void CreateArithmeticPlaceExprOperators(TypeId type);
		
		void CreateDirectAssignmentOperator(TypeId type, TypeId referenceType);
		void CreateCompoundAssignmentOperators(TypeId type, TypeId referenceType);

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

		const Type* GetTypeOfExpression(AST::Node* node, SymbolTypeTable& table);
		const Type* ResolveTypeNode(AST::Nodes::Type* node, SymbolTypeTable& table);

		EXPORT auto& GetGlobalTypeTable() { return m_GlobalSymbolTypeTable; };
		EXPORT auto& GetCachedTypes() { return m_ResolvedOverloadCache; };
		SymbolTypeTable* GetSymbolTypeTableForNode(AST::Node* node);
		bool HasTableForNode(AST::Node* node);


		EXPORT ~SemanticAnalyzer();

	private:
		void Analyze(AST::Node* node, SymbolTypeTable& table, OptType expectedType = {});
		void AnalyzeScope(Nodes::Scope* scope);

		// Returns all matching properties on the object
		ResolvedMemberAccess AnalyzeMemberAccess(AST::Node* node, SymbolTypeTable& table, OptType expectedType = {});
		std::optional<Symbol*> AnalyzeScopeResolution(AST::Node* node, SymbolTypeTable& table, OptType expectedType = {});
		void AnalyzeUnaryExpression(AST::Nodes::UnaryExpression* node, SymbolTypeTable& table, OptType expectedType = {});
		void AnalyzeBinaryExpression(AST::Nodes::BinaryExpression* node, SymbolTypeTable& table, OptType expectedType = {});


		void GetReturnTypes(AST::Node* node, std::vector<const Type*>& returnTypes, SymbolTypeTable& table, OptType expectedType = {});

		SymbolTypeTable* CreateSymbolTypeTable(SymbolTableType tableKind, SymbolTypeTable* upwardTable);

		void CreateTablesForScope(Nodes::Scope* node, SymbolTypeTable* upwardTable);

		std::optional<CallableSignature> ResolveOperatorOverload(Nodes::OperatorExpression* expression, SymbolTypeTable& table, std::vector<const Type*> arguments, OptType expectedType = {});


		VariableSymbol* CreateSymbolForVariableDeclaration(Nodes::VariableDeclaration* node, SymbolTypeTable& table, VariableSymbolType variableType);
		CallableSymbol* CreateSymbolForFunctionDeclaration(Nodes::FunctionDefinitionStatement* node, SymbolTypeTable& table, bool isMethod = false);

		VariableSymbol* CreateSymbolForClassMemberDeclaration(Nodes::VariableDeclaration* node, ClassSymbol& classSymbol);
		CallableSymbol* CreateSymbolForMethodDeclaration(Nodes::FunctionDefinitionStatement* node, ClassSymbol& classSymbol);

		std::vector<TypeId> CreateSymbolsForCallableParameters(Nodes::FunctionDefinitionStatement* node);
		OptType AnalyzeCallableDefinition(Nodes::FunctionDefinitionStatement* node, SymbolTypeTable& table, OptType declaredReturnType);
		CallableSymbol* CreateCallableSymbol(Nodes::FunctionDefinitionStatement* node, SymbolTypeTable& table, const std::string& callableName, CallableSymbolType callableKind, std::vector<O::TypeId> parameterTypeIds, const Type* returnType);

		bool IsCallableDeclarationUnique(SymbolTypeTable& table, const std::string& callableName, std::vector<O::TypeId> parameterTypeIds, const Type* returnType);

		Symbol* GetSymbolForNode(AST::Node* node, SymbolTypeTable& table);

		bool DoesTypesMatchThrowing(TypeTable& localTypeTable, const Type* otherType, const Type* expectedType);
		bool DoesTypesMatch(TypeTable& localTypeTable, const Type* otherType, const Type* expectedType);

		std::optional<CallableSignature> ResolveOverload(TypeTable& localTypeTable, std::vector<CallableSignature> overloads, DetailedCallableSignature calle, OptType expectedReturnType = {});
		
		void SetTableForNode(AST::Node* node, SymbolTypeTable* table);


		const Type* InsertArray(const Type* underlyingType, TypeTable& localTypeTable);
		//O::Type& InsertTuple(std::vector<O::Type> underlyingTypes, TypeTable& localTypeTable);
		//O::Type& InsertFunction(std::vector<O::Type> argumentTypes, O::Type returnType);

		void MakeError(const std::string& message, CompileTimeError::Severity severity = CompileTimeError::Error);

		void MakeErrorAlreadyDefined(const std::string symbolName, SymbolType symbolType);
		void MakeErrorCallableAlreadyDefined(const std::string symbolName, SymbolType symbolType, CallableSignature signature, TypeTable& types);
		void MakeErrorNotDefined(const std::string symbolName);
		void MakeErrorInvalidCallableName(const std::string symbolName, SymbolType symbol);
		void MakeErrorInvalidDeclaredType(const std::string symbolName, const std::string declaredType, const std::string expetedType);
		void MakeErrorTypeInvalidProperty(const O::Type* type, const std::string property);
		void MakeErrorTypeCallableNotDefined(const std::string typeName, DetailedCallableSignature signature);
		void MakeErrorTypeCallableNotDefined(const std::string typeName, const std::string name);



	private:
		AST::Node* m_Program;

		OperatorDefinitions m_OperatorDefinitions;

		std::unordered_map<AST::Node*, CallableSignature> m_ResolvedOverloadCache;
		std::unordered_map<AST::Node*, ExpressionType> m_CachedExpressionTypes;
		std::unordered_map<AST::Node*, Symbol*> m_CachedSymbolsForNodes;
		std::unordered_map<AST::Node*, SymbolTypeTable*> m_TableForNode;

		SymbolTypeTable* m_GlobalSymbolTypeTable;
	};

}