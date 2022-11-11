#pragma once

#include "../../Parser.h"
#include "../ValueTypes.h"
#include <string>
#include "Value.h"

#include <map>

namespace ASTint 
{

class ScopeFrame 
{
	std::map<std::string, ASTint::Value> m_Variables;
};

class ASTInterpreter
{
public:
	static ASTInterpreter& Get();
	ASTInterpreter(ASTNode* tree);

	void MakeError(std::string error);
	Value MakeErrorValueReturn(std::string error);

	Value InterpretTree(ASTNode* node);

private:
	ASTInterpreter() {};

public:
	ASTNode* m_ASTTree = nullptr;

	std::string m_Error = "";
};

}