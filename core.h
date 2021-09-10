#pragma once
#include "interpreter.h"

namespace Core
{
	Module* initCore();
	Table<String, ValueType*> initCoreData();

	extern "C" {
		void call_function(void* function_pointer, Instruction* parameter_pointer, size_t parameter_size);
	}

	String fromInt(int64 num);
	String fromUint(uint64 num);

	String toStringGlobal(ValueType* value);
	String toStringLocal(Instruction instruction);
	String toStringLocalArray(Instruction instruction);
	void print();
	void scan();


	void conditional();
	void conditionalTrue();
	void conditionalFalse();


	void getValueProcedure();

	void getNamespace();
	void atContextByIndex();
	void atContextByName();

	void callWithContext();
	void renameArrayContext();


	void callThis();
	void contextMethod();


	void invokeResolve();
	void invokeProcedure();
	void invokeFunction();
	void invokeNativeFunction();

	void assign();

	void ignore();

	template<size_t _index_r>
	void getPointer();

	template<size_t _index_r>
	void getReference();

	template<size_t _index_r>
	void getValue();


	template<typename _TypeLeft, typename _TypeRight, typename _TypeResult, ValueType type, _TypeResult(*function) (_TypeLeft, _TypeRight) >
	void binaryFunctionInterface();


	/*void functionInterfaceForward() {
		void* func = memory.at<void*>(stack.instructions.get_r(1).shift);
		Array<Instruction>* param = &stack.arrays.at(stack.instructions.get_r(0).shift);

		reinterpret_cast<void(*)(Array<Instruction>*)>(func)(param);

		g_erase_s_r(3);

		specification->type.destructor[ValueType::arr](stack.instructions.get_r(1));
	}*/



	template<typename _TypeLeft, typename _TypeRight, typename _TypeResult>
	_TypeResult add(_TypeLeft l, _TypeRight r) { return l + r; }

	template<typename _TypeLeft, typename _TypeRight, typename _TypeResult>
	_TypeResult sub(_TypeLeft l, _TypeRight r) { return l - r; }

	template<typename _TypeLeft, typename _TypeRight, typename _TypeResult>
	_TypeResult mul(_TypeLeft l, _TypeRight r) { return l * r; }

	template<typename _TypeLeft, typename _TypeRight, typename _TypeResult>
	_TypeResult div(_TypeLeft l, _TypeRight r) { return l / r; }

	template<typename _Type, typename _TypeResult>
	_TypeResult negate(_Type t) { return -t; }

	template<typename _Type>
	_Type negate(_Type t) { return -t; }

	template<typename _TypeLeft, typename _TypeRight, typename _TypeResult, ValueType _valueType>
	void createFloat();

	void contextAtIndex();
	void contextAtName();

	void concatenate();

	void allArrayInclusive();
	void allArrayExclusive();
	void allGroupInclusive();
	void allGroupExclusive();
	void allContext();

	void commaPrefix();
	void commaPostfix();
	void commaBinary();

	void getChild();

}