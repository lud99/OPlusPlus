#include "AssemblyCompiler.h"

#include <iostream>

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

void Section::AddLine(std::string line, const std::string& comment)
{
	std::string l = line;
	if (comment != "")
		l += " ; " + comment;

	m_Lines.push_back(l);
}

std::string ResolveCorrectMathInstruction(ASTNode* n, bool reverse = false)
{
	if (n->type == ASTTypes::Add)
		return "add eax, ebx";
	else if (n->type == ASTTypes::Subtract)
	{
		// Because the result is stored in the first operand, and all code assumes eax has the result. Therefore the value of ebx has to be moved to eax.
		if (reverse)
			return "sub ebx, eax\nmov eax, ebx";

		return "sub eax, ebx";
	}
	else if (n->type == ASTTypes::Multiply)
		return "imul eax, ebx";
	else if (n->type == ASTTypes::Divide)
	{
		/*if (reverse)
			return Opcodes::div_reverse;*/

		assert(!reverse);

		return "mov edx, 0\nidiv ebx";
	}

	abort();

	return "";
};

void AssemblyCompiler::Compile(ASTNode* node)
{
	ASTNode* left = node->left;
	ASTNode* right = node->right;

	auto AddLinesForPerformingMath = [&](bool reverse = false) {
		// pop latest to the "left" reg
		// pop the one after to the "right" reg

		m_TextSection.AddLine("; pop values");
		m_TextSection.AddLine("pop eax");
		m_TextSection.AddLine("pop ebx");

		m_TextSection.AddLine("; math operation");
		m_TextSection.AddLine(ResolveCorrectMathInstruction(node, reverse));

		m_TextSection.AddLine("push eax");
		m_TextSection.AddLine("");
	};

	switch (node->type)
	{
	case ASTTypes::ProgramBody:
	{
		m_TextSection.AddLine("push ebp; save old top of stack");
		m_TextSection.AddLine("mov ebp, esp; current top of stack is bottom of new stack frame");
		
		return Compile(left);

		m_TextSection.AddLine("mov esp, ebp; restore esp, now points to old ebp(start of frame)");
		m_TextSection.AddLine("pop ebp; restore old ebp");
	}
	case ASTTypes::Scope:
	{
		/*m_TextSection.AddLine("push ebp; save old top of stack");
		m_TextSection.AddLine("mov ebp, esp; current top of stack is bottom of new stack frame");*/

		
			
		for (int i = 0; i < node->arguments.size(); i++)
		{
			ASTNode* n = node->arguments[i];

			Compile(n);
		}

		/*m_TextSection.AddLine("mov esp, ebp; restore esp, now points to old ebp(start of frame)");
		m_TextSection.AddLine("pop ebp; restore old ebp");*/
	}
	case ASTTypes::Empty:
		break;
	case ASTTypes::VariableDeclaration:
	{
		const std::string& variableName = node->right->stringValue;
		if (m_Context.HasVariable(variableName))
			return MakeError("Variable '" + variableName + "' has already been declared in this scope");

		AssemblyCompilerContext::Variable variable = m_Context.CreateVariable(variableName, NodeVariableTypeToValueType(node->left));

		m_TextSection.AddLine("mov dword [ebp - " + std::to_string(variable.m_Index) + "], 0");
		m_TextSection.AddLine("sub esp, 4");

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

			m_TextSection.AddLine("pop eax");
			m_TextSection.AddLine("mov dword [ebp - " + std::to_string(variable.m_Index) + "], eax");
			m_TextSection.AddLine("sub esp, 4");

			return;
		}

		const std::string& variableName = node->left->stringValue;

		if (!m_Context.HasVariable(variableName))
			return MakeError("Variable '" + variableName + "' has not been declared in this scope");

		AssemblyCompilerContext::Variable variable = m_Context.GetVariable(variableName);

		// Evaluate the assignment on the rhs
		Compile(node->right);

		m_TextSection.AddLine("pop eax");

		m_TextSection.AddLine("mov dword [ebp - " + std::to_string(variable.m_Index) + "], eax");

		break;
	}
	case ASTTypes::PropertyAssign:
		break;
	case ASTTypes::CompareEquals:
	{
		Compile(node->left);
		Compile(node->right);

		m_TextSection.AddLine("pop eax");
		m_TextSection.AddLine("pop ebx");

		m_TextSection.AddLine("cmp ebx, eax");
	}
		break;
	case ASTTypes::CompareNotEquals:
	case ASTTypes::CompareLessThan:
	case ASTTypes::CompareGreaterThan:
	case ASTTypes::CompareLessThanEqual:
	case ASTTypes::CompareGreaterThanEqual:
	{
		Compile(node->left);
		Compile(node->right);

		m_TextSection.AddLine("pop eax");
		m_TextSection.AddLine("pop ebx");

		m_TextSection.AddLine("cmp ebx, eax");
		
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
		m_TextSection.AddLine("mov eax, " + std::to_string((int)node->numberValue));
		m_TextSection.AddLine("push eax");
		return;
	}
	case ASTTypes::DoubleLiteral:
		break;
	case ASTTypes::StringLiteral:
		break;
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

		m_TextSection.AddLine("mov eax, [ebp - " + std::to_string(variable.m_Index) + "]");
		m_TextSection.AddLine("push eax");
	}
		break;
	case ASTTypes::PropertyAccess:
		break;
	case ASTTypes::ListInitializer:
		break;
	case ASTTypes::MathExpression:
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
			m_TextSection.AddLine("; push lhs");
			Compile(left);

			AddLinesForPerformingMath();

			return;
		}
		if (left->IsMathOperator())
		{
			Compile(left);
			m_TextSection.AddLine("; push rhs");
			Compile(right);
			AddLinesForPerformingMath(true /* reverse the operand order */);

			return;
		}

		m_TextSection.AddLine("; push rhs and lhs");
		Compile(right);
		Compile(left);

		AddLinesForPerformingMath();

		break;
	}
	case ASTTypes::PostIncrement:
		break;
	case ASTTypes::PreIncrement:
		break;
	case ASTTypes::PostDecrement:
		break;
	case ASTTypes::PreDecrement:
		break;
	case ASTTypes::Line:
		break;
	case ASTTypes::FunctionCall:
		break;
	case ASTTypes::Return:
		break;
	case ASTTypes::IfStatement:
	{
		Compile(node->left);

		std::string jmpInstruction = ComparisonTypeToJumpInstruction(node->left->type);

		m_TextSection.AddLine(jmpInstruction + " end");

		Compile(node->right);

		m_TextSection.AddLine("end:");
	}
		break;
	case ASTTypes::Else:
		break;
	case ASTTypes::WhileStatement:
		break;
	case ASTTypes::ForStatement:
		break;
	case ASTTypes::FunctionDefinition:
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

bool AssemblyCompilerContext::HasVariable(const std::string& variableName)
{
	return m_Variables.count(variableName) == 1;
}

AssemblyCompilerContext::Variable& AssemblyCompilerContext::GetVariable(const std::string& variableName)
{
	assert(HasVariable(variableName));
	return m_Variables[variableName];
}

AssemblyCompilerContext::Variable& AssemblyCompilerContext::CreateVariable(const std::string& variableName, ValueTypes type)
{
	assert(!HasVariable(variableName));

	Variable var;
	var.m_Name = variableName;
	var.m_Index = m_NextFreeVariableIndex;
	m_NextFreeVariableIndex += 4; // variable size in bytes, 4 for int
	var.m_Type = type;

	m_Variables[variableName] = var;
	return m_Variables[variableName];
}
