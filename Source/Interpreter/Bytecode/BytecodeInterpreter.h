#pragma once

#include "../Value.h"
#include "BytecodeCompiler.h"
#include "Debugger.h"
#include "ConstantsPool.h"

#include "Stack.h"

#include <tuple>

struct ASTNode;

namespace Ö::Bytecode {
	class ExecutionContext;

	class Frame
	{
	public:
		Frame();
		Frame(ExecutionContext* ctx) : m_Context(ctx) {};

		Value& GetVariable(uint32_t index);

		//void StoreVariable(uint32_t index, Value value, ValueTypes variableType);

		//Stack<Value>& Stack();

		~Frame() {};

	public:
		static const int InitialStackCount = 16;

		Stack<Value> m_Stack;
		EncodedInstructions* m_Instructions = nullptr;

	private:
		std::vector<Value> m_Variables;

		ExecutionContext* m_Context = nullptr;
	};

	class ExecutionContext
	{
	public:
		ExecutionContext();

		//void PushFrame(Frame frame);
		/*void PushFrame(ExecutionContext* ctx);
		Frame PopFrame();
		Frame& GetTopFrame();*/

		void Execute();

	public:
		static const int InitialFrameCount = 16;

		//Ö :: Bytecode :: CompiledCallable c;

		std::unordered_map<uint16_t, CompiledCallable> m_Functions;
		RuntimeConstantsPool m_ConstantsPool;

		int m_Id = 0;
		int m_ProgramCounter = 0;

		Stack<Frame> m_Frames = Stack<Frame>(InitialFrameCount);

		std::string m_Exception;
	};

	class BytecodeInterpreter
	{
	public:
		EXPORT static BytecodeInterpreter& Get();

		EXPORT Value CreateAndRunProgram(std::string fileContent, std::string& error, bool verbose = false);

		std::string InterpretBytecode();

		ExecutionContext* CreateContext();
		void AddContext(ExecutionContext* context);
		void RemoveContext(int id);
		ExecutionContext* GetContext(int id);

		inline Value ThrowExceptionValue(std::string error) { ExceptionError = error; return Value(ValueTypes::Void); };

	private:
		BytecodeInterpreter() {};

		int m_NextFreeContextId = 0;

	public:
		Instructions m_Instructions;
		std::unordered_map<int, ExecutionContext*> m_Contexts;

		Heap m_Heap;

		std::string ExceptionError;

		Compiler::BytecodeCompiler m_Compiler;
		Compiler::CompiledFile m_CompiledFile;

		/*Console m_Console;*/
		Debugger m_Debugger;
	};
}