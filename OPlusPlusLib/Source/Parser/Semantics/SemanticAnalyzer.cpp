#include "SemanticAnalyzer.h"

namespace O
{
	using namespace AST;

	SemanticAnalyzer::SemanticAnalyzer(AST::Node* program)
	{
		m_Program = program;
	}

	// Create symbol tables for each scope
	void SemanticAnalyzer::Analyze(AST::Node* node, SymbolTable* upwardSymbolTable, SymbolTable* localSymbolTable)
	{
		switch (node->m_Type)
		{
		case O::AST::NodeType::EmptyStatement:
			return;
		case O::AST::NodeType::Program:
		case O::AST::NodeType::BlockStatement:
		{
			Scope* scopeNode = (Scope*)node;

			bool isGlobal = node->m_Type == NodeType::Program;
			scopeNode->m_LocalSymbolTable = 
				SymbolTable(isGlobal ? SymbolTableType::Global : SymbolTableType::Local, upwardSymbolTable);

			for (auto& line : scopeNode->m_Lines)
			{
				Analyze(line, upwardSymbolTable, &scopeNode->m_LocalSymbolTable);
			}

			break;
		}
		case O::AST::NodeType::BasicType:
			break;
		case O::AST::NodeType::Identifier:
			break;

		// Perform typechecking and create the variable symbol
		case O::AST::NodeType::VariableDeclaration:
		{
			VariableDeclaration* variableDeclaration = (VariableDeclaration*)node;

			NodeType variableType = variableDeclaration->m_VariableType->m_Type;

			if (variableType == NodeType::BasicType)
			{
				variableDeclaration->m_VariableType
			}
			else if (variableType == NodeType::ArrayType)
			{

			}

			//const std::string& typeName = variableDeclaration->m_VariableType->m_Type;
			//if (!m_TypeTable.HasType(typeName))
			//	abort(); // TODO: error

			break;
		}
		case O::AST::NodeType::AssignmentExpression:
			break;
		case O::AST::NodeType::BinaryExpression:
			break;
		case O::AST::NodeType::UnaryExpression:
			break;
		case O::AST::NodeType::CallExpression:
			break;
		case O::AST::NodeType::TupleExpression:
			break;
		case O::AST::NodeType::FunctionDefinition:
			break;
		case O::AST::NodeType::ExpressionFunctionDefinition:
			break;
		case O::AST::NodeType::IfStatement:
			break;
		case O::AST::NodeType::WhileStatement:
			break;
		case O::AST::NodeType::ForStatement:
			break;
		case O::AST::NodeType::LoopStatement:
			break;
		case O::AST::NodeType::Closure:
			break;
		case O::AST::NodeType::Continue:
			break;
		case O::AST::NodeType::Break:
			break;
		case O::AST::NodeType::Return:
			break;
		case O::AST::NodeType::ClassDeclaration:
			break;
		case O::AST::NodeType::IntLiteral:
			break;
		case O::AST::NodeType::FloatLiteral:
			break;
		case O::AST::NodeType::DoubleLiteral:
			break;
		case O::AST::NodeType::BoolLiteral:
			break;
		case O::AST::NodeType::StringLiteral:
			break;
		default:
			break;
		}
	}

	ValueType SemanticAnalyzer::GetTypeOfNode(AST::Node* node)
	{
		switch (node->m_Type)
		{
		case O::AST::NodeType::EmptyStatement:
			return PrimitiveValueTypes::Void;

		case O::AST::NodeType::Program:
		case O::AST::NodeType::BlockStatement:
			return PrimitiveValueTypes::Void;

		case O::AST::NodeType::BasicType:
		{
			BasicType* basicType = (BasicType*)node;
			m_TypeTable.GetType(basicType->m_TypeName);
		}
		case O::AST::NodeType::ArrayType:
			break;
		case O::AST::NodeType::TupleType:
			break;
		case O::AST::NodeType::FunctionType:
			break;

		case O::AST::NodeType::Identifier:
			break;

			// Perform typechecking and create the variable symbol
		case O::AST::NodeType::VariableDeclaration:
		{
			VariableDeclaration* variableDeclaration = (VariableDeclaration*)node;

			NodeType variableType = variableDeclaration->m_VariableType->m_Type;

			if (variableType == NodeType::BasicType)
			{
				variableDeclaration->m_VariableType
			}
			else if (variableType == NodeType::ArrayType)
			{

			}

			//const std::string& typeName = variableDeclaration->m_VariableType->m_Type;
			//if (!m_TypeTable.HasType(typeName))
			//	abort(); // TODO: error

			break;
		}
		case O::AST::NodeType::AssignmentExpression:
			break;
		case O::AST::NodeType::BinaryExpression:
			break;
		case O::AST::NodeType::UnaryExpression:
			break;
		case O::AST::NodeType::CallExpression:
			break;
		case O::AST::NodeType::TupleExpression:
			break;
		case O::AST::NodeType::FunctionDefinition:
			break;
		case O::AST::NodeType::ExpressionFunctionDefinition:
			break;
		case O::AST::NodeType::IfStatement:
			break;
		case O::AST::NodeType::WhileStatement:
			break;
		case O::AST::NodeType::ForStatement:
			break;
		case O::AST::NodeType::LoopStatement:
			break;
		case O::AST::NodeType::Closure:
			break;
		case O::AST::NodeType::Continue:
			break;
		case O::AST::NodeType::Break:
			break;
		case O::AST::NodeType::Return:
			break;
		case O::AST::NodeType::ClassDeclaration:
			break;
		case O::AST::NodeType::IntLiteral:
			break;
		case O::AST::NodeType::FloatLiteral:
			break;
		case O::AST::NodeType::DoubleLiteral:
			break;
		case O::AST::NodeType::BoolLiteral:
			break;
		case O::AST::NodeType::StringLiteral:
			break;
		default:
			break;
		}
	}
}
