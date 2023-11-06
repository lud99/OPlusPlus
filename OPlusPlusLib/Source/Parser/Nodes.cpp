#include "Nodes.h"

namespace O::AST {
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

	CallExpression::CallExpression(Node* callee, TupleExpression* arguments)
	{
		m_Type = NodeType::CallExpression;
		m_Callee = callee;
		m_Arguments = arguments;
	}

	void CallExpression::Print(std::string padding)
	{
		// todo: print correctly
		std::cout << padding << TypeToString() << " (" << m_Callee->ToString() << "):\n";

		m_Arguments->Print(padding + "    ");
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
	BoolLiteral::BoolLiteral(bool value)
	{
		m_Value = value;
		m_Type = NodeType::BoolLiteral;
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

	void ReturnStatement::Print(std::string padding)
	{
		std::cout << padding << TypeToString() << ": \n";
		if (m_ReturnValue)
			m_ReturnValue->Print(padding + "    ");
	}

	FunctionDefinitionStatement::FunctionDefinitionStatement(Type* returnType, Identifier* name, std::vector<VariableDeclaration*> parameters, Node* body)
	{
		m_Type = NodeType::FunctionDefinition;
		m_ReturnType = returnType;
		m_Name = name;
		m_Parameters = parameters;
		m_Body = body;
	}

	void FunctionDefinitionStatement::Print(std::string padding)
	{
		std::string newPadding = padding + "        ";
		std::cout << padding << TypeToString() << ": \n";

		std::cout << padding + "    (return type): \n";
		if (m_ReturnType) m_ReturnType->Print(newPadding);

		std::cout << padding + "    (name): \n";
		if (m_Name) m_Name->Print(newPadding);

		std::cout << padding + "    (parameters): \n";
		for (auto& parameter : m_Parameters)
			parameter->Print(newPadding);

		std::cout << padding + "    (body): \n";
		if (m_Body)
			m_Body->Print(newPadding);
	}

	ClosureExpression::ClosureExpression(BlockStatement* body)
	{
		m_Type = NodeType::Closure;
		m_Body = body;
	}

	void ClosureExpression::Print(std::string padding)
	{
		std::cout << padding << TypeToString() << ": \n";
		m_Body->Print(padding + "    ");
	}

	LoopStatement::LoopStatement(BlockStatement* body)
	{
		m_Type = NodeType::LoopStatement;
		m_Body = body;
	}

	void LoopStatement::Print(std::string padding)
	{
		std::cout << padding << TypeToString() << ": \n";
		m_Body->Print(padding + "    ");
	}
	BreakStatement::BreakStatement(Node* breakValue)
	{
		m_Type = NodeType::Break;
		m_BreakValue = breakValue;
	}
	void BreakStatement::Print(std::string padding)
	{
		std::cout << padding << TypeToString() << ": \n";
		if (m_BreakValue)
			m_BreakValue->Print(padding + "    ");
	}

	ClassDeclarationStatement::ClassDeclarationStatement(Identifier* name)
	{
		m_Type = NodeType::ClassDeclaration;
		m_Name = name;
	}

	void ClassDeclarationStatement::Print(std::string padding)
	{
		std::string newPadding = padding + "        ";
		std::cout << padding << TypeToString() << ": \n";

		m_Name->Print(padding + "    ");

		std::cout << padding + "    (member variables): \n";
		for (auto& node : m_MemberDeclarations)
			node->Print(newPadding);
		std::cout << padding + "    (methods): \n";
		for (auto& node : m_MethodDeclarations)
			node->Print(newPadding);
		std::cout << padding + "    (nested classes): \n";
		for (auto& node : m_NestedClassDeclarations)
			node->Print(newPadding);
	}

	TupleExpression::TupleExpression(std::vector<Node*> elements)
	{
		m_Type = NodeType::TupleExpression;
		m_Elements = elements;
	}

	void TupleExpression::Print(std::string padding)
	{
		std::cout << padding << TypeToString() << " (" << m_Elements.size() << "): \n";

		for (auto& element : m_Elements)
			element->Print(padding + "    ");
	}

	LambdaExpression::LambdaExpression(Type* returnType, TupleExpression* parameters, Node* body)
	{
		m_Type = NodeType::LambdaExpression;
		m_ReturnType = returnType;
		m_Parameters = parameters;
		m_Body = body;
	}
	void LambdaExpression::Print(std::string padding)
	{
		std::string newPadding = padding + "        ";
		std::cout << padding << TypeToString() << ": \n";

		std::cout << padding + "    (return type): \n";
		if (m_ReturnType) m_ReturnType->Print(newPadding);

		std::cout << padding + "    (parameters): \n";
		m_Parameters->Print(newPadding);

		std::cout << padding + "    (body): \n";
		if (m_Body)
			m_Body->Print(newPadding);
	}
}