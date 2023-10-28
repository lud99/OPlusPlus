#include "Nodes.h"

namespace Ö::AST {
	std::string Node::TypeToString()
	{
		return std::string(magic_enum::enum_name(m_Type));
	};
	void Node::Print(std::string padding)
	{
		std::cout << padding << TypeToString() << (ToString() != "" ? ": " : "") << ToString() << "\n";
	}

	Identifier::Identifier(const std::string& name)
	{
		m_Name = name;
		m_Type = NodeType::Identifier;
	}

	Type::Type(const std::string& typeName)
	{
		m_TypeName = typeName;
		m_Type = NodeType::Typename;
	}

	VariableDeclaration::VariableDeclaration(Type* variableType, Identifier* variableName, Node* assignedValue)
	{
		m_Type = NodeType::VariableDeclaration;

		m_VariableType = variableType;
		m_VariableName = variableName;
		m_AssignedValue = assignedValue;
	}

	void VariableDeclaration::Print(std::string padding)
	{
		std::cout << padding << TypeToString() << ":\n";
		m_VariableType->Print(padding + "    ");
		m_VariableName->Print(padding + "    ");
		if (m_AssignedValue)
		{
			std::cout << padding + "    (value):\n";
			m_AssignedValue->Print(padding + "        ");
		}
	}

	BinaryExpression::BinaryExpression(Node* left, Operators::Operator op, Node* right)
	{
		m_Type = NodeType::BinaryExpression;
		m_Operator = op;

		m_Lhs = left;
		m_Rhs = right;
	}

	void BinaryExpression::Print(std::string padding)
	{
		std::cout << padding << TypeToString() << " (" << ToString() << "):\n";
		m_Lhs->Print(padding + "    ");
		m_Rhs->Print(padding + "    ");
	}

	std::string BinaryExpression::ToString()
	{
		return std::string(magic_enum::enum_name(m_Operator.m_Name)) + " (" + m_Operator.m_Symbol + ")";
	}

	UnaryExpression::UnaryExpression(Node* operand, Operators::Operator op)
	{
		m_Type = NodeType::UnaryExpression;
		m_Operator = op;
		m_Operand = operand;
	}

	void UnaryExpression::Print(std::string padding)
	{
		std::cout << padding << TypeToString() << " (" << ToString() << "):\n";
		m_Operand->Print(padding + "    ");
	}

	std::string UnaryExpression::ToString()
	{
		return std::string(magic_enum::enum_name(m_Operator.m_Name)) + " (" + m_Operator.m_Symbol + ")";
	}

	CallExpression::CallExpression(Node* callee, std::vector<Node*> arguments)
	{
		m_Type = NodeType::CallExpression;
		m_Callee = callee;
		m_Arguments = arguments;
	}

	void CallExpression::Print(std::string padding)
	{
		std::cout << padding << TypeToString() << " (" << m_Callee->ToString() << "):\n";

		for (auto& argument : m_Arguments)
		{
			argument->Print(padding + "    ");
		}
	}

	IntLiteral::IntLiteral(int value)
	{
		m_Value = value;
		m_Type = NodeType::IntLiteral;
	}

	FloatLiteral::FloatLiteral(float value)
	{
		m_Value = value;
		m_Type = NodeType::FloatLiteral;
	}

	DoubleLiteral::DoubleLiteral(double value)
	{
		m_Value = value;
		m_Type = NodeType::DoubleLiteral;
	}

	StringLiteral::StringLiteral(std::string value)
	{
		m_Value = value;
		m_Type = NodeType::StringLiteral;
	}

	WhileStatement::WhileStatement(Node* condition, BlockStatement* body)
	{
		m_Condition = condition;
		m_Body = body;
		m_Type = NodeType::WhileStatement;
	}
	IfStatement::IfStatement(Node* condition, BlockStatement* body, BlockStatement* elseArm)
	{
		m_Condition = condition;

		m_Body = body;
		m_ElseArm = elseArm;
		m_Type = NodeType::IfStatement;
	}
	void IfStatement::Print(std::string padding)
	{
		std::string newPadding = padding + "        ";
		std::cout << padding << TypeToString() << ": \n";

		std::cout << padding + "    (condition): \n";
		m_Condition->Print(newPadding);

		std::cout << padding + "    (body): \n";
		m_Body->Print(newPadding);

		if (m_ElseArm)
		{
			std::cout << padding + "    (else): \n";
			m_ElseArm->Print(newPadding);
		}
	}

	ForStatement::ForStatement(Node* initialization, Node* condition, Node* advancement, BlockStatement* body)
	{
		m_Type = NodeType::ForStatement;

		m_Initialization = initialization;
		m_Condition = condition;
		m_Advancement = advancement;
		m_Body = body;
	}
	void ForStatement::Print(std::string padding)
	{
		std::string newPadding = padding + "        ";
		std::cout << padding << TypeToString() << ": \n";

		std::cout << padding + "    (initialization): \n";
		if (m_Initialization) m_Initialization->Print(newPadding);

		std::cout << padding + "    (condition): \n";
		if (m_Condition) m_Condition->Print(newPadding);

		std::cout << padding + "    (advancement): \n";
		if (m_Advancement) m_Advancement->Print(newPadding);

		std::cout << padding + "    (body): \n";
		if (m_Body) m_Body->Print(newPadding);
	}
	void ConditionalStatement::Print(std::string padding)
	{
		std::string newPadding = padding + "        ";
		std::cout << padding << TypeToString() << ": \n";

		std::cout << padding + "    (condition): \n";
		m_Condition->Print(newPadding);

		std::cout << padding + "    (body): \n";
		m_Body->Print(newPadding);
	}
	void Scope::Print(std::string padding)
	{
		std::cout << padding << TypeToString() << ": \n";
		for (auto& line : m_Lines)
			line->Print(padding + "    ");
	}

	ReturnStatement::ReturnStatement(Node* returnValue)
	{
		m_Type = NodeType::Return;
		m_ReturnValue = returnValue;
	}

	FunctionPrototypeStatement::FunctionPrototypeStatement(Type* returnType, Identifier* name, std::vector<Node*> parameters)
	{
		m_Type = NodeType::FunctionPrototype;
		m_ReturnType = returnType;
		m_Name = name;
		m_Parameters = parameters;
	}

	void FunctionPrototypeStatement::Print(std::string padding)
	{
		std::string newPadding = padding + "        ";
		std::cout << padding << TypeToString() << ": \n";

		std::cout << padding + "    (return type): \n";
		m_ReturnType->Print(newPadding);

		std::cout << padding + "    (name): \n";
		m_Name->Print(newPadding);

		std::cout << padding + "    (parameters): \n";
		for (auto& parameter : m_Parameters)
			parameter->Print(newPadding);
	}
}