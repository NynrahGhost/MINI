#pragma once
#include "interpreter.h"

namespace Core
{
	Module getCore();

	//Module* core = 0;

	void test(Program& program);

	void getValueProcedure(Program& program);
	/* 
	[123; "string"] -> function
	{arg0: 123; arg1: "string"} -> function
	@arg0 /= 2 , print @arg1
	@arg0 <= @0 , @arg1 <= @1;
	@[number;string];
	@number
	
	


	*/

	void invokeFunction(Program& program);
	void invokeNativeProcedure(Program& program);

	void add(Program& program);
	void sub(Program& program);
	void mul(Program& program);
	void div(Program& program);

	void addf(Program& program);
	void subf(Program& program);
	void mulf(Program& program);
	void divf(Program& program);


	void allArrayInclusive(Program& program);
	void allArrayExclusive(Program& program);
	void allGroupInclusive(Program& program);
	void allGroupExclusive(Program& program);
	void allContext(Program& program);

	void comma(Program& program);

	void getChild(Program& program);

}