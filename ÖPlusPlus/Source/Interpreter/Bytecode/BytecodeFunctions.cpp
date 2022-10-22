#include "BytecodeFunctions.h"

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
//	StackValue& val = arguments->at(index); \
//	bool isCorrectType = false; \
//	if (expectedType == Value::String) \
//		isCorrectType = val.IsString(); \
//	else \
//		isCorrectType = val.m_Type == expectedType; \
//    if (!isCorrectType) \
//        return BytecodeInterpreter::Get().ThrowExceptionValue("Argument " + std::to_string(index) + " was " + ValueTypeToString(val.m_Type) + ", expected a " + ValueTypeToString(expectedType)); \
//}

void BytecodeFunctions::InitializeDefaultFunctions()
{
	//NativeFunctions["__print_stack"] = &__print_stack;
	NativeFunctions["print"] = &print;
	NativeFunctions["printf"] = &printf;
	/*NativeFunctions["to_string"] = &to_string;
	NativeFunctions["to_string_raw"] = &to_string_raw;
	NativeFunctions["to_number"] = &to_number;
	NativeFunctions["typeof"] = &typeof;
	NativeFunctions["floor"] = &floor;
	NativeFunctions["round"] = &round;
	NativeFunctions["ceil"] = &ceil;
	NativeFunctions["random"] = &random;
	NativeFunctions["restart"] = &restart;
	NativeFunctions["to_char"] = &to_char;
	NativeFunctions["to_ascii_code"] = &to_ascii_code;
	NativeFunctions["format_string"] = &format_string;
	NativeFunctions["length"] = &length;
	NativeFunctions["at"] = &at;

	NativeFunctions["time_now"] = &time_now;*/
	//NativeFunctions["exit"] = &exit;

	//NativeFunctions["sleep_for"] = &sleep_for;
	//NativeFunctions["get_next_arg"] = &get_next_arg;

	///* Arrays */
	//NativeFunctions["array_push"] = &array_push;
	//NativeFunctions["array_concat"] = &array_concat;
	//NativeFunctions["array_pop"] = &array_pop;
	//NativeFunctions["array_at"] = &array_at;
	//NativeFunctions["array_reverse"] = &array_reverse;

	///* Threads */
	//NativeFunctions["thread_start"] = &thread_start;

	///*NativeFunctions["execute_program_source"] = &execute_program_source;*/

	//NativeFunctions["console_read_line"] = &console_read_line;
	//NativeFunctions["console_init_drawing"] = &console_init_drawing;
	//NativeFunctions["console_write_pixel"] = &console_write_pixel;
	//NativeFunctions["console_read_pixel"] = &console_read_pixel;
	//NativeFunctions["console_fill"] = &console_fill;
	//NativeFunctions["console_clear"] = &console_clear;
	//NativeFunctions["console_update"] = &console_update;
	//NativeFunctions["console_width"] = &console_width;
	//NativeFunctions["console_height"] = &console_height;

	//NativeFunctions["read_file"] = &read_file;
	//NativeFunctions["write_file"] = &write_file;

	///* Network */
	//NativeFunctions["open_socket"] = &open_socket;
	//NativeFunctions["send_socket"] = &send_socket;
	//NativeFunctions["receive_socket"] = &receive_socket;

	//NativeFunctions["get_key_down"] = &get_key_down;

	srand((unsigned int)time(0));
}

CallableFunction BytecodeFunctions::GetFunctionByName(std::string name)
{
	CallableFunction func = (CallableFunction)NativeFunctions[name];

	return func;
}

void BytecodeFunctions::ThrowException(std::string error)
{

}
//
//StackValue BytecodeFunctions::__print_stack(StackValueArray* args)
//{
//	std::cout << "\nOperand stack	   Variables stack\n";
//	std::cout << "Top: " << Runner::Get().GetContext(0)->GetTopFrame().m_OperandStackTop << "\n";
//	for (int i = 0; i < 16; i++)
//	{
//		StackValue& stack = Runner::Get().GetContext(0)->GetTopFrame().m_OperandStack[i];
//		StackValue& variable = Runner::Get().GetContext(0)->GetTopFrame().m_VariablesList[i];
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
//	return StackValue(Value::Null);
//}

StackValue BytecodeFunctions::print(StackValueArray* args)
{
	std::string printed;
	for (int i = 0; i < args->size(); i++)
	{
		printed += args->at(i).ToFormattedString(false);

		if (i < args->size() - 1) printed += " ";
	}

	std::cout << printed << "\n";

	return StackValue(ValueTypes::Void);
}


StackValue BytecodeFunctions::printf(StackValueArray* args)
{
	StackValue formatted = format_string(args);
	std::cout << formatted.ToString() << "\n";

	// Cleanup
	BytecodeInterpreter::Get().m_Heap.DeleteObject(formatted.m_HeapEntryPointer);

	return StackValue(ValueTypes::Void);
}

StackValue BytecodeFunctions::format_string(StackValueArray* args)
{
	//EnsureTypeOfArg(args, 0, ValueTypes::String);

	std::string formatted = args->at(0).GetString();

	if (args->size() == 1)
		return args->at(0);

	// Iterate all args
	for (int i = 1; i < args->size(); i++)
	{
		std::size_t charPos = formatted.find('$');
		if (charPos == std::string::npos) // Nothing to format
			return BytecodeInterpreter::Get().m_Heap.CreateString(formatted);

		// Remove the '$'
		formatted.erase(charPos, 1);

		// Insert the thing at that position
		formatted = formatted.insert(charPos, args->at(i).ToString());
	}

	return BytecodeInterpreter::Get().m_Heap.CreateString(formatted);
}

std::map<std::string, CallableFunction> BytecodeFunctions::NativeFunctions;