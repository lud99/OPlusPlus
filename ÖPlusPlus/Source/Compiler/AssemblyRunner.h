#pragma once

#include "AssemblyCompiler.h"

#include <string>

namespace ASM {
	class AssemblyRunner
	{
	public:
		AssemblyRunner() {};

		AssemblyRunner(const std::string& fileContent, const std::string& buildDir);

		std::string Compile(bool quiet = false);

		std::string Execute(bool quiet = false);

		const std::string& GetCompiledCode() { return m_Code; }

		ASM::AssemblyCompiler& GetCompiler() { return m_Compiler; }
	private:
		ASM::AssemblyCompiler m_Compiler;

		std::string m_FileContent;

		std::string m_BuildDir;

		std::string m_Code;
	};
}