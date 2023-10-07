#pragma once

#include <string>
#include <unordered_map>

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
		std::unordered_map<std::string, uint16_t> m_Classes;

		std::unordered_map<std::string, BuiltInFunctions::Prototype> m_BuiltInFunctions;

		uint16_t m_CurrentFreeSlot = 0;
	};

	class _CompilerContext
	{
	public:
		struct Variable
		{
			std::string m_Name = "";
			uint16_t m_Index = 0;

			bool m_IsGlobal = false;

			ValueType m_Type = ValueTypes::Void;

			Variable(uint32_t index = 0, std::string name = "", ValueType type = ValueTypes::Void, bool isGlobal = false) : m_Name(name), m_Index(index), m_Type(type), m_IsGlobal(isGlobal) {};
		};

		struct Function
		{
			std::string m_Name = "";
			ValueTypes m_ReturnType = ValueTypes::Void;

			uint16_t m_Index = 0;

			// TODO: Parameters

			Instructions m_Body;
		};

		struct Class
		{
			std::string m_Name = "";
			uint16_t m_Index = 0;

			uint16_t m_CurrentFreeMemberIndex = 0;

			Instructions m_InternalConstructor;
			std::unordered_map<std::string, Function> m_Methods;

			std::unordered_map<std::string, Variable> m_MemberVariables;
		};

		struct LoopInfo
		{
			bool m_InLoop = false;
			int m_Reset = -1;
			int m_End = -1;
			int m_BodyDepth = -1;
		};

	public:
		bool CreateVariableIndex(std::string& variableName, ValueTypes type, int& index); // Returns false if the variable exists, true if it was created
		bool CreateVariableIndex(Variable& variable); // Returns false if the variable exists, true if it was created. Sets the index on the variable object
		int CreateVariableIndex(std::string& variableName, ValueTypes type);

		Variable CreateMemberVariable(Class& cls, Variable& variable);

		Variable GetVariable(const std::string& variableName);
		bool HasVariable(const std::string& variableName);

		Function& CreateFunction(const std::string& functionName, ValueTypes returnType);
		Function& GetFunction(const std::string& functionName);
		bool HasFunction(const std::string& functionName);

		Class& CreateClass(const std::string& className);
		Class& GetClass(const std::string& className);
		bool HasClass(const std::string& className);

		Function& CreateMethod(Class& cls, const std::string& methodName, ValueTypes returnType);
		Function& GetMethod(Class& cls, const std::string& methodName);
		bool HasMethod(Class& cls, const std::string& methodName);

	public:
		std::unordered_map<std::string, Variable> m_Variables;
		std::unordered_map<std::string, Function> m_Functions;
		std::unordered_map<std::string, Class> m_Classes;
		//std::unordered_map<std::string, uint32_t> m_IndiciesForStringConstants;



		uint32_t m_NextFreeVariableIndex = 0;
		//uint32_t m_NextFreeFunctionIndex = 0;
		//uint32_t m_NextFreeStringConstantIndex = 0;

		//bool m_IsModule = false;
		//bool m_ShouldExportVariable = false;
		//int m_ModuleIndex = -1;
		//bool m_IsThreadedFunction = false;

		CompiledFile* m_CompiledFile = nullptr;

		LoopInfo m_LoopInfo;
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
		Opcodes ResolveCorrectStoreOpcode(ValueType type);
		Opcodes ResolveCorrectLoadOpcode(ValueType type);
		ValueType GetValueTypeOfNode(ASTNode* node);

		void CompileClass(ASTNode* node, SymbolTable::ClassSymbol* parentClass = nullptr);
		SymbolTable::FunctionSymbol* CompileClassMethod(ASTNode* node, SymbolTable::ClassSymbol& classSymbol, Instructions& instructions);
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

		/*std::unordered_map<SymbolTable::Symbol, Variable> m_Variables;
		std::unordered_map<SymbolTable::Symbol, Function> m_Functions;
		std::unordered_map<SymbolTable::Symbol, Class> m_Classes;*/

		uint32_t m_CurrentScope = 0;
	};
}