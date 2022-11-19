#pragma once

#include "../Parser.h"

class Section
{
public:
	void AddLine(std::string line, const std::string& comment = "");

	std::vector<std::string>& GetLines() { return m_Lines; }

private:
	std::vector<std::string> m_Lines;
};

class AssemblyCompiler
{
public:
	AssemblyCompiler() {};

	void Compile(ASTNode* node);

public:
	Section m_DataSection;
	Section m_TextSection;
};