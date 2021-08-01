#pragma once
#include "interpreter.h"

namespace Core
{
	Module getCore();

	//Module* core = 0;

	void test(Program& program);

	void getValueProcedure(Program& program);


	void invokeFunction(Program& program);
	void invokeNativeProcedure(Program& program);


	void allArrayInclusive(Program& program);
	void allArrayExclusive(Program& program);
	void allGroupInclusive(Program& program);
	void allGroupExclusive(Program& program);
	void allContext(Program& program);

	void comma(Program& program);

	void getChild(Program& program);

}