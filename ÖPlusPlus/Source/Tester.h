#pragma once

#include <vector>

#include "Compiler/AssemblyRunner.h"

class Tester
{
public:
	Tester(const std::string& buildDir);

	bool RunTests();

	~Tester();
private:
	std::vector<ASM::AssemblyRunner> testInstances;

	std::string m_FolderPath;
	std::string m_BuildDir;
};