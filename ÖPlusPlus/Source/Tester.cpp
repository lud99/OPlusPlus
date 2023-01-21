#include "Tester.h"

#include <filesystem>
#include <fstream>
#include <iostream>

static std::vector<std::string> SplitString(const std::string& txt, char ch, bool includeLast = true)
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

Tester::Tester()
{
	m_FolderPath = "Programs\\Tests";
}

bool Tester::RunTests()
{
	bool passedAllTests = true;

	namespace fs = std::filesystem;

	std::vector<std::string> testPaths;

	for (const auto& entry : fs::directory_iterator(m_FolderPath))
	{
		auto parts = SplitString(entry.path().string(), '.');
		if (parts[parts.size() - 1] == "ö") // Source code file
		{
			testPaths.push_back(entry.path().string());

			// Compile the tests now
			
			// TODO: generalize 
			AssemblyRunner test(entry.path().string()); 
			std::string compileError = test.Compile();

			if (compileError != "")
			{
				std::cout << "\n\n" << compileError << "\n\n";
				abort();
			}
			else
			{
				testInstances.push_back(test);
			}
		}
	}

	for (int i = 0; i < testPaths.size(); i++)
	{
		bool passedTest = true;

		const std::string& testPath = testPaths[i];

		AssemblyRunner& test = testInstances[i];

		std::ifstream resultFile(testPath + ".result");
		if (!resultFile.good())
		{
			std::cout << "Couldn't open test result file " << (testPath + ".result") << " \n\n";
			abort();
		}

		std::string expectedResult = "";
		for (std::string line; std::getline(resultFile, line);)
		{
			if (line != "")
				expectedResult += line + "\n";
		}

		// TODO: generalize
		std::string result = test.Execute();

		auto resultLines = SplitString(result, '\n', false);
		auto expectedResultLines = SplitString(expectedResult, '\n', false);

		std::cout << "Testing file " << testPath << ":\n\n";

		if (expectedResultLines.size() != resultLines.size())
		{
			std::cout << "Code file and test file to not have the same amount of test cases\n";
			std::cout << "Program output:\n" << result;
			std::cout << "\nExpected:\n";
			for (int j = 0; j < expectedResultLines.size(); j++)
			{
				std::cout << expectedResultLines[j] << "\n";
			}
			passedAllTests = false;
			break;
		}

		for (int j = 0; j < resultLines.size(); j++)
		{
			std::cout << "Test " << (i + 1) << ": ";
			if (resultLines[j] == expectedResultLines[j])
			{
				std::cout << "Passed (" << resultLines[j] << " == " << expectedResultLines[j] << ")\n";
			}
			else
			{
				std::cout << "Failed (" << resultLines[j] << " == " << expectedResultLines[j] << ")\n";
				passedAllTests = false;
				passedTest = false;
			}
		}

		if (passedTest)
			std::cout << "Passed all tests in the file :)\n";
		else
			std::cout << "Failed a test in the file :(\n";
	}

	return passedAllTests;
}

Tester::~Tester()
{
}
