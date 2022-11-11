#include "ASTInterpreter.h"

#include "../Bytecode/BytecodeFunctions.h";

namespace ASTint
{
	ASTInterpreter& ASTInterpreter::Get()
	{
		static ASTInterpreter instance;
		return instance;
	}
	ASTInterpreter::ASTInterpreter(ASTNode* tree)
	{
		m_ASTTree = tree;
	}

	void ASTInterpreter::MakeError(std::string error)
	{
		m_Error = error;
	}
	Value ASTInterpreter::MakeErrorValueReturn(std::string error)
	{
		m_Error = error;
		return Value(ValueTypes::Void);
	}

	Value ASTInterpreter::InterpretTree(ASTNode* node)
	{
		if (m_Error != "")
			return Value(ValueTypes::Void);

		switch (node->type)
		{
		case ASTTypes::Empty:
			break;
		case ASTTypes::ProgramBody:
			return InterpretTree(node->left);
		case ASTTypes::Scope:
		{
			auto& nodes = node->arguments;
			for (int i = 0; i < nodes.size() - 1; i++)
			{
				InterpretTree(nodes[i]);
			}

			// Return the value of the last line
			return InterpretTree(nodes[nodes.size() - 1]);
		}
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
			return Value((int)node->numberValue, ValueTypes::Integer);
		case ASTTypes::DoubleLiteral:
			return Value(node->numberValue, ValueTypes::Float);
		case ASTTypes::StringLiteral:
			return Value(node->stringValue, ValueTypes::StringConstant);
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
		
		case ASTTypes::Add: 
		{
			Value lhs = InterpretTree(node->left);
			Value rhs = InterpretTree(node->right);

			return Value::Add(lhs, rhs);
		}
		case ASTTypes::Subtract:
			break;
		case ASTTypes::Multiply:
			break;
		case ASTTypes::Divide:
			break;
		case ASTTypes::Xor:
			break;
		case ASTTypes::ToThePower:
			break;
		case ASTTypes::Modulus:
			break;
		case ASTTypes::PlusEquals:
			break;
		case ASTTypes::MinusEquals:
			break;
		case ASTTypes::PostIncrement:
			break;
		case ASTTypes::PreIncrement:
			break;
		case ASTTypes::PostDecrement:
			break;
		case ASTTypes::PreDecrement:
			break;
		case ASTTypes::FunctionCall:
		{
			std::vector<Value> args(node->arguments.size());

			for (int i = 0; i < node->arguments.size(); i++)
			{
				args[i] = InterpretTree(node->arguments[i]);
			}

			Functions

			break;
		}
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
}
