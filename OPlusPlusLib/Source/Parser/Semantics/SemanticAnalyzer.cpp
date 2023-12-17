#include "SemanticAnalyzer.h"

namespace O
{
	using namespace AST;

	OperatorDefinitions::OperatorDefinitions()
	{
		GeneratePrimitiveOperators((TypeId)PrimitiveValueTypes::Integer);
		GenerateAssignmentOperators((TypeId)PrimitiveValueTypes::Integer);

		GeneratePrimitiveOperators((TypeId)PrimitiveValueTypes::Double);
		GenerateAssignmentOperators((TypeId)PrimitiveValueTypes::Double);
	}

	void OperatorDefinitions::GeneratePrimitiveOperators(TypeId type)
	{
		CallableSignature binarySignature = { { type, type }, type };
		CallableSignature unarySignature = { { type }, type };

		using namespace Operators;
		std::vector<Operators::Name> primitiveUnaryOperators = {
			PostfixIncrement, PostfixDecrement,
			PrefixIncrement, PrefixDecrement,

			UnaryPlus, UnaryMinus,
			BitwiseNot
		};
		std::vector<Operators::Name> primitiveBinaryOperators = {
			Multiplication, Division, Remainder,
			Addition, Subtraction,

			DirectAssignment // TODO: move when constant datatypes are implemented
		};

		std::vector<Operators::Name> primitiveUnaryBooleanOperators = {
			LogicalNot
		};
		std::vector<Operators::Name> primitiveBinaryBooleanOperators = {
			LessThan, LessThanOrEqual,
			GreaterThan, GreaterThanOrEqual,
			Equality, NotEqual,
		};

		// Doubles and integers has all operators defined on them
		for (Operators::Name op : primitiveUnaryOperators)
		{
			m_OperatorSignatures[op].push_back({ { type }, type });
		}
		for (Operators::Name op : primitiveUnaryBooleanOperators)
		{
			m_OperatorSignatures[op].push_back({ { type }, PrimitiveValueTypes::Bool });
		}

		for (Operators::Name op : primitiveBinaryOperators)
		{
			m_OperatorSignatures[op].push_back({ { type, type }, type });
		}
		for (Operators::Name op : primitiveBinaryBooleanOperators)
		{
			m_OperatorSignatures[op].push_back({ { type, type }, PrimitiveValueTypes::Bool });
		}
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
		SymbolTable dummySymbolTable;
		TypeTable dummyTypeTable;

		Analyze(m_Program, dummySymbolTable, dummyTypeTable);
	}

	void SemanticAnalyzer::AnalyzeScope(Nodes::Scope* scope)
	{
		for (auto& line : scope->m_Lines)
		{
			Analyze(line, scope->m_LocalSymbolTable, scope->m_LocalTypeTable);
			//if (HasError())
				//return;
		}
	}

	void SemanticAnalyzer::GetReturnTypes(AST::Node* node, std::vector<Type>& returnTypes, SymbolTable& localSymbolTable, TypeTable& localTypeTable)
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

				GetReturnTypes(line, returnTypes, scope->m_LocalSymbolTable, scope->m_LocalTypeTable);

			}

			return;
		}
		case NodeKind::WhileStatement:
		{
			ConditionalStatement* statement = (ConditionalStatement*)node;
			BlockStatement* body = statement->m_Body;
			if (!body) return;

			GetReturnTypes(body, returnTypes, body->m_LocalSymbolTable, body->m_LocalTypeTable);

			return;
		}
		case NodeKind::ForStatement:
		{
			ForStatement* statement = (ForStatement*)node;
			BlockStatement* body = statement->m_Body;
			if (!body) return;

			GetReturnTypes(body, returnTypes, body->m_LocalSymbolTable, body->m_LocalTypeTable);

			return;
		}
		case NodeKind::IfStatement:
		{
			// Main body
			IfStatement* statement = (IfStatement*)node;
			BlockStatement* body = statement->m_Body;
			if (!body) return;

			GetReturnTypes(body, returnTypes, body->m_LocalSymbolTable, body->m_LocalTypeTable);

			// Else body
			BlockStatement* elseBody = statement->m_ElseArm;
			if (!elseBody) return;
			
			GetReturnTypes(elseBody, returnTypes, elseBody->m_LocalSymbolTable, elseBody->m_LocalTypeTable);

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
				returnTypes.push_back(*localTypeTable.Lookup(PrimitiveValueTypes::Void));
				return;
			}

			O::Type type = GetTypeOfNode(returnStatement->m_ReturnValue, localSymbolTable, localTypeTable);
			if (HasError())
				return;

			returnTypes.push_back(type);
			return;
		}
			
		default:
			break;
		}
	}

	void SemanticAnalyzer::CreateTablesForScope(Nodes::Scope* node, SymbolTable& localSymbolTable, TypeTable& localTypeTable)
	{
		if (node->m_Type == NodeKind::Program)
		{
			node->m_LocalSymbolTable = SymbolTable(SymbolTableType::Global, nullptr);
			node->m_LocalTypeTable = TypeTable(TypeTableType::Global, nullptr);

			m_GlobalSymbolTable = &node->m_LocalSymbolTable;
			m_GlobalTypeTable = &node->m_LocalTypeTable;
		} else
		{
			node->m_LocalSymbolTable = SymbolTable(SymbolTableType::Local, &localSymbolTable);
			node->m_LocalTypeTable = TypeTable(TypeTableType::Local, &localTypeTable);
		}
	}

	VariableSymbol* SemanticAnalyzer::CreateSymbolForVariableDeclaration(Nodes::VariableDeclaration* node, SymbolTable& localSymbolTable, TypeTable& localTypeTable, VariableSymbolType variableKind)
	{
		Type variableType;
		if (node->m_VariableType)
		{
			Analyze(node->m_VariableType, localSymbolTable, localTypeTable);
			if (HasError())
				return nullptr;

			variableType = GetTypeOfNode(node->m_VariableType, localSymbolTable, localTypeTable);
			if (HasError())
				return nullptr;
		}
		
		if (HasError())
			return nullptr;

		if (node->m_AssignedValue)
		{
			Analyze(node->m_AssignedValue, localSymbolTable, localTypeTable);
			if (HasError())
				return nullptr;

			Type assignedValueType = GetTypeOfNode(node->m_AssignedValue, localSymbolTable, localTypeTable);
			if (HasError())
				return nullptr;

			// If variable has no type, then infer it from the assignment
			if (!node->m_VariableType)
				variableType = assignedValueType;

			// Makes errors if types dont match.
			if (!DoesTypesMatchThrowing(localTypeTable, assignedValueType, variableType))
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
		if (localSymbolTable.Has(variableName))
		{
			MakeErrorAlreadyDefined(variableName, SymbolType::Variable);
			return nullptr;
		}

		return localSymbolTable.InsertVariable(variableName, variableType.id, variableKind);
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
			auto symbol = CreateSymbolForVariableDeclaration(parameter, node->m_ParametersSymbolTable, node->m_ParametersTypeTable, VariableSymbolType::Local);
			if (HasError())
				return {};

			parameterTypes.push_back(symbol->m_DataType);
		}
		return parameterTypes;
	}

	CallableSymbol* SemanticAnalyzer::CreateSymbolForFunctionDeclaration(Nodes::FunctionDefinitionStatement* node, SymbolTable& localSymbolTable, TypeTable& localTypeTable, bool isMethod)
	{
		std::string functionName = node->m_Name->ToString();

		auto symbols = localSymbolTable.Lookup(functionName);
		bool isOverload = !symbols.empty();

		// Initialize symbol table for the function parameters to live in
		// They are not created in the body symbol table, as expressive functions has no scope node to attach the table to
		node->m_ParametersSymbolTable = SymbolTable(SymbolTableType::Local, &localSymbolTable);
		node->m_ParametersTypeTable = TypeTable(TypeTableType::Local, &localTypeTable);

		auto parameterTypes = CreateSymbolsForCallableDefinition(node);

		std::optional<Type> declaredReturnType = {};
		if (node->m_ReturnType)
			declaredReturnType = GetTypeOfNode(node->m_ReturnType, localSymbolTable, localTypeTable);

		auto returnTypeOpt = AnalyzeCallableDefinition(node, node->m_ParametersSymbolTable, node->m_ParametersTypeTable, declaredReturnType);
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
				O::Type& parameterType = *node->m_ParametersTypeTable.Lookup(parameterTypes[i]);
				O::Type& otherParameterType = *node->m_ParametersTypeTable.Lookup(function->m_ParameterTypes[i]);

				if (parameterType.id != otherParameterType.id)
					isIdentical = false;
			}

			if (isIdentical)
			{
				MakeErrorAlreadyDefined(functionName, SymbolType::Function);
				return nullptr;
			}
		}

		// 	uint16_t functionId = m_ConstantsPool.AddAndGetFunctionReferenceIndex(functionName);
		uint16_t callableId = 0; // TODO: Implement properly

		CallableSymbol callable = CallableSymbol(functionName, SymbolType::Function, returnType.id, callableId, CallableSymbolType::Normal);
		callable.m_ParameterTypes = parameterTypes;

		return localSymbolTable.InsertCallable(callable);
	}

	VariableSymbol* SemanticAnalyzer::CreateSymbolForClassMemberDeclaration(Nodes::VariableDeclaration* node, ClassSymbol& classSymbol)
	{
		return CreateSymbolForVariableDeclaration(node, *classSymbol.m_Symbols, *classSymbol.m_Types, VariableSymbolType::Member);
	}

	CallableSymbol* SemanticAnalyzer::CreateSymbolForMethodDeclaration(Nodes::FunctionDefinitionStatement* node, ClassSymbol& classSymbol)
	{
		SymbolTable& classSymbolTable = *classSymbol.m_Symbols;
		TypeTable& classTypeTable = *classSymbol.m_Types;

		std::string methodName = node->m_Name->ToString();
		CallableSymbolType methodType = CallableSymbolType::Normal;

		// TODO: Add function overloading
		auto symbols = classSymbolTable.Lookup(methodName);
		if (!symbols.empty())
		{
			// Special case for constructors as the class symbol has already been declared
			if (symbols[0]->m_SymbolType != SymbolType::Class)
				return nullptr;

			ClassSymbol* symbol = (ClassSymbol*)symbols[0];
			if (symbol->m_Name != methodName)
			{
				MakeErrorInvalidCallableName(methodName, SymbolType::Class);
				return nullptr;
			}

			// Otherwise its a constructor
			methodType = CallableSymbolType::Constructor;
		}

		// Initialize symbol table for the function parameters to live in
		// They are not created in the body symbol table, as expressive functions has no scope node to attach the table to
		node->m_ParametersSymbolTable = SymbolTable(SymbolTableType::Local, &classSymbolTable);
		node->m_ParametersTypeTable = TypeTable(TypeTableType::Local, &classTypeTable);

		auto parameterTypes = CreateSymbolsForCallableDefinition(node);

		// Because this is a method, the first argument should be 'this'
		VariableSymbol* thisSymbol = (VariableSymbol*)classSymbolTable.Lookup("this")[0];
		assert(thisSymbol);

		parameterTypes.insert(parameterTypes.begin(), thisSymbol->m_DataType);

		std::optional<Type> declaredReturnTypeOpt;
		if (node->m_ReturnType)
			declaredReturnTypeOpt = GetTypeOfNode(node->m_ReturnType, classSymbolTable, classTypeTable);

		if (methodType == CallableSymbolType::Constructor)
		{
			Type classType = *classTypeTable.Lookup(thisSymbol->m_DataType);

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

		auto returnTypeOpt = AnalyzeCallableDefinition(node, node->m_ParametersSymbolTable, classTypeTable, declaredReturnTypeOpt);
		if (HasError())
			return nullptr;
		assert(returnTypeOpt.has_value());
		Type returnType = returnTypeOpt.value();

		// If the function 

		// 	uint16_t functionId = m_ConstantsPool.AddAndGetFunctionReferenceIndex(functionName);
		uint16_t callableId = 0; // TODO: Implement properly

		CallableSymbol callable = CallableSymbol(methodName, SymbolType::Method, returnType.id, callableId, methodType);
		callable.m_ParameterTypes = parameterTypes;

		return classSymbolTable.InsertCallable(callable);
	}

	std::optional<Type> SemanticAnalyzer::AnalyzeCallableDefinition(Nodes::FunctionDefinitionStatement* node, SymbolTable& localSymbolTable, TypeTable& localTypeTable, std::optional<Type> declaredReturnType)
	{
		using namespace Nodes;
		std::string functionName = node->m_Name->ToString();

		// Analayze body to look for errors
		Analyze(node->m_Body, localSymbolTable, localTypeTable);

		// Analyze the body and look for the return statements
		assert(node->m_Body->m_Type == NodeKind::BlockStatement);

		Scope* body = (Scope*)node->m_Body;

		std::vector<O::Type> returnValueTypes;
		GetReturnTypes(body, returnValueTypes, localSymbolTable, localTypeTable);

		assert(localTypeTable.GetHeightOfTypeRelation(*localTypeTable.Lookup(PrimitiveValueTypes::Double)) == 2);
		assert(localTypeTable.GetHeightOfTypeRelation(*localTypeTable.Lookup(PrimitiveValueTypes::Bool)) == 0);
		assert(localTypeTable.GetHeightOfTypeRelation(*localTypeTable.Lookup(PrimitiveValueTypes::Integer)) == 1);
		assert(localTypeTable.GetHeightOfTypeRelation(*localTypeTable.Lookup(PrimitiveValueTypes::Void)) == 0);

		// Check if all of the types are implicitly compatible with the 'most general' type of the return types
		// (or if there is one one specified)
		// This is to find a sort of 'greatest common denominator' between them that accomodates all return values

		//Type returnType;

		auto sortedReturnTypes = SortTypeEntries(localTypeTable, returnValueTypes);
		
		// No specified returnvalue, so try to infer it
		O::Type returnType;
		if (!declaredReturnType.has_value())
		{
			// If no return statements and no annoted type, so the return type has to be void
			if (sortedReturnTypes.empty())
				returnType = *localTypeTable.Lookup(PrimitiveValueTypes::Void);
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

			if (DoesTypesMatchThrowing(localTypeTable, type, returnType))
				compatible.push_back(type);
		}

		if (HasError())
		{
			MakeError("Could not compile function '" + functionName + "'");
			return {};
		}

		return returnType;
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

	std::optional<CallableSignature> SemanticAnalyzer::ResolveOverload(TypeTable& localTypeTable, std::vector<CallableSignature> overloads, std::vector<Type> arguments)
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
			if (signature.parameterTypes.size() != arguments.size())
				continue;

			// Check if each argument type is compatible with corresponding parameter type
			for (int i = 0; i < signature.parameterTypes.size(); i++)
			{
				O::Type& parameter = *localTypeTable.Lookup(signature.parameterTypes[i]);
				O::Type& argument = arguments[i];

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
					O::Type& argument = arguments[i];

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

		return stepsToSignatures[0].signature;
	}

	// Create symbol tables for each scope
	void SemanticAnalyzer::Analyze(AST::Node* node, SymbolTable& localSymbolTable, TypeTable& localTypeTable)
	{
		using namespace Nodes;
		//if (HasError())
			//return;

		switch (node->m_Type)
		{
		case NodeKind::EmptyStatement:
			return;
		case NodeKind::Program:
		case NodeKind::BlockStatement:
		{
			CreateTablesForScope((Scope*)node, localSymbolTable, localTypeTable);
			AnalyzeScope((Scope*)node);

			break;
		}
		case NodeKind::BasicType:
			break;
		case NodeKind::Identifier:
		{
			Identifier* identifier = (Identifier*)node;
			
			if (!localSymbolTable.Has(identifier->m_Name))
				return MakeErrorNotDefined(identifier->m_Name);

			break;
		}

		// For normal variable declarations in scopes (not class members)
		case NodeKind::VariableDeclaration:
		{
			CreateSymbolForVariableDeclaration((VariableDeclaration*)node, localSymbolTable, localTypeTable, VariableSymbolType::Local);
			return;
		}

		case NodeKind::BinaryExpression:
		{
			BinaryExpression* expression = (BinaryExpression*)node;
			Analyze(expression->m_Lhs, localSymbolTable, localTypeTable);
			if (HasError())
				return;

			Analyze(expression->m_Rhs, localSymbolTable, localTypeTable);
			if (HasError())
				return;

			O::Type& lhs = GetTypeOfNode(expression->m_Lhs, localSymbolTable, localTypeTable);
			O::Type& rhs = GetTypeOfNode(expression->m_Rhs, localSymbolTable, localTypeTable);

			auto operatorOpt = ResolveOverload(localTypeTable, m_OperatorDefinitions.m_OperatorSignatures[expression->m_Operator.m_Name], { lhs, rhs });
			if (!operatorOpt.has_value())
			{
				MakeError("Operator " + expression->m_Operator.m_Symbol + " not defined for " + lhs.name + " and " + rhs.name);
				return;
			}

			m_ResolvedOverloadCache[node] = operatorOpt.value();

			break;
		}
		case NodeKind::UnaryExpression:
		{
			UnaryExpression* expression = (UnaryExpression*)node;
			Analyze(expression->m_Operand, localSymbolTable, localTypeTable);
			if (HasError())
				return;

			O::Type& operand = GetTypeOfNode(expression->m_Operand, localSymbolTable, localTypeTable);

			auto operatorOpt = ResolveOverload(localTypeTable, m_OperatorDefinitions.m_OperatorSignatures[expression->m_Operator.m_Name], { operand });
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

			// Check function exists
			Analyze(call->m_Callee, localSymbolTable, localTypeTable);

			auto matchingFunctions = localSymbolTable.Lookup(call->m_Callee->ToString());

			//ResolveOverload

			std::vector<CallableSignature> matchingCallableSignatures;
			for (Symbol* symbol : matchingFunctions)
			{
				CallableSymbol* callable = (CallableSymbol*)symbol;

				matchingCallableSignatures.push_back({ callable->m_ParameterTypes, callable->m_DataType });
			}

			std::vector<O::Type> argumentTypes;
			for (O::AST::Node* argument : call->m_Arguments->m_Elements) 
			{
				argumentTypes.push_back(GetTypeOfNode(argument, localSymbolTable, localTypeTable));
			}

			auto opt = ResolveOverload(localTypeTable, matchingCallableSignatures, argumentTypes);

			CallableSymbol* matchingCallable = (CallableSymbol*)matchingFunctions[0];

			// First validate
			for (int i = 0; i < call->m_Arguments->m_Elements.size(); i++)
			{
				auto& argument = call->m_Arguments->m_Elements[i];

				Analyze(argument, localSymbolTable, localTypeTable);

				O::Type& argumentType = GetTypeOfNode(argument, localSymbolTable, localTypeTable);
				O::Type& parameterType = *localTypeTable.Lookup(matchingCallable->m_ParameterTypes[i]);

				if (!DoesTypesMatchThrowing(localTypeTable, argumentType, parameterType))
					return;
			}

			// Then typecheck arguments


			auto& a = m_OperatorDefinitions;
			int b = 5;

			break;
		}
		case NodeKind::TupleExpression:
		{
			TupleExpression* tuple = (TupleExpression*)node;
			for (auto& element : tuple->m_Elements)
			{
				Analyze(element, localSymbolTable, localTypeTable);
			}

			std::vector<O::Type> elementTypes;
			std::vector<TypeId> elementTypeIds;
			for (AST::Node* element : tuple->m_Elements)
			{
				auto& type = GetTypeOfNode(element, localSymbolTable, localTypeTable);
				if (HasError()) return;

				elementTypes.push_back(type);
				elementTypeIds.push_back(type.id);
			}

			// Cache the type of the tuple
			m_ResolvedOverloadCache[node] = { elementTypeIds, localTypeTable.InsertTuple(elementTypes).id };

			break;
		}
		case NodeKind::FunctionDefinition:
		{
			FunctionDefinitionStatement* functionNode = (FunctionDefinitionStatement*)node;

			CreateSymbolForFunctionDeclaration((FunctionDefinitionStatement*)node, localSymbolTable, localTypeTable);

			// Analyze the body
			// TODO: Ensure a return exists
			// TODO: Typecheck returned type and function return type
			Analyze(functionNode->m_Body, functionNode->m_ParametersSymbolTable, localTypeTable);

			return;
		}

		case NodeKind::ExpressionFunctionDefinition:
			break;
		case NodeKind::IfStatement:
		{
			IfStatement* ifStatement = (IfStatement*)node;
			Analyze(ifStatement->m_Condition, localSymbolTable, localTypeTable);
			Analyze(ifStatement->m_Body, localSymbolTable, localTypeTable);
			if (ifStatement->m_ElseArm)
				Analyze(ifStatement->m_ElseArm, localSymbolTable, localTypeTable);
			
			break;
		}
		case NodeKind::WhileStatement:
		{
			WhileStatement* whileStatement = (WhileStatement*)node;
			Analyze(whileStatement->m_Condition, localSymbolTable, localTypeTable);
			Analyze(whileStatement->m_Body, localSymbolTable, localTypeTable);

			break;
		}
		case NodeKind::ForStatement:
		{
			ForStatement* forStatement = (ForStatement*)node;

			if (forStatement->m_Initialization)
				Analyze(forStatement->m_Initialization, localSymbolTable, localTypeTable);
			if (forStatement->m_Condition)
				Analyze(forStatement->m_Condition, localSymbolTable, localTypeTable);
			if (forStatement->m_Advancement)
				Analyze(forStatement->m_Advancement, localSymbolTable, localTypeTable);

			Analyze(forStatement->m_Body, localSymbolTable, localTypeTable);

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

			Analyze(returnNode->m_ReturnValue, localSymbolTable, localTypeTable);

			break;
		}
		case NodeKind::ClassDeclaration:
		{
			ClassDeclarationStatement* classNode = (ClassDeclarationStatement*)node;

			std::string name = classNode->m_Name->ToString();

			if (localTypeTable.HasCompleteType(name))
				return MakeErrorAlreadyDefined(name, SymbolType::Class);

			O::Type classType = localTypeTable.Insert(name, TypeEntryType::Class);

			classNode->m_ClassSymbol = localSymbolTable.InsertClass(name, classType.id, &localSymbolTable, &localTypeTable);
			auto& classSymbol = *classNode->m_ClassSymbol;
			
			// Add the 'this' symbol 
			// TODO: wont work for nested classes (Solution is to remove it maybe in some good way)
			classSymbol.m_Symbols->InsertVariable("this", classType.id, VariableSymbolType::Local);
			
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
		default:
			break;
		}
	}

	Type& SemanticAnalyzer::GetTypeOfNode(AST::Node* node, SymbolTable& localSymbolTable, TypeTable& localTypeTable)
	{
		using namespace Nodes;
		switch (node->m_Type)
		{
		case NodeKind::EmptyStatement:
			return *localTypeTable.Lookup(PrimitiveValueTypes::Void);

		case NodeKind::Program:
		case NodeKind::BlockStatement:
		{
			// Case where the goal is to look for return statements
		}

		case NodeKind::BasicType:
		{
			BasicType* basicType = (BasicType*)node;
			return *localTypeTable.Lookup(basicType->m_TypeName);
		}
		case NodeKind::ArrayType:
		{
			ArrayType* arrType = (ArrayType*)node;

			O::Type& type = GetTypeOfNode(arrType->m_UnderlyingType, localSymbolTable, localTypeTable);
			return localTypeTable.InsertArray(type);
		}
		case NodeKind::TupleType:
		{
			TupleType* tupleType = (TupleType*)node;

			std::vector<O::Type> elementTypes;
			for (Nodes::Type* element : tupleType->m_Elements)
			{
				elementTypes.push_back(GetTypeOfNode(element, localSymbolTable, localTypeTable));
			}

			return *localTypeTable.Lookup(m_ResolvedOverloadCache[node].returnType);
		}
		case NodeKind::FunctionType:
		{
			FunctionType* functionType = (FunctionType*)node;

			std::vector<O::Type> parameterTypes;
			for (Nodes::Type* parameter : functionType->m_Parameters)
			{
				parameterTypes.push_back(GetTypeOfNode(parameter, localSymbolTable, localTypeTable));
			}
			O::Type returnType = GetTypeOfNode(functionType->m_ReturnType, localSymbolTable, localTypeTable);

			return localTypeTable.InsertFunction(parameterTypes, returnType);
		}

		case NodeKind::Identifier:
		{
			Identifier* identifier = (Identifier*)node;
			if (!localSymbolTable.Has(identifier->m_Name))
			{
				MakeErrorNotDefined(identifier->m_Name);
				return *localTypeTable.Lookup(PrimitiveValueTypes::Void);
			}

			// TODO: support retriving type of overloaded functions. Should be infered from the context (i think)
			auto symbols = localSymbolTable.Lookup(identifier->m_Name);
			assert(symbols[0]->m_SymbolType != SymbolType::Function && symbols[0]->m_SymbolType != SymbolType::Method);

			return *localTypeTable.Lookup(symbols[0]->m_DataType);
		}

			// Perform typechecking and create the variable symbol
		case NodeKind::VariableDeclaration:
			return *localTypeTable.Lookup(PrimitiveValueTypes::Void);

		case NodeKind::BinaryExpression:
		{
			return *localTypeTable.Lookup(m_ResolvedOverloadCache[node].returnType);
		}
		case NodeKind::UnaryExpression:
		{
			assert(m_ResolvedOverloadCache.count(node) == 1);
			
			return *localTypeTable.Lookup(m_ResolvedOverloadCache[node].returnType);
		}
		case NodeKind::CallExpression:
		{
			// TODO: The function overload is onyl cached when it is called,it should be upon generaton aswell
			CallExpression* call = (CallExpression*)node;

			return *localTypeTable.Lookup(m_ResolvedOverloadCache[node].returnType);

			// TODO: Determine function overloads?
			//return GetTypeOfNode(call->m_Callee, localSymbolTable, localTypeTable);
		}
		case NodeKind::TupleExpression:
		{
			TupleExpression* tuple = (TupleExpression*)node;

			return *localTypeTable.Lookup(m_ResolvedOverloadCache[node].returnType);
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
			return *localTypeTable.Lookup(PrimitiveValueTypes::Integer);
		case NodeKind::FloatLiteral:
			abort();
		case NodeKind::DoubleLiteral:
			return *localTypeTable.Lookup(PrimitiveValueTypes::Double);
		case NodeKind::BoolLiteral:
			return *localTypeTable.Lookup(PrimitiveValueTypes::Bool);
		case NodeKind::StringLiteral:
			return *localTypeTable.Lookup(PrimitiveValueTypes::String);

		default:
			break;
		}
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

	SemanticAnalyzer::~SemanticAnalyzer()
	{
	}
}
