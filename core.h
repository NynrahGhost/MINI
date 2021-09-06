#pragma once
#include "interpreter.h"

namespace Core
{
	Module initCore();
	Table<String, ValueType*> initCoreData();

	extern "C" {
		void call_function(void* function_pointer, Instruction* parameter_pointer, size_t parameter_size);
	}

	String fromInt(int64 num);
	String fromUint(uint64 num);

	String toStringGlobal(Program& program, ValueType* value);
	String toStringLocal(Program& program, Instruction instruction);
	String toStringLocalArray(Program& program, Instruction instruction);
	void print(Program& program);
	void scan(Program& program);


	void conditional(Program& program);
	void conditionalTrue(Program& program);
	void conditionalFalse(Program& program);


	void getValueProcedure(Program& program);

	void getNamespace(Program& program);
	void atContextByIndex(Program& program);
	void atContextByName(Program& program);

	void callWithContext(Program& program);
	void renameArrayContext(Program& program);


	void callThis(Program& program);
	void contextMethod(Program& program);


	void invokeResolve(Program& program);
	void invokeProcedure(Program& program);
	void invokeFunction(Program& program);
	void invokeNativeFunction(Program& program);

	void assign(Program& program);

	void ignore(Program& program);

	template<size_t _index_r>
	void getPointer(Program& program);

	template<size_t _index_r>
	void getReference(Program& program);

	template<size_t _index_r>
	void getValue(Program& program);


	template<typename _TypeLeft, typename _TypeRight, typename _TypeResult, ValueType type, _TypeResult(*function) (_TypeLeft, _TypeRight) >
	void binaryFunctionInterface(Program& program);


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
	void createFloat(Program& program);

	void contextAtIndex(Program& program);
	void contextAtName(Program& program);

	void concatenate(Program& program);

	void allArrayInclusive(Program& program);
	void allArrayExclusive(Program& program);
	void allGroupInclusive(Program& program);
	void allGroupExclusive(Program& program);
	void allContext(Program& program);

	void commaPrefix(Program& program);
	void commaPostfix(Program& program);
	void commaBinary(Program& program);

	void getChild(Program& program);

}