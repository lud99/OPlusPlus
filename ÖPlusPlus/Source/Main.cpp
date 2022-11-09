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

#include "Interpreter/Bytecode/BytecodeInterpreter.h"
#include "Interpreter/Bytecode/BytecodeFunctions.h"
#include "Interpreter/AST/ASTInterpreter.h"

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

	std::string filepath = "Programs/ASTint.ö";

	std::ifstream file(filepath);
	if (!file.good())
		std::cout << "Couldn't open file " << filepath << "\n\n";

	std::string fileContent = "";
	for (std::string line; std::getline(file, line);)
	{
		fileContent += line + "\n";
	}

	Lexer lexer;
	std::string error = lexer.CreateTokens(fileContent);
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

	//BytecodeFunctions::InitializeDefaultFunctions();

	//std::string error;

	ASTint::ASTInterpreter interpreter(tree.parent);
	ASTint::Value v = interpreter.InterpretTree(tree.parent);
	//BytecodeInterpreter::Get().CreateAndRunProgram("Programs/function.ö", error);

	if (error != "") std::cout << "error: " << error;


	while (true)
	{

	}
}