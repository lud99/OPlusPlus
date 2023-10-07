#pragma once

#include <fstream>
#include <iostream>

#include "BytecodeCompiler.h"
#include "../Functions.h"

#include "../../Utils.hpp"

#include "Heap.h"

//
//ValueTypes NodeTypeToValueType(ASTNode* n)
//{
//	if (n->typeEntry == ASTTypes::IntLiteral) return ValueTypes::Number;
//	if (n->typeEntry == ASTTypes::String) return ValueTypes::String;
//	if (n->typeEntry == ASTTypes::Null) return ValueTypes::Empty;
//	//if (n->stringValue == "array") return ValueTypes::Array;
//	//if (n->stringValue == "function") return ValueTypes::Function;
//
//	return ValueTypes::Empty;
//}
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

uint16_t ContextConstantsPool::AddAndGetClassIndex(std::string className)
{
	if (m_Classes.count(className) == 1)
		return m_Classes[className];

	uint16_t index = ++m_CurrentFreeSlot;
	m_Classes[className] = index;

	return index;
}

//uint32_t CompilerContext::AddStringConstant(ContextConstantsPool& constants, std::string string)
//{
//	char* stringConstant = CopyString(string.c_str());
//
//	// If an index already exists for this string
//	if (m_IndiciesForStringConstants.count(string) == 1)
//		return m_IndiciesForStringConstants[string];
//
//	// Otherwise, create the string and use the next free slot
//	uint32_t index = constants.m_FreeStringSlot++;
//	m_IndiciesForStringConstants[string] = index;
//
//	//constants.m_StringConstants[index] = HeapEntry(0, index, stringConstant);
//
//	const int a = sizeof(std::string);
//
//	return index;
//}
//
//CompilerContext::Variable CompilerContext::GetVariable(const std::string& variableName)
//{
//	// If an index already exists for this variable
//	if (m_Variables.count(variableName) == 1)
//		return m_Variables[variableName];
//
//	return Variable();
//}
//
//bool CompilerContext::HasVariable(const std::string& variableName)
//{
//	return m_Variables.count(variableName) == 1;
//}
//
//CompilerContext::Function& CompilerContext::CreateFunction(const std::string& functionName, ValueTypes returnType)
//{
//	assert(!HasFunction(functionName));
//
//	uint16_t index = m_ConstantsPool.AddAndGetFunctionReferenceIndex(functionName);
//
//	Function function = { functionName, returnType, index };
//
//	m_Functions[functionName] = function;
//
//	m_CompiledFile->m_Functions[function.m_Index] = function.m_Body;
//
//	return m_Functions[functionName];
//}
//
//CompilerContext::Function& CompilerContext::GetFunction(const std::string& functionName)
//{
//	assert(HasFunction(functionName));
//
//	return m_Functions[functionName];
//}
//
//bool CompilerContext::HasFunction(const std::string& functionName)
//{
//	return m_Functions.count(functionName) == 1;
//}
//
//CompilerContext::Class& CompilerContext::CreateClass(const std::string& className)
//{
//	assert(!HasClass(className));
//
//	uint16_t index = m_ConstantsPool.AddAndGetClassIndex(className);
//
//	Class cls = { className, index };
//	//cls.actualClass = Bytecode::Class(className, index);
//
//	m_Classes[className] = cls;
//
//	//m_CompiledFile->m_Classes[cls.m_Index] = cls.actualClass;
//
//	return m_Classes[className];
//}
//
//CompilerContext::Class& CompilerContext::GetClass(const std::string& className)
//{
//	assert(HasClass(className));
//
//	return m_Classes[className];
//}
//
//bool CompilerContext::HasClass(const std::string& className)
//{
//	return m_Classes.count(className) == 1;
//}
//
//CompilerContext::Function& CompilerContext::CreateMethod(Class& cls, const std::string& methodName, ValueTypes returnType)
//{
//	assert(!HasMethod(cls, methodName));
//
//	uint16_t index = m_ConstantsPool.AddAndGetFunctionReferenceIndex(methodName);
//
//	Function function = { methodName, returnType, index };
//
//	cls.m_Methods[methodName] = function;
//
//	//m_CompiledFile->m_Functions[function.m_Index] = function.m_Body;
//
//	return cls.m_Methods[methodName];
//}
//
//CompilerContext::Function& CompilerContext::GetMethod(Class& cls, const std::string& methodName)
//{
//	assert(HasMethod(cls, methodName));
//
//	return cls.m_Methods[methodName];
//}
//
//bool CompilerContext::HasMethod(Class& cls, const std::string& methodName)
//{
//	return cls.m_Methods.count(methodName) == 1;
//}
//
//bool CompilerContext::CreateVariableIndex(std::string& variableName, ValueTypes typeEntry, int& index)
//{
//	// If an index already exists for this variable
//	if (m_Variables.count(variableName) == 1)
//	{
//		index = m_Variables[variableName].m_Index;
//		return false;
//	}
//
//	// Use the next free slot
//	index = m_NextFreeVariableIndex++;
//	m_Variables[variableName] = Variable(index, variableName, typeEntry);
//	return true;
//}
//
//bool CompilerContext::CreateVariableIndex(Variable& variable)
//{
//	// If an index already exists for this variable
//	if (m_Variables.count(variable.m_Name) == 1)
//	{
//		variable.m_Index = m_Variables[variable.m_Name].m_Index;
//		return false;
//	}
//
//	// Use the next free slot
//	variable.m_Index = m_NextFreeVariableIndex++;
//	m_Variables[variable.m_Name] = Variable(variable.m_Index, variable.m_Name, variable.m_Type, variable.m_IsGlobal);
//	return true;
//}
//
//int CompilerContext::CreateVariableIndex(std::string& variableName, ValueTypes typeEntry)
//{
//	int index = 0;
//	CreateVariableIndex(variableName, typeEntry, index);
//	return index;
//}
//
//CompilerContext::Variable CompilerContext::CreateMemberVariable(Class& cls, Variable& variable)
//{
//	assert(!HasVariable(variable.m_Name));
//
//	// Use the next free slot
//	variable.m_Index = cls.m_CurrentFreeMemberIndex++;
//	m_Variables[variable.m_Name] = variable;
//
//	return m_Variables[variable.m_Name];
//}

BytecodeCompiler::BytecodeCompiler()
{
	auto AddFunction = [&](std::string name, BuiltInFunctions::BuiltInFunctionCallable pointer, ValueType returnType)
	{
		uint16_t id = ++m_ConstantsPool.m_CurrentFreeSlot;

		assert(m_ConstantsPool.m_BuiltInFunctions.count(name) == 0);

		m_ConstantsPool.m_BuiltInFunctions[name] = BuiltInFunctions::Prototype(id, name, pointer, returnType);

		auto function = m_SymbolTable.InsertFunction(0, name, &m_TypeTable.GetEntryFromId(returnType), id);
		// TODO: function->m_Parameters
		function->m_IsBuiltIn = true;
	};

	using namespace BuiltInFunctions;
	AddFunction("print", _print, ValueTypes::Void);
	AddFunction("printf", _printf, ValueTypes::Void);
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

void BytecodeCompiler::CompileClass(ASTNode* node, SymbolTable::ClassSymbol* parentClass)
{
	std::string className = node->stringValue;

	if (m_SymbolTable.Has(className))
		return MakeError("Class " + className + " has already been declared");

	auto& classType = m_TypeTable.GetTypeEntry(className);
	uint16_t classId = m_ConstantsPool.AddAndGetClassIndex(className);

	// A class can be nested inside another. If that is the case that information should be saved to the inner class
	SymbolTable::ClassSymbol* classSymbol = nullptr;
	if (parentClass)
		classSymbol = m_SymbolTable.InsertInnerClass(m_CurrentScope, className, &classType, parentClass);
	else
		classSymbol = m_SymbolTable.InsertClass(m_CurrentScope, className, &classType);

	// Create the object that holds the information used when the bytecode is interpreted
	ClassInstance classInstance(className, classId);

	uint16_t methodId = 0;
	
	// Iterate the declaration
	for (ASTNode* line : node->left->arguments)
	{
		if (line->type == ASTTypes::Class)
		{
			m_CurrentScope++;

			Instructions nestedClass;
			Compile(line, nestedClass);

			m_SymbolTable.Remove(m_CurrentScope);
			m_CurrentScope--;
		}
		else if (line->type == ASTTypes::Assign || line->type == ASTTypes::VariableDeclaration)
		{
			Compile(line, classInstance.m_InternalConstructor);
		}
		else if (line->type == ASTTypes::FunctionDefinition)
		{
			m_CurrentScope++;

			Instructions methodBody;
			auto methodSymbol = CompileClassMethod(line, *classSymbol, methodBody);
			if (HasError())
				return;

			CompiledCallable method = {
				methodSymbol->m_StorableValueType->Resolve().id,
				{},
				methodBody,
				EncodeInstructions(methodBody)
			};

			classInstance.m_Methods[methodId] = method;
			methodId++;

			m_CurrentScope--;
		}
		else
		{
			return MakeError("Unsupported code in class declaration");
		}
	}

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

SymbolTable::FunctionSymbol* BytecodeCompiler::CompileClassMethod(ASTNode* node, SymbolTable::ClassSymbol& classSymbol, Instructions& instructions)
{
	//const std::string& className = node->parent->stringValue;

	//SymbolTable::ClassSymbol& classSymbol = *(SymbolTable::ClassSymbol*)m_SymbolTable.Lookup(className);

	std::string methodName = node->left->arguments[1]->stringValue;
	std::string fullName = classSymbol.m_Name + "::" + methodName;

	if (classSymbol.m_Methods->Has(methodName))
	{
		MakeError("Method " + fullName + " has already been defined");
		return nullptr;
	}

	if (!IsFunctionNameValid(methodName))
	{
		MakeError("Method " + methodName + " is not valid");
		return nullptr;
	}
		

	// A function declaration has to be global
	//if (m_CurrentScope != 0)
		//return MakeError("Function " + functionName + " is not in the global scope");

	// TODO: method return typeEntry
	auto& returnType = m_TypeTable.GetTypeEntry("void");
	auto methodSymbol = classSymbol.m_Methods->InsertMethod(m_CurrentScope, methodName, &returnType);
	
	//auto& method = m_Context.CreateMethod(classDeclaration, methodName, ValueTypes::Void);

	m_CurrentScope++;

	// Compile function
	if (node->right)
	{
		//CompilerContext initialContext = m_Context;

		// TODO: Compile arguments
		/*if (node->left->arguments.size() >= 3)
		{
			for (int i = 2; i < node->left->arguments.size(); i++)
			{
				ASTNode* n = node->left->arguments[i];

				if (n->typeEntry == ASTTypes::VariableDeclaration)
				{
					Compile(n, instructions, false);
					instructions.erase(instructions.end() - 2);
				}
			}
		}*/

		Compile(node->right, instructions, false);
		if (HasError())
			return nullptr;

		//m_Context.m_Variables = initialContext.m_Variables;
	}

	// Some method definitions has a manual return statement, but if none exists then create one automatically
	if (instructions.back().GetOpcode() != Opcodes::ret && instructions.back().GetOpcode() != Opcodes::ret_void)
	{
		instructions.emplace_back(Opcodes::ret_void);
	}


	//classSymbol.m_Methods[methodName] = method;

	m_CurrentScope--;

	return methodSymbol;
}

void BytecodeCompiler::CompileFunction(ASTNode* node)
{
	std::string functionName = node->left->arguments[1]->stringValue;

	if (m_SymbolTable.Has(functionName))
		return MakeError("Function " + functionName + " has already been defined");

	if (!IsFunctionNameValid(functionName))
		return MakeError("Function " + functionName + " is not valid");

	// A function declaration has to be global
	if (m_CurrentScope != 0)
		return MakeError("Function " + functionName + " is not in the global scope");

	uint16_t functionId = m_ConstantsPool.AddAndGetFunctionReferenceIndex(functionName);

	auto functionSymbol = m_SymbolTable.InsertFunction(m_CurrentScope, functionName, &m_TypeTable.GetEntryFromId(ValueTypes::Void), functionId); // TODO: function return type
	//Function function = { functionSymbol };
	//m_Functions[*functionSymbol] = function;

	m_CurrentScope++;

	int functionStart = 0;

	// Reset export variable so the local function variables don't get exported
	//bool shouldExport = m_Context.m_ShouldExportVariable;
	//m_Context.m_ShouldExportVariable = false;

	// Compile function
	Instructions functionBody;
	if (node->right)
	{
		//CompilerContext initialContext = m_Context;

		// TODO: Compile arguments
		/*if (node->left->arguments.size() >= 3)
		{
			for (int i = 2; i < node->left->arguments.size(); i++)
			{
				ASTNode* n = node->left->arguments[i];

				if (n->typeEntry == ASTTypes::VariableDeclaration)
				{
					Compile(n, instructions, false);
					instructions.erase(instructions.end() - 2);
				}
			}
		}*/

		Compile(node->right, functionBody, false);
		if (m_Error != "")
			return;

		//m_Context.m_Variables = initialContext.m_Variables;
	}

	//m_Context.m_ShouldExportVariable = shouldExport;

	// Some function definitions has a manual return statement, but if none exists then create one automatically
	if (functionBody.back().GetOpcode() != Opcodes::ret && functionBody.back().GetOpcode() != Opcodes::ret_void)
	{
		functionBody.emplace_back(Opcodes::ret_void);
	}

	// Function before
	//instructions[functionStart - 1].m_Arguments[0] = InstructionArgument(int(instructions.size()), ValueTypes::Integer);

	// Insert the function instructions at the start of the bytecode
	//instructions = ConcatVectors(instructions, function);

	//if (m_Context.m_ShouldExportVariable)
		//instructions.push_back(Instruction(Opcodes::load).Arg(m_Context.m_ModuleIndex));

	// The variable for the function stores the adress of the function
	//instructions.push_back(Instruction(Opcodes::push_functionpointer).Arg(functionStart));

	/*instructions.push_back(Instruction(Opcodes::store)
		.Arg(variable.m_Index)
		.Arg((int)variable.m_Type)
		.Arg(variable.m_Name)
		.Arg(variable.m_IsGlobal));*/

	//if (m_Context.m_ShouldExportVariable)
	{
		//instructions.push_back(Instruction(Opcodes::load).Arg(variable.m_Index));
		//instructions.push_back(Instruction(Opcodes::store_property).Arg(variable.m_Name));
	}

	CompiledCallable function = {
		functionSymbol->m_StorableValueType->Resolve().id,
		{},
		functionBody,
		EncodeInstructions(functionBody)
	};

	m_CompiledFile.m_Functions[functionId] = function;
	//m_CompiledFile.m_Functions[function.m_Index] = ;

	m_SymbolTable.Remove(m_CurrentScope);
	m_CurrentScope--;
}


void BytecodeCompiler::CompileAssignment(ASTNode* node, Instructions& instructions)
{
	std::string propertyName = "";

	//m_SymbolTable.Insert(m_CurrentScope, )

	//CompilerContext::Variable variable(-1, node->left->stringValue/*, NodeTypeToValueType(node->right)*/);

	bool assigningToProperty = false;
	bool isClassMemberVariable = false;

	std::string variableName = node->left->stringValue;
	ValueType variableType;

	SymbolTable::VariableSymbol* variableSymbol = nullptr;


	auto CompileDeclaration = [&]() {
		variableName = node->left->right->stringValue;
		variableType = m_TypeTable.GetType(node->left->left->stringValue);

		//bool isGlobal = m_CurrentScope == 0;
		//variable.m_IsGlobal = isGlobal;

		// Create a variable and store it to the module object
		//if (m_Context.m_ShouldExportVariable)
			//ExportVariable(node, variable, instructions);
		//else
		//{

		// Typecheck
		ValueType rhs = GetValueTypeOfNode(node->right);
		if (variableType != rhs)
			return MakeError("Variable type of '" + variableName + "' doesn't match type of right hand side");

		// Check if declared inside a class declaration, then it should be a member variable
		if (node->parent && node->parent->parent && node->parent->parent->type == ASTTypes::Class)
		{
			isClassMemberVariable = true;

			const std::string& className = node->parent->parent->stringValue;
			SymbolTable::ClassSymbol& classSymbol = *(SymbolTable::ClassSymbol*)m_SymbolTable.Lookup(className);

			// If member variable has already been declared
			if (classSymbol.m_MemberVariables->HasAndIs(variableName, SymbolType::Variable))
				return MakeError("Member variable " + variableName + " has already been declared");

			variableSymbol = classSymbol.m_MemberVariables->InsertVariable(m_CurrentScope, variableName, &m_TypeTable.GetEntryFromId(variableType));
			return;
		}

		// Ensure it has not been declared before
		if (m_SymbolTable.HasAndIs(variableName, SymbolType::Variable))
			return MakeError("Variable " + variableName + " has already been declared");

		// Invalid type of variable
		if (variableType == ValueTypes::Void)
			return MakeError("Variable " + variableName + " doesn't have a type");

		// Create it
		variableSymbol = m_SymbolTable.InsertVariable(m_CurrentScope, variableName, &m_TypeTable.GetEntryFromId(variableType));
		return;
	};


	// Resolve if variable decleration on the left. Should create a new variable
	if (node->left->type == ASTTypes::VariableDeclaration)
	{
		CompileDeclaration();

		if (HasError()) 
			return;
	}
	// Assigning to a property
	//else if (node->left->type == ASTTypes::PropertyAccess)
	//{
	//	assigningToProperty = true;
	//	propertyName = node->left->right->stringValue;

	//	ASTNode* n = node->left;
	//	while (true)
	//	{
	//		if (n->left == nullptr)
	//		{
	//			variable.m_Name = n->stringValue;
	//			break;
	//		}

	//		n = n->left;
	//	}

	//	// Assigning to an existing variable
	//	std::string name = variable.m_Name;
	//	variable = m_Context.GetVariable(variable.m_Name);
	//	if (variable.m_Index == -1)
	//		return MakeError("Variable " + name + " cannot be assigned to, because it hasn't been declared");

	//	Compile(node->left, instructions);
	//	instructions.pop_back();
	//}
	else
	{
		// Assigning to an existing variable

		if (!m_SymbolTable.HasAndIs(variableName, SymbolType::Variable))
		{
			return MakeError("Undeclared variable " + variableName);
		}
			//if (node->right->typeEntry == ASTTypes::FunctionDefinition)
			//	return MakeError("Cannot assign a function declaration with a name, to a variable");

			////if (node->right->typeEntry != ASTTypes::AnonymousFunction)
			//	//return MakeError("Variable " + name + " cannot be assigned to, because it hasn't been declared");

			//// Function declarations creates the variable
			//variable.m_Type = ValueTypes::FunctionPointer;
			//if (!m_Context.CreateVariableIndex(variable.m_Name, variable.m_Type, variable.m_Index))
			//	return MakeError("Variable " + name + " has already been declared");
		

		//variable = m_Context.GetVariable(variable.m_Name);
		variableSymbol = (SymbolTable::VariableSymbol*)m_SymbolTable.Lookup(variableName);
		variableType = variableSymbol->GetTypeTableEntry().id;

		// Typecheck
		ValueType rhs = GetValueTypeOfNode(node->right);
		if (variableType != rhs)
			return MakeError("Variable type of " + variableName + " doesn't match type of right hand side");
	}

	// Cant export normal assignments
	//if (node->left->typeEntry != ASTTypes::VariableDeclaration && m_Context.m_ShouldExportVariable)
		//return MakeError("Can only export variable declarations, not normal assignments");

	Compile(node->right, instructions);



	if (!isClassMemberVariable)
	{
		instructions.push_back(Instruction(ResolveCorrectStoreOpcode(GetValueTypeOfNode(node->left)), { (uint8_t)variableSymbol->m_Index }));
	}
	else
	{
		
		instructions.push_back(Instruction(Opcodes::store_member, { (uint8_t)variableSymbol->m_Index }));
		//instructions.push_back(Instruction(Opcodes::store_property).Arg(node->left->right->stringValue));
	}

	// Store the variable to the module, but also as a normal variable
	//if (m_Context.m_ShouldExportVariable)
	{
		//instructions.push_back(Instruction(Opcodes::load).Arg(variable.m_Index));
		//instructions.push_back(Instruction(Opcodes::store_property).Arg(variable.m_Name));
	}
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
		const std::string& typeName = left->stringValue;
		const std::string& variableName = right->stringValue;

		if (!m_TypeTable.HasType(typeName))
			return MakeError("Type '" + typeName + "' has not been defined");

		TypeTableEntry& typeEntry = m_TypeTable.GetTypeEntry(typeName);

		//CompilerContext::Variable variable(-1, right->stringValue, m_Context.m_TypeTable.GetType(left->stringValue), isGlobalVariable);
		//if (variable.m_Type == ValueTypes::String)
			//variable.m_Type = ValueTypes::StringConstant;

		//if (m_Context.m_ShouldExportVariable)
			//ExportVariable(node, variable, instructions);
		//else

		//if (m_SymbolTable.Has(variableName))
			//return MakeError("Symbol with name '" + variableName + "' has already been declared");

		SymbolTable::VariableSymbol* variableSymbol = m_SymbolTable.InsertVariable(m_CurrentScope, variableName, &typeEntry);

		// Assign a default value to it
		if (typeEntry.id == ValueTypes::Integer)
			CompileIntLiteral(0, instructions);
		else if (typeEntry.id == ValueTypes::Float)
			CompileDoubleLiteral(0.0f, instructions);
		else if (typeEntry.id == ValueTypes::String)
			CompileStringLiteral("", instructions);

		// Instantiate class
		// TODO: if class, run code for constructor?
		if (typeEntry.type == TypeTableType::Class)
		{
			//assert(.HasClass(typeEntry.name));

			instructions.push_back({ Opcodes::instantiate_class, { (uint8_t)typeEntry.id } });
		}

		instructions.push_back({ ResolveCorrectStoreOpcode(typeEntry.id), {(uint8_t)variableSymbol->m_Index }});

		break;
	}

	case ASTTypes::Assign:
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

	//case ASTTypes::PropertyAccess:
	//{
	//	if (right->typeEntry == ASTTypes::FunctionCall)
	//	{
	//		// Compile the function call
	//		// Push all the arguments onto the stack backwards
	//		for (int i = right->arguments.size() - 1; i >= 0; i--)
	//		{
	//			Compile(right->arguments[i], instructions);
	//		}

	//		Compile(left, instructions); // Source object
	//		instructions.push_back(Instruction(Opcodes::load_property).Arg(right->stringValue));

	//		instructions.push_back(Instruction(Opcodes::call).Arg(right->arguments.size()).Arg(right->stringValue));
	//	}
	//	else
	//	{
	//		Compile(left, instructions); // Source object
	//		instructions.push_back(Instruction(Opcodes::load_property).Arg(right->stringValue));
	//	}

	//	break;
	//}


	//case ASTTypes::PropertyAssign:
	//{
	//	Compile(left, instructions); // Key
	//	Compile(right, instructions); // Value

	//	break;
	//}

	case ASTTypes::FunctionCall:
	{
		// Push all the arguments onto the stack

		// TODO: validation for arguments

		// Push the arguments backwards
		for (int i = node->arguments.size() - 1; i >= 0; i--)
		{
			Compile(node->arguments[i], instructions);
		}

		const std::string& functionName = node->stringValue;

		// TODO: Calling native functions
		//if (m_Context.m_ConstantsPool.(functionName))
		{
			//CompileStringLiteral(functionName, instructions);
			//instructions.push_back({ Opcodes::call_native, ResultCanBeDiscarded(node)).Arg(functionName).Arg(node->arguments.size()));
			//instructions.push_back({ Opcodes::call_native, { (uint8_t)});
			//break;
		}

		if (!m_SymbolTable.HasAndIs(functionName, SymbolType::Function))
			return MakeError("Function " + functionName + " not defined");

		SymbolTable::FunctionSymbol* function = (SymbolTable::FunctionSymbol*)m_SymbolTable.Lookup(functionName);

		//instructions.push_back(Instruction(Opcodes::load).Arg(variable.m_Index));
		//instructions.push_back(Instruction(Opcodes::call, ResultCanBeDiscarded(node)).Arg(node->arguments.size()).Arg(node->stringValue));
		instructions.push_back(Instruction(Opcodes::call, { (uint8_t)function->m_Id })); // TODO: support large index

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

	case ASTTypes::MemberAcessor:
	{
		break;
	}

	case ASTTypes::Variable:
	{
		const std::string& variableName = node->stringValue;
		if (!m_SymbolTable.HasAndIs(variableName, SymbolType::Variable))
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
		if (!m_SymbolTable.HasAndIs(variableName, SymbolType::Variable))
		{
			MakeError("Variable '" + variableName + "' has not been declared in this scope");
			return ValueTypes::Void;
		}

		return m_SymbolTable.Lookup(variableName)->m_StorableValueType->id;
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
		abort();
		//const std::string& functionName = node->stringValue;

		//// Check user defined functions
		//if (!m_Context.HasFunction(functionName))
		//{
		//	MakeError("Function '" + functionName + "' has not been defined");
		//	return ValueTypes::Void;
		//}

		//auto& function = m_Context.GetFunction(functionName);
		//return function.m_ReturnType;
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