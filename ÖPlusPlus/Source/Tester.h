#pragma once

#include <vector>

#include "Compiler/AssemblyRunner.h"

#ifdef ASM

class Tester
{
public:
	Tester();

	bool RunTests();

	~Tester();
private:
	std::vector<AssemblyRunner> testInstances;

	std::string m_FolderPath;
};

#endif