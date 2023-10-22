#include "Nodes.h"

namespace Ö::AST {
	Node::Node()
	{
	}
	Node::Node(Node* parent)
	{
		m_Parent = parent;
	}

	Program::Program(Node* parent)
	{
		m_Parent = parent;
		m_Type = NodeType::Program;
	}

	BlockStatement::BlockStatement(Node* parent)
	{
		m_Parent = parent;
		m_Type = NodeType::BlockStatement;
	}

	Identifier::Identifier(Node* parent, const std::string& name)
	{
		m_Parent = parent;
		m_Name = name;
		m_Type = NodeType::Identifier;
	}

	Type::Type(Node* parent, const std::string& typeName)
	{
		m_Parent = parent;
		m_TypeName = typeName;
		abort();
		//m_Type = NodeType::;
	}

	VariableDeclaration::VariableDeclaration(Node* parent)
	{
		m_Parent = parent;
		m_Type = NodeType::VariableDeclaration;
	}

	BinaryExpression::BinaryExpression(Node* parent, Node* left, Operators::Operator op, Node* right)
	{
		m_Parent = parent;
		m_Type = NodeType::BinaryExpression;
		m_Operator = op;

		m_Lhs = left;
		m_Rhs = right;
	}

	UnaryExpression::UnaryExpression(Node* parent, Node* operand, Operators::Operator op)
	{
		m_Parent = parent;
		m_Type = NodeType::UnaryExpression;
		m_Operator = op;
		m_Operand = operand;
	}

	CallExpression::CallExpression(Node* parent, Node* callee, std::vector<Node*> arguments)
	{
		m_Parent = parent;
		m_Type = NodeType::CallExpression;
		m_Callee = callee;
		m_Arguments = arguments;
	}

	IntLiteral::IntLiteral(Node* parent, int value)
	{
		m_Parent = parent;
		m_Value = value;
		m_Type = NodeType::IntLiteral;
	}

	FloatLiteral::FloatLiteral(Node* parent, float value)
	{
		m_Parent = parent;
		m_Value = value;
		m_Type = NodeType::FloatLiteral;
	}

	DoubleLiteral::DoubleLiteral(Node* parent, double value)
	{
		m_Parent = parent;
		m_Value = value;
		m_Type = NodeType::DoubleLiteral;
	}

	StringLiteral::StringLiteral(Node* parent, std::string value)
	{
		m_Parent = parent;
		m_Value = value;
		m_Type = NodeType::StringLiteral;
	}
}