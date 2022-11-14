#include "ASTInterpreter.h"

#include "../Functions.h"

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
	bool ScopeFrame::HasFunction(const std::string& name)
	{
		return m_Functions.count(name) == 1;
	}
}

namespace ASTint
{
	ASTInterpreter& ASTInterpreter::Get()
	{
		static ASTInterpreter instance;
		return instance;
	}
	void ASTInterpreter::Initialize(ASTNode* tree)
	{
		m_ASTTree = tree;
		m_ScopeFrames.resize(ScopeFramesCount);
		for (int i = 0; i < ScopeFramesCount; i++)
		{
			m_ScopeFrames[i] = ScopeFrame();
		}
	}

	Value ASTInterpreter::Execute(ASTNode* tree)
	{
		Initialize(tree);
		return InterpretTree(m_ASTTree);
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

		if (m_ShouldReturn)
			return Value();

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
			ScopeFrame& previousFrame = GetTopFrame();
			ScopeFrame& frame = PushFrame();

			// Copy variables from previous frame to the current frame
			InheritVariables(previousFrame, frame);

			Value valueOfLastLine;

			auto& nodes = node->arguments;
			for (int i = 0; i < nodes.size() - 1; i++)
			{
				valueOfLastLine = InterpretTree(nodes[i]);

				if (m_ShouldReturn)
					return valueOfLastLine;
			}

			// Return the value of the last line
			valueOfLastLine = InterpretTree(nodes[nodes.size() - 1]);

			// Pop the scope
			PopFrame();
			
			return valueOfLastLine;
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

			// If this variable exists at scopes above, copy set them to 'rhs'
			for (int i = m_ScopeFrameTop; i >= 0; i--)
			{
				if (m_ScopeFrames[i].HasVariable(variableName))
					m_ScopeFrames[i].SetVariable(variableName, rhs);
				else
					break; // If this frame doesn't have the variable, then the ones furhter above won't either
			}

			return Value(variableName, ValueTypes::String);
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
				return MakeErrorValueReturn("Variable '" + variableName + "' has not been defined in this scope");
			
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
		case ASTTypes::PostIncrement:
		{
			const std::string& variableName = node->left->stringValue;

			if (!currentFrame.HasVariable(variableName))
				return MakeErrorValueReturn("Variable '" + variableName + "' has not been defined in this scope");

			Value variable = currentFrame.GetVariable(variableName);
			
			currentFrame.SetVariable(variableName, Value::Increment(variable));

			// Return the variable as it was before the increment
			return variable; 
		}
		case ASTTypes::PreIncrement:
		{
			const std::string& variableName = node->left->stringValue;

			if (!currentFrame.HasVariable(variableName))
				return MakeErrorValueReturn("Variable '" + variableName + "' has not been defined in this scope");

			Value variable = currentFrame.GetVariable(variableName);

			currentFrame.SetVariable(variableName, Value::Increment(variable));

			// Return the variable as it is after the increment
			return currentFrame.GetVariable(variableName);
		}
		case ASTTypes::PostDecrement:
		{
			const std::string& variableName = node->left->stringValue;

			if (!currentFrame.HasVariable(variableName))
				return MakeErrorValueReturn("Variable '" + variableName + "' has not been defined in this scope");

			Value variable = currentFrame.GetVariable(variableName);

			currentFrame.SetVariable(variableName, Value::Decrement(variable));

			// Return the variable as it was before the decrement
			return variable;
		}
		case ASTTypes::PreDecrement:
		{
			const std::string& variableName = node->left->stringValue;

			if (!currentFrame.HasVariable(variableName))
				return MakeErrorValueReturn("Variable '" + variableName + "' has not been defined in this scope");

			Value variable = currentFrame.GetVariable(variableName);

			currentFrame.SetVariable(variableName, Value::Decrement(variable));

			// Return the variable as it is after the decrement
			return currentFrame.GetVariable(variableName);
		}
		case ASTTypes::FunctionCall:
		{
			const std::string& functionName = node->stringValue;

			std::vector<Value> args;

			for (int i = 0; i < node->arguments.size(); i++)
			{
				args.push_back(InterpretTree(node->arguments[i]));
			}

			// User defined function
			if (currentFrame.HasFunction(functionName))
			{
				ASTNode* functionDefinition = currentFrame.m_Functions[functionName];
				ASTNode* functionPrototype = functionDefinition->left;

				ScopeFrame previousFrame = GetTopFrame();
				ScopeFrame& frame = PushFrame();

				InheritVariables(previousFrame, frame);

				// Args
				for (int i = 2; i < functionPrototype->arguments.size(); i++)
				{
					// Create the parameter variables
					InterpretTree(functionPrototype->arguments[i]);

					// Set the value of them (if there are any arguments passed)
					if (!args.empty())
					{
						const std::string& variableName = ResolveVariableName(functionPrototype->arguments[i]);

						frame.SetVariable(variableName, args[i - 2]);
					}
				}
				
				// Jump to the function
				Value returnValue = InterpretTree(functionDefinition->right);

				m_ShouldReturn = false;

				return returnValue;
			}

			// Internal function
			CallableFunction function = Functions::GetFunctionByName(functionName);
			if (!function)
				return MakeErrorValueReturn("Function '" + functionName + "' doesn't exist");

			return function(args);
		}
		case ASTTypes::Return:
		{
			Value returnValue = InterpretTree(node->left);
			m_ShouldReturn = true;

			return returnValue;
		}
		case ASTTypes::IfStatement:
		{
			Value lastResult;
			Value condition = InterpretTree(node->left);
			if (condition.IsTruthy())
			{
				lastResult = InterpretTree(node->right);

				if (m_ShouldReturn)
					return lastResult;
			}

			return lastResult;
		}
		case ASTTypes::Else:
			break;
		case ASTTypes::WhileStatement:
		{
			Value lastResult;

			auto condition = [&]() { return InterpretTree(node->left); };
			while (condition().IsTruthy())
			{
				lastResult = InterpretTree(node->right);

				if (m_ShouldReturn)
					return lastResult;
			}

			return lastResult;
		}
		case ASTTypes::ForStatement:
		{
			ScopeFrame previousFrame = GetTopFrame();
			ScopeFrame& frame = PushFrame();

			// Copy variables from previous frame to the current frame
			InheritVariables(previousFrame, frame);

			// 1. Variable
			InterpretTree(node->arguments[0]); // Create or set the variable
			std::string variableName = ResolveVariableName(node->arguments[0]);

			Value lastResult;

			// 2. Condition
			auto condition = [&]() { return InterpretTree(node->arguments[1]); };
			while (condition().IsTruthy())
			{
				// Execute the scope
				lastResult = InterpretTree(node->right);

				if (m_ShouldReturn)
					return lastResult;

				// 3. Action (increment, decrement)
				InterpretTree(node->arguments[2]);
			}

			PopFrame();

			return lastResult;
		}
		case ASTTypes::FunctionDefinition:
		{
			InterpretTree(node->left);

			const std::string& functionName = node->left->arguments[1]->stringValue;
			currentFrame.m_Functions[functionName] = node;

			return Value(functionName, ValueTypes::String);
		}
		case ASTTypes::FunctionPrototype:
		{
			Value returnValue = NodeVariableTypeToValueType(node->arguments[0]);
			const std::string& functionName = node->arguments[1]->stringValue;
			// variables
			
			// Create a variable that corresponds to the function
			if (currentFrame.HasVariable(functionName) || Functions::GetFunctionByName(functionName))
				return MakeErrorValueReturn("Function '" + functionName + "' has already been defined");

			currentFrame.CreateVariable(functionName, Value("{ function body }", ValueTypes::String));
			
			return Value();
		}
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
	std::string ASTInterpreter::ResolveVariableName(ASTNode* node)
	{
		switch (node->type)
		{
		case ASTTypes::VariableDeclaration:
		{
			std::string variableName = node->left->right->stringValue;
			return variableName;
		}
		case ASTTypes::Assign:
		{
			std::string variableName = node->left->stringValue;

			if (node->left->type == ASTTypes::VariableDeclaration)
				variableName = node->left->right->stringValue;

			return variableName;
		}
		case ASTTypes::Variable:
		{
			std::string variableName = node->stringValue;

			return variableName;
		}
		default:
			abort();
		}

		return "";
	}
	void ASTInterpreter::InheritVariables(ScopeFrame& previous, ScopeFrame& current)
	{
		// Copy variables from previous frame to the current fram
		for (auto& map : previous.m_Variables)
		{
			current.m_Variables[map.first] = map.second;
		}
	}
}
