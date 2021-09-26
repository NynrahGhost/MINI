#include "core.h"

namespace Core {

	void destructorArr(ValueType* ptr, Instruction& instr, void* efficient0, Instruction& efficient1) {
		/*auto arr = g_stack_array.get(instr.shift);

		efficient0 = &g_specification->type.destructor;

		instr.shift = arr.get(0).shift; //Naive approach, assuming data is 'tight' //IF_ERROR:array

		while (arr.max_index) {
			efficient1 = arr.get_r(0);
			if (((Table<ValueType, Destructor>&)efficient0).count(efficient1.value)) {
				((DestructorProcedure)((Table<ValueType, Destructor>&)efficient0)[efficient1.value])(g_memory.content + efficient1.shift, efficient1);
			}
			else
			{
				((Table<ValueType, Destructor>&)efficient0)[ValueType::all](g_memory.content + efficient1.shift);
			}
			--arr.max_index;
		}*/
	}

}