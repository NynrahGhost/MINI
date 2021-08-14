#pragma once
#include "interpreter.h"

namespace Core
{
	Module getCore();

	extern "C" {
		void call_function(void* function_pointer, Instruction* parameter_pointer, size_t parameter_size);
	}

	void test(Program& program);

	void getValueProcedure(Program& program);
	/* 
	[123; "string"] -> function
	{arg0: 123; arg1: "string"} -> function
	@arg0 /= 2 , print @arg1
	@arg0 <= @0 , @arg1 <= @1;
	@[number;string];
	@number
	
	dll::function[

	function[arg0; arg1; arg2]
	function@[arg1; arg2];

	*/

	void invokeFunction(Program& program);
	void invokeNativeProcedure(Program& program);

	void assign(Program& program);

	void nameLookupR0(Program& program);
	void nameLookupR1(Program& program);
	void nameLookupR2(Program& program);


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



	void add(Program& program);
	//template<typename _TypeLeft, typename _TypeRight, typename _TypeResult>
	//void sub(Program& program);

	template<typename _TypeLeft, typename _TypeRight, typename _TypeResult, ValueType _valueType>
	void createFloat(Program& program);


	void allArrayInclusive(Program& program);
	void allArrayExclusive(Program& program);
	void allGroupInclusive(Program& program);
	void allGroupExclusive(Program& program);
	void allContext(Program& program);

	void comma(Program& program);

	void getChild(Program& program);

}