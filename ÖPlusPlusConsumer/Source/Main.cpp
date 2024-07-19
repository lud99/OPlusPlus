#include <ÖPlusPlusLib.h>

#include <fstream>
#include <iostream>
#include <sstream>

int main(const char* args) 
{
	using namespace O;

	setlocale(LC_ALL, "");

	std::ifstream file("Programs/class_constructor.ö");
	if (!file.good())
	{
		std::cout << "Could not open file :(\n";
		return 0;
	}


	std::stringstream buffer;
	buffer << file.rdbuf();

	const std::string& source = buffer.str();

	O::Lexer::Lexer lexer, lexerWithComments;
	std::string error = lexer.CreateTokens(source);
	if (error != "")
	{
		std::cout << "Lexer error: " << error << "\n";
		return 0;
	}

	// Hacky way to generate tokens with comments
	// TODO: Should not require 2 steps
	error = lexerWithComments.CreateTokens(source, true);
	if (error != "")
	{
		std::cout << "Lexer error (with comments): " << error << "\n";
		return 0;
	}

	std::cout << O::Lexer::Lexer::ReconstructSourcecode(lexerWithComments.GetTokens(), true) << "\n\n";

	O::AST::Parser parser(lexer.GetTokens(), lexerWithComments.GetTokens());
	AST::Node* tree = parser.ParseProgram();

	if (tree)
		tree->Print("", nullptr, nullptr);


	if (parser.HasError())
	{
		parser.PrintErrors(lexerWithComments.GetTokens());
		return 0;
	}
	assert(tree);

	SemanticAnalyzer anal(tree, parser);
	anal.AnalyzeProgram();

	if (anal.HasError())
	{
		std::cout << "Generated AST\n\n";
		tree->Print("", nullptr, nullptr);

		std::cout << "\n\n";

		anal.PrintErrors(lexerWithComments.GetTokens());
	}
	//else
	{
		if (tree)
		{
			std::cout << "Generated AST after semantic analysis with type information\n\n";
			tree->Print("", anal.GetGlobalTypeTable(), &anal);
		}
	}

	std::cout << "\n\n";

}