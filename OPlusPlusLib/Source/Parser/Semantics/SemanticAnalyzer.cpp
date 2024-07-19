#include "SemanticAnalyzer.h"

#include <set>

#include "../Lexer.h"
#include "../../Utils.hpp"

namespace O
{
	using namespace AST;


	// int = double
	// int = 5.0 <=> (int, double => int)

/*		arr: int[];
		let a = arr[0] // [] <=> (int[], int => int)
		let b: int& = arr[0] // [] <=> (int[], int => int&)
		let c = Animal.age; . <=> (Animal, "age" => int)
		let c: int = Animal.age + 5; . <=> (Animal, "age" => int)
		= (int&, int => int&)

		T& implicit subtyp till T
		*/



	void OperatorDefinitions::Create(SymbolTypeTable& table)
	{
		TypeId integerRef = table.types.LookupReference((TypeId)PrimitiveValueTypes::Integer)->id;
		TypeId doubleRef = table.types.LookupReference((TypeId)PrimitiveValueTypes::Double)->id;
		TypeId boolRef = table.types.LookupReference((TypeId)PrimitiveValueTypes::Bool)->id;
		TypeId stringRef = table.types.LookupReference((TypeId)PrimitiveValueTypes::String)->id;

		// Integer
		CreateArithmeticValueExprOperators((TypeId)PrimitiveValueTypes::Integer);
		CreateArithmeticPlaceExprOperators(integerRef);

		CreateBooleanValueExprOperators((TypeId)PrimitiveValueTypes::Integer);

		CreateCompoundAssignmentOperators((TypeId)PrimitiveValueTypes::Integer, integerRef);
		CreateDirectAssignmentOperator((TypeId)PrimitiveValueTypes::Integer, integerRef);

		// Double
		CreateArithmeticValueExprOperators((TypeId)PrimitiveValueTypes::Double);
		CreateArithmeticPlaceExprOperators((TypeId)PrimitiveValueTypes::Double);

		CreateBooleanValueExprOperators((TypeId)PrimitiveValueTypes::Double);

		CreateCompoundAssignmentOperators((TypeId)PrimitiveValueTypes::Double, doubleRef);
		CreateDirectAssignmentOperator((TypeId)PrimitiveValueTypes::Double, doubleRef);

		// Bool
		// TODO: Add boolean algebra arithmetic operators?
		CreateBooleanValueExprOperators((TypeId)PrimitiveValueTypes::Bool);

		CreateDirectAssignmentOperator((TypeId)PrimitiveValueTypes::Integer, boolRef);

		// Strings

		// Strings only have concatination and comparison operators defined on them
		// TODO: Prettier function for defining this
		TypeId stringType = (TypeId)PrimitiveValueTypes::String;
		m_BuiltInOperatorDefinitions[Operators::Addition].push_back({ { stringType, stringType }, stringType });
		m_BuiltInOperatorDefinitions[Operators::CompoundAssignmentSum].push_back(
			{ { stringRef, stringType }, (TypeId)PrimitiveValueTypes::Void });

		CreateBooleanValueExprOperators((TypeId)PrimitiveValueTypes::String);
		CreateDirectAssignmentOperator((TypeId)PrimitiveValueTypes::String, stringRef);


	}

	void OperatorDefinitions::CreateArithmeticValueExprOperators(TypeId type)
	{
		using namespace Operators;

		CallableSignature binarySignature = { { type, type }, type };
		CallableSignature unarySignature = { { type }, type };

		std::vector<Operators::Name> unaryOperators = {
			UnaryPlus,
			UnaryMinus,
		};

		std::vector<Operators::Name> binaryOperators = {
			Multiplication, Division, Remainder,
			Addition, Subtraction,
		};

		for (Operators::Name op : unaryOperators)
		{
			m_OperatorSignatures[op].push_back(unarySignature);
		}
		for (Operators::Name op : binaryOperators)
		{
			m_OperatorSignatures[op].push_back(binarySignature);
		}
	}
	void OperatorDefinitions::CreateBooleanValueExprOperators(TypeId type)
	{
		using namespace Operators;

		CallableSignature binarySignature = { { type, type }, PrimitiveValueTypes::Bool };
		CallableSignature unarySignature = { { type }, PrimitiveValueTypes::Bool };

		std::vector<Operators::Name> unaryOperators = {
			LogicalNot
		};
		std::vector<Operators::Name> binaryOperators = {
			LessThan, LessThanOrEqual,
			GreaterThan, GreaterThanOrEqual,
			Equality, NotEqual,
		};

		for (Operators::Name op : unaryOperators)
		{
			m_OperatorSignatures[op].push_back(unarySignature);
		}
		for (Operators::Name op : binaryOperators)
		{
			m_OperatorSignatures[op].push_back(binarySignature);
		}
	}

	void OperatorDefinitions::CreateArithmeticPlaceExprOperators(TypeId type)
	{
		using namespace Operators;

		CallableSignature binarySignature = { { type, type }, type };
		CallableSignature unarySignature = { { type }, type };

		std::vector<Operators::Name> unaryOperators = {
			PostfixIncrement,
			PostfixDecrement,
			PostfixIncrement,
			PostfixDecrement,
		};

		for (Operators::Name op : unaryOperators)
		{
			m_OperatorSignatures[op].push_back(unarySignature);
		}
	}

	void OperatorDefinitions::CreateDirectAssignmentOperator(TypeId type, TypeId referenceType)
	{
		using namespace Operators;

		m_OperatorSignatures[DirectAssignment].push_back(
			{ { referenceType, type }, (TypeId)PrimitiveValueTypes::Void });
	}

	void OperatorDefinitions::CreateCompoundAssignmentOperators(TypeId type, TypeId referenceType)
	{
		using namespace Operators;

		m_OperatorSignatures[CompoundAssignmentSum].push_back(
			{ { referenceType, type }, (TypeId)PrimitiveValueTypes::Void });

		m_OperatorSignatures[CompoundAssignmentDifference].push_back(
			{ { referenceType, type }, (TypeId)PrimitiveValueTypes::Void });

		m_OperatorSignatures[CompoundAssignmentProduct].push_back(
			{ { referenceType, type }, (TypeId)PrimitiveValueTypes::Void });

		m_OperatorSignatures[CompoundAssignmentQuotinent].push_back(
			{ { referenceType, type }, (TypeId)PrimitiveValueTypes::Void });
	}

	SemanticAnalyzer::SemanticAnalyzer(AST::Node* program, Parser& parser)
	{
		m_Program = program;
		m_NodesToTokesMappings = parser.m_NodesToTokesMappings;
		m_SourceCode = O::Lexer::Lexer::ReconstructSourcecode(parser.GetTokensIncludingComments());
	}

	void SemanticAnalyzer::AnalyzeProgram()
	{
		//m_GlobalSymbolTypeTable = CreateSymbolTypeTable(SymbolTableType::Global, nullptr);

		// Only used because Analyze takes a reference.
		// dummyTable is never used 
		SymbolTypeTable dummyTable = { {}, { TypeTableType::Local, nullptr } };
		Analyze(m_Program, dummyTable);
	}

	const std::string& SemanticAnalyzer::GetSourceCode()
	{
		return m_SourceCode;
	}

	TokenRange SemanticAnalyzer::GetTokenRangeForNode(AST::Node* node)
	{
		assert(m_NodesToTokesMappings.count(node) == 1);

		return m_NodesToTokesMappings[node];
	}

	void SemanticAnalyzer::AnalyzeScope(Nodes::Scope* scope)
	{
		for (auto& line : scope->m_Lines)
		{
			Analyze(line, GetSymbolTypeTableForNode(scope));
			//if (HasError())
				//return;
		}
	}

	void SemanticAnalyzer::GetReturnTypes(AST::Node* node, std::vector<const Type*>& returnTypes, SymbolTypeTable& table, std::optional<const Type*> expectedType)
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

				GetReturnTypes(line, returnTypes, GetSymbolTypeTableForNode(scope), expectedType);

			}

			return;
		}
		case NodeKind::WhileStatement:
		{
			ConditionalStatement* statement = (ConditionalStatement*)node;
			BlockStatement* body = statement->m_Body;
			if (!body) return;

			GetReturnTypes(body, returnTypes, GetSymbolTypeTableForNode(body), expectedType);

			return;
		}
		case NodeKind::ForStatement:
		{
			ForStatement* statement = (ForStatement*)node;
			BlockStatement* body = statement->m_Body;
			if (!body) return;

			GetReturnTypes(body, returnTypes, GetSymbolTypeTableForNode(body), expectedType);

			return;
		}
		case NodeKind::IfStatement:
		{
			// Main body
			IfStatement* statement = (IfStatement*)node;
			BlockStatement* body = statement->m_Body;
			if (!body) return;

			GetReturnTypes(body, returnTypes, GetSymbolTypeTableForNode(body), expectedType);

			// Else body
			BlockStatement* elseBody = statement->m_ElseArm;
			if (!elseBody) return;

			GetReturnTypes(elseBody, returnTypes, GetSymbolTypeTableForNode(elseBody), expectedType);

			return;
		}

		case NodeKind::LoopStatement:
			abort();
			break;
		case NodeKind::Closure:
			break;

		case NodeKind::FunctionDefinition:
		{
			FunctionDefinitionStatement* function = (FunctionDefinitionStatement*)node;

			if (function->IsExpression())
			{
				returnTypes.push_back(GetTypeOfExpression(function->m_Body, table));
			}
			else
			{
				GetReturnTypes(function->m_Body, returnTypes, table, expectedType);
			}

			break;
		}

		case NodeKind::Return:
		{
			ReturnStatement* returnStatement = (ReturnStatement*)node;

			// TODO: wont work for typedefs when they are added, need to use the check equivalence function
			// when checking if its already in the list

			// return; which implies returning void if no return values
			if (!returnStatement->m_ReturnValue)
			{
				const O::Type* voidType = table.types.Lookup(PrimitiveValueTypes::Void);

				// Only add type if it isn't already added
				if (std::find(returnTypes.begin(), returnTypes.end(), voidType) == returnTypes.end())
					returnTypes.push_back(voidType);

				return;
			}

			const O::Type* type = GetTypeOfExpression(returnStatement->m_ReturnValue, table);
			if (HasError())
				return;

			// Only add type if it isn't already added
			if (std::find(returnTypes.begin(), returnTypes.end(), type) == returnTypes.end())
				returnTypes.push_back(type);

			return;
		}

		default:
			break;
		}
	}

	SymbolTypeTable* SemanticAnalyzer::CreateSymbolTypeTable(SymbolTableType tableKind, SymbolTypeTable* upwardTable)
	{
		if (tableKind == SymbolTableType::Global)
		{
			auto table = new SymbolTypeTable{ SymbolTable(SymbolTableType::Global, nullptr), TypeTable(TypeTableType::Global, nullptr) };

			// Generate operators and reference types for primitives
			m_OperatorDefinitions.Create(*table);

			return table;
		}

		assert(upwardTable != nullptr);

		return new SymbolTypeTable{ SymbolTable(SymbolTableType::Local, &upwardTable->symbols), TypeTable(TypeTableType::Local, &upwardTable->types) };
	}

	SymbolTypeTable& SemanticAnalyzer::CreateTableForNode(AST::Node* node, SymbolTypeTable* upwardTable)
	{
		if (node->m_Type == NodeKind::Program)
			m_GlobalSymbolTypeTable = SetTableForNode(node, CreateSymbolTypeTable(SymbolTableType::Global, nullptr));
		else
			SetTableForNode(node, CreateSymbolTypeTable(SymbolTableType::Local, upwardTable));

		return GetSymbolTypeTableForNode(node);
	}

	std::optional<CallableSignature> SemanticAnalyzer::ResolveOperatorOverload(Nodes::OperatorExpression* expression, SymbolTypeTable& table, std::vector<const Type*> arguments, std::optional<const Type*> expectedType)
	{
		DetailedCallableSignature calleSignature = {
			arguments,
			{},
			expression->m_Operator.ToString(),
			CallableSymbolType::Operator
		};

		// If one of the operands is incomplete, then assume the overload exists and create a new type
		// TODO: eg: x + A, but + is not defined on A, then we can throw an error instead of a new 
		if (Or(arguments, [](const Type* t) { return t->kind == TypeKind::Incomplete; }))
		{
			// Dont think both operands can be incomplete at the same time
			assert(!And(arguments, [](const Type* t) { return t->kind == TypeKind::Incomplete; }));

			// The new unknown type
			const Type* x_i = table.types.InsertIncomplete();

			std::vector<TypeId> reducedArgs;
			for (auto& arg : arguments)
			{
				reducedArgs.push_back(arg->id);
			}

			m_ResolvedOverloadCache[expression] = { reducedArgs, x_i->id };
			return m_ResolvedOverloadCache[expression];
		}


		auto operatorOpt = ResolveOverload(table.types, m_OperatorDefinitions.m_OperatorSignatures[expression->m_Operator.m_Name], calleSignature, expectedType);
		if (!operatorOpt.has_value())
		{
			MakeError("Operator " + expression->m_Operator.m_Symbol + " not defined for types " +
				Join(arguments, " and ", [&table](const Type* t) { return t->GetName(&table.types); }), expression);

			m_ResolvedOverloadCache.erase(expression);

			return {};
		}

		m_ResolvedOverloadCache[expression] = operatorOpt.value();
		return operatorOpt;
	}


	VariableSymbol* SemanticAnalyzer::CreateSymbolForVariableDeclaration(Nodes::VariableDeclaration* node, SymbolTypeTable& table, VariableSymbolType variableKind)
	{
		// Case for typed variable declaration
		std::optional<const Type*> variableType;
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

			const Type* assignedValueType = GetTypeOfExpression(node->m_AssignedValue, table);
			if (HasError())
				return nullptr;

			// If variable has no type, then infer it from the assignment
			if (!node->m_VariableType)
				variableType = assignedValueType;

			// Makes errors if types dont match.
			if (!DoesTypesMatchThrowing(table.types, assignedValueType, variableType.value(), node))
				return nullptr;
		}
		else
		{
			// let x; x has no type annotated and no value to infer from, so error
			if (!node->m_VariableType)
			{
				MakeError("Variable '" + node->m_VariableName->ToString() + "' has no annotated or infered type", node);
				return nullptr;
			}
		}

		std::string variableName = node->m_VariableName->ToString();
		if (table.symbols.Has(variableName))
		{
			MakeErrorAlreadyDefined(variableName, SymbolType::Variable, node);
			return nullptr;
		}

		return table.symbols.InsertVariable(variableName, variableType.value()->id, variableKind);
	}

	std::vector<const Type*> SortTypeEntries(TypeTable& localTypeTable, std::vector<const Type*> types)
	{
		std::sort(types.begin(), types.end(), [&](const Type* t1, const Type* t2) {
			return localTypeTable.GetHeightOfTypeRelation(t1) > localTypeTable.GetHeightOfTypeRelation(t2);
			});

		return types;
	}

	std::vector<TypeId> SemanticAnalyzer::CreateSymbolsForCallableParameters(Nodes::FunctionDefinitionStatement* node)
	{
		auto& table = GetSymbolTypeTableForNode(node);
		std::vector<TypeId> parameterTypes;
		for (Nodes::VariableDeclaration* parameter : node->m_Parameters->m_Parameters)
		{
			auto symbol = CreateSymbolForVariableDeclaration(parameter, table, VariableSymbolType::Local);
			if (HasError())
				return {};

			parameterTypes.push_back(symbol->m_DataType);
		}
		return parameterTypes;
	}

	CallableSymbol* SemanticAnalyzer::CreateCallableSymbol(AST::Nodes::FunctionDefinitionStatement* node, SymbolTypeTable& table, const std::string& callableName, CallableSymbolType callableKind, std::vector<O::TypeId> parameterTypeIds, const Type* returnType)
	{

		// 	uint16_t functionId = m_ConstantsPool.AddAndGetFunctionReferenceIndex(functionName);
		uint16_t callableId = SymbolTable::GetNextCallableId();

		CallableSymbol callable = CallableSymbol(callableName, SymbolType::Method, returnType->id, callableId, callableKind);
		callable.m_ParameterTypes = parameterTypeIds;

		const Type* functionPointerType = table.types.InsertFunction(table.types.Lookup(parameterTypeIds), returnType);

		if (callableKind == CallableSymbolType::Lambda)
		{
			m_ResolvedOverloadCache[node] = { {}, functionPointerType->id };
		}
		else
		{
			m_ResolvedOverloadCache[node] = { parameterTypeIds, returnType->id };
			m_ResolvedOverloadCache[node->m_Name] = { {}, functionPointerType->id };
		}

		return table.symbols.InsertCallable(callable);
	}

	CallableSymbol* SemanticAnalyzer::CreateAndDetermineReturnTypeForCallableDeclaration(Nodes::FunctionDefinitionStatement* node, SymbolTypeTable& table, CallableSymbolType callableType)
	{

		// -- Algorithm for resolving return type of recursive function --
		// 1. Assign the return type of f to be X of incomplete kind
		// 2. analyze f and create new types for expressions involving f (X_i)
		// 3. ...
		

		std::string functionName = !node->m_IsLambda ? node->m_Name->ToString() : "";
		auto& parametersTable = GetSymbolTypeTableForNode(node);


		std::vector<TypeId> parameterTypeIds;
		for (auto& [name, symbols] : parametersTable.symbols.GetSymbols())
		{
			Symbol* parameter = symbols[0];
			parameterTypeIds.push_back(parameter->m_DataType);
		}

		// Get the declared type if it exists
		OptType declaredReturnType = node->m_ReturnType ? ResolveTypeNode(node->m_ReturnType, table) : std::nullopt;
		//if (!declaredReturnType.has_value())
			//return {};

		// Either create a new unknown type or use the declared
		const Type* returnType = nullptr;
		if (callableType == CallableSymbolType::Constructor)
		{
			const Symbol* thisSymbol = table.symbols.Lookup("this")[0];
			const Type* classType = table.types.Lookup(thisSymbol->m_DataType);
			returnType = classType;
		}
		else
		{
			returnType = !node->m_ReturnType ? table.types.InsertIncomplete() : declaredReturnType.value();
		}

		const Type X = *returnType;
		const Type Xref = *table.types.LookupReference(X.id);


		// Create the symbol for the callable so recursion works
		CallableSymbol* symbol =
			CreateCallableSymbol(node, table, functionName, callableType, parameterTypeIds, returnType);

		Analyze(node->m_Body, parametersTable);
		if (HasError())
			return {};

		if (declaredReturnType.has_value())
		{
			IsValidReturnTypesInCallableDefinition(node, callableType, declaredReturnType, true);
			if (HasError())
				return nullptr;

			return symbol;
		}


		// Now the expressions involving f has a value
		// Replace x with the possible return values
		std::vector<const Type*> possibleReturnTypes;
		GetReturnTypes(node, possibleReturnTypes, parametersTable /* TODO: add expected type argument? */);
		if (callableType == CallableSymbolType::Constructor)
		{
			const Symbol* thisSymbol = table.symbols.Lookup("this")[0];
			const Type* classType = table.types.Lookup(thisSymbol->m_DataType);
			possibleReturnTypes.push_back(classType);
		}

		// If no return statements, then the function has to return void
		if (possibleReturnTypes.empty())
		{
			// If there is a declared type, then it has to not return void (nothing)
			if (!declaredReturnType.has_value())
			{
				if (callableType == CallableSymbolType::Normal)
				{
					// We can early return if body is empty, but there could still be recursion 
					// involving the returntype that might not be valid if x = void
					possibleReturnTypes.push_back(parametersTable.types.Lookup(PrimitiveValueTypes::Void));
				}
			}

		}


		auto errors = GetErrors();

		// Try again with each of the possibilities
		std::vector<const Type*> validPossibilities;
		for (const Type* possibility : possibleReturnTypes)
		{
			if (possibility->kind == TypeKind::Incomplete)
				continue;



			parametersTable.types.Replace(returnType, possibility);

			// Propogate the new x
			SetErrors(errors); // Reset errors

			Analyze(node->m_Body, parametersTable);

			// If an error occured, then X != this possibility
			// But.. if there is only one possible return type, then that has to be the type
			// even though the body itself might not compile
			// (except when there is a declared type)

			if (HasError() && possibleReturnTypes.size() != 1 && !declaredReturnType.has_value())
				continue;

			OptType possibleReturnType = IsValidReturnTypesInCallableDefinition(node, callableType, declaredReturnType);

			if (possibleReturnType.has_value())
			{
				// X and the possible return value have to be equal, otherwise there in a contradiction
				// because f return of type int if there are return statements returning doubles
				// 
				// edit: or atleast the returned types has to be an implicit subtype of the possible return type
				if (parametersTable.types.IsTypeImplicitSubtypeOf(possibility, possibleReturnType.value()))
					validPossibilities.push_back(possibleReturnType.value());
				//if (parametersTable.types.AreTypesEquivalent(possibleReturnType.value(), possibility))
			}
		}
		SetErrors(errors); // Reset errors

		if (validPossibilities.size() == 1)
		{
			// Replace X with the correct type and analyze again
			// This is so none of the invalid possibilities has affected the types
			//parametersTable.types.Replace(returnType, validPossibilities[0]);
			//parametersTable.types.Replace(parametersTable.types.LookupReference(returnType->id), 
				//parametersTable.types.LookupReference(validPossibilities[0]->id));


			parametersTable.types.Replace(X.id, validPossibilities[0]->id);
			parametersTable.types.Replace(Xref.id,
				parametersTable.types.LookupReference(validPossibilities[0]->id)->id);

			//m_ResolvedOverloadCache[node->m_Name].returnType = validPossibilities[0]->id;


			if (symbol)
				symbol->m_DataType = validPossibilities[0]->id;

			

			Analyze(node->m_Body, parametersTable);
			return symbol;
		}

		// Otherwise bring back the incomplete type 'X'
		parametersTable.types.Replace(returnType, &X);
		Analyze(node->m_Body, parametersTable);

		if (validPossibilities.empty())
		{
			MakeError("Could not infer a valid return type for " + functionName, node);
			return nullptr;
		}
		if (validPossibilities.size() > 1)
		{
			MakeError("Could not infer a valid return type for " + functionName + ", multiple choices !! better error !!", node);
			return nullptr;
		}
	}

	OptType SemanticAnalyzer::IsValidReturnTypesInCallableDefinition(Nodes::FunctionDefinitionStatement* node, CallableSymbolType callableType, OptType declaredReturnType, bool throwing)
	{
		std::string functionName = !node->m_IsLambda ? node->m_Name->ToString() : "<anonymous function>";

		auto& localTable = GetSymbolTypeTableForNode(node);

		std::vector<const O::Type*> returnValueTypes;
		GetReturnTypes(node, returnValueTypes, localTable, declaredReturnType);
		if (callableType == CallableSymbolType::Constructor)
		{
			const Symbol* thisSymbol = localTable.symbols.Lookup("this")[0];
			const Type* classType = localTable.types.Lookup(thisSymbol->m_DataType);
			returnValueTypes.push_back(classType);
		}

		// If no return statements but a declared return type
		if (returnValueTypes.empty() && declaredReturnType.has_value() && throwing)
		{
			if (callableType != CallableSymbolType::Constructor)
			{
				auto type = declaredReturnType.value();
				if (!localTable.types.AreTypesEquivalent(type->id, PrimitiveValueTypes::Void))
				{
					MakeErrorNoReturn(functionName, type->GetName(&localTable.types), node);
					return {};
				}
			}
		}

		// Check if all of the types are implicitly compatible with the 'most general' type of the return types
		// (or if there is one one specified)
		// This is to find a sort of 'greatest common denominator' between them that accomodates all return values
		auto sortedReturnTypes = SortTypeEntries(localTable.types, returnValueTypes);

		// No specified returnvalue, so try to infer it
		const O::Type* returnType = nullptr;
		if (!declaredReturnType.has_value())
		{
			// If no return statements and no annoted type, so the return type has to be void
			if (sortedReturnTypes.empty())
				returnType = localTable.types.Lookup(PrimitiveValueTypes::Void);
			else
				returnType = sortedReturnTypes[0]; // Otherwise it is the 'most general' type of the returns
		}
		else
		{
			returnType = declaredReturnType.value();
		}

		std::vector<const O::Type*> compatible;
		for (auto& type : sortedReturnTypes)
		{
			if (throwing)
			{
				if (!DoesTypesMatchThrowing(localTable.types, type, returnType, node))
					return {};
			}
			else
			{
				if (!DoesTypesMatch(localTable.types, type, returnType))
					return {};
			}
		}

		return returnType;
	}


	CallableSymbol* SemanticAnalyzer::CreateSymbolForFunctionDeclaration(Nodes::FunctionDefinitionStatement* node, SymbolTypeTable& table, bool isMethod)
	{
		// Initialize symbol table for the function parameters to live in
		auto& parametersTable = CreateTableForNode(node, &table);
		auto parameterTypeIds = CreateSymbolsForCallableParameters(node);

		auto callableType = CallableSymbolType::Normal;
		if (node->m_IsLambda)
			callableType = CallableSymbolType::Lambda;

		// TODO: Lambda without typed parameters should infer from context. 
		// Right now the program crashes

		CallableSymbol* symbol = CreateAndDetermineReturnTypeForCallableDeclaration(node, table, callableType);

		// TODO: Continue compiling even if body failed
		if (HasError())
			return nullptr;

		assert(symbol);

		if (!IsCallableDeclarationSymbolUnique(table, symbol, node))
			return nullptr;

		return symbol;
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
					MakeErrorInvalidCallableName(methodName, SymbolType::Class, node);
					return nullptr;
				}

				// Otherwise its a constructor
				methodType = CallableSymbolType::Constructor;
			}
		}

		// Initialize symbol table for the function parameters to live in
		auto& parametersTable = CreateTableForNode(node, &classTable);

		auto parameterTypeIds = CreateSymbolsForCallableParameters(node);

		// Because this is a method, the first argument should be 'this'
		VariableSymbol* thisSymbol = (VariableSymbol*)classTable.symbols.Lookup("this")[0];
		assert(thisSymbol);

		if (methodType == CallableSymbolType::Normal)
			parameterTypeIds.insert(parameterTypeIds.begin(), thisSymbol->m_DataType);

		// TODO: figure out if operators should have 'this' as parameter
		assert(methodType != CallableSymbolType::Operator);

		OptType declaredReturnTypeOpt;
		if (node->m_ReturnType)
			declaredReturnTypeOpt = ResolveTypeNode(node->m_ReturnType, classTable);

		if (methodType == CallableSymbolType::Constructor)
		{
			const Type* classType = classTable.types.Lookup(thisSymbol->m_DataType);

			// If a return type is specified for a constructor, it has to be the type of the class
			if (declaredReturnTypeOpt.has_value())
			{
				if (classTable.types.AreTypesEquivalent(declaredReturnTypeOpt.value(), classType))
				{
					MakeErrorInvalidDeclaredType(methodName,
						declaredReturnTypeOpt.value()->GetName(&classTable.types),
						classType->GetName(&classTable.types));
					return nullptr;
				}
			}
			else
			{
				// Otherwise infer the type should be same as the class if not specified
				declaredReturnTypeOpt = classType;
			}
		}

		CallableSymbol* method = CreateAndDetermineReturnTypeForCallableDeclaration(node, classTable, methodType);
		if (HasError())
			return nullptr;
		assert(method);

		if (!IsCallableDeclarationSymbolUnique(classTable, method, node))
			return nullptr;

		return method;
	}

	bool SemanticAnalyzer::IsCallableDeclarationSymbolUnique(SymbolTypeTable& table, CallableSymbol* declaredFunction, AST::Node* node, bool compareReturnTypes)
	{
		// This function is ran after the symbol has been created and checked against other sybmols
		// TODO: If duplicate, what happens?

		auto symbols = table.symbols.Lookup(declaredFunction->m_Name);
		for (Symbol* symbol : symbols)
		{
			CallableSymbol* otherFunction = (CallableSymbol*)symbol;

			// Dont compare to itself
			if (otherFunction->m_Id == declaredFunction->m_Id)
				continue;

			bool isIdentical = true;

			if (declaredFunction->m_ParameterTypes.size() != otherFunction->m_ParameterTypes.size())
				continue;

			// Check if the currently analyzed otherFunction return type is same as other functions returntype
			if (compareReturnTypes && !table.types.AreTypesEquivalent(declaredFunction->m_ReturnType, otherFunction->m_ReturnType))
				continue;

			// Assume they are identical and look for contradictions
			auto& parameterTypes = declaredFunction->m_ParameterTypes;
			for (int i = 0; i < parameterTypes.size(); i++)
			{
				if (!table.types.AreTypesEquivalent(parameterTypes[i],
					otherFunction->m_ParameterTypes[i]))
				{
					isIdentical = false;
					break;
				}
			}

			if (isIdentical)
			{
				MakeErrorCallableAlreadyDefined(declaredFunction->m_Name, SymbolType::Function,
					{ parameterTypes, declaredFunction->m_ReturnType }, table.types, node);
				return false;
			}
		}

		return true;
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

			if (expression->m_Operator.m_Name == Operators::Name::ScopeResolution ||
				expression->m_Operator.m_Name == Operators::Name::MemberAccess)
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

	SymbolTypeTable& SemanticAnalyzer::GetSymbolTypeTableForNode(AST::Node* node)
	{
		assert(HasTableForNode(node));
		return *m_TableForNode[node];
	}

	bool SemanticAnalyzer::HasTableForNode(AST::Node* node)
	{
		return m_TableForNode.count(node) == 1;
	}

	ResolvedMemberAccess SemanticAnalyzer::AnalyzeMemberAccess(AST::Node* node, SymbolTypeTable& table, OptType expectedType)
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

		ClassSymbol* parentSymbol = (ClassSymbol*)GetSymbolForNode(expr->m_Lhs, table);
		SymbolTypeTable* localTable = parentSymbol ? parentSymbol->m_Table : &table;

		const O::Type* lhsType = GetTypeOfExpression(expr->m_Lhs, *localTable);
		if (HasError())
			return {};



		switch (lhsType->kind)
		{
		case O::TypeKind::Incomplete:
		case O::TypeKind::Error:
			break;
		case O::TypeKind::Class:
		case TypeKind::Reference:
		{
			//ClassSymbol* classSymbol = nullptr;
			if (lhsType->kind == TypeKind::Reference)
			{
				lhsType = localTable->types.Lookup(lhsType->typeArguments[0]);
			}
			// a.b
			// TODO: How wil this work on nested classes?
			// TODO: this code will run for class variables aswell, may want to change

			// lhs

			ClassSymbol* classSymbol = !parentSymbol ? (ClassSymbol*)table.symbols.LookupClassByType(lhsType->id) : parentSymbol;

			localTable = classSymbol->m_Table;
			Analyze(expr->m_Rhs, *classSymbol->m_Table);


			//ClassSymbol* classSymbol = (ClassSymbol*)table.symbols.LookupOne(lhsType.name);

			//expr->m_Lhs

			// rhs

			// cases: 
			// .a
			// .f() 


			// TODO: A::B.C::prop; doesn't work
			// B.C::prop is interpreted as B . (C::Prop) but should be (B.C)::prop
			if (expr->m_Rhs->m_Type == NodeKind::Identifier)
			{
				Identifier* prop = (Identifier*)expr->m_Rhs;
				auto results = classSymbol->m_Table->symbols.Lookup(prop->m_Name);
				if (results.empty())
				{
					MakeError("Member '" + prop->m_Name + "' doesn't exist on class " + lhsType->GetName(&localTable->types), prop);
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
					const O::Type* referenceType = localTable->types.LookupReference(results[0]->m_DataType);
					m_ResolvedOverloadCache[node] = { {}, referenceType->id };

					m_CachedSymbolsForNodes[node] = results[0];

					SetTableForNode(expr, localTable);

					SetTableForNode(expr->m_Lhs, localTable);
					SetTableForNode(expr->m_Rhs, classSymbol->m_Table);
				}

				return { classSymbol, results };

			}

			abort();

			break;
		}
		case O::TypeKind::Function:
		case O::TypeKind::Method:
		{
			MakeErrorTypeInvalidProperty(lhsType->GetName(&localTable->types), expr->m_Rhs->ToString());
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

			MakeErrorTypeInvalidProperty(lhsType->GetName(&localTable->types), expr->m_Rhs->ToString());

			break;
		}
		case O::TypeKind::Primitive:
		case O::TypeKind::Tuple:
		case O::TypeKind::Nullable:
		{
			// TODO: Add methods to primitives
			MakeErrorTypeInvalidProperty(lhsType->GetName(&localTable->types), expr->m_Rhs->ToString());
			return {};
		}
		case O::TypeKind::Typedef:
		default:
			abort();
		}

		return {};
	}

	bool IdentifierIsTypename(Nodes::Identifier* node, TypeTable& types) { return types.HasType(node->m_Name); }

	std::optional<Symbol*> SemanticAnalyzer::AnalyzeScopeResolution(AST::Node* node, SymbolTypeTable& table, OptType expectedType)
	{
		using namespace Nodes;

		BinaryExpression* expr = (BinaryExpression*)node;
		Analyze(expr->m_Lhs, table);

		// Ex: A::B::C. A::B is resolved first, but to resolve C we need the symbol table of B

		ClassSymbol* parentSymbol = (ClassSymbol*)GetSymbolForNode(expr->m_Lhs, table);
		SymbolTypeTable* localTable = parentSymbol ? parentSymbol->m_Table : &table;

		const O::Type* lhsType = GetTypeOfExpression(expr->m_Lhs, *localTable);
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

		// Resolve Reference types to the underlying type
		if (lhsType->kind == TypeKind::Reference)
			lhsType = localTable->types.Lookup(lhsType->typeArguments[0]);

		switch (lhsType->kind)
		{
		case TypeKind::Class:
		{
			ClassSymbol* classSymbol = !parentSymbol ? (ClassSymbol*)table.symbols.LookupClassByType(lhsType->id) : parentSymbol;

			localTable = classSymbol->m_Table;
			Analyze(expr->m_Rhs, *localTable);

			// Class::Identifier
			if (expr->m_Rhs->m_Type == NodeKind::Identifier)
			{
				Identifier* prop = (Identifier*)expr->m_Rhs;

				Symbol* member = localTable->symbols.LookupOne(prop->m_Name);

				m_CachedSymbolsForNodes[node] = member;

				TypeId referenceTypeId = localTable->types.LookupReference(member->m_DataType)->id;
				m_ResolvedOverloadCache[node] = { {}, referenceTypeId };

				SetTableForNode(expr, localTable);

				SetTableForNode(expr->m_Lhs, localTable); // TODO: propably the wrong table used
				SetTableForNode(expr->m_Rhs, localTable);

				// The results could be multiple symbols if the identifier is a name for a function
				// TODO: Resolve overload based on expectedType

				return member;

			}

			//if (expr->m_Rhs->m_Type == NodeKind::BinaryExpression)

			break;
		}
		case TypeKind::Function:
		{
			MakeError("Scope Resolution can not be used on a function", node);
			return {};
		}

		default:
			abort();
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
					MakeError_OLD("Member '" + prop->m_Name + "' doesn't exist on class " + lhsType.name);
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
		abort();
		return {};
	}

	void SemanticAnalyzer::AnalyzeBinaryExpression(AST::Nodes::BinaryExpression* expression, SymbolTypeTable& table, std::optional<const Type*> expectedType)
	{
		if (expression->m_Operator.m_Name == Operators::Name::ScopeResolution)
		{
			AnalyzeScopeResolution(expression, table, expectedType);
			return;
		}
		if (expression->m_Operator.m_Name == Operators::Name::MemberAccess)
		{
			AnalyzeMemberAccess(expression, table, expectedType);
			//assert(resolved.rhs.size() == 1);
			return;
		}

		Analyze(expression->m_Lhs, table, expectedType);
		if (HasError())
			return;

		Analyze(expression->m_Rhs, table, expectedType);
		if (HasError())
			return;

		std::vector<const Type*> arguments = {
			GetTypeOfExpression(expression->m_Lhs, table),
			GetTypeOfExpression(expression->m_Rhs, table)
		};

		ResolveOperatorOverload(expression, table, arguments, expectedType);
	}

	void SemanticAnalyzer::AnalyzeUnaryExpression(AST::Nodes::UnaryExpression* expression, SymbolTypeTable& table, std::optional<const Type*> expectedType)
	{
		Analyze(expression->m_Operand, table, expectedType);
		if (HasError())
			return;

		std::vector<const Type*> arguments = {
			GetTypeOfExpression(expression->m_Operand, table)
		};

		ResolveOperatorOverload(expression, table, arguments, expectedType);
	}

	bool SemanticAnalyzer::DoesTypesMatchThrowing(TypeTable& localTypeTable, const Type* otherType, const Type* expectedType, AST::Node* node)
	{
		// 1. Case when types are the same
		if (localTypeTable.AreTypesEquivalent(otherType->id, expectedType->id))
			return true;


		// 2. typedefs

		// 3. type relations
		auto typeRelation = localTypeTable.GetFullTypeRelationTo(otherType, expectedType);
		std::string otherTypeName = otherType->GetName(&localTypeTable);
		std::string expectedTypeName = expectedType->GetName(&localTypeTable);

		if (!typeRelation.has_value())
		{
			MakeError("Incompatible types. '" + otherTypeName + "' cannot be converted to '" + expectedTypeName + "' as they have no relation", node);
			return false;
		}

		// Is valid if the otherType can be implicit converted to the supertype
		if (typeRelation.value() == TypeRelation::Implicit)
		{
			return true;
		}
		else
		{
			MakeError("Incompatible types. '" + otherTypeName + "' cannot be converted to '" + expectedTypeName + "' implicitly", node);
			return false;
		}

		abort();
		return false;
	}

	bool SemanticAnalyzer::DoesTypesMatch(TypeTable& localTypeTable, const Type* otherType, const Type* expectedType)
	{
		// 1. Case when types are the same
		if (localTypeTable.AreTypesEquivalent(otherType->id, expectedType->id))
			return true;


		// 2. typedefs

		// 3. type relations
		auto typeRelation = localTypeTable.GetFullTypeRelationTo(otherType, expectedType);
		if (!typeRelation.has_value())
			return false;

		// Is valid if the otherType can be implicit converted to the supertype
		return typeRelation.value() == TypeRelation::Implicit;
	}

	std::optional<CallableSignature> SemanticAnalyzer::ResolveOverload(TypeTable& localTypeTable, std::vector<CallableSignature> overloads, DetailedCallableSignature calle, std::optional<const O::Type*> expectedReturnType)
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
				const O::Type* parameter = localTypeTable.Lookup(signature.parameterTypes[i]);
				const O::Type* argument = calle.parameterTypes[i];

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
					const O::Type* parameter = localTypeTable.Lookup(signature.parameterTypes[i]);
					const O::Type* argument = calle.parameterTypes[i];

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

		std::string calleArgumentTypesString = Join(calle.parameterTypes, std::string(", "),
			[localTypeTable](const O::Type* t) { return t->GetName(&localTypeTable); });

		// multiple matches, but could not determine which to use
		if (!expectedReturnType.has_value())
		{
			std::string error = "Found multiple matching overloaded functions for  " + calle.name + " , but could not determine which one to use. Argument types: " + calleArgumentTypesString + "\nDid you mean:\n";

			for (auto& e : stepsToSignatures)
			{

				std::vector<const Type*> parameterTypes;
				for (TypeId id : e.signature.parameterTypes)
				{
					parameterTypes.push_back(localTypeTable.Lookup(id));
				}
				const Type* returnType = localTypeTable.Lookup(e.signature.returnType);


				std::string candidateArgumentTypesString = Join(parameterTypes, std::string(", "),
					[localTypeTable](const Type* t) { return t->GetName(&localTypeTable); });
				std::string signatureString = calle.name + " (" + calleArgumentTypesString + "): " + returnType->GetName(&localTypeTable);

				error += "\n" + signatureString;

				//e.signature.
			}

			MakeError_Void(error, Token());

			return {};
		}

		std::string calleSignatureString = "(" + calleArgumentTypesString + " => " + calle.returnType->GetName(&localTypeTable) + ")";


		// Should return type overloading only match if the return type and expected type are equivalent, or if they are compatible?
		// Compatible: Might make it confusing which overload is choosen, and cases where f: int and f: double would both be matches
		// Equivalent: More strict and may reduce unintented behavior. But issue with "let a: int = 2.0 + 2.0". 
		// double + double has no overload with int, so wont compile.
		// For now: do "compatible" variant.

		// Steps are the steps to the return type only
		std::vector<SignatureWithSteps> potentialMatchesReturnType;
		for (auto& match : closestMatches)
		{
			const O::Type* returnType = localTypeTable.Lookup(match.signature.returnType);
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
				"none of them matched with the expected return type " +
				expectedReturnType.value()->GetName(&localTypeTable), Token());

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

	SymbolTypeTable* SemanticAnalyzer::SetTableForNode(AST::Node* node, SymbolTypeTable* table)
	{
		// If replacing the table with another one
		if (m_TableForNode.count(node) != 0)
		{
			std::cout << "REPLACE TABLE\n";
			delete m_TableForNode[node];
		}

		m_TableForNode[node] = table;
		return table;
	}

	const O::Type* SemanticAnalyzer::InsertArray(const O::Type* underlyingType, TypeTable& localTypeTable)
	{
		bool typeExisted = false;
		const O::Type* type = localTypeTable.InsertArray(underlyingType, typeExisted);

		if (!typeExisted)
		{
			m_OperatorDefinitions.m_OperatorSignatures[Operators::Subscript]
				.push_back({ { type->id, (TypeId)PrimitiveValueTypes::Integer }, underlyingType->id });
		}

		return type;
	}

	//O::Type& SemanticAnalyzer::InsertTuple(std::vector<O::Type> underlyingTypes, TypeTable& localTypeTable)
	//{
	//	// TODO: tuples of different types cannot have their accessed type known at compiletime since the index might be a variable

	//}

	// Create symbol tables for each scope
	void SemanticAnalyzer::Analyze(AST::Node* node, SymbolTypeTable& table, std::optional<const Type*> expectedType)
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
			SymbolTypeTable& localTable = CreateTableForNode(node, &table);

			for (auto& line : scope->m_Lines)
			{
				Analyze(line, localTable, expectedType);
			}

			break;
		}
		case NodeKind::BasicType:
			break;
		case NodeKind::Identifier:
		{
			Identifier* identifier = (Identifier*)node;

			if (!table.symbols.Has(identifier->m_Name))
				return MakeErrorNotDefined(identifier->m_Name, node);

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
			AnalyzeBinaryExpression((BinaryExpression*)node, table, expectedType);
			break;
		}
		case NodeKind::UnaryExpression:
		{
			AnalyzeUnaryExpression((UnaryExpression*)node, table, expectedType);
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
					return MakeErrorNotDefined(callee, node);
			}
			else
			{
				const O::Type* calledOnType = table.types.Lookup(calleeSymbol->m_DataType);

				if (matchingFunctions.empty())
					return MakeErrorTypeCallableNotDefined(calledOnType->GetName(&table.types), "!!TODO!!");

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
				// If trying to call a function pointer
				if (symbol->m_SymbolType == SymbolType::Variable)
				{
					const O::Type* functionPointerType = table.types.Lookup(symbol->m_DataType);
					assert(functionPointerType->kind == O::TypeKind::Function);

					auto parameterTypes = SliceVector(functionPointerType->typeArguments, 0, (int)functionPointerType->typeArguments.size() - 1);
					auto returnType = functionPointerType->typeArguments[functionPointerType->typeArguments.size() - 1];

					matchingCallableSignatures.push_back({ parameterTypes, returnType });
				}
				else
				{
					// If function has name of a class, then the class constructor should be used
					// Look it up in the class table
					CallableSymbol* callable = (CallableSymbol*)symbol;
					matchingCallableSignatures.push_back({ callable->m_ParameterTypes, callable->m_DataType });
				}
			}

			DetailedCallableSignature calleSignature = {
				{},
				{},
				callee,
				CallableSymbolType::Normal,
			};

			// Add the class instance the object is access on (in the case of method invokation)
			if (isCallingMethod)
				calleSignature.parameterTypes.push_back(table.types.Lookup(calleeSymbol->m_DataType));

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
					const O::Type* calledOnType = table.types.Lookup(calleeSymbol->m_DataType);
					return MakeError("No matching function '" + callee + "' found on " + calledOnType->GetName(&table.types), call);
				}

				if (!matchingCallableSignatures.empty())
				{
					std::string argumentTypesString = Join(calleSignature.parameterTypes, ", ", [&table](const O::Type* t) { return t->GetName(&table.types); });

					return MakeError("No matching function '" + callee + "' found for argument types (" + argumentTypesString + ")", call);
				}
				else
				{
					return MakeErrorNotDefined(callee, node);
				}

				return;
			}

			m_ResolvedOverloadCache[node] = matchingCallableOpt.value();

			if (isCallingMethod)
			{
				const O::Type* returnType = table.types.Lookup(matchingCallableOpt.value().returnType);

				m_ResolvedOverloadCache[call->m_Callee] = { {}, table.types.InsertFunction(calleSignature.parameterTypes, returnType)->id };
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

			std::vector<const O::Type*> elementTypes;
			std::vector<TypeId> elementTypeIds;
			for (AST::Node* element : tuple->m_Elements)
			{
				const O::Type* type = GetTypeOfExpression(element, table);
				if (HasError()) return;

				elementTypes.push_back(type);
				elementTypeIds.push_back(type->id);
			}

			// Cache the type of the tuple
			m_ResolvedOverloadCache[node] = { elementTypeIds, table.types.InsertTuple(elementTypes)->id };

			break;
		}
		case NodeKind::FunctionDefinition:
		{
			FunctionDefinitionStatement* functionNode = (FunctionDefinitionStatement*)node;

			CallableSymbol* function = CreateSymbolForFunctionDeclaration((FunctionDefinitionStatement*)node, table);
			return;
		}
		case NodeKind::ExpressionFunctionDefinition:
			abort();
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
			abort();

			break;
		case NodeKind::Closure:
			abort();

			break;
		case NodeKind::Continue:
			abort();

			break;
		case NodeKind::Break:
			abort();
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
				return MakeErrorAlreadyDefined(name, SymbolType::Class, classNode);

			const O::Type* classType = table.types.Insert(name, TypeKind::Class);

			classNode->m_ClassSymbol = table.symbols.InsertClass(name, classType->id, &table.symbols, &table.types);
			auto& classSymbol = *classNode->m_ClassSymbol;

			// Add the 'this' symbol 
			// TODO: wont work for nested classes (Solution is to remove it maybe in some good way)
			classSymbol.m_Table->symbols.InsertVariable("this", classType->id, VariableSymbolType::Local);

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
					MakeError("Could not infer type of empty array", arr);
					return;
				}

				m_ResolvedOverloadCache[node] = { {}, expectedType.value()->id };
				return;
			}

			// Ensure all elements are of the same type (identical)
			const O::Type* firstType = GetTypeOfExpression(arr->m_Elements[0], table);
			for (int i = 1; i < arr->m_Elements.size(); i++)
			{
				const O::Type* type = GetTypeOfExpression(arr->m_Elements[i], table);

				// TODO: This should run for literal types only, reference types that are subtypes should be ok
				if (!table.types.AreTypesEquivalent(firstType, type))
				{
					MakeError("Array cannot contain elements of different types (" +
						firstType->GetName(&table.types) + " and " + type->GetName(&table.types) + ")", arr);
					return;
				}
			}

			// Cache the type of the tuple
			m_ResolvedOverloadCache[node] = { {}, InsertArray(firstType, table.types)->id };
			return;
		}
		default:
			break;
		}
	}

	const Type* SemanticAnalyzer::GetTypeOfExpression(AST::Node* node, SymbolTypeTable& table)
	{
		using namespace Nodes;
		switch (node->m_Type)
		{
		case NodeKind::Identifier:
		{
			Identifier* identifier = (Identifier*)node;
			if (!table.symbols.Has(identifier->m_Name))
			{
				MakeErrorNotDefined(identifier->m_Name, node);
				return nullptr;
			}

			// TODO: support retriving type of overloaded functions. Should be infered from the context (i think)
			auto symbols = table.symbols.Lookup(identifier->m_Name);

			if (symbols[0]->m_SymbolType == SymbolType::Function || symbols[0]->m_SymbolType == SymbolType::Method)
			{
				//table.types.InsertFunction()

				return table.types.Lookup(m_ResolvedOverloadCache[node].returnType);
			}

			return table.types.Lookup(symbols[0]->m_DataType);
		}

		case NodeKind::BinaryExpression:
		{
			BinaryExpression* expr = (BinaryExpression*)node;
			if (m_ResolvedOverloadCache.count(node) == 0) return nullptr;

			return table.types.Lookup(m_ResolvedOverloadCache[node].returnType);
		}
		case NodeKind::UnaryExpression:
		{
			if (m_ResolvedOverloadCache.count(node) == 0) return nullptr;

			return table.types.Lookup(m_ResolvedOverloadCache[node].returnType);
		}
		case NodeKind::CallExpression:
		{
			// TODO: The function overload is only cached when it is called,it should be upon generaton aswell
			CallExpression* call = (CallExpression*)node;

			if (m_ResolvedOverloadCache.count(node) == 0) return nullptr;

			return table.types.Lookup(m_ResolvedOverloadCache[node].returnType);
		}
		case NodeKind::TupleExpression:
		{
			TupleExpression* tuple = (TupleExpression*)node;

			if (m_ResolvedOverloadCache.count(node) == 0) return nullptr;

			return table.types.Lookup(m_ResolvedOverloadCache[node].returnType);
		}
		case NodeKind::FunctionDefinition:
		{
			FunctionDefinitionStatement* function = (FunctionDefinitionStatement*)node;

			if (function->m_IsLambda)
			{
				return table.types.Lookup(m_ResolvedOverloadCache[node].returnType);
			}

			MakeError_Void("A function definition does not have a type", GetTokenRangeForNode(function));

			break;
		}
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
			return table.types.Lookup(PrimitiveValueTypes::Integer);
		case NodeKind::FloatLiteral:
			abort();
		case NodeKind::DoubleLiteral:
			return table.types.Lookup(PrimitiveValueTypes::Double);
		case NodeKind::BoolLiteral:
			return table.types.Lookup(PrimitiveValueTypes::Bool);
		case NodeKind::StringLiteral:
			return table.types.Lookup(PrimitiveValueTypes::String);
		case NodeKind::ArrayLiteral:
		{
			if (m_ResolvedOverloadCache.count(node) == 0) return nullptr;

			return table.types.Lookup(m_ResolvedOverloadCache[node].returnType);
		}

		default:
			break;
		}

		abort(); // remove later
	}

	// TODO: Add error checking after everything
	OptType SemanticAnalyzer::ResolveTypeNode(AST::Nodes::Type* node, SymbolTypeTable& table)
	{
		using namespace Nodes;
		switch (node->m_Type)
		{
		case NodeKind::BasicType:
		{
			BasicType* basicType = (BasicType*)node;
			if (!table.types.HasCompleteType(basicType->m_TypeName))
			{
				MakeError_Void("Type " + basicType->m_TypeName + " has not been declared in this scope", Token());
				return {};
			}

			return table.types.Lookup(basicType->m_TypeName);
		}
		case NodeKind::ArrayType:
		{
			ArrayType* arrType = (ArrayType*)node;

			auto optType = ResolveTypeNode(arrType->m_UnderlyingType, table);
			if (HasError())
				return {};

			return InsertArray(optType.value(), table.types);
		}
		case NodeKind::TupleType:
		{
			TupleType* tupleType = (TupleType*)node;

			std::vector<const O::Type*> elementTypes;
			for (Nodes::Type* element : tupleType->m_Elements)
			{
				auto typeOpt = ResolveTypeNode(element, table);
				if (HasError())
					return {};

				elementTypes.push_back(typeOpt.value());
			}

			return table.types.InsertTuple(elementTypes);
		}
		case NodeKind::FunctionType:
		{
			FunctionType* functionType = (FunctionType*)node;

			std::vector<const O::Type*> parameterTypes;
			for (Nodes::Type* parameter : functionType->m_Parameters)
			{
				auto typeOpt = ResolveTypeNode(parameter, table);
				if (HasError())
					return {};

				parameterTypes.push_back(typeOpt.value());
			}

			OptType returnType = ResolveTypeNode(functionType->m_ReturnType, table);
			if (HasError())
				return {};

			return table.types.InsertFunction(parameterTypes, returnType.value());
		}
		}

		abort();
		return {};
	}

	void SemanticAnalyzer::MakeError(const std::string& message, AST::Node* node, CompileTimeError::Severity severity)
	{
		MakeError_Void(message, GetTokenRangeForNode(node), severity);
	}

	void SemanticAnalyzer::MakeError_OLD_(const std::string& message, CompileTimeError::Severity severity)
	{
		MakeError_Void(message, Token(), severity);
	}

	void SemanticAnalyzer::MakeErrorAlreadyDefined(const std::string symbolName, SymbolType symbolType, AST::Node* node)
	{
		std::string message = SymbolTypeToString(symbolType) + " " + symbolName + " is already defined";
		MakeError(message, node);
	}

	void SemanticAnalyzer::MakeErrorCallableAlreadyDefined(const std::string symbolName, SymbolType symbolType, CallableSignature signature, TypeTable& types, AST::Node* node)
	{
		std::string signatureStr = "(";
		for (int i = 0; i < signature.parameterTypes.size() - 1; i++)
		{
			signatureStr += types.Lookup(signature.parameterTypes[i])->GetName(&types) + ", ";
		}

		signatureStr += types.Lookup(signature.parameterTypes.back())->GetName(&types) + " => " + types.Lookup(signature.returnType)->GetName(&types) + ")";

		std::string message = SymbolTypeToString(symbolType) + " " + symbolName + " " + signatureStr + " is already defined";
		MakeError(message, node);
	}

	void SemanticAnalyzer::MakeErrorNotDefined(const std::string symbolName, O::AST::Node* node)
	{
		std::string message = "Symbol " + symbolName + " has not been defined";

		MakeError_Void(message, GetTokenRangeForNode(node));

	}

	void SemanticAnalyzer::MakeErrorInvalidCallableName(const std::string symbolName, SymbolType symbolType, AST::Node* node)
	{
		std::string message = "Invalid name for callable " + symbolName + " (" + SymbolTypeToString(symbolType) + ")";
		MakeError(message, node);
	}

	void SemanticAnalyzer::MakeErrorNoReturn(const std::string& functionName, const std::string& returnType, AST::Node* node)
	{
		std::string message = functionName + " with return type " + returnType + " has to return a value";
		MakeError_Void(message, GetTokenRangeForNode(node));

	}

	void SemanticAnalyzer::MakeErrorInvalidDeclaredType(const std::string symbolName, const std::string declaredType, const std::string expectedType)
	{
		std::string message = "Invalid type " + declaredType + " declared for callable " + symbolName + ", expected type " + expectedType;
		MakeError_OLD_(message);
	}

	void SemanticAnalyzer::MakeErrorTypeInvalidProperty(const std::string typeName, const std::string property)
	{
		MakeError_OLD_("Type " + typeName + " doesn't have a property '" + property + "'");
	}

	void SemanticAnalyzer::MakeErrorTypeCallableNotDefined(const std::string typeName, DetailedCallableSignature signature)
	{
		//MakeError("Function " + "is not defined on type " + type.name);

	}

	void SemanticAnalyzer::MakeErrorTypeCallableNotDefined(const std::string typeName, const std::string name)
	{
		MakeError_OLD_("Function " + name + " is not defined on type " + typeName);
	}


	SemanticAnalyzer::~SemanticAnalyzer()
	{
	}
}
