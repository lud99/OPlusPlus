#include "Functions.h"

#include <time.h>
#include <iostream>	

#include "Bytecode/BytecodeInterpreter.h"

namespace Ö
{

//BuiltInFunction* BuiltInFunctions::GetFunction(uint16_t id)
//{
//	// If the specified function doesn't exist
//	if (id > m_BuiltInFunctions.size() - 1)
//		return nullptr;

//	return &m_BuiltInFunctions[id];
//}

//CallableFunction BuiltInFunctions::GetFunctionPointer(uint16_t id)
//{
//	// If the specified function doesn't exist
//	if (id > m_BuiltInFunctions.size() - 1)
//		return nullptr;

//	return m_BuiltInFunctions[id].functionPointer;
//}
//
//void BuiltInFunctions::ThrowException(std::string error)
//{
//
//}
//
//void BuiltInFunctions::AddFunction(std::string name, CallableFunction pointer, ValueType returnType)
//{
//	assert(m_FunctionNameToId.count(name) != 0); // Don't allow duplicate functions
//
//	uint16_t id = m_CurrentFreeId++;
//	m_FunctionNameToId[name] = id;
//
//	m_BuiltInFunctions.emplace_back(id, name, pointer, returnType);
//}

//Value Functions::print(ARGS)
//{
//	//const std::string& base = args[0].GetString();
//
//	if (args.empty())
//	{
//		//ThrowException("Function 'print' cannot be called without arguments");
//		return Value(ValueTypes::Void);
//	}
//
//	/*if (args.size() == 1)
//		printf(base.c_str());
//	else if (args.size() == 2)
//		printf(base.c_str(), args[1].ToFormattedString(false).c_str());
//	else if (args.size() == 3)
//		printf(base.c_str(), args[1].ToFormattedString(false), args[2].ToFormattedString(false));
//	else if (args.size() == 4)
//		printf(base.c_str(), args[1].ToFormattedString(false), args[2].ToFormattedString(false), args[3].ToFormattedString(false));
//	else if (args.size() == 5)
//		printf(base.c_str(), args[1].ToFormattedString(false), args[2].ToFormattedString(false), args[3].ToFormattedString(false), args[4].ToFormattedString(false));
//	else
//	{
//		ThrowException("Function doesn't accept more than 5 arguments");
//		return Value(ValueTypes::Void);
//	}*/
//
//	std::string printed;
//	for (int i = 0; i < args.size(); i++)
//	{
//		printed += args.at(i).ToFormattedString(false);
//
//		if (i < args.size() - 1) printed += " ";
//	}
//
//	std::cout << printed;
//
//	return Value(ValueTypes::Void);
//}

Value BuiltInFunctions::_print(ARGS)
{
	std::cout << args[0].ToFormattedString() << "\n";

	return Value(ValueTypes::Void);
}

Value BuiltInFunctions::_printf(ARGS)
{
	Value formatted;// = format_string(args);
	std::cout << formatted.ToString();

	// Cleanup
	Bytecode::BytecodeInterpreter::Get().m_Heap.DeleteObject(formatted.m_HeapEntryPointer);

	return Value(ValueTypes::Void);
}

//Value BuiltInFunctions::format_string(ARGS)
//{
//	//EnsureTypeOfArg(args, 0, ValueTypes::String);

//	std::string formatted = args.at(0).GetString();

//	if (args.size() == 1)
//		return Bytecode::BytecodeInterpreter::Get().m_Heap.CreateString(formatted);

//	std::string formats[] = {
//		"%f", "%i", "%s"
//	};

//	int argIndex = 1;
//	for (int j = 0; j < 3; j++)
//	{
//		for (int i = 1; i < args.size(); i++)
//		{
//			std::size_t charPos = formatted.find(formats[j]);

//			if (charPos != std::string::npos) // Something to format
//			{
//				// Remove the '%.'
//				formatted.erase(charPos, 2);

//				// Insert the thing at that position
//				formatted = formatted.insert(charPos, args.at(argIndex).ToString());
//				argIndex++;
//			}
//		}
//	}

//	return Bytecode::BytecodeInterpreter::Get().m_Heap.CreateString(formatted);
//}

//Value BuiltInFunctions::_rand(ARGS)
//{
//	return Value(rand(), ValueTypes::Integer);
//}

//Value BuiltInFunctions::_srand(ARGS)
//{
//	srand(args[0].GetInt());

//	return Value(ValueTypes::Void);
//}

//Value BuiltInFunctions::_time(ARGS)
//{
//	return Value((int)time(0), ValueTypes::Integer);
//}

//Value BuiltInFunctions::rand_range(ARGS)
//{
//	int min = args[0].GetInt();
//	int max = args[1].GetInt();

//	int value = min + rand() / (RAND_MAX / (max - min + 1) + 1);
//	return Value(value, ValueTypes::Integer);
//}

//Value BuiltInFunctions::_sin(ARGS)
//{
//	float argument = args[0].GetFloat();
//	return Value(std::sin(argument), ValueTypes::Float);
//}

//Value BuiltInFunctions::_cos(ARGS)
//{
//	float argument = args[0].GetFloat();
//	return Value(std::cos(argument), ValueTypes::Float);
//}

//Value BuiltInFunctions::_tan(ARGS)
//{
//	float argument = args[0].GetFloat();
//	return Value(std::tan(argument), ValueTypes::Float);
//}

//Value BuiltInFunctions::_sqrt(ARGS)
//{
//	float argument = args[0].GetFloat();
//	return Value(std::sqrt(argument), ValueTypes::Float);
//}

//Value BuiltInFunctions::_pow(ARGS)
//{
//	float base = args[0].GetFloat();
//	float exponent = args[1].GetFloat();
//	return Value(std::pow(base, exponent), ValueTypes::Float);
//}

//Value BuiltInFunctions::abs_float(ARGS)
//{
//	float arg = args[0].GetFloat();
//	return Value(fabs(arg), ValueTypes::Float);
//}

///*Value BuiltInFunctions::rand_range_float(ARGS)
//{
//	float min = args[0].GetFloat();
//	float max = args[1].GetFloat();

//	float value = min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
//	return Value(value, ValueTypes::Float);
//}*/

//Value BuiltInFunctions::to_int(ARGS)
//{
//	assert(args[0].GetType() == ValueTypes::Float);

//	return Value((int)args[0].GetFloat(), ValueTypes::Integer);
//}

//Value BuiltInFunctions::to_float(ARGS)
//{
//	assert(args[0].GetType() == ValueTypes::Integer);

//	return Value((float)args[0].GetInt(), ValueTypes::Float);
//}
}