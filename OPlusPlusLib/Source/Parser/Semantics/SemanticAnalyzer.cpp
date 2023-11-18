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

			TypeTableEntry& variableType = GetTypeOfNode(variableDeclaration->m_VariableType);

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

	TypeTableEntry& SemanticAnalyzer::GetTypeOfNode(AST::Node* node)
	{
		switch (node->m_Type)
		{
		case O::AST::NodeType::EmptyStatement:
			return m_TypeTable.GetType(PrimitiveValueTypes::Void);

		case O::AST::NodeType::Program:
		case O::AST::NodeType::BlockStatement:
			return m_TypeTable.GetType(PrimitiveValueTypes::Void);

		case O::AST::NodeType::BasicType:
		{
			BasicType* basicType = (BasicType*)node;
			return m_TypeTable.GetType(basicType->m_TypeName);
		}
		case O::AST::NodeType::ArrayType:
		{
			ArrayType* arrType = (ArrayType*)node;

			TypeTableEntry& type = GetTypeOfNode(arrType->m_UnderlyingType);
			return m_TypeTable.AddArray(type);
		}
		case O::AST::NodeType::TupleType:
		{
			TupleType* tupleType = (TupleType*)node;

			std::vector<TypeTableEntry> elementTypes;
			for (AST::Type* element : tupleType->m_Elements)
			{
				elementTypes.push_back(GetTypeOfNode(element));
			}

			return m_TypeTable.AddTuple(elementTypes);
		}
		case O::AST::NodeType::FunctionType:
		{
			FunctionType* functionType = (FunctionType*)node;

			std::vector<TypeTableEntry> parameterTypes;
			for (AST::Type* parameter : functionType->m_Parameters)
			{
				parameterTypes.push_back(GetTypeOfNode(parameter));
			}
			TypeTableEntry returnType = GetTypeOfNode(functionType->m_ReturnType);

			return m_TypeTable.AddFunction(parameterTypes, returnType);
		}

		case O::AST::NodeType::Identifier:
			break;

			// Perform typechecking and create the variable symbol
		case O::AST::NodeType::VariableDeclaration:
			return m_TypeTable.GetType(PrimitiveValueTypes::Void);

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
	SemanticAnalyzer::~SemanticAnalyzer()
	{
	}
}
