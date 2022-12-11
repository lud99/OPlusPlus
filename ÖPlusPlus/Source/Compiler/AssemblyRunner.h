#pragma once

#include "AssemblyCompiler.h"

#include <string>

class AssemblyRunner
{
public:
	AssemblyRunner() {};

	AssemblyRunner(const std::string& filepath);

	void Compile();

	std::string Execute();

	const std::string& GetCompiledCode() { return m_Code; }
private:
	AssemblyCompiler m_Compiler;

	std::string m_Filepath;

	std::string m_Code;
};