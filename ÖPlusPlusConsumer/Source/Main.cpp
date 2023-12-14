#include <ÍPlusPlusLib.h>

#include <fstream>
#include <iostream>
#include <sstream>

int main(const char* args) 
{
	using namespace O;

	setlocale(LC_ALL, "");

	std::ifstream file("Programs/function_overloads.÷");
	if (!file.good())
	{
		std::cout << "Could not open file :(\n";
		return 0;
	}


	std::stringstream buffer;
	buffer << file.rdbuf();

	const std::string& source = buffer.str();

	O::Lexer::Lexer lexer;
	std::string error = lexer.CreateTokens(source);
	if (error != "")
	{
		std::cout << "Lexer error: " << error << "\n";
		return 0;
	}

	std::cout << O::Lexer::Lexer::ReconstructSourcecode(lexer.GetTokens()) << "\n\n";

	O::AST::Parser parser(lexer.GetTokens());
	AST::Node* tree = parser.ParseProgram();

	if (parser.HasError())
	{
		parser.PrintErrors(lexer.GetTokens());
		return 0;
	}

	SemanticAnalyzer anal(tree);
	anal.AnalyzeProgram();

	if (anal.HasError())
	{
		anal.PrintErrors(lexer.GetTokens());
		return 0;
	}

	if (tree)
		tree->Print();
}