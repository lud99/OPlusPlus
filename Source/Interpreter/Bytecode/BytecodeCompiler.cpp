#pragma once

#include <fstream>
#include <iostream>

#include "BytecodeCompiler.h"
#include "../Functions.h"

#include "../../Utils.hpp"

#include "Heap.h"

namespace Ö::Bytecode::Compiler {

Opcodes ASTComparisonTypeToOpcode(ASTTypes type)
{
	if (type == ASTTypes::CompareEquals)
		return Opcodes::eq;
	if (type == ASTTypes::CompareNotEquals)
		return Opcodes::neq;
	if (type == ASTTypes::CompareLessThan)
		return Opcodes::cmplt;
	if (type == ASTTypes::CompareGreaterThan)
		return Opcodes::cmpgt;
	if (type == ASTTypes::CompareLessThanEqual)
		return Opcodes::cmple;
	if (type == ASTTypes::CompareGreaterThanEqual)
		return Opcodes::cmpge;

	return Opcodes::no_op;
}

Opcodes BytecodeCompiler::ResolveCorrectMathOpcode(ASTNode* n, bool reverse)
{
	if (n->type == ASTTypes::Add)
		return Opcodes::add;
	else if (n->type == ASTTypes::Subtract)
	{
		if (reverse)
			return Opcodes::sub_reverse;

		return Opcodes::sub;
	}
	else if (n->type == ASTTypes::Multiply)
		return Opcodes::mul;
	else if (n->type == ASTTypes::Divide)
	{
		if (reverse)
			return Opcodes::div_reverse;

		return Opcodes::div;
	}

	/*if (n->typeEntry == ASTTypes::ToThePower)
		return (reverse ? Opcodes::pow_rev : Opcodes::pow);
	if (n->typeEntry == ASTTypes::Modulus)
		return (reverse ? Opcodes::mod_rev : Opcodes::mod);
	if (n->typeEntry == ASTTypes::Xor)
		return (reverse ? Opcodes::xr_rev : Opcodes::xr);*/

	abort();

	return Opcodes::no_op;
};

Opcodes BytecodeCompiler::ResolveCorrectStoreOpcode(ValueType type)
{
	TypeTableEntry typeEntry = m_TypeTable.ResolveEntry(m_TypeTable.GetEntryFromId(type));

	switch (type)
	{
	case ValueTypes::Integer:
		return Opcodes::store_i;
	case ValueTypes::Float:
		return Opcodes::store_f;
	case ValueTypes::String:
		return Opcodes::store_sref;

	// Handle user defined types
	default:
		if (typeEntry.type == TypeTableType::Class)
			return Opcodes::store_objref;

		abort();
	}

	return Opcodes::no_op;
};

Opcodes BytecodeCompiler::ResolveCorrectLoadOpcode(ValueType type)
{
	TypeTableEntry typeEntry = m_TypeTable.ResolveEntry(m_TypeTable.GetEntryFromId(type));

	switch (type)
	{
	case ValueTypes::Integer:
		return Opcodes::load_i;
	case ValueTypes::Float:
		return Opcodes::load_f;
	case ValueTypes::String:
		return Opcodes::load_sref;

		// Handle user defined types
	default:
		if (typeEntry.type == TypeTableType::Class)
			return Opcodes::load_objref;

		abort();
	}

	return Opcodes::no_op;
};

ContextConstantsPool::ContextConstantsPool()
{

}

uint16_t ContextConstantsPool::AddAndGetIntegerIndex(int32_t value)
{
	// If an index already exists for this typeEntry
	if (m_Integers.count(value) == 1)
		return m_Integers[value];

	// Otherwise, create the entry and use the next free slot
	uint16_t index = ++m_CurrentFreeSlot;
	m_Integers[value] = index;

	return index;
}

uint16_t ContextConstantsPool::AddAndGetFloatIndex(float value)
{
	// If an index already exists for this typeEntry
	if (m_Floats.count(value) == 1)
		return m_Floats[value];

	// Otherwise, create the entry and use the next free slot
	uint16_t index = ++m_CurrentFreeSlot;
	m_Floats[value] = index;

	return index;
}

uint16_t ContextConstantsPool::AddAndGetStringIndex(std::string value)
{
	if (m_Strings.count(value) == 1)
		return m_Strings[value];

	// Otherwise, create the string and use the next free slot
	uint16_t index = ++m_CurrentFreeSlot;
	m_Strings[value] = index;

	return index;
}

uint16_t ContextConstantsPool::AddAndGetFunctionReferenceIndex(std::string functionName)
{
	if (m_Functions.count(functionName) == 1)
		return m_Functions[functionName];

	uint16_t index = ++m_CurrentFreeSlot;
	m_Functions[functionName] = index;

	return index;
}

uint16_t ContextConstantsPool::AddAndGetMethodReferenceIndex(std::string methodName)
{
	// TODO: Allow multiple methods (in different classes) with same name
	if (m_Methods.count(methodName) == 1)
		return m_Methods[methodName];

	uint16_t index = ++m_CurrentFreeSlot;
	m_Methods[methodName] = index;

	return index;
}

uint16_t ContextConstantsPool::AddAndGetClassIndex(std::string className)
{
	if (m_Classes.count(className) == 1)
		return m_Classes[className];

	uint16_t index = ++m_CurrentFreeSlot;
	m_Classes[className] = index;

	return index;
}

BytecodeCompiler::BytecodeCompiler()
{
	auto AddFunction = [&](std::string name, BuiltInFunctions::BuiltInFunctionCallable pointer, ValueType returnType, std::vector<ValueType> parameters)
	{
		uint16_t id = ++m_ConstantsPool.m_CurrentFreeSlot;

		assert(m_ConstantsPool.m_BuiltInFunctions.count(name) == 0);

		m_ConstantsPool.m_BuiltInFunctions[name] = BuiltInFunctions::Prototype(id, name, pointer, returnType);

		auto function = m_SymbolTable.InsertFunction(0, name, &m_TypeTable.GetEntryFromId(returnType), id);
		function->m_ParameterTypes = parameters;
		function->m_IsBuiltIn = true;
	};

	using namespace BuiltInFunctions;
	AddFunction("print", _print, ValueTypes::Void, { ValueTypes::String });
	AddFunction("printf", _printf, ValueTypes::Void, { ValueTypes::String });
}

CompiledFile BytecodeCompiler::CompileASTTree(ASTNode* root, TypeTable typeTable)
{
	//m_Context.m_TypeTable = typeTable;
	m_TypeTable = typeTable;

	// 1. Compile
	Compile(root, m_CompiledFile.m_TopLevelInstructions);

	// 2. Optimize
	OptimzeInstructions(m_CompiledFile.m_TopLevelInstructions);
	for (auto& entry : m_CompiledFile.m_Functions)
	{
		OptimzeInstructions(entry.second.body); // Function body
	}

	// 3. Encode to a proper bytecode buffer
	EncodeCompiledInstructions();

	// 4. Convert the context constants pool to the runtime version
	for (auto& entry : m_ConstantsPool.m_Integers)
		m_CompiledFile.m_ConstantsPool.m_Integers[entry.second] = entry.first;
	for (auto& entry : m_ConstantsPool.m_Floats)
		m_CompiledFile.m_ConstantsPool.m_Floats[entry.second] = entry.first;
	for (auto& entry : m_ConstantsPool.m_Strings)
		m_CompiledFile.m_ConstantsPool.m_Strings[entry.second] = entry.first;
	for (auto& entry : m_ConstantsPool.m_Functions)
		m_CompiledFile.m_ConstantsPool.m_FunctionReferences[entry.second] = entry.first;
	for (auto& entry : m_ConstantsPool.m_Methods)
		m_CompiledFile.m_ConstantsPool.m_MethodReferences[entry.second] = entry.first;
	for (auto& entry : m_ConstantsPool.m_Classes)
		m_CompiledFile.m_ConstantsPool.m_ClassReferences[entry.second] = entry.first;
	for (auto& entry : m_ConstantsPool.m_BuiltInFunctions)
		m_CompiledFile.m_ConstantsPool.m_BuiltInFunctions[entry.second.id] = entry.second;

	return m_CompiledFile;
}

void BytecodeCompiler::PrintInstructions(Instructions& instructions, std::string indent)
{
	for (uint32_t i = 0; i < instructions.size(); i++)
	{
		std::cout << indent << "(" << i << ") " << OpcodeToString(instructions[i].GetOpcode()) << " ";

		for (uint32_t a = 0; a < instructions[i].GetArguments().size(); a++)
		{
			if (a > 0 && a < instructions[i].GetArguments().size() - 1)
				std::cout << ", ";

			std::cout << (int)instructions[i].GetArguments()[a];
		}

		std::cout << "\n";
	}
	std::cout << "\n";
}

void BytecodeCompiler::PrintInstructions(EncodedInstructions& instructions)
{
	
}


std::optional<SymbolTable::VariableSymbol*> BytecodeCompiler::CreateSymbolForVariableDeclaration(ASTNode* node, ASTNode* parent, bool& isClassMemberVariable)
{
	auto IsTypeAccessible = [&](ASTNode* node, TypeTableEntry& variableType)
	{
		// A child class cannot be created without accessing the type with scope resolution
		if ((node->type == ASTTypes::Variable || node->type == ASTTypes::VariableType) && variableType.isPrivate)
		{
			// But if inside a class that has that class as a child class, then it is allowed
			if (m_CurrentParsingClass)
			{
				auto matchingChildClass = m_CurrentParsingClass->m_ChildClasses->LookupClassByTypeFirstLayer(variableType.id);
				if (matchingChildClass)
					return true;
			}

			return false;
		}
		
		return true;
	};

	const std::string& variableName = node->right->stringValue;

	auto variableTypeOpt = ResolveVariableType(node->left);
	if (!variableTypeOpt.has_value() || HasError())
		return std::nullopt;

	TypeTableEntry* variableType = variableTypeOpt.value();
	if (!IsTypeAccessible(node->left, *variableType))
	{
		MakeError("Symbol " + variableType->name + " is not accessible in this scope");
		return std::nullopt;
	}


	// Check if declared inside a class declaration, then it should be a member variable
	if (parent && parent->parent && parent->parent->type == ASTTypes::Class)
	{
		assert(m_CurrentParsingClass != nullptr);

		// If member variable has already been declared
		if (m_CurrentParsingClass->m_MemberVariables->Has(variableName))
		{
			MakeError("Symbol " + variableName + " has already been declared");
			return std::nullopt;
		}

		return m_CurrentParsingClass->m_MemberVariables->InsertVariable(m_CurrentScope, variableName, variableType);
	}

	// Ensure it has not been declared before
	if (m_SymbolTable.Has(variableName))
	{
		MakeError("Symbol " + variableName + " has already been declared");
		return std::nullopt;
	}

	// Invalid type of variable
	if (variableType->id == ValueTypes::Void)
	{
		MakeError("Variable " + variableName + " doesn't have a type");
		return std::nullopt;
	}

	return m_SymbolTable.InsertVariable(m_CurrentScope, variableName, variableType);
}

std::optional<CompiledCallable> BytecodeCompiler::CompileCallable(ASTNode* node, SymbolTable::FunctionSymbol& symbol)
{
	// Compile callable (function or method)
	Instructions body;
	m_CurrentScope++;

	// Compile the parameters
	for (int i = 2; i < node->left->arguments.size(); i++)
	{
		ASTNode* parameter = node->left->arguments[i];

		if (parameter->type == ASTTypes::VariableDeclaration)
		{
			bool isInClass = false;
			auto parameterSymbol = CreateSymbolForVariableDeclaration(parameter, parameter->parent, isInClass);

			if (!parameterSymbol.has_value() || HasError())
				return std::nullopt;

			symbol.m_ParameterTypes.push_back(parameterSymbol.value()->m_StorableValueType->Resolve().id);
		}
		else
		{
			MakeError("Invalid code in function parameters for funtion " + symbol.m_Name);
			return std::nullopt;
		}
	}

	// Compile body
	Compile(node->right, body, false);
	if (m_Error != "")
		return std::nullopt;

	// Some bodies has a manual return statement, but if none exists then create one automatically
	if (body.back().GetOpcode() != Opcodes::ret && body.back().GetOpcode() != Opcodes::ret_void)
		body.emplace_back(Opcodes::ret_void);

	uint16_t constantsPoolIndex = symbol.m_Id;

	CompiledCallable callable = {
		symbol.m_StorableValueType->Resolve().id,
		symbol.m_ParameterTypes,
		constantsPoolIndex,
		body,
		EncodeInstructions(body)
	};

	m_SymbolTable.Remove(m_CurrentScope);
	m_CurrentScope--;

	return callable;
}

void BytecodeCompiler::CompileCallableCall(ASTNode* node, Instructions& instructions, SymbolTable& symbolTableContext)
{
	const std::string& functionName = node->stringValue;
	if (!symbolTableContext.Has(functionName))
		return MakeError("Function " + functionName + " has not been defined");

	SymbolTable::FunctionSymbol* callableSymbol = (SymbolTable::FunctionSymbol*)symbolTableContext.Lookup(functionName);

	if (callableSymbol->m_SymbolType != SymbolType::Function && callableSymbol->m_SymbolType != SymbolType::Method)
		return MakeError("Symbol " + functionName + " cannot be called, is not a function or a method");

	std::vector<ValueType> parametersToCheck = callableSymbol->m_ParameterTypes;

	// A method has a hidden this paramater at the beginning. Pretend it doesn't exist when
	// typechecking as it is not provided by the sourcecode.
	if (callableSymbol->m_SymbolType == SymbolType::Method)
		parametersToCheck = std::vector<ValueType>(parametersToCheck.begin() + 1, parametersToCheck.end());

	if (parametersToCheck.size() != node->arguments.size())
		return MakeError("Missmatch in number of arguments for " + functionName + ". The function expects " +
			std::to_string(parametersToCheck.size()) + " but " + std::to_string(node->arguments.size())
			+ " where provided.");

	for (int i = 0; i < parametersToCheck.size(); i++)
	{
		ValueType expectedType = parametersToCheck[i];
		ValueType actualType = GetValueTypeOfNode(node->arguments[i]);

		if (actualType != expectedType)
			return MakeError("Missmatch in type of argument at position " + std::to_string(i) + " for " + functionName);
	}

	// Push all the arguments onto the stack backwards
	for (int i = node->arguments.size() - 1; i >= 0; i--)
	{
		Compile(node->arguments[i], instructions);
	}

	instructions.push_back(Instruction(Opcodes::call, { (uint8_t)callableSymbol->m_Id })); // TODO: support large index


	// TODO: Calling native functions
	//if (m_Context.m_ConstantsPool.(functionName))
	{
		//CompileStringLiteral(functionName, instructions);
		//instructions.push_back({ Opcodes::call_native, ResultCanBeDiscarded(node)).Arg(functionName).Arg(node->arguments.size()));
		//instructions.push_back({ Opcodes::call_native, { (uint8_t)});
		//break;
	}

	//auto symbol = symbolTableContext.Lookup(functionName);
	//if (symbol->m_SymbolType == SymbolType::Function || symbol->m_SymbolType == SymbolType::Method)
	//{
	//	SymbolTable::FunctionSymbol* function = (SymbolTable::FunctionSymbol*)symbol;

	//	instructions.push_back(Instruction(Opcodes::call, { (uint8_t)function->m_Id })); // TODO: support large index
	// TODO: fix
	//}
	//else if (symbol->m_SymbolType == SymbolType::Variable)
	//{
	//	SymbolTable::VariableSymbol* variable = (SymbolTable::VariableSymbol*)symbol;

	//	instructions.push_back(Instruction(Opcodes::call_fromstack)); // TODO: support large index
	//}
}

std::optional<SymbolTable::Symbol*> BytecodeCompiler::CompilePropertyAccess(ASTNode* node, Instructions& instructions)
{
	// Returns the symbol of the class with the same type as the variable
	auto GetClassSymbolOfVariable = [&](SymbolTable::Symbol* symbol)
	{
		ValueType variableStoredType = symbol->m_StorableValueType->id;
		return (SymbolTable::ClassSymbol*)m_SymbolTable.LookupClassByType(variableStoredType);
	};

	// Left is a propertyAccess, a variable or functionCall
	// Right is a variable or functionCall

	if (node->type == ASTTypes::Variable)
	{
		Compile(node, instructions);
		if (HasError())
			return std::nullopt;

		return (SymbolTable::Symbol*)GetClassSymbolOfVariable(m_SymbolTable.Lookup(node->stringValue));
	}
	else if (node->type == ASTTypes::FunctionCall)
	{
		abort();
		return std::nullopt;
	}

	assert(node->type == ASTTypes::PropertyAccess);

	auto parentSymbolOpt = CompilePropertyAccess(node->left, instructions);
	if (!parentSymbolOpt.has_value() || HasError())
		return std::nullopt;
	auto parentSymbol = parentSymbolOpt.value();

	// Must resolve the type of the lhs 
	//ValueType lhs = GetValueTypeOfNode(node->left);
	//ValueType rhs = GetValueTypeOfNode(right);

	const std::string& propertyName = node->right->stringValue;

	if (parentSymbol->m_SymbolType != SymbolType::Class)
	{
		MakeError("Failed to do property access on variable of type " + parentSymbol->m_Name + ", it is not a class");
		return std::nullopt;
	}

	SymbolTable::ClassSymbol* classSymbol = (SymbolTable::ClassSymbol*)parentSymbol;

	SymbolTable::VariableSymbol* prop = (SymbolTable::VariableSymbol*)classSymbol->m_MemberVariables->Lookup(propertyName);
	SymbolTable::FunctionSymbol* method = (SymbolTable::FunctionSymbol*)classSymbol->m_Methods->Lookup(propertyName);
	if (!prop && !method)
	{
		MakeError("Symbol '" + propertyName + "' doesn't exist on class '" + classSymbol->m_Name + "'");
		return std::nullopt;
	}

	// If property is a variable. Store or load it depending on the context
	if (prop)
	{
		Opcodes storeOrLoadOpcode = Opcodes::load_member;
		if (node->parent->IsAssignment())
			storeOrLoadOpcode = Opcodes::store_member;

		instructions.push_back({ storeOrLoadOpcode, { (uint8_t)prop->m_Index } });
	}

	// If trying to call the property. Make sure it is not a class member and a valid method.
	// Because the object the method is on is on the stack from a previous iteration, just compile the function as the 
	// 'this' parameter is already there and a method is a function (with this as first parameter).
	if (node->right->type == ASTTypes::FunctionCall)
	{
		if (prop)
		{
			MakeError("Symbol '" + propertyName + "' on '" + classSymbol->m_Name + "' is not a method, but a " + SymbolTypeToString(prop->m_SymbolType));
			return std::nullopt;
		}

		if (!method)
		{
			MakeError("Symbol '" + propertyName + "' on '" + classSymbol->m_Name + "' doesn't exist");
			return std::nullopt;
		}

		CompileCallableCall(node->right, instructions, *classSymbol->m_Methods);

		return (SymbolTable::VariableSymbol*)method;
	}

	auto propClassSymbol = GetClassSymbolOfVariable(prop);
	if (propClassSymbol) 
		return propClassSymbol;

	return prop;
}


std::optional<SymbolTable::ClassSymbol*> BytecodeCompiler::ResolveScopeResolution(ASTNode* node)
{
	// Left is a scope resolution or an identifier
	// Right is an identifier

	if (node->type == ASTTypes::Variable)
	{
		const std::string& typeName = node->stringValue;

		if (!m_TypeTable.HasType(typeName))
		{
			MakeError("Type " + typeName + " has not been defined");
			return std::nullopt;
		}

		ValueType baseType = m_TypeTable.GetType(typeName);

		auto cls = m_SymbolTable.LookupClassByType(baseType);
		if (!cls)
		{
			MakeError("Class " + typeName + " has not been defined");
			return std::nullopt;
		}

		return cls;
	}

	auto parentClassSymbol = ResolveScopeResolution(node->left);
	if (!parentClassSymbol.has_value() || HasError())
		return std::nullopt;

	const std::string& childTypeName = node->right->stringValue;

	if (!m_TypeTable.HasType(childTypeName))
	{
		MakeError("Type " + childTypeName + " has not been defined");
		return std::nullopt;
	}

	ValueType childType = m_TypeTable.GetType(childTypeName);

	auto childClass = parentClassSymbol.value()->m_ChildClasses->LookupClassByType(childType);
	if (!childClass)
	{
		MakeError("Child class '" + childTypeName + "' has not been defined on class " + parentClassSymbol.value()->m_Name);
		return std::nullopt;
	}

	return childClass;
}

std::optional<TypeTableEntry*> BytecodeCompiler::ResolveVariableType(ASTNode* node)
{
	switch (node->type)
	{
	case ASTTypes::VariableType:
	case ASTTypes::Variable:
	{
		const std::string& typeName = node->stringValue;

		if (!m_TypeTable.HasType(typeName))
		{
			MakeError("Type '" + typeName + "' has not been defined");
			return std::nullopt;
		}

		return &m_TypeTable.GetTypeEntry(typeName);
	}
	case ASTTypes::ScopeResolution:
	{
		auto resolvedSymbol = ResolveScopeResolution(node);
		if (!resolvedSymbol.has_value() || HasError())
			return std::nullopt;

		return resolvedSymbol.value()->m_StorableValueType;
	}
	default:
		abort();
	}
}

void BytecodeCompiler::CompileClass(ASTNode* node, SymbolTable::ClassSymbol* parentClass)
{
	std::string className = node->stringValue;

	if (m_SymbolTable.Has(className))
		return MakeError("Class " + className + " has already been declared");

	uint16_t classId = m_ConstantsPool.AddAndGetClassIndex(className);

	// m_TypeTable.Add(className, TypeTableType::Class);//m_TypeTable.GetTypeEntry(className);
	
	// If parsing a child class
	SymbolTable::ClassSymbol* classSymbol = nullptr;
	if (m_CurrentParsingClass)
	{
		auto classType = &m_TypeTable.AddPrivateType(className, TypeTableType::Class);
		classSymbol = m_CurrentParsingClass->m_ChildClasses->InsertClass(m_CurrentScope, className, classType);
	} 
	else
	{
		auto classType = &m_TypeTable.Add(className, TypeTableType::Class);
		classSymbol = m_SymbolTable.InsertClass(m_CurrentScope, className, classType);
	}

	m_CurrentParsingClass = classSymbol;

	// Symbol that refers to the current instance of the class. Declared to avoid errors, might fix.
	//m_SymbolTable.InsertVariable(m_CurrentScope + 1, "this", &classType);

	// Create the object that holds the information used when the bytecode is interpreted
	ClassInstance classInstance(className, classId);

	m_CurrentScope++;

	// Iterate the declaration
	for (ASTNode* line : node->left->arguments)
	{
		if (HasError())
			return;

		if (line->type == ASTTypes::Class)
		{
			Instructions nestedClass;

			Compile(line, nestedClass);
			m_CurrentParsingClass = classSymbol;

			// Doesn't remove the class symbol, but all the contents it has
			//classSymbol->m_ChildClasses->Remove(m_CurrentScope);
		}
		else if (line->type == ASTTypes::Assign || line->type == ASTTypes::VariableDeclaration)
		{
			Compile(line, classInstance.m_InternalConstructor);
		}
		else if (line->type == ASTTypes::FunctionDefinition)
		{
			auto compiledMethod = CompileClassMethod(line, *classSymbol);
			if (!compiledMethod.has_value() || HasError())
				return;

			uint16_t ind = compiledMethod.value().constantsPoolIndex;

			classInstance.m_Methods[compiledMethod.value().constantsPoolIndex] = compiledMethod.value();

			//classSymbol->m_Methods->Remove(m_CurrentScope);
		}
		else
		{
			return MakeError("Unsupported code in class declaration");
		}
	}
	m_CurrentScope--;
	m_CurrentParsingClass = nullptr;

	// Member variables
	classInstance.m_MemberVariables.reserve(classSymbol->m_MemberVariables->GetSymbols().size());
	for (auto& entry : classSymbol->m_MemberVariables->GetSymbols())
	{
		//CompilerContext::Variable& variable = entry.second;
		SymbolTable::VariableSymbol& variableSymbol = *(SymbolTable::VariableSymbol*)entry.second;

		classInstance.m_MemberVariables[variableSymbol.m_Index] = { variableSymbol.GetTypeTableEntry().Resolve().id, Value() };
	}

	m_CompiledFile.m_Classes[classInstance.m_Index] = classInstance;
}

std::optional<CompiledCallable> BytecodeCompiler::CompileClassMethod(ASTNode* node, SymbolTable::ClassSymbol& classSymbol)
{
	std::string returnType = node->left->arguments[0]->stringValue;
	std::string methodName = node->left->arguments[1]->stringValue;
	std::string fullName = classSymbol.m_Name + "::" + methodName;

	if (!node->right)
	{
		MakeError("Function " + methodName + " is missing a body");
		return std::nullopt;
	}

	if (!m_TypeTable.HasType(returnType))
	{
		MakeError("Type '" + returnType + "' has not been defined'");
		return std::nullopt;
	}
	if (classSymbol.m_Methods->Has(methodName))
	{
		MakeError("Symbol " + fullName + " has already been defined");
		return std::nullopt;
	}

	if (!IsFunctionNameValid(methodName))
	{
		MakeError("Method " + methodName + " is not valid");
		return std::nullopt;
	}

	// TODO: Check so no methods are created inside each other

	auto& returnTypeEntry = m_TypeTable.GetTypeEntry(returnType);
	uint16_t methodId = m_ConstantsPool.AddAndGetMethodReferenceIndex(methodName);
	auto& methodSymbol = *classSymbol.m_Methods->InsertMethod(m_CurrentScope, methodName, &returnTypeEntry, methodId);

	// Add the hidden 'this' parameter
	//auto thisSymbol = m_SymbolTable.InsertVariable(m_CurrentScope + 1, "this", classSymbol.m_StorableValueType);
	methodSymbol.m_ParameterTypes.push_back(classSymbol.m_StorableValueType->id);
	
	return CompileCallable(node, methodSymbol);
}

void BytecodeCompiler::CompileFunction(ASTNode* node)
{
	std::string returnType = node->left->arguments[0]->stringValue;
	std::string functionName = node->left->arguments[1]->stringValue;

	if (!node->right)
		return MakeError("Function " + functionName + " is missing a body");

	if (m_SymbolTable.Has(functionName))
		return MakeError("Function " + functionName + " has already been defined");

	if (!IsFunctionNameValid(functionName))
		return MakeError("Function " + functionName + " is not valid");

	if (!m_TypeTable.HasType(returnType))
		return MakeError("Type '" + returnType + "' has not been defined'");

	// A function declaration has to be global
	//if (m_CurrentScope != 0)
		//return MakeError("Function " + functionName + " is not in the global scope");

	auto& returnTypeEntry = m_TypeTable.GetTypeEntry(returnType);
	uint16_t functionId = m_ConstantsPool.AddAndGetFunctionReferenceIndex(functionName);
	auto& functionSymbol = *m_SymbolTable.InsertFunction(m_CurrentScope, functionName, &returnTypeEntry, functionId);

	auto compiledFunction = CompileCallable(node, functionSymbol);
	if (!compiledFunction.has_value() || HasError())
		return;

	m_CompiledFile.m_Functions[functionId] = compiledFunction.value();
}

void BytecodeCompiler::CompileAssignment(ASTNode* node, Instructions& instructions)
{
	std::string propertyName = "";

	//m_SymbolTable.Insert(m_CurrentScope, )

	//CompilerContext::Variable variable(-1, node->left->stringValue/*, NodeTypeToValueType(node->right)*/);

	bool isClassMemberVariable = false;

	std::string variableName = node->left->stringValue;
	ValueType variableType;

	SymbolTable::VariableSymbol* variableSymbol = nullptr;

	// As long as the assignment isn't += to a property, compile the rhs first.
	// This is because then the value is on the stack and can easily be popped and stored
	// Property compound assignments requires the property access be compiled first
	// for more efficiently adding a value to itself without having to access the 
	// property all over again.
	if (node->left->type != ASTTypes::PropertyAccess && !node->IsCompoundAssignment())
	{
		Compile(node->right, instructions);
		if (HasError())
			return;
	}

	// Assigning to a property.
	// Compile propery access part
	if (node->left->type == ASTTypes::PropertyAccess)
	{
		auto propertySymbol = CompilePropertyAccess(node->left, instructions);
		if (!propertySymbol.has_value() || HasError())
			return;

		// Remove the store to property instruction
		Instruction storeInstruction = instructions.back();
		instructions.pop_back();

		if (node->IsCompoundAssignment())
		{
			instructions.push_back({ Opcodes::dup });
			instructions.push_back({ Opcodes::load_member, storeInstruction.GetArguments()});

			Compile(node->right, instructions);
			if (HasError())
				return;

			if (node->type == ASTTypes::PlusEquals)
				instructions.push_back({ Opcodes::add });
			if (node->type == ASTTypes::MinusEquals)
				instructions.push_back({ Opcodes::sub }); // TODO: Might be in the reverse order
		}
		else
		{
			Compile(node->right, instructions);
			if (HasError())
				return;
		}

		// And add it back after the rhs has been evaluated, pushed to stack and can the be stored
		instructions.push_back(storeInstruction);

		ValueType lhs = propertySymbol.value()->m_StorableValueType->id;
		ValueType rhs = GetValueTypeOfNode(node->right);

		if (lhs != rhs)
			return MakeError("Variable '" + propertySymbol.value()->m_Name + "' (" + m_TypeTable.GetEntryFromId(lhs).name +
				") cannot be assigned something of type '" + m_TypeTable.GetEntryFromId(rhs).name + "'");


		return;
	}

	// Resolve if variable decleration on the left. Should create a new variable
	if (node->left->type == ASTTypes::VariableDeclaration)
	{
		if (node->IsCompoundAssignment())
			return MakeError("Cannot perform compound assignment on a variable declaration");

		auto variableSymbolOptional = CreateSymbolForVariableDeclaration(node->left, node->parent, isClassMemberVariable);
		if (variableSymbolOptional.has_value())
			variableSymbol = variableSymbolOptional.value();

		if (HasError()) 
			return;

		ValueType lhs = variableSymbol->m_StorableValueType->id;
		ValueType rhs = GetValueTypeOfNode(node->right);

		if (lhs != rhs)
			return MakeError("Variable '" + variableSymbol->m_Name + "' (" + m_TypeTable.GetEntryFromId(lhs).name +
				") cannot be assigned something of type '" + m_TypeTable.GetEntryFromId(rhs).name + "'");
	}

	else
	{
		// Assigning to an existing variable

		if (!m_SymbolTable.HasAndIs(variableName, SymbolType::Variable))
		{
			return MakeError("Undeclared variable " + variableName);
		}

		variableSymbol = (SymbolTable::VariableSymbol*)m_SymbolTable.Lookup(variableName);
		variableType = variableSymbol->GetTypeTableEntry().id;

		// Load the variable on the lhs of assignment add/sub the rhs with that before assigning
		if (node->IsCompoundAssignment())
		{
			Compile(node->left, instructions);

			if (node->type == ASTTypes::PlusEquals)
				instructions.push_back({ Opcodes::add });
			if (node->type == ASTTypes::MinusEquals)
				instructions.push_back({ Opcodes::sub }); // TODO: Might be in the reverse order
		}

		// Typecheck
		ValueType rhs = GetValueTypeOfNode(node->right);
		if (variableType != rhs)
			return MakeError("Variable '" + variableName + "' (" + m_TypeTable.GetEntryFromId(variableType).name +
				") cannot be assigned something of type '" + m_TypeTable.GetEntryFromId(rhs).name + "'");
	}

	assert(variableSymbol != nullptr);

	if (isClassMemberVariable)
		instructions.push_back(Instruction(Opcodes::store_member, { (uint8_t)variableSymbol->m_Index }));
	else
		instructions.push_back(Instruction(ResolveCorrectStoreOpcode(GetValueTypeOfNode(node->left)), { (uint8_t)variableSymbol->m_Index }));
}

void BytecodeCompiler::CompileMathOperation(ASTNode* node, Instructions& instructions)
{
	auto TypecheckLeftAndRightNode = [&](ASTNode* lhs, ASTNode* rhs) -> void
	{
		// Typechecking
		ValueType typeLhs = GetValueTypeOfNode(lhs);
		if (HasError()) return;
		ValueType typeRhs = GetValueTypeOfNode(rhs);
		if (HasError()) return;

		if (typeLhs != typeRhs)
		{
			const std::string& typeLhsString = m_TypeTable.GetEntryFromId(typeLhs).name;
			const std::string& typeRhsString = m_TypeTable.GetEntryFromId(typeRhs).name;

			return MakeError("Non-match matching types for " + node->right->ToString(false) + " (" + typeLhsString + " and " + typeRhsString + ")");
		}
	};

	ASTNode* left = node->left;
	ASTNode* right = node->right;

	// Recursivly perform the operations, do the inner ones first
	if (right->IsMathOperator())
	{
		TypecheckLeftAndRightNode(right, left);
		if (HasError()) return;

		Compile(right, instructions);
		Compile(left, instructions);

		instructions.emplace_back(ResolveCorrectMathOpcode(node));

		return;
	}
	if (left->IsMathOperator())
	{
		TypecheckLeftAndRightNode(right, left);
		if (HasError()) return;

		Compile(left, instructions);
		Compile(right, instructions);

		instructions.emplace_back(ResolveCorrectMathOpcode(node, true));

		return;
	}

	TypecheckLeftAndRightNode(left, right);
	if (HasError()) return;

	Compile(right, instructions);
	Compile(left, instructions);

	if (node->IsMathOperator())
		instructions.emplace_back(ResolveCorrectMathOpcode(node));
}

void BytecodeCompiler::CompileIntLiteral(int64_t value, Instructions& instructions)
{
	// 8 bit signed int
	if (value >= SCHAR_MIN && value <= SCHAR_MAX)
	{
		// Push directly with the single byte
		uint8_t byte = value;

		instructions.push_back({ Opcodes::push_ibyte, { byte } });
	}
	// 16 bit signed int
	else if (value >= SHRT_MIN && value <= SHRT_MAX)
	{
		// Push directly with two bytes
		uint8_t lowByte = value & 0xff;
		uint8_t highByte = value >> 8;

		instructions.push_back({ Opcodes::push_ishort, { highByte, lowByte } });
	}
	// 32 bit signed int
	else if (value >= INT_MIN && value <= INT_MAX)
	{
		// Create or get an entry in the constants pool
		uint16_t constantsIndex = m_ConstantsPool.AddAndGetIntegerIndex(value);

		instructions.push_back(LoadFromConstantsPool(constantsIndex));
	}
	// TODO: 64 bit 
	else
	{

		abort();
	}
}

void BytecodeCompiler::CompileDoubleLiteral(float value, Instructions& instructions)
{
	// Create or get an entry in the constants pool
	uint16_t constantsIndex = m_ConstantsPool.AddAndGetFloatIndex(value);

	instructions.push_back(LoadFromConstantsPool(constantsIndex));
}

void BytecodeCompiler::CompileStringLiteral(std::string value, Instructions& instructions)
{
	// Create or get an entry in the constants pool
	uint16_t constantsIndex = m_ConstantsPool.AddAndGetStringIndex(value);

	instructions.push_back(LoadFromConstantsPool(constantsIndex));
}

Instruction BytecodeCompiler::LoadFromConstantsPool(uint16_t index)
{
	// Index is less than 1 byte
	if (index <= UCHAR_MAX)
	{
		return { Opcodes::load_constant, { (uint8_t)index } };
	}
	else
	{
		// Split index into two bytes
		uint8_t lowByte = index & 0xff;
		uint8_t highByte = index >> 8;

		return { Opcodes::load_constant_wide_index, { highByte, lowByte } };
	}
}

bool BytecodeCompiler::IsFunctionNameValid(const std::string& name)
{
	if (name == "") 
		return false;

	// TODO: Add more checks

	return true;
}

bool ResultCanBeDiscarded(ASTNode* node)
{
	assert(node->parent != nullptr);

	return (node->parent->type == ASTTypes::Scope ||
		node->parent->type == ASTTypes::ProgramBody ||
		node->parent->type == ASTTypes::ForStatement);
}

void BytecodeCompiler::Compile(ASTNode* node, Instructions& instructions, bool canCreateScope, bool canMakeVariablesLocal)
{
	ASTNode* left = node->left;
	ASTNode* right = node->right;

	if (HasError())
		return;

	switch (node->type)
	{
	case ASTTypes::ModuleBody:
	{
		return Compile(node->left, instructions, false);
	}
	case ASTTypes::ProgramBody:
	case ASTTypes::Scope:
	{
		//CompilerContext initialContext = m_Context;

		if (node->type == ASTTypes::Scope && canCreateScope)
		{
			m_CurrentScope++;
			//instructions.push_back({ Opcodes::create_scope_frame.Arg(m_CurrentScope));
		}

		// Iterate the lines of the scope
		for (int i = 0; i < node->arguments.size(); i++)
		{
			ASTNode* n = node->arguments[i];
			/*if (n->typeEntry == ASTTypes::FunctionDefinition)
				continue;
			if (IsAnonFunctionDef(n))
				continue;*/

			if (HasError())
				return;

			Compile(n, instructions);
		}

		// Reset so the local variables created in the scope are discarded
		if (canMakeVariablesLocal)
		{
			//m_Context.m_Variables = initialContext.m_Variables;
			//m_Context.m_NextFreeVariableIndex = initialContext.m_NextFreeVariableIndex;
		}

		if (node->type == ASTTypes::Scope && canCreateScope)
		{
			// Delete the symbol table for the local scope as it is no longer needed
			m_SymbolTable.Remove(m_CurrentScope);

			//instructions.push_back(Instruction(Opcodes::pop_scope_frame).Arg(m_CurrentScope));
			m_CurrentScope--;

		}

		break;
	}

	case ASTTypes::Class:
	{
		CompileClass(node);
		break;
	}

	case ASTTypes::IfStatement:
	{
		// Format: check condition, jmp_if_false, scope code
		// The if statement is inversed, so if the condition is true then it just increments the pc and keeps running.
		// If it's false, then it jumps over the code for that statement 

		// Create bytecode for the condition
		Compile(node->left, instructions);

		uint32_t positionOfJumpOpcode = instructions.size();

		// Replace the dummy jump opcode with the proper one
		uint32_t positionOfEndOfScope = -1;
		instructions.push_back(Opcodes::jmp_if_false);

		// Create bytecode for the scope
		Compile(node->right, instructions);

		positionOfEndOfScope = instructions.size(); // Position of the end of the scope to run

		// Insert the jump opcode before the scope bytecode
		instructions[positionOfJumpOpcode] = Instruction(Opcodes::jmp_if_false, { (uint8_t)positionOfEndOfScope });

		break;
	}

	case ASTTypes::Else:
	{
		// Format: check condition, jmp_if_false, scope code
		// The if statement is inversed, so if the condition is true then it just increments the pc and keeps running.
		// If it's false, then it jumps over the code for that statement 

		int positionOfJumpOutOpcodeLeft = -1;
		int positionOfJumpOutOpcodeRight = -1;

		auto CreateCodeForIfStatements = [&](ASTNode* childNode)
		{
			// Create bytecode for the condition
			Compile(childNode->left, instructions);

			uint32_t positionOfJumpOpcode = instructions.size();

			// Insert a dummy jump opcode that will be removed
			instructions.emplace_back(Opcodes::jmp_if_false);

			// Create bytecode for the scope
			Compile(childNode->right, instructions);

			uint32_t positionOfEndOfScope = instructions.size() + 1; // Position of the end of the scope to run

			// Replace the dummy jump opcode with the proper one
			instructions[positionOfJumpOpcode] = Instruction(Opcodes::jmp_if_false, { (uint8_t)positionOfEndOfScope });

			instructions.emplace_back(Opcodes::jmp);
		};

		if (node->left->type == ASTTypes::IfStatement)
		{
			CreateCodeForIfStatements(node->left);
			positionOfJumpOutOpcodeLeft = instructions.size() - 1; // the jump index is at .size() - 1
		}

		if (node->right->type == ASTTypes::IfStatement)
		{
			CreateCodeForIfStatements(node->right);
			positionOfJumpOutOpcodeRight = instructions.size() - 1;  // the jump index is at .size() - 1
		}

		if (node->right->type == ASTTypes::Else || node->right->type == ASTTypes::Scope)
			Compile(node->right, instructions);

		uint8_t endOfElseIfLadder = instructions.size();

		if (positionOfJumpOutOpcodeLeft != -1) 
			instructions[positionOfJumpOutOpcodeLeft] = Instruction(Opcodes::jmp, { endOfElseIfLadder });
		if (positionOfJumpOutOpcodeRight != -1)
			instructions[positionOfJumpOutOpcodeRight] = Instruction(Opcodes::jmp, { endOfElseIfLadder });

		break;
	}

	case ASTTypes::WhileStatement:
	{
		// Format: check condition, jmp_if_false, scope code
		// The if statement is inversed, so if the condition is true then it just increments the pc and keeps running.
		// If it's false, then it jumps over the code for that statement 
		//m_CurrentScope++;

		////instructions.push_back(Instruction(Opcodes::create_scope_frame).Arg(m_CurrentScope));

		//uint32_t positionForCondition = instructions.size();

		//// Generate bytecode for the scope, but only to know the amount of instructions
		//std::vector<Instruction> tempInst = instructions;
		//BytecodeCompiler temp;
		//temp.m_Context = m_Context;
		//temp.m_CompiledFile.m_ConstantsPool = m_CompiledFile.m_ConstantsPool;

		//// Temp compilations
		//// Main scope
		//temp.Compile(node->left, tempInst);
		//temp.Compile(node->right, tempInst);
		//uint32_t scopeEnd = tempInst.size() + 2; // The position of the pop_frame opcode, which is the end of the scope

		//m_Context.m_LoopInfo.m_End = scopeEnd;
		//m_Context.m_LoopInfo.m_Reset = positionForCondition;
		//m_Context.m_LoopInfo.m_BodyDepth = m_CurrentScope + 1;

		//// Create bytecode for the condition
		//Compile(node->left, instructions);

		//// Insert a dummy jump opcode that will be removed
		//
		//instructions.push_back({ Opcodes::jmp_if_false, { (uint8_t)scopeEnd } });
		//

		//// Create bytecode for the scope
		//Compile(node->right, instructions);

		//// Jump back to the condition
		//instructions.push_back({ Opcodes::jmp, { (uint8_t)positionForCondition } });
		////instructions.push_back(Instruction(Opcodes::pop_scope_frame).Arg(m_CurrentScope));

		//// Reset the loop information
		//m_Context.m_LoopInfo = CompilerContext::LoopInfo();

		//m_CurrentScope--;

		break;
	}

	//case ASTTypes::ForStatement:
	//{
	//	// Format: initialization, check condition, jmp_if_false, scope code
	//	// The if statement is inversed, so if the condition is true then it just increments the pc and keeps running.
	//	// If it's false, then it jumps over the code for that statement 
	//	m_CurrentScope++;
	//	//instructions.push_back(Instruction(Opcodes::create_scope_frame).Arg(m_CurrentScope));

	//	// Create bytecode for the initialization
	//	Compile(node->arguments[0], instructions);

	//	uint32_t positionForCondition = instructions.size();

	//	// Create bytecode for the condition
	//	Compile(node->arguments[1], instructions);

	//	uint32_t positionOfJumpOpcode = instructions.size();

	//	// Insert a dummy jump opcode that will be removed
	//	instructions.emplace_back(Opcodes::jmp_if_false);

	//	// Generate bytecode for the scope, but only to know the amount of instructions
	//	CompilerContext::LoopInfo m_OriginalLoopInfo = m_Context.m_LoopInfo;
	//	std::vector<Instruction> tempInst = instructions;
	//	BytecodeCompiler temp;
	//	temp.m_Context = m_Context;
	//	temp.m_Context.m_LoopInfo.m_InLoop = true;
	//	temp.m_Constants = m_Constants;

	//	// Temp compilations
	//	// Main scope
	//	temp.Compile(node->right, tempInst);
	//	uint32_t loopResetPosition = tempInst.size() - 2; // Subtracting two for some reason, maybe to account for some future instructions?

	//	// Increment part
	//	temp.Compile(node->arguments[2], tempInst);
	//	uint32_t scopeEnd = tempInst.size() - 1; // Subtracting 1 because of the pop_frame opcode, which is the end of the scope

	//	m_Context.m_LoopInfo.m_InLoop = true;
	//	m_Context.m_LoopInfo.m_End = scopeEnd;
	//	m_Context.m_LoopInfo.m_Reset = loopResetPosition;
	//	m_Context.m_LoopInfo.m_BodyDepth = m_CurrentScope + 1;

	//	// Actually create bytecode for the scope
	//	Compile(node->right, instructions, false);

	//	// Add the increment bytecode
	//	Compile(node->arguments[2], instructions);

	//	// Replace the dummy jump opcode with the proper one
	//	instructions[positionOfJumpOpcode] = Instruction(Opcodes::jmp_if_false, { scopeEnd });

	//	// Jump back to the condition
	//	instructions.push_back({ Opcodes::jmp, { positionForCondition} });

	//	//instructions.push_back(Instruction(Opcodes::pop_scope_frame).Arg(m_CurrentScope));

	//	// Reset the loop information
	//	m_Context.m_LoopInfo = m_OriginalLoopInfo;

	//	m_CurrentScope--;

	//	break;
	//}

	case ASTTypes::FunctionDefinition:
	{
		CompileFunction(node);
		break;
	}



	case ASTTypes::VariableDeclaration:
	{
		bool isGlobalVariable = m_CurrentScope == 0;

		bool isClassMember = false;
		auto variableSymbol = CreateSymbolForVariableDeclaration(node, node->parent, isClassMember);
		
		if (!variableSymbol.has_value() || HasError())
			return;

		auto& typeEntry = variableSymbol.value()->m_StorableValueType->Resolve();

		// Assign a default value to it
		if (typeEntry.id == ValueTypes::Integer)
			CompileIntLiteral(0, instructions);
		else if (typeEntry.id == ValueTypes::Float)
			CompileDoubleLiteral(0.0f, instructions);
		else if (typeEntry.id == ValueTypes::String)
			CompileStringLiteral("", instructions);

		// Instantiate class
		// TODO: Wide indecies
		// TODO: if class, run code for constructor?
		if (typeEntry.type == TypeTableType::Class)
		{
			uint16_t classId = m_ConstantsPool.AddAndGetClassIndex(typeEntry.name);

			instructions.push_back({ Opcodes::instantiate_class, { (uint8_t)classId } });
		}

		instructions.push_back({ ResolveCorrectStoreOpcode(typeEntry.id), {(uint8_t)variableSymbol.value()->m_Index}});

		break;
	}

	case ASTTypes::Assign:
	case ASTTypes::PlusEquals:
	case ASTTypes::MinusEquals:
	{
		CompileAssignment(node, instructions);
		break;
	}

	//case ASTTypes::PostDecrement:
	//case ASTTypes::PostIncrement:
	//{
	//	// Load the variable
	//	int index = m_Context.GetVariable(node->left->stringValue).m_Index;

	//	if (index == -1)
	//		return MakeError("Variable " + node->stringValue + " doesn't exist");

	//	if (node->typeEntry == ASTTypes::PostIncrement)
	//		instructions.push_back(Instruction(Opcodes::post_inc, ResultCanBeDiscarded(node)).Arg(index));
	//	if (node->typeEntry == ASTTypes::PostDecrement)
	//		instructions.push_back(Instruction(Opcodes::post_dec, ResultCanBeDiscarded(node)).Arg(index));

	//	break;
	//}

	case ASTTypes::Add:
	case ASTTypes::Subtract:
	case ASTTypes::Multiply:
	case ASTTypes::Divide:
	case ASTTypes::Xor:
	case ASTTypes::ToThePower:
	case ASTTypes::Modulus:
	{
		CompileMathOperation(node, instructions);
		break;
	}

	case ASTTypes::FunctionCall:
	{
		CompileCallableCall(node, instructions, m_SymbolTable);
		break;
	}

	case ASTTypes::Return:
	{
		// Check if there is an statement after the return
		if (left && left->type != ASTTypes::Empty)
		{
			// Convert the expression to bytecode
			Compile(left, instructions);

			instructions.emplace_back(Opcodes::ret);
		}
		else
		{
			// Otherwise return null
			instructions.emplace_back(Opcodes::ret_void);
		}

		break;
	}

	case ASTTypes::Break:
	{
		//if (!m_Context.m_LoopInfo.m_InLoop)
			return MakeError("A break statement needs to be inside a loop");

		//instructions.push_back({ Opcodes::jmp, { Value(m_Context.m_LoopInfo.m_End, ValueTypes::Integer) } });
		break;
	}
	case ASTTypes::Continue:
	{
		//if (!m_Context.m_LoopInfo.m_InLoop)
			return MakeError("A continue statement needs to be inside a loop");

		//instructions.push_back(Instruction(Opcodes::pop_scope_frame).Arg(m_Context.m_LoopInfo.m_BodyDepth));
		//instructions.push_back({ Opcodes::jmp, { Value(m_Context.m_LoopInfo.m_Reset, ValueTypes::Integer) } });
		break;
	}

	case ASTTypes::CompareEquals:
	case ASTTypes::CompareNotEquals:
	case ASTTypes::CompareLessThan:
	case ASTTypes::CompareGreaterThan:
	case ASTTypes::CompareLessThanEqual:
	case ASTTypes::CompareGreaterThanEqual:
	{
		Compile(node->right, instructions);
		Compile(node->left, instructions);

		instructions.emplace_back(ASTComparisonTypeToOpcode(node->type));

		break;
	}

	/*case ASTTypes::And:
	case ASTTypes::Or:
	{
		Compile(node->right, instructions);
		Compile(node->left, instructions);

		if (node->typeEntry == ASTTypes::And)
			instructions.emplace_back(Opcodes::logical_and);
		if (node->typeEntry == ASTTypes::Or)
			instructions.emplace_back(Opcodes::logical_or);

		break;
	}

	case ASTTypes::Not:
	{
		Compile(node->left, instructions);

		instructions.emplace_back(Opcodes::logical_not);

		break;
	}*/

	//case ASTTypes::MemberAcessor:
	//{
	//	// Slot 0 should always contain the 'this' value
	//	instructions.push_back({ Opcodes::load_objref, { (uint8_t)0} });

	//	//m_CurrentParsingClass.

	//	instructions.push_back({ Opcodes::load_member, { (uint8_t)0 } });
	//	
	//	Compile(left, instructions);

	//	break;
	//}

	case ASTTypes::PropertyAccess:
	{
		CompilePropertyAccess(node, instructions);

		break;
	}

	case ASTTypes::ScopeResolution:
	{
		//abort();

		break;
	}

	case ASTTypes::Variable:
	{
		const std::string& variableName = node->stringValue;
		if (m_SymbolTable.Has(variableName))
		{
			auto symbol = m_SymbolTable.Lookup(variableName);
			if (symbol->m_SymbolType == SymbolType::Function)
			{
				SymbolTable::FunctionSymbol* f = (SymbolTable::FunctionSymbol*)symbol;
				instructions.push_back({ ResolveCorrectLoadOpcode(ValueTypes::Integer), { (uint8_t)f->m_Id} });
				return;
			}
		} else
			return MakeError("Variable " + variableName + " doesn't exist");
			
		SymbolTable::VariableSymbol& variable = *(SymbolTable::VariableSymbol*)m_SymbolTable.Lookup(variableName);
		uint16_t index = variable.m_Index;

		// Index is less than 1 byte
		if (index <= UCHAR_MAX)
		{
			instructions.push_back({ ResolveCorrectLoadOpcode(variable.m_StorableValueType->id), { (uint8_t)index} });
		}
		else
		{
			// TODO: 
			abort();
			// Split index into two bytes
			uint8_t lowByte = index & 0xff;
			uint8_t highByte = index >> 8;

			//instructions.push_back({ Opcodes::load_constant_wide_index, { highByte, lowByte } });
		}

		break;
	}

	case ASTTypes::Null:
	{
		//instructions.push_back(Instruction(Opcodes::push_null, ResultCanBeDiscarded(node)));

		break;
	}

	case ASTTypes::IntLiteral:
	{
		CompileIntLiteral(node->numberValue, instructions);
		break;
	}

	case ASTTypes::DoubleLiteral:
	{
		CompileDoubleLiteral(node->numberValue, instructions);
		break;
	}

	case ASTTypes::StringLiteral:
	{
		CompileStringLiteral(node->stringValue, instructions);
		break;
	}

	//case ASTTypes::ListInitializer:
	//{
	//	bool isObject = false;

	//	for (int i = node->arguments.size() - 1; i >= 0; i--)
	//	{
	//		Compile(node->arguments[i], instructions);

	//		if (node->arguments[i]->typeEntry == ASTTypes::PropertyAssign)
	//			isObject = true;
	//	}

	//	bool discardValue = node->parent->typeEntry == ASTTypes::Scope;

	//	// Check if the initializer is an object. Specify how many operand to add to the array, to be able to have nested arrays
	//	if (isObject)
	//		instructions.push_back(Instruction(Opcodes::object_create, discardValue).Arg(node->arguments.size()));
	//	else
	//		instructions.push_back(Instruction(Opcodes::array_create, discardValue).Arg(node->arguments.size()));

	//	break;
	//}
	}

	if (m_Error != "")
		return;
}

void BytecodeCompiler::EncodeCompiledInstructions()
{
	m_CompiledFile.m_EncodedTopLevelInstructions = EncodeInstructions(m_CompiledFile.m_TopLevelInstructions);

	for (auto& entry : m_CompiledFile.m_Functions)
	{
		//m_CompiledFile.m_EncodedFunctions[entry.first] = EncodeInstructions(entry.second);
	}

	for (auto& classEntry : m_CompiledFile.m_Classes)
	{
		/*for (auto& methodEntry : classEntry.second.m_Methods)
		{
			m_CompiledFile.m_Classes[classEntry.first].m_Methods[methodEntry.first] = EncodeInstructions(methodEntry.second);
		}*/

		//m_CompiledFile.m_Classes[classEntry.first].m_InternalConstructor = EncodeInstructions(classEntry.second.m_Constructor);
	}


}

void BytecodeCompiler::OptimzeInstructions(Instructions& instructions)
{
	// TODO: implement
}

EncodedInstructions BytecodeCompiler::EncodeInstructions(Instructions& instructions)
{
	InstructionsEncoder encoded;
	for (auto& instruction : instructions)
	{
		encoded.AddOpcode(instruction.GetOpcode());

		for (uint8_t argument : instruction.GetArguments())
		{
			encoded.AddInt(argument);
		}
	}

	return encoded.GetData();
}


ValueType BytecodeCompiler::GetValueTypeOfNode(ASTNode* node)
{
	switch (node->type)
	{
	case ASTTypes::ProgramBody:
	case ASTTypes::Scope:
	{
		abort();
		break;
	}
	case ASTTypes::VariableDeclaration:
	case ASTTypes::GlobalVariableDeclaration:
		return GetValueTypeOfNode(node->left);
	case ASTTypes::VariableType:
		return m_TypeTable.GetType(node->stringValue);
	case ASTTypes::Assign:
	{
		abort();
		break;
	}

	case ASTTypes::CompareEquals:
	case ASTTypes::CompareNotEquals:
	case ASTTypes::CompareLessThan:
	case ASTTypes::CompareGreaterThan:
	case ASTTypes::CompareLessThanEqual:
	case ASTTypes::CompareGreaterThanEqual:
	{
		abort();
		return GetValueTypeOfNode(node->left);
		// Should be a boolean (or int), but right now comparisions don't store a value. 
		// They are only used for the conditional jump instruction

		break;
	}
	case ASTTypes::And:
	case ASTTypes::Or:
	case ASTTypes::Not:
	{
		abort();
		// Should be a boolean (or int), but right now comparisions don't store a value. 
		// They are only used for the conditional jump instruction
	}

	case ASTTypes::IntLiteral:
		return ValueTypes::Integer;
	case ASTTypes::DoubleLiteral:
		return ValueTypes::Float;
	case ASTTypes::StringLiteral:
		return ValueTypes::String;

	case ASTTypes::Bool:
	case ASTTypes::ArrayType:
	case ASTTypes::FunctionType:
	case ASTTypes::ObjectType:
	{
		abort();
		break;
	}
	case ASTTypes::Variable:
	{
		const std::string& variableName = node->stringValue;
		if (!m_SymbolTable.Has(variableName))
		{
			MakeError("Variable '" + variableName + "' has not been declared in this scope");
			return ValueTypes::Void;
		}

		auto symbol = m_SymbolTable.Lookup(variableName);
		if (symbol->m_SymbolType == SymbolType::Variable)
			return symbol->m_StorableValueType->id;
		else if (symbol->m_SymbolType == SymbolType::Function)
			return ValueTypes::Integer;
		else
		{
			MakeError("Symbol '" + variableName + "' is not a variable");
			return ValueTypes::Void;
		}

		break;
	}

	case ASTTypes::Add:
	case ASTTypes::Subtract:
	case ASTTypes::Multiply:
	case ASTTypes::Divide:
	{
		// Recursivly perform the operations, do the inner ones first
		if (node->right->IsMathOperator())
		{
			uint16_t right = GetValueTypeOfNode(node->right);
			uint16_t left = GetValueTypeOfNode(node->left);

			if (left != right)
			{
				MakeError("Non-match matching types for " + node->right->ToString(false));
				return ValueTypes::Void;
			}
			// right is the same as left
			return right;
		}
		if (node->left->IsMathOperator())
		{
			ValueType left = GetValueTypeOfNode(node->left);
			ValueType right = GetValueTypeOfNode(node->right);

			if (left != right)
			{
				MakeError("Non-match matching types for " + node->right->ToString(false));
				return ValueTypes::Void;
			}
			// right is the same as left
			return right;
		}

		ValueType left = GetValueTypeOfNode(node->left);
		ValueType right = GetValueTypeOfNode(node->right);

		if (left != right)
		{
			MakeError("Non-match matching types for " + node->right->ToString(false));
			return ValueTypes::Void;
		}
		// right is the same as left
		return right;

	}
	case ASTTypes::PostIncrement:
	case ASTTypes::PostDecrement:
	{
		// Ensure that the increment is alone on the line. In the case of a for-statement, it is only allowed at the "action" spot
		if (node->parent->type == ASTTypes::ForStatement)
			assert(node->parent->arguments[2] == node);
		else
			assert(node->parent->type == ASTTypes::Scope);

		const std::string& variableName = node->left->stringValue;
		if (!m_SymbolTable.HasAndIs(variableName, SymbolType::Variable))
		{
			MakeError("Variable '" + variableName + "' has not been declared in this scope");
			return ValueTypes::Void;
		}

		return m_SymbolTable.Lookup(variableName)->m_StorableValueType->id;
	}
	case ASTTypes::PreIncrement:
	case ASTTypes::PreDecrement:
		abort();
	case ASTTypes::FunctionCall:
	{
		const std::string& functionName = node->stringValue;

		assert(m_SymbolTable.HasAndIs(functionName, SymbolType::Function));

		SymbolTable::FunctionSymbol* symbol = (SymbolTable::FunctionSymbol*)m_SymbolTable.Lookup(functionName);
		return symbol->m_StorableValueType->Resolve().id;

		//// Check user defined functions
		//if (!m_Context.HasFunction(functionName))
		//{
		//	MakeError("Function '" + functionName + "' has not been defined");
		//	return ValueTypes::Void;
		//}

		//auto& function = m_Context.GetFunction(functionName);
		//return function.m_ReturnType;
	}

	case ASTTypes::PropertyAccess:
	{
		const std::string& propertyName = node->right->stringValue;

		ValueType lhs = GetValueTypeOfNode(node->left);

		const std::string typeName = m_TypeTable.GetEntryFromId(lhs).name;

		auto classSymbol = m_SymbolTable.LookupClassByType(lhs);
		if (!classSymbol)
		{
			MakeError("Failed to do property access on lhs with type " + std::to_string(lhs) + ", it is not a class");
			return ValueTypes::Void;
		}

		SymbolTable::VariableSymbol* prop = (SymbolTable::VariableSymbol*)classSymbol->m_MemberVariables->Lookup(propertyName);
		SymbolTable::FunctionSymbol* method = (SymbolTable::FunctionSymbol*)classSymbol->m_Methods->Lookup(propertyName);
		if (!prop && !method)
		{
			MakeError("Symbol '" + propertyName + "' doesn't exist on type '" + typeName + "'");
			return ValueTypes::Void;
		}



		// If trying to call the property. Make sure it is not a class member and a valid method.
		// Because the object the method is on is on the stack from a previous iteration, just compile the function as the 
		// 'this' parameter is already there and a method is a function (with this as first parameter).
		if (node->right->type == ASTTypes::FunctionCall)
		{
			if (prop)
			{
				MakeError("Symbol '" + propertyName + "' on '" + classSymbol->m_Name + "' is not a method, but a " + SymbolTypeToString(prop->m_SymbolType));
				return ValueTypes::Void;
			}

			if (!method)
			{
				MakeError("Symbol '" + propertyName + "' on '" + classSymbol->m_Name + "' doesn't exist");
				return ValueTypes::Void;
			}

			return method->m_StorableValueType->id;
		}

		return prop->m_StorableValueType->id;

		break;
	}

	case ASTTypes::Empty:
		return ValueTypes::Void;

	default:
	{
		std::cout << "Unhandled node type " << node->ToString(false) << "\n";
		abort();
	}
	}
}

void BytecodeCompiler::MakeError(std::string error)
{
	m_Error = error;
}

}