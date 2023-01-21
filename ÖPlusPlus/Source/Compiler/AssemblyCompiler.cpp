#include "AssemblyCompiler.h"

#include <iostream>

#define USE_FLOATS 1

std::string FloatToString(float value)
{
	std::string str = std::to_string(value);
	str.replace(str.find(","), 1, ".");

	return str;
}

void CreateNewCallFrame(Section& section)
{
	section.AddLine("");
	section.AddComment("Create call frame");
	section.AddInstruction("push", "ebp");
	section.AddInstruction("mov", "ebp", "esp");
	section.AddLine("");
}
void RestoreOldCallFrame(Section& section)
{
	section.AddLine("");
	section.AddComment("Restore call frame");
	section.AddInstruction("mov", "esp", "ebp");
	section.AddInstruction("pop", "ebp");
	section.AddLine("");
}

bool ResultCanBeDiscarded(ASTNode* node)
{
	assert(node->parent != nullptr);

	return (node->parent->type == ASTTypes::Scope ||
		node->parent->type == ASTTypes::ProgramBody ||
		node->parent->type == ASTTypes::ForStatement);
}

std::string MangleName(std::string str)
{
	return str + std::to_string(rand() % 100);
}

//
//bool IsReservedWord(const std::string& str)
//{
//	const char* reservedWords = {
//		"global",
//		"extern",
//		"CMAIN",
//		"pop",
//		"push",
//		"call",
//
//	}
//}

std::string ComparisonTypeToJumpInstruction(ASTTypes& type)
{
	if (type == ASTTypes::CompareEquals)
		return "jne";
	if (type == ASTTypes::CompareNotEquals)
		return "je";
	if (type == ASTTypes::CompareGreaterThan)
		return "jle";
	if (type == ASTTypes::CompareGreaterThanEqual)
		return "jl";
	if (type == ASTTypes::CompareLessThan)
		return "jge";
	if (type == ASTTypes::CompareLessThanEqual)
		return "jg";

	abort();
	return "";
}

// Floating comparisons is different from integer cmp
// Floating point comparison is unsigned, so the jump functions for unsigned comparison has to be used instead
// https://stackoverflow.com/questions/20906639/difference-between-ja-and-jg-in-assembly
// http://unixwiz.net/techtips/x86-jumps.html
// https://stackoverflow.com/questions/20906639/difference-between-ja-and-jg-in-assembly

std::string ComparisonTypeToJumpInstructionFloat(ASTTypes& type)
{
	if (type == ASTTypes::CompareEquals)
		return "jne";
	if (type == ASTTypes::CompareNotEquals)
		return "je";
	if (type == ASTTypes::CompareGreaterThan)
		return "jbe";
	if (type == ASTTypes::CompareGreaterThanEqual)
		return "jb";
	if (type == ASTTypes::CompareLessThan)
		return "jae";
	if (type == ASTTypes::CompareLessThanEqual)
		return "ja";

	abort();
	return "";
}

ValueTypes NodeVariableTypeToValueType(ASTNode* n)
{
	assert(n->type == ASTTypes::VariableType);

	if (n->stringValue == "int") return ValueTypes::Integer;
	if (n->stringValue == "float") return ValueTypes::Float;
	if (n->stringValue == "double") return ValueTypes::Float;
	if (n->stringValue == "string") return ValueTypes::String;

	return ValueTypes::Void;
}

bool IsGlobalVariableDeclaration(ASTNode* n)
{
	return (n->type == ASTTypes::GlobalVariableDeclaration ||
		(n->type == ASTTypes::Assign && n->left->type == ASTTypes::GlobalVariableDeclaration));
}

bool HasPriorityCompilation(ASTNode* n)
{
	if (n->type == ASTTypes::GlobalVariableDeclaration ||
		(n->type == ASTTypes::Assign && n->left->type == ASTTypes::GlobalVariableDeclaration))
	{
		return true;
	}

	if (n->type == ASTTypes::FunctionDefinition)
		return true;

	return false;
}

std::vector<std::pair<ValueTypes, std::string>> AssemblyCompiler::ReverseFunctionArguments(ASTNode* node)
{
	std::vector<std::pair<ValueTypes, std::string>> argsSortedByType;

	// 1, 2, x, 3
	// 3, x, 2, 1

	// i1, i2, f3, f4
	// f3, f4, i2, i1

	// 1, y, x, 3
	// 3, y, x, 1

	// 1, y, 2, x, z
	// z, x, 2, y, 1

	for (int i = node->arguments.size() - 1; i >= 0; i--)
	{
		std::string name = "arg" + std::to_string(i);

		// Float variable, because the intermediate value is not stored in a variable
		if (!m_Context.HasVariable(name))
		{
			argsSortedByType.push_back(std::make_pair(ValueTypes::Float, name));
		}
		else
		{
			auto& variable = m_Context.GetVariable(name);
			argsSortedByType.push_back(std::make_pair(variable.m_Type, name));
		}
	}

	// Now find the floats in the array and reverse them
	std::vector<std::pair<ValueTypes, std::string>> floatArguments;

	// Collect floats
	for (int i = 0; i < argsSortedByType.size(); i++)
	{
		if (argsSortedByType[i].first == ValueTypes::Float)
			floatArguments.push_back(argsSortedByType[i]);
	}

	// Reverse them
	std::reverse(floatArguments.begin(), floatArguments.end());

	// Put the reversed items back into the original positions
	for (int i = 0, j = 0; i < argsSortedByType.size(); i++)
	{
		if (argsSortedByType[i].first == ValueTypes::Float)
		{
			argsSortedByType[i] = floatArguments[j];
			j++;
		}
	}

	return argsSortedByType;
}

std::vector<std::pair<ValueTypes, int>> AssemblyCompiler::ReverseFunctionArgumentIndicies(ASTNode* node)
{
	std::vector<std::pair<ValueTypes, int>> argsSortedByType;

	// Collect arguments in reverse order
	for (int i = node->arguments.size() - 1; i >= 0; i--)
	{
		ValueTypes typeOfArgument = GetValueTypeOfNode(node->arguments[i]);
		if (m_Error != "")
			return {};

		argsSortedByType.push_back(std::make_pair(typeOfArgument, i));
	}

	// Now find the floats in the array and reverse them
	std::vector<std::pair<ValueTypes, int>> floatArguments;

	// Collect floats
	for (int i = 0; i < argsSortedByType.size(); i++)
	{
		if (argsSortedByType[i].first == ValueTypes::Float)
			floatArguments.push_back(argsSortedByType[i]);
	}

	// Reverse them
	std::reverse(floatArguments.begin(), floatArguments.end());

	// Put the reversed items back into the original positions
	for (int i = 0, j = 0; i < argsSortedByType.size(); i++)
	{
		if (argsSortedByType[i].first == ValueTypes::Float)
		{
			argsSortedByType[i] = floatArguments[j];
			j++;
		}
	}

	return argsSortedByType;
}

void Section::AddInstruction(Instruction inst)
{
	m_Lines.push_back(inst);
}

void Section::AddInstruction(std::string op, std::string dest, std::string src, std::string comment)
{
	Instruction inst(op, dest, src, comment);
	AddInstruction(inst);
}

void Section::AddComment(const std::string& comment)
{
	Instruction inst("", "", "", comment);
	inst.m_IsOnlyComment = true;
	m_Lines.push_back(inst);
}

void Section::AddLabel(const std::string& label)
{
	Instruction inst(label, "", "", "");
	inst.m_IsLabel = true;
	m_Lines.push_back(inst);
}

void Section::AddLine(const std::string& line)
{
	Instruction inst(line, "", "", "");
	m_Lines.push_back(inst);
}

void Section::AddCorrectMathInstruction(ASTNode* n, ValueTypes type, bool reverse)
{
	if (n->type == ASTTypes::Add)
	{
		// multiply and pop, leaving st0 = old_st0 * old_st1. (st1 is freed / empty now)
		if (type == ValueTypes::Float)
			AddInstruction("faddp");
		else
			AddInstruction("add", "eax", "ebx");
	}
	else if (n->type == ASTTypes::Subtract)
	{
		// Because the result is stored in the first operand, and all code assumes eax has the result. Therefore the value of ebx has to be moved to eax.
		if (reverse)
		{
			if (type == ValueTypes::Float)
			{
				abort();
			}
			else
			{
				AddInstruction("sub", "ebx", "eax");
				AddInstruction("mov", "eax", "ebx");
			}
		}
		else
		{
			if (type == ValueTypes::Float)
				AddInstruction("fsubrp");
			else
				AddInstruction("sub", "eax", "ebx");

		}
	}
	else if (n->type == ASTTypes::Multiply)
	{
		if (type == ValueTypes::Float)
			AddInstruction("fmulp");
		else
			AddInstruction("imul", "eax", "ebx");
	}
		
	else if (n->type == ASTTypes::Divide)
	{
		/*if (reverse)
			return Opcodes::div_reverse;*/

		assert(!reverse);

		if (type == ValueTypes::Float)
		{
			AddInstruction("fdivrp");
		} 
		else
		{
			AddInstruction("mov", "edx", "0");
			AddInstruction("idiv", "ebx");
		}
	} else
		abort();
}

AssemblyCompiler::AssemblyCompiler()
{
	// Declare internal functions
	// printf
	auto& printFunc = m_Context.CreateFunction("print", ValueTypes::Void);
	printFunc.m_ActualName = "printf";
	printFunc.m_IsStdLib = true;
	
	printFunc.m_Arguments = {
		ValueTypes::String,
		ValueTypes::Any,
		ValueTypes::Any,
		ValueTypes::Any,
		ValueTypes::Any,
		ValueTypes::Any,
		ValueTypes::Any,
		// TODO: ...
	};

	auto printfFunc = printFunc;
	printfFunc.m_Name = "printf";
	m_Context.m_Functions["printf"] = printfFunc;


	// rand
	auto& randFunc = m_Context.CreateFunction("rand", ValueTypes::Integer);
	randFunc.m_IsStdLib = true;
	// srand
	auto& srandFunc = m_Context.CreateFunction("srand", ValueTypes::Integer, { ValueTypes::Integer });
	srandFunc.m_IsStdLib = true;
	// time
	auto& timeFunc = m_Context.CreateFunction("time", ValueTypes::Integer, { ValueTypes::Integer });
	timeFunc.m_IsStdLib = true;


	// to_float
	auto& to_floatFunc = m_Context.CreateFunction("to_float", ValueTypes::Float, { ValueTypes::Integer } );
	to_floatFunc.m_IsStdLib = true;
}

void AssemblyCompiler::Compile(ASTNode* node)
{
	ASTNode* left = node->left;
	ASTNode* right = node->right;

	auto AddLinesForPerformingMath = [&](bool reverse = false) {
		// pop latest to the "left" reg
		// pop the one after to the "right" reg

		ValueTypes typeLhs = GetValueTypeOfNode(left);
		ValueTypes typeRhs = GetValueTypeOfNode(right);
		ValueTypes type = typeLhs;

		// Typechecking
		if (typeLhs != typeRhs)
			return MakeError("Non-match matching types for " + node->ToString(false) + "(" + ValueTypeToString(typeLhs) + " and " + ValueTypeToString(typeRhs) + ")");

		//m_TextSection.AddLine("; pop values");
		if (type == ValueTypes::Integer || type == ValueTypes::String)
		{
			m_TextSection.AddInstruction("pop", "eax");
			m_TextSection.AddInstruction("pop", "ebx");
		}

		m_TextSection.AddComment("math operation");
		m_TextSection.AddCorrectMathInstruction(node, type, reverse);

		if (type == ValueTypes::Integer || type == ValueTypes::String)
			m_TextSection.AddInstruction("push", "eax");

		m_TextSection.AddComment("");
	};

	if (m_Error != "")
		return;

	switch (node->type)
	{
	case ASTTypes::ProgramBody:
	{
		m_TextSection.AddLabel("global CMAIN");
		m_TextSection.AddLabel("CMAIN:");
		CreateNewCallFrame(m_TextSection);

		// Look for functions and compile them before the main program

		// Compile global constant declarations as the first thing
		for (int i = 0; i < node->left->arguments.size(); i++)
		{
			// Prioritize compiling global variable declarations and declaration + assignment
			// Functions compile in the next pass
			ASTNode* n = node->left->arguments[i];
			if (IsGlobalVariableDeclaration(n))
			{
				m_TextSection.AddComment("Set value of global variable");
				Compile(n);
			}
			}

		// Weird hack to compile the function definitions after the variables, but place the generated code before them
		Section mainTextSection = m_TextSection;
		auto& functionText = m_TextSection.GetLines();
		functionText.clear(); // Reset the compiled code

		// Compile function definitions
		for (int i = 0; i < node->left->arguments.size(); i++)
		{
			ASTNode* n = node->left->arguments[i];
			if (n->type == ASTTypes::FunctionDefinition)
				Compile(n);
		}

		auto& mainText = mainTextSection.GetLines();

		// Insert function code before the main code, in the main code vector
		mainText.insert(std::begin(mainText), std::begin(functionText), std::end(functionText));

		m_TextSection = mainTextSection;

		Compile(left);

		RestoreOldCallFrame(m_TextSection);

		m_TextSection.AddInstruction("ret");
		break;
	}
	case ASTTypes::Scope:
	{
		AssemblyCompilerContext prevContext = m_Context;

		for (int i = 0; i < node->arguments.size(); i++)
		{
			ASTNode* n = node->arguments[i];

			if (n->type != ASTTypes::FunctionDefinition && !IsGlobalVariableDeclaration(n))
				Compile(n);
		}

		int labelIndex = m_Context.m_LoopInfo.labelIndex;

		m_Context = prevContext;
		m_Context.m_LoopInfo.labelIndex = labelIndex;
	}
	case ASTTypes::Empty:
		break;
	case ASTTypes::VariableDeclaration:
	case ASTTypes::GlobalVariableDeclaration:
	{
		const std::string& name = node->right->stringValue;
		if (m_Context.HasVariable(name))
			return MakeError("Variable '" + name + "' has already been declared in this scope");

		ValueTypes type = GetValueTypeOfNode(node->left);
		int size = 4;

		if (type == ValueTypes::Integer || type == ValueTypes::String)
			size = 4;
		else if (type == ValueTypes::Float)
			size = 8;

		Publicity publicity = Publicity::Local;
		if (node->type == ASTTypes::GlobalVariableDeclaration)
		{
			// If not in the global scope
			if (node->parent == nullptr || node->parent->parent->type != ASTTypes::ProgramBody)
				return MakeError("Global variable declaration has to be at the global scope in the file");

			publicity = Publicity::Global;
		}

		AssemblyCompilerContext::Variable variable = m_Context.CreateVariable(name, type, size, publicity);

		if (publicity == Publicity::Global)
		{
			m_DataSection.AddLine(variable.m_MangledName + " " + "DD 0");
		}
		else 
		{
			if (type == ValueTypes::Integer || type == ValueTypes::String)
			{
				m_TextSection.AddInstruction("mov", variable.GetASMLocation("dword"), "0");
				m_TextSection.AddInstruction("sub", "esp", "4");
			}
			else if (type == ValueTypes::Float)
			{
				m_TextSection.AddInstruction("fstp", variable.GetASMLocation("qword"));
				m_TextSection.AddInstruction("sub", "esp", "8");
			}
		}

		break;
	}
	case ASTTypes::VariableType:
		break;
	case ASTTypes::Assign:
	{
		if (node->left->type == ASTTypes::VariableDeclaration || node->left->type == ASTTypes::GlobalVariableDeclaration)
		{
			const std::string& name = node->left->right->stringValue;
			if (m_Context.HasVariable(name))
				return MakeError("Variable '" + name + "' has already been declared in this scope");

			// Evaluate the assignment on the rhs
			Compile(node->right);

			// Typecheck rhs and variable type
			ValueTypes rhsType = GetValueTypeOfNode(node->right);
			ValueTypes variableType = NodeVariableTypeToValueType(node->left->left);
			if (variableType != rhsType)
				return MakeError("Right hand side with value '" + ValueTypeToString(rhsType) + "' cannot be assigned to an '" + ValueTypeToString(variableType) + "'");

			int size = 4;

			if (variableType == ValueTypes::Integer || variableType == ValueTypes::String)
			{
				size = 4;
				m_TextSection.AddInstruction("pop", "eax");
			}
			else if (variableType == ValueTypes::Float)
			{
				size = 8;
			}

			Publicity publicity = Publicity::Local;
			if (node->left->type == ASTTypes::GlobalVariableDeclaration)
			{
				// If not in the global scope
				if (node->parent == nullptr || node->parent->parent->type != ASTTypes::ProgramBody)
					return MakeError("Global variable declaration has to be at the global scope in the file");

				publicity = Publicity::Global;
			}

			AssemblyCompilerContext::Variable variable = m_Context.CreateVariable(name, variableType, size, publicity);

			if (variable.m_Publicity == Publicity::Global)
			{
				m_DataSection.AddLine(variable.m_MangledName + " " + "DD 0");
				m_TextSection.AddInstruction("mov", variable.GetASMLocation("dword"), "eax");
			}
			else
			{
				if (variableType == ValueTypes::Integer || variableType == ValueTypes::String)
				{
					m_TextSection.AddInstruction("mov", variable.GetASMLocation("dword"), "eax");
					m_TextSection.AddInstruction("sub", "esp", "4");
				}
				else if (variableType == ValueTypes::Float)
				{
					// Convert the float into a float
					//m_TextSection.AddInstruction("fld", "dword [eax]");
					m_TextSection.AddInstruction("fstp", variable.GetASMLocation("qword"));
					m_TextSection.AddInstruction("sub", "esp", "8");
				}
			}

			return;
		}

		const std::string& variableName = node->left->stringValue;

		if (!m_Context.HasVariable(variableName))
			return MakeError("Variable '" + variableName + "' has not been declared in this scope");

		AssemblyCompilerContext::Variable variable = m_Context.GetVariable(variableName);

		// Evaluate the assignment on the rhs
		Compile(node->right);

		m_TextSection.AddInstruction("pop", "eax");

		if (variable.m_Publicity == Publicity::Global)
			m_TextSection.AddInstruction("mov", "dword [" + variable.m_MangledName + "]", "eax");
		else
			m_TextSection.AddInstruction("mov", "dword [ebp - " + std::to_string(variable.m_Index) + "]", "eax");

		break;
	}
	case ASTTypes::PropertyAssign:
		break;
	case ASTTypes::CompareEquals:
	case ASTTypes::CompareNotEquals:
	case ASTTypes::CompareLessThan:
	case ASTTypes::CompareGreaterThan:
	case ASTTypes::CompareLessThanEqual:
	case ASTTypes::CompareGreaterThanEqual:
	{
		ValueTypes typeLhs = GetValueTypeOfNode(left);
		ValueTypes typeRhs = GetValueTypeOfNode(right);
		if (typeLhs != typeRhs)
			return MakeError("Non-match matching types for " + node->right->ToString(false) + " (" + ValueTypeToString(typeLhs) + " and " + ValueTypeToString(typeRhs) + ")");


		Compile(node->left);
		Compile(node->right);


		if (typeLhs == ValueTypes::Integer)
		{
			m_TextSection.AddInstruction("pop", "eax");
			m_TextSection.AddInstruction("pop", "ebx");

			m_TextSection.AddInstruction("cmp", "ebx", "eax");
		} 
		else if (typeRhs == ValueTypes::String)
		{
			abort();
		}
		else if (typeRhs == ValueTypes::Float)
		{
			m_TextSection.AddInstruction("fxch", "st1");
			m_TextSection.AddInstruction("fcomip");
			m_TextSection.AddInstruction("fstp", "st0");
		}

		break;
	}
	case ASTTypes::And:
		break;
	case ASTTypes::Or:
		break;
	case ASTTypes::Not:
		break;
	case ASTTypes::Null:
		break;
	case ASTTypes::IntLiteral:
	{
		m_TextSection.AddInstruction("mov", "eax", std::to_string((int)node->numberValue));
		m_TextSection.AddInstruction("push", "eax");
		return;

	}
	case ASTTypes::DoubleLiteral:
	{		
		// Store the float literal in the data section as a constant.
		// Whole numbers can be used like "mov eax, 2", but it doesn't work with "2.0"
		std::string index = "";
		std::string variableName = "";

		if (!m_Constants.HasFloat(node->numberValue))
		{
			index = std::to_string(m_Constants.StoreFloat(node->numberValue));
			variableName = "float_" + index;

			/*auto& variable = m_Context.CreateVariable(variableName, ValueTypes::Float, 4, Publicity::Global);*/

			m_DataSection.AddLine(variableName + " " + "DQ " + FloatToString(node->numberValue));
		}
		else
		{
			index = std::to_string(m_Constants.GetFloatIndex(node->numberValue));
			variableName = "float_" + index;
		}

		// Push number onto top of FPU stack
		m_TextSection.AddInstruction("fld", "qword [" + variableName + "]");

		break;
	}
		
	case ASTTypes::StringLiteral:
	{
		// TODO: Store string in global variable
		// 
		// Allocate space for string
		const std::string& str = node->stringValue;

		int strSize = str.size() + 1;

		m_TextSection.AddInstruction("sub", "esp", std::to_string(strSize));

		m_TextSection.AddInstruction("mov", "byte [ebp - " + std::to_string(m_Context.Allocate(1)) + "]", "0x00");

		for (int i = str.length() - 1; i >= 0; i--)
		{
			// (`) are required instead of (") for escape codes to work, like newlines
			m_TextSection.AddInstruction("mov", "byte [ebp - " + std::to_string(m_Context.Allocate(1)) + "]", "\`" + std::string(1, str[i]) + "\`");
		}

		int indexOfFirstCharacter = m_Context.m_CurrentVariableIndex;

		m_TextSection.AddInstruction("lea", "eax", "[ebp - " + std::to_string(indexOfFirstCharacter) + "]", "copy pointer of start of string");
		m_TextSection.AddInstruction("push", "eax");

		break;
	}
	case ASTTypes::Bool:
		break;
	case ASTTypes::ArrayType:
		break;
	case ASTTypes::FunctionType:
		break;
	case ASTTypes::ObjectType:
		break;
	case ASTTypes::Variable:
	{
		const std::string& variableName = node->stringValue;
		if (!m_Context.HasVariable(variableName))
			return MakeError("Variable '" + variableName + "' has not been declared in this scope");

		AssemblyCompilerContext::Variable variable = m_Context.GetVariable(variableName);

		if (variable.m_Type == ValueTypes::Integer || variable.m_Type == ValueTypes::String)
		{
			m_TextSection.AddInstruction("mov", "eax", variable.GetASMLocation(), variable.m_MangledName);
			m_TextSection.AddInstruction("push", "eax");
		}
		else if (variable.m_Type == ValueTypes::Float)
		{
			m_TextSection.AddInstruction("fld", variable.GetASMLocation("qword"), "", variable.m_MangledName);
			//m_TextSection.AddInstruction("lea", "eax", variable.GetASMLocation());
		}

		//m_TextSection.AddInstruction("push", "eax");
	}
		break;
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
			// Typechecking
			ValueTypes typeRhs = GetValueTypeOfNode(right);
			ValueTypes typeLhs = GetValueTypeOfNode(left);
			if (typeLhs != typeRhs)
				return MakeError("Non-match matching types for " + node->right->ToString(false) + " (" + ValueTypeToString(typeLhs) + " and " + ValueTypeToString(typeRhs) + ")");

			Compile(right);
			m_TextSection.AddComment("push lhs");
			Compile(left);

			AddLinesForPerformingMath();

			return;
		}
		if (left->IsMathOperator())
		{
			// Typechecking
			ValueTypes typeLhs = GetValueTypeOfNode(left);
			ValueTypes typeRhs = GetValueTypeOfNode(right);
			if (typeLhs != typeRhs)
				return MakeError("Non-match matching types for " + node->right->ToString(false) + " (" + ValueTypeToString(typeLhs) + " and " + ValueTypeToString(typeRhs) + ")");

			Compile(left);
			m_TextSection.AddComment("push rhs");
			Compile(right);
			AddLinesForPerformingMath(true /* reverse the operand order */);

			return;
		}

		// Typechecking
		ValueTypes typeRhs = GetValueTypeOfNode(right);
		ValueTypes typeLhs = GetValueTypeOfNode(left);
		if (typeLhs != typeRhs)
			return MakeError("Non-match matching types for " + node->right->ToString(false) + " (" + ValueTypeToString(typeLhs) + " and " + ValueTypeToString(typeRhs) + ")");

		m_TextSection.AddComment("push rhs and lhs");
		Compile(right);
		Compile(left);

		AddLinesForPerformingMath();

		break;
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
		if (!m_Context.HasVariable(variableName))
			return MakeError("Variable '" + variableName + "' has not been declared in this scope");

		AssemblyCompilerContext::Variable variable = m_Context.GetVariable(variableName);

		std::string opcode = node->type == ASTTypes::PostIncrement ? "inc" : "dec";

		m_TextSection.AddInstruction(opcode, variable.GetASMLocation("dword"));
		break;
	}
	case ASTTypes::PreIncrement:
		break;
	case ASTTypes::PreDecrement:
		break;
	case ASTTypes::Line:
		break;
	case ASTTypes::FunctionCall:
	{
		const int maxArgumentCount = 4;
		const char* argumentRegisters[maxArgumentCount] = {
			"ebx",
			"edx",
			"ecx",
			"eax"
		};

		if (node->arguments.size() > maxArgumentCount)
			return MakeError("Too many arguments for function. A maximum of 4 is supported");

		std::string functionName = node->stringValue;

		// Check if the function has been defined
		if (!m_Context.HasFunction(functionName))
			return MakeError("Function '" + functionName + "' has not been defined");

		auto& function = m_Context.GetFunction(functionName);

		int argumentsSize = 0;

		auto argumentIndicies = ReverseFunctionArgumentIndicies(node);

		// Evaluate the arguments. The results are stored as variables on the stack
		for (int j = 0; j < argumentIndicies.size(); j++)
		{
			int i = argumentIndicies[j].second;

			Compile(node->arguments[i]);

			ValueTypes typeOfArgument = GetValueTypeOfNode(node->arguments[i]);
			if (m_Error != "")
				return;

			if (i >= function.m_Arguments.size())
				return MakeError("Function '" + functionName + "' doesn't accept this many arguments");

			// Ensure that the passed argument is the same as the type of the argument in the function prototype
			if (function.m_Arguments[i] != typeOfArgument && function.m_Arguments[i] != ValueTypes::Any)
				return MakeError("Parameter of type '" + ValueTypeToString(typeOfArgument) + "' cannot be passed to function that expects a '" + ValueTypeToString(function.m_Arguments[i]) + "'");

			if (typeOfArgument == ValueTypes::Float)
			{
				//m_Context.CreateVariable("arg" + std::to_string(i), ValueTypes::Float, 8);
				argumentsSize += 8;
			}
				
			if (typeOfArgument == ValueTypes::Integer || typeOfArgument == ValueTypes::String)
			{
				auto& variable = m_Context.CreateVariable("arg" + std::to_string(i), typeOfArgument, 4);
				m_TextSection.AddInstruction("mov", variable.GetASMLocation("dword"), "eax", "store argument " + ValueTypeToString(typeOfArgument));
				m_TextSection.AddInstruction("sub", "esp", "4");

				argumentsSize += 4;
			}
		}

		auto argsSortedByType = ReverseFunctionArguments(node);

		// The value of the evaluated arguments are stored in registers
		for (int i = 0; i < argsSortedByType.size(); i++)
		{
			const std::string& name = argsSortedByType[i].second;

			if (!m_Context.HasVariable(name))
				continue;

			auto& variable = m_Context.GetVariable(name);

			if (variable.m_Type == ValueTypes::Integer || variable.m_Type == ValueTypes::String)
				m_TextSection.AddInstruction("mov", argumentRegisters[i], variable.GetASMLocation("dword"), "evaluated argument");
		}

		// Push caller-saved registers
		// Before calling a subroutine, the caller should save the contents of certain registers that are designated caller-saved. 
		// The caller-saved registers are EAX, ECX, EDX. Since the called subroutine is allowed to modify these registers, if the caller relies on their values after the subroutine returns, 
		// the caller must push the values in these registers onto the stack (so they can be restore after the subroutine returns.

		//m_TextSection.AddInstruction("push", "eax");
		//m_TextSection.AddInstruction("push", "ecx");
		m_TextSection.AddInstruction("push", "edx");

		CreateNewCallFrame(m_TextSection);

		// The registers are pushed on to the new call stack
		for (int i = 0; i < argsSortedByType.size(); i++)
		{
			const std::string& name = argsSortedByType[i].second;

			// No variable means that it is a float
			if (!m_Context.HasVariable(name))
			{
				m_TextSection.AddInstruction("sub", "esp", "8");
				m_TextSection.AddInstruction("fstp", "qword [esp]");
				continue;
			}

			auto& variable = m_Context.GetVariable(name);

			/*if (variable.m_Type == ValueTypes::Float)
			{
				m_TextSection.AddInstruction("sub", "esp", "8");
				m_TextSection.AddInstruction("fstp", "qword [esp]");
			}
			else */
			if (variable.m_Type == ValueTypes::Integer || variable.m_Type == ValueTypes::String)
				m_TextSection.AddInstruction("push", argumentRegisters[i]);

			// Delete the temporary argument variable.The index will still be occupied, but the name will be free
			m_Context.DeleteVariable(name);
		}

		m_TextSection.AddInstruction("call", function.m_ActualName);

		// Remove the arguments from the stack
		m_TextSection.AddInstruction("add", "esp", std::to_string(argumentsSize), "size of arguments");

		RestoreOldCallFrame(m_TextSection);

		// Restore the contents of caller-saved registers (EAX, ECX, EDX) by popping them off of the stack. 
		// The caller can assume that no other registers were modified by the subroutine.
		m_TextSection.AddInstruction("pop", "edx");
		//m_TextSection.AddInstruction("pop", "ecx"); // TODO: The ecx register is never used by this compiler 
		//m_TextSection.AddInstruction("pop", "eax"); // Should be required, but we don't care about the old eax (most likely)

		if (!ResultCanBeDiscarded(node))
		{
			if (function.m_ReturnType == ValueTypes::Integer || function.m_ReturnType == ValueTypes::String)
				m_TextSection.AddInstruction("push", "eax");
		}

	}
	break;
	case ASTTypes::Return:
	{
		Compile(node->left);

		// Ensure correct types
		ValueTypes rhsType = GetValueTypeOfNode(node->left);
		if (rhsType == ValueTypes::Integer || rhsType == ValueTypes::String)
			m_TextSection.AddInstruction("pop", "eax");

		const std::string& functionName = m_Context.m_CurrentParsingFunctionName;

		if (functionName == "")
			return MakeError("Return has to be inside a function body");

		auto& function = m_Context.GetFunction(functionName);
		if (function.m_ReturnType == ValueTypes::Void)
		{
			// Ensure that void functions cannot return a value
			if (node->left != nullptr && node->left->type != ASTTypes::Empty)
				return MakeError("A void function cannot return a value. An empty return statement should be used instead");
		}

		if (function.m_ReturnType != rhsType)
			return MakeError("Return value '" + ValueTypeToString(rhsType) + "' does not match function prototype return value '" + ValueTypeToString(function.m_ReturnType) + "'");

		m_TextSection.AddComment("Subroutine Epilogue");

		// Before returning, we need to restore the old values of callee-saved registers (EDI, ESI and EBX)
		m_TextSection.AddComment("Restore calle-saved registers");
		m_TextSection.AddInstruction("pop", "ebx");
		//m_TextSection.AddInstruction("pop", "edi");
		//m_TextSection.AddInstruction("pop", "esi");

		// Deallocate local variables. Restores the stack pointer to the base where the first variable is
		m_TextSection.AddInstruction("mov", "esp", "ebp", "deallocate local variables");

		m_TextSection.AddInstruction("pop", "ebp", "", "restore old base pointer");

		m_TextSection.AddInstruction("ret");
	}
		break;
	case ASTTypes::IfStatement:
	{
		if (node->parent->type != ASTTypes::Else)
			m_Context.m_LoopInfo.labelIndex++;

		int labelIndex = m_Context.m_LoopInfo.labelIndex;

		Compile(node->left);

		ValueTypes type = GetValueTypeOfNode(node->left);

		std::string jmpInstruction = "";
		if (type == ValueTypes::Float)
			jmpInstruction = ComparisonTypeToJumpInstructionFloat(node->left->type);
		else
			jmpInstruction = ComparisonTypeToJumpInstruction(node->left->type);

		if (node->parent->type != ASTTypes::Else)
			m_TextSection.AddInstruction(jmpInstruction, "if_end" + std::to_string(labelIndex));
		else
			m_TextSection.AddInstruction(jmpInstruction, "if_else" + std::to_string(labelIndex));

		Compile(node->right);

		if (node->parent->type != ASTTypes::Else)
			m_TextSection.AddLabel("if_end" + std::to_string(labelIndex) + ":");
	
		break;
	}
	case ASTTypes::Else:
	{
		m_Context.m_LoopInfo.labelIndex++;
		int labelIndex = m_Context.m_LoopInfo.labelIndex;

		Compile(node->left);

		m_TextSection.AddInstruction("jmp", "if_end" + std::to_string(labelIndex));

		m_TextSection.AddLabel("if_else" + std::to_string(labelIndex) + ":");

		Compile(node->right);

		m_TextSection.AddLabel("if_end" + std::to_string(labelIndex) + ":");

		break;
	}
	case ASTTypes::WhileStatement:
	{
		m_Context.m_LoopInfo.labelIndex++;

		int labelIndex = m_Context.m_LoopInfo.labelIndex;

		m_TextSection.AddLabel("while_start" + std::to_string(labelIndex) + ":");

		Compile(node->left);

		ValueTypes type = GetValueTypeOfNode(node->left);

		std::string jmpInstruction = "";
		if (type == ValueTypes::Float)
			jmpInstruction = ComparisonTypeToJumpInstructionFloat(node->left->type);
		else
			jmpInstruction = ComparisonTypeToJumpInstruction(node->left->type);

		m_TextSection.AddInstruction(jmpInstruction, "while_end" + std::to_string(labelIndex));

		Compile(node->right);

		m_TextSection.AddInstruction("jmp", "while_start" + std::to_string(labelIndex));

		m_TextSection.AddLabel("while_end" + std::to_string(labelIndex) + ":");

		break;
	}
	case ASTTypes::ForStatement:
	{
		// 1. Variable
		Compile(node->arguments[0]);

		// 2. Condition
		m_Context.m_LoopInfo.labelIndex++;

		int labelIndex = m_Context.m_LoopInfo.labelIndex;

		m_TextSection.AddLabel("for_start" + std::to_string(labelIndex) + ":");

		Compile(node->arguments[1]);

		// Skip loop if condition is false
		ValueTypes type = GetValueTypeOfNode(node->arguments[1]);

		std::string jmpInstruction = "";
		if (type == ValueTypes::Float)
			jmpInstruction = ComparisonTypeToJumpInstructionFloat(node->arguments[1]->type);
		else
			jmpInstruction = ComparisonTypeToJumpInstruction(node->arguments[1]->type);

		m_TextSection.AddInstruction(jmpInstruction, "for_end" + std::to_string(labelIndex));

		// Body
		Compile(node->right);

		// 3. Action
		Compile(node->arguments[2]);

		// Jump back to condition
		m_TextSection.AddInstruction("jmp", "for_start" + std::to_string(labelIndex));

		m_TextSection.AddLabel("for_end" + std::to_string(labelIndex) + ":");

		break;
	}
	case ASTTypes::FunctionDefinition:
	{
		ASTNode* functionPrototype = node->left;
		ValueTypes returnType = NodeVariableTypeToValueType(functionPrototype->arguments[0]);
		const std::string& name = functionPrototype->arguments[1]->stringValue;

		if (m_Context.HasFunction(name))
			return MakeError("Function '" + name + "' has already been defined");
		
		// Create arguments for the function definition
		std::vector<ValueTypes> functionArguments;
		functionArguments.reserve(functionPrototype->arguments.size() - 2); // Reserve space. Not really needed :)
		for (int i = 2; i < functionPrototype->arguments.size(); i++)
		{
			functionArguments.push_back(GetValueTypeOfNode(functionPrototype->arguments[i]));
		}

		AssemblyCompilerContext::Function& function = m_Context.CreateFunction(name, returnType, functionArguments);

		m_TextSection.AddLabel(name + ":");

		m_TextSection.AddComment("Subroutine Prologue");

		CreateNewCallFrame(m_TextSection);

		// Next, save the values of the callee-saved registers that will be used by the function. To save registers, push them onto the stack. 
		// The callee-saved registers are EBX, EDI, and ESI (ESP and EBP will also be preserved by the calling convention, but need not be pushed on the stack during this step).
		m_TextSection.AddInstruction("push", "ebx");
		//m_TextSection.AddInstruction("push", "edi");
		//m_TextSection.AddInstruction("push", "esi");

		// New context for function
		AssemblyCompilerContext ctx = m_Context;

		m_Context.m_CurrentParsingFunctionName = name;

		m_TextSection.AddComment("Get arguments");
		// Compile arguments
		int byteOffsetThisArgument = 8;
		for (int i = 2; i < functionPrototype->arguments.size(); i++)
		{
			ASTNode* variableDeclaration = functionPrototype->arguments[i];

			ValueTypes typeOfArgument = GetValueTypeOfNode(variableDeclaration);

			// Pop the arguments and store in a register
			// The index for the argument is the base pointer + 8 + 4n (n = 0,1,2,3...)
			// The first arg would be at +4, but because we have to push ebp as the first thing in the function, it is +8

			// Ints and strings are stored as usual
			if (typeOfArgument == ValueTypes::Integer || typeOfArgument == ValueTypes::String)
			{
				m_TextSection.AddInstruction("mov", "eax", "[ebp + " + std::to_string(byteOffsetThisArgument) + "]");
				byteOffsetThisArgument += 4;
				Compile(variableDeclaration);

				// Change the instruction that stores 0 into the variable to use the eax register instead
				m_TextSection.GetLines()[m_TextSection.GetLines().size() - 2].m_Src = "eax";
			}
			// Floats are 8-bytes and need to be handled differently
			else if (typeOfArgument == ValueTypes::Float)
			{
				// Create the variable
				const std::string& name = variableDeclaration->right->stringValue;
				if (m_Context.HasVariable(name))
					return MakeError("Variable '" + name + "' has already been declared in this scope");

				AssemblyCompilerContext::Variable& variable = m_Context.CreateVariable(name, typeOfArgument, 8, Publicity::Local);

				m_TextSection.AddComment(name);
				m_TextSection.AddInstruction("mov", "eax", "[ebp + " + std::to_string(byteOffsetThisArgument) + "]");
				byteOffsetThisArgument += 4;
				m_TextSection.AddInstruction("mov", variable.GetASMLocation("dword"), "eax");

				m_TextSection.AddInstruction("mov", "eax", "[ebp + " + std::to_string(byteOffsetThisArgument) + "]");
				byteOffsetThisArgument += 4;
				m_TextSection.AddInstruction("mov", variable.GetASMLocation("dword", 4), "eax");

				m_TextSection.AddInstruction("sub", "esp", "8");

				// Change the instruction that stores 0 into the variable to use the eax register instead
				//m_TextSection.GetLines()[m_TextSection.GetLines().size() - 2].m_Src = "eax";
			}
		}

		m_TextSection.AddComment("");
		m_TextSection.AddComment("Body");
		Compile(node->right);

		m_Context = ctx;
	}
		break;
	case ASTTypes::FunctionPrototype:
		break;
	case ASTTypes::Break:
		break;
	case ASTTypes::Continue:
		break;
	default:
		break;
	}
}

void AssemblyCompiler::Optimize()
{
	auto& lines = m_TextSection.GetLines();

	std::vector<int> linesToRemove;

	/*lines.clear();
	m_TextSection.AddInstruction("push", "eax");
	m_TextSection.AddComment("hi");
	m_TextSection.AddInstruction("pop", "eax");*/

	Instruction prevInst;
	for (int i = 0; i < lines.size(); i++)
	{
		Instruction& inst = lines[i];

		Instruction prevInst;

		// Bad code for getting the previous line that isn't a blank or comment
		int prevInstIndex = i - 1;
		while (prevInstIndex >= 0 && lines[prevInstIndex].m_Op == "")
		{
			if (prevInstIndex > 0) prevInstIndex--;
		}

		if (prevInstIndex >= 0) prevInst = lines[prevInstIndex];

		// Unneeded stack operations
		if (prevInst.m_Op == "push" && inst.m_Op == "pop")
		{
			if (inst.m_Dest == prevInst.m_Dest && inst.m_Src == prevInst.m_Src)
			{
				linesToRemove.push_back(prevInstIndex);
				linesToRemove.push_back(i);
			}
		}

		// Remove push op that segfaults when returning
		if (prevInst.m_Op == "push" && inst.m_Op == "ret")
		{
			linesToRemove.push_back(prevInstIndex);
		}

		// Adding 0
		if (inst.m_Op == "add" && inst.m_Src == "0")
			linesToRemove.push_back(i);
		// Subtracting 0
		if (inst.m_Op == "sub" && inst.m_Src == "0")
			linesToRemove.push_back(i);
	}

	std::vector<Instruction> newInstructions;

	for (int i = 0; i < lines.size(); i++)
	{
		bool add = true;
		for (int toRemove : linesToRemove)
		{
			if (i == toRemove)
			{
				add = false;
				break;
			}
		}

		if (add)
			newInstructions.push_back(lines[i]);
	}

	lines = newInstructions;
}

ValueTypes AssemblyCompiler::GetValueTypeOfNode(ASTNode* node)
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
		return NodeVariableTypeToValueType(node);
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
		if (!m_Context.HasVariable(variableName))
		{
			MakeError("Variable '" + variableName + "' has not been declared in this scope");
			return ValueTypes::Void;
		}

		AssemblyCompilerContext::Variable variable = m_Context.GetVariable(variableName);

		return variable.m_Type;
	}
	break;
	case ASTTypes::Add:
	case ASTTypes::Subtract:
	case ASTTypes::Multiply:
	case ASTTypes::Divide:
	{
		// Recursivly perform the operations, do the inner ones first
		if (node->right->IsMathOperator())
		{
			ValueTypes right = GetValueTypeOfNode(node->right);
			ValueTypes left = GetValueTypeOfNode(node->left);

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
			ValueTypes left = GetValueTypeOfNode(node->left);
			ValueTypes right = GetValueTypeOfNode(node->right);

			if (left != right)
			{
				MakeError("Non-match matching types for " + node->right->ToString(false));
				return ValueTypes::Void;
			}
			// right is the same as left
			return right;
		}

		ValueTypes left = GetValueTypeOfNode(node->left);
		ValueTypes right = GetValueTypeOfNode(node->right);

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
		if (!m_Context.HasVariable(variableName))
		{
			MakeError("Variable '" + variableName + "' has not been declared in this scope");
			return ValueTypes::Void;
		}

		AssemblyCompilerContext::Variable variable = m_Context.GetVariable(variableName);
		return variable.m_Type;
	}
	case ASTTypes::PreIncrement:
	case ASTTypes::PreDecrement:
		abort();
	case ASTTypes::FunctionCall:
	{
		const std::string& functionName = node->stringValue;

		// Check user defined functions
		if (!m_Context.HasFunction(functionName))
		{
			MakeError("Function '" + functionName + "' has not been defined");
			return ValueTypes::Void;
		}

		auto& function = m_Context.GetFunction(functionName);
		return function.m_ReturnType;
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

bool AssemblyCompilerContext::HasVariable(const std::string& variableName)
{
	return m_Variables.count(variableName) == 1;
}

AssemblyCompilerContext::Variable& AssemblyCompilerContext::GetVariable(const std::string& variableName)
{
	assert(HasVariable(variableName));
	return m_Variables[variableName];
}

AssemblyCompilerContext::Variable& AssemblyCompilerContext::CreateVariable(const std::string& variableName, ValueTypes type, int size, Publicity publicity)
{
	//std::string editedVariableName = variableName;
	//editedVariableName += std::to_string(rand() % 100);
	assert(!HasVariable(variableName));

	Variable var;
	var.m_Name = variableName;
	var.m_MangledName = MangleName(variableName);

	if (publicity == Publicity::Local)
		var.m_Index = Allocate(size);

	var.m_Type = type;
	var.m_Publicity = publicity;

	m_Variables[variableName] = var;
	return m_Variables[variableName];
}

bool AssemblyCompilerContext::DeleteVariable(const std::string& variableName)
{
	assert(HasVariable(variableName));
	m_Variables.erase(variableName);
	return true;
}

bool AssemblyCompilerContext::HasFunction(const std::string& functionName)
{
	return m_Functions.count(functionName) == 1;
}

AssemblyCompilerContext::Function& AssemblyCompilerContext::GetFunction(const std::string& functionName)
{
assert(HasFunction(functionName));
return m_Functions[functionName];
}

AssemblyCompilerContext::Function& AssemblyCompilerContext::CreateFunction(const std::string& functionName, ValueTypes returnType, std::vector<ValueTypes> arguments)
{
	assert(!HasFunction(functionName));

	Function func;
	func.m_Name = functionName;
	func.m_MangledName = MangleName(functionName);
	func.m_ReturnType = returnType;
	func.m_Arguments = arguments;

	func.m_ActualName = func.m_Name;

	m_Functions[functionName] = func;
	return m_Functions[functionName];
}

int AssemblyCompilerContext::Allocate(int size)
{
	m_CurrentVariableIndex += size;
	return m_CurrentVariableIndex;
}

Instruction::Instruction(std::string op, std::string dest, std::string src, std::string comment)
{
	m_Op = op;
	m_Dest = dest;
	m_Src = src;
	m_Comment = comment;
}

std::string Instruction::ToString()
{
	std::string s;
	if (m_Op != "")
		s += m_Op;
	if (m_Dest != "")
		s += " " + m_Dest;
	if (m_Src != "")
		s += ", " + m_Src;
	if (m_Comment != "" && !m_IsOnlyComment)
		s += " ; " + m_Comment;
	if (m_Comment != "" && m_IsOnlyComment)
		s += "; " + m_Comment;

	std::string newS = "";
	for (int i = 0; i < s.length(); i++)
	{
		if (s[i] == '\n')
		{
			newS += "\\n";
		}
		else
		{
			newS += s[i];
		}
	}

	return newS;
}

std::string AssemblyCompilerContext::Variable::GetASMLocation(const std::string& datatype, int offset)
{
	std::string prefix = datatype == "" ? "" : (datatype + " ");

	if (m_Publicity == Publicity::Global)
		return prefix + "[" + m_MangledName + "]";
	else
		return prefix + "[ebp - " + std::to_string(m_Index - offset) + "]";

	abort();
	return "";
}

int ConstantsPool::GetFloatIndex(float value)
{
	std::string key = FloatToString(value);

	assert(m_FloatConstants.count(key) != 0);

	return m_FloatConstants[key];
}

int ConstantsPool::StoreFloat(float value)
{
	std::string key = FloatToString(value);

	assert(m_FloatConstants.count(key) == 0);

	m_FloatConstants[key] = ++m_FloatIndex;

	return m_FloatIndex;
}

bool ConstantsPool::HasFloat(float value)
{
	std::string key = FloatToString(value);
	return m_FloatConstants.count(key) != 0;
}
