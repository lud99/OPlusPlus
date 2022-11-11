#pragma once

#include <vector>
#include <string>

#include "Value.h"
#include <map>

typedef std::vector<Value> ValueArray;

typedef Value(*CallableFunction)(ValueArray);

namespace BytecodeFunctions
{
	extern std::map<std::string, CallableFunction> NativeFunctions;

	void InitializeDefaultFunctions();
	CallableFunction GetFunctionByName(std::string name);

	void ThrowException(std::string error);

	// Debug
	//StackValue __print_stack(StackValueArray* args);

	StackValue print(StackValueArray* args);
	StackValue printf(StackValueArray* args);
	/*StackValue to_string(StackValueArray* args);
	StackValue to_string_raw(StackValueArray* args);
	StackValue to_number(StackValueArray* args);
	StackValue typeof(StackValueArray* args);
	StackValue floor(StackValueArray* args);
	StackValue round(StackValueArray* args);
	StackValue ceil(StackValueArray* args);
	StackValue random(StackValueArray* args);
	StackValue restart(StackValueArray* args);
	StackValue to_char(StackValueArray* args);
	StackValue to_ascii_code(StackValueArray* args);*/
	StackValue format_string(StackValueArray* args);

	/*StackValue time_now(StackValueArray* args);

	StackValue length(StackValueArray* args);
	StackValue at(StackValueArray* args);

	StackValue sleep_for(StackValueArray* args);*/

	//StackValue get_next_arg(StackValueArray* args);

	/* Arrays */
	//StackValue array_push(StackValueArray* args);
	//StackValue array_concat(StackValueArray* args);
	//StackValue array_pop(StackValueArray* args);
	//StackValue array_at(StackValueArray* args);
	//StackValue array_reverse(StackValueArray* args);

	///* Threads */
	//StackValue thread_start(StackValueArray* args);

	//StackValue execute_program_source(StackValueArray* args);

	/* IO */
	//StackValue console_read_line(StackValueArray* args);
	//StackValue console_init_drawing(StackValueArray* args);
	//StackValue console_write_pixel(StackValueArray* args);
	//StackValue console_read_pixel(StackValueArray* args);
	//StackValue console_fill(StackValueArray* args);
	//StackValue console_clear(StackValueArray* args);
	//StackValue console_update(StackValueArray* args);
	//StackValue console_width(StackValueArray* args);
	//StackValue console_height(StackValueArray* args);
	//StackValue open_file(StackValueArray* args);
	//StackValue close_file(StackValueArray* args);
	//StackValue read_file(StackValueArray* args);
	//StackValue write_file(StackValueArray* args);

	///* Network */
	//StackValue open_socket(StackValueArray* args);
	//StackValue send_socket(StackValueArray* args);
	//StackValue receive_socket(StackValueArray* args);

	//StackValue exit(StackValueArray* args);

	//StackValue get_key_down(StackValueArray* args);
}