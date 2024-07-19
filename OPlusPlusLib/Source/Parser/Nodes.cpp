#include "Nodes.h"

#include "../Utils.hpp"

#include "Semantics/SemanticAnalyzer.h";

namespace O::AST
{
	std::string Node::TypeToString()
	{
		return std::string(magic_enum::enum_name(m_Type));
	}
	
	void Node::Print(std::string padding, SymbolTypeTable* table, SemanticAnalyzer* analyzer)
	{
		std::cout << padding << TypeToString() << (ToString() != "" ? ": " : "") << ToString() << "\n";
	}
};

namespace O::AST::Nodes 
{
	std::string CachedTypeToString(Node* node, SymbolTypeTable* table, SemanticAnalyzer* analyzer)
	{
		if (analyzer->HasTableForNode(node))
			table = &analyzer->GetSymbolTypeTableForNode(node);
		
		assert(table);

		const O::Type* type = analyzer->GetTypeOfExpression(node, *table);
		if (!type)
			return "<unknown type>";

		return type->GetName(&table->types);
	}
	std::string SymbolDataTypeToString(std::string symbolName, SymbolTypeTable* table)
	{
		return table->types.Lookup(table->symbols.LookupOne(symbolName)->m_DataType)->GetName(&table->types);
	}

	std::string GetSourceCodeForNode(Node* node, SemanticAnalyzer* analyzer)
	{
		if (!analyzer) return "";

		// TODO: Doesn't work when comments exists in the file, also its broken for classes right now
		auto range = analyzer->GetTokenRangeForNode(node);

		auto& src = analyzer->GetSourceCode();
		
		int length = range.m_End.index - range.m_Start.index + 1;
		return "[" + analyzer->GetSourceCode().substr(range.m_Start.index, length) + "]\n";
	}

	std::string GetT(Node* node, SymbolTypeTable* table, SemanticAnalyzer* analyzer)
	{
		if (analyzer)
			return " -> " + CachedTypeToString(node, table, analyzer) + "\n";

		assert(!analyzer);
		return "\n";
	}

	std::string GetA(CallableSignature signature, SymbolTypeTable& table)
	{
		return "(" + Join(signature.parameterTypes, ", ",
			[&](TypeId& id) {
				return table.types.Lookup(id)->GetName(&table.types);
			}) + ")";
	}

	std::string MaybeLookupName(SymbolTypeTable* table, TypeId typeId)
	{
		if (!table) return "<unknown type>";
		return table->types.Lookup(typeId)->GetName(&table->types);
	}

	Identifier::Identifier(const std::string& name)
	{
		m_Name = name;
		m_Type = NodeKind::Identifier;
	}

	void Identifier::Print(std::string padding, SymbolTypeTable* table, SemanticAnalyzer* analyzer)
	{
		std::cout << padding << GetSourceCodeForNode(this, analyzer);
		std::cout << padding << TypeToString() << ": " << ToString() << GetT(this, table, analyzer);
	}

	BasicType::BasicType(const std::string& typeName)
	{
		m_TypeName = typeName;
		m_Type = NodeKind::BasicType;
	}

	std::string BasicType::ToString()
	{
		std::string str = m_TypeName;
		if (m_IsNullable)
			str += "?";

		return str;
	}

	/*void BasicType::Print(std::string padding, SymbolTypeTable* table, SemanticAnalyzer* analyzer)
	{
		std::cout << padding << TypeToString() << ": " << ToString() << "\n";
		if (m_IsArray)
			std::cout << "(array)";
	}*/

	VariableDeclaration::VariableDeclaration(Identifier* variableName, Type* variableType, Node* assignedValue)
	{
		m_Type = NodeKind::VariableDeclaration;

		m_VariableName = variableName;
		m_VariableType = variableType;
		m_AssignedValue = assignedValue;
	}

	void VariableDeclaration::Print(std::string padding, SymbolTypeTable* table, SemanticAnalyzer* analyzer)
	{
		std::cout << padding << GetSourceCodeForNode(this, analyzer);
		std::cout << padding << TypeToString() << "\n";
		m_VariableName->Print(padding + "    ", table, analyzer);

		if (m_VariableType)
			m_VariableType->Print(padding + "    ", table, analyzer);
		else
			std::cout << padding << "    (infered type)\n";

		if (m_AssignedValue)
		{
			std::cout << padding + "    (value)\n";
			m_AssignedValue->Print(padding + "        ", table, analyzer);
		}
	}

	BinaryExpression::BinaryExpression(Node* left, Operators::Operator op, Node* right)
	{
		m_Type = NodeKind::BinaryExpression;
		m_Operator = op;

		m_Lhs = left;
		m_Rhs = right;
	}

	void BinaryExpression::Print(std::string padding, SymbolTypeTable* table, SemanticAnalyzer* analyzer)
	{
		std::cout << padding << GetSourceCodeForNode(this, analyzer);
		std::cout << padding << TypeToString() << " (" << ToString() << ")" << GetT(this, table, analyzer);

		m_Lhs->Print(padding + "    ", table, analyzer);
		m_Rhs->Print(padding + "    ", table, analyzer);
	}

	std::string BinaryExpression::ToString()
	{
		return std::string(magic_enum::enum_name(m_Operator.m_Name)) + " " + m_Operator.m_Symbol;
	}

	UnaryExpression::UnaryExpression(Node* operand, Operators::Operator op)
	{
		m_Type = NodeKind::UnaryExpression;
		m_Operator = op;
		m_Operand = operand;
	}

	void UnaryExpression::Print(std::string padding, SymbolTypeTable* table, SemanticAnalyzer* analyzer)
	{
		std::cout << padding << GetSourceCodeForNode(this, analyzer);
		std::cout << padding << TypeToString() << "(" << ToString() << ")" + GetT(this, table, analyzer);

		m_Operand->Print(padding + "    ", table, analyzer);
	}

	std::string UnaryExpression::ToString()
	{
		return std::string(magic_enum::enum_name(m_Operator.m_Name)) + m_Operator.m_Symbol;
	}

	CallExpression::CallExpression(Node* callee, std::vector<Node*> arguments)
	{
		m_Type = NodeKind::CallExpression;
		m_Callee = callee;
		m_Arguments = arguments;
	}

	void CallExpression::Print(std::string padding, SymbolTypeTable* table, SemanticAnalyzer* analyzer)
	{
		std::cout << padding << TypeToString() << " ";

		if (analyzer)
		{
			CallableSignature signature = analyzer->GetCachedTypes()[this];

			std::cout << GetA(signature, *table) << " -> ";
			std::cout << MaybeLookupName(table, signature.returnType);
		}
		std::cout << "\n";

		m_Callee->Print(padding + "    ", table, analyzer);
		
		for (Node* argument : m_Arguments)
		{
			argument->Print(padding + "    ", table, analyzer);
		}
	}

	IntLiteral::IntLiteral(int value)
	{
		m_Value = value;
		m_Type = NodeKind::IntLiteral;
	}

	FloatLiteral::FloatLiteral(float value)
	{
		m_Value = value;
		m_Type = NodeKind::FloatLiteral;
	}

	DoubleLiteral::DoubleLiteral(double value)
	{
		m_Value = value;
		m_Type = NodeKind::DoubleLiteral;
	}
	BoolLiteral::BoolLiteral(bool value)
	{
		m_Value = value;
		m_Type = NodeKind::BoolLiteral;
	}

	StringLiteral::StringLiteral(std::string value)
	{
		m_Value = value;
		m_Type = NodeKind::StringLiteral;
	}

	WhileStatement::WhileStatement(Node* condition, BlockStatement* body)
	{
		m_Condition = condition;
		m_Body = body;
		m_Type = NodeKind::WhileStatement;
	}
	IfStatement::IfStatement(Node* condition, BlockStatement* body, BlockStatement* elseArm)
	{
		m_Condition = condition;

		m_Body = body;
		m_ElseArm = elseArm;
		m_Type = NodeKind::IfStatement;
	}
	void IfStatement::Print(std::string padding, SymbolTypeTable* table, SemanticAnalyzer* analyzer)
	{
		std::string newPadding = padding + "        ";
		std::cout << padding << TypeToString() << ": \n";

		std::cout << padding + "    (condition) \n";
		m_Condition->Print(newPadding, table, analyzer);

		std::cout << padding + "    (body) \n";
		m_Body->Print(newPadding, &analyzer->GetSymbolTypeTableForNode(m_Body), analyzer);

		if (m_ElseArm)
		{
			std::cout << padding + "    (else) \n";
			m_ElseArm->Print(newPadding, &analyzer->GetSymbolTypeTableForNode(m_ElseArm), analyzer);
		}
	}

	ForStatement::ForStatement(Node* initialization, Node* condition, Node* advancement, BlockStatement* body)
	{
		m_Type = NodeKind::ForStatement;

		m_Initialization = initialization;
		m_Condition = condition;
		m_Advancement = advancement;
		m_Body = body;
	}
	void ForStatement::Print(std::string padding, SymbolTypeTable* table, SemanticAnalyzer* analyzer)
	{
		std::string newPadding = padding + "        ";
		std::cout << padding << TypeToString() << " \n";

		std::cout << padding + "    (initialization) \n";
		if (m_Initialization) m_Initialization->Print(newPadding, table, analyzer);

		std::cout << padding + "    (condition) \n";
		if (m_Condition) m_Condition->Print(newPadding, table, analyzer);

		std::cout << padding + "    (advancement) \n";
		if (m_Advancement) m_Advancement->Print(newPadding, table, analyzer);

		std::cout << padding + "    (body) \n";
		if (m_Body) m_Body->Print(newPadding, &analyzer->GetSymbolTypeTableForNode(m_Body), analyzer);
	}
	void ConditionalStatement::Print(std::string padding, SymbolTypeTable* table, SemanticAnalyzer* analyzer)
	{
		std::string newPadding = padding + "        ";
		std::cout << padding << TypeToString() << ": \n";

		std::cout << padding + "    (condition) \n";
		m_Condition->Print(newPadding, table, analyzer);

		std::cout << padding + "    (body) \n";
		m_Body->Print(newPadding, &analyzer->GetSymbolTypeTableForNode(m_Body), analyzer);
	}
	void Scope::Print(std::string padding, SymbolTypeTable* table, SemanticAnalyzer* analyzer)
	{
		std::cout << padding << TypeToString() << ": \n";

		std::string newPadding = padding + "        ";

		SymbolTypeTable* localTable = nullptr;
		if (analyzer)
		{
			localTable = &analyzer->GetSymbolTypeTableForNode(this);
			if (!localTable->symbols.GetSymbols().empty())
			{
				std::cout << padding << "    " << (m_Type == NodeKind::Program ? "Global type table" : "Local type table") << ": \n";
				localTable->types.Print(newPadding);
				std::cout << "\n";
			}
			
			if (!localTable->types.GetTypes().empty())
			{
				std::cout << padding << "    " << (m_Type == NodeKind::Program ? "Global symbol table" : "Local symbol table") << ": \n";
				localTable->symbols.Print(localTable->types, newPadding);
				std::cout << "\n";
			}
		}

		std::cout << padding << "    (lines)\n";
		for (auto& line : m_Lines)
			line->Print(newPadding, localTable, analyzer);
	}

	ReturnStatement::ReturnStatement(Node* returnValue)
	{
		m_Type = NodeKind::Return;
		m_ReturnValue = returnValue;
	}

	void ReturnStatement::Print(std::string padding, SymbolTypeTable* table, SemanticAnalyzer* analyzer)
	{
		std::cout << padding << TypeToString() << " \n";
		if (m_ReturnValue)
			m_ReturnValue->Print(padding + "    ", table, analyzer);
	}

	FunctionDefinitionStatement::FunctionDefinitionStatement(Type* returnType, Identifier* name, FunctionParameters* parameters, Node* body, bool isLambda)
	{
		m_Type = NodeKind::FunctionDefinition;
		m_ReturnType = returnType;
		m_Name = name;
		m_Parameters = parameters;
		m_Body = body;
		m_IsLambda = isLambda;
	}

	void FunctionDefinitionStatement::Print(std::string padding, SymbolTypeTable* table, SemanticAnalyzer* analyzer)
	{
		std::string newPadding = padding + "        ";
		std::cout << padding << TypeToString() << "\n";

		std::string returnTypeText = "";
		SymbolTypeTable* parametersTable = nullptr;
		if (analyzer)
		{
			std::cout << padding << "    Parameters symbol table" << ": \n";

			parametersTable = &analyzer->GetSymbolTypeTableForNode(this);

			parametersTable->symbols.Print(parametersTable->types, newPadding);
			std::cout << "\n";

			CallableSignature signature = analyzer->GetCachedTypes()[this];
			returnTypeText = " -> " + MaybeLookupName(table, signature.returnType);
		}

		if (m_ReturnType)
		{
			std::cout << padding << "    (return type)" << returnTypeText << "\n";
			m_ReturnType->Print(newPadding, table, analyzer);
		}
		else
		{
			std::cout << padding << "    (infered return type)" << returnTypeText << "\n";
		}

		std::cout << padding + "    (name) \n";
		if (m_Name) m_Name->Print(newPadding, table, analyzer);

		std::cout << padding << "    (parameters) ";
		if (analyzer)
		{
			CallableSignature signature = analyzer->GetCachedTypes()[this];
			std::cout << GetA(signature, *table);
		}
		
		std::cout << "\n";
		m_Parameters->Print(newPadding, parametersTable, analyzer);

		std::cout << padding + "    (name) \n";
		if (m_Name) m_Name->Print(newPadding, table, analyzer);

		std::cout << padding + "    (body) \n";
		if (m_Body) m_Body->Print(newPadding, IsExpression() ? table : nullptr, analyzer);
	}

	ClosureExpression::ClosureExpression(BlockStatement* body)
	{
		m_Type = NodeKind::Closure;
		m_Body = body;
	}

	void ClosureExpression::Print(std::string padding, SymbolTypeTable* table, SemanticAnalyzer* analyzer)
	{
		std::cout << padding << TypeToString() << " \n";
		m_Body->Print(padding + "    ", &analyzer->GetSymbolTypeTableForNode(m_Body), analyzer);
	}

	LoopStatement::LoopStatement(BlockStatement* body)
	{
		m_Type = NodeKind::LoopStatement;
		m_Body = body;
	}

	void LoopStatement::Print(std::string padding, SymbolTypeTable* table, SemanticAnalyzer* analyzer)
	{
		std::cout << padding << TypeToString() << " \n";
		m_Body->Print(padding + "    ", table, analyzer);
	}
	BreakStatement::BreakStatement(Node* breakValue)
	{
		m_Type = NodeKind::Break;
		m_BreakValue = breakValue;
	}
	void BreakStatement::Print(std::string padding, SymbolTypeTable* table, SemanticAnalyzer* analyzer)
	{
		std::cout << padding << TypeToString() << " \n";
		if (m_BreakValue)
			m_BreakValue->Print(padding + "    ", table, analyzer);
	}

	ClassDeclarationStatement::ClassDeclarationStatement(Identifier* name)
	{
		m_Type = NodeKind::ClassDeclaration;
		m_Name = name;
	}

	void ClassDeclarationStatement::Print(std::string padding, SymbolTypeTable* table, SemanticAnalyzer* analyzer)
	{
		std::string newPadding = padding + "        ";
		std::cout << padding << TypeToString() << "\n";

		std::cout << padding + "    (class symbols)\n";
		if (m_ClassSymbol)
			m_ClassSymbol->m_Table->symbols.Print(m_ClassSymbol->m_Table->types, newPadding);

		std::cout << "\n" << padding + "    (class types) \n";
		if (m_ClassSymbol)
			m_ClassSymbol->m_Table->types.Print(newPadding);
		std::cout << "\n";

		SymbolTypeTable* classTable = m_ClassSymbol ? m_ClassSymbol->m_Table : nullptr;

		m_Name->Print(padding + "    ", table, analyzer);

		std::cout << padding + "    (member variables) \n";
		for (auto& node : m_MemberDeclarations)
			node->Print(newPadding, classTable, analyzer);
		std::cout << padding + "    (methods) \n";
		for (auto& node : m_MethodDeclarations)
			node->Print(newPadding, classTable, analyzer);
		std::cout << padding + "    (nested classes) \n";
		for (auto& node : m_NestedClassDeclarations)
			node->Print(newPadding, classTable, analyzer);
	}

	TupleExpression::TupleExpression(std::vector<Node*> elements)
	{
		m_Type = NodeKind::TupleExpression;
		m_Elements = elements;
	}

	void TupleExpression::Print(std::string padding, SymbolTypeTable* table, SemanticAnalyzer* analyzer)
	{
		std::cout << padding << TypeToString() << " (" << m_Elements.size() << ") \n";

		for (auto& element : m_Elements)
			element->Print(padding + "    ", table, analyzer);
	}

	
	FunctionType::FunctionType(std::vector<Type*> parameters, Type* returnType)
	{
		m_Type = NodeKind::FunctionType;
		m_Parameters = parameters;
		m_ReturnType = returnType;
	}

	void FunctionType::Print(std::string padding, SymbolTypeTable* table, SemanticAnalyzer* analyzer)
	{
		std::string newPadding = padding + "        ";
		std::cout << padding << TypeToString() << ": \n";

		std::cout << padding + "    (parameter types): \n";
		for (auto& type : m_Parameters)
		{
			type->Print(newPadding, table, analyzer);
		}

		std::cout << padding + "    (return type): \n";
		m_ReturnType->Print(newPadding, table, analyzer);
	}

	TupleType::TupleType(std::vector<Type*> elements)
	{
		m_Type = NodeKind::TupleType;
		m_Elements = elements;
	}

	void TupleType::Print(std::string padding, SymbolTypeTable* table, SemanticAnalyzer* analyzer)
	{
		std::string newPadding = padding + "        ";
		std::cout << padding << TypeToString() << ": \n";

		for (auto& type : m_Elements)
		{
			type->Print(newPadding, table, analyzer);
		}
	}
	FunctionParameters::FunctionParameters(std::vector<VariableDeclaration*> parameters)
	{
		m_Type = NodeKind::FunctionParameters;
		m_Parameters = parameters;
	}
	void FunctionParameters::Print(std::string padding, SymbolTypeTable* table, SemanticAnalyzer* analyzer)
	{
		for (auto& parameter : m_Parameters)
		{
			parameter->Print(padding, table, analyzer);
		}
	}
	ArrayType::ArrayType(Type* underlyingType)
	{
		m_Type = NodeKind::ArrayType;
		m_UnderlyingType = underlyingType;
	}
	/*std::string ArrayType::ToString()
	{
		return ->ToString();
	}*/
	void ArrayType::Print(std::string padding, SymbolTypeTable* table, SemanticAnalyzer* analyzer)
	{
		std::cout << padding << TypeToString() << GetT(this, table, analyzer);

		m_UnderlyingType->Print(padding + "    ", table, analyzer);
	}
	ArrayLiteral::ArrayLiteral(std::vector<Node*> elements)
	{
		m_Type = NodeKind::ArrayLiteral;
		m_Elements = elements;
	}

	std::string ArrayLiteral::ToString()
	{
		return "[" + Join(m_Elements, ", ", [](Node* t) { return t->ToString(); }) + "]";
	}

	void Literal::Print(std::string padding, SymbolTypeTable* table, SemanticAnalyzer* analyzer)
	{
		std::cout << padding << GetSourceCodeForNode(this, analyzer);
		std::cout << padding << TypeToString() << ": " << ToString() << GetT(this, table, analyzer);
	}
}