#include "../Functions.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <random>
#include <chrono>
#include <thread>
#include <conio.h>

#include "BytecodeInterpreter.h"

//#define EnsureTypeOfArg(arguments, index, expectedType) \
//{ \
//	if (!ElementExists(*arguments, index)) return BytecodeInterpreter::Get().ThrowExceptionValue("Argument " + std::to_string(index)  + " expected, but not found"); \
//	Value& val = arguments->at(index); \
//	bool isCorrectType = false; \
//	if (expectedType == Value::String) \
//		isCorrectType = val.IsString(); \
//	else \
//		isCorrectType = val.m_Type == expectedType; \
//    if (!isCorrectType) \
//        return BytecodeInterpreter::Get().ThrowExceptionValue("Argument " + std::to_string(index) + " was " + ValueTypeToString(val.m_Type) + ", expected a " + ValueTypeToString(expectedType)); \
//}

//
//Value BytecodeFunctions::__print_stack(ValueArray* args)
//{
//	std::cout << "\nOperand stack	   Variables stack\n";
//	std::cout << "Top: " << Runner::Get().GetContext(0)->GetTopFrame().m_OperandStackTop << "\n";
//	for (int i = 0; i < 16; i++)
//	{
//		Value& stack = Runner::Get().GetContext(0)->GetTopFrame().m_OperandStack[i];
//		Value& variable = Runner::Get().GetContext(0)->GetTopFrame().m_VariablesList[i];
//
//		std::cout << "(" << ValueTypeToString(stack.m_Type) << ") ";
//		if (stack.IsObject())
//			std::cout << stack.m_HeapEntryPointer << ", ";
//
//		std::string s = stack.ToFormattedString();
//		for (int i = 0; i < s.length(); i++) {
//			if (s[i] == '\n') {
//				s.replace(i, 1, "\\");
//				s.insert(++i, "n");
//			}
//			if (s[i] == '\r') {
//				s.replace(i, 1, "\\");
//				s.insert(++i, "r");
//			}
//		}
//		std::cout << s << "               ";
//
//		std::cout << "(" << ValueTypeToString(variable.m_Type) << ") ";
//
//		if (variable.IsObject())
//			std::cout << variable.m_HeapEntryPointer << ", ";
//
//		s = variable.ToFormattedString();
//		for (int i = 0; i < s.length(); i++) {
//			if (s[i] == '\n') {
//				s.replace(i, 1, "\\");
//				s.insert(++i, "n");
//			}
//			if (s[i] == '\r') {
//				s.replace(i, 1, "\\");
//				s.insert(++i, "r");
//			}
//		}
//
//		std::cout << s << "\n";
//	}
//
//	return Value(Value::Null);
//}

Value Functions::printf(ARGS)
{
	Value formatted = format_string(args);
	std::cout << formatted.ToString() << "\n";

	// Cleanup
	BytecodeInterpreter::Get().m_Heap.DeleteObject(formatted.m_HeapEntryPointer);

	return Value(ValueTypes::Void);
}

Value Functions::format_string(ARGS)
{
	//EnsureTypeOfArg(args, 0, ValueTypes::String);

	std::string formatted = args.at(0).GetString();

	if (args.size() == 1)
		return args.at(0);

	// Iterate all args
	for (int i = 1; i < args.size(); i++)
	{
		std::size_t charPos = formatted.find('$');
		if (charPos == std::string::npos) // Nothing to format
			return BytecodeInterpreter::Get().m_Heap.CreateString(formatted);

		// Remove the '$'
		formatted.erase(charPos, 1);

		// Insert the thing at that position
		formatted = formatted.insert(charPos, args.at(i).ToString());
	}

	return BytecodeInterpreter::Get().m_Heap.CreateString(formatted);
}