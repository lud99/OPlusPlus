#pragma once

#include <string>
#include <unordered_map>

#include "../../Lexer.h"
#include "../../Parser.h"
#include "../Value.h"

// https://dzone.com/articles/introduction-to-java-bytecode

namespace Bytecode {
	enum class Opcodes
	{
		push_number,
		push_stringconst, // Push a string reference onto the top of the stack. arg = stringIndex
		push_null, // Pushes a null
		push_functionpointer, // Pushes a pointer (number) to the start of a function

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
			"push_number",
			"push_stringconst",
			"push_null",
			"push_functionpointer",
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

	class InstructionArgument : public Value
	{
	public:
		using Value::Value;

		std::string m_ArgStringValue = "";
		int m_ArgIntValue = 0;
	};

	constexpr int InstructionArgSize = 8;
	struct Instruction
	{
	public:
		Instruction() {};
		Instruction(Opcodes type) : m_Type(type) {};
		Instruction(Opcodes type, bool discardValue) : m_Type(type), m_DiscardValue(discardValue) {};

		Instruction& Arg(InstructionArgument arg);
		Instruction& Arg(int arg);
		Instruction& Arg(double arg, ValueTypes type);
		Instruction& Arg(std::string arg);

	public:
		Opcodes m_Type = Opcodes::no_op;
		bool m_DiscardValue = false;

		InstructionArgument m_Arguments[InstructionArgSize];

		uint8_t m_ArgsCount = 0;
	};

	typedef std::vector<Instruction> Instructions;

	struct ConstantsPool
	{
		int m_NumericConstants[STACK_SIZE];
		HeapEntry m_StringConstants[STACK_SIZE];

		std::unordered_map<std::string, Instructions> m_GlobalFunctions;

		uint32_t m_FreeNumericSlot = 0;
		uint32_t m_FreeStringSlot = 0;
	};

	class BytecodeConverterContext
	{
	public:
		struct Variable
		{
			std::string m_Name = "";
			int m_Index = -1;

			bool m_IsGlobal = false;

			ValueTypes m_Type = ValueTypes::Void;

			Variable(int index = -1, std::string name = "", ValueTypes type = ValueTypes::Void, bool isGlobal = false) : m_Name(name), m_Index(index), m_Type(type), m_IsGlobal(isGlobal) {};
		};

		struct LoopInfo
		{
			bool m_InLoop = false;
			int m_Reset = -1;
			int m_End = -1;
			int m_BodyDepth = -1;
		};

	public:
		uint32_t AddStringConstant(ConstantsPool& constants, std::string string);

		Variable GetVariable(std::string& variableName);
		bool CreateVariableIndex(std::string& variableName, ValueTypes type, int& index); // Returns false if the variable exists, true if it was created
		bool CreateVariableIndex(Variable& variable); // Returns false if the variable exists, true if it was created. Sets the index on the variable object
		int CreateVariableIndex(std::string& variableName, ValueTypes type);

	public:
		std::unordered_map<std::string, Variable> m_Variables;
		std::unordered_map<std::string, uint32_t> m_IndiciesForStringConstants;

		uint32_t m_NextFreeVariableIndex = 0;
		uint32_t m_NextFreeStringConstantIndex = 0;

		bool m_IsModule = false;
		bool m_ShouldExportVariable = false;
		int m_ModuleIndex = -1;
		bool m_IsThreadedFunction = false;

		LoopInfo m_LoopInfo;
	};

	class BytecodeCompiler
	{
	public:
		BytecodeCompiler();

		void Compile(ASTNode* node, std::vector<Instruction>& instructions, bool canCreateScope = true, bool canMakeVariablesLocal = true);

		void ExportVariable(ASTNode* node, BytecodeConverterContext::Variable& variable, std::vector<Instruction>& instructions);

		void PreCompileFunction(ASTNode* node);
		int CompileFunction(ASTNode* node, std::vector<Instruction>& instructions);
		void PreCompileAnonymousFunction(ASTNode* node);
		int CompileAnonymousFunction(ASTNode* node, std::vector<Instruction>& instructions);

		void CompileAssignment(ASTNode* node, std::vector<Instruction>& instructions);

		void Throw(std::string error);

		~BytecodeCompiler() {};

	public:
		std::string m_Error;

		uint32_t m_StartExecutionAt = 0;
		ConstantsPool m_Constants;

		BytecodeConverterContext m_Context;

		uint32_t m_CurrentScope = 0;
	};
}