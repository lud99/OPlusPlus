#include "Debugger.h"

#include "BytecodeInterpreter.h"
#include "../../Utils.hpp"

#include <iostream>

std::vector<std::string> split(std::string s, std::string delimiter) {
	size_t pos_start = 0, pos_end, delim_len = delimiter.length();
	std::string token;
	std::vector<std::string> res;

	while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
		token = s.substr(pos_start, pos_end - pos_start);
		pos_start = pos_end + delim_len;
		res.push_back(token);
	}

	res.push_back(s.substr(pos_start));
	return res;
}

namespace Ö::Bytecode {
	Debugger::Debugger()
	{
	}

	Debugger::Debugger(Instructions instructions)
	{
		m_Instructions = instructions;
	}

	void Debugger::BreakAtInstruction(int instruction)
	{
		m_Breakpoint = instruction;
	}

	void Debugger::StepForward()
	{
		BytecodeInterpreter::Get().GetContext(0)->m_ProgramCounter++;
	}

	void Debugger::Render()
	{
		if (!m_Enabled) return;

		ExecutionContext* ctx = BytecodeInterpreter::Get().GetContext(0);

		if (ctx->m_ProgramCounter == m_Breakpoint)
		{
			m_ContinueToBreakpoint = false;
		}
		else
			if (m_ContinueToBreakpoint) return;

		// Print
		std::cout << "\n";
		for (int i = 0; i < m_Instructions.size(); i++)
		{
			if (i == ctx->m_ProgramCounter)
				std::cout << "HERE ---> ";

			/*std::cout << "(" << i << ") " << OpcodeToString(m_Instructions[i].m_Type) << " ";

			for (int a = 0; a < InstructionArgSize; a++)
			{
				if (m_Instructions[i].m_Arguments[a].GetType() != ValueTypes::Void)
				{
					if (a > 0 && a < InstructionArgSize - 1) std::cout << ", ";
					std::cout << m_Instructions[i].m_Arguments[a].ToString();
				}
			}*/

			std::cout << "\n";
		}

		std::cout << "\nOperand stack	   Variables stack\n";
		for (int i = 0; i < 8; i++)
		{
			//const Value& stack = ctx->GetTopFrame().m_Stack.GetData()[i];
			//Value& variable = ctx->GetTopFrame().m_Variables[i];

			//std::cout << "(" << ValueTypeToString(stack.GetType()) << ") ";
			////if (stack.IsObject())
			//	//std::cout << stack.m_HeapEntryPointer << ", ";

			//std::cout << stack.ToFormattedString();
			//std::cout << "               ";

			//std::cout << "(" << ValueTypeToString(variable.GetType()) << ") ";
			////if (variable.IsObject())
			//	//std::cout << variable.m_HeapEntryPointer << ", ";
			//std::cout << variable.ToFormattedString() << "\n";
		}
		std::cout << "\nCurrent instruction: " << ctx->m_ProgramCounter << "\n";
		std::cout << "\nBreakpoint: " << m_Breakpoint << "\n\n";

		ReadCommands();
	}

	void Debugger::ReadCommands()
	{
		ExecutionContext* ctx = BytecodeInterpreter::Get().GetContext(0);

		while (true)
		{
			std::cout << "Debugger: ";
			std::string in;
			std::getline(std::cin >> std::ws, in);

			std::vector<std::string> splitted = split(in, " ");

			std::string cmd = splitted[0];
			std::vector<std::string> args = SliceVector(splitted, 1);

			if (cmd == "br")
			{
				int instruction = std::stoi(args[0]);

				BreakAtInstruction(instruction);
			}
			else if (cmd == "s")
			{
				return;
			}
			else if (cmd == "r")
			{
				ctx->m_ProgramCounter = 0;

				return;
			}
			else if (cmd == "c")
			{
				m_ContinueToBreakpoint = true;

				return;
			}
			else
				return;
		}
	}

	Debugger::~Debugger()
	{
	}
}