#include "AssemblyRunner.h"

#include "../Lexer.h"
#include "../Parser.h"

#include <iostream>
#include <fstream>

#include <chrono>
#include <cstdio>
#include <iostream>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

#include <Windows.h>

namespace ASM {
	AssemblyRunner::AssemblyRunner(const std::string& fileContent, const std::string& buildDir)
	{
		m_FileContent = fileContent;
		m_BuildDir = buildDir;
	}

	std::string AssemblyRunner::Compile(bool quiet)
	{
		Lexer lexer;
		std::string error = lexer.CreateTokens(m_FileContent);
		if (error != "")
			return "Lexer Error: " + error;

		if (!quiet)
		{
			for (int i = 0; i < lexer.m_Tokens.size(); i++)
				std::cout << lexer.m_Tokens[i].ToString() << ": " << lexer.m_Tokens[i].m_Value << " [" << lexer.m_Tokens[i].m_Depth << "]\n";
			std::cout << "\n";
		}

		Parser parser;

		ASTNode tree;
		tree.parent = new ASTNode(ASTTypes::ProgramBody);
		tree.parent->left = &tree;

		parser.CreateAST(lexer.m_Tokens, &tree, tree.parent);

		if (parser.m_Error != "")
			return "AST Error: " + parser.m_Error;

		if (!quiet)
			parser.PrintASTTree(tree.parent, 0);

		m_Compiler.Compile(tree.parent);
		m_Compiler.Optimize();

		if (m_Compiler.m_Error != "")
			return "ASM Compiler Error: " + m_Compiler.m_Error;

		m_Code += "%include \"" + m_BuildDir + "\\io.inc\"\n";
		m_Code += "%include \"" + m_BuildDir + "\\stdlib.inc\"\n";
		m_Code += "%include \"" + m_BuildDir + "\\functions.inc\"\n\n";

		m_Code += "section .data\n";
		for (int i = 0; i < m_Compiler.m_DataSection.GetLines().size(); i++)
		{
			ASM::Instruction inst = m_Compiler.m_DataSection.GetLines()[i];

			if (!inst.m_IsLabel)
				m_Code += "	" + inst.ToString() + "\n";
			else
				m_Code += inst.ToString() + "\n";
		}

		m_Code += "\nsection .text\n";
		for (int i = 0; i < m_Compiler.m_TextSection.GetLines().size(); i++)
		{
			ASM::Instruction inst = m_Compiler.m_TextSection.GetLines()[i];

			if (!inst.m_IsLabel)
				m_Code += "	" + inst.ToString() + "\n";
			else
				m_Code += inst.ToString() + "\n";
		}

		return m_Compiler.m_Error;
	}

	std::string AssemblyRunner::Execute(bool quiet)
	{
		std::ofstream file;
		file.open(m_BuildDir + "/generated/program.asm");
		if (!file.good())
		{
			std::cout << "ASM Build Error: Cannot write to program.asm\n";
			return "";
		}

		file << m_Code;
		file.close();

		const std::string batFile = "\"" + std::filesystem::current_path().generic_string() + "/" + m_BuildDir + "/build.bat" + "\"";

		const std::string nasm = "nasm -f win32 \"" + m_BuildDir + "\\generated\\program.asm\" -o \"" + m_BuildDir + "\\generated\\program.o\"";
		const std::string gcc = "gcc -o \"" + m_BuildDir + "\\program\" \"" + m_BuildDir + "\\generated\\program.o\" -m32"; 

		system(nasm.c_str());
		system(gcc.c_str());

		//system(batFile.c_str());

		auto start = std::chrono::high_resolution_clock::now();
		
		if (!quiet) 
			std::cout << "Console output:\n";

		const std::string cmd = "\""  + m_BuildDir + "\\program.exe\"";

		// Execute the built exe file and grab the std output
		std::array<char, 128> buffer;
		std::string result;
		std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd.c_str(), "r"), _pclose);
		if (!pipe) {
			throw std::runtime_error("_popen() failed!");
		}
		while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
			std::string s = buffer.data();

			std::cout << s;
			result += s;
		}

		auto stop = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

		if (!quiet)  
			std::cout << "\nExecution took: " << (duration.count()) << "ms" << "\n";

		return result;
	}
}