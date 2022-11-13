#pragma once

#include <vector>
#include <string>

#include <map>

#include "../Value.h"

typedef Value(*CallableFunction)(ValueArray*);

namespace BytecodeFunctions
{
	extern std::map<std::string, CallableFunction> NativeFunctions;

	void InitializeDefaultFunctions();
	CallableFunction GetFunctionByName(std::string name);

	void ThrowException(std::string error);

	// Debug
	//StackValue __print_stack(ValueArray* args);

	Value print(ValueArray* args);
	Value printf(ValueArray* args);
	/*StackValue to_string(ValueArray* args);
	StackValue to_string_raw(ValueArray* args);
	StackValue to_number(ValueArray* args);
	StackValue typeof(ValueArray* args);
	StackValue floor(ValueArray* args);
	StackValue round(ValueArray* args);
	StackValue ceil(ValueArray* args);
	StackValue random(ValueArray* args);
	StackValue restart(ValueArray* args);
	StackValue to_char(ValueArray* args);
	StackValue to_ascii_code(ValueArray* args);*/
	Value format_string(ValueArray* args);

	/*StackValue time_now(ValueArray* args);

	StackValue length(ValueArray* args);
	StackValue at(ValueArray* args);

	StackValue sleep_for(ValueArray* args);*/

	//StackValue get_next_arg(ValueArray* args);

	/* Arrays */
	//StackValue array_push(ValueArray* args);
	//StackValue array_concat(ValueArray* args);
	//StackValue array_pop(ValueArray* args);
	//StackValue array_at(ValueArray* args);
	//StackValue array_reverse(ValueArray* args);

	///* Threads */
	//StackValue thread_start(ValueArray* args);

	//StackValue execute_program_source(ValueArray* args);

	/* IO */
	//StackValue console_read_line(ValueArray* args);
	//StackValue console_init_drawing(ValueArray* args);
	//StackValue console_write_pixel(ValueArray* args);
	//StackValue console_read_pixel(ValueArray* args);
	//StackValue console_fill(ValueArray* args);
	//StackValue console_clear(ValueArray* args);
	//StackValue console_update(ValueArray* args);
	//StackValue console_width(ValueArray* args);
	//StackValue console_height(ValueArray* args);
	//StackValue open_file(ValueArray* args);
	//StackValue close_file(ValueArray* args);
	//StackValue read_file(ValueArray* args);
	//StackValue write_file(ValueArray* args);

	///* Network */
	//StackValue open_socket(ValueArray* args);
	//StackValue send_socket(ValueArray* args);
	//StackValue receive_socket(ValueArray* args);

	//StackValue exit(ValueArray* args);

	//StackValue get_key_down(ValueArray* args);
}