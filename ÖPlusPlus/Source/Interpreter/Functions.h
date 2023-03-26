#pragma once

#include <vector>
#include <string>
#include <iostream>

#include "../Utils.hpp"

#include <map>

#include "Value.h"

#define ARGS ValueArray args

typedef Value(*CallableFunction)(ARGS);

namespace Functions
{
	extern std::map<std::string, CallableFunction> NativeFunctions;

	extern ExecutionMethods m_ExecutionMethod;

	void InitializeDefaultFunctions(ExecutionMethods method);
	CallableFunction GetFunctionByName(std::string name);

	void ThrowException(std::string error);

	// Debug
	//StackValue __print_stack(ValueArray* args);

	Value print(ARGS);
	Value _printf(ARGS);
	Value _rand(ARGS);
	Value _srand(ARGS);
	Value _time(ARGS);
	Value rand_range(ARGS);
	//Value rand_range_float(ARGS);

	Value _sin(ARGS);
	Value _cos(ARGS);
	Value _tan(ARGS);
	Value _sqrt(ARGS);
	Value _pow(ARGS);

	Value abs_float(ARGS);

	Value to_int(ARGS);
	Value to_float(ARGS);

	//Value srand(ARGS);
	//Value time(ARGS);
	/*StackValue to_string(ARGS);
	StackValue to_string_raw(ARGS);
	StackValue to_number(ARGS);
	StackValue typeof(ARGS);
	StackValue floor(ARGS);
	StackValue round(ARGS);
	StackValue ceil(ARGS);
	StackValue random(ARGS);
	StackValue restart(ARGS);
	StackValue to_char(ARGS);
	StackValue to_ascii_code(ARGS);*/
	Value format_string(ARGS);

	/*StackValue time_now(ARGS);

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