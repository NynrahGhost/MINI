#include "interpreter.h"
#include "core.h"
#include <iostream>
#include <istream>
#include <fstream>
#include <sstream>
#include <functional>
#include "common_types.h"
#include "common_utils.h"

int main()
{
	const charT* script;

	//script = T("!<<('Input ''text: \n'), !>>ref, !<<'Your text: \n', !<<ref , !<<'\n'");
	//script = T("1234.print['well'; 'World!'];");
	script = T("print['Hello';'World';'!'];");
	//script = T("load_library('C:\\Users\\Ghost\\source\\repos\\TestDll\\x64\\Debug\\TestDll.dll');");

	//auto table = new Table<String, ValueType*>();

	//g_data.init();// = *(new Array<Table<String, ValueType*>>());
	g_data = Core::initCoreData();//Table<String, ValueType*>();
	g_stack_namespace.add(&g_data);
	//g_memory.add<Table<String, ValueType*>*>(table);
	g_stack_context.add(Instruction::val(ValueType::autotable, 0));
	g_stack_instruction.init();// = *(new Array<Instruction>());
	g_stack_instruction.add(Instruction::atom(InstructionType::start));


	//std::ifstream stream = std::ifstream("tests.mini");
	std::ifstream stream = std::ifstream("test.mini");
	//std::istringstream stream = std::istringstream(script);

	/*
	std::stringstream stream = std::stringstream();
	stream << script;/**/

	int result = (int)run(stream);
	//std::cout << g_memory.max_index << std::endl;
	//std::cout << Core::call_test() << std::endl;
	return result;
	/**/
}