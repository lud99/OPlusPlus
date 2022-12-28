#include "AssemblyRunner.h"

#include "../Lexer.h"
#include "../Parser.h"

#include <iostream>
#include <fstream>

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>


#include <Windows.h>

AssemblyRunner::AssemblyRunner(const std::string& filepath)
{
	m_Filepath = filepath;
}

std::string AssemblyRunner::Compile()
{
	std::ifstream file(m_Filepath);
	if (!file.good())
		return "File Error: Couldn't open file " + m_Filepath;

	std::string fileContent = "";
	for (std::string line; std::getline(file, line);)
	{
		fileContent += line + "\n";
	}

	Lexer lexer;
	std::string error = lexer.CreateTokens(fileContent);
	if (error != "")
		return "Lexer Error: " + error;

	for (int i = 0; i < lexer.m_Tokens.size(); i++)
		std::cout << lexer.m_Tokens[i].ToString() << ": " << lexer.m_Tokens[i].m_Value << " [" << lexer.m_Tokens[i].m_Depth << "]\n";
	std::cout << "\n";

	Parser parser;

	ASTNode tree;
	tree.parent = new ASTNode(ASTTypes::ProgramBody);
	tree.parent->left = &tree;

	parser.CreateAST(lexer.m_Tokens, &tree, tree.parent);

	if (parser.m_Error != "")
		return "AST Error: " + parser.m_Error;

	parser.PrintASTTree(tree.parent, 0);

	m_Compiler.Compile(tree.parent);
	m_Compiler.Optimize();

	if (m_Compiler.m_Error != "") 
		return "ASM Compiler Error: " + m_Compiler.m_Error;

	m_Code += "%include \"io.inc\"\n";
	m_Code += "%include \"stdlib.inc\"\n";
	m_Code += "%include \"functions.inc\"\n\n";

	m_Code += "section .data\n";
	for (int i = 0; i < m_Compiler.m_DataSection.GetLines().size(); i++)
	{
		Instruction inst = m_Compiler.m_DataSection.GetLines()[i];

		if (!inst.m_IsLabel)
			m_Code += "	" + inst.ToString() + "\n";
		else
			m_Code += inst.ToString() + "\n";
	}

	m_Code += "\nsection .text\n";
	for (int i = 0; i < m_Compiler.m_TextSection.GetLines().size(); i++)
	{
		Instruction inst = m_Compiler.m_TextSection.GetLines()[i];

		if (!inst.m_IsLabel)
			m_Code += "	" + inst.ToString() +"\n";
		else
			m_Code += inst.ToString() + "\n";
	}

	return m_Compiler.m_Error;
}

std::string AssemblyRunner::Execute()
{
	std::ofstream file;
	file.open("Build/temp/program.asm");
	file << m_Code;
	file.close();

	system(".\\Build\\build.bat");

	const char* cmd = ".\\Build\\program.exe";
	
	// Execute the built exe file and grab the std output
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
	if (!pipe) {
		throw std::runtime_error("_popen() failed!");
	}
	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
		result += buffer.data();
	}

	return result;
}
