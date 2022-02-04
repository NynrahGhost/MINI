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

	void onEnterContextReference() {
		Instruction context = g_stack_instruction.get_r(1);
		switch (**(ValueType**)(g_val_mem.content + context.shift))
		{
		case ValueType::table:
			/*auto ptr = *(ValueType**)(g_memory.content + context.shift);
			auto ptr1 = ptr + 1;
			auto ptr2 = (Table<String, ValueType*>**)ptr1;*/
			//auto ptr3 = (Table<String, ValueType*>*)(g_memory.content + context.shift) + 1;
			g_stack_namespace.add(
				*(Table<String, ValueType*>**)
					(*(ValueType**)(g_val_mem.content + context.shift) + 1)
			);
			return;
		}
	}

	void onExitContextReference() {
		Instruction context = g_stack_instruction.get_r(1);
		switch ((ValueType)context.modifier)
		{
		case ValueType::table:
			--g_stack_namespace.max_index;
			return;
		}
	}

	void onEnterContextNamespace() {
		g_stack_namespace.add(g_val_mem.at<Table<String, ValueType*>*>(g_stack_instruction.get_r(1).shift));
	}

	void onExitContextNamespace() {
		--g_stack_namespace.max_index;
	}
}