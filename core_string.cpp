#include "core.h"

namespace Core {

	void concatenateStrings() {
		g_stack_instruction.at_r(1).modifier += g_stack_instruction.get_r(0).modifier;
		--g_stack_instruction.max_index;
	}
	void concatenateStringsOp() {
		g_stack_instruction.at_r(2).modifier += g_stack_instruction.get_r(0).modifier;
		g_op_mem.max_index = g_stack_instruction.get_r(1).shift;
		g_stack_instruction.max_index -= 2;
	}

}