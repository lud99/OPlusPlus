#pragma once

#include "../Parser.h"

#include "../Interpreter/ValueTypes.h";

#include <unordered_map>

class Section
{
public:
	void AddLine(std::string line, const std::string& comment = "");

	std::vector<std::string>& GetLines() { return m_Lines; }

private:
	std::vector<std::string> m_Lines;
};


class AssemblyCompilerContext
{
public:
	struct Variable
	{
		std::string m_Name = "";
		uint32_t m_Index = 0;

		bool m_IsGlobal = false;

		ValueTypes m_Type = ValueTypes::Void;

		//Variable(int index = 0, std::string name = "", ValueTypes type = ValueTypes::Void, bool isGlobal = false) : m_Name(name), m_Index(index), m_Type(type), m_IsGlobal(isGlobal) {};
	};

	struct LoopInfo
	{
		bool m_InLoop = false;
		int m_Reset = -1;
		int m_End = -1;
		int m_BodyDepth = -1;
	};

public:
	bool HasVariable(const std::string& variableName);
	Variable& GetVariable(const std::string& variableName);
	Variable& CreateVariable(const std::string& variableName, ValueTypes type);

public:
	std::unordered_map<std::string, Variable> m_Variables;

	uint32_t m_NextFreeVariableIndex = 4;

	LoopInfo m_LoopInfo;
};

class AssemblyCompiler
{
public:
	AssemblyCompiler() {};

	void Compile(ASTNode* node);

	void MakeError(std::string error) { m_Error = error; }

public:
	Section m_DataSection;
	Section m_TextSection;

	AssemblyCompilerContext m_Context;

	std::string m_Error;
};