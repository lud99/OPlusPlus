#include "Functions.h"

#include <time.h>
#include <iostream>	

void Functions::InitializeDefaultFunctions()
{
	//NativeFunctions["__print_stack"] = &__print_stack;
	NativeFunctions["print"] = &print;
	NativeFunctions["printf"] = &printf;
	NativeFunctions["rand_range"] = &rand_range;
	
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
	return (CallableFunction)NativeFunctions[name];
}

void Functions::ThrowException(std::string error)
{

}

Value Functions::print(ARGS)
{
	std::string printed;
	for (int i = 0; i < args.size(); i++)
	{
		printed += args.at(i).ToFormattedString(false);

		if (i < args.size() - 1) printed += " ";
	}

	std::cout << printed << "\n";

	return Value(ValueTypes::Void);
}

Value Functions::rand_range(ARGS)
{
	int min = args[0].GetInt();
	int max = args[1].GetInt();

	int value = min + rand() / (RAND_MAX / (max - min + 1) + 1);
	return Value(value, ValueTypes::Integer);
}


// Stub the functions if in assembly mode
#ifdef ASM
Value Functions::printf(ARGS) { return Value(); };
#endif

std::map<std::string, CallableFunction> Functions::NativeFunctions;