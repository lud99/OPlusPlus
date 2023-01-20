#include <fstream>
#include <string>
#include <ctype.h>
#include <iostream>
#include <vector>
#include <map>

#include "Lexer.h"
#include "Parser.h"

#include "Utils.hpp"

#include <iostream>

#include <string.h>
#include <vector>
#include <locale>
#include <sstream>
#include <filesystem>

#include "Interpreter/Functions.h"

#define TEST 1

#ifdef BYTECODE
#include "Interpreter/Bytecode/BytecodeInterpreter.h"
#endif
#ifdef AST
#include "Interpreter/AST/ASTInterpreter.h"
#endif
#ifdef ASM
#include "Compiler/AssemblyRunner.h"
#endif

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
	Functions::InitializeDefaultFunctions();

	srand(100);

	int inside = 0;
	int its = 10;

	for (int i = 0; i < its; i++) {
		float x = rand_range_float(-1.0, 1.0);
		float y = rand_range_float(-1.0, 1.0);

		printf("x,y: %f, %f\n", x, y);

		if (x * x + y * y <= 1.0) {
			inside++;
			
		};
	};

	

	float p = 4.0 * float(inside) / float(its);

	for (int i = 0; i < 20; i++)
	{
		printf("%f\n", rand_range_float(-1.0, 1.0));
	}

	std::string error;
	Value v;
#ifdef BYTECODE
	std::string filepath = "Programs/calc_pi.ö";

	//Functions::InitializeDefaultFunctions();

	v = BytecodeInterpreter::Get().CreateAndRunProgram(filepath, error);

	if (error != "")
		std::cout << error << "\n";
#endif // BYTECODE
#ifdef AST
	

	std::ifstream file(filepath);
	if (!file.good())
		std::cout << "Couldn't open file " << filepath << "\n\n";

	std::string fileContent = "";
	for (std::string line; std::getline(file, line);)
	{
		fileContent += line + "\n";
	}

	Lexer lexer;
	error = lexer.CreateTokens(fileContent);
	if (error != "")
		std::cout << error << "\n\n";

	for (int i = 0; i < lexer.m_Tokens.size(); i++)
		std::cout << lexer.m_Tokens[i].ToString() << ": " << lexer.m_Tokens[i].m_Value << " [" << lexer.m_Tokens[i].m_Depth << "]\n";
	std::cout << "\n";

	Parser parser;

	ASTNode tree;
	tree.parent = new ASTNode(ASTTypes::ProgramBody);
	tree.parent->left = &tree;

	parser.CreateAST(lexer.m_Tokens, &tree, tree.parent);

	if (parser.m_Error != "")
		std::cout << "AST Error: " << parser.m_Error << "\n";

	parser.PrintASTTree(tree.parent, 0);

	auto& interpreter = ASTint::ASTInterpreter::Get();

	v = interpreter.Execute(tree.parent);
	if (interpreter.m_Error != "") std::cout << "AST Interpreter error: " << interpreter.m_Error << "\n";
#endif // AST

	bool runTests = false;
	std::string filepath = "";

	for (int i = 1; i < argc; i++)
	{
		std::string arg = argv[i];
		
		if (arg == "-t")
		{
			runTests = true;
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
	}

	filepath = "Programs/calc_pi.ö";
	

	if (runTests)
	{
#ifdef ASM
		Tester tester;

		bool passedAllTests = tester.RunTests();

		if (passedAllTests)
			std::cout << "Passed all the tests in all files!! :)\n";
		else
			std::cout << "Failed one of the test files :(\n";
#endif
	}
	else
	{
		// Default. Run a specified program

#ifdef ASM
		AssemblyRunner runner(filepath);
		error = runner.Compile();

		if (error != "")
		{
			std::cout << "\n\n" << error << "\n\n";
		}
		else
		{
			std::cout << "Code:\n" << runner.GetCompiledCode() << "\n";

			std::cout << "Program output: " << runner.Execute() << "\n";
		}

#endif
	}

	//BytecodeInterpreter::Get().CreateAndRunProgram("Programs/function.ö", error);

//	if (error != "") std::cout << "error: " << error;


	while (true)
	{

	}
}