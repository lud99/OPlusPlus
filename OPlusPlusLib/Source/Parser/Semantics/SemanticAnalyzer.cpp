#include "SemanticAnalyzer.h"

namespace O
{
	using namespace AST;

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

	void SemanticAnalyzer::AnalyzeScope(Scope* scope)
	{
		for (auto& line : scope->m_Lines)
		{
			Analyze(line, scope->m_LocalSymbolTable, scope->m_LocalTypeTable);
			//if (HasError())
				//return;
		}
	}

	void SemanticAnalyzer::CreateTablesForScope(AST::Scope* node, SymbolTable& localSymbolTable, TypeTable& localTypeTable)
	{
		if (node->m_Type == NodeType::Program)
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

	VariableSymbol* SemanticAnalyzer::CreateSymbolForVariableDeclaration(VariableDeclaration* node, SymbolTable& localSymbolTable, TypeTable& localTypeTable)
	{
		TypeTableEntry variableType;
		if (node->m_VariableType)
			variableType = GetTypeOfNode(node->m_VariableType, localSymbolTable, localTypeTable);
		
		if (HasError())
			return nullptr;

		if (node->m_AssignedValue)
		{
			TypeTableEntry assignedValueType = GetTypeOfNode(node->m_AssignedValue, localSymbolTable, localTypeTable);
			if (HasError())
				return nullptr;

			// If variable has no type, then infer it from the assignment
			if (!node->m_VariableType)
				variableType = assignedValueType;

			// Makes errors if types dont match.
			if (!DoesTypesMatch(localTypeTable, variableType, assignedValueType))
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

		// TODO: Add class members
		return localSymbolTable.InsertVariable(variableName, variableType.id, VariableSymbolType::Local);
	}

	CallableSymbol* SemanticAnalyzer::CreateSymbolForFunctionDeclaration(AST::FunctionDefinitionStatement* node, SymbolTable& localSymbolTable, TypeTable& localTypeTable)
	{
		TypeTableEntry& returnType = GetTypeOfNode(node->m_ReturnType, localSymbolTable, localTypeTable);
		std::string functionName = node->m_Name->ToString();

		// TODO: Add function overloading
		auto symbols = localSymbolTable.Lookup(functionName);
		if (!symbols.empty())
		{
			MakeErrorAlreadyDefined(functionName, SymbolType::Function);
			return nullptr;
		}

		// Initialize symbol table for the function parameters to live in
		// They are not created in the body symbol table, as expressive functions has no scope node to attach the table to
		node->m_ParametersSymbolTable = SymbolTable(SymbolTableType::Local, &localSymbolTable);
		node->m_ParametersTypeTable = TypeTable(TypeTableType::Local, &localTypeTable);

		// Create symbols for parameters
		std::vector<ValueType> parameterTypes;
		for (VariableDeclaration* parameter : node->m_Parameters->m_Parameters)
		{
			auto symbol = CreateSymbolForVariableDeclaration(parameter, node->m_ParametersSymbolTable, node->m_ParametersTypeTable);
			if (HasError())
				return nullptr; 

			parameterTypes.push_back(symbol->m_DataType);
		}

		// TODO: Add class methods

		// 	uint16_t functionId = m_ConstantsPool.AddAndGetFunctionReferenceIndex(functionName);
		uint16_t callableId = 0; // TODO: Implement properly

		CallableSymbol callable = CallableSymbol(functionName, SymbolType::Function, returnType.id, callableId, CallableSymbolType::Normal);
		callable.m_ParameterTypes = parameterTypes;

		return localSymbolTable.InsertCallable(callable);
	}

	VariableSymbol* SemanticAnalyzer::CreateSymbolForClassMemberDeclaration(AST::VariableDeclaration* node, ClassSymbol& classSymbol)
	{
		return CreateSymbolForVariableDeclaration(node, *classSymbol.m_Symbols, *classSymbol.m_Types);
	}

	CallableSymbol* SemanticAnalyzer::CreateSymbolForMethodDeclaration(AST::FunctionDefinitionStatement* node, ClassSymbol& classSymbol)
	{
		return CreateSymbolForFunctionDeclaration(node, *classSymbol.m_Symbols, *classSymbol.m_Types);
	}

	bool SemanticAnalyzer::DoesTypesMatch(TypeTable& localTypeTable, TypeTableEntry& expectedType, TypeTableEntry& otherType)
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

	// Create symbol tables for each scope
	void SemanticAnalyzer::Analyze(AST::Node* node, SymbolTable& localSymbolTable, TypeTable& localTypeTable)
	{
		//if (HasError())
			//return;

		switch (node->m_Type)
		{
		case O::AST::NodeType::EmptyStatement:
			return;
		case O::AST::NodeType::Program:
		case O::AST::NodeType::BlockStatement:
		{
			CreateTablesForScope((Scope*)node, localSymbolTable, localTypeTable);
			AnalyzeScope((Scope*)node);

			break;
		}
		case O::AST::NodeType::BasicType:
			break;
		case O::AST::NodeType::Identifier:
		{
			Identifier* identifier = (Identifier*)node;
			
			if (!localSymbolTable.Has(identifier->m_Name))
				return MakeErrorNotDefined(identifier->m_Name);

			break;
		}

		// For normal variable declarations in scopes (not class members)
		// TODO: Add typechecking
		case O::AST::NodeType::VariableDeclaration:
		{
			CreateSymbolForVariableDeclaration((VariableDeclaration*)node, localSymbolTable, localTypeTable);
			return;
		}

		case O::AST::NodeType::BinaryExpression:
		{
			BinaryExpression* expression = (BinaryExpression*)node;
			Analyze(expression->m_Lhs, localSymbolTable, localTypeTable);
			if (HasError())
				return;
			Analyze(expression->m_Rhs, localSymbolTable, localTypeTable);

			break;
		}
		case O::AST::NodeType::UnaryExpression:
		{
			UnaryExpression* expression = (UnaryExpression*)node;
			Analyze(expression->m_Operand, localSymbolTable, localTypeTable);

			break;
		}
		case O::AST::NodeType::CallExpression:
		{
			CallExpression* call = (CallExpression*)node;

			Analyze(call->m_Callee, localSymbolTable, localTypeTable);

			for (auto& argument : call->m_Arguments->m_Elements)
			{
				Analyze(argument, localSymbolTable, localTypeTable);
			}

			break;
		}
		case O::AST::NodeType::TupleExpression:
		{
			TupleExpression* tuple = (TupleExpression*)node;
			for (auto& element : tuple->m_Elements)
			{
				Analyze(element, localSymbolTable, localTypeTable);
			}

			break;
		}
		case O::AST::NodeType::FunctionDefinition:
		{
			FunctionDefinitionStatement* functionNode = (FunctionDefinitionStatement*)node;

			CreateSymbolForFunctionDeclaration((FunctionDefinitionStatement*)node, localSymbolTable, localTypeTable);

			// Analyze the body
			// TODO: Ensure a return exists
			// TODO: Typecheck returned type and function return type
			Analyze(functionNode->m_Body, functionNode->m_ParametersSymbolTable, localTypeTable);

			return;
		}

		case O::AST::NodeType::ExpressionFunctionDefinition:
			break;
		case O::AST::NodeType::IfStatement:
		{
			IfStatement* ifStatement = (IfStatement*)node;
			Analyze(ifStatement->m_Condition, localSymbolTable, localTypeTable);
			Analyze(ifStatement->m_Body, localSymbolTable, localTypeTable);
			if (ifStatement->m_ElseArm)
				Analyze(ifStatement->m_ElseArm, localSymbolTable, localTypeTable);
			
			break;
		}
		case O::AST::NodeType::WhileStatement:
		{
			WhileStatement* whileStatement = (WhileStatement*)node;
			Analyze(whileStatement->m_Condition, localSymbolTable, localTypeTable);
			Analyze(whileStatement->m_Body, localSymbolTable, localTypeTable);

			break;
		}
		case O::AST::NodeType::ForStatement:
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
		case O::AST::NodeType::LoopStatement:
			break;
		case O::AST::NodeType::Closure:
			break;
		case O::AST::NodeType::Continue:
			break;
		case O::AST::NodeType::Break:
			break;
		case O::AST::NodeType::Return:
			break;
		case O::AST::NodeType::ClassDeclaration:
		{
			ClassDeclarationStatement* classNode = (ClassDeclarationStatement*)node;

			std::string name = classNode->m_Name->ToString();

			if (localTypeTable.HasCompleteType(name))
				return MakeErrorAlreadyDefined(name, SymbolType::Class);

			TypeTableEntry& classType = *localTypeTable.Lookup(name);

			auto& classSymbol = *localSymbolTable.InsertClass(name, classType.id, &localSymbolTable, &localTypeTable);
			
			for (auto declaration : classNode->m_MemberDeclarations) {
				CreateSymbolForClassMemberDeclaration(declaration, classSymbol);
			}
			for (auto declaration : classNode->m_MethodDeclarations) {
				CreateSymbolForMethodDeclaration(declaration, classSymbol);
			}

			break;
		}
		case O::AST::NodeType::IntLiteral:
			break;
		case O::AST::NodeType::FloatLiteral:
			break;
		case O::AST::NodeType::DoubleLiteral:
			break;
		case O::AST::NodeType::BoolLiteral:
			break;
		case O::AST::NodeType::StringLiteral:
			break;
		default:
			break;
		}
	}

	TypeTableEntry& SemanticAnalyzer::GetTypeOfNode(AST::Node* node, SymbolTable& localSymbolTable, TypeTable& localTypeTable)
	{
		switch (node->m_Type)
		{
		case O::AST::NodeType::EmptyStatement:
			return *localTypeTable.Lookup(PrimitiveValueTypes::Void);

		case O::AST::NodeType::Program:
		case O::AST::NodeType::BlockStatement:
			return *localTypeTable.Lookup(PrimitiveValueTypes::Void);

		case O::AST::NodeType::BasicType:
		{
			BasicType* basicType = (BasicType*)node;
			return *localTypeTable.Lookup(basicType->m_TypeName);
		}
		case O::AST::NodeType::ArrayType:
		{
			ArrayType* arrType = (ArrayType*)node;

			TypeTableEntry& type = GetTypeOfNode(arrType->m_UnderlyingType, localSymbolTable, localTypeTable);
			return localTypeTable.InsertArray(type);
		}
		case O::AST::NodeType::TupleType:
		{
			TupleType* tupleType = (TupleType*)node;

			std::vector<TypeTableEntry> elementTypes;
			for (AST::Type* element : tupleType->m_Elements)
			{
				elementTypes.push_back(GetTypeOfNode(element, localSymbolTable, localTypeTable));
			}

			return localTypeTable.InsertTuple(elementTypes);
		}
		case O::AST::NodeType::FunctionType:
		{
			FunctionType* functionType = (FunctionType*)node;

			std::vector<TypeTableEntry> parameterTypes;
			for (AST::Type* parameter : functionType->m_Parameters)
			{
				parameterTypes.push_back(GetTypeOfNode(parameter, localSymbolTable, localTypeTable));
			}
			TypeTableEntry returnType = GetTypeOfNode(functionType->m_ReturnType, localSymbolTable, localTypeTable);

			return localTypeTable.InsertFunction(parameterTypes, returnType);
		}

		case O::AST::NodeType::Identifier:
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
		case O::AST::NodeType::VariableDeclaration:
			return *localTypeTable.Lookup(PrimitiveValueTypes::Void);

		case O::AST::NodeType::BinaryExpression:
		{
			BinaryExpression* expression = (BinaryExpression*)node;
			auto& lhs = GetTypeOfNode(expression->m_Lhs, localSymbolTable, localTypeTable);
			auto& rhs = GetTypeOfNode(expression->m_Rhs, localSymbolTable, localTypeTable);

			// TODO: Add operators and their compatible types
			return lhs;
		}
		case O::AST::NodeType::UnaryExpression:
		{
			UnaryExpression* expression = (UnaryExpression*)node;
			return GetTypeOfNode(expression->m_Operand, localSymbolTable, localTypeTable);
		}
		case O::AST::NodeType::CallExpression:
		{
			CallExpression* call = (CallExpression*)node;

			// TODO: Determine function overloads?
			return GetTypeOfNode(call->m_Callee, localSymbolTable, localTypeTable);
		}
		case O::AST::NodeType::TupleExpression:
		{
			TupleExpression* tuple = (TupleExpression*)node;

			std::vector<TypeTableEntry> elementTypes;
			for (AST::Node* element : tuple->m_Elements)
			{
				auto& type = GetTypeOfNode(element, localSymbolTable, localTypeTable);
				if (HasError())
					return *localTypeTable.Lookup(PrimitiveValueTypes::Void);
				
				elementTypes.push_back(type);
			}

			return localTypeTable.InsertTuple(elementTypes);
		}
		case O::AST::NodeType::FunctionDefinition:
			break;
		case O::AST::NodeType::ExpressionFunctionDefinition:
			break;
		case O::AST::NodeType::IfStatement:
			break;
		case O::AST::NodeType::WhileStatement:
			break;
		case O::AST::NodeType::ForStatement:
			break;
		case O::AST::NodeType::LoopStatement:
			break;
		case O::AST::NodeType::Closure:
			break;
		case O::AST::NodeType::Continue:
			break;
		case O::AST::NodeType::Break:
			break;
		case O::AST::NodeType::Return:
			break;
		case O::AST::NodeType::ClassDeclaration:
			break;
		case O::AST::NodeType::IntLiteral:
			return *localTypeTable.Lookup(PrimitiveValueTypes::Integer);
		case O::AST::NodeType::FloatLiteral:
			abort();
		case O::AST::NodeType::DoubleLiteral:
			return *localTypeTable.Lookup(PrimitiveValueTypes::Double);
		case O::AST::NodeType::BoolLiteral:
			return *localTypeTable.Lookup(PrimitiveValueTypes::Bool);
		case O::AST::NodeType::StringLiteral:
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

	SemanticAnalyzer::~SemanticAnalyzer()
	{
	}
}
