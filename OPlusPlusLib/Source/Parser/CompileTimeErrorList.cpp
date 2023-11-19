#include "CompileTimeErrorList.h"

#include "Nodes.h"
#include "../Utils.hpp"

namespace O
{

void CompileTimeErrorList::MakeError_Void(const std::string& message, Lexer::Token errorToken, CompileTimeError::Severity severity)
{
	CompileTimeError error;
	error.message = message;
	error.severity = severity;
	error.errorToken = errorToken;

	m_Errors.push_back(error);
}

void CompileTimeErrorList::PrintErrors(Lexer::Tokens tokens)
{
	std::string source = O::Lexer::Lexer::ReconstructSourcecode(tokens);
	auto sourceLines = split(source, '\n');

	for (auto& error : m_Errors)
	{
		auto startPosition = error.errorToken.m_StartPosition;

		std::string severity = std::string(magic_enum::enum_name(error.severity));
		std::cout << severity << ": " << error.message << "\n\n";

		std::string lineOfError = sourceLines[startPosition.line];

		int indent = std::to_string(startPosition.line).length();

		std::string leftPadding = " " + Replicate(indent, " ") + " | ";

		if (startPosition.line != 0)
		{
			std::string lineBefore = sourceLines[startPosition.line - 1];
			std::cout << leftPadding << lineBefore << "\n";
		}
		std::cout << " " << startPosition.line + 1 << " | " << lineOfError << "\n";
		std::cout << leftPadding;

		int errorMarkerLength = std::max(error.errorToken.ToFormattedValueString().length(), size_t(1));

		std::cout << Replicate(startPosition.column, " ");
		std::cout << Replicate(errorMarkerLength, "^");

		std::cout << "\n\n";

		//std::cout << error.positionInSource.line + 1 << ":" << error.positionInSource.column + 1;
	}
}

}