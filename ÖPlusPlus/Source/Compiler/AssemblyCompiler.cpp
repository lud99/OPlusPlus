#include "AssemblyCompiler.h"

void Section::AddLine(std::string line, const std::string& comment)
{
	std::string l = line;
	if (comment != "")
		l += " ; " + comment;

	m_Lines.push_back(l);
}

std::string ResolveCorrectMathInstruction(ASTNode* n)
{
	if (n->type == ASTTypes::Add)
		return "add";
	else if (n->type == ASTTypes::Subtract)
	{
		//if (reverse)
			//return Opcodes::sub_reverse;

		return "sub";
	}
	else if (n->type == ASTTypes::Multiply)
		return "imul";// Opcodes::mul;
	else if (n->type == ASTTypes::Divide)
	{
		/*if (reverse)
			return Opcodes::div_reverse;*/

		return "idiv";
	}

	//if (n->type == ASTTypes::ToThePower)
	//	return (reverse ? Opcodes::pow_rev : Opcodes::pow);
	//if (n->type == ASTTypes::Modulus)
	//	return (reverse ? Opcodes::mod_rev : Opcodes::mod);
	//if (n->type == ASTTypes::Xor)
	//	return (reverse ? Opcodes::xr_rev : Opcodes::xr);

	abort();

	return "";
};

void AssemblyCompiler::Compile(ASTNode* node)
{
	ASTNode* left = node->left;
	ASTNode* right = node->right;

	switch (node->type)
	{
	case ASTTypes::ProgramBody:
	{
		return Compile(left);
	}
	case ASTTypes::Scope:
	{
		for (int i = 0; i < node->arguments.size(); i++)
		{
			ASTNode* n = node->arguments[i];

			Compile(n);
		}
	}
	case ASTTypes::Empty:
		break;
	case ASTTypes::VariableDeclaration:
		break;
	case ASTTypes::VariableType:
		break;
	case ASTTypes::Assign:
		break;
	case ASTTypes::PropertyAssign:
		break;
	case ASTTypes::CompareEquals:
		break;
	case ASTTypes::CompareNotEquals:
		break;
	case ASTTypes::CompareLessThan:
		break;
	case ASTTypes::CompareGreaterThan:
		break;
	case ASTTypes::CompareLessThanEqual:
		break;
	case ASTTypes::CompareGreaterThanEqual:
		break;
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
		/*     mov eax, 1 ; first number goes into eax
    mov ebx, 2 ; rest into ebx*/
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
		m_TextSection.AddLine("; " + ResolveCorrectMathInstruction(node));

		// Recursivly perform the operations, do the inner ones first
		if (right->IsMathOperator())
		{
			Compile(right);
			Compile(left);

			m_TextSection.AddLine("; perfom math op");

			// pop latest to the "left" reg
			m_TextSection.AddLine("pop eax");
			// pop to the "right" reg
			m_TextSection.AddLine("pop ebx");
			m_TextSection.AddLine(ResolveCorrectMathInstruction(node) + " eax, ebx");
			m_TextSection.AddLine("push eax");

			return;
		}
		if (left->IsMathOperator())
		{
			Compile(left);
			Compile(right);

			m_TextSection.AddLine("; perfom math op");

			// pop latest to the "left" reg
			m_TextSection.AddLine("pop eax");
			// pop to the "right" reg
			m_TextSection.AddLine("pop ebx");
			m_TextSection.AddLine(ResolveCorrectMathInstruction(node) + " eax, ebx");
			m_TextSection.AddLine("push eax");

			return;
		}

		Compile(right);
		
		//m_TextSection.AddLine("mov eax, " + std::to_string((int)left->numberValue));
		//m_TextSection.AddLine("mov ebx, " + std::to_string((int)right->numberValue));

		//
		
		Compile(left);

		// pop latest to the "left" reg
		m_TextSection.AddLine("pop eax");
		// pop to the "right" reg
		m_TextSection.AddLine("pop ebx");

		//m_TextSection.AddLine("add eax, ebx");

		// The current node has to be a math node
		assert(node->IsMathOperator());
		m_TextSection.AddLine(ResolveCorrectMathInstruction(node) + " eax, ebx");
		m_TextSection.AddLine("push eax");

		m_TextSection.AddLine("");

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
