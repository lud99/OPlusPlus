#pragma once

#include <string>
#include "Lexer.h"

// Lexer error
// AST Parser error
// Semantic error
// Compilation error

namespace O
{
	struct CompileTimeError
	{
		enum Severity {
			Info,
			Warning,
			Error
		};

		Severity severity;
		std::string message;

		// Should be a union but C++ wont let me :(
		O::Lexer::Token errorToken;
		O::Lexer::TokenRange errorRange;

		bool isSingle = true;
	};

	struct Node;
	class CompileTimeErrorList
	{
	public:
		void MakeError_Void(const std::string& message, O::Lexer::Token errorToken, CompileTimeError::Severity severity = CompileTimeError::Error);
		void MakeError_Void(const std::string& message, O::Lexer::TokenRange errorRange, CompileTimeError::Severity severity = CompileTimeError::Error);


		EXPORT bool HasError() { return !m_Errors.empty(); }
		EXPORT void SetErrors(std::vector<CompileTimeError>& errors) { m_Errors = errors; }
		EXPORT auto& GetErrors() { return m_Errors; };

		EXPORT void PrintErrors(O::Lexer::Tokens tokens);

	private:
		std::vector<CompileTimeError> m_Errors;
	};

}