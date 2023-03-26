#pragma once

#include "../../Parser.h"
#include "../ValueTypes.h"
#include <string>
#include "../Value.h"

#include <vector>

#include <map>
#include <stack>

namespace AST 
{

constexpr int ScopeFramesCount = 5000;

class ScopeFrame 
{
public:
	bool HasVariable(const std::string& name);
	Value& GetVariable(const std::string& name);
	void SetVariable(const std::string& name, Value value);
	Value& CreateVariable(const std::string& name, Value value);

	bool HasFunction(const std::string& name);
	ASTNode* GetFunction(const std::string& name);
	void CreateFunction(const std::string& name, ASTNode* start);

public:
	std::map<std::string, Value> m_Variables;
	std::map<std::string, Value> m_InitialVariables;
	std::map<std::string, ASTNode*> m_Functions;

	std::vector<Value> m_ArgumentsForFunction;
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
	void InheritGlobalVariables(ScopeFrame& previous, ScopeFrame& current);
	void PropagateVariables(ScopeFrame& previous, ScopeFrame& current);

private:
	std::vector<ScopeFrame> m_ScopeFrames;
	int m_ScopeFrameTop = 0;

	bool m_ShouldReturn = false;

public:
	ASTNode* m_ASTTree = nullptr;

	std::string m_Error = "";
};

}