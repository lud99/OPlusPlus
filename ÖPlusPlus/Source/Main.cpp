#include <fstream>
#include <string>
#include <ctype.h>
#include <iostream>
#include <vector>
#include <map>
#include <thread>
#include <chrono>
#include <filesystem>

#include "json.hpp"

#include "Lexer.h"
#include "Parser.h"

#include "Utils.hpp"

#include <iostream>

#include <string.h>
#include <vector>
#include <locale>
#include <sstream>
#include <filesystem>
#include <chrono>

#include "Interpreter/Functions.h"

#include "Interpreter/Bytecode/BytecodeInterpreter.h"
#include "Interpreter/AST/ASTInterpreter.h"
#include "Compiler/AssemblyRunner.h"

#include "Tester.h"

double rand_range_float(double min, double max)  {
	return ((max - min) * (double(rand()) / 32767.0)) + min;
};

static std::vector<std::string> split(const std::string& txt, char ch, bool includeLast = true)
{
	std::vector<std::string> strs;
	size_t pos = txt.find(ch);
	size_t initialPos = 0;
	strs.clear();

	// Decompose statement
	while (pos != std::string::npos) {
		strs.push_back(txt.substr(initialPos, pos - initialPos));
		initialPos = pos + 1;

		pos = txt.find(ch, initialPos);
	}

	// Add the last one
	if (includeLast)
		strs.push_back(txt.substr(initialPos, std::min(pos, txt.size()) - initialPos + 1));

	return strs;
}

std::string stringRaw(std::string s)
{
	for (int i = 0; i < s.length(); i++) {
		if (s[i] == '\n') {
			s.replace(i, 1, "\\");
			s.insert(++i, "n");
		}
		if (s[i] == '\r') {
			s.replace(i, 1, "\\");
			s.insert(++i, "r");
		}
	}

	return s;
}

int main(int argc, const char* argv[])
{
	setlocale(LC_ALL, "");

	std::string error;

	bool runTests = false;
	bool onlyTokens = false;
	bool quiet = false;
	std::string filepath = "";// "Programs/hello_world.ö";
	std::string fileContent = "";
	std::string asmBuildDir = std::filesystem::current_path().generic_string() + "\\ASM Build files";
	ExecutionMethods method = ExecutionMethods::Bytecode;

	// Iterate over arguments
	for (int i = 1; i < argc; i++)
	{
		std::string arg = argv[i];

		if (arg == "-t")
		{
			runTests = true;
		}

		if (arg == "-q")
		{
			quiet = true;
		}

		if (arg == "-asm")
		{
			method = ExecutionMethods::Assembly;
		}
		if (arg == "-ast")
		{
			method = ExecutionMethods::AST;
		}
		if (arg == "-bytecode")
		{
			method = ExecutionMethods::Bytecode;
		}

		if (arg == "-buildDir")
		{
			asmBuildDir = argv[i + 1];
		}

		if (arg == "-f")
		{
			// Expect filename as next arg
			if (i >= argc - 1)
			{
				std::cout << "Expected argument with path to file after -f argument\n";
				abort();
			}

			filepath = argv[i + 1];
		}

		if (arg == "-fc")
		{
			// Expect text content as next arg up to the end
			if (i >= argc - 1)
			{
				std::cout << "Expected argument with file contents after -fc argument\n";
				abort();
			}

			for (int j = i + 1; j < argc; j++)
			{
				fileContent += argv[j];
				fileContent += "\n";
			}
		}

		if (arg == "-tokens")
		{
			onlyTokens = true;
		}
	}

	ExecutionMethods_Global::m_Method = method;

	Functions::InitializeDefaultFunctions(method);

	// Print the tokens for the source code in -fc argument in JSON format for use in VSCode extension
	if (onlyTokens)
	{
		Lexer lexerForTokens;
		error = lexerForTokens.CreateTokens(fileContent, true);
		if (error != "") {
			std::cout << error;
			return 1;
		}

		Lexer lexerForParsing;
		lexerForParsing.CreateTokens(fileContent, false);

		Parser parser;

		ASTNode tree;
		tree.parent = new ASTNode(ASTTypes::ProgramBody);
		tree.parent->left = &tree;

		parser.CreateAST(lexerForParsing.m_Tokens, &tree, tree.parent);

		if (parser.m_Error != "")
		{
			std::cout << parser.m_Error;
			return 1;
		}

		using json = nlohmann::json;
		json tokens = json::array();

		for (int i = 0; i < lexerForTokens.m_Tokens.size(); i++)
		{
			Token& token = lexerForTokens.m_Tokens[i];
			tokens.push_back({ {"type", token.ToString() }, { "value", token.m_Value }, { "index", token.m_StartPosition} });
		}

		std::cout << tokens;
		return 0;
	}

	if (runTests)
	{
		Tester tester(asmBuildDir);

		bool passedAllTests = tester.RunTests();

		if (passedAllTests)
			std::cout << "Passed all the tests in all files!! :)\n";
		else
			std::cout << "Failed one of the test files :(\n";

		while (true) {};
	}

	// Read file if filepath is specified
	if (filepath != "")
	{
		std::ifstream file(filepath);
		if (!file.good())
			std::cout << "Couldn't open file " << filepath << "\n\n";


		for (std::string line; std::getline(file, line);)
		{
			fileContent += line + "\n";
		}
	}
	else
	{
		if (fileContent == "")
		{
			std::cout << "Expected a file in -fc or -f argument, got none\n";
			return 1;
		}
	}

	if (method == ExecutionMethods::Bytecode)
	{
		Value v = Bytecode::BytecodeInterpreter::Get().CreateAndRunProgram(fileContent, error, !quiet);

		if (error != "")
		{
			std::cout << error << "\n";
			return 1;
		}
			
	}
	else if (method == ExecutionMethods::AST)
	{
		Lexer lexer;
		error = lexer.CreateTokens(fileContent);
		if (error != "")
			std::cout << error << "\n\n";

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
		{
			std::cout << "AST Error: " << parser.m_Error << "\n";
			return 1;
		}

		if (!quiet) parser.PrintASTTree(tree.parent, 0);

		auto& interpreter = AST::ASTInterpreter::Get();

		auto start = std::chrono::high_resolution_clock::now();

		if (!quiet) std::cout << "Console output:\n";

		interpreter.Execute(tree.parent);
		if (interpreter.m_Error != "")
		{
			std::cout << "AST Interpreter error: " << interpreter.m_Error << "\n";
			return 1;
		}

		auto stop = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

		if (!quiet) std::cout << "\nExecution took: " << (duration.count()) << "ms" << "\n";
	}
	else if (method == ExecutionMethods::Assembly)
	{
		ASM::AssemblyRunner runner(fileContent, asmBuildDir);
		error = runner.Compile(quiet);

		if (error != "")
		{
			std::cout << error << "\n\n";
			return 1;
		}
		else
		{
			if (!quiet) 
				std::cout << "Code:\n" << runner.GetCompiledCode() << "\n";

			std::string output = runner.Execute(quiet);

			if (!quiet)
				std::cout << "Program output: " << output << "\n";
			else 
			{
				//std::cout << output;
				return 0;
			}
		}
	}

	/*while (true)
	{

	}*/
}

ExecutionMethods ExecutionMethods_Global::m_Method;