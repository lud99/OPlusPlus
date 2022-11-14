#pragma once

#include <fstream>
#include <iostream>

#include "BytecodeCompiler.h"
#include "BytecodeFunctions.h"

#include "../../Utils.hpp"

#include "Heap.h"

ValueTypes NodeVariableTypeToValueType(ASTNode* n)
{
	assert(n->type == ASTTypes::VariableType);

	if (n->stringValue == "int") return ValueTypes::Integer;
	if (n->stringValue == "float") return ValueTypes::Float;
	if (n->stringValue == "double") return ValueTypes::Float;
	if (n->stringValue == "string") return ValueTypes::StringConstant;
	//if (n->stringValue == "char") return ValueTypes::Char;
	//if (n->stringValue == "array") return ValueTypes::Array;
	//if (n->stringValue == "function") return ValueTypes::Function;
	//if (n->stringValue == "object") return ValueTypes::Object;

	return ValueTypes::Void;
}
//
//ValueTypes NodeTypeToValueType(ASTNode* n)
//{
//	if (n->type == ASTTypes::IntLiteral) return ValueTypes::Number;
//	if (n->type == ASTTypes::String) return ValueTypes::String;
//	if (n->type == ASTTypes::Null) return ValueTypes::Empty;
//	//if (n->stringValue == "array") return ValueTypes::Array;
//	//if (n->stringValue == "function") return ValueTypes::Function;
//
//	return ValueTypes::Empty;
//}

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

uint32_t BytecodeConverterContext::AddStringConstant(ConstantsPool& constants, std::string string)
{
	char* stringConstant = CopyString(string.c_str());

	// If an index already exists for this string
	if (m_IndiciesForStringConstants.count(string) == 1)
		return m_IndiciesForStringConstants[string];

	// Otherwise, create the string and use the next free slot
	uint32_t index = constants.m_FreeStringSlot++;
	m_IndiciesForStringConstants[string] = index;

	constants.m_StringConstants[index] = HeapEntry(0, index, stringConstant);

	return index;
}

BytecodeConverterContext::Variable BytecodeConverterContext::GetVariable(std::string& variableName)
{
	// If an index already exists for this variable
	if (m_Variables.count(variableName) == 1)
		return m_Variables[variableName];

	return Variable();
}

bool BytecodeConverterContext::CreateVariableIndex(std::string& variableName, ValueTypes type, int& index)
{
	// If an index already exists for this variable
	if (m_Variables.count(variableName) == 1)
	{
		index = m_Variables[variableName].m_Index;
		return false;
	}

	// Use the next free slot
	index = m_NextFreeVariableIndex++;
	m_Variables[variableName] = Variable(index, variableName, type);
	return true;
}

bool BytecodeConverterContext::CreateVariableIndex(Variable& variable)
{
	// If an index already exists for this variable
	if (m_Variables.count(variable.m_Name) == 1)
	{
		variable.m_Index = m_Variables[variable.m_Name].m_Index;
		return false;
	}

	// Use the next free slot
	variable.m_Index = m_NextFreeVariableIndex++;
	m_Variables[variable.m_Name] = Variable(variable.m_Index, variable.m_Name, variable.m_Type, variable.m_IsGlobal);
	return true;
}

int BytecodeConverterContext::CreateVariableIndex(std::string& variableName, ValueTypes type)
{
	int index = 0;
	CreateVariableIndex(variableName, type, index);
	return index;
}


void BytecodeCompiler::PreCompileFunction(ASTNode* node)
{
	BytecodeConverterContext::Variable variable(-1, node->left->arguments[1]->stringValue, ValueTypes::Integer/*ValueTypes::FunctionPointer*/);

	m_Context.CreateVariableIndex(variable.m_Name, variable.m_Type, variable.m_Index);
}

int BytecodeCompiler::CompileFunction(ASTNode* node, std::vector<Instruction>& instructions)
{
	PreCompileFunction(node);
	BytecodeConverterContext::Variable variable = m_Context.GetVariable(node->left->arguments[1]->stringValue);
	if (variable.m_Index == -1)
	{
		Throw("Function " + variable.m_Name + " has not been declared somehow");
		return -1;
	}

	// A function declaration has to be global
	if (m_CurrentScope != 0)
	{
		Throw("Function " + variable.m_Name + " is not in the global scope");
		return -1;
	}
	variable.m_IsGlobal = true;

	m_CurrentScope++;

	int functionStart = instructions.size() + 1;

	//std::vector<Instruction> function;

	instructions.emplace_back(Opcodes::skip_function);
	instructions.push_back(Instruction(Opcodes::create_function_frame).Arg((int)m_CurrentScope));

	// Reset export variable so the local function variables don't get exported
	bool shouldExport = m_Context.m_ShouldExportVariable;
	m_Context.m_ShouldExportVariable = false;

	if (node->right)
	{
		//Instructions body;

		BytecodeConverterContext initialContext = m_Context;

		// Compile arguments
		if (node->left->arguments.size() >= 3)
		{
			for (int i = 2; i < node->left->arguments.size(); i++)
			{
				ASTNode* n = node->left->arguments[i];
				
				if (n->type == ASTTypes::VariableDeclaration)
				{
					Compile(n, instructions, false);
					instructions.erase(instructions.end() - 2);
				}
			}
		}

		Compile(node->right, instructions, false);

		m_Context.m_Variables = initialContext.m_Variables;

		// Make the function global in case of it being marked as threaded
		//if (m_Context.m_IsThreadedFunction)
		//	m_Constants.m_GlobalFunctions[variable.m_Name] = body;

		//function = ConcatVectors(function, body);

	}

	m_Context.m_ShouldExportVariable = shouldExport;

	// Some function defenitions has a manual return statement, but if none exists then create one automatically
	if (instructions.back().m_Type != Opcodes::ret && instructions.back().m_Type != Opcodes::ret_void)
	{
		//if (m_Context.m_IsThreadedFunction)
			//function.emplace_back(Opcodes::thread_end);
		//else
		instructions.emplace_back(Opcodes::ret_void);
	}

	// Function before
	instructions[functionStart - 1].m_Arguments[0] = InstructionArgument(int(instructions.size()), ValueTypes::Integer);

	// Insert the function instructions at the start of the bytecode
	//instructions = ConcatVectors(instructions, function);

	if (m_Context.m_ShouldExportVariable)
		instructions.push_back(Instruction(Opcodes::load).Arg(m_Context.m_ModuleIndex));

	// The variable for the function stores the adress of the function
	instructions.push_back(Instruction(Opcodes::push_functionpointer).Arg(functionStart));

	instructions.push_back(Instruction(Opcodes::store)
		.Arg(variable.m_Index)
		.Arg((int)variable.m_Type)
		.Arg(variable.m_Name)
		.Arg(variable.m_IsGlobal));

	if (m_Context.m_ShouldExportVariable)
	{
		instructions.push_back(Instruction(Opcodes::load).Arg(variable.m_Index));
		instructions.push_back(Instruction(Opcodes::store_property).Arg(variable.m_Name));
	}

	m_CurrentScope--;

	return functionStart;
}

void BytecodeCompiler::PreCompileAnonymousFunction(ASTNode* node)
{
	/*BytecodeConverterContext::Variable variable = m_Context.GetVariable(node->left->stringValue);
	if (variable.m_Index == -1)
	{
		m_Context.CreateVariableIndex(node->left->stringValue, ValueTypes::FunctionPointer);
	}*/
}

int BytecodeCompiler::CompileAnonymousFunction(ASTNode* node, std::vector<Instruction>& instructions)
{
	PreCompileAnonymousFunction(node);
	m_CurrentScope++;

	int functionStart = instructions.size() + 1;

	instructions.emplace_back(Opcodes::skip_function);
	instructions.push_back(Instruction(Opcodes::create_function_frame).Arg(m_CurrentScope));

	if (node->right) Compile(node->right, instructions, false);

	// Some function defenitions has a manual return statement, but if none exists then create one automatically
	if (instructions.back().m_Type != Opcodes::ret && instructions.back().m_Type != Opcodes::ret_void)
		instructions.emplace_back(Opcodes::ret_void);

	instructions[functionStart - 1].m_Arguments[0] = InstructionArgument((int)instructions.size(), ValueTypes::Integer);

	// The variable for the function stores the adress of the function
	instructions.push_back(Instruction(Opcodes::push_functionpointer).Arg(functionStart));

	m_CurrentScope--;

	return functionStart;
}

void BytecodeCompiler::CompileAssignment(ASTNode* node, std::vector<Instruction>& instructions)
{
	std::string propertyName = "";

	BytecodeConverterContext::Variable variable(-1, node->left->stringValue/*, NodeTypeToValueType(node->right)*/);

	bool assigningToProperty = false;

	// Resolve if variable decleration on the left. Should create a new variable
	if (node->left->type == ASTTypes::VariableDeclaration)
	{
		variable.m_Name = node->left->right->stringValue;
		variable.m_Type = NodeVariableTypeToValueType(node->left->left);

		bool isGlobal = m_CurrentScope == 0;
		variable.m_IsGlobal = isGlobal;

		// Create a variable and store it to the module object
		if (m_Context.m_ShouldExportVariable)
			ExportVariable(node, variable, instructions);
		else
		{
			if (!m_Context.CreateVariableIndex(variable))
				return Throw("Variable " + variable.m_Name + " has already been declared");

			// Error if the type of the value on the right couldn't be evaluated, because it's not a constant
			if (variable.m_Type == ValueTypes::Void && node->right->type != ASTTypes::Null)
				return Throw("Variable " + variable.m_Name + " doesn't have a type");
		}
	}
	// Assigning to a property
	else if (node->left->type == ASTTypes::PropertyAccess)
	{
		assigningToProperty = true;
		propertyName = node->left->right->stringValue;

		ASTNode* n = node->left;
		while (true)
		{
			if (n->left == nullptr)
			{
				variable.m_Name = n->stringValue;
				break;
			}

			n = n->left;
		}

		// Assigning to an existing variable
		std::string name = variable.m_Name;
		variable = m_Context.GetVariable(variable.m_Name);
		if (variable.m_Index == -1)
			return Throw("Variable " + name + " cannot be assigned to, because it hasn't been declared");

		Compile(node->left, instructions);
		instructions.pop_back();
	}
	else
	{
		// Assigning to an existing variable
		std::string name = variable.m_Name;
		variable = m_Context.GetVariable(variable.m_Name);
		if (variable.m_Index == -1)
		{
			//if (node->right->type == ASTTypes::FunctionDefinition)
			//	return Throw("Cannot assign a function declaration with a name, to a variable");

			////if (node->right->type != ASTTypes::AnonymousFunction)
			//	//return Throw("Variable " + name + " cannot be assigned to, because it hasn't been declared");

			//// Function declarations creates the variable
			//variable.m_Type = ValueTypes::FunctionPointer;
			//if (!m_Context.CreateVariableIndex(variable.m_Name, variable.m_Type, variable.m_Index))
			//	return Throw("Variable " + name + " has already been declared");
		}
	}

	// Cant export normal assignments
	if (node->left->type != ASTTypes::VariableDeclaration && m_Context.m_ShouldExportVariable)
		return Throw("Can only export variable declarations, not normal assignments");

	Compile(node->right, instructions);

	if (!assigningToProperty)
	{
		instructions.push_back(Instruction(Opcodes::store)
			.Arg(variable.m_Index)
			.Arg((int)variable.m_Type)
			.Arg(variable.m_Name)
			.Arg(variable.m_IsGlobal));
	}
	else
	{
		instructions.push_back(Instruction(Opcodes::store_property).Arg(node->left->right->stringValue));
	}

	// Store the variable to the module, but also as a normal variable
	if (m_Context.m_ShouldExportVariable)
	{
		instructions.push_back(Instruction(Opcodes::load).Arg(variable.m_Index));
		instructions.push_back(Instruction(Opcodes::store_property).Arg(variable.m_Name));
	}
}

void BytecodeCompiler::ExportVariable(ASTNode* node, BytecodeConverterContext::Variable& variable, std::vector<Instruction>& instructions)
{
	assert(m_Context.m_ShouldExportVariable);

	if (node->type == ASTTypes::Assign)
	{
		if (node->left->type != ASTTypes::VariableDeclaration)
			return Throw("Cannot export an assignment to existing variable '" + variable.m_Name + "', only declarations");

	}
	else if (node->type == ASTTypes::VariableDeclaration)
	{

	}
	else {
		return Throw("Can only export variables");
	}

	if (!m_Context.CreateVariableIndex(variable.m_Name, variable.m_Type, variable.m_Index))
		return Throw("Variable " + variable.m_Name + " has already been exported or declared");

	instructions.push_back(Instruction(Opcodes::load).Arg(m_Context.m_ModuleIndex));
}

bool ResultCanBeDiscarded(ASTNode* node)
{
	assert(node->parent != nullptr);

	return (node->parent->type == ASTTypes::Scope ||
		node->parent->type == ASTTypes::ProgramBody ||
		node->parent->type == ASTTypes::ForStatement);
}

BytecodeCompiler::BytecodeCompiler()
{
}

void BytecodeCompiler::Compile(ASTNode* node, std::vector<Instruction>& instructions, bool canCreateScope, bool canMakeVariablesLocal)
{
	auto ResolveCorrectMathInstruction = [](ASTNode* n, bool reverse = false)
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

		if (n->type == ASTTypes::ToThePower)
			return (reverse ? Opcodes::pow_rev : Opcodes::pow);
		if (n->type == ASTTypes::Modulus)
			return (reverse ? Opcodes::mod_rev : Opcodes::mod);
		if (n->type == ASTTypes::Xor)
			return (reverse ? Opcodes::xr_rev : Opcodes::xr);

		abort();

		return Opcodes::no_op;
	};

	ASTNode* left = node->left;
	ASTNode* right = node->right;

	if (m_Error != "")
		return;

	switch (node->type)
	{
	case ASTTypes::ModuleBody:
	{
		return Compile(node->left, instructions, false);
	}
	case ASTTypes::ProgramBody:
	{
		return Compile(node->left, instructions, false);
	}
	case ASTTypes::Scope:
	{
		BytecodeConverterContext initialContext = m_Context;

		if (canCreateScope)
		{
			m_CurrentScope++;
			instructions.push_back(Instruction(Opcodes::create_scope_frame).Arg(m_CurrentScope));
		}

		for (int i = 0; i < node->arguments.size(); i++)
		{
			ASTNode* n = node->arguments[i];
			/*if (n->type == ASTTypes::FunctionDefinition)
				continue;
			if (IsAnonFunctionDef(n))
				continue;*/

			Compile(n, instructions);
		}

		// Precompile functions so that they can be accessed before they are actually declared
		//for (int i = 0; i < node->arguments.size(); i++)
		//{
		//	ASTNode* n = node->arguments[i];
		//	if (n->type == ASTTypes::FunctionDefinition)
		//		PreCompileFunction(n);
		//	if (IsAnonFunctionDef(n))
		//		PreCompileAnonymousFunction(n);
		//}
		//	
		//// Compile everything else
		//for (int i = 0; i < node->arguments.size(); i++)
		//{
		//	ASTNode* n = node->arguments[i];
		//	if (n->type == ASTTypes::FunctionDefinition)
		//		continue;
		//	if (IsAnonFunctionDef(n))
		//		continue;

		//	Compile(n, instructions);
		//}

		//// Finally compile the function body
		//std::vector<Instruction> functions;
		//for (int i = 0; i < node->arguments.size(); i++)
		//{
		//	ASTNode* n = node->arguments[i];
		//	if (IsAnonFunctionDef(n))
		//		Compile(n, functions);
		//	else if (n->type == ASTTypes::FunctionDefinition)
		//		Compile(n, functions);
		//}
		//instructions = ConcatVectors(functions, instructions);

		// Reset so the local variables created in the scope are discarded
		if (canMakeVariablesLocal)
		{
			m_Context.m_Variables = initialContext.m_Variables;
			//m_Context.m_NextFreeVariableIndex = initialContext.m_NextFreeVariableIndex;
		}

		if (canCreateScope)
		{
			instructions.push_back(Instruction(Opcodes::pop_scope_frame).Arg(m_CurrentScope));
			m_CurrentScope--;
		}

		break;
	}

	case ASTTypes::IfStatement:
	{
		// Format: check condition, jmp_if_false, scope code
		// The if statement is inversed, so if the condition is true then it just increments the pc and keeps running.
		// If it's false, then it jumps over the code for that statement 

		// Create bytecode for the condition
		Compile(node->left, instructions);

		int positionOfJumpOpcode = instructions.size();

		// Replace the dummy jump opcode with the proper one
		int positionOfEndOfScope = -1;
		instructions.emplace_back(Opcodes::jmp_if_false);

		// Create bytecode for the scope
		Compile(node->right, instructions);

		positionOfEndOfScope = instructions.size(); // Position of the end of the scope to run

		// Insert the jump opcode before the scope bytecode
		instructions[positionOfJumpOpcode] = Instruction(Opcodes::jmp_if_false).Arg(positionOfEndOfScope);

		break;
	}

	case ASTTypes::Else:
	{
		// Format: check condition, jmp_if_false, scope code
		// The if statement is inversed, so if the condition is true then it just increments the pc and keeps running.
		// If it's false, then it jumps over the code for that statement 

		int endOfElseIfLadder = 0;
		int positionOfJumpOutOpcodeLeft = -1;
		int positionOfJumpOutOpcodeRight = -1;

		auto CreateCodeForIfStatements = [&](ASTNode* childNode)
		{
			// Create bytecode for the condition
			Compile(childNode->left, instructions);

			int positionOfJumpOpcode = instructions.size();

			// Insert a dummy jump opcode that will be removed
			instructions.emplace_back(Opcodes::jmp_if_false);

			// Create bytecode for the scope
			Compile(childNode->right, instructions);

			int positionOfEndOfScope = instructions.size() + 1; // Position of the end of the scope to run

			// Replace the dummy jump opcode with the proper one
			instructions[positionOfJumpOpcode] = Instruction(Opcodes::jmp_if_false).Arg(positionOfEndOfScope);

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

		endOfElseIfLadder = instructions.size();

		if (positionOfJumpOutOpcodeLeft != -1) instructions[positionOfJumpOutOpcodeLeft] = Instruction(Opcodes::jmp).Arg(endOfElseIfLadder);
		if (positionOfJumpOutOpcodeRight != -1) instructions[positionOfJumpOutOpcodeRight] = Instruction(Opcodes::jmp).Arg(endOfElseIfLadder);

		break;
	}

	case ASTTypes::WhileStatement:
	{
		// Format: check condition, jmp_if_false, scope code
		// The if statement is inversed, so if the condition is true then it just increments the pc and keeps running.
		// If it's false, then it jumps over the code for that statement 
		m_CurrentScope++;

		instructions.push_back(Instruction(Opcodes::create_scope_frame).Arg(m_CurrentScope));

		int positionForCondition = instructions.size();

		// Generate bytecode for the scope, but only to know the amount of instructions
		std::vector<Instruction> tempInst = instructions;
		BytecodeCompiler temp;
		temp.m_Context = m_Context;
		temp.m_Constants = m_Constants;

		// Temp compilations
		// Main scope
		temp.Compile(node->left, tempInst);
		temp.Compile(node->right, tempInst);
		int scopeEnd = tempInst.size() + 2; // The position of the pop_frame opcode, which is the end of the scope

		m_Context.m_LoopInfo.m_End = scopeEnd;
		m_Context.m_LoopInfo.m_Reset = positionForCondition;
		m_Context.m_LoopInfo.m_BodyDepth = m_CurrentScope + 1;

		// Create bytecode for the condition
		Compile(node->left, instructions);

		// Insert a dummy jump opcode that will be removed
		instructions.push_back(Instruction(Opcodes::jmp_if_false).Arg(scopeEnd));

		// Create bytecode for the scope
		Compile(node->right, instructions);

		// Jump back to the condition
		instructions.push_back(Instruction(Opcodes::jmp).Arg(positionForCondition));
		instructions.push_back(Instruction(Opcodes::pop_scope_frame).Arg(m_CurrentScope));

		// Reset the loop information
		m_Context.m_LoopInfo = BytecodeConverterContext::LoopInfo();

		m_CurrentScope--;

		break;
	}

	case ASTTypes::ForStatement:
	{
		// Format: initialization, check condition, jmp_if_false, scope code
		// The if statement is inversed, so if the condition is true then it just increments the pc and keeps running.
		// If it's false, then it jumps over the code for that statement 
		m_CurrentScope++;
		instructions.push_back(Instruction(Opcodes::create_scope_frame).Arg(m_CurrentScope));

		// Create bytecode for the initialization
		Compile(node->arguments[0], instructions);

		int positionForCondition = instructions.size();

		// Create bytecode for the condition
		Compile(node->arguments[1], instructions);

		int positionOfJumpOpcode = instructions.size();

		// Insert a dummy jump opcode that will be removed
		instructions.emplace_back(Opcodes::jmp_if_false);

		// Generate bytecode for the scope, but only to know the amount of instructions
		BytecodeConverterContext::LoopInfo m_OriginalLoopInfo = m_Context.m_LoopInfo;
		std::vector<Instruction> tempInst = instructions;
		BytecodeCompiler temp;
		temp.m_Context = m_Context;
		temp.m_Context.m_LoopInfo.m_InLoop = true;
		temp.m_Constants = m_Constants;

		// Temp compilations
		// Main scope
		temp.Compile(node->right, tempInst);
		int loopResetPosition = tempInst.size() - 2; // Subtracting two for some reason, maybe to account for some future instructions?

		// Increment part
		temp.Compile(node->arguments[2], tempInst);
		int scopeEnd = tempInst.size() - 1; // Subtracting 1 because of the pop_frame opcode, which is the end of the scope

		m_Context.m_LoopInfo.m_InLoop = true;
		m_Context.m_LoopInfo.m_End = scopeEnd;
		m_Context.m_LoopInfo.m_Reset = loopResetPosition;
		m_Context.m_LoopInfo.m_BodyDepth = m_CurrentScope + 1;

		// Actually create bytecode for the scope
		Compile(node->right, instructions, false);

		// Add the increment bytecode
		Compile(node->arguments[2], instructions);

		// Replace the dummy jump opcode with the proper one
		instructions[positionOfJumpOpcode] = Instruction(Opcodes::jmp_if_false).Arg(scopeEnd);

		// Jump back to the condition
		instructions.push_back(Instruction(Opcodes::jmp).Arg(positionForCondition));

		instructions.push_back(Instruction(Opcodes::pop_scope_frame).Arg(m_CurrentScope));

		// Reset the loop information
		m_Context.m_LoopInfo = m_OriginalLoopInfo;

		m_CurrentScope--;

		break;
	}

	case ASTTypes::FunctionDefinition:
	{
		CompileFunction(node, instructions);
		break;
	}

	case ASTTypes::VariableDeclaration:
	{
		bool isGlobalVariable = m_CurrentScope == 0;
		BytecodeConverterContext::Variable variable(-1, right->stringValue, NodeVariableTypeToValueType(left), isGlobalVariable);

		if (m_Context.m_ShouldExportVariable)
			ExportVariable(node, variable, instructions);
		else
			m_Context.CreateVariableIndex(variable);

		// Assign a default value to it
		if (variable.m_Type == ValueTypes::Integer) // 0
			instructions.push_back(Instruction(Opcodes::push_number).Arg(int(0)));
		if (variable.m_Type == ValueTypes::Float) // 0
			instructions.push_back(Instruction(Opcodes::push_number).Arg(double(0.0), ValueTypes::Float));
		else if (variable.m_Type == ValueTypes::String) // ""
			instructions.push_back(Instruction(Opcodes::push_stringconst).Arg(m_Context.AddStringConstant(m_Constants, "")));
		//else if (variable.m_Type == ValueTypes::Array) // []
		//	instructions.emplace_back(Opcodes::array_create_empty);
		//else if (variable.m_Type == ValueTypes::Object) // {}
		//	instructions.emplace_back(Opcodes::object_create_empty);

		instructions.push_back(Instruction(Opcodes::store).
			Arg(variable.m_Index)
			.Arg((int)variable.m_Type)
			.Arg(variable.m_Name)
			.Arg(variable.m_IsGlobal));

		// Store the variable value to the module property, but also as a variable
		if (m_Context.m_ShouldExportVariable)
		{
			instructions.push_back(Instruction(Opcodes::load).Arg(variable.m_Index));
			instructions.push_back(Instruction(Opcodes::store_property).Arg(variable.m_Name));
		}

		break;
	}

	case ASTTypes::Assign:
	{
		CompileAssignment(node, instructions);

		break;
	}

	case ASTTypes::PostDecrement:
	case ASTTypes::PostIncrement:
	{
		// Load the variable
		int index = m_Context.GetVariable(node->left->stringValue).m_Index;

		if (index == -1)
			return Throw("Variable " + node->stringValue + " doesn't exist");

		if (node->type == ASTTypes::PostIncrement)
			instructions.push_back(Instruction(Opcodes::post_inc, ResultCanBeDiscarded(node)).Arg(index));
		if (node->type == ASTTypes::PostDecrement)
			instructions.push_back(Instruction(Opcodes::post_dec, ResultCanBeDiscarded(node)).Arg(index));

		break;
	}

	case ASTTypes::Add:
	case ASTTypes::Subtract:
	case ASTTypes::Multiply:
	case ASTTypes::Divide:
	case ASTTypes::Xor:
	case ASTTypes::ToThePower:
	case ASTTypes::Modulus:
	{
		// Recursivly perform the operations, do the inner ones first
		if (right->IsMathOperator())
		{
			Compile(right, instructions);
			Compile(left, instructions);

			instructions.emplace_back(ResolveCorrectMathInstruction(node));

			return;
		}
		if (left->IsMathOperator())
		{
			Compile(left, instructions);
			Compile(right, instructions);

			instructions.emplace_back(ResolveCorrectMathInstruction(node, true));

			return;
		}

		Compile(right, instructions);
		Compile(left, instructions);

		if (node->IsMathOperator())
			instructions.emplace_back(ResolveCorrectMathInstruction(node));

		break;
	}

	case ASTTypes::PropertyAccess:
	{
		if (right->type == ASTTypes::FunctionCall)
		{
			// Compile the function call
			// Push all the arguments onto the stack backwards
			for (int i = right->arguments.size() - 1; i >= 0; i--)
			{
				Compile(right->arguments[i], instructions);
			}

			Compile(left, instructions); // Source object
			instructions.push_back(Instruction(Opcodes::load_property).Arg(right->stringValue));

			instructions.push_back(Instruction(Opcodes::call).Arg(right->arguments.size()).Arg(right->stringValue));
		}
		else
		{
			Compile(left, instructions); // Source object
			instructions.push_back(Instruction(Opcodes::load_property).Arg(right->stringValue));
		}

		break;
	}


	case ASTTypes::PropertyAssign:
	{
		Compile(left, instructions); // Key
		Compile(right, instructions); // Value

		break;
	}

	case ASTTypes::FunctionCall:
	{
		// Push all the arguments onto the stack

		// Push the arguments backwards
		for (int i = node->arguments.size() - 1; i >= 0; i--)
		{
			Compile(node->arguments[i], instructions);
		}

		// Calling native functions
		if (BytecodeFunctions::GetFunctionByName(node->stringValue))
		{
			instructions.push_back(Instruction(Opcodes::call_native, ResultCanBeDiscarded(node)).Arg(node->stringValue).Arg(node->arguments.size()));
			break;
		}

		BytecodeConverterContext::Variable variable = m_Context.GetVariable(node->stringValue);
		if (variable.m_Index == -1)
			return Throw("Function " + node->stringValue + " not defined");

		instructions.push_back(Instruction(Opcodes::load).Arg(variable.m_Index));
		instructions.push_back(Instruction(Opcodes::call).Arg(node->arguments.size()).Arg(node->stringValue));

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
		if (!m_Context.m_LoopInfo.m_InLoop)
			return Throw("A break statement needs to be inside a loop");

		instructions.push_back(Instruction(Opcodes::jmp).Arg(m_Context.m_LoopInfo.m_End));
		break;
	}
	case ASTTypes::Continue:
	{
		if (!m_Context.m_LoopInfo.m_InLoop)
			return Throw("A continue statement needs to be inside a loop");

		//instructions.push_back(Instruction(Opcodes::pop_scope_frame).Arg(m_Context.m_LoopInfo.m_BodyDepth));
		instructions.push_back(Instruction(Opcodes::jmp).Arg(m_Context.m_LoopInfo.m_Reset));
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

	case ASTTypes::And:
	case ASTTypes::Or:
	{
		Compile(node->right, instructions);
		Compile(node->left, instructions);

		if (node->type == ASTTypes::And)
			instructions.emplace_back(Opcodes::logical_and);
		if (node->type == ASTTypes::Or)
			instructions.emplace_back(Opcodes::logical_or);

		break;
	}

	case ASTTypes::Not:
	{
		Compile(node->left, instructions);

		instructions.emplace_back(Opcodes::logical_not);

		break;
	}

	case ASTTypes::Variable:
	{
		int index = m_Context.GetVariable(node->stringValue).m_Index;

		if (index == -1)
			return Throw("Variable " + node->stringValue + " doesn't exist");

		instructions.push_back(Instruction(Opcodes::load).Arg(index));

		break;
	}

	case ASTTypes::Null:
	{
		instructions.push_back(Instruction(Opcodes::push_null, ResultCanBeDiscarded(node)));

		break;
	}

	case ASTTypes::IntLiteral:
	{
		//bool discardValue = node->parent->type == ASTTypes::Scope;

		instructions.push_back(Instruction(Opcodes::push_number).Arg((int)node->numberValue));

		break;
	}

	case ASTTypes::DoubleLiteral:
	{
		//bool discardValue = node->parent->type == ASTTypes::Scope;

		instructions.push_back(Instruction(Opcodes::push_number).Arg(node->numberValue, ValueTypes::Float));

		break;
	}

	case ASTTypes::StringLiteral:
	{
		bool discardValue = node->parent->type == ASTTypes::Scope;

		uint32_t constantsIndex = m_Context.AddStringConstant(m_Constants, node->stringValue);
		instructions.push_back(Instruction(Opcodes::push_stringconst, discardValue).Arg(constantsIndex));

		break;
	}

	case ASTTypes::ListInitializer:
	{
		bool isObject = false;

		for (int i = node->arguments.size() - 1; i >= 0; i--)
		{
			Compile(node->arguments[i], instructions);

			if (node->arguments[i]->type == ASTTypes::PropertyAssign)
				isObject = true;
		}

		bool discardValue = node->parent->type == ASTTypes::Scope;

		// Check if the initializer is an object. Specify how many operand to add to the array, to be able to have nested arrays
		if (isObject)
			instructions.push_back(Instruction(Opcodes::object_create, discardValue).Arg(node->arguments.size()));
		else
			instructions.push_back(Instruction(Opcodes::array_create, discardValue).Arg(node->arguments.size()));

		break;
	}
	}

	if (m_Error != "")
		return;
}

void BytecodeCompiler::Throw(std::string error)
{
	m_Error = error;
}

Instruction& Instruction::Arg(InstructionArgument arg)
{
	m_Arguments[m_ArgsCount++] = arg;
	return *this;
}
Instruction& Instruction::Arg(int arg)
{
	m_Arguments[m_ArgsCount++] = InstructionArgument(arg, ValueTypes::Integer);
	return *this;
}
Instruction& Instruction::Arg(double arg, ValueTypes type)
{
	assert(type == ValueTypes::Float);

	m_Arguments[m_ArgsCount++] = InstructionArgument(arg, ValueTypes::Float);
	return *this;
}
Instruction& Instruction::Arg(std::string arg)
{
	m_Arguments[m_ArgsCount++] = InstructionArgument(arg, ValueTypes::String);

	return *this;
}