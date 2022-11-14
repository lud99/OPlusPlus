#include "ASTInterpreter.h"

#include "../Bytecode/BytecodeFunctions.h"

#include <iostream>
#include <assert.h>

ValueTypes NodeVariableTypeToValueType(ASTNode* n)
{
	assert(n->type == ASTTypes::VariableType);

	if (n->stringValue == "int") return ValueTypes::Integer;
	if (n->stringValue == "float") return ValueTypes::Float;
	if (n->stringValue == "double") return ValueTypes::Float;
	if (n->stringValue == "string") return ValueTypes::String;
	//if (n->stringValue == "char") return ValueTypes::Char;
	//if (n->stringValue == "array") return ValueTypes::Array;
	//if (n->stringValue == "function") return ValueTypes::Function;
	//if (n->stringValue == "object") return ValueTypes::Object;

	return ValueTypes::Void;
}

// Scope frame
namespace ASTint
{
	bool ScopeFrame::HasVariable(const std::string& name)
	{
		return m_Variables.count(name) == 1;
	}

	Value& ScopeFrame::GetVariable(const std::string& name)
	{
		assert(HasVariable(name));
		return m_Variables[name];
	}

	void ScopeFrame::SetVariable(const std::string& name, Value value)
	{
		assert(HasVariable(name));
		m_Variables[name] = value;
	}

	Value& ScopeFrame::CreateVariable(const std::string& name, Value value)
	{
		assert(!HasVariable(name));

		m_Variables[name] = value;
		return m_Variables[name];
	}
}

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
		m_ScopeFrames.resize(ScopeFramesCount);
		for (int i = 0; i < ScopeFramesCount; i++)
		{
			m_ScopeFrames[i] = ScopeFrame();
		}
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

		if (node->type == ASTTypes::ProgramBody)
			PushFrame();

		ScopeFrame& currentFrame = GetTopFrame();

		switch (node->type)
		{
		case ASTTypes::Empty:
			break;
		case ASTTypes::ProgramBody:
			return InterpretTree(node->left);
		case ASTTypes::Scope:
		{
			ScopeFrame frame = PushFrame();
			auto& nodes = node->arguments;
			for (int i = 0; i < nodes.size() - 1; i++)
			{
				InterpretTree(nodes[i]);
			}

			// Return the value of the last line
			return InterpretTree(nodes[nodes.size() - 1]);
		}
		case ASTTypes::VariableDeclaration:
		{
			const std::string& variableName = node->right->stringValue;
			
			ValueTypes variableType = NodeVariableTypeToValueType(node->left);

			Value value;

			if (variableType == ValueTypes::Integer)
				value = Value(0, ValueTypes::Integer);
			else if (variableType == ValueTypes::Float)
				value = Value(0.0, ValueTypes::Float);
			else if (variableType == ValueTypes::String)
				value = Value("", ValueTypes::String);

			return currentFrame.CreateVariable(variableName, value);
		}
		case ASTTypes::VariableType:
			break;
		case ASTTypes::Assign:
		{
			std::string variableName = node->left->stringValue;

			// Create the variable first if it is a declaration
			if (node->left->type == ASTTypes::VariableDeclaration)
			{
				variableName = node->left->right->stringValue;
				InterpretTree(node->left);
			}

			// Evaluate value at rhs
			Value rhs = InterpretTree(node->right);
			currentFrame.SetVariable(variableName, rhs);

			return rhs;
		}
		case ASTTypes::PropertyAssign:
			break;
		case ASTTypes::CompareEquals:
		{
			Value lhs = InterpretTree(node->left);
			Value rhs = InterpretTree(node->right);

			return Value(Value::CompareEquals(lhs, rhs), ValueTypes::Integer);
		}
		case ASTTypes::CompareNotEquals:
		{
			Value lhs = InterpretTree(node->left);
			Value rhs = InterpretTree(node->right);

			return Value(Value::CompareNotEquals(lhs, rhs), ValueTypes::Integer);
		}
		case ASTTypes::CompareLessThan:
		{
			Value lhs = InterpretTree(node->left);
			Value rhs = InterpretTree(node->right);

			return Value(Value::CompareLessThan(lhs, rhs), ValueTypes::Integer);
		}
		case ASTTypes::CompareGreaterThan:
		{
			Value lhs = InterpretTree(node->left);
			Value rhs = InterpretTree(node->right);

			return Value(Value::CompareGreaterThan(lhs, rhs), ValueTypes::Integer);
		}
		case ASTTypes::CompareLessThanEqual:
		{
			Value lhs = InterpretTree(node->left);
			Value rhs = InterpretTree(node->right);

			return Value(Value::CompareLessThanEqual(lhs, rhs), ValueTypes::Integer);
		}
		case ASTTypes::CompareGreaterThanEqual:
		{
			Value lhs = InterpretTree(node->left);
			Value rhs = InterpretTree(node->right);

			return Value(Value::CompareGreaterThanEqual(lhs, rhs), ValueTypes::Integer);
		}
		case ASTTypes::And:
		{
			Value lhs = InterpretTree(node->left);
			Value rhs = InterpretTree(node->right);

			return Value(lhs.IsTruthy() && rhs.IsTruthy(), ValueTypes::Integer);
		}
		case ASTTypes::Or:
		{
			Value lhs = InterpretTree(node->left);
			Value rhs = InterpretTree(node->right);

			return Value(lhs.IsTruthy() || rhs.IsTruthy(), ValueTypes::Integer);
		}
		case ASTTypes::Not:
		{
			Value lhs = InterpretTree(node->left);
			Value rhs = InterpretTree(node->right);

			return Value(!lhs.IsTruthy(), ValueTypes::Integer);
		}
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
		{
			const std::string& variableName = node->stringValue;
			
			if (!currentFrame.HasVariable(variableName))
				MakeErrorValueReturn("Variable '" + variableName + "' has not been defined");
			
			return currentFrame.GetVariable(variableName);
		}		
		case ASTTypes::Add: 
		{
			Value lhs = InterpretTree(node->left);
			Value rhs = InterpretTree(node->right);

			return Value::Add(lhs, rhs);
		}
		case ASTTypes::Subtract:
		{
			Value lhs = InterpretTree(node->left);
			Value rhs = InterpretTree(node->right);

			return Value::Subtract(lhs, rhs);
		}
		case ASTTypes::Multiply:
		{
			Value lhs = InterpretTree(node->left);
			Value rhs = InterpretTree(node->right);

			return Value::Multiply(lhs, rhs);
		}
		case ASTTypes::Divide:
		{
			Value lhs = InterpretTree(node->left);
			Value rhs = InterpretTree(node->right);

			return Value::Divide(lhs, rhs);
		}
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
			std::vector<Value> args;

			for (int i = 0; i < node->arguments.size(); i++)
			{
				args.push_back(InterpretTree(node->arguments[i]));
			}

			std::cout << args[0].ToString() << "\n";

			break;
		}
		case ASTTypes::Return:
			break;
		case ASTTypes::IfStatement:
		{
			Value condition = InterpretTree(node->left);
			if (condition.IsTruthy())
				return InterpretTree(node->right); // Scope body

			return Value(false, ValueTypes::Integer);
		}
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

		return Value(ValueTypes::Void);
	}

	ScopeFrame& ASTInterpreter::PushFrame()
	{
		assert(m_ScopeFrameTop >= 0 && m_ScopeFrameTop < ScopeFramesCount - 1);

		m_ScopeFrameTop++;
		m_ScopeFrames[m_ScopeFrameTop] = ScopeFrame();
		return m_ScopeFrames[m_ScopeFrameTop];
	}

	ScopeFrame ASTInterpreter::PopFrame()
	{
		assert(m_ScopeFrameTop > 0 && m_ScopeFrameTop < ScopeFramesCount);

		ScopeFrame top = m_ScopeFrames[m_ScopeFrameTop];

		m_ScopeFrames[m_ScopeFrameTop] = ScopeFrame();

		m_ScopeFrameTop--;

		return top;
	}

	ScopeFrame& ASTInterpreter::GetTopFrame()
	{
		assert(m_ScopeFrameTop >= 0 && m_ScopeFrameTop < ScopeFramesCount);

		return m_ScopeFrames[m_ScopeFrameTop];
	}
}
