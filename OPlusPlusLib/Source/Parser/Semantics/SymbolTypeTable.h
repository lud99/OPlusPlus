#pragma once

#include "TypeTable.h"
#include "SymbolTable.h"

namespace O
{
	struct SymbolTypeTable
	{
		SymbolTable symbols;
		TypeTable types;
	};
};