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

StackValue BytecodeInterpreter::CreateAndRunProgram(std::string filepath, std::string& error, bool verbose)
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
	m_Compiler.Compile(&tree, instructions);

	//m_ProgramCounter = m_Compiler.m_StartExecutionAt;
	m_ConstantsPool = m_Compiler.m_Constants;

	// Print
	std::cout << "\n";
	for (int i = 0; i < instructions.size(); i++)
	{
		std::cout << "(" << i << ") " << OpcodeToString(instructions[i].m_Type) << " ";

		for (int a = 0; a < INSTRUCTION_ARG_SIZE; a++)
		{
			if (instructions[i].m_Arguments[a].m_Type != ValueTypes::Void)
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
		return StackValue();
	}

	std::cout << sizeof(StackFrame) << ", " << sizeof(StackValue) << ", " << sizeof(ExecutionContext) << "\n";

	//m_Debugger = Debugger(instructions);
	//m_Debugger.m_Enabled = true;

	//m_Debugger.Render();

	auto start = std::chrono::high_resolution_clock::now();

	std::cout << "Console output:\n";

	m_Instructions = instructions;
	std::string exception = InterpretBytecode();

	if (exception != "")
		std::cout << "Bytecode execution error: (" << GetContext(0)->m_ProgramCounter - 1 << ") " << exception << "\n";

	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

	std::cout << "\nExecution took: " << (duration.count()) << "ms" << "\n";

	return StackValue();
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
	/*m_InitialVariablesList = new StackValue[STACK_SIZE];
	m_VariablesList = new StackValue[STACK_SIZE];
	m_OperandStack = new StackValue[STACK_SIZE];*/
}

StackValue& StackFrame::GetVariable(uint32_t index)
{
	// If out of range, throw exception
	assert(index >= 0 && index < STACK_SIZE);

	assert(m_VariablesList[index].m_Type != ValueTypes::Void);

	return m_VariablesList[index];
}

StackValue StackFrame::PopOperand()
{
	// If no operands to pop, throw exception
	assert(m_OperandStackTop >= 0 && m_OperandStackTop < STACK_SIZE);

	assert(m_OperandStackTop != 0);

	m_OperandStackTop--;
	StackValue top = m_OperandStack[m_OperandStackTop];

	m_OperandStack[m_OperandStackTop] = StackValue();

	return top;
}

void StackFrame::StoreVariable(uint32_t index, StackValue value)
{
	assert(index >= 0 && index < STACK_SIZE);

	ValueTypes variableType = m_VariablesList[index].m_Type;

	// Do type checking
	if (value.m_Type != ValueTypes::Void &&
		variableType != ValueTypes::Void &&
		variableType != ValueTypes::Void &&
		!StackValue::IsSamePrimitiveType(variableType, value.m_Type))

		return m_Context->ThrowExceptionVoid("Cannot store a value of type " + ValueTypeToString(value.m_Type) +
			" into a variable of type " + ValueTypeToString(variableType));

	m_VariablesList[index] = value;
}

void StackFrame::StoreVariable(uint32_t index, StackValue value, ValueTypes variableType)
{
	assert(index >= 0 && index < STACK_SIZE);

	if (value.m_Type != ValueTypes::Void && !StackValue::IsSamePrimitiveType(value.m_Type, variableType))
		return m_Context->ThrowExceptionVoid("Cannot assign a value of type " + ValueTypeToString(value.m_Type) +
			" to a variable of type " + ValueTypeToString(variableType));

	m_VariablesList[index] = value;
}

void StackFrame::PushOperand(StackValue value)
{
	assert(m_OperandStackTop >= 0 && m_OperandStackTop < STACK_SIZE - 1);

	m_OperandStack[m_OperandStackTop] = value;
	m_OperandStackTop++;
}

void StackFrame::PushOperand(double value)
{
	PushOperand(StackValue(value, ValueTypes::Float));
}
void StackFrame::PushOperand(int value)
{
	PushOperand(StackValue(value, ValueTypes::Integer));
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
		//m_Debugger.Render();

		StackFrame& stackFrame = m_StackFrames[m_StackFrameTop];

		if (m_ProgramCounter >= instructions.size()) break;

		Instruction& instruction = instructions[m_ProgramCounter++];

		switch (instruction.m_Type)
		{
		case Opcodes::no_op: break;
		case Opcodes::push_number:
		{
			if (!instruction.m_DiscardValue) {
				if (instruction.m_Arguments[0].m_Type == ValueTypes::Float)
					stackFrame.PushOperand(instruction.m_Arguments[0].GetFloat());

				if (instruction.m_Arguments[0].m_Type == ValueTypes::Integer)
					stackFrame.PushOperand(instruction.m_Arguments[0].GetInt());
			}
			
			break;
		}
		case Opcodes::push_stringconst:
		{
			HeapEntry& stringConstant = constants.m_StringConstants[instruction.m_Arguments[0].GetInt()];

			if (!instruction.m_DiscardValue)
				stackFrame.PushOperand(StackValue(stringConstant, ValueTypes::StringConstant));

			break;
		}
		case Opcodes::push_null:
		{
			abort();
			if (!instruction.m_DiscardValue)
				stackFrame.PushOperand(StackValue(0, ValueTypes::Void));

			break;
		}
		case Opcodes::push_functionpointer:
		{
			if (!instruction.m_DiscardValue)
				stackFrame.PushOperand(StackValue(instruction.m_Arguments[0].GetInt(), ValueTypes::Integer/*ValueTypes::FunctionPointer*/));

			break;
		}
		/*case Opcodes::array_create_empty:
		{
			HeapEntry& emptyArray = heap.CreateArray();

			if (!instruction.m_DiscardValue)
				stackFrame.PushOperand(StackValue(emptyArray));

			break;
		}*/
		//case Opcodes::array_create:
		//{
		//	HeapEntry& emptyArray = heap.CreateArray();

		//	int itemCount = instruction.m_Arguments[0].GetInt();

		//	StackValueArray arrayItems;
		//	arrayItems.emplace_back(emptyArray);

		//	// Pull all the operands that should be added
		//	for (int i = 0; i < itemCount; i++)
		//	{
		//		arrayItems.push_back(stackFrame.PopOperand());
		//	}

		//	Functions::array_push(&arrayItems);

		//	if (!instruction.m_DiscardValue)
		//		stackFrame.PushOperand(StackValue(emptyArray));

		//	break;
		//}
		//case Opcodes::object_create_empty:
		//{
		//	stackFrame.PushOperand(StackValue(heap.CreateObject()));

		//	break;
		//}
		//case Opcodes::object_create:
		//{
		//	StackValue emptyObject = heap.CreateObject();

		//	ObjectInstance& object = *emptyObject.GetObjectInstance();

		//	int itemCount = instruction.m_Arguments[0].DataAsFloat();

		//	// Pull all the operands that should be added
		//	for (int i = 0; i < itemCount; i++)
		//	{
		//		StackValue value = stackFrame.PopOperand();
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
			StackValue operand = stackFrame.PopOperand();

			ValueTypes variableType = (ValueTypes)(instruction.m_Arguments[1].GetInt());

			uint32_t index = instruction.m_Arguments[0].GetInt();

			if (instruction.m_ArgsCount >= 2)
				stackFrame.StoreVariable(index, operand, variableType);
			else
				stackFrame.StoreVariable(index, operand);

			break;
		}

		/*case Opcodes::store_property:
		{
			StackValue value = stackFrame.PopOperand();
			StackValue objectVal = stackFrame.PopOperand();

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

			StackValue& variable = stackFrame.GetVariable(index);
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
		//		stackFrame.PushOperand(StackValue(Value::Empty));
		//	else
		//		stackFrame.PushOperand(object[key]);

		//	break;
		//}

		case Opcodes::eq:
		{
			StackValue value1 = stackFrame.PopOperand();
			StackValue value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(StackValue::CompareEquals(value1, value2));

			break;
		}
		case Opcodes::neq:
		{
			StackValue value1 = stackFrame.PopOperand();
			StackValue value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(StackValue::CompareNotEquals(value1, value2));

			break;
		}

		case Opcodes::cmpgt:
		{
			StackValue value1 = stackFrame.PopOperand();
			StackValue value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(StackValue::CompareGreaterThan(value1, value2));

			break;
		}
		case Opcodes::cmpge:
		{
			StackValue value1 = stackFrame.PopOperand();
			StackValue value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(StackValue::CompareGreaterThanEqual(value1, value2));

			break;
		}
		case Opcodes::cmplt:
		{
			StackValue value1 = stackFrame.PopOperand();
			StackValue value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(StackValue::CompareLessThan(value1, value2));

			break;
		}
		case Opcodes::cmple:
		{
			StackValue value1 = stackFrame.PopOperand();
			StackValue value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(StackValue::CompareLessThanEqual(value1, value2));

			break;
		}
		case Opcodes::logical_and:
		{
			StackValue value1 = stackFrame.PopOperand();
			StackValue value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(value1.IsTruthy() && value2.IsTruthy());

			break;
		}
		case Opcodes::logical_or:
		{
			StackValue value1 = stackFrame.PopOperand();
			StackValue value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(value1.IsTruthy() || value2.IsTruthy());

			break;
		}
		case Opcodes::logical_not:
		{
			StackValue value1 = stackFrame.PopOperand();

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
			StackValue value = stackFrame.PopOperand();

			if (value.IsTruthy())
				m_ProgramCounter = instruction.m_Arguments[0].GetInt();

			break;
		}
		case Opcodes::jmp_if_false:
		{
			StackValue value = stackFrame.PopOperand();

			if (!value.IsTruthy())
				m_ProgramCounter = instruction.m_Arguments[0].GetInt();

			break;
		}

		case Opcodes::add:
		{
			StackValue value1 = stackFrame.PopOperand();
			StackValue value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(StackValue::Add(value1, value2));

			break;
		}
		case Opcodes::sub:
		{
			StackValue value1 = stackFrame.PopOperand();
			StackValue value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(StackValue::Subtract(value1, value2));

			break;
		}
		case Opcodes::sub_reverse:
		{
			StackValue value1 = stackFrame.PopOperand();
			StackValue value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(StackValue::Subtract(value2, value1));

			break;
		}
		case Opcodes::mul:
		{
			StackValue value1 = stackFrame.PopOperand();
			StackValue value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(StackValue::Multiply(value1, value2));

			break;
		}
		case Opcodes::div:
		{
			StackValue value1 = stackFrame.PopOperand();
			StackValue value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(StackValue::Divide(value1, value2));

			break;
		}
		case Opcodes::div_reverse:
		{
			StackValue value1 = stackFrame.PopOperand();
			StackValue value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(StackValue::Divide(value2, value1));

			break;
		}
		/*case Opcodes::pow:
		{
			StackValue value1 = stackFrame.PopOperand();
			StackValue value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(StackValue::Pow(value1, value2));

			break;
		}
		case Opcodes::pow_rev:
		{
			StackValue value1 = stackFrame.PopOperand();
			StackValue value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(StackValue::Pow(value2, value1));

			break;
		}
		case Opcodes::mod:
		{
			StackValue value1 = stackFrame.PopOperand();
			StackValue value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(StackValue::Mod(value1, value2));

			break;
		}
		case Opcodes::mod_rev:
		{
			StackValue value1 = stackFrame.PopOperand();
			StackValue value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(StackValue::Mod(value2, value1));

			break;
		}
		case Opcodes::xr:
		{
			StackValue value1 = stackFrame.PopOperand();
			StackValue value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(StackValue::Xor(value1, value2));

			break;
		}
		case Opcodes::xr_rev:
		{
			StackValue value1 = stackFrame.PopOperand();
			StackValue value2 = stackFrame.PopOperand();

			stackFrame.PushOperand(StackValue::Xor(value2, value1));

			break;
		}*/
		case Opcodes::post_inc:
		{
			uint32_t index = instruction.m_Arguments[0].GetInt();

			StackValue& variable = stackFrame.GetVariable(index);

			if (!instruction.m_DiscardValue)
				stackFrame.PushOperand(variable);

			if (variable.m_Type == ValueTypes::Integer)
				variable.m_ValueInt++;
			else if(variable.m_Type == ValueTypes::Float)
				variable.m_ValueFloat++;

			break;
		}
		case Opcodes::post_dec:
		{
			uint32_t index = instruction.m_Arguments[0].GetInt();

			StackValue& variable = stackFrame.GetVariable(index);

			if (!instruction.m_DiscardValue)
				stackFrame.PushOperand(variable);

			if (variable.m_Type == ValueTypes::Integer)
				variable.m_ValueInt--;
			else if (variable.m_Type == ValueTypes::Float)
				variable.m_ValueFloat--;

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
				StackValue returnValue = functionStack.PopOperand();

				GetTopFrame().PushOperand(returnValue);
			}

			DeleteLocalVariables(GetTopFrame(), functionStack);

			break;
		}

		case Opcodes::ret_void:
		{
			m_ProgramCounter = stackFrame.m_ReturnAdress;

			StackFrame functionStack = PopFrame();

			GetTopFrame().PushOperand(StackValue(0, ValueTypes::Void));

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

			// Copy the variables from the previous scope into this
			for (int i = 0; i < STACK_SIZE; i++)
			{
				newFrame.m_VariablesList[i] = stackFrame.m_VariablesList[i];

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

			StackValue functionLocation = stackFrame.PopOperand();

			if (functionLocation.m_Type == ValueTypes::Void)
			{
				std::string name = instruction.m_Arguments[1]._m_String;
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
			std::string functionName = instruction.m_Arguments[0]._m_String;
			uint32_t argCount = instruction.m_Arguments[1].GetInt();

			// Get the args
			StackValueArray args;
			for (int i = 0; i < argCount; i++)
			{
				args.push_back(stackFrame.PopOperand());
			}

			CallableFunction function = BytecodeFunctions::GetFunctionByName(functionName);
			StackValue returnValue = function(&args);

			if (Exception())
				ThrowExceptionVoid(functionName + "(): " + m_Exception);

			if (!instruction.m_DiscardValue) 
			{
				assert(returnValue.m_Type != ValueTypes::Void);
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
		if (localFrame.m_VariablesList[i].m_Type != ValueTypes::Void && localFrame.m_InitialVariablesList[i].m_Type == ValueTypes::Void)
			localFrame.m_VariablesList[i].Delete();
		else
			topFrame.m_VariablesList[i] = localFrame.m_VariablesList[i];
	}
}

void ExecutionContext::ClearOperands(StackFrame& frame)
{
	for (int i = 0; i < STACK_SIZE; i++)
	{
		frame.m_OperandStack[i].Delete();
	}
}

StackValue ExecutionContext::ThrowExceptionValue(std::string error)
{
	m_Exception = error;
	return StackValue();
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