#pragma once
#include "interpreter.h"
#include "core_math.h"

namespace Core
{
	Module* initCore();
	Table<String, ValueType*> initCoreData();

	extern ValueType none;

	void doNothing();
	void destructorArr(ValueType* ptr, Instruction& instr, void* efficient0, Instruction& efficient1);


	extern "C" {
		void call_function(void* function_pointer, Instruction* parameter_pointer, size_t parameter_size);
	}

	extern "C" {
		uint64 call_test();
	}

	String fromInt(int64 num);
	String fromUint(uint64 num);

	String toStringGlobal(ValueType* value);
	String toStringLocal(Instruction instruction);
	String toStringLocalArray(Instruction instruction);
	
	void print(Instruction instr);
	void scan();

	void instantiate();
	void instantiateObject();
	void declareVariable();

	void conditional();
	void conditionalTrue();
	void conditionalFalse();

	void loopWhilePrefix(Instruction arg, Instruction op);
	//void loopWhilePostfix();
	void loopWhile(Instruction condition, Instruction expr);
	void loopWhileMoveValue(Instruction value);

	void loadLibrary();
	void freeLibrary();

	void getNamespace();
	void atContextByIndex();
	void atContextByName();

	void callObjectPrefix(Instruction obj, Instruction method);
	void callObjectPostfix();
	void callObjectBinaryLeft();
	void callObjectBinaryRight();
	//void callObjectCoalescing();

	void callWithContext();
	void renameArrayContext();


	void callThis();
	void contextMethod();


	void invokeResolve();
	void invokeProcedure();
	void invokeFunction();
	void invokeNativeFunction();
	void invokeNativeFunctionUnary();

	void callFunction(charT* function);

	void assignToName();
	void assignToReference();
	bool assign(void* toValue, ValueType toType, void* fromValue, ValueType fromType);
	void declareAssign();

	void ignore();


	void swapInstructionsRxRy();


	void getPointerR0();
	void getPointerR1();

	void getReferenceR0();
	void getReferenceR1();

	//Dereference a name
	void findValueR0();
	void findValueR1();
	void findValueR2();
	void findValueR0R2();

	//Dereference a reference
	void getValueR0();
	void getValueR1();
	void getValueR2();
	void getValueR0R2();

	ValueType* findName(String name);

	ValueType* nameGet();
	ValueType* nameAt();

	ValueType* tableGet();
	ValueType* tableAt();

	template<typename _TypeLeft, typename _TypeRight, typename _TypeResult, ValueType type, _TypeResult(*function) (_TypeLeft, _TypeRight) >
	void binaryFunctionInterface();


	/*void functionInterfaceForward() {
		void* func = memory.at<void*>(g_stack_instruction.get_r(1).shift);
		Array<Instruction>* param = &g_stack_array.at(g_stack_instruction.get_r(0).shift);

		reinterpret_cast<void(*)(Array<Instruction>*)>(func)(param);

		g_erase_s_r(3);

		specification->type.destructor[ValueType::arr](g_stack_instruction.get_r(1));
	}*/


	int64 add(int64 left, int64 right);

	template<typename _Type, typename _TypeResult>
	_TypeResult negate(_Type t) { return -t; }

	template<typename _Type>
	_Type negate(_Type t) { return -t; }

	template<typename _TypeLeft, typename _TypeRight, typename _TypeResult, ValueType _valueType>
	void createFloat() {
		_TypeResult number = 0;
		_TypeLeft left = g_val_mem.get<_TypeLeft>(g_stack_instruction.get_r(2).shift);
		_TypeRight right = g_val_mem.get<_TypeLeft>(g_stack_instruction.get_r(0).shift);

		if (right == 0)
			number = left * 1.0;
		else
			number = left * 1.0 + ((right * 1.0) / pow(10, floor(log10(right)) + 1));

		g_val_mem.max_index -= sizeof(void*) + sizeof(_TypeLeft) + sizeof(_TypeRight);

		g_stack_instruction.max_index -= 2;
		g_stack_instruction.at_r(0).value = _valueType;
		g_val_mem.add<_TypeResult>(number);
	}

	void contextAtIndex();
	void contextAtName();

	void concatenate(Instruction& instruction_r2, Instruction instruction_r0, Instruction instruction_r1);
	void concatenateStrings();
	void concatenateStringsOp();

	enum class EntryPrefix : charT {
		reserved,
		prefix,
		postfix,
		binaryLeft,
		binaryRight,
		enterContext,
		exitContext,
	};
	void getTableEntryLiteral(String);
	void getTableEntry();
	void getTablePrefix();
	void getTablePostfix();
	void getTableBinaryLeft();
	void getTableBinaryRight();

	void tableEntry(String);
	//void tableGet();
	//void tableAt();
	void tablePrefix();
	void tablePostfix();
	void tableBinaryLeft();
	void tableBinaryRight();

	void getNamespaceEntryLiteral(String);
	void getNamespaceEntry();
	void getNamespacePrefix();
	void getNamespacePostfix();
	void getNamespaceBinaryLeft();
	void getNamespaceBinaryRight();

	void allArrayInclusive();
	void allArrayExclusive();
	void allGroupInclusive();
	void allGroupExclusive();
	void allContext();

	void commaPrefix();
	void commaPostfix();
	void commaBinary();

	void getChild();

	void onEnterContextNamespace();
	void onEnterContextReference();
	void onExitContextNamespace();
}