#pragma once

#include "BytecodeInterpreter.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <assert.h>
#include <thread>

#include "../../Utils.hpp"
#include "BytecodeFunctions.h"
#include "../../Parser.h"

BytecodeInterpreter& BytecodeInterpreter::Get()
{
	static BytecodeInterpreter instance;
	return instance;
}

Value BytecodeInterpreter::CreateAndRunProgram(std::string filepath, std::string& error, bool verbose)
{
	std::ifstream file(filepath);
	if (!file.good())
		std::cout << "Couldn't open file " << filepath << "\n\n";

	std::string fileContent = "";
	for (std::string line; std::getline(file, line);)
	{
		fileContent += line + "\n";
	}

	Lexer lexer;
	error = lexer.CreateTokens(fileContent);
	if (error != "")
		std::cout << error << "\n\n";

	for (int i = 0; i < lexer.m_Tokens.size(); i++)
		std::cout << lexer.m_Tokens[i].ToString() << ": " << lexer.m_Tokens[i].m_Value << " [" << lexer.m_Tokens[i].m_Depth << "]\n";
	std::cout << "\n";

	Parser parser;

	ASTNode tree;
	tree.parent = new ASTNode(ASTTypes::ProgramBody);
	tree.parent->left = &tree;

	parser.CreateAST(lexer.m_Tokens, &tree, tree.parent);

	if (parser.m_Error != "")
		std::cout << "AST Error: " << parser.m_Error << "\n";

	parser.PrintASTTree(tree.parent, 0);

	std::vector<Instruction> instructions;
	m_Compiler.Compile(tree.parent, instructions);

	//m_ProgramCounter = m_Compiler.m_StartExecutionAt;
	m_ConstantsPool = m_Compiler.m_Constants;

	// Print
	std::cout << "\n";
	for (int i = 0; i < instructions.size(); i++)
	{
		std::cout << "(" << i << ") " << OpcodeToString(instructions[i].m_Type) << " ";

		for (int a = 0; a < INSTRUCTION_ARG_SIZE; a++)
		{
			if (instructions[i].m_Arguments[a].GetType() != ValueTypes::Void)
			{
				if (a > 0 && a < INSTRUCTION_ARG_SIZE - 1) std::cout << ", ";
				std::cout << instructions[i].m_Arguments[a].ToString();
			}
		}

		std::cout << "\n";
	}
	//std::cout << "Program counter: " << m_ProgramCounter << "\n\n";

	if (m_Compiler.m_Error != "")
	{
		error = "Bytecode compilation error: " + m_Compiler.m_Error;
		return Value();
	}

	std::cout << sizeof(StackFrame) << ", " << sizeof(Value) << ", " << sizeof(ExecutionContext) << "\n";

	m_Debugger = Debugger(instructions);
	m_Debugger.m_Enabled = false;

	

	auto start = std::chrono::high_resolution_clock::now();

	std::cout << "Console output:\n";

	m_Instructions = instructions;
	std::string exception = InterpretBytecode();

	if (exception != "")
		std::cout << "Bytecode execution error: (" << GetContext(0)->m_ProgramCounter - 1 << ") " << exception << "\n";

	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

	std::cout << "\nExecution took: " << (duration.count()) << "ms" << "\n";

	return Value();
}

std::string BytecodeInterpreter::InterpretBytecode()
{
	// Main thread
	ExecutionContext* ctx = CreateContext();
	ctx->m_Instructions = m_Instructions;

	ctx->Execute();

	if (ctx->Exception())
		return ctx->m_Exception;

	return "";
}

StackFrame::StackFrame()
{
}
StackFrame::StackFrame(ExecutionContext* ctx)
{
	m_Context = ctx;
}

void StackFrame::Alloc()
{
	/*m_InitialVariablesList = new Value[STACK_SIZE];
	m_VariablesList = new Value[STACK_SIZE];
	m_OperandStack = new Value[STACK_SIZE];*/
}

Value& StackFrame::GetVariable(uint32_t index)
{
	// If out of range, throw exception
	assert(index >= 0 && index < STACK_SIZE);

	assert(m_VariablesList[index].GetType() != ValueTypes::Void);

	return m_VariablesList[index];
}

Value StackFrame::PopOperand()
{
	// If no operands to pop, throw exception
	assert(m_OperandStackTop >= 0 && m_OperandStackTop < STACK_SIZE);

	assert(m_OperandStackTop != 0);

	m_OperandStackTop--;
	Value top = m_OperandStack[m_OperandStackTop];

	m_OperandStack[m_OperandStackTop] = Value();

	return top;
}

void StackFrame::StoreVariable(uint32_t index, Value value)
{
	assert(index >= 0 && index < STACK_SIZE);

	ValueTypes variableType = m_VariablesList[index].GetType();

	// Do type checking
	if (value.GetType() != ValueTypes::Void &&
		variableType != ValueTypes::Void &&
		variableType != ValueTypes::Void &&
		!Value::IsSamePrimitiveType(variableType, value.GetType()))

		return m_Context->ThrowExceptionVoid("Cannot store a value of type " + ValueTypeToString(value.GetType()) +
			" into a variable of type " + ValueTypeToString(variableType));

	m_VariablesList[index] = value;
}

void StackFrame::StoreVariable(uint32_t index, Value value, ValueTypes variableType)
{
	assert(index >= 0 && index < STACK_SIZE);

	if (value.GetType() != ValueTypes::Void && !Value::IsSamePrimitiveType(value.GetType(), variableType))
		return m_Context->ThrowExceptionVoid("Cannot assign a value of type " + ValueTypeToString(value.GetType()) +
			" to a variable of type " + ValueTypeToString(variableType));

	m_VariablesList[index] = value;
}

void StackFrame::PushOperand(Value value)
{
	assert(m_OperandStackTop >= 0);

	if (m_OperandStackTop >= STACK_SIZE - 1) {
		std::cout << "STACK OVERFLOW";
		abort();
	}

	m_OperandStack[m_OperandStackTop] = value;
	m_OperandStackTop++;
}

void StackFrame::PushOperand(double value)
{
	PushOperand(Value(value, ValueTypes::Float));
}
void StackFrame::PushOperand(int value)
{
	PushOperand(Value(value, ValueTypes::Integer));
}

void StackFrame::Delete()
{
	//delete m_VariablesList;
	//delete m_InitialVariablesList;
	//delete m_OperandStack;

	//m_OperandStack = nullptr;
}

StackFrame::~StackFrame()
{

}
//
//void ExecutionContext::PushFrame(StackFrame frame)
//{
//	assert(m_StackFrameTop >= 0 && m_StackFrameTop < STACKFRAME_SIZE - 1);
//
//	m_StackFrames[m_StackFrameTop];
//	m_StackFrameTop++;
//}

ExecutionContext::ExecutionContext()
{
	m_StackFrames.reserve(STACKFRAME_SIZE);
}

void ExecutionContext::PushFrame(ExecutionContext* ctx)
{
	assert(m_StackFrameTop >= 0 && m_StackFrameTop < STACKFRAME_SIZE - 1);

	m_StackFrameTop++;
	m_StackFrames[m_StackFrameTop] = StackFrame(ctx);
	m_StackFrames[m_StackFrameTop].Alloc();
}

StackFrame ExecutionContext::PopFrame()
{
	assert(m_StackFrameTop > 0 && m_StackFrameTop < STACKFRAME_SIZE);

	StackFrame top = m_StackFrames[m_StackFrameTop];

	// iterate and delete strings (might cause issues)
	/*for (int i = 0; i < STACK_SIZE; i++)
	{
		if (top.m_OperandStack[i].IsString())
		{
			delete top.m_OperandStack[i].GetString();
			((HeapEntry*)top.m_OperandStack[i].m_Value)->m_Data = nullptr;
		}

		if (top.m_VariablesList[i].IsString())
		{
			delete top.m_VariablesList[i].GetString();
			((HeapEntry*)top.m_VariablesList[i].m_Value)->m_Data = nullptr;
		}
	}*/
	//std::cout << "Pop frame\n";
	//m_StackFrames[m_StackFrameTop].Delete();

	//m_StackFrames[m_StackFrameTop] = StackFrame(nullptr); // Might be bad to reset the stacks, but who knows

	m_StackFrames[m_StackFrameTop] = StackFrame(nullptr);

	m_StackFrameTop--;

	return top;
}

StackFrame& ExecutionContext::GetTopFrame()
{
	assert(m_StackFrameTop >= 0 && m_StackFrameTop < STACKFRAME_SIZE);

	return m_StackFrames[m_StackFrameTop];
}

void ExecutionContext::Execute()
{
	// Initialize stack frames
	m_StackFrames.resize(STACKFRAME_SIZE);
	for (int i = 0; i < STACKFRAME_SIZE; i++)
	{
		m_StackFrames[i] = StackFrame(this);
		m_StackFrames[i].Alloc();
	}

	Instructions& instructions = m_Instructions;
	ConstantsPool& constants = BytecodeInterpreter::Get().m_ConstantsPool;
	Heap& heap = BytecodeInterpreter::Get().m_Heap;

	//StackFrame& rootFrame = m_StackFrames[0];
	//rootFrame.m_OperandStackTop = m_StackFrames[0].m_OperandStackTop;
	//std::copy(m_StackFrames[0].m_OperandStack, m_StackFrames[0].m_OperandStack + STACK_SIZE, rootFrame.m_OperandStack);
	//std::copy(m_StackFrames[0].m_VariablesList, m_StackFrames[0].m_VariablesList + STACK_SIZE, rootFrame.m_VariablesList);

	//m_StackFrames[0] = rootFrame;

	while (true)
	{
		BytecodeInterpreter::Get().m_Debugger.Render();

		StackFrame& stackFrame = m_StackFrames[m_StackFrameTop];

		if (m_ProgramCounter >= instructions.size()) break;

		Instruction& instruction = instructions[m_ProgramCounter++];

		switch (instruction.m_Type)
		{
		case Opcodes::no_op: break;
		case Opcodes::push_number:
		{
			if (!instruction.m_DiscardValue) {
				if (instruction.m_Arguments[0].GetType() == ValueTypes::Float)
					stackFrame.PushOperand(instruction.m_Arguments[0].GetFloat());

				if (instruction.m_Arguments[0].GetType() == ValueTypes::Integer)
					stackFrame.PushOperand(instruction.m_Arguments[0].GetInt());
			}
			
			break;
		}
		case Opcodes::push_stringconst:
		{
			HeapEntry& stringConstant = constants.m_StringConstants[instruction.m_Arguments[0].GetInt()];

			if (!instruction.m_DiscardValue)
				stackFrame.PushOperand(Value(stringConstant, ValueTypes::StringConstant));

			break;
		}
		case Opcodes::push_null:
		{
			abort();
			if (!instruction.m_DiscardValue)
				stackFrame.PushOperand(Value(0, ValueTypes::Void));

			break;
		}
		case Opcodes::push_functionpointer:
		{
			if (!instruction.m_DiscardValue)
				stackFrame.PushOperand(Value(instruction.m_Arguments[0].GetInt(), ValueTypes::Integer/*ValueTypes::FunctionPointer*/));

			break;
		}
		/*case Opcodes::array_create_empty:
		{
			HeapEntry& emptyArray = heap.CreateArray();

			if (!instruction.m_DiscardValue)
				stackFrame.PushOperand(Value(emptyArray));

			break;
		}*/
		//case Opcodes::array_create:
		//{
		//	HeapEntry& emptyArray = heap.CreateArray();

		//	int itemCount = instruction.m_Arguments[0].GetInt();

		//	ValueArray arrayItems;
		//	arrayItems.emplace_back(emptyArray);

		//	// Pull all the operands that should be added
		//	for (int i = 0; i < itemCount; i++)
		//	{
		//		arrayItems.push_back(stackFrame.PopOperand());
		//	}

		//	Functions::array_push(&arrayItems);

		//	if (!instruction.m_DiscardValue)
		//		stackFrame.PushOperand(Value(emptyArray));

		//	break;
		//}
		//case Opcodes::object_create_empty:
		//{
		//	stackFrame.PushOperand(Value(heap.CreateObject()));

		//	break;
		//}
		//case Opcodes::object_create:
		//{
		//	Value emptyObject = heap.CreateObject();

		//	ObjectInstance& object = *emptyObject.GetObjectInstance();

		//	int itemCount = instruction.m_Arguments[0].DataAsFloat();

		//	// Pull all the operands that should be added
		//	for (int i = 0; i < itemCount; i++)
		//	{
		//		Value value = stackFrame.PopOperand();
		//		std::string key = stackFrame.PopOperand().GetString();
		//		object[key] = value;
		//	}

		//	if (!instruction.m_DiscardValue)
		//		stackFrame.PushOperand(emptyObject);

		//	break;
		//}
		case Opcodes::pop:
		{
			stackFrame.PopOperand();

			break;
		}
		case Opcodes::store:
		{
			Value operand = stackFrame.PopOperand();

			ValueTypes variableType = (ValueTypes)(instruction.m_Arguments[1].GetInt());

			uint32_t index = instruction.m_Arguments[0].GetInt();

			if (instruction.m_ArgsCount >= 2) {
				stackFrame.StoreVariable(index, operand, variableType);
				
				if (instruction.m_ArgsCount == 4)
				{
					// TODO: fix
					stackFrame.m_VariablesList[index].m_Flag = Value::Flags::LocalVariable;
				}
			} 
			else
				stackFrame.StoreVariable(index, operand);

			break;
		}

		/*case Opcodes::store_property:
		{
			Value value = stackFrame.PopOperand();
			Value objectVal = stackFrame.PopOperand();

			if (objectVal.m_Type != ValueTypes::Object)
			{
				ThrowExceptionVoid("Only objects can have properties. The value you tried to assign to is a " + ValueTypeToString(objectVal.m_Type));
				break;
			}

			ObjectInstance& object = *objectVal.GetObjectInstance();

			std::string key = instruction.m_Arguments[0].StringData;

			object[key] = value;

			break;
		}*/
		case Opcodes::load:
		{
			uint32_t index = instruction.m_Arguments[0].GetInt();

			Value& variable = stackFrame.GetVariable(index);
			if (Exception()) return;

			stackFrame.PushOperand(variable);

			break;
		}
		//case Opcodes::load_property:
		//{
		//	ObjectInstance& object = *stackFrame.PopOperand().GetObjectInstance();
		//	std::string key = instruction.m_Arguments[0].StringData;

		//	// Property doesn't exist
		//	if (object.count(key) == 0)
		//		stackFrame.PushOperand(Value(Value::Empty));
		//	else
		//		stackFrame.PushOperand(object[key]);

		//	break;
		//}

		case Opcodes::eq:
		{
			Value value1 = stackFrame.PopOperand();
			Value value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(Value::CompareEquals(value1, value2));

			break;
		}
		case Opcodes::neq:
		{
			Value value1 = stackFrame.PopOperand();
			Value value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(Value::CompareNotEquals(value1, value2));

			break;
		}

		case Opcodes::cmpgt:
		{
			Value value1 = stackFrame.PopOperand();
			Value value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(Value::CompareGreaterThan(value1, value2));

			break;
		}
		case Opcodes::cmpge:
		{
			Value value1 = stackFrame.PopOperand();
			Value value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(Value::CompareGreaterThanEqual(value1, value2));

			break;
		}
		case Opcodes::cmplt:
		{
			Value value1 = stackFrame.PopOperand();
			Value value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(Value::CompareLessThan(value1, value2));

			break;
		}
		case Opcodes::cmple:
		{
			Value value1 = stackFrame.PopOperand();
			Value value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(Value::CompareLessThanEqual(value1, value2));

			break;
		}
		case Opcodes::logical_and:
		{
			Value value1 = stackFrame.PopOperand();
			Value value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(value1.IsTruthy() && value2.IsTruthy());

			break;
		}
		case Opcodes::logical_or:
		{
			Value value1 = stackFrame.PopOperand();
			Value value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(value1.IsTruthy() || value2.IsTruthy());

			break;
		}
		case Opcodes::logical_not:
		{
			Value value1 = stackFrame.PopOperand();

			stackFrame.PushOperand(!value1.IsTruthy());

			break;
		}

		case Opcodes::jmp:
		{
			m_ProgramCounter = instruction.m_Arguments[0].GetInt();

			break;
		}
		case Opcodes::jmp_if_true:
		{
			Value value = stackFrame.PopOperand();

			if (value.IsTruthy())
				m_ProgramCounter = instruction.m_Arguments[0].GetInt();

			break;
		}
		case Opcodes::jmp_if_false:
		{
			Value value = stackFrame.PopOperand();

			if (!value.IsTruthy())
				m_ProgramCounter = instruction.m_Arguments[0].GetInt();

			break;
		}

		case Opcodes::add:
		{
			Value value1 = stackFrame.PopOperand();
			Value value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(Value::Add(value1, value2));

			break;
		}
		case Opcodes::sub:
		{
			Value value1 = stackFrame.PopOperand();
			Value value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(Value::Subtract(value1, value2));

			break;
		}
		case Opcodes::sub_reverse:
		{
			Value value1 = stackFrame.PopOperand();
			Value value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(Value::Subtract(value2, value1));

			break;
		}
		case Opcodes::mul:
		{
			Value value1 = stackFrame.PopOperand();
			Value value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(Value::Multiply(value1, value2));

			break;
		}
		case Opcodes::div:
		{
			Value value1 = stackFrame.PopOperand();
			Value value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(Value::Divide(value1, value2));

			break;
		}
		case Opcodes::div_reverse:
		{
			Value value1 = stackFrame.PopOperand();
			Value value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(Value::Divide(value2, value1));

			break;
		}
		/*case Opcodes::pow:
		{
			Value value1 = stackFrame.PopOperand();
			Value value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(Value::Pow(value1, value2));

			break;
		}
		case Opcodes::pow_rev:
		{
			Value value1 = stackFrame.PopOperand();
			Value value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(Value::Pow(value2, value1));

			break;
		}
		case Opcodes::mod:
		{
			Value value1 = stackFrame.PopOperand();
			Value value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(Value::Mod(value1, value2));

			break;
		}
		case Opcodes::mod_rev:
		{
			Value value1 = stackFrame.PopOperand();
			Value value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(Value::Mod(value2, value1));

			break;
		}
		case Opcodes::xr:
		{
			Value value1 = stackFrame.PopOperand();
			Value value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(Value::Xor(value1, value2));

			break;
		}
		case Opcodes::xr_rev:
		{
			Value value1 = stackFrame.PopOperand();
			Value value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(Value::Xor(value2, value1));

			break;
		}*/
		case Opcodes::post_inc:
		{
			uint32_t index = instruction.m_Arguments[0].GetInt();

			Value& variable = stackFrame.GetVariable(index);

			if (!instruction.m_DiscardValue)
				stackFrame.PushOperand(variable);

			if (variable.GetType() == ValueTypes::Integer)
				variable.GetInt()++;
			else if(variable.GetType() == ValueTypes::Float)
				variable.GetFloat()++;

			break;
		}
		case Opcodes::post_dec:
		{
			uint32_t index = instruction.m_Arguments[0].GetInt();

			Value& variable = stackFrame.GetVariable(index);

			if (!instruction.m_DiscardValue)
				stackFrame.PushOperand(variable);

			if (variable.GetType() == ValueTypes::Integer)
				variable.GetInt()--;
			else if (variable.GetType() == ValueTypes::Float)
				variable.GetFloat()--;

			break;
		}

		case Opcodes::ret:
		{
			// If on global scope
			if (m_StackFrameTop == 0)
			{
				return;
			}

			m_ProgramCounter = stackFrame.m_ReturnAdress;

			StackFrame functionStack = PopFrame();

			// If to actually return anything
			if (functionStack.m_OperandStackTop != 0)
			{
				Value returnValue = functionStack.PopOperand();

				GetTopFrame().PushOperand(returnValue);
			}

			DeleteLocalVariables(GetTopFrame(), functionStack);

			break;
		}

		case Opcodes::ret_void:
		{
			m_ProgramCounter = stackFrame.m_ReturnAdress;

			StackFrame functionStack = PopFrame();

			GetTopFrame().PushOperand(Value(0, ValueTypes::Void));

			DeleteLocalVariables(GetTopFrame(), functionStack);

			break;
		}

		case Opcodes::skip_function:
		{
			m_ProgramCounter = (int)instruction.m_Arguments[0].GetInt();

			break;
		}

		case Opcodes::create_scope_frame:
		{
			PushFrame(this);
			StackFrame& newFrame = GetTopFrame();
			newFrame.m_ReturnAdress = stackFrame.m_ReturnAdress;

			// Copy the variables from the previous scope into this
			for (int i = 0; i < STACK_SIZE; i++)
			{
				newFrame.m_VariablesList[i] = stackFrame.m_VariablesList[i];

				// Set the initial variables so local variables can be compared, and discarded
				newFrame.m_InitialVariablesList[i] = newFrame.m_VariablesList[i];
			}

			break;
		}

		case Opcodes::create_function_frame:
		{
			PushFrame(this);
			StackFrame& newFrame = GetTopFrame();

			// Copy the variables from the global scope into this frame
			for (int i = 0; i < STACK_SIZE; i++)
			{
				newFrame.m_VariablesList[i] = m_StackFrames[0].m_VariablesList[i];

				// Set the initiali variables so local variables can be compared, and discarded
				newFrame.m_InitialVariablesList[i] = newFrame.m_VariablesList[i];
			}

			// Get the return adress and set it
			newFrame.m_ReturnAdress = stackFrame.PopOperand().GetInt();

			uint32_t argCount = stackFrame.PopOperand().GetInt();

			// Pull the arguments
			for (int i = 0; i < argCount; i++)
			{
				newFrame.PushOperand(stackFrame.PopOperand());
			}

			break;
		}

		case Opcodes::pop_scope_frame:
		{
			int frameDepth = instruction.m_Arguments[0].GetInt();

			assert(frameDepth <= m_StackFrameTop);

			// If popping a frame, but popping a frame that isn't on top (kinda). Then pop the skipped ones too
			while (frameDepth != m_StackFrameTop)
			{
				StackFrame scopeStack = PopFrame();

				DeleteLocalVariables(GetTopFrame(), scopeStack);
				ClearOperands(scopeStack);
			}

			// All the extra frames have been popped, now pop the intended frame
			StackFrame scopeStack = PopFrame();

			DeleteLocalVariables(GetTopFrame(), scopeStack);
			ClearOperands(scopeStack);

			break;
		}

		case Opcodes::call:
		{
			int argCount = instruction.m_Arguments[0].GetInt();

			Value functionLocation = stackFrame.PopOperand();

			if (functionLocation.GetType() == ValueTypes::Void)
			{
				std::string name = instruction.m_Arguments[1].GetString();
				return ThrowExceptionVoid("Function '" + name + "' is not defined");
			}

			//assert(functionLocation.m_Type == ValueTypes::FunctionPointer);

			// The function needs to know how many arguments to pull
			stackFrame.PushOperand(argCount);

			// Push the return adress onto the operand stack
			stackFrame.PushOperand(m_ProgramCounter);

			// Jump to the function
			m_ProgramCounter = functionLocation.GetInt();

			break;
		}

		case Opcodes::call_native:
		{
			std::string functionName = instruction.m_Arguments[0].GetString();
			uint32_t argCount = instruction.m_Arguments[1].GetInt();

			// Get the args
			ValueArray args;
			for (int i = 0; i < argCount; i++)
			{
				args.push_back(stackFrame.PopOperand());
			}

			CallableFunction function = BytecodeFunctions::GetFunctionByName(functionName);
			Value returnValue = function(&args);

			if (Exception())
				ThrowExceptionVoid(functionName + "(): " + m_Exception);

			if (!instruction.m_DiscardValue) 
			{
				assert(returnValue.GetType() != ValueTypes::Void);
				stackFrame.PushOperand(returnValue);
			}	

			break;
		}

		case Opcodes::stop: return;
		default: abort();
		}

		if (Exception()) return;
	}
}

void ExecutionContext::DeleteLocalVariables(StackFrame& topFrame, StackFrame& localFrame)
{
	// Check what variables have been created
	for (int i = 0; i < STACK_SIZE; i++)
	{
		// If a variable is new
		if (localFrame.m_VariablesList[i].GetType() != ValueTypes::Void && localFrame.m_InitialVariablesList[i].GetType() == ValueTypes::Void)
			localFrame.m_VariablesList[i].Delete();
		else {
			if (topFrame.m_VariablesList[i].m_Flag == Value::Flags::LocalVariable)
				topFrame.m_VariablesList[i] = localFrame.m_VariablesList[i];
		}
			
	}
}

void ExecutionContext::ClearOperands(StackFrame& frame)
{
	for (int i = 0; i < STACK_SIZE; i++)
	{
		frame.m_OperandStack[i].Delete();
	}
}

Value ExecutionContext::ThrowExceptionValue(std::string error)
{
	m_Exception = error;
	return Value();
}
void ExecutionContext::ThrowExceptionVoid(std::string error) { m_Exception = error; }
bool ExecutionContext::Exception() { return m_Exception != "" || BytecodeInterpreter::Get().ExceptionError != ""; }

ExecutionContext* BytecodeInterpreter::CreateContext()
{
	ExecutionContext* ctx = new ExecutionContext();
	ctx->m_Id = m_NextFreeContextId++;

	AddContext(ctx);
	return GetContext(ctx->m_Id);
}

void BytecodeInterpreter::AddContext(ExecutionContext* context)
{
	assert(context->m_Id >= 0);

	assert(m_Contexts.count(context->m_Id) == 0);

	m_Contexts[context->m_Id] = context;
}

void BytecodeInterpreter::RemoveContext(int id)
{
	assert(id >= 0);

	assert(m_Contexts.count(id) == 1);

	m_Contexts.erase(id);
}

ExecutionContext* BytecodeInterpreter::GetContext(int id)
{
	assert(id >= 0);

	assert(m_Contexts.count(id) == 1);

	return m_Contexts[id];
}