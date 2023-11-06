#include <ÖPlusPlusLib.h>

#include <iostream>
#include <fstream>
#include <sstream>

using namespace Ö;


AST::Node* CreateProgram(std::string fileContent, bool verbose = true)
{
	std::string error;

	Lexer lexer;
	error = lexer.CreateTokens(fileContent);
	if (error != "")
	{
		std::cout << error << "\n\n";
		return nullptr;
	}

	if (verbose)
	{
		for (auto& token : lexer.GetTokens())
			std::cout << token.ToString() << ": " << token.m_Value << " [" << token.m_Depth << "]\n";
		std::cout << "\n";
	}

	AST::Parser parser;

	AST::Node root = parser.CreateRootNode();
	AST::Node* tree = parser.CreateAST(lexer.GetTokens(), &root);

	if (parser.HasError())
	{
		std::cout << "AST Error: " << parser.GetError() << "\n";
		return nullptr;
	}

	if (verbose)
		tree->Print();

	// Do basic tests

	return tree;
}

bool approximatelyEqual(float a, float b, float epsilon)
{
	return fabs(a - b) <= ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}

bool RunTests()
{
	std::vector<std::string> sources = {
		"1 + 2 / 3 - 8;",
		"1 - 2 - 3",
		"1 + 2 - 3 - 4",
		"1 * 2 * 3 + 5 - 1 - 2 - 3 - 5 / 5",
		"1 * -3 + 5 - 5 * 4 / 8 - 1 + 2 - 3 + 4",
		"(1 + 2) * 3",

		// difficult
		"2 + (17 * 2 - 30) * 5 + 2 - (8 / 2) * 4",
		"(1 + 2) - (3 + 4)",
		"((1 + 2) - (3 + 4))",
		"1 - (3 - (1 + 2) + 3)",
		"2 * 3",
		"1 - (3 / (1 + 2) + 3)",
		"10 / 2",
		"2 + (17 * 2 - 30)",
		"2 + (17 * 2 - 30) * 5 + 2 - (8 / 2) * 4",
		"17*2 - 30",

		// unary ops
		"- 1 + 2",
		"- 1 + 5 - - 2 * 65",
		"1 - - 2 + 3 - 5 * 9 - 8",
		"+1 + +2 + +3",
		"+1 - 2 - - 3 + +8",
		"+1 - 2 - - 3 + +-8",
		"+1 - 2 - - 3 + + - 8",
		"+1 - 2 - - 3 + - + 8",
		"+ - + 8"
	};

	//      +
	//  -1   +
	//      -2   -3       

	// interpreted as
	// (1 * (2 * 3)) +  (5 - (1 - (2 - (3 - 5 / 5))))

	std::vector<float> results = {
		1 + 2.0f / 3.0f - 8,
		1 - 2 - 3,
		1 + 2 - 3 - 4,
		1 * 2 * 3 + 5 - 1 - 2 - 3 - 5.0f / 5.0f,
		1 * -3 + 5 - 5 * 4.0f / 8.0f - 1 + 2 - 3 + 4,
		(1 + 2) * 3,

		// hard
		2 + (17 * 2 - 30) * 5 + 2 - (8 / 2) * 4,
		(1 + 2) - (3 + 4),
		((1 + 2) - (3 + 4)),
		1 - (3 - (1 + 2) + 3),
		2 * 3,
		1 - (3.0f / (1 + 2) + 3),
		10 / 2,
		2 + (17 * 2 - 30),
		2 + (17 * 2 - 30) * 5 + 2 - (8 / 2) * 4,
		17 * 2 - 30,
		
		// Unary
		-1 + 2,
		-1 + 5 - -2 * 65,
		1 - -2 + 3 - 5 * 9 - 8,
		+1 + +2 + +3,
		+1 - 2 - - 3 + +8,
		+1 - 2 - -3 + +-8,
		+1 - 2 - -3 + + - 8,
		+1 - 2 - -3 + - + 8,
		+-+8


	};
	assert(sources.size() == results.size());

	auto FailedTest = [&](int i, float result, AST::Node* node)
		{
			std::cout << "Failed test " << i << "! Expected result " << results[i] << " but got " << result << "\n";
			if (node) node->Print();

			return false;
		};

	for (int i = 0; i < sources.size(); i++)
	{
		auto node = CreateProgram(sources[i], true);
		AST::Parser p;

		if (!node) return FailedTest(i, 0, node);

		float result = p.TemporaryEvaluator(node);
		
		if (approximatelyEqual(result, results[i], 0.1f))
		{
			std::cout << "Passed test " << i << " successfully\n";
		}
		else
		{
			return FailedTest(i, result, node);
		}
	}

	return true;
}

int main()
{
	std::ifstream file("./semantic_anal.ö");
	if (!file.is_open())
	{
		std::cout << "Failed to open file\n";
		while (true) {};
	}
	std::stringstream buffer;
	buffer << file.rdbuf();

	// "- 1 * + 5 - 2" error!
	// "- 1 + 5 - 2 * 65" add to test

	//AST::Node program = CreateProgram(buffer.str());

	RunTests();

	while (true) {};

	return 0;
}