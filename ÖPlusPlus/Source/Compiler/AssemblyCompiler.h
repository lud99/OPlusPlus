#pragma once

#include "../Parser.h"

#include "../Interpreter/ValueTypes.h";

#include <unordered_map>

#ifdef ASM

class ConstantsPool
{
public:

	int GetFloatIndex(float value);
	int StoreFloat(float value);
	bool HasFloat(float value);

private:
	std::unordered_map<std::string, int> m_FloatConstants;

	int m_FloatIndex = 0;
};


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
	void AddLine(const std::string& line);

	void AddCorrectMathInstruction(ASTNode* n, bool reverse = false);

	std::vector<Instruction>& GetLines() { return m_Lines; }

private:
	std::vector<Instruction> m_Lines;
};

enum class Publicity
{
	Local,
	Global
};

class AssemblyCompilerContext;
class AssemblyCompilerContext
{
public:
	struct Variable
	{
		std::string m_Name = "";
		std::string m_MangledName = "";
		uint32_t m_Index = 0;

		Publicity m_Publicity;

		ValueTypes m_Type = ValueTypes::Void;

		std::string GetASMLocation(const std::string& datatype = "");

		//Variable(int index = 0, std::string name = "", ValueTypes type = ValueTypes::Void, bool isGlobal = false) : m_Name(name), m_Index(index), m_Type(type), m_IsGlobal(isGlobal) {};
	};

	struct Function
	{
		std::string m_Name = "";
		std::string m_MangledName = "";
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
	Variable& CreateVariable(const std::string& variableName, ValueTypes type, int size = 4, Publicity publicity = Publicity::Local);
	bool DeleteVariable(const std::string& variableName);

	bool HasFunction(const std::string& functionName);
	Function& GetFunction(const std::string& functionName);
	Function& CreateFunction(const std::string& functionName, ValueTypes returnType /* todo args*/);

	int Allocate(int size);

public:
	std::unordered_map<std::string, Variable> m_Variables;
	std::unordered_map<std::string, Function> m_Functions;

	uint32_t m_CurrentVariableIndex = 0;

	LoopInfo m_LoopInfo;

	AssemblyCompilerContext* m_GlobalContext = nullptr;
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
	AssemblyCompilerContext m_GlobalContext;

	ConstantsPool m_Constants;

	std::string m_Error;
};

#endif