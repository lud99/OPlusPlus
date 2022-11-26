#include <fstream>
#include <string>
#include <ctype.h>
#include <iostream>
#include <vector>
#include <map>

#include "Lexer.h"
#include "Parser.h"

#include <iostream>

#include <string.h>
#include <vector>
#include <locale>
#include <sstream>

#include "Interpreter/Functions.h"

#ifdef BYTECODE_INTERPRETER
#include "Interpreter/Bytecode/BytecodeInterpreter.h"
#endif
#ifdef AST_INTERPRETER
#include "Interpreter/AST/ASTInterpreter.h"
#endif
#ifdef ASM_COMPILER
#include "Compiler/AssemblyCompiler.h"
#endif

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

int main()
{
	setlocale(LC_ALL, "");
	Functions::InitializeDefaultFunctions();

	std::string filepath = "Programs/if.ö";

	std::string error;
	Value v;
#ifdef BYTECODE_INTERPRETER
	

	v = BytecodeInterpreter::Get().CreateAndRunProgram(filepath, error);
#endif // BYTECODE_INTERPRETER
#ifdef AST_INTERPRETER
	

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
#endif // AST_INTERPRETER

#ifdef ASM_COMPILER
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

	AssemblyCompiler compiler;
	compiler.Compile(tree.parent);
	compiler.Optimize();

	std::cout << "section .data\n";
	for (int i = 0; i < compiler.m_DataSection.GetLines().size(); i++)
	{
		std::string s = compiler.m_DataSection.GetLines()[i].ToString();
		//if (s != "")
		std::cout << s << "\n";
	}

	std::cout << "\n\nsection .text\n";
	for (int i = 0; i < compiler.m_TextSection.GetLines().size(); i++)
	{
		std::string s = compiler.m_TextSection.GetLines()[i].ToString();
		//if (s != "")
		std::cout << s << "\n";
	}

	if (compiler.m_Error != "") std::cout << "\nASM Compiler error: " << compiler.m_Error << "\n";
#endif

	//BytecodeInterpreter::Get().CreateAndRunProgram("Programs/function.ö", error);

	if (error != "") std::cout << "error: " << error;


	while (true)
	{

	}
}