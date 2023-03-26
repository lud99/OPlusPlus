#pragma once

#include <string>
#include <sstream>
#include <vector>

enum class ExecutionMethods {
	Assembly,
	AST,
	Bytecode
};

namespace ExecutionMethods_Global 
{
	extern ExecutionMethods m_Method;
};

static double StringToDouble(std::string s)
{
	std::istringstream streamToConvert(s);
	streamToConvert.imbue(std::locale("C"));
	double result = 0.0;
	streamToConvert >> result;

	if (streamToConvert.fail() || streamToConvert.bad() || !streamToConvert.eof()) {
		abort();
	}

	return result;
}

template <typename T>
static bool ElementExists(std::vector<T>& vector, int index)
{
	return (index >= 0 && index < vector.size());
}

template <typename T>
static std::vector<T> ConcatVectors(std::vector<T>& v1, std::vector<T>& v2)
{
std::vector<T> concated(v1);
concated.insert(concated.end(), v2.begin(), v2.end());

return concated;
}

template <typename T>
static std::vector<T> SliceVector(std::vector<T>& vector, int start = -1, int end = -1)
{
	std::vector<T> sliced;
	// Slice a portion of the vector tokens (if wanted)
	if (start == -1) start = 0;
	if (end == -1) end = vector.size();
	for (int i = start; i < end; i++)
		sliced.push_back(vector[i]);

	return sliced;
}

template <typename T>
static std::vector<T> SliceVectorC(std::vector<T> vector, int start = -1, int end = -1)
{
	std::vector<T> sliced;
	// Slice a portion of the vector tokens (if wanted)
	if (start == -1) start = 0;
	if (end == -1) end = vector.size();
	for (int i = start; i < end; i++)
		sliced.push_back(vector[i]);

	return sliced;
}

static bool BeginsWith(std::string string, std::string find) { return string.rfind(find, 0) == 0; }

static std::string RemoveTabs(std::string line)
{
	std::string newStr;
	for (unsigned int i = 0; i < line.length(); i++)
	{
		int code = line[i];
		if (code != 9 /* tab */ && code != 32 /* space */)
		{
			// Copy the rest of the string
			for (unsigned int c = i; c < line.length(); c++)
				newStr += line[c];

			break;
		}
	}

	return newStr;
}

static bool IsValidNumberChar(char ch)
{
	if (ch == ' ') return false;
	if (isdigit(ch) || ch == '.' || ch == '-')
		return true;

	return false;
}

static bool IsValidVariableChar(char ch)
{
	if (ch == ' ') return false;
	if (isalpha(ch) || ch == '_')
		return true;

	return false;
}

static bool IsValidNumberPart(std::string string, int index, std::string& error)
{
	// 5....1;
	char current = string[index];

	if (isdigit(current))
		return true;

	// At end or start
	if (index == 0 || index == string.length() - 1)
	{
		if (index == 0)
		{
			if (current == '-')
			{
				if (isdigit(string[index + 1]))
					return true;

				return false;
			}

			if (current == '.')
			{
				if (isdigit(string[index + 1]))
					return true;

				error = "Expected number after decimal point";
				return false;
			}
		}
		else
		{
			if (current == '.')
			{
				error = "Expected number after decimal point";
				return false;
			}

			if (current == '-')
			{
				error = "Expected number after minus sign";
				return false;
			}

			return isdigit(current);
		}
	}
	else
	{
		// Walk backwards and check if this variable part is actually valid
		if (isdigit(current))
			return true;

		if (current == '.')
		{
			if (string[index + 1] == '.' || !IsValidNumberPart(string, index + 1, error))
			{
				error = "Expected a number after the decimal point";
				return false;
			}

			if (string[index - 1] == ' ' || IsValidNumberPart(string, index - 1, error))
			{
				// Check for more decimal points
				for (int i = index - 1; i >= 0; i--)
				{
					// Don't check stuff that isn't part of the number
					if (!IsValidNumberChar(string[i]))
						break;

					if (string[i] == '.')
					{
						error = "Cannot have multiple decimal points in a number";
						return false;
					}
				}

				return true;
			}
		}

		if (current == '-')
		{
			if (string[index + 1] == '-' || !IsValidNumberPart(string, index + 1, error))
			{
				return false;
			}

			if (string[index - 1] == ' ' || IsValidNumberPart(string, index - 1, error))
			{
				// Check for more minus signs
				for (int i = index - 1; i >= 0; i--)
				{
					// Don't check stuff that isn't part of the number
					if (!IsValidNumberChar(string[i]))
						break;

					if (string[i] == '-')
					{
						error = "Cannot have multiple minus signs in a number";
						return false;
					}
				}

				return true;
			}
		}
	}

	return false;
}

static bool IsValidVariablePart(std::string string, int index, std::string& error)
{
	char current = string[index];

	// At end or start
	if (index == 0 || index == string.length() - 1)
	{
		if (IsValidVariableChar(current))
			return true;

		// Check if prev char exists, and if so if its a variable
		if (index > 0)
		{
			if (isdigit(current) && IsValidVariablePart(string, index - 1, error))
				return true;
		}
	}
	else
	{
		if (current == ' ')
			return false;

		if (IsValidVariableChar(current))
			return true;

		if (!IsValidVariablePart(string, index - 1, error) && !IsValidVariableChar(string[index + 1]))
			return false;

		if (isdigit(current) && IsValidVariablePart(string, index - 1, error))
			return true;

		if (isdigit(current))
			error = "Variable name cannot start with a number";
	}

	return false;
}

static bool IsNumber(const std::string& s)
{
	double num = atof(s.c_str());

	char* p;
	strtod(s.c_str(), &p);
	return *p == 0;
}

static void* SetStringAtPointer(std::string string, void* data)
{
	const char* str = string.c_str();

	data = new char[string.length() + 1 /* null terminator */];

	// fill
	for (unsigned int i = 0; i < string.length(); i++)
	{
		((char*)data)[i] = string[i];
	}
	((char*)data)[string.length()] = '\0'; // add null byte

	return data;
}

static std::string CastToString(void* data)
{
	const char* str = (const char*)data;
	return std::string(str);
}

static char* CopyString(const char* str)
{
	char* newString = new char[strlen(str) + 1];
	strncpy_s(newString, strlen(str) + 1, str, strlen(str) + 1);

	return newString;
}