#include "Windows.h"
#include <libloaderapi.h>

//constexpr auto loadDLL = LoadLibraryW;
//constexpr auto unloadDLL = FreeLibrary;

#include "core.h"
#include <iostream>

namespace Core {
	void loadLibrary() {
		auto instr = g_stack_instruction.get(0);
		void* dll;
#if ENCODING == 8
		wchar_t* wideName = (wchar_t*)alloca(instr.modifier);	//TODO: check if succeeded
		size_t charConvertedNum;
		mbstowcs_s(&charConvertedNum, wideName, instr.modifier, (charT*)(g_val_mem.content + instr.shift), instr.modifier);
		dll = LoadLibraryW(wideName);
#elif ENCODING == 16
		dll = LoadLibraryW((charT*)(g_memory.content + shift));
#elif ENCODING == 32
#error not implemented
#endif
		std::cout << dll << std::endl;
		g_memory_delete_span_r(2);
		g_stack_instruction.add(Instruction::val(ValueType::dll, g_val_mem.max_index));
		g_val_mem.add<void*>(dll);
	}

	void freeLibrary() {
		FreeLibrary(g_val_mem.at<HMODULE>(g_stack_instruction.get_r(0).shift));
		g_memory_delete_span_r(2);
	}

	void getProcedure() {
	
	}
}