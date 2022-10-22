#pragma once

#include "StackValue.h"
#include "BytecodeCompiler.h"
#include "Debugger.h"

#include <tuple>

struct ASTNode;
class ExecutionContext;

class StackFrame
{
public:
	StackFrame();
	StackFrame(ExecutionContext* ctx);

	void Alloc();

	StackValue& GetVariable(uint32_t index);
	StackValue PopOperand();

	void StoreVariable(uint32_t index, StackValue value);
	void StoreVariable(uint32_t index, StackValue value, ValueTypes variableType);
	void PushOperand(StackValue value);
	void PushOperand(double value);
	void PushOperand(int value);

	void Delete();
	~StackFrame();

public:
	StackValue m_VariablesList[STACK_SIZE];// = nullptr;
	StackValue m_OperandStack[STACK_SIZE];// = nullptr;

	StackValue m_InitialVariablesList[STACK_SIZE];// = nullptr;

	int m_ReturnAdress = 0;

	uint32_t m_OperandStackTop = 0;

	ExecutionContext* m_Context = nullptr;
};

class ExecutionContext
{
public:
	ExecutionContext();

	//void PushFrame(StackFrame frame);
	void PushFrame(ExecutionContext* ctx);
	StackFrame PopFrame();
	StackFrame& GetTopFrame();

	void Execute();

	void DeleteLocalVariables(StackFrame& topFrame, StackFrame& localFrame);
	void ClearOperands(StackFrame& frame);

	StackValue ThrowExceptionValue(std::string error);
	void ThrowExceptionVoid(std::string error);
	bool Exception();

public:
	Instructions m_Instructions;

	int m_ProgramCounter = 0;

	std::vector<StackFrame> m_StackFrames;

	//StackFrame m_StackFrames[STACKFRAME_SIZE];

	uint32_t m_StackFrameTop = 0;

	int m_Id = -1;

	std::string m_Exception;
};

class BytecodeInterpreter
{
public:
	static BytecodeInterpreter& Get();

	StackValue CreateAndRunProgram(std::string source, std::string& error, bool verbose = false);

	std::string InterpretBytecode();

	ExecutionContext* CreateContext();
	void AddContext(ExecutionContext* context);
	void RemoveContext(int id);
	ExecutionContext* GetContext(int id);

	inline StackValue ThrowExceptionValue(std::string error) { ExceptionError = error; return StackValue(ValueTypes::Void); };

private:
	BytecodeInterpreter() {};

	int m_NextFreeContextId = 0;

public:
	Instructions m_Instructions;
	std::unordered_map<int, ExecutionContext*> m_Contexts;

	Heap m_Heap;
	ConstantsPool m_ConstantsPool;

	std::string ExceptionError;

	BytecodeCompiler m_Compiler;

	/*Console m_Console;*/
	Debugger m_Debugger;
};