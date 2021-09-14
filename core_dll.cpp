#include "Windows.h"
#include <libloaderapi.h>

//constexpr auto loadDLL = LoadLibraryW;
//constexpr auto unloadDLL = FreeLibrary;

#include "core.h"

namespace Core {
	void loadLibrary() {
		auto instr = g_stack_instruction.get(0);
		void* dll;
#if ENCODING == 8
		wchar_t* wideName;	//TODO: check if succeeded
		mbstowcs(wideName, (charT*)(g_memory.content + instr.shift), instr.modifier);
		dll = LoadLibraryW(wideName);
#elif ENCODING == 16
		dll = LoadLibraryW((charT*)(g_memory.content + shift));
#elif ENCODING == 32
#error not implemented
#endif
		g_memory_delete_span_r(2);
		g_stack_instruction.add(Instruction::val(ValueType::dll, g_memory.max_index));
		g_memory.add<void*>(dll);
	}

	void freeLibrary() {
		FreeLibrary(g_memory.at<HMODULE>(g_stack_instruction.get_r(0).shift));
		g_memory_delete_span_r(2);
	}

	void getProcedure() {
	
	}
}