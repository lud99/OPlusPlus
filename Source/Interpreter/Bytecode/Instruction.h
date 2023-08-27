#pragma once

#include <string>
#include <unordered_map>

namespace Ö::Bytecode
{
	enum class Opcodes
	{
		// Push opcodes
		push_ibyte,
		push_ishort,
		push_sref,

		// Load from constants pool
		load_constant,
		load_constant_wide_index,
		load_wconstant,

		// Store
		store_i,
		store_f,
		store_sref,

		// Store into class member
		store_member,
		load_member,

		array_create,
		array_create_empty, // Pushes a reference to a newly created empty array

		object_create,
		object_create_empty,

		pop, // Pops the top of the operand stack (discarding it)

		store, // Pop the top operand (number or string reference) and store it in local variable at an index
		store_property,
		load, // Load a value from a local variable at index and push it onto the operand stack
		load_property, // Loads a property from an object (index) onto the stack

		add, // Pop the 2 values on the stack, add them, and push the result onto the stack
		sub,
		sub_reverse, // Subtraction. n2 - n1
		mul,
		div,
		div_reverse, // Division. n2 / n1
		xr, // XOR
		xr_rev,
		pow, // base, exponent
		pow_rev,
		mod,
		mod_rev,
		post_inc, // Increments a local variable by 1
		pre_inc,
		post_dec, // Decrements a local variable by 1
		pre_dec,

		jmp, // Jumps to an instruction (sets the program counter)
		ret, // Returns the top value on the stack, from a function back to where it was called from
		ret_void, // Returns 'null' from a function back to where it was called from
		thread_end, // Ends thread execution and returns
		jmp_if_true, // Jumps to an instruction if the top of the stack is 1
		jmp_if_false, // Jumps to an instruction if the top of the stack is 1
		eq, // 1 if equals, 0 if not
		neq, //  0 if equals, 1 if it does
		cmpgt, // 1 if top of stack is greater than the below value
		cmpge, // 1 if top of stack is greater than or equal to the below value
		cmplt, // 1 if top of stack is less than the below value
		cmple, // 1 if top of stack is less than or equal to the below value
		logical_and, // Pops 2 from stack and returns if both are true
		logical_or, // Pops 2 from stack and returns if one of them are true
		logical_not,

		cmp, // 1 if equal, 0 if not

		create_scope_frame,
		pop_scope_frame,
		create_function_frame,
		pop_stack_frame,

		loop_break,
		loop_continue,

		call, // Calls a function from a reference. Tha arguments must have been pushed to the stack
		call_native, // {name}, {arg count}
		skip_function, // Skips the function that is below. Used to skip functions that have not been called. x = end of function

		no_op, // Does nothing
		stop
	};

	static std::string OpcodeToString(Opcodes opcode)
	{
		std::string names[] = {
			"push_ibyte",
			"push_ishort",
			"push_sref",

			"load_constant",
			"load_constant_wide_index",
			"load_wconstant",

			"store_i",
			"store_f",
			"store_sref",

			"store_member",
			"load_member",

			"array_create",
			"array_create_empty",
			"object_create",
			"object_create_empty",

			"pop",
			"store",
			"store_property",
			"load",
			"load_property",

			"add",
			"sub",
			"sub_reverse",
			"mul",
			"div",
			"div_reverse",
			"xr",
			"xr_rev",
			"pow",
			"pow_rev",
			"mod",
			"mod_rev",
			"post_inc",
			"pre_inc",
			"post_dec",
			"pre_dec",

			"jmp",
			"ret",
			"ret_void",
			"thread_end",
			"jmp_if_true",
			"jmp_if_false",
			"eq",
			"neq",
			"cmpgt",
			"cmpge",
			"cmplt",
			"cmple",
			"logical_and",
			"logical_or",
			"logical_not",

			"cmp",

			"create_scope_frame",
			"pop_scope_frame",
			"create_function_frame",
			"pop_stack_frame",

			"loop_break",
			"loop_continue",

			"call",
			"call_native",
			"skip_function",

			"no_op",
			"stop"
		};

		return names[(int)opcode];
	}

	class Instruction
	{
	public:
		Instruction(Opcodes opcode) : m_Opcode(opcode) {};
		Instruction(Opcodes opcode, std::vector<uint8_t> arguments) : m_Opcode(opcode), m_Arguments(arguments) {};

		Opcodes GetOpcode() { return m_Opcode; };
		const std::vector<uint8_t>& GetArguments() { return m_Arguments; };

	private:
		Opcodes m_Opcode;
		std::vector<uint8_t> m_Arguments;
	};

	typedef std::vector<Instruction> Instructions;
	typedef std::vector<uint8_t> EncodedInstructions;

	class InstructionsEncoder
	{
	public:
		void AddOpcode(Opcodes opcode) { m_Data.emplace_back((uint8_t)opcode); };
		void AddInt(int value) { m_Data.emplace_back((uint8_t)value); };

		const EncodedInstructions& GetData() { return m_Data; };

	private:
		EncodedInstructions m_Data;
	};
}