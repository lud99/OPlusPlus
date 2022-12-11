#pragma once

#include "../Parser.h"

#include "../Interpreter/ValueTypes.h";

#include <unordered_map>

struct Instruction
{
	std::string m_Op;
	std::string m_Dest;
	std::string m_Src;

	std::string m_Comment;

	bool m_IsLabel = false;
	bool m_IsOnlyComment = false;

	Instruction() {};
	Instruction(std::string op, std::string dest = "", std::string src = "", std::string comment = "");

	std::string ToString();
};

class Section
{
public:
	void AddInstruction(Instruction inst);
	void AddInstruction(std::string op, std::string dest = "", std::string src = "", std::string comment = "");
	void AddComment(const std::string& comment);
	void AddLabel(const std::string& label);

	void AddCorrectMathInstruction(ASTNode* n, bool reverse = false);

	std::vector<Instruction>& GetLines() { return m_Lines; }

private:
	std::vector<Instruction> m_Lines;
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

	struct Function
	{
		std::string m_Name = "";
		ValueTypes m_ReturnType = ValueTypes::Void;

		//std::vector<Variable> m_Arguments;
	};

	struct LoopInfo
	{
		bool m_InLoop = false;
		int m_Reset = -1;
		int m_End = -1;
		int m_BodyDepth = -1;
		int labelIndex = 0;
	};

public:
	bool HasVariable(const std::string& variableName);
	Variable& GetVariable(const std::string& variableName);
	Variable& CreateVariable(const std::string& variableName, ValueTypes type, int size = 4);

	bool HasFunction(const std::string& functionName);
	Function& GetFunction(const std::string& functionName);
	Function& CreateFunction(const std::string& functionName, ValueTypes returnType /* todo args*/);

	int Allocate(int size);

public:
	std::unordered_map<std::string, Variable> m_Variables;
	std::unordered_map<std::string, Function> m_Functions;

	uint32_t m_CurrentVariableIndex = 0;

	LoopInfo m_LoopInfo;
};

class AssemblyCompiler
{
public:
	AssemblyCompiler() {};

	void Compile(ASTNode* node);
	void Optimize();

	void MakeError(std::string error) { m_Error = error; }

public:
	Section m_DataSection;
	Section m_TextSection;

	AssemblyCompilerContext m_Context;

	std::string m_Error;
};