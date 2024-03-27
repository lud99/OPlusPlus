#include "SemanticAnalyzer.h"

#include "../../Utils.hpp"

namespace O
{
	using namespace AST;

	OperatorDefinitions::OperatorDefinitions()
	{
		GeneratePrimitiveOperators((TypeId)PrimitiveValueTypes::Integer);
		GenerateAssignmentOperators((TypeId)PrimitiveValueTypes::Integer);

		GeneratePrimitiveOperators((TypeId)PrimitiveValueTypes::Double);
		GenerateAssignmentOperators((TypeId)PrimitiveValueTypes::Double);

		// int = double
		// int = 5.0 <=> (int, double => int)

		// TODO: Add ops for booleans

		/* Operators for numerical types :
		{ PostfixIncrement, PlaceExpression }, 
		{ PostfixDecrement, PlaceExpression }
		{ PostfixIncrement, PlaceExpression }
		{ PostfixDecrement, PlaceExpression }

		{ UnaryPlus, ValueExpression }
		{ UnaryMinus, ValueExpression }
		{ LogicalNot, ValueExpression }

		{ Multiplication, ValueExpression }
		{ Division, ValueExpression }
		{ Remainder, ValueExpression }

		{ Addition, ValueExpression }
		{ Subtraction, ValueExpression }

		{ LessThan, ValueExpression }
		{ LessThanOrEqual, ValueExpression }
		{ GreaterThan, ValueExpression }
		{ GreaterThanOrEqual, ValueExpression }

		{ Equality, ValueExpression }
		{ NotEqual, ValueExpression }

	
		{ DirectAssignment, PlaceExpression }
		CompoundAssignmentSum, CompoundAssignmentDifference,
		CompoundAssignmentProduct, CompoundAssignmentQuotinent,
		arr : int[];
		let a = arr[0] // [] <=> (int[], int => int)
		let b: int& = arr[0] // [] <=> (int[], int => int&)
		let c = Animal.age; . <=> (Animal, "age" => int)
		let c: int = Animal.age + 5; . <=> (Animal, "age" => int)
		= (int&, int => int&)

		T& implicit subtyp till T
		*/
		/*
		Nullable,

		// p = 2
		Call,
		Subscript,
		MemberAccess,

		// p = 3
		Closure,
		PrefixIncrement, PrefixDecrement,
		UnaryPlus, UnaryMinus,
		LogicalNot, BitwiseNot,

		// Binary
		// p = 5
		Multiplication, Division, Remainder,

		// p = 6
		Addition, Subtraction,
		
		// p = 9
		LessThan, LessThanOrEqual,
		GreaterThan, GreaterThanOrEqual,

		// p = 10
		Equality, NotEqual,
		
		// p = 16
		DirectAssignment,
		CompoundAssignmentSum, CompoundAssignmentDifference,
		CompoundAssignmentProduct, CompoundAssignmentQuotinent,
		
		*/

		// Strings
		m_OperatorSignatures[Operators::Addition].push_back({ { PrimitiveValueTypes::String, PrimitiveValueTypes::String }, PrimitiveValueTypes::String });


	}

	void OperatorDefinitions::GeneratePrimitiveOperators(TypeId type)
	{
		//CallableSignature binarySignature = { { type, type }, type };
		//CallableSignature unarySignature = { { type }, type };

		//using namespace Operators;
		//struct Op
		//{
		//	Operators::Name name;
		//	ExpressionType expressionType;
		//};

		//std::vector<Op> primitiveUnaryOperators = {
		//	{ PostfixIncrement, PlaceExpression },
		//	{ PostfixDecrement, PlaceExpression },
		//	{ PostfixIncrement, PlaceExpression },
		//	{ PostfixDecrement, PlaceExpression },

		//	{ UnaryPlus, ValueExpression },
		//	{ UnaryMinus, ValueExpression },
		//	{ LogicalNot, ValueExpression },
		//};

		//std::vector<Name> primitiveUnaryOperatorsValueExpr = {
		//	UnaryPlus, UnaryMinus,
		//	BitwiseNot
		//};

		//std::vector<Operators::Name> primitiveBinaryOperatorsValueExpr = {
		//	Multiplication, Division, Remainder,
		//	Addition, Subtraction,
		//};
		//std::vector<Operators::Name> primitiveBinaryOperatorsPlaceExpr = {
		//	Multiplication, Division, Remainder,
		//	Addition, Subtraction,

		//	DirectAssignment,
		//	CompoundAssignmentSum, CompoundAssignmentDifference,
		//	CompoundAssignmentProduct, CompoundAssignmentQuotinent,
		//};

		//std::vector<Operators::Name> primitiveUnaryBooleanOperators = {
		//	LogicalNot
		//};
		//std::vector<Operators::Name> primitiveBinaryBooleanOperators = {
		//	LessThan, LessThanOrEqual,
		//	GreaterThan, GreaterThanOrEqual,
		//	Equality, NotEqual,
		//};

		//// Doubles and integers has all operators defined on them
		//for (Operators::Name op : primitiveUnaryOperators)
		//{
		//	m_OperatorSignatures[op].push_back({ { type }, type });
		//}
		//for (Operators::Name op : primitiveUnaryBooleanOperators)
		//{
		//	m_OperatorSignatures[op].push_back({ { type }, PrimitiveValueTypes::Bool });
		//}

		//for (Operators::Name op : primitiveBinaryOperators)
		//{
		//	m_OperatorSignatures[op].push_back({ { type, type }, type });
		//}
		//for (Operators::Name op : primitiveBinaryBooleanOperators)
		//{
		//	m_OperatorSignatures[op].push_back({ { type, type }, PrimitiveValueTypes::Bool });
		//}
	}

	void OperatorDefinitions::GenerateAssignmentOperators(TypeId type)
	{
		using namespace Operators;
		for (auto& [op, signature] : m_OperatorSignatures)
		{
			// Copy the signature from + to += if + has been defined
			// TODO: Check for constant types, then this should not be ok.
			if (op == Addition)
				m_OperatorSignatures[CompoundAssignmentSum] = signature;
			if (op == Subtraction)
				m_OperatorSignatures[CompoundAssignmentDifference] = signature;
			if (op == Multiplication)
				m_OperatorSignatures[CompoundAssignmentProduct] = signature;
			if (op == Division)
				m_OperatorSignatures[CompoundAssignmentQuotinent] = signature;
		}
	}

	SemanticAnalyzer::SemanticAnalyzer(AST::Node* program)
	{
		m_Program = program;
	}

	void SemanticAnalyzer::AnalyzeProgram()
	{
		SymbolTypeTable dummyTable;

		Analyze(m_Program, dummyTable);
	}

	void SemanticAnalyzer::AnalyzeScope(Nodes::Scope* scope)
	{
		for (auto& line : scope->m_Lines)
		{
			Analyze(line, scope->m_LocalTable);
			//if (HasError())
				//return;
		}
	}

	void SemanticAnalyzer::GetReturnTypes(AST::Node* node, std::vector<Type>& returnTypes, SymbolTypeTable& table, std::optional<O::Type> expectedType)
	{
		using namespace Nodes;
		if (HasError())
			return;

		switch (node->m_Type)
		{
		case NodeKind::EmptyStatement:
			break;
		case NodeKind::Program:
		case NodeKind::BlockStatement:
		{
			Scope* scope = (Scope*)node;
			for (AST::Node* line : scope->m_Lines)
			{
				assert(line);

				GetReturnTypes(line, returnTypes, scope->m_LocalTable, expectedType);

			}

			return;
		}
		case NodeKind::WhileStatement:
		{
			ConditionalStatement* statement = (ConditionalStatement*)node;
			BlockStatement* body = statement->m_Body;
			if (!body) return;

			GetReturnTypes(node, returnTypes, body->m_LocalTable, expectedType);

			return;
		}
		case NodeKind::ForStatement:
		{
			ForStatement* statement = (ForStatement*)node;
			BlockStatement* body = statement->m_Body;
			if (!body) return;

			GetReturnTypes(node, returnTypes, body->m_LocalTable, expectedType);

			return;
		}
		case NodeKind::IfStatement:
		{
			// Main body
			IfStatement* statement = (IfStatement*)node;
			BlockStatement* body = statement->m_Body;
			if (!body) return;

			GetReturnTypes(node, returnTypes, body->m_LocalTable, expectedType);

			// Else body
			BlockStatement* elseBody = statement->m_ElseArm;
			if (!elseBody) return;
			
			GetReturnTypes(node, returnTypes, elseBody->m_LocalTable, expectedType);

			return;
		}
		
		case NodeKind::LoopStatement:
			abort();
			break;
		case NodeKind::Closure:
			break;
		
		case NodeKind::Return:
		{
			ReturnStatement* returnStatement = (ReturnStatement*)node;

			// return; which implies returning void if no return values
			if (!returnStatement->m_ReturnValue)
			{
				returnTypes.push_back(*table.types.Lookup(PrimitiveValueTypes::Void));
				return;
			}

			O::Type type = GetTypeOfExpression(returnStatement->m_ReturnValue, table);
			if (HasError())
				return;

			returnTypes.push_back(type);
			return;
		}
			
		default:
			break;
		}
	}

	SymbolTypeTable CreateSymbolTypeTable(SymbolTableType tableKind, SymbolTypeTable& upwardTable)
	{
		if (tableKind == SymbolTableType::Global)
			return { SymbolTable(SymbolTableType::Global, nullptr), TypeTable(TypeTableType::Global, nullptr) };
		
		return { SymbolTable(SymbolTableType::Local, &upwardTable.symbols), TypeTable(TypeTableType::Local, &upwardTable.types) };
	}

	void SemanticAnalyzer::CreateTablesForScope(Nodes::Scope* node, SymbolTypeTable& table)
	{
		if (node->m_Type == NodeKind::Program)
		{
			node->m_LocalTable = CreateSymbolTypeTable(SymbolTableType::Global, table);
			m_GlobalSymbolTypeTable = &node->m_LocalTable;
		} else
		{
			node->m_LocalTable = CreateSymbolTypeTable(SymbolTableType::Local, table);
		}
	}

	VariableSymbol* SemanticAnalyzer::CreateSymbolForVariableDeclaration(Nodes::VariableDeclaration* node, SymbolTypeTable& table, VariableSymbolType variableKind)
	{
		// Case for typed variable declaration
		std::optional<Type> variableType;
		if (node->m_VariableType)
		{
			variableType = ResolveTypeNode(node->m_VariableType, table);

			Analyze(node->m_VariableType, table);
		}
		
		if (HasError())
			return nullptr;

		if (node->m_AssignedValue)
		{
			Analyze(node->m_AssignedValue, table, variableType);
			if (HasError())
				return nullptr;

			Type assignedValueType = GetTypeOfExpression(node->m_AssignedValue, table);
			if (HasError())
				return nullptr;

			// If variable has no type, then infer it from the assignment
			if (!node->m_VariableType)
				variableType = assignedValueType;

			// Makes errors if types dont match.
			if (!DoesTypesMatchThrowing(table.types, assignedValueType, variableType.value()))
				return nullptr;
		}
		else
		{
			// let x; x has no type annotated and no value to infer from, so error
			if (!node->m_VariableType)
			{
				MakeError("Variable '" + node->m_VariableName->ToString() + "' has no annotated or infered type");
				return nullptr;
			}
		}

		std::string variableName = node->m_VariableName->ToString();
		if (table.symbols.Has(variableName))
		{
			MakeErrorAlreadyDefined(variableName, SymbolType::Variable);
			return nullptr;
		}

		return table.symbols.InsertVariable(variableName, variableType.value().id, variableKind);
	}

	std::vector<Type> SortTypeEntries(TypeTable& localTypeTable, std::vector<Type> types)
	{
		std::sort(types.begin(), types.end(), [&](Type& t1, Type& t2) {
			return localTypeTable.GetHeightOfTypeRelation(t1) > localTypeTable.GetHeightOfTypeRelation(t2);
		});

		return types;
	}

	std::vector<TypeId> SemanticAnalyzer::CreateSymbolsForCallableDefinition(Nodes::FunctionDefinitionStatement* node)
	{
		std::vector<TypeId> parameterTypes;
		for (Nodes::VariableDeclaration* parameter : node->m_Parameters->m_Parameters)
		{
			auto symbol = CreateSymbolForVariableDeclaration(parameter, node->m_ParametersTable, VariableSymbolType::Local);
			if (HasError())
				return {};

			parameterTypes.push_back(symbol->m_DataType);
		}
		return parameterTypes;
	}

	CallableSymbol* SemanticAnalyzer::CreateSymbolForFunctionDeclaration(Nodes::FunctionDefinitionStatement* node, SymbolTypeTable& table, bool isMethod)
	{
		std::string functionName = node->m_Name->ToString();

		auto symbols = table.symbols.Lookup(functionName);
		bool isOverload = !symbols.empty();

		// Initialize symbol table for the function parameters to live in
		// They are not created in the body symbol table, as expressive functions has no scope node to attach the table to
		node->m_ParametersTable = CreateSymbolTypeTable(SymbolTableType::Local, table);

		auto parameterTypes = CreateSymbolsForCallableDefinition(node);

		std::optional<Type> declaredReturnType = {};
		if (node->m_ReturnType)
			declaredReturnType = ResolveTypeNode(node->m_ReturnType, table);

		auto returnTypeOpt = AnalyzeCallableDefinition(node, node->m_ParametersTable, declaredReturnType);
		if (HasError())
			return nullptr;
		assert(returnTypeOpt.has_value());
		Type returnType = returnTypeOpt.value();

		// Check if the parameter types are different from other functions with same name
		// If not, then alreadyDefined error

		for (Symbol* symbol : symbols)
		{
			CallableSymbol* function = (CallableSymbol*)symbol;
			bool isIdentical = true;

			if (parameterTypes.size() != function->m_ParameterTypes.size())
				continue;

			if (returnType.id != function->m_DataType)
				continue;

			// Assume they are identical and look for contradictions
			for (int i = 0; i < parameterTypes.size(); i++)
			{
				O::Type& parameterType = *node->m_ParametersTable.types.Lookup(parameterTypes[i]);
				O::Type& otherParameterType = *node->m_ParametersTable.types.Lookup(function->m_ParameterTypes[i]);

				if (parameterType.id != otherParameterType.id)
					isIdentical = false;
			}

			if (isIdentical)
			{
				MakeErrorCallableAlreadyDefined(functionName, SymbolType::Function, { parameterTypes, returnType.id }, table.types);
				return nullptr;
			}
		}

		// 	uint16_t functionId = m_ConstantsPool.AddAndGetFunctionReferenceIndex(functionName);
		uint16_t callableId = 0; // TODO: Implement properly

		CallableSymbol callable = CallableSymbol(functionName, SymbolType::Function, returnType.id, callableId, CallableSymbolType::Normal);
		callable.m_ParameterTypes = parameterTypes;

		std::vector<O::Type> parameterTypesFull;
		for (TypeId id : parameterTypes)
		{
			parameterTypesFull.push_back(*table.types.Lookup(id));
		}

		O::Type functionPointerType = table.types.InsertFunction(parameterTypesFull, returnType);

		m_ResolvedOverloadCache[node] = { parameterTypes, returnType.id };
		m_ResolvedOverloadCache[node->m_Name] = { {}, functionPointerType.id };

		return table.symbols.InsertCallable(callable);
	}

	VariableSymbol* SemanticAnalyzer::CreateSymbolForClassMemberDeclaration(Nodes::VariableDeclaration* node, ClassSymbol& classSymbol)
	{
		// TODO: name clash with local variables in class member name
		return CreateSymbolForVariableDeclaration(node, *classSymbol.m_Table, VariableSymbolType::Member);
	}

	CallableSymbol* SemanticAnalyzer::CreateSymbolForMethodDeclaration(Nodes::FunctionDefinitionStatement* node, ClassSymbol& classSymbol)
	{
		SymbolTypeTable& classTable = *classSymbol.m_Table;

		std::string methodName = node->m_Name->ToString();
		CallableSymbolType methodType = CallableSymbolType::Normal;

		// TODO: Add function overloading for constructos?
		auto symbols = classTable.symbols.Lookup(methodName);
		if (!symbols.empty())
		{
			// Special case for constructors as the class symbol has already been declared
			if (symbols[0]->m_SymbolType == SymbolType::Class)
			{
				ClassSymbol* symbol = (ClassSymbol*)symbols[0];
				if (symbol->m_Name != methodName)
				{
					MakeErrorInvalidCallableName(methodName, SymbolType::Class);
					return nullptr;
				}

				// Otherwise its a constructor
				methodType = CallableSymbolType::Constructor;
			}
		}

		// Initialize symbol table for the function parameters to live in
		// They are not created in the body symbol table, as expressive functions has no scope node to attach the table to
		node->m_ParametersTable = CreateSymbolTypeTable(SymbolTableType::Local, classTable);

		auto parameterTypes = CreateSymbolsForCallableDefinition(node);

		// Because this is a method, the first argument should be 'this'
		VariableSymbol* thisSymbol = (VariableSymbol*)classTable.symbols.Lookup("this")[0];
		assert(thisSymbol);

		if (methodType == CallableSymbolType::Normal)
			parameterTypes.insert(parameterTypes.begin(), thisSymbol->m_DataType);

		// TODO: figure out if operators should have 'this' as parameter
		assert(methodType != CallableSymbolType::Operator); 

		std::optional<Type> declaredReturnTypeOpt;
		if (node->m_ReturnType)
			declaredReturnTypeOpt = ResolveTypeNode(node->m_ReturnType, classTable);

		if (methodType == CallableSymbolType::Constructor)
		{
			Type classType = *classTable.types.Lookup(thisSymbol->m_DataType);

			// If a return type is specified for a constructor, it has to be the type of the class
			if (declaredReturnTypeOpt.has_value())
			{
				if (declaredReturnTypeOpt.value().id != classType.id)
				{
					MakeErrorInvalidDeclaredType(methodName, declaredReturnTypeOpt.value().name, classType.name);
					return nullptr;
				}
			} 
			else
			{
				// Otherwise infer the type should be same as the class if not specified
				declaredReturnTypeOpt = classType;
			}
		}

		auto returnTypeOpt = AnalyzeCallableDefinition(node, node->m_ParametersTable, declaredReturnTypeOpt);
		if (HasError())
			return nullptr;
		assert(returnTypeOpt.has_value());
		Type returnType = returnTypeOpt.value();


		// Check if the parameter types are different from other functions with same name
		// If not, then alreadyDefined error

		for (Symbol* symbol : symbols)
		{
			CallableSymbol* function = (CallableSymbol*)symbol;
			bool isIdentical = true;

			if (parameterTypes.size() != function->m_ParameterTypes.size())
				continue;

			if (returnType.id != function->m_DataType)
				continue;

			// Assume they are identical and look for contradictions
			for (int i = 0; i < parameterTypes.size(); i++)
			{
				O::Type& parameterType = *node->m_ParametersTable.types.Lookup(parameterTypes[i]);
				O::Type& otherParameterType = *node->m_ParametersTable.types.Lookup(function->m_ParameterTypes[i]);

				if (parameterType.id != otherParameterType.id)
					isIdentical = false;
			}

			if (isIdentical)
			{
				MakeErrorCallableAlreadyDefined(methodName, SymbolType::Function, { parameterTypes, returnType.id }, classTable.types);
				return nullptr;
			}
		}

		// 	uint16_t functionId = m_ConstantsPool.AddAndGetFunctionReferenceIndex(functionName);
		uint16_t callableId = 0; // TODO: Implement properly

		CallableSymbol callable = CallableSymbol(methodName, SymbolType::Method, returnType.id, callableId, methodType);
		callable.m_ParameterTypes = parameterTypes;

		std::vector<O::Type> parameterTypesFull;
		for (TypeId id : parameterTypes)
		{
			parameterTypesFull.push_back(*classSymbol.m_Table->types.Lookup(id));
		}

		O::Type functionPointerType = classSymbol.m_Table->types.InsertFunction(parameterTypesFull, returnType);

		m_ResolvedOverloadCache[node] = { parameterTypes, returnType.id };
		m_ResolvedOverloadCache[node->m_Name] = { {}, functionPointerType.id };

		return classTable.symbols.InsertCallable(callable);
	}

	std::optional<Type> SemanticAnalyzer::AnalyzeCallableDefinition(Nodes::FunctionDefinitionStatement* node, SymbolTypeTable& table, std::optional<Type> declaredReturnType)
	{
		using namespace Nodes;
		std::string functionName = node->m_Name->ToString();

		// Analayze body to look for errors
		Analyze(node->m_Body, table, declaredReturnType);

		// Analyze the body and look for the return statements
		// TODO: Does not work for expressive functions
		assert(node->m_Body->m_Type == NodeKind::BlockStatement);

		Scope* body = (Scope*)node->m_Body;

		std::vector<O::Type> returnValueTypes;
		GetReturnTypes(body, returnValueTypes, table, declaredReturnType);

		assert(table.types.GetHeightOfTypeRelation(*table.types.Lookup(PrimitiveValueTypes::Double)) == 2);
		assert(table.types.GetHeightOfTypeRelation(*table.types.Lookup(PrimitiveValueTypes::Bool)) == 0);
		assert(table.types.GetHeightOfTypeRelation(*table.types.Lookup(PrimitiveValueTypes::Integer)) == 1);
		assert(table.types.GetHeightOfTypeRelation(*table.types.Lookup(PrimitiveValueTypes::Void)) == 0);

		// Check if all of the types are implicitly compatible with the 'most general' type of the return types
		// (or if there is one one specified)
		// This is to find a sort of 'greatest common denominator' between them that accomodates all return values

		//Type returnType;

		auto sortedReturnTypes = SortTypeEntries(table.types, returnValueTypes);
		
		// No specified returnvalue, so try to infer it
		O::Type returnType;
		if (!declaredReturnType.has_value())
		{
			// If no return statements and no annoted type, so the return type has to be void
			if (sortedReturnTypes.empty())
				returnType = *table.types.Lookup(PrimitiveValueTypes::Void);
			else
				returnType = sortedReturnTypes[0];
		}
		else 
		{
			returnType = declaredReturnType.value();
		}

		std::vector<O::Type> compatible;
		for (auto& type : sortedReturnTypes) {
			if (type.id == returnType.id)
				continue;

			if (DoesTypesMatchThrowing(table.types, type, returnType))
				compatible.push_back(type);
		}

		if (HasError())
		{
			MakeError("Could not compile function '" + functionName + "'");
			return {};
		}

		return returnType;
	}

	Symbol* SemanticAnalyzer::GetSymbolForNode(AST::Node* node, SymbolTypeTable& table)
	{
		using namespace Nodes;

		switch (node->m_Type)
		{
		case O::AST::NodeKind::BasicType:
		case O::AST::NodeKind::ArrayType:
		case O::AST::NodeKind::TupleType:
		case O::AST::NodeKind::FunctionType:
		{
			// TODO: Add symbols to primitive types with methods
			abort();
			return nullptr;
		}
		case O::AST::NodeKind::Identifier:
		{
			break;
			Identifier* identifier = (Identifier*)node;
			auto symbols = table.symbols.Lookup(identifier->m_Name);
			assert(symbols.size() == 1);
			
			return symbols[0];
		}
		case O::AST::NodeKind::VariableDeclaration:
			break;
		case O::AST::NodeKind::BinaryExpression:
		{
			// Return resolved symbol in case of '::' and '.' operators that returns a symbol
			BinaryExpression* expression = (BinaryExpression*)node;

			if (!m_CachedSymbolsForNodes.count(node))
				return nullptr;

			if (expression->m_Operator.m_Name == Operators::Name::ScopeResolution)
				return m_CachedSymbolsForNodes[node];

			break;
		}
		case O::AST::NodeKind::UnaryExpression:
			break;
		case O::AST::NodeKind::CallExpression:
			break;
		case O::AST::NodeKind::TupleExpression:
			break;
		case O::AST::NodeKind::FunctionParameters:
			break;
		case O::AST::NodeKind::FunctionDefinition:
			break;
		case O::AST::NodeKind::ExpressionFunctionDefinition:
			break;
		case O::AST::NodeKind::LambdaExpression:
			break;
		case O::AST::NodeKind::BlockStatement:
			break;
		case O::AST::NodeKind::IfStatement:
			break;
		case O::AST::NodeKind::WhileStatement:
			break;
		case O::AST::NodeKind::ForStatement:
			break;
		case O::AST::NodeKind::LoopStatement:
			break;
		case O::AST::NodeKind::Closure:
			break;
		case O::AST::NodeKind::Continue:
			break;
		case O::AST::NodeKind::Break:
			break;
		case O::AST::NodeKind::Return:
			break;
		case O::AST::NodeKind::ClassDeclaration:
			break;
		case O::AST::NodeKind::IntLiteral:
			break;
		case O::AST::NodeKind::FloatLiteral:
			break;
		case O::AST::NodeKind::DoubleLiteral:
			break;
		case O::AST::NodeKind::BoolLiteral:
			break;
		case O::AST::NodeKind::StringLiteral:
			break;
		default:
			break;
		}

		//abort();

		return nullptr;
	}

	ResolvedMemberAccess SemanticAnalyzer::AnalyzeMemberAccess(AST::Node* node, SymbolTypeTable& table, std::optional<O::Type> expectedType)
	{
		// f(1).a
		// a.b
		// (a + b).c
		// (++a).c
		// a.f()
		// (statics?) Animal.count 
		// (maybe) { "a" }.a
		// [1, 2, 3].length
		// otherwise error

		// let f() ...
		// f.b
		// perhaps function pointers should have properties?

		using namespace Nodes;

		BinaryExpression* expr = (BinaryExpression*)node;

		Analyze(expr->m_Lhs, table);
		O::Type& lhsType = GetTypeOfExpression(expr->m_Lhs, table);

		switch (lhsType.kind)
		{
		case O::TypeKind::Incomplete:
		case O::TypeKind::Error:
			break;
		case O::TypeKind::Class:
		{
			// a.b
			// TODO: How wil this work on nested classes?
			// TODO: this code will run for class variables aswell, may want to change

			// lhs
			ClassSymbol* classSymbol = (ClassSymbol*)table.symbols.LookupOne(lhsType.name);

			//expr->m_Lhs
			
			// rhs

			// cases: 
			// .a
			// .f() 
			if (expr->m_Rhs->m_Type == NodeKind::Identifier) 
			{
				Identifier* prop = (Identifier*)expr->m_Rhs;
				auto results = classSymbol->m_Table->symbols.Lookup(prop->m_Name);
				if (results.empty()) 
				{
					MakeError("Member '" + prop->m_Name + "' doesn't exist on class " + lhsType.name);
					return {};
				}

				if (results.size() > 1)
				{
					std::vector<O::Type> types;
					for (Symbol* result : results)
					{
						assert(result->IsCallable());
						types.push_back(*table.types.Lookup(result->m_DataType));
					}

					// Create function pointer
					//table.types.InsertFunction(types, )
				}
				else
				{
					m_ResolvedOverloadCache[node] = { {}, results[0]->m_DataType };
				}

				return { classSymbol, results };

			}
			
			abort();
			
			break;
		}
		case O::TypeKind::Function:
		case O::TypeKind::Method:
		{
			MakeErrorTypeInvalidProperty(lhsType, expr->m_Rhs->ToString());
			return {};
		}
		case O::TypeKind::Array:
		{
			// TODO: Use some other way of storing all properties
			if (expr->m_Rhs->ToString() == "length")
			{
				m_ResolvedOverloadCache[node] = { {}, (TypeId)PrimitiveValueTypes::Integer };

				// TODO TODO: Memoryleak, super hacky
				return { nullptr,
					{ new VariableSymbol("length", SymbolType::Variable, (TypeId)PrimitiveValueTypes::Integer) } };
			}

			MakeErrorTypeInvalidProperty(lhsType, expr->m_Rhs->ToString());
			
			break;
		}
		case O::TypeKind::Primitive:
		case O::TypeKind::Tuple:
		case O::TypeKind::Nullable:
		{
			// TODO: Add methods to primitives
			MakeErrorTypeInvalidProperty(lhsType, expr->m_Rhs->ToString());
			return {};
		}
		case O::TypeKind::Typedef:
		default:
			abort();
		}

		return {};
	}

	bool IdentifierIsTypename(Nodes::Identifier* node, TypeTable& types) { return types.HasType(node->m_Name); }

	std::optional<Symbol*> SemanticAnalyzer::AnalyzeScopeResolution(AST::Node* node, SymbolTypeTable& table, std::optional<O::Type> expectedType)
	{
		using namespace Nodes;

		BinaryExpression* expr = (BinaryExpression*)node;
		Analyze(expr->m_Lhs, table);
		
		// Ex: A::B::C. A::B is resolved first, but to resolve C we need the symbol table of B

		ClassSymbol* parentSymbol = (ClassSymbol*)GetSymbolForNode(expr->m_Lhs, table);
		SymbolTypeTable* localTable = parentSymbol ? parentSymbol->m_Table : &table;

		O::Type lhsType = GetTypeOfExpression(expr->m_Lhs, *localTable);
		if (HasError())
			return {};


		// Cases:
		// ExprWithType::StaticProperty
		// ExprWithType::InnerType
		// ExprWithType::StaticMethod
		// 
		// TODO:
		//
		// static properties should only be accessed on objects with scope resolution, not property access

		switch (lhsType.kind)
		{
		case TypeKind::Class:
		{
			ClassSymbol* classSymbol = !parentSymbol ? (ClassSymbol*)table.symbols.LookupClassByType(lhsType.id) : parentSymbol;

			localTable = classSymbol->m_Table;
			Analyze(expr->m_Rhs, *classSymbol->m_Table);

			// Class::Identifier
			if (expr->m_Rhs->m_Type == NodeKind::Identifier)
			{
				Identifier* prop = (Identifier*)expr->m_Rhs;

				Symbol* member = classSymbol->m_Table->symbols.LookupOne(prop->m_Name);

				m_CachedSymbolsForNodes[node] = member;

				// The results could be multiple symbols if the identifier is a name for a function
				// TODO: Resolve overload based on expectedType

				return member;

			}

			break;
		}
		case TypeKind::Function:
		{
			MakeError("Scope Resolution can not be used on a function");
			return {};
		}
		
		default:
			break;
		}

#ifdef other
		switch (symbol->m_SymbolType)
		{
		case O::TypeKind::Incomplete:
		case O::TypeKind::Error:
			break;
		case O::TypeKind::Class:
		{
			// lhs
			ClassSymbol* classSymbol = (ClassSymbol*)table.symbols.LookupOne(lhsType.name);

			// Can access nested classes, stuff in namespaces and static members
			// TODO: Ensure members are static
			if (expr->m_Rhs->m_Type == NodeKind::Identifier)
			{
				Identifier* prop = (Identifier*)expr->m_Rhs;
				auto results = classSymbol->m_Table->symbols.Lookup(prop->m_Name);
				if (results.empty())
				{
					MakeError("Member '" + prop->m_Name + "' doesn't exist on class " + lhsType.name);
					return {};
				}

				if (results.size() > 1)
				{
					std::vector<O::Type> types;
					for (Symbol* result : results)
					{
						assert(result->IsCallable());
						types.push_back(*table.types.Lookup(result->m_DataType));
					}

					// Create function pointer
					//table.types.InsertFunction(types, )

				}
				else
				{
					m_ResolvedOverloadCache[node] = { {}, results[0]->m_DataType };
					m_CachedSymbolsForNodes[node] = results[0];
				}

				return { { classSymbol, results } };

			}

			abort();

			break;
		}
		case O::TypeKind::Function:
		case O::TypeKind::Method:
		{
			MakeErrorTypeInvalidProperty(lhsType, expr->m_Rhs->ToString());
			return {};
		}
		case O::TypeKind::Array:
		{
			MakeErrorTypeInvalidProperty(lhsType, expr->m_Rhs->ToString());

			break;
		}
		case O::TypeKind::Primitive:
		case O::TypeKind::Tuple:
		case O::TypeKind::Nullable:
		{
			// TODO: Add methods to primitives
			MakeErrorTypeInvalidProperty(lhsType, expr->m_Rhs->ToString());
			return {};
		}
		case O::TypeKind::Typedef:
		default:
			abort();
		}
#endif

		return {};
	}

	bool SemanticAnalyzer::DoesTypesMatchThrowing(TypeTable& localTypeTable, Type& otherType, Type& expectedType)
	{
		// 1. Case when types are the same
		if (expectedType.id == otherType.id)
			return true;


		// 2. typedefs

		// 3. type relations
		auto typeRelation = localTypeTable.GetFullTypeRelationTo(otherType, expectedType);
		if (!typeRelation.has_value())
		{
			MakeError("Incompatible types. '" + otherType.name + "' cannot be converted to '" + expectedType.name + "' as they have no relation");
			return false;
		}

		// Is valid if the otherType can be implicit converted to the supertype
		if (typeRelation.value() == TypeRelation::Implicit)
		{
			return true;
		}
		else 
		{
			MakeError("Incompatible types. '" + otherType.name + "' cannot be converted to '" + expectedType.name + "' implicitly");
			return false;
		}
		
		abort();
		return false;
	}

	bool SemanticAnalyzer::DoesTypesMatch(TypeTable& localTypeTable, Type& otherType, Type& expectedType)
	{
		// 1. Case when types are the same
		if (expectedType.id == otherType.id)
			return true;


		// 2. typedefs

		// 3. type relations
		auto typeRelation = localTypeTable.GetFullTypeRelationTo(otherType, expectedType);
		if (!typeRelation.has_value())
			return false;

		// Is valid if the otherType can be implicit converted to the supertype
		return typeRelation.value() == TypeRelation::Implicit;
	}

	std::optional<CallableSignature> SemanticAnalyzer::ResolveOverload(TypeTable& localTypeTable, std::vector<CallableSignature> overloads, DetailedCallableSignature calle, std::optional<O::Type> expectedReturnType)
	{
		struct SignatureWithSteps
		{
			int steps;
			CallableSignature signature;
		};

		std::vector<SignatureWithSteps> stepsToSignatures;
		for (CallableSignature signature : overloads)
		{
			bool allArgumentsMatched = true;

			// Must have same number of args. TODO: Add default values for parameters
			if (signature.parameterTypes.size() != calle.parameterTypes.size())
				continue;

			// Check if each argument type is compatible with corresponding parameter type
			for (int i = 0; i < signature.parameterTypes.size(); i++)
			{
				O::Type& parameter = *localTypeTable.Lookup(signature.parameterTypes[i]);
				O::Type& argument = calle.parameterTypes[i];

				if (!DoesTypesMatch(localTypeTable, argument, parameter))
				{
					allArgumentsMatched = false;
					break;
				}
			}

			if (allArgumentsMatched)
			{
				// If they are, determine how far away each of the arguments are from the actuall parameter type
				// This list is then sorted afterwards to determine which overload to use

				int totalSteps = 0;
				for (int i = 0; i < signature.parameterTypes.size(); i++)
				{
					O::Type& parameter = *localTypeTable.Lookup(signature.parameterTypes[i]);
					O::Type& argument = calle.parameterTypes[i];

					totalSteps += localTypeTable.GetHeightOfTypeRelation(parameter) - localTypeTable.GetHeightOfTypeRelation(argument);
				}

				stepsToSignatures.push_back({ totalSteps, signature });
			}
		}

		if (stepsToSignatures.empty())
			return {};

		// Sort the matches and choose the closest one based on how many types differs
		std::sort(stepsToSignatures.begin(), stepsToSignatures.end(), [](SignatureWithSteps s1, SignatureWithSteps s2) {
			return s1.steps < s2.steps;
		});

		// Put the all matches with the same steps as the closest one, to get all closest matches
		std::vector<SignatureWithSteps> closestMatches;
		int firstSteps = stepsToSignatures[0].steps;
		for (int i = 0; i < stepsToSignatures.size(); i++)
		{
			if (stepsToSignatures[i].steps != firstSteps)
			{
				// If only one overload matches, then choose that one
				if (i == 1)
					return stepsToSignatures[0].signature;

				break;
			}

			closestMatches.push_back(stepsToSignatures[i]);
		}

		if (stepsToSignatures.size() == 1)
			return stepsToSignatures[0].signature;
		
		std::string calleArgumentTypesString = Join(calle.parameterTypes, std::string(", "), [](O::Type& t) { return t.name; });
		std::string calleSignatureString = "(" + calleArgumentTypesString + " => " + calle.returnType.name + ")";


		// multiple matches, but could not determine which to use
		if (!expectedReturnType.has_value())
		{
			MakeError_Void("Found multiple matching overloaded functions for  " + calle.name + " , but could not determine which one to use. Argument types are " + calleArgumentTypesString, Token());
			return {};
		}

		// Should return type overloading only match if the return type and expected type are equivalent, or if they are compatible?
		// Compatible: Might make it confusing which overload is choosen, and cases where f: int and f: double would both be matches
		// Equivalent: More strict and may reduce unintented behavior. But issue with "let a: int = 2.0 + 2.0". 
		// double + double has no overload with int, so wont compile.
		// For now: do "compatible" variant.

		// Steps are the steps to the return type only
		std::vector<SignatureWithSteps> potentialMatchesReturnType;
		for (auto& match : closestMatches)
		{
			O::Type& returnType = *localTypeTable.Lookup(match.signature.returnType);
			if (localTypeTable.IsTypeImplicitSubtypeOf(returnType, expectedReturnType.value()))
			{
				int steps = localTypeTable.GetHeightOfTypeRelation(expectedReturnType.value()) - localTypeTable.GetHeightOfTypeRelation(returnType);

				potentialMatchesReturnType.push_back({ steps, match.signature });
			}
		}

		if (potentialMatchesReturnType.empty())
		{
			// TODO: hint system for printing potential functions
			MakeError_Void("Found multiple matching overloads for " + calle.name + " based on argument types, but " +  
				"none of them matched with the expected return type " + expectedReturnType.value().name, Token());

			for (auto& match : closestMatches) {
				MakeError_Void("Potential match based only on arguments", Token(), CompileTimeError::Info);
			}

			return {};
		}

		// Do as with arguments: Choose the ones with least steps

		if (potentialMatchesReturnType.size() > 1)
		{
			MakeError_Void("Found multiple matching return-type overloaded functions for  " + calle.name + " , but could not determine which one to use. Argument types are " + calleArgumentTypesString, Token());
			return {};
		}

		return potentialMatchesReturnType[0].signature;
	}

	O::Type& SemanticAnalyzer::InsertArray(O::Type& underlyingType, TypeTable& localTypeTable)
	{
		bool typeExisted = false;
		O::Type& type = localTypeTable.InsertArray(underlyingType, typeExisted);

		if (!typeExisted)
		{
			m_OperatorDefinitions.m_OperatorSignatures[Operators::Subscript]
				.push_back({ { type.id, (TypeId)PrimitiveValueTypes::Integer }, underlyingType.id });
		}
		
		return type;
	}

	//O::Type& SemanticAnalyzer::InsertTuple(std::vector<O::Type> underlyingTypes, TypeTable& localTypeTable)
	//{
	//	// TODO: tuples of different types cannot have their accessed type known at compiletime since the index might be a variable

	//}

	// Create symbol tables for each scope
	void SemanticAnalyzer::Analyze(AST::Node* node, SymbolTypeTable& table, std::optional<O::Type> expectedType)
	{
		using namespace Nodes;
		if (HasError())
			return;

		switch (node->m_Type)
		{
		case NodeKind::EmptyStatement:
			return;
		case NodeKind::Program:
		case NodeKind::BlockStatement:
		{
			Scope* scope = (Scope*)node;
			CreateTablesForScope(scope, table);
			
			for (auto& line : scope->m_Lines)
			{
				Analyze(line, scope->m_LocalTable, expectedType);
			}

			break;
		}
		case NodeKind::BasicType:
			break;
		case NodeKind::Identifier:
		{
			Identifier* identifier = (Identifier*)node;
			
			if (!table.symbols.Has(identifier->m_Name))
				return MakeErrorNotDefined(identifier->m_Name);

			break;
		}

		// For normal variable declarations in scopes (not class members)
		case NodeKind::VariableDeclaration:
		{
			CreateSymbolForVariableDeclaration((VariableDeclaration*)node, table, VariableSymbolType::Local);
			return;
		}

		case NodeKind::BinaryExpression:
		{
			BinaryExpression* expression = (BinaryExpression*)node;
			if (expression->m_Operator.m_Name == Operators::Name::ScopeResolution)
			{
				std::optional<Symbol*> symbol = AnalyzeScopeResolution(expression, table, expectedType);
				assert(symbol.has_value());
				m_ResolvedOverloadCache[node] = { {}, symbol.value()->m_DataType };

				return;
			}

			if (expression->m_Operator.m_Name == Operators::Name::MemberAccess)
			{
				auto resolved = AnalyzeMemberAccess(expression, table, expectedType);
				assert(resolved.rhs.size() == 1);
				m_ResolvedOverloadCache[node] = { {}, resolved.rhs[0]->m_DataType };

				return;
			}

			Analyze(expression->m_Lhs, table, expectedType);
			if (HasError())
				return;


			Analyze(expression->m_Rhs, table, expectedType);
			if (HasError())
				return;

			O::Type& lhs = GetTypeOfExpression(expression->m_Lhs, table);
			O::Type& rhs = GetTypeOfExpression(expression->m_Rhs, table);

			DetailedCallableSignature calleSignature = {
				{ lhs, rhs },
				{},
				expression->m_Operator.ToString(),
				CallableSymbolType::Operator
			};

			auto operatorOpt = ResolveOverload(table.types, m_OperatorDefinitions.m_OperatorSignatures[expression->m_Operator.m_Name], calleSignature, expectedType);
			if (!operatorOpt.has_value())
			{
				MakeError("Operator " + expression->m_Operator.m_Symbol + " not defined for types " + lhs.name + " and " + rhs.name);
				return;
			}

			m_ResolvedOverloadCache[node] = operatorOpt.value();

			break;
		}
		case NodeKind::UnaryExpression:
		{
			UnaryExpression* expression = (UnaryExpression*)node;
			Analyze(expression->m_Operand, table, expectedType);
			if (HasError())
				return;

			O::Type& operand = GetTypeOfExpression(expression->m_Operand, table);

			DetailedCallableSignature calleSignature = {
				{ operand },
				{},
				expression->m_Operator.ToString(),
				CallableSymbolType::Operator
			};

			auto operatorOpt = ResolveOverload(table.types, m_OperatorDefinitions.m_OperatorSignatures[expression->m_Operator.m_Name], calleSignature, expectedType);
			if (!operatorOpt.has_value())
			{
				MakeError("Operator " + expression->m_Operator.m_Symbol + " not defined for " + operand.name);
				return;
			}

			m_ResolvedOverloadCache[node] = operatorOpt.value();

			break;
		}
		case NodeKind::CallExpression:
		{
			CallExpression* call = (CallExpression*)node;

			// Check if the function is called on a method or instance of function pointer type.

			std::vector<O::Symbol*> matchingFunctions;
			bool isCallingMethod = false;
			Symbol* calleeSymbol = nullptr;
			if (call->m_Callee->m_Type == NodeKind::BinaryExpression)
			{
				BinaryExpression* expr = (BinaryExpression*)call->m_Callee;
				if (expr->m_Operator.m_Name == Operators::Name::MemberAccess)
				{
					auto resolved = AnalyzeMemberAccess(expr, table);
					calleeSymbol = resolved.lhs;
					matchingFunctions = resolved.rhs;

					if (HasError())
						return;

					isCallingMethod = true;

					m_ResolvedOverloadCache[expr] = { {}, resolved.rhs[0]->m_DataType };
				}
			}

			if (!isCallingMethod)
				Analyze(call->m_Callee, table, expectedType);

			if (HasError())
				return; 
			
			// Validate arguments
			std::vector<O::Type> functionSignature;
			for (auto& argument : call->m_Arguments)
			{
				Analyze(argument, table, expectedType);
			}

			if (HasError())
				return;

			// If calling a normal function, then look for symbols with it's name in the scope
			std::string callee;
			std::optional<O::Type> calledOnType = {};
			if (!isCallingMethod)
			{
				callee = call->m_Callee->ToString();
				matchingFunctions = table.symbols.Lookup(callee);
				if (matchingFunctions.empty())
					return MakeErrorNotDefined(callee);
			}
			else
			{
				O::Type& calledOnType = *table.types.Lookup(calleeSymbol->m_DataType);

				if (matchingFunctions.empty())
					return MakeErrorTypeCallableNotDefined(calledOnType.name, "!!TODO!!");

				callee = matchingFunctions[0]->m_Name;
			}

			// If function has name of a class, then the class constructor should be used
			// Look it up in the class table
			if (matchingFunctions[0]->m_SymbolType == SymbolType::Class)
			{
				assert(matchingFunctions.size() == 1);
				ClassSymbol* classSymbol = (ClassSymbol*)matchingFunctions[0];

				matchingFunctions = classSymbol->m_Table->symbols.Lookup(callee);
			}

			// Create signature object
			std::vector<CallableSignature> matchingCallableSignatures;
			
			for (Symbol* symbol : matchingFunctions)
			{
				// If function has name of a class, then the class constructor should be used
				// Look it up in the class table
				if (symbol->m_SymbolType == SymbolType::Variable)
					abort();

				CallableSymbol* callable = (CallableSymbol*)symbol;
				matchingCallableSignatures.push_back({ callable->m_ParameterTypes, callable->m_DataType });
			}

			DetailedCallableSignature calleSignature = {
				{},
				{},
				callee,
				CallableSymbolType::Normal,
			};

			// Add the class instance the object is access on (in the case of method invokation)
			if (isCallingMethod)
				calleSignature.parameterTypes.push_back(*table.types.Lookup(calleeSymbol->m_DataType));

			for (O::AST::Node* argument : call->m_Arguments) 
			{
				calleSignature.parameterTypes.push_back(GetTypeOfExpression(argument, table));
			}

			auto matchingCallableOpt = ResolveOverload(table.types, matchingCallableSignatures, calleSignature, expectedType);
			if (!matchingCallableOpt.has_value())
			{
				if (isCallingMethod)
				{
					// TODO: Better error
					O::Type& calledOnType = *table.types.Lookup(calleeSymbol->m_DataType);
					return MakeError("No matching function '" + callee + "' found on " + calledOnType.name);
					//return MakeErrorTypeCallableNotDefined(calledOnType.name, callee);
				}

				return MakeErrorNotDefined(callee);
			}

			m_ResolvedOverloadCache[node] = matchingCallableOpt.value();

			if (isCallingMethod)
			{
				O::Type returnType = *table.types.Lookup(matchingCallableOpt.value().returnType);

				m_ResolvedOverloadCache[call->m_Callee] = { {}, table.types.InsertFunction(calleSignature.parameterTypes, returnType).id };
			}

			break;
		}
		case NodeKind::TupleExpression:
		{
			TupleExpression* tuple = (TupleExpression*)node;
			for (auto& element : tuple->m_Elements)
			{
				Analyze(element, table, expectedType);
			}

			std::vector<O::Type> elementTypes;
			std::vector<TypeId> elementTypeIds;
			for (AST::Node* element : tuple->m_Elements)
			{
				auto& type = GetTypeOfExpression(element, table);
				if (HasError()) return;

				elementTypes.push_back(type);
				elementTypeIds.push_back(type.id);
			}

			// Cache the type of the tuple
			m_ResolvedOverloadCache[node] = { elementTypeIds, table.types.InsertTuple(elementTypes).id };

			break;
		}
		case NodeKind::FunctionDefinition:
		{
			FunctionDefinitionStatement* functionNode = (FunctionDefinitionStatement*)node;

			CallableSymbol* function = CreateSymbolForFunctionDeclaration((FunctionDefinitionStatement*)node, table);
			if (HasError())
				return;
			
			std::optional<O::Type> returnType = {};
			if (functionNode->m_ReturnType)
				returnType = ResolveTypeNode(functionNode->m_ReturnType, table);

			// Analyze the body
			// TODO: Ensure a return exists
			// TODO: Typecheck returned type and function return type
			Analyze(functionNode->m_Body, functionNode->m_ParametersTable, returnType);

			return;
		}

		case NodeKind::ExpressionFunctionDefinition:
			break;
		case NodeKind::IfStatement:
		{
			IfStatement* ifStatement = (IfStatement*)node;
			Analyze(ifStatement->m_Condition, table, expectedType);
			Analyze(ifStatement->m_Body, table, expectedType);
			if (ifStatement->m_ElseArm)
				Analyze(ifStatement->m_ElseArm, table, expectedType);
			
			break;
		}
		case NodeKind::WhileStatement:
		{
			WhileStatement* whileStatement = (WhileStatement*)node;
			Analyze(whileStatement->m_Condition, table, expectedType);
			Analyze(whileStatement->m_Body, table, expectedType);

			break;
		}
		case NodeKind::ForStatement:
		{
			ForStatement* forStatement = (ForStatement*)node;

			if (forStatement->m_Initialization)
				Analyze(forStatement->m_Initialization, table, expectedType);
			if (forStatement->m_Condition)
				Analyze(forStatement->m_Condition, table, expectedType);
			if (forStatement->m_Advancement)
				Analyze(forStatement->m_Advancement, table, expectedType);

			Analyze(forStatement->m_Body, table, expectedType);

			break;
		}
		case NodeKind::LoopStatement:
			break;
		case NodeKind::Closure:
			break;
		case NodeKind::Continue:
			break;
		case NodeKind::Break:
			break;
		case NodeKind::Return:
		{
			ReturnStatement* returnNode = (ReturnStatement*)node;

			// Returns void, nothing to analyze
			if (!returnNode->m_ReturnValue)
				return;

			Analyze(returnNode->m_ReturnValue, table, expectedType);

			break;
		}
		case NodeKind::ClassDeclaration:
		{
			ClassDeclarationStatement* classNode = (ClassDeclarationStatement*)node;

			std::string name = classNode->m_Name->ToString();

			if (table.types.HasCompleteType(name))
				return MakeErrorAlreadyDefined(name, SymbolType::Class);

			O::Type classType = table.types.Insert(name, TypeKind::Class);

			classNode->m_ClassSymbol = table.symbols.InsertClass(name, classType.id, &table.symbols, &table.types);
			auto& classSymbol = *classNode->m_ClassSymbol;
			
			// Add the 'this' symbol 
			// TODO: wont work for nested classes (Solution is to remove it maybe in some good way)
			classSymbol.m_Table->symbols.InsertVariable("this", classType.id, VariableSymbolType::Local);
			
			// TODO: The order should match the order they are declared in probably
			for (auto declaration : classNode->m_NestedClassDeclarations) {
				Analyze(declaration, *classSymbol.m_Table);
			}
			for (auto declaration : classNode->m_MemberDeclarations) {
				CreateSymbolForClassMemberDeclaration(declaration, classSymbol);
			}
			for (auto declaration : classNode->m_MethodDeclarations) {
				CreateSymbolForMethodDeclaration(declaration, classSymbol);
			}

			break;
		}
		case NodeKind::IntLiteral:
			break;
		case NodeKind::FloatLiteral:
			break;
		case NodeKind::DoubleLiteral:
			break;
		case NodeKind::BoolLiteral:
			break;
		case NodeKind::StringLiteral:
			break;
		case NodeKind::ArrayLiteral:
		{
			ArrayLiteral* arr = (ArrayLiteral*)node;

			// First validate all elements
			for (auto& element : arr->m_Elements)
			{
				Analyze(element, table, expectedType);
			}

			// If empty array, then the type of it is the expected type
			// otherwise probably error? or should an empty array be subtype of all arrays?
			if (arr->m_Elements.empty())
			{
				if (!expectedType.has_value())
				{
					MakeError("Could not infer type of empty array");
					return;
				}

				m_ResolvedOverloadCache[node] = { {}, expectedType.value().id };
				return;
			}

			// Ensure all elements are of the same type (identical)
			O::Type firstType = GetTypeOfExpression(arr->m_Elements[0], table);
			for (int i = 1; i < arr->m_Elements.size(); i++)
			{
				O::Type& type = GetTypeOfExpression(arr->m_Elements[i], table);

				// TODO: This should run for literal types only, reference types that are subtypes should be ok
				if (!table.types.AreTypesEquivalent(firstType, type))
				{
					MakeError("Array cannot contain elements of different types (" + firstType.name + " and " + type.name + ")");
					return;
				}
			}

			// Cache the type of the tuple
			m_ResolvedOverloadCache[node] = { {}, InsertArray(firstType, table.types).id };
			return;
		}
		default:
			break;
		}
	}

	Type& SemanticAnalyzer::GetTypeOfExpression(AST::Node* node, SymbolTypeTable& table)
	{
		using namespace Nodes;
		switch (node->m_Type)
		{
		case NodeKind::Identifier:
		{
			Identifier* identifier = (Identifier*)node;
			if (!table.symbols.Has(identifier->m_Name))
			{
				MakeErrorNotDefined(identifier->m_Name);
				return *table.types.Lookup(PrimitiveValueTypes::Void);
			}

			// TODO: support retriving type of overloaded functions. Should be infered from the context (i think)
			auto symbols = table.symbols.Lookup(identifier->m_Name);

			if (symbols[0]->m_SymbolType == SymbolType::Function || symbols[0]->m_SymbolType == SymbolType::Method)
			{
				//table.types.InsertFunction()

				return *table.types.Lookup(m_ResolvedOverloadCache[node].returnType);
			}

			return *table.types.Lookup(symbols[0]->m_DataType);
		}

			// Perform typechecking and create the variable symbol
		case NodeKind::VariableDeclaration:
			return *table.types.Lookup(PrimitiveValueTypes::Void);

		case NodeKind::BinaryExpression:
		{
			BinaryExpression* expr = (BinaryExpression*)node;
			assert(m_ResolvedOverloadCache.count(node) == 1);

			return *table.types.Lookup(m_ResolvedOverloadCache[node].returnType);
		}
		case NodeKind::UnaryExpression:
		{
			assert(m_ResolvedOverloadCache.count(node) == 1);

			return *table.types.Lookup(m_ResolvedOverloadCache[node].returnType);
		}
		case NodeKind::CallExpression:
		{
			// TODO: The function overload is onyl cached when it is called,it should be upon generaton aswell
			CallExpression* call = (CallExpression*)node;

			return *table.types.Lookup(m_ResolvedOverloadCache[node].returnType);
		}
		case NodeKind::TupleExpression:
		{
			TupleExpression* tuple = (TupleExpression*)node;

			return *table.types.Lookup(m_ResolvedOverloadCache[node].returnType);
		}
		case NodeKind::FunctionDefinition:
			break;
		case NodeKind::ExpressionFunctionDefinition:
			break;
		case NodeKind::IfStatement:
			break;
		case NodeKind::WhileStatement:
			break;
		case NodeKind::ForStatement:
			break;
		case NodeKind::LoopStatement:
			break;
		case NodeKind::Closure:
			break;
		case NodeKind::Continue:
			break;
		case NodeKind::Break:
			break;
		case NodeKind::Return:
			break;
		case NodeKind::ClassDeclaration:
			break;
		case NodeKind::IntLiteral:
			return *table.types.Lookup(PrimitiveValueTypes::Integer);
		case NodeKind::FloatLiteral:
			abort();
		case NodeKind::DoubleLiteral:
			return *table.types.Lookup(PrimitiveValueTypes::Double);
		case NodeKind::BoolLiteral:
			return *table.types.Lookup(PrimitiveValueTypes::Bool);
		case NodeKind::StringLiteral:
			return *table.types.Lookup(PrimitiveValueTypes::String);
		case NodeKind::ArrayLiteral:
			return *table.types.Lookup(m_ResolvedOverloadCache[node].returnType);

		default:
			break;
		}

		abort(); // remove later
	}

	Type& SemanticAnalyzer::ResolveTypeNode(AST::Nodes::Type* node, SymbolTypeTable& table)
	{
		using namespace Nodes;
		switch (node->m_Type)
		{
		case NodeKind::BasicType:
		{
			BasicType* basicType = (BasicType*)node;
			return *table.types.Lookup(basicType->m_TypeName);
		}
		case NodeKind::ArrayType:
		{
			ArrayType* arrType = (ArrayType*)node;

			O::Type& type = ResolveTypeNode(arrType->m_UnderlyingType, table);
			return InsertArray(type, table.types);
		}
		case NodeKind::TupleType:
		{
			TupleType* tupleType = (TupleType*)node;

			std::vector<O::Type> elementTypes;
			for (Nodes::Type* element : tupleType->m_Elements)
			{
				elementTypes.push_back(ResolveTypeNode(element, table));
			}

			return table.types.InsertTuple(elementTypes);
		}
		case NodeKind::FunctionType:
		{
			FunctionType* functionType = (FunctionType*)node;

			std::vector<O::Type> parameterTypes;
			for (Nodes::Type* parameter : functionType->m_Parameters)
			{
				parameterTypes.push_back(ResolveTypeNode(parameter, table));
			}
			O::Type returnType = ResolveTypeNode(functionType->m_ReturnType, table);

			return table.types.InsertFunction(parameterTypes, returnType);
		}
		}

		abort();
		O::Type t;
		return t;
	}

	void SemanticAnalyzer::MakeError(const std::string& message,  CompileTimeError::Severity severity)
	{
		MakeError_Void(message, Token(), severity);
	}

	void SemanticAnalyzer::MakeErrorAlreadyDefined(const std::string symbolName, SymbolType symbolType)
	{
		std::string message = SymbolTypeToString(symbolType) + " " + symbolName + " is already defined";
		MakeError(message);
	}

	void SemanticAnalyzer::MakeErrorCallableAlreadyDefined(const std::string symbolName, SymbolType symbolType, CallableSignature signature, TypeTable& types)
	{
		std::string signatureStr = "(";
		for (int i = 0; i < signature.parameterTypes.size() - 1; i++)
		{
			signatureStr += types.Lookup(signature.parameterTypes[i])->name + ", ";
		}

		signatureStr += types.Lookup(signature.parameterTypes.back())->name + " => " + types.Lookup(signature.returnType)->name + ")";

		std::string message = SymbolTypeToString(symbolType) + " " + symbolName + " " + signatureStr + " is already defined";
		MakeError(message);
	}

	void SemanticAnalyzer::MakeErrorNotDefined(const std::string symbolName)
	{
		std::string message = "Symbol " + symbolName + " has not been defined";
		MakeError(message);
	}

	void SemanticAnalyzer::MakeErrorInvalidCallableName(const std::string symbolName, SymbolType symbolType)
	{
		std::string message = "Invalid name for callable " + symbolName + " (" + SymbolTypeToString(symbolType) + ")";
		MakeError(message);
	}

	void SemanticAnalyzer::MakeErrorInvalidDeclaredType(const std::string symbolName, const std::string declaredType, const std::string expectedType)
	{
		std::string message = "Invalid type " + declaredType + " declared for callable " + symbolName + ", expected type " + expectedType;
		MakeError(message);
	}

	void SemanticAnalyzer::MakeErrorTypeInvalidProperty(O::Type& type, const std::string property)
	{
		MakeError("Type " + type.name + " doesn't have a property '" + property + "'");
	}

	void SemanticAnalyzer::MakeErrorTypeCallableNotDefined(const std::string typeName, DetailedCallableSignature signature)
	{
		//MakeError("Function " + "is not defined on type " + type.name);

	}

	void SemanticAnalyzer::MakeErrorTypeCallableNotDefined(const std::string typeName, const std::string name)
	{
		MakeError("Function " + name + " is not defined on type " + typeName);
	}


	SemanticAnalyzer::~SemanticAnalyzer()
	{
	}
}
