#pragma once

#include "BytecodeInterpreter.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <assert.h>
#include <thread>

#include "../../Utils.hpp"
#include "../Functions.h"
#include "../../Parser.h"
#include "ClassInstance.h"

namespace Ö::Bytecode {

	BytecodeInterpreter& BytecodeInterpreter::Get()
	{
		static BytecodeInterpreter instance;
		return instance;
	}

	Value BytecodeInterpreter::CreateAndRunProgram(std::string fileContent, std::string& error, bool verbose)
	{
		Lexer lexer;
		error = lexer.CreateTokens(fileContent);
		if (error != "")
		{
			std::cout << error << "\n\n";
			return Value();
		}

		if (verbose)
		{
			for (auto& token : lexer.GetTokens())
				std::cout << token.ToString() << ": " << token.m_Value << " [" << token.m_Depth << "]\n";
			std::cout << "\n";
		}

		Parser parser;

		ASTNode tree = parser.CreateRootNode();

		parser.CreateAST(lexer.GetTokens(), &tree);

		if (parser.HasError())
		{
			std::cout << "AST Error: " << parser.GetError() << "\n";
			return Value();
		}
			
		if (verbose)
			parser.PrintASTTree(&tree, 0);

		m_CompiledFile = m_Compiler.CompileASTTree(&tree, parser.GetGeneratedTypeTable());
		if (m_Compiler.m_Error != "")
		{
			std::cout << "\nBytecode compilation error: " << m_Compiler.m_Error << "\n\n";
			return Value();
		}
			

		

		//m_ProgramCounter = m_Compiler.m_StartExecutionAt;
		//m_ConstantsPool = m_CompiledFile.m_ConstantsPool;

		// Print
		if (verbose)
		{
			Compiler::CompiledFile& file = m_CompiledFile;
			// Constants pool
			std::cout << "Constants pool:\n";
			for (auto& entry : file.m_ConstantsPool.m_Integers)
			{
				std::cout << "#" << entry.first << " = Integer: " << entry.second << "\n";
			}
			for (auto& entry : file.m_ConstantsPool.m_Floats)
			{
				std::cout << "#" << entry.first << " = Float: " << entry.second << "\n";
			}
			for (auto& entry : file.m_ConstantsPool.m_Strings)
			{
				std::cout << "#" << entry.first << " = String: " << entry.second << "\n";
			}
			for (auto& entry : file.m_ConstantsPool.m_FunctionReferences)
			{
				std::cout << "#" << entry.first << " = FunctionReference: " << entry.second << "\n";
			}
			for (auto& entry : file.m_ConstantsPool.m_BuiltInFunctions)
			{
				std::cout << "#" << entry.first << " = BuiltInFunctionReference: " << entry.second.name << "\n";
			}
			for (auto& entry : file.m_ConstantsPool.m_MethodReferences)
			{
				std::cout << "#" << entry.first << " = MethodReference: " << entry.second << "\n";
			}
			for (auto& entry : file.m_ConstantsPool.m_ClassReferences)
			{
				std::cout << "#" << entry.first << " = ClassReference: " << entry.second << "\n";
			}
			std::cout << "\n";	
			Sttring s;

			std::cout << "Type Table:\n";
			for (auto& entry : m_Compiler.m_TypeTable.AllTypes())
			{
				auto PrintType = [](TypeTableEntry entry) -> void {
					std::cout << "#" << entry.id << ": " << entry.name << ", "
						<< TypeTableTypeToString(entry.type);

					if (entry.redirect)
						std::cout << " and is typedef";
					std::cout << "\n";
				};

				PrintType(entry.second);
			}
			std::cout << "\n";

			std::cout << "Top-level Symbol Table:\n";
			for (auto& [_, symbols] : m_Compiler.m_SymbolTable.GetSymbols())
			{
				for (auto symbol : symbols)
				{
					std::cout << symbol->m_StorableValueType->name << " " <<
						symbol->m_Name << ", " << SymbolTypeToString(symbol->m_SymbolType) << "\n";
				}
			}
			std::cout << "\n";

			std::cout << "Classes:\n";
			for (auto& entry : file.m_Classes)
			{
				std::cout << "#" << entry.first << " = " << entry.second.m_Name << ":\n";

				std::cout << "  Class Member Variables:\n";
				for (auto& memberEntry : entry.second.m_MemberVariables)
				{
					auto& typeEntry = m_Compiler.m_TypeTable.GetEntryFromId(memberEntry.second.type);
					std::cout << "  #" << memberEntry.first << ": " << typeEntry.name << "(" << typeEntry.id << ")" << "\n";
				}
				std::cout << "\n";

				std::cout << "  Class Internal Constructor:\n";
				Compiler::BytecodeCompiler::PrintInstructions(entry.second.m_InternalConstructor, "  ");

				std::cout << "  Class Methods:\n";
				for (auto& methodEntry : entry.second.m_Methods)
				{
					auto& method = methodEntry.second;
					std::string returnType = m_Compiler.m_TypeTable.GetEntryFromId(method.returnType).name;

					std::cout << "  #" << methodEntry.first << " (" << returnType << ") (";

					for (int i = 0; i < method.parameters.size(); i++)
					{
						std::cout << m_Compiler.m_TypeTable.GetEntryFromId(method.parameters[i]).name;
						if (i < method.parameters.size() - 1)
							std::cout << ", ";
					}
					std::cout << "):\n";

					Compiler::BytecodeCompiler::PrintInstructions(method.body, "  ");
				}
				std::cout << "\n";
			}
			std::cout << "\n";

			std::cout << "Functions:\n";
			for (auto& entry : file.m_Functions)
			{
				auto& function = entry.second;
				std::string returnType = m_Compiler.m_TypeTable.GetEntryFromId(function.returnType).name;

				std::cout << "  #" << entry.first << " " << returnType << " (";
				
				for (int i = 0; i < function.parameters.size(); i++)
				{
					std::cout << m_Compiler.m_TypeTable.GetEntryFromId(function.parameters[i]).name;
					if (i < function.parameters.size() - 1)
						std::cout << ", ";
				}
				
				std::cout << "):\n";
				Compiler::BytecodeCompiler::PrintInstructions(function.body, "  ");
			}
			std::cout << "\n";

			std::cout << "Main Body:\n";
			Compiler::BytecodeCompiler::PrintInstructions(file.m_TopLevelInstructions);

			/*std::cout << "Encoded body\n";
			for (uint8_t byte : file.m_EncodedTopLevelInstructions)
			{
				printf("%02X ", byte);
			}*/
		}
		return Value();
		//std::cout << "Program counter: " << m_ProgramCounter << "\n\n";

		//std::cout << sizeof(Frame) << ", " << sizeof(Value) << ", " << sizeof(ExecutionContext) << "\n";

		/*m_Debugger = Debugger(instructions);
		m_Debugger.m_Enabled = false;*/

		auto start = std::chrono::high_resolution_clock::now();

		if (verbose) std::cout << "Console output:\n";

		//m_Instructions = instructions;
		InterpretBytecode();

		if (ExceptionError != "")
			std::cout << "Bytecode execution error: (" << GetContext(0)->m_ProgramCounter << ") " << ExceptionError << "\n";

		auto stop = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

		if (verbose) std::cout << "\nExecution took: " << (duration.count()) << "ms" << "\n";

		return Value();
	}

	std::string BytecodeInterpreter::InterpretBytecode()
	{
		// Main thread
		ExecutionContext* ctx = CreateContext();
		ctx->m_ConstantsPool = m_CompiledFile.m_ConstantsPool;
		ctx->m_Functions = m_CompiledFile.m_Functions;
		//ctx->m_Instructions = m_CompiledFile.m_EncodedTopLevelInstructions;
		
		// Because the entry point is not in a function, a frame has to be created
		Frame frame(ctx);
		//frame.m_Instructions = &m_CompiledFile.m_EncodedTopLevelInstructions;
		ctx->m_Frames.Push(frame);

		ctx->Execute();

		//if (ctx->Exception())
			//return ctx->m_Exception;

		return "";
	}

	Frame::Frame()
	{
		m_Stack = Stack<Value>(InitialStackCount);
	}

	Value& Frame::GetVariable(uint32_t index)
	{
		// If out of range, throw exception
		assert(index >= 0 && index < STACK_SIZE);

		assert(m_Variables[index].GetType() != ValueTypes::Void);

		return m_Variables[index];
	}

	ExecutionContext::ExecutionContext()
	{
	}

	void ExecutionContext::Execute()
	{
		//using namespace Bytecode;

		
		//CompileTimeConstantsPool& constants = BytecodeInterpreter::Get().m_ConstantsPool;
		Heap& heap = BytecodeInterpreter::Get().m_Heap;

		while (true)
		{
			//BytecodeInterpreter::Get().m_Debugger.Render();

			Frame& stackFrame = m_Frames.GetTop();
			EncodedInstructions& instructions = *stackFrame.m_Instructions;

			if (m_ProgramCounter >= instructions.size()) break;

			Opcodes opcode = (Opcodes)instructions[m_ProgramCounter];

			switch (opcode)
			{
			case Opcodes::no_op: break;
			case Opcodes::push_ibyte:
			{
				int8_t byte = instructions[m_ProgramCounter + 1];

				stackFrame.m_Stack.Push(Value((int32_t)byte, ValueTypes::Integer));
				m_ProgramCounter += 2;
			
				break;
			}
			case Opcodes::push_ishort:
			{
				int8_t highByte = instructions[m_ProgramCounter + 1];
				int8_t lowByte = instructions[m_ProgramCounter + 2];

				stackFrame.m_Stack.Push(Value(int32_t((highByte << 8) | lowByte), ValueTypes::Integer));

				m_ProgramCounter += 3;

				break;
			}

			case Opcodes::load_constant:
			{
				int8_t index = instructions[m_ProgramCounter + 1];
				ConstantPoolType type = m_ConstantsPool.GetTypeOfConstant(index);
				if (type == ConstantPoolType::Integer)
					stackFrame.m_Stack.Push({ m_ConstantsPool.GetInteger(index), ValueTypes::Integer });
				else if (type == ConstantPoolType::Float)
					stackFrame.m_Stack.Push({ m_ConstantsPool.GetFloat(index), ValueTypes::Float });
				else if (type == ConstantPoolType::String)
				{

					// TODO: Add string allocation
					//stackFrame.m_Stack.Push({ (int32_t)m_ConstantsPool.GetInteger(index), ValueTypes::Integer });
				}
					
				m_ProgramCounter += 2;

				break;
			}

			case Opcodes::ret:
			{
				Value returnValue = stackFrame.m_Stack.Pop();
				Value returnAdress = stackFrame.m_Stack.Pop();

				m_Frames.Pop();
				m_Frames.GetTop().m_Stack.Push(returnValue);

				m_ProgramCounter = returnAdress.GetInt();

				break;
			}

			case Opcodes::call:
			{
				int8_t location = instructions[m_ProgramCounter + 1];

				Frame functionFrame(this);
				//functionFrame.m_Instructions = &m_Functions[location].encodedBody;

				m_ProgramCounter += 2;

				functionFrame.m_Stack.Push(Value(m_ProgramCounter, ValueTypes::Integer));
				m_Frames.Push(functionFrame);

				// TODO: should store the function arguments in the variable stack

				m_ProgramCounter = 0;

				break;
			}

			case Opcodes::call_native:
			{
				std::vector<Value> args;
				while (stackFrame.m_Stack.HasMore())
				{
					args.push_back(stackFrame.m_Stack.Pop());
				}

				BuiltInFunctions::_print(args);

				m_ProgramCounter += 1;

				break;
			}

			case Opcodes::stop: return;
			default: abort();
			}

			//if (Exception()) return;
		}
	}

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
}