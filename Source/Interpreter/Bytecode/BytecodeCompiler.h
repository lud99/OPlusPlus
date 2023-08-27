#pragma once

#include <string>
#include <unordered_map>

#include "../../Lexer.h"
#include "../../Parser.h"
#include "../Value.h"

// https://dzone.com/articles/introduction-to-java-bytecode

#include "ClassInstance.h"
#include "Instruction.h"

namespace Ö::Bytecode {

	enum class ConstantPoolType
	{
		Integer,
		Float,
		Double,
		Long,
		String,
		FunctionReference,
		MethodReference,
		ClassReference
	};

	class RuntimeConstantsPool
	{
	public:

		//bool HasConstant(ConstantPoolType type, uint16_t index);

		ConstantPoolType GetTypeOfConstant(uint16_t index);

		int32_t GetInteger(uint16_t index);
		float GetFloat(uint16_t index);
		const std::string& GetString(uint16_t index);
		//uint16_t AddOrGetFunctionReferenceIndex(std::string functionName);

	public:
		std::unordered_map<uint16_t, int32_t> m_Integers;
		std::unordered_map<uint16_t, float> m_Floats;
		std::unordered_map<uint16_t, std::string> m_Strings;

		std::unordered_map<uint16_t, std::string> m_FunctionReferences;
		std::unordered_map<uint16_t, std::string> m_ClassReferences;
	};



	// > 16 bit are stored in constants pool
	// floats are always stored, indexes are uint8
	// When index is larger, use uint16
	// signed byte push
	// signed short push

	class CompiledFile
	{
	public:
		RuntimeConstantsPool m_ConstantsPool;
		EncodedInstructions m_EncodedTopLevelInstructions;
		Instructions m_TopLevelInstructions;

		std::unordered_map<uint16_t, Instructions> m_Functions;
		std::unordered_map<uint16_t, EncodedInstructions> m_EncodedFunctions;

		std::unordered_map<uint16_t, ClassInstance> m_Classes;
	};

	class ContextConstantsPool
	{
	public:
		//bool HasConstant(ConstantType type, uint16_t index);

		uint16_t AddAndGetIntegerIndex(int32_t value);
		uint16_t AddAndGetFloatIndex(float value);
		uint16_t AddAndGetStringIndex(std::string value);
		uint16_t AddAndGetFunctionReferenceIndex(std::string functionName);
		uint16_t AddAndGetClassIndex(std::string className);
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

		uint16_t m_CurrentFreeSlot = 0;
	};

	class CompilerContext
	{
	public:
		struct Variable
		{
			std::string m_Name = "";
			uint16_t m_Index = 0;

			bool m_IsGlobal = false;

			ValueTypes m_Type = ValueTypes::Void;

			Variable(uint32_t index = 0, std::string name = "", ValueTypes type = ValueTypes::Void, bool isGlobal = false) : m_Name(name), m_Index(index), m_Type(type), m_IsGlobal(isGlobal) {};
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
		//uint32_t AddStringConstant(CompileTimeConstantsPool& constants, std::string string);

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

		ContextConstantsPool m_ConstantsPool;

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

		CompiledFile CompileASTTree(ASTNode* root);

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

		Opcodes ResolveCorrectMathInstruction(ASTNode* n, bool reverse = false);
		Opcodes ResolveCorrectStoreInstruction(ValueTypes type);
		ValueTypes GetValueTypeOfNode(ASTNode* node);

		void CompileClass(ASTNode* node);
		CompilerContext::Function* CompileClassMethod(ASTNode* node, CompilerContext::Class& classDeclaration);
		void CompileFunction(ASTNode* node);
		void CompileAssignment(ASTNode* node, Instructions& instructions);
		void CompileMathOperation(ASTNode* node, Instructions& instructions);
		void CompileIntLiteral(ASTNode* node, Instructions& instructions);
		void CompileDoubleLiteral(ASTNode* node, Instructions& instructions);
		void CompileStringLiteral(ASTNode* node, Instructions& instructions);

		Instruction LoadFromConstantsPool(uint16_t index);

		bool IsFunctionNameValid(const std::string& name);

		void MakeError(std::string error);

	public:
		std::string m_Error;

		uint32_t m_StartExecutionAt = 0;

		CompiledFile m_CompiledFile;

		CompilerContext m_Context;

		uint32_t m_CurrentScope = 0;
	};
}