#include "AssemblyCompiler.h"

#include <iostream>

void CreateNewCallFrame(Section& section)
{
	section.AddComment("Create call frame");
	section.AddInstruction("push", "ebp");
	section.AddInstruction("mov", "ebp", "esp");
}
void RestoreOldCallFrame(Section& section)
{
	section.AddComment("Restore call frame");
	section.AddInstruction("mov", "esp", "ebp");
	section.AddInstruction("pop", "ebp");
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

ValueTypes NodeVariableTypeToValueType(ASTNode* n)
{
	assert(n->type == ASTTypes::VariableType);

	if (n->stringValue == "int") return ValueTypes::Integer;
	if (n->stringValue == "float") return ValueTypes::Float;
	if (n->stringValue == "double") return ValueTypes::Float;
	if (n->stringValue == "string") return ValueTypes::StringConstant;

	return ValueTypes::Void;
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

void Section::AddCorrectMathInstruction(ASTNode* n, bool reverse)
{
	if (n->type == ASTTypes::Add)
		AddInstruction("add", "eax", "ebx");
	else if (n->type == ASTTypes::Subtract)
	{
		// Because the result is stored in the first operand, and all code assumes eax has the result. Therefore the value of ebx has to be moved to eax.
		if (reverse)
		{
			AddInstruction("sub", "ebx", "eax");
			AddInstruction("mov", "eax", "ebx");
		}
		else
		{
			AddInstruction("sub", "eax", "ebx");
		}
	}
	else if (n->type == ASTTypes::Multiply)
		AddInstruction("imul", "eax", "ebx");
	else if (n->type == ASTTypes::Divide)
	{
		/*if (reverse)
			return Opcodes::div_reverse;*/

		assert(!reverse);

		AddInstruction("mov", "edx", "0");
		AddInstruction("idiv", "ebx");
	} else
		abort();
}

void AssemblyCompiler::Compile(ASTNode* node)
{
	ASTNode* left = node->left;
	ASTNode* right = node->right;

	auto AddLinesForPerformingMath = [&](bool reverse = false) {
		// pop latest to the "left" reg
		// pop the one after to the "right" reg

		//m_TextSection.AddLine("; pop values");
		m_TextSection.AddInstruction("pop", "eax");
		m_TextSection.AddInstruction("pop", "ebx");

		m_TextSection.AddComment("math operation");
		m_TextSection.AddCorrectMathInstruction(node, reverse);

		m_TextSection.AddInstruction("push", "eax");
		m_TextSection.AddComment("");
	};

	switch (node->type)
	{
	case ASTTypes::ProgramBody:
	{
		// Look for functions and compile them before the main program
		// TODO: global variable constants
		for (int i = 0; i < node->left->arguments.size(); i++)
		{
			if (node->left->arguments[i]->type == ASTTypes::FunctionDefinition)
				Compile(node->left->arguments[i]);
		}

		m_TextSection.AddLabel("global CMAIN");
		m_TextSection.AddLabel("CMAIN:");
		CreateNewCallFrame(m_TextSection);

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

			if (n->type != ASTTypes::FunctionDefinition)
				Compile(n);
		}

		m_Context = prevContext;
	}
	case ASTTypes::Empty:
		break;
	case ASTTypes::VariableDeclaration:
	{
		const std::string& name = node->right->stringValue;
		if (m_Context.HasVariable(name))
			return MakeError("Variable '" + name + "' has already been declared in this scope");

		ValueTypes type = NodeVariableTypeToValueType(node->left);
		int size = 4;

		// Check if this declaration is at the highest scope
		Publicity publicity = Publicity::Local;
		if (node->parent != nullptr && node->parent->parent->type == ASTTypes::ProgramBody)
			publicity = Publicity::Global;

		AssemblyCompilerContext::Variable variable = m_Context.CreateVariable(name, type, size, publicity);

		if (publicity == Publicity::Global)
		{
			m_DataSection.AddLine(variable.m_MangledName + " " + "DD 0");
		}
		else 
		{
			m_TextSection.AddInstruction("mov", "dword [ebp - " + std::to_string(variable.m_Index) + "]", "0");
			m_TextSection.AddInstruction("sub", "esp", "4");
		}

		break;
	}
	case ASTTypes::VariableType:
		break;
	case ASTTypes::Assign:
	{
		if (node->left->type == ASTTypes::VariableDeclaration)
		{
			const std::string& variableName = node->left->right->stringValue;
			if (m_Context.HasVariable(variableName))
				return MakeError("Variable '" + variableName + "' has already been declared in this scope");

			// Evaluate the assignment on the rhs
			Compile(node->right);

			AssemblyCompilerContext::Variable variable = m_Context.CreateVariable(variableName, NodeVariableTypeToValueType(node->left->left));

			m_TextSection.AddInstruction("pop", "eax");
			m_TextSection.AddInstruction("mov", "dword [ebp - " + std::to_string(variable.m_Index) + "]", "eax");
			m_TextSection.AddInstruction("sub", "esp", "4");

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
		Compile(node->left);
		Compile(node->right);

		m_TextSection.AddInstruction("pop", "eax");
		m_TextSection.AddInstruction("pop", "ebx");

		m_TextSection.AddInstruction("cmp", "ebx", "eax");
		
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
		break;
	case ASTTypes::StringLiteral:
	{
		// Allocate space for string
		const std::string& str = node->stringValue;

		int strSize = str.size() + 1;

		m_TextSection.AddInstruction("sub", "esp", std::to_string(strSize));

		m_TextSection.AddInstruction("mov", "byte [ebp - " + std::to_string(m_Context.Allocate(1)) + "]", "0x00");

		for (int i = str.length() - 1; i >= 0; i--)
		{
			m_TextSection.AddInstruction("mov", "byte [ebp - " + std::to_string(m_Context.Allocate(1)) + "]", "\"" + std::string(1, str[i]) + "\"");
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

		if (variable.m_Publicity == Publicity::Global)
			m_TextSection.AddInstruction("mov", "eax", "[" + variable.m_MangledName + "]");
		else
			m_TextSection.AddInstruction("mov", "eax", "[ebp - " + std::to_string(variable.m_Index) + "]");

		m_TextSection.AddInstruction("push", "eax");
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
			Compile(right);
			m_TextSection.AddComment("push lhs");
			Compile(left);

			AddLinesForPerformingMath();

			return;
		}
		if (left->IsMathOperator())
		{
			Compile(left);
			m_TextSection.AddComment("push rhs");
			Compile(right);
			AddLinesForPerformingMath(true /* reverse the operand order */);

			return;
		}

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

		m_TextSection.AddInstruction(opcode, "dword [ebp - " + std::to_string(variable.m_Index) + "]");
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
			"eax",
			"ecx",
			"edx",
			"ebx"
		};

		if (node->arguments.size() > maxArgumentCount)
			return MakeError("Too many arguments for function. A maximum of 4 is supported");

		// Evaluate the arguments. The results are stored as variables on the stack
		for (int i = node->arguments.size() - 1; i >= 0; i--)
		{
			Compile(node->arguments[i]);

			int variableIndex = m_Context.CreateVariable("arg" + std::to_string(i), ValueTypes::Integer /* todo */, 4).m_Index;

			m_TextSection.AddInstruction("mov", "dword [ebp - " + std::to_string(variableIndex) + "]", "eax", "store argument");
			m_TextSection.AddInstruction("sub", "esp", "4");
		}

		// The value of the evaluated arguments are stored in registers
		for (int i = node->arguments.size() - 1; i >= 0; i--)
		{
			int variableIndex = m_Context.GetVariable("arg" + std::to_string(i)).m_Index;
			m_TextSection.AddInstruction("mov", argumentRegisters[i], "dword [ebp - " + std::to_string(variableIndex) + "]", "evaluated argument");

			// Delete the temporary argument variable. The index will still be occupied, but the name will be free
			m_Context.DeleteVariable("arg" + std::to_string(i));
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
		for (int i = node->arguments.size() - 1; i >= 0; i--)
		{
			m_TextSection.AddInstruction("push", argumentRegisters[i]);
		}

		m_TextSection.AddInstruction("call", node->stringValue);

		// Remove the arguments from the stack
		m_TextSection.AddInstruction("add", "esp", std::to_string(node->arguments.size() * 4), "Assume all arguments are 4 bytes each");

		RestoreOldCallFrame(m_TextSection);

		// Restore the contents of caller-saved registers (EAX, ECX, EDX) by popping them off of the stack. 
		// The caller can assume that no other registers were modified by the subroutine.
		m_TextSection.AddInstruction("pop", "edx");
		//m_TextSection.AddInstruction("pop", "ecx");
		//m_TextSection.AddInstruction("pop", "eax"); // Should be required, but we don't care about the old eax (most likely)

		if (!ResultCanBeDiscarded(node))
			m_TextSection.AddInstruction("push", "eax");
	}
		break;
	case ASTTypes::Return:
	{
		Compile(node->left);

		m_TextSection.AddInstruction("pop", "eax");

		m_TextSection.AddComment("Subroutine Epilogue");

		// Before returning, we need to restore the old values of callee-saved registers (EDI, ESI and EBX)
		m_TextSection.AddComment("Restore calle-saved registers");
		m_TextSection.AddInstruction("pop", "ebx");
		//m_TextSection.AddInstruction("pop", "edi");
		//m_TextSection.AddInstruction("pop", "esi");

		// Deallocate local variables. Restores the stack pointer to the base where the first variable is
		m_TextSection.AddInstruction("mov", "esp", "ebp", "deallocate local varibales");

		m_TextSection.AddInstruction("pop", "ebp", "", "restore old base pointer");

		m_TextSection.AddInstruction("ret");
	}
		break;
	case ASTTypes::IfStatement:
	{
		if (node->parent->type != ASTTypes::Else)
			m_Context.m_LoopInfo.labelIndex++;

		Compile(node->left);

		std::string jmpInstruction = ComparisonTypeToJumpInstruction(node->left->type);

		if (node->parent->type != ASTTypes::Else)
			m_TextSection.AddInstruction(jmpInstruction, "if_end" + std::to_string(m_Context.m_LoopInfo.labelIndex));
		else
			m_TextSection.AddInstruction(jmpInstruction, "if_else" + std::to_string(m_Context.m_LoopInfo.labelIndex));

		Compile(node->right);

		if (node->parent->type != ASTTypes::Else)
			m_TextSection.AddLabel("if_end" + std::to_string(m_Context.m_LoopInfo.labelIndex) + ":");
	
		break;
	}
	case ASTTypes::Else:
	{
		m_Context.m_LoopInfo.labelIndex++;

		Compile(node->left);

		m_TextSection.AddInstruction("jmp", "if_end" + std::to_string(m_Context.m_LoopInfo.labelIndex));

		m_TextSection.AddLabel("if_else" + std::to_string(m_Context.m_LoopInfo.labelIndex) + ":");

		Compile(node->right);

		m_TextSection.AddLabel("if_end" + std::to_string(m_Context.m_LoopInfo.labelIndex) + ":");

		break;
	}
	case ASTTypes::WhileStatement:
	{
		m_Context.m_LoopInfo.labelIndex++;

		m_TextSection.AddLabel("while_start" + std::to_string(m_Context.m_LoopInfo.labelIndex) + ":");

		Compile(node->left);

		std::string jmpInstruction = ComparisonTypeToJumpInstruction(node->left->type);

		m_TextSection.AddInstruction(jmpInstruction, "while_end" + std::to_string(m_Context.m_LoopInfo.labelIndex));

		Compile(node->right);

		m_TextSection.AddInstruction("jmp", "while_start" + std::to_string(m_Context.m_LoopInfo.labelIndex));
		
		m_TextSection.AddLabel("while_end" + std::to_string(m_Context.m_LoopInfo.labelIndex) + ":");

		break;
	}
	case ASTTypes::ForStatement:
	{
		// 1. Variable
		Compile(node->arguments[0]);

		// 2. Condition
		m_Context.m_LoopInfo.labelIndex++;
		m_TextSection.AddLabel("for_start" + std::to_string(m_Context.m_LoopInfo.labelIndex) + ":");

		Compile(node->arguments[1]); 
		
		// Skip loop if condition is false
		std::string jmpInstruction = ComparisonTypeToJumpInstruction(node->arguments[1]->type);
		m_TextSection.AddInstruction(jmpInstruction, "for_end" + std::to_string(m_Context.m_LoopInfo.labelIndex));
		
		// Body
		Compile(node->right);

		// 3. Action
		Compile(node->arguments[2]);

		// Jump back to condition
		m_TextSection.AddInstruction("jmp", "for_start" + std::to_string(m_Context.m_LoopInfo.labelIndex));

		m_TextSection.AddLabel("for_end" + std::to_string(m_Context.m_LoopInfo.labelIndex) + ":");

		break;
	}
	case ASTTypes::FunctionDefinition:
	{
		ASTNode* functionPrototype = node->left;
		ValueTypes returnType = NodeVariableTypeToValueType(functionPrototype->arguments[0]);
		const std::string& name = functionPrototype->arguments[1]->stringValue;

		if (m_Context.HasFunction(name))
			return MakeError("Function '" + name + "' has already been defined");

		AssemblyCompilerContext::Function& function = m_Context.CreateFunction(name, returnType);

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

		m_TextSection.AddComment("Get arguments");
		// Compile arguments
		for (int i = 2; i < functionPrototype->arguments.size(); i++)
		{
			ASTNode* variableDeclaration = functionPrototype->arguments[i];

			// Pop the arguments and store in a register
			// The index for the argument is the base pointer + 8 + 4n
			// The first arg would be at +4, but because we have to push ebp as the first thing in the function, it is +8
			
			int indexOfArg;
			m_TextSection.AddInstruction("mov", "eax", "[ebp + " + std::to_string(8 + 4 * (i - 2)) + "]");
			Compile(variableDeclaration);

			// Change the instruction that stores 0 into the variable to use the eax register instead
			m_TextSection.GetLines()[m_TextSection.GetLines().size() - 2].m_Src = "eax";
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

AssemblyCompilerContext::Function& AssemblyCompilerContext::CreateFunction(const std::string& functionName, ValueTypes returnType)
{
	assert(!HasFunction(functionName));

	Function var;
	var.m_Name = functionName;
	var.m_MangledName = MangleName(functionName);
	var.m_ReturnType = returnType;

	m_Functions[functionName] = var;
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

	return s;
}
