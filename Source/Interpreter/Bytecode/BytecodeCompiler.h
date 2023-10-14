#pragma once

#include <string>
#include <unordered_map>
#include <optional>

#include "../../Lexer.h"
#include "../../Parser.h"
#include "../Value.h"

// https://dzone.com/articles/introduction-to-java-bytecode

#include "ClassInstance.h"
#include "ConstantsPool.h"
#include "Instruction.h"
#include "SymbolTable.h"

namespace Ö::Bytecode::Compiler {

	class CompiledFile
	{
	public:
		RuntimeConstantsPool m_ConstantsPool;
		EncodedInstructions m_EncodedTopLevelInstructions;
		Instructions m_TopLevelInstructions;

		std::unordered_map<uint16_t, CompiledCallable> m_Functions;
		std::unordered_map<uint16_t, ClassInstance> m_Classes;
		//std::unordered_map<uint16_t, BuiltInFunction*> m_BuiltInFunctionsTable;
	};

	class ContextConstantsPool
	{
	public:
		//bool HasConstant(ConstantType typeEntry, uint16_t index);
		ContextConstantsPool();

		uint16_t AddAndGetIntegerIndex(int32_t value);
		uint16_t AddAndGetFloatIndex(float value);
		uint16_t AddAndGetStringIndex(std::string value);
		uint16_t AddAndGetFunctionReferenceIndex(std::string functionName);
		uint16_t AddAndGetMethodReferenceIndex(std::string functionName);
		uint16_t AddAndGetClassIndex(std::string className);
		uint16_t AddAndGetBuiltInFunction(BuiltInFunctions::Prototype functionPrototype);
		//uint16_t AddOrGetClassReferenceIndex(std::string className);

		//ConstantType GetTypeOfConstantByIndex(uint16_t index);

	public:
		std::unordered_map<int32_t, uint16_t> m_Integers;
		std::unordered_map<float, uint16_t> m_Floats;
		// TODO: double std::unordered_map<int, double> m_Doubles;
		// TODO: Long
		std::unordered_map<std::string, uint16_t> m_Strings; // TODO: use char* instead for saving memory?

		std::unordered_map<std::string, uint16_t> m_Functions;
		std::unordered_map<std::string, uint16_t> m_Methods; // TODO: store type information?
		std::unordered_map<std::string, uint16_t> m_Classes;

		std::unordered_map<std::string, BuiltInFunctions::Prototype> m_BuiltInFunctions;

		uint16_t m_CurrentFreeSlot = 0;
	};

	class BytecodeCompiler
	{
	public:
		BytecodeCompiler();

		CompiledFile CompileASTTree(ASTNode* root, TypeTable typeTable);

		bool HasError() { return m_Error != ""; };
		const std::string& GetError() { return m_Error; };

		static void PrintInstructions(Instructions& instructions, std::string indent = "");
		static void PrintInstructions(EncodedInstructions& instructions);

		~BytecodeCompiler() {};

	private:
		void Compile(ASTNode* node, Instructions& instructions, bool canCreateScope = true, bool canMakeVariablesLocal = true);

		void OptimzeInstructions(Instructions& instructions);
		
		void EncodeCompiledInstructions();
		EncodedInstructions EncodeInstructions(Instructions& instructions);

		Opcodes ResolveCorrectMathOpcode(ASTNode* n, bool reverse = false);
		Opcodes ResolveCorrectStoreOpcode(ValueType type, bool isClassMember = false);
		Opcodes ResolveCorrectLoadOpcode(ValueType type);
		ValueType GetValueTypeOfNode(ASTNode* node);

		std::optional<SymbolTable::VariableSymbol*> CreateSymbolForVariableDeclaration(ASTNode* node, ASTNode* parent, bool& isClassMemberVariable);
		std::optional<SymbolTable::Symbol*> CompilePropertyAccess(ASTNode* node, Instructions& instructions);
		std::optional<SymbolTable::ClassSymbol*> ResolveScopeResolution(ASTNode* node);
		std::optional<TypeTableEntry*> ResolveVariableType(ASTNode* node);


		std::optional<CompiledCallable> CompileCallable(ASTNode* node, SymbolTable::FunctionSymbol& symbol);
		void CompileCallableCall(ASTNode* node, Instructions& instructions, SymbolTable& symbolTableContext);

		void CompileClass(ASTNode* node, SymbolTable::ClassSymbol* parentClass = nullptr);
		std::optional<CompiledCallable> CompileClassMethod(ASTNode* node, SymbolTable::ClassSymbol& classSymbol);

		void CompileFunction(ASTNode* node);

		void CompileAssignment(ASTNode* node, Instructions& instructions);
		void CompileMathOperation(ASTNode* node, Instructions& instructions);
		void CompileIntLiteral(int64_t value, Instructions& instructions);
		void CompileDoubleLiteral(float value, Instructions& instructions);
		void CompileStringLiteral(std::string value, Instructions& instructions);

		Instruction LoadFromConstantsPool(uint16_t index);

		bool IsFunctionNameValid(const std::string& name);

		void MakeError(std::string error);

	public:
		std::string m_Error;

		uint32_t m_StartExecutionAt = 0;

		CompiledFile m_CompiledFile;

		//CompilerContext m_Context;
		SymbolTable m_SymbolTable;
		TypeTable m_TypeTable;
		ContextConstantsPool m_ConstantsPool;

		SymbolTable::ClassSymbol* m_CurrentParsingClass = nullptr;
		SymbolTable::FunctionSymbol* m_CurrentParsingCallable = nullptr;

		/*std::unordered_map<SymbolTable::Symbol, Variable> m_Variables;
		std::unordered_map<SymbolTable::Symbol, Function> m_Functions;
		std::unordered_map<SymbolTable::Symbol, Class> m_Classes;*/

		uint32_t m_CurrentScope = 0;
	};
}