#pragma once

#include <vector>

#include "BytecodeCompiler.h"

namespace Bytecode {
	class Debugger
	{
	public:
		Debugger();
		Debugger(Instructions instructions);

		void BreakAtInstruction(int instruction);
		void StepForward();

		void Render();

		void ReadCommands();

		~Debugger();

	public:
		Instructions m_Instructions;

		int m_Breakpoint = -1;
		bool m_ContinueToBreakpoint = false;

		bool m_Enabled = false;
	};
}