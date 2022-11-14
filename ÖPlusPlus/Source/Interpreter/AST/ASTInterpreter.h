#pragma once

#include "../../Parser.h"
#include "../ValueTypes.h"
#include <string>
#include "../Value.h"

#include <vector>

#include <map>

namespace ASTint 
{

constexpr int ScopeFramesCount = 64;

class ScopeFrame 
{
public:
	bool HasVariable(const std::string& name);
	Value& GetVariable(const std::string& name);
	void SetVariable(const std::string& name, Value value);
	Value& CreateVariable(const std::string& name, Value value);

public:
	std::map<std::string, Value> m_Variables;
};

class ASTInterpreter
{
public:
	static ASTInterpreter& Get();
	void Initialize(ASTNode* tree);
	Value Execute(ASTNode* tree);

	void MakeError(std::string error);
	Value MakeErrorValueReturn(std::string error);

	Value InterpretTree(ASTNode* node);

private:
	ASTInterpreter() {};

	ScopeFrame& PushFrame();
	ScopeFrame PopFrame();
	ScopeFrame& GetTopFrame();

	std::string ResolveVariableName(ASTNode* node);
	void InheritVariables(ScopeFrame& previous, ScopeFrame& current);

private:
	std::vector<ScopeFrame> m_ScopeFrames;
	int m_ScopeFrameTop = 0;

public:
	ASTNode* m_ASTTree = nullptr;

	std::string m_Error = "";
};

}