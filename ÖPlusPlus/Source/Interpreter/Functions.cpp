#include "Functions.h"

#include <time.h>
#include <iostream>	

#include "Bytecode/BytecodeInterpreter.h"

void Functions::InitializeDefaultFunctions(ExecutionMethods method)
{
	m_ExecutionMethod = method;

	//NativeFunctions["__print_stack"] = &__print_stack;
	NativeFunctions["print"] = &_printf;
	NativeFunctions["printf"] = &_printf;
	NativeFunctions["rand"] = &_rand;
	NativeFunctions["srand"] = &_srand;
	NativeFunctions["time"] = &_time;
	NativeFunctions["rand_range"] = &rand_range;
	//NativeFunctions["rand_range_float"] = &rand_range_float;

	NativeFunctions["sin"] = &_sin;
	NativeFunctions["cos"] = &_cos;
	NativeFunctions["tan"] = &_tan;
	NativeFunctions["sqrt"] = &_sqrt;
	NativeFunctions["pow"] = &_pow;

	NativeFunctions["to_int"] = &to_int;
	NativeFunctions["to_float"] = &to_float;

	NativeFunctions["abs_float"] = &abs_float;
	
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

	srand(time(0));
}

CallableFunction Functions::GetFunctionByName(std::string name)
{
	if (NativeFunctions.count(name) == 0)
		return nullptr;

	return (CallableFunction)NativeFunctions[name];
}

void Functions::ThrowException(std::string error)
{

}

Value Functions::print(ARGS)
{
	const std::string& base = args[0].GetString();
	
	if (args.empty())
	{
		ThrowException("Function 'print' cannot be called without arguments");
		return Value(ValueTypes::Void);
	}
		
	/*if (args.size() == 1)
		printf(base.c_str());
	else if (args.size() == 2)
		printf(base.c_str(), args[1].ToFormattedString(false).c_str());
	else if (args.size() == 3)
		printf(base.c_str(), args[1].ToFormattedString(false), args[2].ToFormattedString(false));
	else if (args.size() == 4)
		printf(base.c_str(), args[1].ToFormattedString(false), args[2].ToFormattedString(false), args[3].ToFormattedString(false));
	else if (args.size() == 5)
		printf(base.c_str(), args[1].ToFormattedString(false), args[2].ToFormattedString(false), args[3].ToFormattedString(false), args[4].ToFormattedString(false));
	else
	{
		ThrowException("Function doesn't accept more than 5 arguments");
		return Value(ValueTypes::Void);
	}*/

	std::string printed;
	for (int i = 0; i < args.size(); i++)
	{
		printed += args.at(i).ToFormattedString(false);

		if (i < args.size() - 1) printed += " ";
	}

	std::cout << printed;

	return Value(ValueTypes::Void);
}

Value Functions::_printf(ARGS)
{
	Value formatted = format_string(args);
	std::cout << formatted.ToString();

	// Cleanup
	if (m_ExecutionMethod == ExecutionMethods::Bytecode)
		Bytecode::BytecodeInterpreter::Get().m_Heap.DeleteObject(formatted.m_HeapEntryPointer);
		 
	return Value(ValueTypes::Void);
}

Value Functions::format_string(ARGS)
{
	//EnsureTypeOfArg(args, 0, ValueTypes::String);

	std::string formatted = args.at(0).GetString();

	if (args.size() == 1)
	{
		if (m_ExecutionMethod == ExecutionMethods::Bytecode)
			return Bytecode::BytecodeInterpreter::Get().m_Heap.CreateString(formatted);

		if (m_ExecutionMethod == ExecutionMethods::AST)
			return Value(formatted, ValueTypes::String);
	}
		

	std::string formats[] = {
		"%f", "%i", "%s"
	};

	int argIndex = 1;
	for (int j = 0; j < 3; j++)
	{
		for (int i = 1; i < args.size(); i++)
		{
			std::size_t charPos = formatted.find(formats[j]);

			if (charPos != std::string::npos) // Something to format
			{
				// Remove the '%.'
				formatted.erase(charPos, 2);

				// Insert the thing at that position
				formatted = formatted.insert(charPos, args.at(argIndex).ToString());
				argIndex++;
			}
		}
	}
	
	if (m_ExecutionMethod == ExecutionMethods::Bytecode)
		return Bytecode::BytecodeInterpreter::Get().m_Heap.CreateString(formatted);
	if (m_ExecutionMethod == ExecutionMethods::AST)
		return Value(formatted, ValueTypes::String);
}

Value Functions::_rand(ARGS)
{
	return Value(rand(), ValueTypes::Integer);
}

Value Functions::_srand(ARGS)
{
	srand(args[0].GetInt());

	return Value(ValueTypes::Void);
}

Value Functions::_time(ARGS)
{
	return Value((int)time(0), ValueTypes::Integer);
}

Value Functions::rand_range(ARGS)
{
	int min = args[0].GetInt();
	int max = args[1].GetInt();

	int value = min + rand() / (RAND_MAX / (max - min + 1) + 1);
	return Value(value, ValueTypes::Integer);
}

Value Functions::_sin(ARGS)
{
	float argument = args[0].GetFloat();
	return Value(std::sin(argument), ValueTypes::Float);
}

Value Functions::_cos(ARGS)
{
	float argument = args[0].GetFloat();
	return Value(std::cos(argument), ValueTypes::Float);
}

Value Functions::_tan(ARGS)
{
	float argument = args[0].GetFloat();
	return Value(std::tan(argument), ValueTypes::Float);
}

Value Functions::_sqrt(ARGS)
{
	float argument = args[0].GetFloat();
	return Value(std::sqrt(argument), ValueTypes::Float);
}

Value Functions::_pow(ARGS)
{
	float base = args[0].GetFloat();
	float exponent = args[1].GetFloat();
	return Value(std::pow(base, exponent), ValueTypes::Float);
}

Value Functions::abs_float(ARGS)
{
	float arg = args[0].GetFloat();
	return Value(fabs(arg), ValueTypes::Float);
}

/*Value Functions::rand_range_float(ARGS)
{
	float min = args[0].GetFloat();
	float max = args[1].GetFloat();

	float value = min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
	return Value(value, ValueTypes::Float);
}*/

Value Functions::to_int(ARGS)
{
	assert(args[0].GetType() == ValueTypes::Float);

	return Value((int)args[0].GetFloat(), ValueTypes::Integer);
}

Value Functions::to_float(ARGS)
{
	assert(args[0].GetType() == ValueTypes::Integer);

	return Value((float)args[0].GetInt(), ValueTypes::Float);
}

std::map<std::string, CallableFunction> Functions::NativeFunctions;
ExecutionMethods Functions::m_ExecutionMethod;